// *********************************************************************** //
//
//  MODULE:   BINKW32 (.C)  -- the modern build's binkw32.dll
//
//  PURPOSE:  Plays Tron 2.0's Bink movies with FFmpeg's Bink demuxer and
//            decoders, in place of RAD Game Tools' proprietary
//            binkw32.dll (MODERN.md X-1). The game has three movies, all
//            Bink 1 ("BIKi"), 640x480, each with one audio track:
//            Movies/disnlogo.bik, lithlogo.bik and nvidia.bik, the logos
//            at start (CInterfaceMgr's movie state). Its cinematics are
//            in-engine scenes, not movies. Only the modern build ships
//            this DLL (build/ext.sh builds it, with the static FFmpeg
//            libraries from ext/ffmpeg); the legacy build keeps RAD's.
//            The design is nolf2src's and cjacksrc's player (the same
//            engine family), made for Tron 2.0's engine.
//
//  THE INTERFACE.  Lithtech.exe loads "Binkw32.dll" and gets twelve entry
//  points by their decorated __stdcall names (src/Engine/runtime/kernel/
//  src/sys/win/binkvideomgrimpl.cpp, BinkVideoMgr::Init). It refuses the
//  DLL if one of them, other than _BinkRestoreCursor@4, is missing. No
//  other retail binary imports or loads Bink. This file exports only those
//  twelve (binkw32.def). What the engine expects of them:
//
//    * BinkOpen returns a BINK. The engine reads its first four fields
//      directly: Width, Height, Frames, FrameNum (1 = the first frame, as
//      in RAD's bink.h). The remainder of the structure is ours.
//    * The engine's frame loop (BinkVideoInst::UpdateOnScreen) is
//      Wait (until 0) -> CopyToBuffer -> DoFrame -> NextFrame. The copy
//      goes into a locked Direct3D 9 texture of 32 bits a pixel. Tron's
//      engine does not ask BinkDX8SurfaceType for the format: it always
//      passes BINKSURFACE32 (Lithtech.exe 0x5ac214, the first entry of its
//      format table). BinkDX8SurfaceType must be exported all the same.
//    * The movie is done when FrameNum is Frames - 1 (IsAtLastFrame), so
//      NextFrame never goes past that frame.
//    * BinkVideoInst::Term calls BinkWait until it returns 0, then
//      BinkClose: BinkWait must return 0 soon.
//
//  SOUND.  BinkVideoMgr::Init gives BinkOpenDirectSound (and no
//  DirectSound object) to BinkSetSoundSystem. This player does not use
//  DirectSound. When a movie opens, it decodes the complete first audio
//  track (about 3 MB for the longest movie, lithlogo.bik, 18 s) to 16-bit
//  PCM and plays it with one waveOut buffer. The buffer starts with the
//  first frame. The video frames are timed from that moment with the
//  performance counter. When the game falls behind, the video skips ahead
//  to the frame that is due, so the picture stays with the sound.
//
//  THE CURSOR.  The cursor entry points are for a software mouse cursor
//  drawn over the movie. Their results do not change what the engine
//  shows, so here they do nothing.
//
//  LICENCE.  This file is written for this project. The DLL links FFmpeg
//  statically, configured as LGPL-2.1-or-later (build/ext.sh enables no
//  GPL or non-free part). docs/LICENSE_NOTES.md has the details.
//
// *********************************************************************** //

#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <string.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

// RAD's surface types (bink.h)
#define BINKSURFACE24       1       // B, G, R in memory
#define BINKSURFACE32       3       // B, G, R, x in memory (0x00RRGGBB)
#define BINKSURFACE32R      4       // R, G, B, x in memory
#define BINKSURFACE32A      5       // B, G, R, A in memory (0xAARRGGBB)
#define BINKSURFACE4444     7
#define BINKSURFACE5551     8
#define BINKSURFACE555      9
#define BINKSURFACE565      10
#define BINKSURFACEMASK     15

typedef struct Player Player;

// The engine reads the first four fields. The first seven have RAD's
// layout.
typedef struct BINK
{
	unsigned int Width;                     // 0x00
	unsigned int Height;                    // 0x04
	unsigned int Frames;                    // 0x08
	unsigned int FrameNum;                  // 0x0c, 1 = the first frame
	unsigned int LastFrameNum;              // 0x10
	unsigned int FrameRate;                 // 0x14  frames a second:
	unsigned int FrameRateDiv;              // 0x18  FrameRate / FrameRateDiv
	Player*      pPlayer;
} BINK;

struct Player
{
	AVFormatContext*   pFormat;
	AVCodecContext*    pVideo;
	int                nVideoStream;
	AVPacket*          pPacket;
	AVFrame*           pFrame;
	unsigned int       nDecoded;            // frames decoded (pFrame is the last)
	int                bEOF;

	struct SwsContext* pScale;

	LARGE_INTEGER      nFreq;
	LARGE_INTEGER      nStart;              // when the first frame was shown
	int                bStarted;

	HWAVEOUT           hWave;
	WAVEHDR            Header;
	short*             pPCM;
};


// ----------------------------------------------------------------------- //
//  Sound: one audio track, decoded completely to 16-bit PCM
// ----------------------------------------------------------------------- //

static short FloatToS16(float f)
{
	if (f >= 1.0f)
		return 32767;
	if (f <= -1.0f)
		return -32767;
	return (short)(f * 32767.0f);
}

typedef struct PCMBuffer
{
	short* pData;
	size_t nSamples;                        // shorts in pData
	size_t nAlloc;
} PCMBuffer;

// Adds one decoded frame to the buffer, interleaved. 0 on an error.
static int AddFrame(PCMBuffer* pBuf, const AVFrame* pFrame)
{
	int    nChannels = pFrame->ch_layout.nb_channels;
	enum AVSampleFormat eFormat = (enum AVSampleFormat)pFrame->format;
	int    bPlanar = av_sample_fmt_is_planar(eFormat);
	enum AVSampleFormat ePacked = av_get_packed_sample_fmt(eFormat);
	size_t nNeed = pBuf->nSamples + (size_t)pFrame->nb_samples * (size_t)nChannels;
	int    i, c;

	if (ePacked != AV_SAMPLE_FMT_FLT && ePacked != AV_SAMPLE_FMT_S16)
		return 0;

	if (nNeed > pBuf->nAlloc)
	{
		size_t nNew = pBuf->nAlloc ? pBuf->nAlloc : (size_t)1 << 20;
		short* pNew;

		while (nNew < nNeed)
			nNew *= 2;
		pNew = (short*)realloc(pBuf->pData, nNew * sizeof(short));
		if (!pNew)
			return 0;
		pBuf->pData = pNew;
		pBuf->nAlloc = nNew;
	}

	for (i = 0; i < pFrame->nb_samples; ++i)
	{
		for (c = 0; c < nChannels; ++c)
		{
			int nPlane = bPlanar ? c : 0;
			int nIndex = bPlanar ? i : i * nChannels + c;

			pBuf->pData[pBuf->nSamples++] = (ePacked == AV_SAMPLE_FMT_FLT)
				? FloatToS16(((const float*)pFrame->extended_data[nPlane])[nIndex])
				: ((const short*)pFrame->extended_data[nPlane])[nIndex];
		}
	}
	return 1;
}

// Decodes the first audio track of the file and opens waveOut for it. The
// buffer is prepared here and written at the first frame (StartSound): a
// buffer written now would start to play before the first frame is shown.
// If something fails, the movie plays without sound.
static void OpenSound(Player* pPlayer, const char* pFilename)
{
	AVFormatContext* pFormat = NULL;
	AVCodecContext*  pAudio = NULL;
	AVPacket*        pPacket = NULL;
	AVFrame*         pFrame = NULL;
	PCMBuffer        Buf = { NULL, 0, 0 };
	int              nStream, nRate = 0, nChannels = 0, bEOF = 0;
	WAVEFORMATEX     wfx;

	if (avformat_open_input(&pFormat, pFilename, NULL, NULL) < 0)
		return;
	nStream = av_find_best_stream(pFormat, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (nStream < 0)
		goto done;

	{
		const AVCodecParameters* pPar = pFormat->streams[nStream]->codecpar;
		const AVCodec*           pCodec = avcodec_find_decoder(pPar->codec_id);

		if (!pCodec || !(pAudio = avcodec_alloc_context3(pCodec)) ||
		    avcodec_parameters_to_context(pAudio, pPar) < 0 ||
		    avcodec_open2(pAudio, pCodec, NULL) < 0)
			goto done;
	}

	pPacket = av_packet_alloc();
	pFrame = av_frame_alloc();
	if (!pPacket || !pFrame)
		goto done;

	while (!bEOF)
	{
		bEOF = av_read_frame(pFormat, pPacket) < 0;
		if (!bEOF && pPacket->stream_index != nStream)
		{
			av_packet_unref(pPacket);
			continue;
		}
		avcodec_send_packet(pAudio, bEOF ? NULL : pPacket);
		av_packet_unref(pPacket);
		while (avcodec_receive_frame(pAudio, pFrame) == 0)
		{
			nRate = pFrame->sample_rate;
			nChannels = pFrame->ch_layout.nb_channels;
			if (!AddFrame(&Buf, pFrame))
				goto done;
		}
	}

	if (!Buf.nSamples || nRate <= 0 || nChannels < 1 || nChannels > 2)
		goto done;

	memset(&wfx, 0, sizeof(wfx));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = (WORD)nChannels;
	wfx.nSamplesPerSec = (DWORD)nRate;
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = (WORD)(nChannels * 2);
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	if (waveOutOpen(&pPlayer->hWave, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
	{
		pPlayer->hWave = NULL;
		goto done;
	}
	memset(&pPlayer->Header, 0, sizeof(pPlayer->Header));
	pPlayer->Header.lpData = (LPSTR)Buf.pData;
	pPlayer->Header.dwBufferLength = (DWORD)(Buf.nSamples * sizeof(short));
	if (waveOutPrepareHeader(pPlayer->hWave, &pPlayer->Header, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		waveOutClose(pPlayer->hWave);
		pPlayer->hWave = NULL;
		goto done;
	}
	pPlayer->pPCM = Buf.pData;
	Buf.pData = NULL;

done:
	free(Buf.pData);
	av_frame_free(&pFrame);
	av_packet_free(&pPacket);
	avcodec_free_context(&pAudio);
	avformat_close_input(&pFormat);
}

static void CloseSound(Player* pPlayer)
{
	if (pPlayer->hWave)
	{
		waveOutReset(pPlayer->hWave);
		waveOutUnprepareHeader(pPlayer->hWave, &pPlayer->Header, sizeof(WAVEHDR));
		waveOutClose(pPlayer->hWave);
		pPlayer->hWave = NULL;
	}
	free(pPlayer->pPCM);
	pPlayer->pPCM = NULL;
}

static void StartSound(Player* pPlayer)
{
	if (pPlayer->hWave &&
	    waveOutWrite(pPlayer->hWave, &pPlayer->Header, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		CloseSound(pPlayer);
}


// ----------------------------------------------------------------------- //
//  Video
// ----------------------------------------------------------------------- //

// Seconds since the first frame was shown.
static double Elapsed(const Player* pPlayer)
{
	LARGE_INTEGER nNow;

	QueryPerformanceCounter(&nNow);
	return (double)(nNow.QuadPart - pPlayer->nStart.QuadPart) / (double)pPlayer->nFreq.QuadPart;
}

// Decodes until frame FrameNum is in pFrame. At the end of the file, the
// last decoded frame stays.
static void Decode(BINK* pBink)
{
	Player* pPlayer = pBink->pPlayer;

	while (pPlayer->nDecoded < pBink->FrameNum)
	{
		int nResult = avcodec_receive_frame(pPlayer->pVideo, pPlayer->pFrame);

		if (nResult == 0)
		{
			++pPlayer->nDecoded;
			continue;
		}
		if (nResult != AVERROR(EAGAIN) || pPlayer->bEOF)
			return;                         // no more frames

		if (av_read_frame(pPlayer->pFormat, pPlayer->pPacket) < 0)
		{
			pPlayer->bEOF = 1;
			avcodec_send_packet(pPlayer->pVideo, NULL);
			continue;
		}
		if (pPlayer->pPacket->stream_index == pPlayer->nVideoStream)
			avcodec_send_packet(pPlayer->pVideo, pPlayer->pPacket);
		av_packet_unref(pPlayer->pPacket);
	}
}

static void FreePlayer(Player* pPlayer)
{
	CloseSound(pPlayer);
	sws_freeContext(pPlayer->pScale);
	av_frame_free(&pPlayer->pFrame);
	av_packet_free(&pPlayer->pPacket);
	avcodec_free_context(&pPlayer->pVideo);
	avformat_close_input(&pPlayer->pFormat);
	free(pPlayer);
}


// ----------------------------------------------------------------------- //
//  The exports (binkw32.def gives them RAD's decorated names)
// ----------------------------------------------------------------------- //

// nFlags: the engine can ask for BINKCOPY2XHI, BINKFILEHANDLE and
// BINKFROMMEMORY (its PLAYBACK_* flags). Tron 2.0 opens its movies by file
// name with none of them (CInterfaceMgr::NextMovie), so the flags are not
// used.
BINK* __stdcall BinkOpen(const char* pFilename, unsigned int nFlags)
{
	BINK*          pBink;
	Player*        pPlayer;
	AVStream*      pStream;
	const AVCodec* pCodec;
	AVRational     Rate;

	(void)nFlags;
	av_log_set_level(AV_LOG_QUIET);

	pBink = (BINK*)calloc(1, sizeof(BINK));
	pPlayer = (Player*)calloc(1, sizeof(Player));
	if (!pBink || !pPlayer)
		goto fail;
	pBink->pPlayer = pPlayer;

	if (avformat_open_input(&pPlayer->pFormat, pFilename, NULL, NULL) < 0)
		goto fail;
	pPlayer->nVideoStream = av_find_best_stream(pPlayer->pFormat, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (pPlayer->nVideoStream < 0)
		goto fail;
	pStream = pPlayer->pFormat->streams[pPlayer->nVideoStream];

	pCodec = avcodec_find_decoder(pStream->codecpar->codec_id);
	if (!pCodec || !(pPlayer->pVideo = avcodec_alloc_context3(pCodec)) ||
	    avcodec_parameters_to_context(pPlayer->pVideo, pStream->codecpar) < 0 ||
	    avcodec_open2(pPlayer->pVideo, pCodec, NULL) < 0)
		goto fail;

	pPlayer->pPacket = av_packet_alloc();
	pPlayer->pFrame = av_frame_alloc();
	if (!pPlayer->pPacket || !pPlayer->pFrame)
		goto fail;

	// FFmpeg's Bink demuxer gives the video stream a time base of one
	// frame: the frame rate is its inverse (disnlogo.bik: 2997/100).
	Rate = av_inv_q(pStream->time_base);
	if (Rate.num <= 0 || Rate.den <= 0)
		Rate = av_make_q(15, 1);

	pBink->Width = (unsigned int)pStream->codecpar->width;
	pBink->Height = (unsigned int)pStream->codecpar->height;
	pBink->Frames = (unsigned int)(pStream->nb_frames > 0 ? pStream->nb_frames : pStream->duration);
	if ((int)pBink->Frames < 2)
		pBink->Frames = 2;                  // so that frame 1 is not the last
	pBink->FrameNum = 1;
	pBink->FrameRate = (unsigned int)Rate.num;
	pBink->FrameRateDiv = (unsigned int)Rate.den;

	QueryPerformanceFrequency(&pPlayer->nFreq);
	OpenSound(pPlayer, pFilename);
	Decode(pBink);
	return pBink;

fail:
	if (pPlayer)
		FreePlayer(pPlayer);
	free(pBink);
	return NULL;
}

void __stdcall BinkClose(BINK* pBink)
{
	if (!pBink)
		return;
	FreePlayer(pBink->pPlayer);
	free(pBink);
}

// Nonzero while it is too early to show frame FrameNum. The first call
// starts the clock and the sound.
int __stdcall BinkWait(BINK* pBink)
{
	Player* pPlayer;
	double  fDue;

	if (!pBink)
		return 0;
	pPlayer = pBink->pPlayer;

	if (!pPlayer->bStarted)
	{
		pPlayer->bStarted = 1;
		QueryPerformanceCounter(&pPlayer->nStart);
		StartSound(pPlayer);
		return 0;
	}

	fDue = (double)(pBink->FrameNum - 1) * pBink->FrameRateDiv / pBink->FrameRate;
	return Elapsed(pPlayer) < fDue;
}

int __stdcall BinkDoFrame(BINK* pBink)
{
	if (pBink)
		Decode(pBink);
	return 0;
}

// The next frame, or the frame that is due now if the game is late (a
// stall, a slow machine). The skipped frames are decoded all the same
// (a Bink frame depends on the one before it) but not shown. Never past
// Frames - 1, where the engine stops.
void __stdcall BinkNextFrame(BINK* pBink)
{
	Player*      pPlayer;
	unsigned int nDue;

	if (!pBink)
		return;
	pPlayer = pBink->pPlayer;
	pBink->LastFrameNum = pBink->FrameNum;
	if (pBink->FrameNum < pBink->Frames - 1)
		++pBink->FrameNum;

	if (pPlayer->bStarted)
	{
		nDue = (unsigned int)(Elapsed(pPlayer) * pBink->FrameRate / pBink->FrameRateDiv) + 1;
		if (nDue > pBink->Frames - 1)
			nDue = pBink->Frames - 1;
		if (nDue > pBink->FrameNum)
			pBink->FrameNum = nDue;
	}
}

// Converts frame FrameNum into the caller's surface at (nDestX, nDestY).
// nDestHeight is the height of the surface (the engine gives the height of
// the movie); rows below it are not written.
int __stdcall BinkCopyToBuffer(BINK* pBink, void* pDest, int nPitch, unsigned int nDestHeight,
                               unsigned int nDestX, unsigned int nDestY, unsigned int nFlags)
{
	Player*            pPlayer;
	AVFrame*           pFrame;
	enum AVPixelFormat eFormat;
	int                nBytes, nRows;
	uint8_t*           pDst[4] = { NULL, NULL, NULL, NULL };
	int                nDstPitch[4] = { 0, 0, 0, 0 };

	if (!pBink || !pDest)
		return 0;
	pPlayer = pBink->pPlayer;
	Decode(pBink);
	pFrame = pPlayer->pFrame;
	if (!pPlayer->nDecoded || !pFrame->data[0])
		return 0;

	switch (nFlags & BINKSURFACEMASK)
	{
	// The 32-bit types get an opaque alpha byte: the engine's texture can
	// be A8R8G8B8 (GetTextureDDFormat1(BPP_32)), and an alpha of 0 is not
	// what RAD's DLL writes.
	case BINKSURFACE32:
	case BINKSURFACE32A:  eFormat = AV_PIX_FMT_BGRA;     nBytes = 4; break;
	case BINKSURFACE32R:  eFormat = AV_PIX_FMT_RGBA;     nBytes = 4; break;
	case BINKSURFACE24:   eFormat = AV_PIX_FMT_BGR24;    nBytes = 3; break;
	case BINKSURFACE4444: eFormat = AV_PIX_FMT_RGB444LE; nBytes = 2; break;
	case BINKSURFACE5551:
	case BINKSURFACE555:  eFormat = AV_PIX_FMT_RGB555LE; nBytes = 2; break;
	case BINKSURFACE565:  eFormat = AV_PIX_FMT_RGB565LE; nBytes = 2; break;
	default:              return 0;
	}

	nRows = pFrame->height;
	if (nDestHeight && nDestHeight > nDestY && (int)(nDestHeight - nDestY) < nRows)
		nRows = (int)(nDestHeight - nDestY) & ~1;
	else if (nDestHeight && nDestHeight <= nDestY)
		return 0;
	if (nRows <= 0)
		return 0;

	pPlayer->pScale = sws_getCachedContext(pPlayer->pScale,
		pFrame->width, pFrame->height, (enum AVPixelFormat)pFrame->format,
		pFrame->width, pFrame->height, eFormat, SWS_POINT, NULL, NULL, NULL);
	if (!pPlayer->pScale)
		return 0;

	pDst[0] = (uint8_t*)pDest + (size_t)nDestY * nPitch + (size_t)nDestX * nBytes;
	nDstPitch[0] = nPitch;
	sws_scale(pPlayer->pScale, (const uint8_t* const*)pFrame->data, pFrame->linesize,
	          0, nRows, pDst, nDstPitch);
	return 0;
}

// The engine gives BinkOpenDirectSound and no DirectSound object. The sound
// goes to waveOut (see the header).
int __stdcall BinkSetSoundSystem(void* pOpen, void* pParam)
{
	(void)pOpen;
	(void)pParam;
	return 1;
}

int __stdcall BinkOpenDirectSound(void* pParam)
{
	(void)pParam;
	return 1;
}

// RAD's: the surface type for an IDirect3DSurface8, from its format. Tron's
// engine gets it but does not call it (see the header). It reads the
// format through the interface's vtable (GetDesc is method 8 of
// IDirect3DSurface8), so that this file needs no Direct3D 8 headers.
// -1 for a format that no copy writes.
typedef struct D3D8SurfaceDesc
{
	unsigned int Format, Type, Usage, Pool, Size, MultiSampleType, Width, Height;
} D3D8SurfaceDesc;
typedef long (__stdcall *D3D8GetDescFn)(void* pThis, D3D8SurfaceDesc* pDesc);

int __stdcall BinkDX8SurfaceType(void* pSurface)
{
	D3D8SurfaceDesc Desc;

	if (!pSurface || ((D3D8GetDescFn)(*(void***)pSurface)[8])(pSurface, &Desc) < 0)
		return -1;
	switch (Desc.Format)
	{
	case 20: return BINKSURFACE24;      // D3DFMT_R8G8B8
	case 21: return BINKSURFACE32A;     // D3DFMT_A8R8G8B8
	case 22: return BINKSURFACE32;      // D3DFMT_X8R8G8B8
	case 23: return BINKSURFACE565;     // D3DFMT_R5G6B5
	case 24: return BINKSURFACE555;     // D3DFMT_X1R5G5B5
	case 25: return BINKSURFACE5551;    // D3DFMT_A1R5G5B5
	case 26: return BINKSURFACE4444;    // D3DFMT_A4R4G4B4
	default: return -1;
	}
}

int __stdcall BinkIsSoftwareCursor(void* pSurface, HCURSOR hCursor)
{
	(void)pSurface;
	(void)hCursor;
	return 0;
}

int __stdcall BinkCheckCursor(HWND hWnd, int x, int y, int w, int h)
{
	(void)hWnd; (void)x; (void)y; (void)w; (void)h;
	return 0;
}

void __stdcall BinkRestoreCursor(int bForce)
{
	(void)bForce;
}
