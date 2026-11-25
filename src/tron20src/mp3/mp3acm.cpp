/*
 * mp3acm.cpp -- the modern build's MP3 codec in SndDrv.dll (MODERN.md X-2);
 * see mp3acm.h. #included once, at the end of s_dx8.cpp, in a
 * TRON20SRC_MODERN block.
 *
 * An ACM driver procedure, as an installable ACM codec has, for one
 * conversion: MP3 (WAVE_FORMAT_MPEGLAYER3) to 16-bit PCM with the same
 * rate and channels. ACM calls the procedure with the messages of the ACM
 * driver interface (msacmdrv.h, of the Windows DDK; the compiler has no
 * copy of it, so the structures and messages that this file uses are
 * declared below). ACM gives the driver the Unicode forms of the details
 * structures.
 *
 * minimp3 (github.com/lieff/minimp3, CC0: minimp3/LICENSE) is compiled in
 * here without its SSE code (Visual C++ 6.0 has no <intrin.h>).
 *
 * ACM_STREAMCONVERTF_START starts a stream again: the input and the PCM
 * that it holds are dropped (SndDrv sets it after WaveFile::Cue).
 *
 * The conversion. SndDrv converts in two ways:
 *
 *   * a sound loaded whole (InitSampleFromFile, Init3DSampleFromFile,
 *     DecompressASI): one acmStreamConvert with all of the MP3 data;
 *   * a streamed sound (WaveFile::ReadCompressed): one acmStreamConvert
 *     for each block that it reads from the file (acmStreamSize of 8 KB of
 *     PCM), into an 8 KB buffer. It does not read cbSrcLengthUsed: each
 *     block that it gives must be used completely.
 *
 * So each convert takes all of its input, adds it to the input that the
 * stream holds, decodes every complete frame, and gives as much PCM as the
 * destination holds. The stream keeps the PCM that did not fit for the next
 * convert, and the bytes of a frame that is not complete yet. A frame is
 * complete when the header of the next frame is also there, or when the
 * input ends with the frame (minimp3's test). Fraunhofer's codec also holds
 * input and output in acmStreamConvert.
 *
 * Modern build only (TRON20SRC_MODERN).
 */
#define MINIMP3_IMPLEMENTATION
#define MINIMP3_NO_SIMD
#include "minimp3/minimp3.h"

// ----------------------------------------------------------------------- //
//  The ACM driver interface (msacmdrv.h)
// ----------------------------------------------------------------------- //

#define MP3ACM_DM_BASE                  (DRV_USER + 0x2000)
#define MP3ACM_DM_DRIVER_NOTIFY         (MP3ACM_DM_BASE + 1)
#define MP3ACM_DM_DRIVER_DETAILS        (MP3ACM_DM_BASE + 10)
#define MP3ACM_DM_DRIVER_ABOUT          (MP3ACM_DM_BASE + 11)
#define MP3ACM_DM_FORMATTAG_DETAILS     (MP3ACM_DM_BASE + 25)
#define MP3ACM_DM_FORMAT_DETAILS        (MP3ACM_DM_BASE + 26)
#define MP3ACM_DM_FORMAT_SUGGEST        (MP3ACM_DM_BASE + 27)
#define MP3ACM_DM_STREAM_OPEN           (MP3ACM_DM_BASE + 76)
#define MP3ACM_DM_STREAM_CLOSE          (MP3ACM_DM_BASE + 77)
#define MP3ACM_DM_STREAM_SIZE           (MP3ACM_DM_BASE + 78)
#define MP3ACM_DM_STREAM_CONVERT        (MP3ACM_DM_BASE + 79)
#define MP3ACM_DM_STREAM_RESET          (MP3ACM_DM_BASE + 80)
#define MP3ACM_DM_STREAM_PREPARE        (MP3ACM_DM_BASE + 81)
#define MP3ACM_DM_STREAM_UNPREPARE      (MP3ACM_DM_BASE + 82)

struct Mp3AcmStreamInstance             // ACMDRVSTREAMINSTANCE
{
	DWORD           cbStruct;
	LPWAVEFORMATEX  pwfxSrc;
	LPWAVEFORMATEX  pwfxDst;
	LPWAVEFILTER    pwfltr;
	DWORD           dwCallback;
	DWORD           dwInstance;
	DWORD           fdwOpen;
	DWORD           fdwDriver;
	DWORD           dwDriver;
	HACMSTREAM      has;
};

struct Mp3AcmStreamHeader               // ACMDRVSTREAMHEADER (its start)
{
	DWORD           cbStruct;
	DWORD           fdwStatus;
	DWORD           dwUser;
	LPBYTE          pbSrc;
	DWORD           cbSrcLength;
	DWORD           cbSrcLengthUsed;
	DWORD           dwSrcUser;
	LPBYTE          pbDst;
	DWORD           cbDstLength;
	DWORD           cbDstLengthUsed;
	DWORD           dwDstUser;
	DWORD           fdwConvert;
};

struct Mp3AcmStreamSize                 // ACMDRVSTREAMSIZE
{
	DWORD           cbStruct;
	DWORD           fdwSize;
	DWORD           cbSrcLength;
	DWORD           cbDstLength;
};

struct Mp3AcmFormatSuggest              // ACMDRVFORMATSUGGEST
{
	DWORD           cbStruct;
	DWORD           fdwSuggest;
	LPWAVEFORMATEX  pwfxSrc;
	DWORD           cbwfxSrc;
	LPWAVEFORMATEX  pwfxDst;
	DWORD           cbwfxDst;
};

// ----------------------------------------------------------------------- //
//  The formats
// ----------------------------------------------------------------------- //

// The rates of MPEG-1, -2 and 2.5 layer III; each with one and two
// channels. The enumeration of the formats (ACMDM_FORMAT_DETAILS by index)
// lists these, for each of the two format tags.
static const DWORD s_aMp3AcmRates[] = { 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000 };
#define MP3ACM_NUM_RATES    ( sizeof( s_aMp3AcmRates ) / sizeof( s_aMp3AcmRates[0] ))
#define MP3ACM_NUM_FORMATS  ( MP3ACM_NUM_RATES * 2 )

static bool Mp3Acm_IsRate( DWORD nRate )
{
	for( unsigned int i = 0; i < MP3ACM_NUM_RATES; ++i )
	{
		if( s_aMp3AcmRates[i] == nRate )
			return true;
	}
	return false;
}

static bool Mp3Acm_IsMp3( const WAVEFORMATEX* pwfx )
{
	return pwfx && pwfx->wFormatTag == WAVE_FORMAT_MPEGLAYER3 &&
		( pwfx->nChannels == 1 || pwfx->nChannels == 2 ) && Mp3Acm_IsRate( pwfx->nSamplesPerSec );
}

static bool Mp3Acm_IsPcm16( const WAVEFORMATEX* pwfx )
{
	return pwfx && pwfx->wFormatTag == WAVE_FORMAT_PCM && pwfx->wBitsPerSample == 16 &&
		( pwfx->nChannels == 1 || pwfx->nChannels == 2 ) &&
		pwfx->nBlockAlign == pwfx->nChannels * 2 && Mp3Acm_IsRate( pwfx->nSamplesPerSec );
}

static void Mp3Acm_MakePcm( WAVEFORMATEX* pwfx, DWORD nRate, WORD nChannels )
{
	pwfx->wFormatTag = WAVE_FORMAT_PCM;
	pwfx->nChannels = nChannels;
	pwfx->nSamplesPerSec = nRate;
	pwfx->wBitsPerSample = 16;
	pwfx->nBlockAlign = ( WORD )( nChannels * 2 );
	pwfx->nAvgBytesPerSec = nRate * pwfx->nBlockAlign;
}

// An MP3 format of the enumeration: 64 kbit/s for each channel (40 below
// 32 kHz), as good as any: the decoder reads the real rate in each frame.
static void Mp3Acm_MakeMp3( MPEGLAYER3WAVEFORMAT* pmp3, DWORD nRate, WORD nChannels )
{
	DWORD nBitRate = ( nRate >= 32000 ? 64000 : 40000 ) * nChannels;

	memset( pmp3, 0, sizeof( *pmp3 ));
	pmp3->wfx.wFormatTag = WAVE_FORMAT_MPEGLAYER3;
	pmp3->wfx.nChannels = nChannels;
	pmp3->wfx.nSamplesPerSec = nRate;
	pmp3->wfx.nAvgBytesPerSec = nBitRate / 8;
	pmp3->wfx.nBlockAlign = 1;
	pmp3->wfx.wBitsPerSample = 0;
	pmp3->wfx.cbSize = MPEGLAYER3_WFX_EXTRA_BYTES;
	pmp3->wID = MPEGLAYER3_ID_MPEG;
	pmp3->fdwFlags = MPEGLAYER3_FLAG_PADDING_ISO;
	pmp3->nBlockSize = ( WORD )(( nRate >= 32000 ? 144 : 72 ) * nBitRate / nRate );
	pmp3->nFramesPerBlock = 1;
	pmp3->nCodecDelay = 0;
}

static void Mp3Acm_CopyName( WCHAR* pDest, unsigned int nChars, const char* pSrc )
{
	unsigned int i;

	for( i = 0; i + 1 < nChars && pSrc[i]; ++i )
		pDest[i] = ( WCHAR )( unsigned char )pSrc[i];
	if( nChars )
		pDest[i] = 0;
}

static void Mp3Acm_FormatName( WCHAR* pDest, unsigned int nChars, const WAVEFORMATEX* pwfx )
{
	char szName[64];

	if( pwfx->wFormatTag == WAVE_FORMAT_PCM )
		wsprintfA( szName, "%lu Hz, %u bit, %s", pwfx->nSamplesPerSec, pwfx->wBitsPerSample,
			pwfx->nChannels == 1 ? "mono" : "stereo" );
	else
		wsprintfA( szName, "%lu kbit/s, %lu Hz, %s", pwfx->nAvgBytesPerSec * 8 / 1000,
			pwfx->nSamplesPerSec, pwfx->nChannels == 1 ? "mono" : "stereo" );
	Mp3Acm_CopyName( pDest, nChars, szName );
}

// ----------------------------------------------------------------------- //
//  A stream
// ----------------------------------------------------------------------- //

struct Mp3AcmStream
{
	mp3dec_t        m_Decoder;
	unsigned int    m_nChannels;            // of the PCM
	unsigned char*  m_pIn;                  // MP3 bytes not decoded yet
	unsigned int    m_nIn, m_nInAlloc;
	unsigned char*  m_pOut;                 // PCM not given yet
	unsigned int    m_nOutStart, m_nOut, m_nOutAlloc;
};

// When minimp3 finds no frame in its input, it gives the whole input as
// bytes to skip. Out of sync (at the start, after a reset), it finds a frame
// only when the header of the next frame is also in the input, and a
// streamed block can be shorter than two frames (SndDrv's are about 750
// bytes for 64 kbit/s). So the codec drops input in which minimp3 finds no
// frame only when there is this much of it: ten of the largest layer III
// frames (1441 bytes), as many as minimp3 compares to find its sync.
#define MP3ACM_IN_MIN       16384

static HACMDRIVERID s_hMp3AcmDriverId = NULL;

static bool Mp3Acm_Reserve( unsigned char** ppData, unsigned int* pnAlloc, unsigned int nNeed )
{
	if( nNeed <= *pnAlloc )
		return true;

	unsigned int nNew = *pnAlloc ? *pnAlloc : 16384;
	while( nNew < nNeed )
		nNew *= 2;
	unsigned char* pNew = ( unsigned char* )realloc( *ppData, nNew );
	if( !pNew )
		return false;
	*ppData = pNew;
	*pnAlloc = nNew;
	return true;
}

static void Mp3Acm_Reset( Mp3AcmStream* pStream )
{
	mp3dec_init( &pStream->m_Decoder );
	pStream->m_nIn = 0;
	pStream->m_nOutStart = 0;
	pStream->m_nOut = 0;
}

// Decodes every complete frame of the input into the output.
// bEnd: the input ends here (ACM_STREAMCONVERTF_END).
static bool Mp3Acm_Decode( Mp3AcmStream* pStream, bool bEnd )
{
	mp3d_sample_t aPcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
	unsigned int nUsed = 0;

	// the PCM given already goes first
	if( pStream->m_nOutStart )
	{
		memmove( pStream->m_pOut, pStream->m_pOut + pStream->m_nOutStart, pStream->m_nOut );
		pStream->m_nOutStart = 0;
	}

	while( nUsed < pStream->m_nIn )
	{
		const unsigned char* pFrame = pStream->m_pIn + nUsed;
		unsigned int nLeft = pStream->m_nIn - nUsed;

		// In sync, the codec decodes a frame only when all of it is here.
		// minimp3 searches again for a frame that is not complete (or
		// whose next header is not here yet), and resets its decoder for
		// it: the bit reservoir goes, and the frames after it decode to
		// nothing. So the codec gives minimp3 exactly the frame, which
		// minimp3 then takes as it is. It waits while fewer bytes than a
		// header follow the frame (they can be the start of the next
		// header), except at the end of the input.
		unsigned int nGive = nLeft;
		if( pStream->m_Decoder.header[0] == 0xff )
		{
			if( nLeft <= HDR_SIZE )
			{
				if( !bEnd )
					break;
			}
			else if( hdr_compare( pStream->m_Decoder.header, pFrame ))
			{
				unsigned int nSize = ( unsigned int )( hdr_frame_bytes( pFrame, pStream->m_Decoder.free_format_bytes ) +
					hdr_padding( pFrame ));
				if( nSize > nLeft || ( !bEnd && nSize < nLeft && nLeft - nSize < HDR_SIZE ))
					break;
				nGive = nSize;
			}
		}

		mp3dec_frame_info_t Info;
		int nSamples = mp3dec_decode_frame( &pStream->m_Decoder, pFrame, ( int )nGive, aPcm, &Info );

		if( Info.frame_bytes <= 0 )
			break;                          // the frame is not complete yet
		if( nSamples <= 0 && ( unsigned int )Info.frame_bytes == nLeft && nLeft < MP3ACM_IN_MIN && !bEnd )
			break;                          // no frame found in a short input: wait for more
		nUsed += ( unsigned int )Info.frame_bytes;
		if( nSamples <= 0 || Info.channels < 1 )
			continue;                       // skipped data: a tag, an error

		unsigned int nBytes = ( unsigned int )nSamples * pStream->m_nChannels * 2;
		if( !Mp3Acm_Reserve( &pStream->m_pOut, &pStream->m_nOutAlloc, pStream->m_nOut + nBytes ))
			return false;

		short* pDest = ( short* )( pStream->m_pOut + pStream->m_nOut );
		if(( unsigned int )Info.channels == pStream->m_nChannels )
			memcpy( pDest, aPcm, nBytes );
		else if( pStream->m_nChannels == 1 )
		{
			// a stereo frame in a mono stream: the mean of the two
			for( int i = 0; i < nSamples; ++i )
				pDest[i] = ( short )(( aPcm[i * 2] + aPcm[i * 2 + 1] ) / 2 );
		}
		else
		{
			// a mono frame in a stereo stream: on both channels
			for( int j = 0; j < nSamples; ++j )
				pDest[j * 2] = pDest[j * 2 + 1] = aPcm[j];
		}
		pStream->m_nOut += nBytes;
	}

	pStream->m_nIn -= nUsed;
	if( nUsed && pStream->m_nIn )
		memmove( pStream->m_pIn, pStream->m_pIn + nUsed, pStream->m_nIn );
	return true;
}

static LRESULT Mp3Acm_Convert( Mp3AcmStreamInstance* pInst, Mp3AcmStreamHeader* pHdr )
{
	Mp3AcmStream* pStream = ( Mp3AcmStream* )pInst->dwDriver;
	if( !pStream )
		return MMSYSERR_INVALHANDLE;

	if( pHdr->fdwConvert & ACM_STREAMCONVERTF_START )
		Mp3Acm_Reset( pStream );

	// all of the input, always (see the header)
	if( pHdr->cbSrcLength )
	{
		if( !Mp3Acm_Reserve( &pStream->m_pIn, &pStream->m_nInAlloc, pStream->m_nIn + pHdr->cbSrcLength ))
			return MMSYSERR_NOMEM;
		memcpy( pStream->m_pIn + pStream->m_nIn, pHdr->pbSrc, pHdr->cbSrcLength );
		pStream->m_nIn += pHdr->cbSrcLength;
	}
	pHdr->cbSrcLengthUsed = pHdr->cbSrcLength;

	if( !Mp3Acm_Decode( pStream, ( pHdr->fdwConvert & ACM_STREAMCONVERTF_END ) != 0 ))
		return MMSYSERR_NOMEM;

	// whole sample frames only
	unsigned int nGive = pHdr->cbDstLength - pHdr->cbDstLength % ( pStream->m_nChannels * 2 );
	if( nGive > pStream->m_nOut )
		nGive = pStream->m_nOut;
	memcpy( pHdr->pbDst, pStream->m_pOut + pStream->m_nOutStart, nGive );
	pStream->m_nOutStart += nGive;
	pStream->m_nOut -= nGive;
	pHdr->cbDstLengthUsed = nGive;

	if( pHdr->fdwConvert & ACM_STREAMCONVERTF_END )
	{
		// the last input: what is still held of it is not a frame
		pStream->m_nIn = 0;
	}
	return MMSYSERR_NOERROR;
}

static LRESULT Mp3Acm_Open( Mp3AcmStreamInstance* pInst )
{
	if( !Mp3Acm_IsMp3( pInst->pwfxSrc ) || !Mp3Acm_IsPcm16( pInst->pwfxDst ) ||
		pInst->pwfxSrc->nSamplesPerSec != pInst->pwfxDst->nSamplesPerSec ||
		pInst->pwfxSrc->nChannels != pInst->pwfxDst->nChannels || pInst->pwfltr )
		return ACMERR_NOTPOSSIBLE;
	if( pInst->fdwOpen & ACM_STREAMOPENF_QUERY )
		return MMSYSERR_NOERROR;

	Mp3AcmStream* pStream = ( Mp3AcmStream* )calloc( 1, sizeof( Mp3AcmStream ));
	if( !pStream )
		return MMSYSERR_NOMEM;
	pStream->m_nChannels = pInst->pwfxDst->nChannels;
	Mp3Acm_Reset( pStream );
	pInst->dwDriver = ( DWORD )pStream;

	char szLine[96];
	wsprintfA( szLine, "SndDrv: MP3 codec: stream %p, %lu Hz, %u channel(s)\n",
		( void* )pStream, pInst->pwfxSrc->nSamplesPerSec, pInst->pwfxSrc->nChannels );
	OutputDebugStringA( szLine );
	return MMSYSERR_NOERROR;
}

static LRESULT Mp3Acm_Close( Mp3AcmStreamInstance* pInst )
{
	Mp3AcmStream* pStream = ( Mp3AcmStream* )pInst->dwDriver;
	if( !pStream )
		return MMSYSERR_NOERROR;

	free( pStream->m_pIn );
	free( pStream->m_pOut );
	free( pStream );
	pInst->dwDriver = 0;
	return MMSYSERR_NOERROR;
}

// The sizes for ACMDM_STREAM_SIZE, from the MP3's average rate in bytes: an
// estimate, as an MP3 codec's is (the rate of each frame can change).
static LRESULT Mp3Acm_Size( Mp3AcmStreamInstance* pInst, Mp3AcmStreamSize* pSize )
{
	const WAVEFORMATEX* pSrc = pInst->pwfxSrc;
	const WAVEFORMATEX* pDst = pInst->pwfxDst;
	double fPcmPerMp3 = ( double )pDst->nAvgBytesPerSec / ( pSrc->nAvgBytesPerSec ? pSrc->nAvgBytesPerSec : 1 );

	switch( pSize->fdwSize & ACM_STREAMSIZEF_QUERYMASK )
	{
	case ACM_STREAMSIZEF_SOURCE:
		// and room for one frame more
		pSize->cbDstLength = ( DWORD )( pSize->cbSrcLength * fPcmPerMp3 ) + 1152 * pDst->nBlockAlign;
		return MMSYSERR_NOERROR;
	case ACM_STREAMSIZEF_DESTINATION:
		// nine tenths: a block of input then decodes to less PCM than the
		// destination holds, on average, so the PCM that the codec keeps
		// for the next convert does not grow. SndDrv stops at the end of
		// the data, and loses what the codec still holds then.
		pSize->cbSrcLength = ( DWORD )( pSize->cbDstLength * 0.9 / fPcmPerMp3 );
		if( pSize->cbSrcLength == 0 )
			return ACMERR_NOTPOSSIBLE;
		return MMSYSERR_NOERROR;
	}
	return MMSYSERR_INVALFLAG;
}

// ----------------------------------------------------------------------- //
//  The details of the driver, its format tags and its formats
// ----------------------------------------------------------------------- //

static LRESULT Mp3Acm_DriverDetails( ACMDRIVERDETAILSW* pDetails )
{
	ACMDRIVERDETAILSW Details;
	memset( &Details, 0, sizeof( Details ));
	Details.cbStruct = sizeof( Details );
	Details.fccType = ACMDRIVERDETAILS_FCCTYPE_AUDIOCODEC;
	Details.fccComp = ACMDRIVERDETAILS_FCCCOMP_UNDEFINED;
	Details.wMid = 0xffff;                  // MM_UNMAPPED: no maker id
	Details.wPid = 0xffff;                  // MM_PID_UNMAPPED
	Details.vdwACM = 0x04000000;            // ACM 4.00
	Details.vdwDriver = 0x01000000;
	Details.fdwSupport = ACMDRIVERDETAILS_SUPPORTF_CODEC;
	Details.cFormatTags = 2;
	Details.cFilterTags = 0;
	Mp3Acm_CopyName( Details.szShortName, ACMDRIVERDETAILS_SHORTNAME_CHARS, "minimp3" );
	Mp3Acm_CopyName( Details.szLongName, ACMDRIVERDETAILS_LONGNAME_CHARS,
		"MPEG Layer-3 decoder of SndDrv.dll (minimp3)" );
	Mp3Acm_CopyName( Details.szCopyright, ACMDRIVERDETAILS_COPYRIGHT_CHARS,
		"minimp3: public domain (CC0)" );
	Mp3Acm_CopyName( Details.szLicensing, ACMDRIVERDETAILS_LICENSING_CHARS,
		"CC0 1.0, https://github.com/lieff/minimp3" );
	Mp3Acm_CopyName( Details.szFeatures, ACMDRIVERDETAILS_FEATURES_CHARS,
		"Decodes MPEG-1, -2 and 2.5 layer III to 16-bit PCM" );

	DWORD cbStruct = pDetails->cbStruct < sizeof( Details ) ? pDetails->cbStruct : sizeof( Details );
	memcpy( pDetails, &Details, cbStruct );
	pDetails->cbStruct = cbStruct;
	return MMSYSERR_NOERROR;
}

// Format tag 0 is PCM, 1 is MP3.
static LRESULT Mp3Acm_FormatTagDetails( ACMFORMATTAGDETAILSW* pTag, DWORD fdwDetails )
{
	DWORD nIndex;

	switch( fdwDetails & ACM_FORMATTAGDETAILSF_QUERYMASK )
	{
	case ACM_FORMATTAGDETAILSF_INDEX:
		nIndex = pTag->dwFormatTagIndex;
		if( nIndex > 1 )
			return ACMERR_NOTPOSSIBLE;
		break;
	case ACM_FORMATTAGDETAILSF_FORMATTAG:
		if( pTag->dwFormatTag == WAVE_FORMAT_PCM )
			nIndex = 0;
		else if( pTag->dwFormatTag == WAVE_FORMAT_MPEGLAYER3 )
			nIndex = 1;
		else
			return ACMERR_NOTPOSSIBLE;
		break;
	case ACM_FORMATTAGDETAILSF_LARGESTSIZE:
		if( pTag->dwFormatTag == WAVE_FORMAT_PCM )
			nIndex = 0;
		else if( pTag->dwFormatTag == WAVE_FORMAT_MPEGLAYER3 || pTag->dwFormatTag == WAVE_FORMAT_UNKNOWN )
			nIndex = 1;
		else
			return ACMERR_NOTPOSSIBLE;
		break;
	default:
		return MMSYSERR_NOTSUPPORTED;
	}

	pTag->dwFormatTagIndex = nIndex;
	pTag->dwFormatTag = nIndex ? WAVE_FORMAT_MPEGLAYER3 : WAVE_FORMAT_PCM;
	pTag->cbFormatSize = nIndex ? sizeof( MPEGLAYER3WAVEFORMAT ) : sizeof( PCMWAVEFORMAT );
	pTag->fdwSupport = ACMDRIVERDETAILS_SUPPORTF_CODEC;
	pTag->cStandardFormats = MP3ACM_NUM_FORMATS;
	Mp3Acm_CopyName( pTag->szFormatTag, ACMFORMATTAGDETAILS_FORMATTAG_CHARS,
		nIndex ? "MPEG Layer-3" : "PCM" );
	return MMSYSERR_NOERROR;
}

static LRESULT Mp3Acm_FormatDetails( ACMFORMATDETAILSW* pFormat, DWORD fdwDetails )
{
	switch( fdwDetails & ACM_FORMATDETAILSF_QUERYMASK )
	{
	case ACM_FORMATDETAILSF_INDEX:
		{
			if( pFormat->dwFormatIndex >= MP3ACM_NUM_FORMATS )
				return ACMERR_NOTPOSSIBLE;
			DWORD nRate = s_aMp3AcmRates[pFormat->dwFormatIndex / 2];
			WORD nChannels = ( WORD )( pFormat->dwFormatIndex % 2 + 1 );

			if( pFormat->dwFormatTag == WAVE_FORMAT_PCM )
			{
				if( pFormat->cbwfx < sizeof( PCMWAVEFORMAT ))
					return MMSYSERR_INVALPARAM;
				Mp3Acm_MakePcm( pFormat->pwfx, nRate, nChannels );
				if( pFormat->cbwfx >= sizeof( WAVEFORMATEX ))
					pFormat->pwfx->cbSize = 0;
				pFormat->cbwfx = pFormat->cbwfx >= sizeof( WAVEFORMATEX ) ? sizeof( WAVEFORMATEX ) : sizeof( PCMWAVEFORMAT );
			}
			else if( pFormat->dwFormatTag == WAVE_FORMAT_MPEGLAYER3 )
			{
				if( pFormat->cbwfx < sizeof( MPEGLAYER3WAVEFORMAT ))
					return MMSYSERR_INVALPARAM;
				Mp3Acm_MakeMp3( ( MPEGLAYER3WAVEFORMAT* )pFormat->pwfx, nRate, nChannels );
				pFormat->cbwfx = sizeof( MPEGLAYER3WAVEFORMAT );
			}
			else
				return ACMERR_NOTPOSSIBLE;
		}
		break;
	case ACM_FORMATDETAILSF_FORMAT:
		if( !Mp3Acm_IsMp3( pFormat->pwfx ) && !Mp3Acm_IsPcm16( pFormat->pwfx ))
			return ACMERR_NOTPOSSIBLE;
		break;
	default:
		return MMSYSERR_NOTSUPPORTED;
	}

	pFormat->fdwSupport = ACMDRIVERDETAILS_SUPPORTF_CODEC;
	Mp3Acm_FormatName( pFormat->szFormat, ACMFORMATDETAILS_FORMAT_CHARS, pFormat->pwfx );
	return MMSYSERR_NOERROR;
}

// From MP3: 16-bit PCM with its rate and its channels. A field that the
// caller fixes (ACM_FORMATSUGGESTF_*) must have that value.
static LRESULT Mp3Acm_FormatSuggest( Mp3AcmFormatSuggest* pSuggest )
{
	const WAVEFORMATEX* pSrc = pSuggest->pwfxSrc;
	WAVEFORMATEX* pDst = pSuggest->pwfxDst;
	DWORD fdw = pSuggest->fdwSuggest;

	if( !Mp3Acm_IsMp3( pSrc ) || pSuggest->cbwfxDst < sizeof( PCMWAVEFORMAT ))
		return ACMERR_NOTPOSSIBLE;
	if(( fdw & ACM_FORMATSUGGESTF_WFORMATTAG ) && pDst->wFormatTag != WAVE_FORMAT_PCM )
		return ACMERR_NOTPOSSIBLE;
	if(( fdw & ACM_FORMATSUGGESTF_NCHANNELS ) && pDst->nChannels != pSrc->nChannels )
		return ACMERR_NOTPOSSIBLE;
	if(( fdw & ACM_FORMATSUGGESTF_NSAMPLESPERSEC ) && pDst->nSamplesPerSec != pSrc->nSamplesPerSec )
		return ACMERR_NOTPOSSIBLE;
	if(( fdw & ACM_FORMATSUGGESTF_WBITSPERSAMPLE ) && pDst->wBitsPerSample != 16 )
		return ACMERR_NOTPOSSIBLE;

	Mp3Acm_MakePcm( pDst, pSrc->nSamplesPerSec, pSrc->nChannels );
	if( pSuggest->cbwfxDst >= sizeof( WAVEFORMATEX ))
		pDst->cbSize = 0;
	return MMSYSERR_NOERROR;
}

// ----------------------------------------------------------------------- //
//  The driver procedure
// ----------------------------------------------------------------------- //

static LRESULT CALLBACK Mp3Acm_DriverProc( DWORD dwId, HDRVR hDriver, UINT uMsg, LPARAM lParam1, LPARAM lParam2 )
{
	switch( uMsg )
	{
	case DRV_LOAD:
	case DRV_FREE:
	case DRV_ENABLE:
	case DRV_DISABLE:
	case DRV_CLOSE:
		return 1;
	case DRV_OPEN:
		return 1;                           // any nonzero id: the driver has no state
	case DRV_QUERYCONFIGURE:
		return 0;
	case DRV_CONFIGURE:
		return DRVCNF_OK;
	case DRV_INSTALL:
	case DRV_REMOVE:
		return DRVCNF_OK;

	case MP3ACM_DM_DRIVER_NOTIFY:
		return MMSYSERR_NOERROR;
	case MP3ACM_DM_DRIVER_DETAILS:
		return Mp3Acm_DriverDetails( ( ACMDRIVERDETAILSW* )lParam1 );
	case MP3ACM_DM_FORMATTAG_DETAILS:
		return Mp3Acm_FormatTagDetails( ( ACMFORMATTAGDETAILSW* )lParam1, ( DWORD )lParam2 );
	case MP3ACM_DM_FORMAT_DETAILS:
		return Mp3Acm_FormatDetails( ( ACMFORMATDETAILSW* )lParam1, ( DWORD )lParam2 );
	case MP3ACM_DM_FORMAT_SUGGEST:
		return Mp3Acm_FormatSuggest( ( Mp3AcmFormatSuggest* )lParam1 );
	case MP3ACM_DM_STREAM_OPEN:
		return Mp3Acm_Open( ( Mp3AcmStreamInstance* )lParam1 );
	case MP3ACM_DM_STREAM_CLOSE:
		return Mp3Acm_Close( ( Mp3AcmStreamInstance* )lParam1 );
	case MP3ACM_DM_STREAM_SIZE:
		return Mp3Acm_Size( ( Mp3AcmStreamInstance* )lParam1, ( Mp3AcmStreamSize* )lParam2 );
	case MP3ACM_DM_STREAM_CONVERT:
		return Mp3Acm_Convert( ( Mp3AcmStreamInstance* )lParam1, ( Mp3AcmStreamHeader* )lParam2 );
	case MP3ACM_DM_STREAM_RESET:
		return MMSYSERR_NOERROR;            // no conversion is asynchronous
	case MP3ACM_DM_STREAM_PREPARE:
	case MP3ACM_DM_STREAM_UNPREPARE:
		return MMSYSERR_NOTSUPPORTED;       // ACM prepares the headers itself
	}

	if( uMsg >= MP3ACM_DM_BASE )
		return MMSYSERR_NOTSUPPORTED;
	return DefDriverProc( dwId, hDriver, uMsg, lParam1, lParam2 );
}

// ----------------------------------------------------------------------- //
//  For s_dx8.cpp
// ----------------------------------------------------------------------- //

HACMDRIVERID Mp3Acm_Register( void )
{
	if( s_hMp3AcmDriverId )
		return s_hMp3AcmDriverId;

	// the module that holds the driver procedure: this DLL
	MEMORY_BASIC_INFORMATION Info;
	HINSTANCE hModule = NULL;
	if( VirtualQuery( ( void* )Mp3Acm_DriverProc, &Info, sizeof( Info )))
		hModule = ( HINSTANCE )Info.AllocationBase;

	HACMDRIVERID hId = NULL;
	MMRESULT mmResult = acmDriverAdd( &hId, hModule, ( LPARAM )Mp3Acm_DriverProc, 0,
		ACM_DRIVERADDF_FUNCTION | ACM_DRIVERADDF_LOCAL );
	if( mmResult != 0 )
	{
		LOG_WRITE( g_pLogFile, "MP3 Codec - Unable to add the minimp3 driver (%d)\n", mmResult );
		return NULL;
	}
	LOG_WRITE( g_pLogFile, "MP3 Codec - Selecting the minimp3 driver, %d.\n", hId );
	// (for a debugger, or Wine's +debugstr: the codec is in use)
	char szLine[96];
	wsprintfA( szLine, "SndDrv: MP3 codec: minimp3, local ACM driver %p\n", ( void* )hId );
	OutputDebugStringA( szLine );
	s_hMp3AcmDriverId = hId;
	return hId;
}
