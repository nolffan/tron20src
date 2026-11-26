/*
 * musicplayer.cpp -- the modern build's own music player (MODERN.md X-3);
 * see musicplayer.h. #included once, at the end of ltdirectmusic_impl.cpp,
 * in a TRON20SRC_MODERN block.
 *
 * What it reads (all that Tron 2.0's 28 music control files and 963
 * segments use; MODERN.md X-3 has the survey):
 *
 *   * the control file (CControlFileMgrDStream, as CLTDirectMusicMgr):
 *     NUMINTENSITIES, INITIALINTENSITY, INITIALVOLUME, VOLUMEOFFSET,
 *     INTENSITY, TRANSITION, SECONDARYSEGMENT. The synthesizer and reverb
 *     keys (PCHANNELS, VOICES, SYNTHSAMPLERATE, REVERB...) have no use
 *     here (every file has REVERB OFF). MOTIF, BAND, DLSBANK and the
 *     styles need DirectMusic's styles and instruments: no file has them;
 *     the player writes a warning if one does.
 *   * the segments (RIFF 'DMSG', DirectX 8 and 9 file format, dmusicf.h):
 *     the segment header (length, repeats, loop points, default
 *     resolution), the tempo track ('tetr'), the time signature track
 *     ('TIMS'), the wave track ('wavt': parts, each on a PChannel, with
 *     its wave items: start, duration, volume, the wave by its file name
 *     in a 'DMRF' reference) and, in the sequence track ('seqt'), the
 *     volume (CC 7) and expression (CC 11) curves. The notes of a
 *     sequence track need instruments (DLS) and are not played (two
 *     segments, CORRUPT/WARNING1, have five notes; no band or DLS gives
 *     them an instrument).
 *   * the waves: RIFF 'WAVE', MP3 (format 0x55, minimp3) or PCM (8 or 16
 *     bits). Every music wave of the game is MP3, stereo, 44100 Hz or
 *     22050 Hz (the "low" quality set); the player converts other rates
 *     to MP_RATE (linear interpolation).
 *
 * How it plays (what DirectMusic does, as far as Tron 2.0's music needs):
 *
 *   * The timeline is a count of samples at MP_RATE. A primary segment
 *     starts at a sample; each wave item starts at its time in the
 *     segment, which the segment's tempo map gives (768 ticks a quarter
 *     note; a segment without a tempo track keeps the tempo before it).
 *     A wave plays for its duration, also past the end of its segment,
 *     and when a new segment starts: the engine plays segments with
 *     DMUS_SEGF_NOINVALIDATE, and most of Tron 2.0's waves are longer than
 *     their segments.
 *   * The intensities, the transitions and the command queue are those of
 *     CLTDirectMusicMgr (ChangeIntensity, HandleSegmentNotification_
 *     SegAlmostEnd): an intensity's segments play one after the other,
 *     then loop, then the intensity changes to its next one; a change
 *     plays the transition segment first, if the control file gives one.
 *     The queue runs when the primary segment ends (retail runs it at
 *     DMUS_NOTIFICATION_SEGALMOSTEND; the next segment starts at the end
 *     of the segment in both).
 *   * The enact times: immediately, the next beat, grid or measure (from
 *     the primary segment's time signature track, at or after the next
 *     sample to mix), the end of the segment; "Default" is the resolution
 *     of the segment that starts (its header). No segment has markers, so
 *     "Marker" is immediately (as DirectMusic does without markers).
 *   * Each PChannel has a volume and an expression (CC 7, 100 at start;
 *     CC 11, 127 at start); a wave's gain is that of its item, part and
 *     track, and (CC 7 / 127)^2 (CC 11 / 127)^2 (the DLS curve). Curves
 *     go from their start value to their end value: instant, linear,
 *     sine; exponential and logarithmic are approximated.
 *   * The master volume (SetVolume, in 1/100 dB, with the control file's
 *     VOLUMEOFFSET) and Pause/UnPause (the mix stops where it is and goes
 *     on from there; retail stops DirectMusic and starts the segment again
 *     from the same time) take effect in about 10 ms (a ramp).
 *   * Stop stops every segment and fades the waves out in 10 ms.
 *   * Not reproduced: the synthesizer's notes (above), DirectMusic's
 *     random variations (Tron's items and parts are in all 32 variations,
 *     so all play), clock-time tracks (none), and the reverb (off).
 *
 * The output: a DirectSound object of its own (SndDrv.dll has the game's
 * sounds in another one), a looping secondary buffer of one second, 44100
 * Hz, 16 bits, stereo, and a thread that keeps about 150 ms of mix ahead
 * of the play cursor.
 *
 * Modern build only (TRON20SRC_MODERN).
 */
#include "client_filemgr.h"
#include <math.h>
#include <dsound.h>
#include <dmusicf.h>

#define MINIMP3_IMPLEMENTATION
#define MINIMP3_NO_SIMD
#include "../mp3/minimp3/minimp3.h"

static IClientFileMgr *s_pMpFileMgr;
define_holder(IClientFileMgr, s_pMpFileMgr);

// ----------------------------------------------------------------------- //
//  RIFF
// ----------------------------------------------------------------------- //

#define MP_FCC(a,b,c,d)	((uint32)(uint8)(a) | ((uint32)(uint8)(b) << 8) | ((uint32)(uint8)(c) << 16) | ((uint32)(uint8)(d) << 24))

static uint32 Mp_Rd32(const uint8* p) { return (uint32)p[0] | ((uint32)p[1] << 8) | ((uint32)p[2] << 16) | ((uint32)p[3] << 24); }
static uint16 Mp_Rd16(const uint8* p) { return (uint16)(p[0] | (p[1] << 8)); }
static double Mp_RdDouble(const uint8* p) { double d; memcpy(&d, p, 8); return d; }

struct CMpChunk
{
	uint32 nId;		// the chunk's id
	uint32 nType;	// the form type of a RIFF or LIST, else 0
	const uint8* pData;
	uint32 nLen;
};

// the next chunk in [p, pEnd); false at the end
static bool Mp_NextChunk(const uint8*& p, const uint8* pEnd, CMpChunk& c)
{
	if (pEnd - p < 8)
		return false;
	c.nId = Mp_Rd32(p);
	uint32 nLen = Mp_Rd32(p + 4);
	const uint8* pData = p + 8;
	if (nLen > (uint32)(pEnd - pData))
		nLen = (uint32)(pEnd - pData);
	c.nType = 0;
	c.pData = pData;
	c.nLen = nLen;
	if ((c.nId == MP_FCC('R','I','F','F') || c.nId == MP_FCC('L','I','S','T')) && nLen >= 4)
	{
		c.nType = Mp_Rd32(pData);
		c.pData = pData + 4;
		c.nLen = nLen - 4;
	}
	p = pData + nLen + (nLen & 1);
	if (p > pEnd)
		p = pEnd;
	return true;
}

// ----------------------------------------------------------------------- //
//  Waves
// ----------------------------------------------------------------------- //

class CMpWave
{
public:
	CMpWave() : m_sName(LTNULL), m_pFile(LTNULL), m_pData(LTNULL), m_nDataLen(0), m_nFormat(0),
		m_nChannels(0), m_nRate(0), m_nBits(0), m_pNext(LTNULL) {}
	~CMpWave() { delete [] m_sName; delete [] m_pFile; }

	char* m_sName;
	uint8* m_pFile;
	const uint8* m_pData;
	uint32 m_nDataLen;
	uint16 m_nFormat;		// 1: PCM, 0x55: MP3
	uint16 m_nChannels;
	uint32 m_nRate;
	uint16 m_nBits;
	CMpWave* m_pNext;
};

// ----------------------------------------------------------------------- //
//  Segments
// ----------------------------------------------------------------------- //

struct CMpWaveItem
{
	int32 mtStart;
	int32 mtDuration;
	int32 nVolume;		// item + part + track, 1/100 dB
	uint32 nPChannel;
	CMpWave* pWave;
};

struct CMpCurveItem
{
	int32 mtStart;
	int32 mtDuration;
	uint32 nPChannel;
	int nCC;			// 0: volume (CC 7), 1: expression (CC 11)
	int16 nStart;
	int16 nEnd;
	uint8 nShape;
};

struct CMpTempo
{
	int32 mtTime;
	double dTempo;		// quarter notes a minute
};

struct CMpTimeSig
{
	int32 mtTime;
	int32 mtBeat;		// ticks of a beat
	int32 nBeatsPerMeasure;
	int32 nGridsPerBeat;
};

class CMpSegment
{
public:
	CMpSegment() : m_sName(LTNULL), m_mtLength(0), m_nRepeats(0), m_mtPlayStart(0), m_mtLoopStart(0),
		m_mtLoopEnd(0), m_nResolution(0), m_nDefaultEnact(LTDMEnactDefault), m_aItems(LTNULL),
		m_nItems(0), m_aCurves(LTNULL), m_nCurves(0), m_aTempo(LTNULL), m_nTempo(0), m_aTimeSig(LTNULL),
		m_nTimeSig(0), m_pNext(LTNULL) {}
	~CMpSegment()
	{
		delete [] m_sName; delete [] m_aItems; delete [] m_aCurves; delete [] m_aTempo; delete [] m_aTimeSig;
	}

	// seconds from tick a to tick b (a <= b); dTempo: the tempo before
	// the first tempo change
	double Seconds(int32 a, int32 b, double dTempo) const
	{
		double dSec = 0.0;
		int i = 0;
		while (i < m_nTempo && m_aTempo[i].mtTime <= a)
			dTempo = m_aTempo[i++].dTempo;
		while (a < b)
		{
			int32 c = b;
			if (i < m_nTempo && m_aTempo[i].mtTime < c)
				c = m_aTempo[i].mtTime;
			dSec += (double)(c - a) * 60.0 / (dTempo * DMUS_PPQ);
			a = c;
			if (i < m_nTempo && m_aTempo[i].mtTime <= a)
				dTempo = m_aTempo[i++].dTempo;
		}
		return dSec;
	}

	// the tick dSec seconds after tick a
	int32 TickAfter(int32 a, double dSec, double dTempo) const
	{
		int i = 0;
		while (i < m_nTempo && m_aTempo[i].mtTime <= a)
			dTempo = m_aTempo[i++].dTempo;
		for (;;)
		{
			double dTicksPerSec = dTempo * DMUS_PPQ / 60.0;
			if (i >= m_nTempo)
				return a + (int32)(dSec * dTicksPerSec);
			double dSpan = (double)(m_aTempo[i].mtTime - a) / dTicksPerSec;
			if (dSec < dSpan)
				return a + (int32)(dSec * dTicksPerSec);
			dSec -= dSpan;
			a = m_aTempo[i].mtTime;
			dTempo = m_aTempo[i++].dTempo;
		}
	}

	// the tempo at the end of the segment
	double EndTempo(double dTempo) const { return m_nTempo ? m_aTempo[m_nTempo - 1].dTempo : dTempo; }

	// the time signature at tick mt (4/4 without a track)
	void TimeSigAt(int32 mt, CMpTimeSig& sig, int32& mtNext) const
	{
		sig.mtTime = 0; sig.mtBeat = DMUS_PPQ; sig.nBeatsPerMeasure = 4; sig.nGridsPerBeat = 4;
		mtNext = 0x7fffffff;
		for (int i = 0; i < m_nTimeSig; i++)
		{
			if (m_aTimeSig[i].mtTime <= mt)
				sig = m_aTimeSig[i];
			else
			{
				mtNext = m_aTimeSig[i].mtTime;
				break;
			}
		}
	}

	bool Parse(const uint8* p, uint32 nLen, CModernMusicPlayer* pPlayer, CMpWave* (*pfnLoadWave)(CModernMusicPlayer*, const char*));

	char* m_sName;
	int32 m_mtLength;
	uint32 m_nRepeats;
	int32 m_mtPlayStart;
	int32 m_mtLoopStart;
	int32 m_mtLoopEnd;
	uint32 m_nResolution;
	LTDMEnactTypes m_nDefaultEnact;		// SECONDARYSEGMENT's enact time
	CMpWaveItem* m_aItems;
	int m_nItems;
	CMpCurveItem* m_aCurves;
	int m_nCurves;
	CMpTempo* m_aTempo;
	int m_nTempo;
	CMpTimeSig* m_aTimeSig;
	int m_nTimeSig;
	CMpSegment* m_pNext;
};

// a growing array
template <class T> static void Mp_Append(T*& a, int& n, const T& v)
{
	// grow by powers of two
	if ((n & (n - 1)) == 0)
	{
		int nNew = n ? n * 2 : 1;
		T* aNew = new T[nNew];
		if (n)
			memcpy(aNew, a, n * sizeof(T));
		delete [] a;
		a = aNew;
	}
	a[n++] = v;
}

bool CMpSegment::Parse(const uint8* pFile, uint32 nFileLen, CModernMusicPlayer* pPlayer,
	CMpWave* (*pfnLoadWave)(CModernMusicPlayer*, const char*))
{
	const uint8* p = pFile;
	const uint8* pEnd = pFile + nFileLen;
	CMpChunk form;
	if (!Mp_NextChunk(p, pEnd, form) || form.nId != MP_FCC('R','I','F','F') || form.nType != MP_FCC('D','M','S','G'))
		return false;

	bool bHeader = false;
	p = form.pData;
	pEnd = form.pData + form.nLen;
	CMpChunk c;
	while (Mp_NextChunk(p, pEnd, c))
	{
		if (c.nId == MP_FCC('s','e','g','h') && c.nLen >= 24)
		{
			// DMUS_IO_SEGMENT_HEADER
			m_nRepeats = Mp_Rd32(c.pData);
			m_mtLength = (int32)Mp_Rd32(c.pData + 4);
			m_mtPlayStart = (int32)Mp_Rd32(c.pData + 8);
			m_mtLoopStart = (int32)Mp_Rd32(c.pData + 12);
			m_mtLoopEnd = (int32)Mp_Rd32(c.pData + 16);
			m_nResolution = Mp_Rd32(c.pData + 20);
			bHeader = true;
			if (c.nLen >= 40 && (Mp_Rd32(c.pData + 32) & (DMUS_SEGIOF_REFLENGTH | DMUS_SEGIOF_CLOCKTIME)))
				LTDMConOutWarning("WARNING! Modern music player: segment %s has a clock-time length; the music-time length is used\n", m_sName);
		}
		else if (c.nId == MP_FCC('L','I','S','T') && c.nType == MP_FCC('t','r','k','l'))
		{
			const uint8* pt = c.pData;
			const uint8* ptEnd = c.pData + c.nLen;
			CMpChunk trk;
			while (Mp_NextChunk(pt, ptEnd, trk))
			{
				if (trk.nId != MP_FCC('R','I','F','F') || trk.nType != MP_FCC('D','M','T','K'))
					continue;

				// one track: its header, its extras, then its data
				uint32 nConfig = DMUS_TRACKCONFIG_DEFAULT;
				const uint8* pk = trk.pData;
				const uint8* pkEnd = trk.pData + trk.nLen;
				CMpChunk k;
				while (Mp_NextChunk(pk, pkEnd, k))
				{
					if (k.nId == MP_FCC('t','r','k','x') && k.nLen >= 4)
						nConfig = Mp_Rd32(k.pData);
					else if (k.nId == MP_FCC('t','e','t','r') && k.nLen >= 4)
					{
						// the tempo track: DMUS_IO_TEMPO_ITEM
						uint32 nSize = Mp_Rd32(k.pData);
						if (nSize < 12)
							continue;
						for (const uint8* q = k.pData + 4; q + nSize <= k.pData + k.nLen; q += nSize)
						{
							CMpTempo t;
							t.mtTime = (int32)Mp_Rd32(q);
							t.dTempo = Mp_RdDouble(q + nSize - 8);
							if (t.dTempo < 1.0 || t.dTempo > 1000.0)
								continue;
							Mp_Append(m_aTempo, m_nTempo, t);
						}
					}
					else if (k.nId == MP_FCC('L','I','S','T') && k.nType == MP_FCC('T','I','M','S') && m_nTimeSig == 0)
					{
						// the time signature track (the first one: DirectMusic
						// asks the first track that has the parameter)
						const uint8* pq = k.pData;
						const uint8* pqEnd = k.pData + k.nLen;
						CMpChunk ts;
						while (Mp_NextChunk(pq, pqEnd, ts))
						{
							if (ts.nId != MP_FCC('t','i','m','s') || ts.nLen < 4)
								continue;
							uint32 nSize = Mp_Rd32(ts.pData);
							if (nSize < 8)
								continue;
							for (const uint8* q = ts.pData + 4; q + nSize <= ts.pData + ts.nLen; q += nSize)
							{
								CMpTimeSig s;
								int nBeat = q[5] ? q[5] : 256;
								s.mtTime = (int32)Mp_Rd32(q);
								s.mtBeat = DMUS_PPQ * 4 / nBeat;
								s.nBeatsPerMeasure = q[4] ? q[4] : 4;
								s.nGridsPerBeat = Mp_Rd16(q + 6) ? Mp_Rd16(q + 6) : 1;
								if (s.mtBeat < 1)
									s.mtBeat = 1;
								Mp_Append(m_aTimeSig, m_nTimeSig, s);
							}
						}
					}
					else if (k.nId == MP_FCC('L','I','S','T') && k.nType == MP_FCC('w','a','v','t'))
					{
						// the wave track
						if (!(nConfig & DMUS_TRACKCONFIG_PLAY_ENABLED))
							continue;
						if (nConfig & DMUS_TRACKCONFIG_PLAY_CLOCKTIME)
						{
							LTDMConOutWarning("WARNING! Modern music player: segment %s has a clock-time wave track; not played\n", m_sName);
							continue;
						}
						int32 nTrackVolume = 0;
						const uint8* pw = k.pData;
						const uint8* pwEnd = k.pData + k.nLen;
						CMpChunk w;
						while (Mp_NextChunk(pw, pwEnd, w))
						{
							if (w.nId == MP_FCC('w','a','t','h') && w.nLen >= 4)
								nTrackVolume = (int32)Mp_Rd32(w.pData);
							if (w.nId != MP_FCC('L','I','S','T') || w.nType != MP_FCC('w','a','v','p'))
								continue;

							// a part: DMUS_IO_WAVE_PART_HEADER, then its items
							int32 nPartVolume = 0;
							uint32 nPChannel = 0;
							uint32 nPartVariations = 0xffffffff;
							const uint8* pp = w.pData;
							const uint8* ppEnd = w.pData + w.nLen;
							CMpChunk part;
							while (Mp_NextChunk(pp, ppEnd, part))
							{
								if (part.nId == MP_FCC('w','a','p','h') && part.nLen >= 12)
								{
									nPartVolume = (int32)Mp_Rd32(part.pData);
									nPartVariations = Mp_Rd32(part.pData + 4);
									nPChannel = Mp_Rd32(part.pData + 8);
								}
								if (part.nId != MP_FCC('L','I','S','T') || part.nType != MP_FCC('w','a','v','i'))
									continue;
								const uint8* pi = part.pData;
								const uint8* piEnd = part.pData + part.nLen;
								CMpChunk item;
								while (Mp_NextChunk(pi, piEnd, item))
								{
									if (item.nId != MP_FCC('L','I','S','T') || item.nType != MP_FCC('w','a','v','e'))
										continue;

									// an item: DMUS_IO_WAVE_ITEM_HEADER and the wave's reference
									bool bHead = false;
									CMpWaveItem wi;
									uint32 nItemVariations = 0xffffffff;
									char sFile[_MAX_PATH];
									sFile[0] = 0;
									const uint8* pj = item.pData;
									const uint8* pjEnd = item.pData + item.nLen;
									CMpChunk ic;
									while (Mp_NextChunk(pj, pjEnd, ic))
									{
										if (ic.nId == MP_FCC('w','a','i','h') && ic.nLen >= 64)
										{
											wi.nVolume = (int32)Mp_Rd32(ic.pData) + nPartVolume + nTrackVolume;
											nItemVariations = Mp_Rd32(ic.pData + 8);
											wi.mtStart = (int32)Mp_Rd32(ic.pData + 16);		// rtTime, music time
											wi.mtDuration = (int32)Mp_Rd32(ic.pData + 40);	// rtDuration, music time
											bHead = true;
										}
										else if (ic.nId == MP_FCC('L','I','S','T') && ic.nType == MP_FCC('D','M','R','F'))
										{
											const uint8* pr = ic.pData;
											const uint8* prEnd = ic.pData + ic.nLen;
											CMpChunk r;
											while (Mp_NextChunk(pr, prEnd, r))
											{
												if (r.nId != MP_FCC('f','i','l','e'))
													continue;
												// the file name: WCHAR
												uint32 nChars = r.nLen / 2;
												WCHAR wsFile[_MAX_PATH];
												if (nChars >= _MAX_PATH)
													nChars = _MAX_PATH - 1;
												memcpy(wsFile, r.pData, nChars * 2);
												wsFile[nChars] = 0;
												WideCharToMultiByte(CP_ACP, 0, wsFile, -1, sFile, sizeof(sFile), NULL, NULL);
											}
										}
									}
									if (!bHead || !sFile[0] || !(nItemVariations & nPartVariations))
										continue;
									wi.nPChannel = nPChannel;
									wi.pWave = pfnLoadWave(pPlayer, sFile);
									if (wi.pWave == LTNULL)
										continue;
									Mp_Append(m_aItems, m_nItems, wi);
								}
							}
						}
					}
					else if (k.nId == MP_FCC('s','e','q','t'))
					{
						// the sequence track: the curves (the notes are not
						// played: no instruments)
						if (!(nConfig & DMUS_TRACKCONFIG_PLAY_ENABLED))
							continue;
						const uint8* ps = k.pData;
						const uint8* psEnd = k.pData + k.nLen;
						CMpChunk s;
						while (Mp_NextChunk(ps, psEnd, s))
						{
							if (s.nLen < 4)
								continue;
							uint32 nSize = Mp_Rd32(s.pData);
							if (s.nId == MP_FCC('e','v','t','l') && nSize >= 20 && s.nLen >= 4 + nSize)
								LTDMConOutMsg(3, "Modern music player: segment %s has notes; not played (no instruments)\n", m_sName);
							if (s.nId != MP_FCC('c','u','r','l') || nSize < 28)
								continue;
							for (const uint8* q = s.pData + 4; q + nSize <= s.pData + s.nLen; q += nSize)
							{
								// DMUS_IO_CURVE_ITEM
								if (q[24] != DMUS_CURVET_CCCURVE || (q[26] != 7 && q[26] != 11))
									continue;
								CMpCurveItem cv;
								cv.mtStart = (int32)Mp_Rd32(q) + (int16)Mp_Rd16(q + 16);
								cv.mtDuration = (int32)Mp_Rd32(q + 4);
								cv.nPChannel = Mp_Rd32(q + 12);
								cv.nStart = (int16)Mp_Rd16(q + 18);
								cv.nEnd = (int16)Mp_Rd16(q + 20);
								cv.nShape = q[25];
								cv.nCC = (q[26] == 7) ? 0 : 1;
								Mp_Append(m_aCurves, m_nCurves, cv);
							}
						}
					}
				}
			}
		}
	}
	if (!bHeader)
		return false;

	// a length of at least a beat (a segment of no length would loop
	// without end)
	if (m_mtLength < DMUS_PPQ)
		m_mtLength = DMUS_PPQ;
	if (m_mtLoopEnd <= m_mtLoopStart || m_mtLoopEnd > m_mtLength)
		m_mtLoopEnd = m_mtLength;
	if (m_mtLoopStart < 0 || m_mtLoopStart >= m_mtLoopEnd)
		m_mtLoopStart = 0;
	if (m_mtPlayStart < 0 || m_mtPlayStart >= m_mtLength)
		m_mtPlayStart = 0;

	// the items and curves in time order (insertion sort: few)
	int i, j;
	for (i = 1; i < m_nItems; i++)
	{
		CMpWaveItem v = m_aItems[i];
		for (j = i; j > 0 && m_aItems[j - 1].mtStart > v.mtStart; j--)
			m_aItems[j] = m_aItems[j - 1];
		m_aItems[j] = v;
	}
	for (i = 1; i < m_nCurves; i++)
	{
		CMpCurveItem v = m_aCurves[i];
		for (j = i; j > 0 && m_aCurves[j - 1].mtStart > v.mtStart; j--)
			m_aCurves[j] = m_aCurves[j - 1];
		m_aCurves[j] = v;
	}
	return true;
}

// ----------------------------------------------------------------------- //
//  Playing segments, waves and curves
// ----------------------------------------------------------------------- //

#define MP_REPEAT_INFINITE	0xffffffff

class CMpSegState
{
public:
	CMpSegState(CMpSegment* pSegment, bool bSecondary, __int64 nStart, double dTempo)
	{
		m_pSegment = pSegment;
		m_bSecondary = bSecondary;
		m_nStart = nStart;
		m_dTempo = dTempo;
		m_nRepeatsLeft = pSegment->m_nRepeats;
		m_nStopTime = -1;
		m_pNext = LTNULL;
		BeginPass(pSegment->m_mtPlayStart, nStart);
	}

	// a pass through the segment from tick mtStart, at sample nStart
	void BeginPass(int32 mtStart, __int64 nStart)
	{
		m_mtPassStart = mtStart;
		m_nPassStart = nStart;
		m_mtPassEnd = m_nRepeatsLeft ? m_pSegment->m_mtLoopEnd : m_pSegment->m_mtLength;
		m_nPassEnd = SampleOf(m_mtPassEnd);
		m_nNextItem = 0;
		while (m_nNextItem < m_pSegment->m_nItems && m_pSegment->m_aItems[m_nNextItem].mtStart < mtStart)
			m_nNextItem++;
		m_nNextCurve = 0;
		while (m_nNextCurve < m_pSegment->m_nCurves && m_pSegment->m_aCurves[m_nNextCurve].mtStart < mtStart)
			m_nNextCurve++;
	}

	__int64 SampleOf(int32 mt) const
	{
		return m_nPassStart + (__int64)(m_pSegment->Seconds(m_mtPassStart, mt, m_dTempo) * MP_RATE + 0.5);
	}

	int32 TickAt(__int64 n) const
	{
		if (n <= m_nPassStart)
			return m_mtPassStart;
		return m_pSegment->TickAfter(m_mtPassStart, (double)(n - m_nPassStart) / MP_RATE, m_dTempo);
	}

	CMpSegment* m_pSegment;
	bool m_bSecondary;
	__int64 m_nStart;
	double m_dTempo;		// the tempo before the segment's first tempo
	uint32 m_nRepeatsLeft;
	int32 m_mtPassStart;
	int32 m_mtPassEnd;
	__int64 m_nPassStart;
	__int64 m_nPassEnd;
	int m_nNextItem;
	int m_nNextCurve;
	__int64 m_nStopTime;	// a secondary segment's stop, -1: none
	CMpSegState* m_pNext;
};

#define MP_FADE_SAMPLES		441		// 10 ms
#define MP_END_SAMPLES		128		// the fade at the end of a wave's duration

class CMpVoice
{
public:
	CMpVoice(CMpWave* pWave, __int64 nStart, __int64 nEnd, float fGain, uint32 nPChannel, CMpSegState* pOwner)
	{
		m_pWave = pWave;
		m_nStart = nStart;
		m_nEnd = nEnd;
		m_fGain = fGain;
		m_fLastGain = -1.0f;
		m_nPChannel = nPChannel & 255;
		m_pOwner = pOwner;
		m_fFade = 1.0f;
		m_fFadeStep = 0.0f;
		m_bDone = false;
		m_nSrcPos = 0;
		m_nPcmFrames = 0;
		m_nPcmPos = 0;
		m_nPcmChannels = 1;
		m_dStep = (double)pWave->m_nRate / MP_RATE;
		m_dFrac = 0.0;
		m_pNext = LTNULL;
		if (pWave->m_nFormat == 0x55)
			mp3dec_init(&m_Dec);
		m_bPrimed = false;
	}

	// the next frame of the wave at its own rate; false at its end
	bool NextFrame(float& l, float& r)
	{
		CMpWave* w = m_pWave;
		if (w->m_nFormat == 1)
		{
			uint32 nFrameBytes = (w->m_nBits / 8) * w->m_nChannels;
			if (m_nSrcPos + nFrameBytes > w->m_nDataLen)
				return false;
			const uint8* p = w->m_pData + m_nSrcPos;
			m_nSrcPos += nFrameBytes;
			if (w->m_nBits == 8)
			{
				l = (float)((int)p[0] - 128) * 256.0f;
				r = (w->m_nChannels > 1) ? (float)((int)p[1] - 128) * 256.0f : l;
			}
			else
			{
				l = (float)(int16)Mp_Rd16(p);
				r = (w->m_nChannels > 1) ? (float)(int16)Mp_Rd16(p + 2) : l;
			}
			return true;
		}

		// MP3: decode a frame when the last one is used
		while (m_nPcmPos >= m_nPcmFrames)
		{
			if (m_nSrcPos >= w->m_nDataLen)
				return false;
			mp3dec_frame_info_t info;
			int nSamples = mp3dec_decode_frame(&m_Dec, w->m_pData + m_nSrcPos, (int)(w->m_nDataLen - m_nSrcPos), m_aPcm, &info);
			if (info.frame_bytes <= 0)
				return false;
			m_nSrcPos += info.frame_bytes;
			m_nPcmFrames = nSamples;
			m_nPcmChannels = info.channels > 1 ? 2 : 1;
			m_nPcmPos = 0;
		}
		const short* s = m_aPcm + m_nPcmPos * m_nPcmChannels;
		l = (float)s[0];
		r = (m_nPcmChannels > 1) ? (float)s[1] : l;
		m_nPcmPos++;
		return true;
	}

	// add up to nFrames frames, from the gain g0 to g1, to pAcc; false
	// when the voice has ended
	bool Mix(float* pAcc, uint32 nFrames, float g0, float g1, __int64 nTime)
	{
		if (!m_bPrimed)
		{
			m_fA[0] = m_fA[1] = m_fB[0] = m_fB[1] = 0.0f;
			if (!NextFrame(m_fA[0], m_fA[1]))
				return false;
			m_bSrcEnded = !NextFrame(m_fB[0], m_fB[1]);
			m_bPrimed = true;
		}
		float gStep = nFrames ? (g1 - g0) / (float)nFrames : 0.0f;
		float g = g0;
		for (uint32 i = 0; i < nFrames; i++, g += gStep)
		{
			__int64 nLeft = m_nEnd - (nTime + i);
			if (nLeft <= 0)
				return false;
			float f = (float)m_dFrac;
			float gg = g * m_fFade;
			if (nLeft < MP_END_SAMPLES)
				gg *= (float)nLeft / MP_END_SAMPLES;
			pAcc[i * 2] += (m_fA[0] + (m_fB[0] - m_fA[0]) * f) * gg;
			pAcc[i * 2 + 1] += (m_fA[1] + (m_fB[1] - m_fA[1]) * f) * gg;
			if (m_fFadeStep != 0.0f)
			{
				m_fFade += m_fFadeStep;
				if (m_fFade <= 0.0f)
					return false;
			}
			m_dFrac += m_dStep;
			while (m_dFrac >= 1.0)
			{
				m_dFrac -= 1.0;
				if (m_bSrcEnded)
					return false;
				m_fA[0] = m_fB[0];
				m_fA[1] = m_fB[1];
				m_bSrcEnded = !NextFrame(m_fB[0], m_fB[1]);
				if (m_bSrcEnded)
					m_fB[0] = m_fB[1] = 0.0f;
			}
		}
		return true;
	}

	void FadeOut() { if (m_fFadeStep == 0.0f) m_fFadeStep = -1.0f / MP_FADE_SAMPLES; }

	CMpWave* m_pWave;
	__int64 m_nStart;
	__int64 m_nEnd;
	float m_fGain;			// item, part and track
	float m_fLastGain;
	uint32 m_nPChannel;
	CMpSegState* m_pOwner;	// a secondary segment's state, else LTNULL
	float m_fFade;
	float m_fFadeStep;
	bool m_bDone;
	bool m_bPrimed;
	bool m_bSrcEnded;
	uint32 m_nSrcPos;
	mp3dec_t m_Dec;
	short m_aPcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
	int m_nPcmFrames;
	int m_nPcmPos;
	int m_nPcmChannels;
	double m_dStep;
	double m_dFrac;
	float m_fA[2];
	float m_fB[2];
	CMpVoice* m_pNext;
};

class CMpCurveState
{
public:
	uint32 m_nPChannel;
	int m_nCC;
	__int64 m_nStart;
	__int64 m_nEnd;
	float m_fStart;
	float m_fEnd;
	uint8 m_nShape;
	CMpCurveState* m_pNext;

	float ValueAt(__int64 n) const
	{
		if (m_nShape == DMUS_CURVES_INSTANT || n >= m_nEnd || m_nEnd <= m_nStart)
			return m_fEnd;
		float t = (n <= m_nStart) ? 0.0f : (float)(n - m_nStart) / (float)(m_nEnd - m_nStart);
		switch (m_nShape)
		{
			case DMUS_CURVES_EXP: t = t * t; break;
			case DMUS_CURVES_LOG: t = 1.0f - (1.0f - t) * (1.0f - t); break;
			case DMUS_CURVES_SINE: t = 0.5f - 0.5f * (float)cos(3.14159265 * t); break;
			default: break;	// linear
		}
		return m_fStart + (m_fEnd - m_fStart) * t;
	}
};

// 1/100 dB to a gain
static float Mp_Gain(int32 nVolume)
{
	if (nVolume <= -9600)
		return 0.0f;
	return (float)pow(10.0, (double)nVolume / 2000.0);
}

// a line for the console (LTDMConsoleOutput 4) and for a debugger (or
// Wine's +debugstr): what plays, when
static void Mp_Debug(const char* sFormat, ...)
{
	char sLine[300];
	va_list args;
	va_start(args, sFormat);
	_vsnprintf(sLine, sizeof(sLine) - 1, sFormat, args);
	va_end(args);
	sLine[sizeof(sLine) - 1] = 0;
	OutputDebugStringA(sLine);
	LTDMConOutMsg(4, "%s", sLine);
}

// ----------------------------------------------------------------------- //
//  CModernMusicPlayer
// ----------------------------------------------------------------------- //

#define MP_MIX_MAX_FRAMES	(MP_RATE / 2)
#define MP_BLOCK			256
#define MP_LEAD_MS			150

CModernMusicPlayer::CModernMusicPlayer()
{
	InitializeCriticalSection(&m_cs);
	m_bNoOutput = false;
	m_bInitialized = false;
	m_bLevelInitialized = false;
	m_bCOM = false;
	m_sDirectory[0] = 0;
	m_nNumIntensities = 0;
	m_nInitialIntensity = 0;
	m_nInitialVolume = 0;
	m_nVolumeOffset = 0;
	m_aIntensities = LTNULL;
	m_aTransitions = LTNULL;
	m_nNumTransitions = 0;
	m_pSegments = LTNULL;
	m_pWaves = LTNULL;
	m_nCurIntensity = 0;
	m_nPrevIntensity = 0;
	m_aCommands = LTNULL;
	m_nCommands = 0;
	m_nCommandsAlloc = 0;
	m_nLastCommand = -1;
	m_nPaused = 0;
	m_nTime = 0;
	m_pPrimary = LTNULL;
	m_pPending = LTNULL;
	m_pSecondaries = LTNULL;
	m_nStopTime = -1;
	m_dTempo = 120.0;
	m_pVoices = LTNULL;
	m_pCurves = LTNULL;
	m_fMasterGain = 1.0f;
	m_fMasterTarget = 1.0f;
	m_pDirectSound = LTNULL;
	m_pBuffer = LTNULL;
	m_nBufferBytes = 0;
	m_nWritePos = 0;
	m_hThread = LTNULL;
	m_hExit = LTNULL;
	m_pMixBuffer = LTNULL;
	m_nUnderruns = 0;
	for (int i = 0; i < 256; i++)
	{
		m_afCC[i][0] = 100.0f;
		m_afCC[i][1] = 127.0f;
	}
}

CModernMusicPlayer::~CModernMusicPlayer()
{
	if (m_bInitialized)
		Term();
	DeleteCriticalSection(&m_cs);
}

LTRESULT CModernMusicPlayer::Init()
{
	if (m_bInitialized)
		return LT_ERROR;
	HRESULT hr = CoInitialize(LTNULL);
	m_bCOM = SUCCEEDED(hr);
	if (!StartOutput())
	{
		StopOutput();
		if (m_bCOM)
			CoUninitialize();
		m_bCOM = false;
		return LT_ERROR;
	}
	m_bInitialized = true;
	return LT_OK;
}

LTRESULT CModernMusicPlayer::Term()
{
	if (!m_bInitialized)
		return LT_OK;
	if (m_bLevelInitialized)
		TermLevel();
	StopOutput();
	if (m_bCOM)
		CoUninitialize();
	m_bCOM = false;
	m_bInitialized = false;
	return LT_OK;
}

// ----------------------------------------------------------------------- //
//  The level: the control file, the segments, the waves
// ----------------------------------------------------------------------- //

bool CModernMusicPlayer::ReadFile(const char* sName, uint8*& pData, uint32& nLen)
{
	pData = LTNULL;
	nLen = 0;
	FileRef ref;
	ref.m_FileType = FILE_ANYFILE;
	ref.m_pFilename = sName;
	ILTStream* pStream = s_pMpFileMgr->OpenFile(&ref);
	if (pStream == LTNULL)
		return false;
	nLen = pStream->GetLen();
	pData = new uint8[nLen ? nLen : 1];
	if (pStream->Read(pData, nLen) != LT_OK)
	{
		pStream->Release();
		delete [] pData;
		pData = LTNULL;
		return false;
	}
	pStream->Release();
	return true;
}

static CMpWave* Mp_LoadWaveCallback(CModernMusicPlayer* pPlayer, const char* sName);

// CLTDirectMusicMgr::StringToEnactType's names
static LTDMEnactTypes Mp_StringToEnactType(const char* sName)
{
	static const struct { const char* s; LTDMEnactTypes n; } s_a[] =
	{
		{ "Invalid", LTDMEnactInvalid }, { "Default", LTDMEnactDefault },
		{ "Immediatly", LTDMEnactImmediately }, { "Immediately", LTDMEnactImmediately },
		{ "Immediate", LTDMEnactImmediately }, { "NextBeat", LTDMEnactNextBeat },
		{ "NextMeasure", LTDMEnactNextMeasure }, { "NextGrid", LTDMEnactNextGrid },
		{ "NextSegment", LTDMEnactNextSegment }, { "NextMarker", LTDMEnactNextMarker },
		{ "Beat", LTDMEnactNextBeat }, { "Measure", LTDMEnactNextMeasure },
		{ "Grid", LTDMEnactNextGrid }, { "Segment", LTDMEnactNextSegment },
		{ "Marker", LTDMEnactNextMarker }
	};
	if (sName == LTNULL)
		return LTDMEnactInvalid;
	for (int i = 0; i < (int)(sizeof(s_a) / sizeof(s_a[0])); i++)
	{
		if (stricmp(sName, s_a[i].s) == 0)
			return s_a[i].n;
	}
	return LTDMEnactInvalid;
}

CMpWave* CModernMusicPlayer::LoadWave(const char* sName)
{
	CMpWave* pWave;
	for (pWave = m_pWaves; pWave != LTNULL; pWave = pWave->m_pNext)
	{
		if (stricmp(pWave->m_sName, sName) == 0)
			return pWave->m_pData ? pWave : LTNULL;
	}

	// a wave that does not load is kept too, so that it is not read again
	pWave = new CMpWave;
	pWave->m_sName = new char[strlen(sName) + 1];
	strcpy(pWave->m_sName, sName);
	pWave->m_pNext = m_pWaves;
	m_pWaves = pWave;

	char sPath[_MAX_PATH * 2];
	LTSNPrintF(sPath, sizeof(sPath), "%s\\%s", m_sDirectory, sName);
	uint8* pFile;
	uint32 nLen;
	if (!ReadFile(sPath, pFile, nLen))
	{
		LTDMConOutWarning("WARNING! Modern music player: no wave %s\n", sPath);
		return LTNULL;
	}

	const uint8* p = pFile;
	const uint8* pEnd = pFile + nLen;
	CMpChunk form, c;
	bool bFormat = false;
	if (Mp_NextChunk(p, pEnd, form) && form.nId == MP_FCC('R','I','F','F') && form.nType == MP_FCC('W','A','V','E'))
	{
		p = form.pData;
		pEnd = form.pData + form.nLen;
		while (Mp_NextChunk(p, pEnd, c))
		{
			if (c.nId == MP_FCC('f','m','t',' ') && c.nLen >= 16)
			{
				pWave->m_nFormat = Mp_Rd16(c.pData);
				pWave->m_nChannels = Mp_Rd16(c.pData + 2);
				pWave->m_nRate = Mp_Rd32(c.pData + 4);
				pWave->m_nBits = Mp_Rd16(c.pData + 14);
				bFormat = true;
			}
			else if (c.nId == MP_FCC('d','a','t','a') && pWave->m_pData == LTNULL)
			{
				pWave->m_pData = c.pData;
				pWave->m_nDataLen = c.nLen;
			}
		}
	}
	bool bOk = bFormat && pWave->m_pData != LTNULL && pWave->m_nRate >= 4000 && pWave->m_nRate <= 192000 &&
		pWave->m_nChannels >= 1 && pWave->m_nChannels <= 2 &&
		(pWave->m_nFormat == 0x55 || (pWave->m_nFormat == 1 && (pWave->m_nBits == 8 || pWave->m_nBits == 16)));
	if (!bOk)
	{
		LTDMConOutWarning("WARNING! Modern music player: wave %s is not MP3 or PCM\n", sPath);
		pWave->m_pData = LTNULL;
		delete [] pFile;
		return LTNULL;
	}
	pWave->m_pFile = pFile;
	return pWave;
}

static CMpWave* Mp_LoadWaveCallback(CModernMusicPlayer* pPlayer, const char* sName)
{
	return pPlayer->LoadWave(sName);
}

CMpSegment* CModernMusicPlayer::LoadSegment(const char* sName)
{
	CMpSegment* pSegment;
	for (pSegment = m_pSegments; pSegment != LTNULL; pSegment = pSegment->m_pNext)
	{
		if (stricmp(pSegment->m_sName, sName) == 0)
			return pSegment;
	}

	char sPath[_MAX_PATH * 2];
	LTSNPrintF(sPath, sizeof(sPath), "%s\\%s", m_sDirectory, sName);
	uint8* pFile;
	uint32 nLen;
	if (!ReadFile(sPath, pFile, nLen))
	{
		LTDMConOutWarning("WARNING! LTDirectMusic failed to load segment %s\n", sName);
		return LTNULL;
	}
	pSegment = new CMpSegment;
	pSegment->m_sName = new char[strlen(sName) + 1];
	strcpy(pSegment->m_sName, sName);
	bool bOk = pSegment->Parse(pFile, nLen, this, Mp_LoadWaveCallback);
	delete [] pFile;
	if (!bOk)
	{
		LTDMConOutWarning("WARNING! LTDirectMusic failed to load segment %s\n", sName);
		delete pSegment;
		return LTNULL;
	}
	pSegment->m_pNext = m_pSegments;
	m_pSegments = pSegment;
	LTDMConOutMsg(3, "LTDirectMusic loaded segment %s (modern music player: %d waves, %d curves)\n",
		sName, pSegment->m_nItems, pSegment->m_nCurves);
	return pSegment;
}

CModernMusicPlayer::CTransition* CModernMusicPlayer::GetTransition(int nFrom, int nTo)
{
	// [0] is the default: the next measure, manual, no segment
	if (nFrom <= 0 || nFrom > m_nNumIntensities || nTo <= 0 || nTo > m_nNumIntensities)
		return &m_aTransitions[0];
	return &m_aTransitions[1 + (nFrom - 1) * m_nNumIntensities + (nTo - 1)];
}

bool CModernMusicPlayer::ReadControlFile(CControlFileMgr& controlFile)
{
	CControlFileKey* pKey;
	CControlFileWord* pWord;

	// what this player does not have
	static const char* s_aNotHere[] = { "DLSBANK", "STYLE", "BAND", "MOTIF", "CHORDMAP" };
	for (int n = 0; n < (int)(sizeof(s_aNotHere) / sizeof(s_aNotHere[0])); n++)
	{
		if (controlFile.GetKey(LTNULL, s_aNotHere[n]) != LTNULL)
			LTDMConOutWarning("WARNING! Modern music player: %s is not played (no DirectMusic)\n", s_aNotHere[n]);
	}

	// the intensities (CLTDirectMusicMgr::ReadIntensities)
	for (pKey = controlFile.GetKey(LTNULL, "INTENSITY"); pKey != LTNULL; pKey = pKey->NextWithSameName())
	{
		pWord = pKey->GetFirstWord();
		if (pWord == LTNULL) continue;
		int nIntensity = 0;
		pWord->GetVal(nIntensity);
		if (nIntensity <= 0 || nIntensity > m_nNumIntensities) continue;
		pWord = pWord->Next();
		if (pWord == LTNULL) continue;
		int nLoops = -1;
		pWord->GetVal(nLoops);
		pWord = pWord->Next();
		if (pWord == LTNULL) continue;
		int nNext = 0;
		pWord->GetVal(nNext);
		pWord = pWord->Next();
		if (pWord == LTNULL) continue;
		CIntensity& in = m_aIntensities[nIntensity];
		for (; pWord != LTNULL; pWord = pWord->Next())
		{
			CMpSegment* pSegment = LoadSegment(pWord->GetVal());
			if (pSegment == LTNULL) break;
			Mp_Append(in.apSegments, in.nSegments, pSegment);
		}
		in.nLoops = nLoops;
		in.nNext = nNext;
	}

	// the secondary segments (ReadSecondarySegments)
	for (pKey = controlFile.GetKey(LTNULL, "SECONDARYSEGMENT"); pKey != LTNULL; pKey = pKey->NextWithSameName())
	{
		pWord = pKey->GetFirstWord();
		if (pWord == LTNULL) continue;
		CMpSegment* pSegment = LoadSegment(pWord->GetVal());
		if (pSegment == LTNULL) continue;
		pWord = pWord->Next();
		if (pWord != LTNULL)
			pSegment->m_nDefaultEnact = Mp_StringToEnactType(pWord->GetVal());
	}

	// the transitions (ReadTransitions)
	for (pKey = controlFile.GetKey(LTNULL, "TRANSITION"); pKey != LTNULL; pKey = pKey->NextWithSameName())
	{
		pWord = pKey->GetFirstWord();
		if (pWord == LTNULL) continue;
		int nFrom = 0;
		pWord->GetVal(nFrom);
		if (nFrom <= 0 || nFrom > m_nNumIntensities) continue;
		pWord = pWord->Next();
		if (pWord == LTNULL) continue;
		int nTo = 0;
		pWord->GetVal(nTo);
		if (nTo <= 0 || nTo > m_nNumIntensities) continue;
		pWord = pWord->Next();
		if (pWord == LTNULL) continue;
		LTDMEnactTypes nEnact = Mp_StringToEnactType(pWord->GetVal());
		if (nEnact == LTDMEnactInvalid) continue;
		pWord = pWord->Next();
		if (pWord == LTNULL) continue;
		bool bManual;
		if (stricmp(pWord->GetVal(), "MANUAL") == 0) bManual = true;
		else if (stricmp(pWord->GetVal(), "AUTOMATIC") == 0) bManual = false;
		else continue;
		CTransition* pTransition = GetTransition(nFrom, nTo);
		pWord = pWord->Next();
		if (pWord != LTNULL)
			pTransition->pSegment = LoadSegment(pWord->GetVal());
		pTransition->nEnact = nEnact;
		pTransition->bManual = bManual;
	}
	return true;
}

LTRESULT CModernMusicPlayer::InitLevel(const char* sWorkingDirectory, const char* sControlFileName,
	const char* sDefine1, const char* sDefine2, const char* sDefine3)
{
	CControlFileMgrDStream controlFile;

	if (!m_bInitialized) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::InitLevel"); return LT_ERROR; }
	if (m_bLevelInitialized) { LTDMConOutError("ERROR! InitLevel has already been called and not terminated. (CLTDirectMusicMgr::InitLevel)\n"); return LT_ERROR; }
	if (sControlFileName == LTNULL) { LTDMConOutError("ERROR! Control file name not valid. (CLTDirectMusicMgr::InitLevel)\n"); return LT_ERROR; }
	if (sWorkingDirectory == LTNULL) { LTDMConOutError("ERROR! Working directory not valid. (CLTDirectMusicMgr::InitLevel)\n"); return LT_ERROR; }

	// the directory, without a separator at its end
	LTStrCpy(m_sDirectory, sWorkingDirectory, sizeof(m_sDirectory));
	int nDirLen = strlen(m_sDirectory);
	while (nDirLen > 0 && (m_sDirectory[nDirLen - 1] == '\\' || m_sDirectory[nDirLen - 1] == '/'))
		m_sDirectory[--nDirLen] = 0;

	if (sDefine1 != LTNULL) controlFile.AddDefine(sDefine1);
	if (sDefine2 != LTNULL) controlFile.AddDefine(sDefine2);
	if (sDefine3 != LTNULL) controlFile.AddDefine(sDefine3);

	// the control file: in the directory, if its name has no path
	char sControl[_MAX_PATH * 2];
	if (strchr(sControlFileName, '\\') == LTNULL && strchr(sControlFileName, '/') == LTNULL && strchr(sControlFileName, ':') == LTNULL)
	{
		if (nDirLen > 0 && m_sDirectory[nDirLen - 1] != ':')
			LTSNPrintF(sControl, sizeof(sControl), "%s\\%s", m_sDirectory, sControlFileName);
		else
			LTSNPrintF(sControl, sizeof(sControl), "%s%s", m_sDirectory, sControlFileName);
	}
	else
		LTStrCpy(sControl, sControlFileName, sizeof(sControl));
	if (!controlFile.Init(sControl))
	{
		LTDMConOutError("ERROR! Unable to read control file. (CLTDirectMusicMgr::InitLevel)\n");
		return LT_ERROR;
	}

	m_nNumIntensities = 0;
	m_nInitialIntensity = 0;
	m_nInitialVolume = 0;
	m_nVolumeOffset = 0;
	controlFile.GetKeyVal(LTNULL, "NUMINTENSITIES", m_nNumIntensities);
	controlFile.GetKeyVal(LTNULL, "INITIALINTENSITY", m_nInitialIntensity);
	controlFile.GetKeyVal(LTNULL, "INITIALVOLUME", m_nInitialVolume);
	controlFile.GetKeyVal(LTNULL, "VOLUMEOFFSET", m_nVolumeOffset);
	if (m_nNumIntensities < 1 || m_nNumIntensities > 1000)
	{
		LTDMConOutError("ERROR! Number of intensities is not valid. NumIntensities = %i (CLTDirectMusicMgr::InitLevel)\n", m_nNumIntensities);
		controlFile.Term();
		return LT_ERROR;
	}

	int i;
	m_aIntensities = new CIntensity[m_nNumIntensities + 1];
	for (i = 0; i <= m_nNumIntensities; i++)
	{
		m_aIntensities[i].apSegments = LTNULL;
		m_aIntensities[i].nSegments = 0;
		m_aIntensities[i].nLoops = 0;
		m_aIntensities[i].nNext = 0;
	}
	m_nNumTransitions = 1 + m_nNumIntensities * m_nNumIntensities;
	m_aTransitions = new CTransition[m_nNumTransitions];
	for (i = 0; i < m_nNumTransitions; i++)
	{
		m_aTransitions[i].nEnact = LTDMEnactNextMeasure;
		m_aTransitions[i].bManual = true;
		m_aTransitions[i].pSegment = LTNULL;
	}

	ReadControlFile(controlFile);
	controlFile.Term();

	{
		int nSegments = 0, nWaves = 0;
		CMpSegment* pSegment;
		CMpWave* pWave;
		for (pSegment = m_pSegments; pSegment; pSegment = pSegment->m_pNext) nSegments++;
		for (pWave = m_pWaves; pWave; pWave = pWave->m_pNext) if (pWave->m_pData) nWaves++;
		char sLine[300];
		LTSNPrintF(sLine, sizeof(sLine), "LTDirectMusic: modern music player: %s: %d intensities, %d segments, %d waves\n",
			sControl, m_nNumIntensities, nSegments, nWaves);
		OutputDebugStringA(sLine);
		LTDMConOutMsg(3, sLine);
	}

	EnterCriticalSection(&m_cs);
	m_nCurIntensity = 0;
	m_nPrevIntensity = 0;
	m_nLastCommand = -1;
	m_nPaused = 0;
	m_bLevelInitialized = true;
	LeaveCriticalSection(&m_cs);

	SetVolume(m_nInitialVolume);
	return LT_OK;
}

void CModernMusicPlayer::FreeStates()
{
	while (m_pVoices)
	{
		CMpVoice* p = m_pVoices;
		m_pVoices = p->m_pNext;
		delete p;
	}
	while (m_pCurves)
	{
		CMpCurveState* p = m_pCurves;
		m_pCurves = p->m_pNext;
		delete p;
	}
	while (m_pSecondaries)
	{
		CMpSegState* p = m_pSecondaries;
		m_pSecondaries = p->m_pNext;
		delete p;
	}
	delete m_pPrimary;
	m_pPrimary = LTNULL;
	delete m_pPending;
	m_pPending = LTNULL;
	m_nStopTime = -1;
}

void CModernMusicPlayer::FreeLevel()
{
	ClearCommands();
	delete [] m_aCommands;
	m_aCommands = LTNULL;
	m_nCommandsAlloc = 0;
	if (m_aIntensities)
	{
		for (int i = 0; i <= m_nNumIntensities; i++)
			delete [] m_aIntensities[i].apSegments;
		delete [] m_aIntensities;
		m_aIntensities = LTNULL;
	}
	delete [] m_aTransitions;
	m_aTransitions = LTNULL;
	while (m_pSegments)
	{
		CMpSegment* p = m_pSegments;
		m_pSegments = p->m_pNext;
		delete p;
	}
	while (m_pWaves)
	{
		CMpWave* p = m_pWaves;
		m_pWaves = p->m_pNext;
		delete p;
	}
}

LTRESULT CModernMusicPlayer::TermLevel()
{
	if (!m_bInitialized) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::TermLevel"); return LT_ERROR; }
	if (!m_bLevelInitialized)
	{
		LTDMConOutWarning("WARNING! Level already terminated or never initialized. (CLTDirectMusicMgr::TermLevel)\n");
		return LT_OK;
	}
	EnterCriticalSection(&m_cs);
	FreeStates();
	FreeLevel();
	m_bLevelInitialized = false;
	m_nPaused = 0;
	LeaveCriticalSection(&m_cs);
	return LT_OK;
}

// ----------------------------------------------------------------------- //
//  The intensities and the command queue (CLTDirectMusicMgr's)
// ----------------------------------------------------------------------- //

void CModernMusicPlayer::ClearCommands()
{
	m_nCommands = 0;
	m_nLastCommand = -1;
}

void CModernMusicPlayer::AddCommand(int nType, CMpSegment* pSegment, int nVal)
{
	if (m_nCommands >= m_nCommandsAlloc)
	{
		int nNew = m_nCommandsAlloc ? m_nCommandsAlloc * 2 : 16;
		CCommand* aNew = new CCommand[nNew];
		if (m_nCommands)
			memcpy(aNew, m_aCommands, m_nCommands * sizeof(CCommand));
		delete [] m_aCommands;
		m_aCommands = aNew;
		m_nCommandsAlloc = nNew;
	}
	m_aCommands[m_nCommands].nType = nType;
	m_aCommands[m_nCommands].pSegment = pSegment;
	m_aCommands[m_nCommands].nVal = nVal;
	m_nCommands++;
}

// the sample at which a change with the enact time nEnact starts (pNew:
// the segment that starts, for "Default")
__int64 CModernMusicPlayer::EnactTime(LTDMEnactTypes nEnact, CMpSegment* pNew)
{
	if (nEnact == LTDMEnactDefault)
	{
		uint32 nRes = pNew ? pNew->m_nResolution : (m_pPrimary ? m_pPrimary->m_pSegment->m_nResolution : 0);
		if (nRes & DMUS_SEGF_SEGMENTEND) nEnact = LTDMEnactNextSegment;
		else if (nRes & DMUS_SEGF_MEASURE) nEnact = LTDMEnactNextMeasure;
		else if (nRes & DMUS_SEGF_BEAT) nEnact = LTDMEnactNextBeat;
		else if (nRes & DMUS_SEGF_GRID) nEnact = LTDMEnactNextGrid;
		else nEnact = LTDMEnactImmediately;
	}
	CMpSegState* p = m_pPrimary;
	if (p == LTNULL)
		return m_nTime;
	switch (nEnact)
	{
		case LTDMEnactNextSegment:
			return (p->m_nPassEnd > m_nTime) ? p->m_nPassEnd : m_nTime;

		case LTDMEnactNextBeat:
		case LTDMEnactNextMeasure:
		case LTDMEnactNextGrid:
		{
			int32 mt = p->TickAt(m_nTime);
			CMpTimeSig sig;
			int32 mtNextSig;
			p->m_pSegment->TimeSigAt(mt, sig, mtNextSig);
			int32 nUnit = sig.mtBeat;
			if (nEnact == LTDMEnactNextMeasure) nUnit = sig.mtBeat * sig.nBeatsPerMeasure;
			else if (nEnact == LTDMEnactNextGrid) nUnit = sig.mtBeat / sig.nGridsPerBeat;
			if (nUnit < 1) nUnit = 1;
			int32 mtFrom = mt - sig.mtTime;
			int32 mtBoundary = sig.mtTime + ((mtFrom + nUnit - 1) / nUnit) * nUnit;
			if (mtBoundary > mtNextSig) mtBoundary = mtNextSig;
			if (mtBoundary >= p->m_mtPassEnd)
				return (p->m_nPassEnd > m_nTime) ? p->m_nPassEnd : m_nTime;
			__int64 n = p->SampleOf(mtBoundary);
			return (n > m_nTime) ? n : m_nTime;
		}

		default:	// immediately; a marker (no segment has markers)
			return m_nTime;
	}
}

LTRESULT CModernMusicPlayer::ChangeIntensityLocked(int nNewIntensity, LTDMEnactTypes nStart, bool bFromQueue)
{
	if (nNewIntensity == 0)
	{
		StopLocked(nStart);
		return LT_ERROR;
	}
	if (nNewIntensity <= 0 || nNewIntensity > m_nNumIntensities)
		return LT_ERROR;
	if (m_nCurIntensity == nNewIntensity)
		return LT_OK;

	// while the segment of the last change has not started, the
	// transition is from the intensity before it (as retail)
	CTransition* pTransition = m_nPrevIntensity ? GetTransition(m_nPrevIntensity, nNewIntensity) : GetTransition(m_nCurIntensity, nNewIntensity);
	m_nPrevIntensity = m_nCurIntensity;

	ClearCommands();
	LTDMEnactTypes nEnact = (nStart == LTDMEnactInvalid) ? pTransition->nEnact : nStart;

	CIntensity& in = m_aIntensities[nNewIntensity];
	CMpSegment* pFirst = LTNULL;
	bool bSkipFirst = false;
	if (pTransition->pSegment != LTNULL)
		pFirst = pTransition->pSegment;
	else if (in.nSegments > 0)
	{
		pFirst = in.apSegments[0];
		bSkipFirst = true;
	}
	for (int i = 0; i < in.nSegments; i++)
		AddCommand(MP_CMD_PLAY, in.apSegments[i], 0);
	m_nLastCommand = bSkipFirst ? 0 : -1;
	AddCommand(MP_CMD_LOOP, LTNULL, (in.nNext == nNewIntensity) ? -1 : in.nLoops);
	if (in.nNext > 0)
		AddCommand(MP_CMD_CHANGE, LTNULL, in.nNext);

	m_nCurIntensity = nNewIntensity;

	if (pFirst != LTNULL)
	{
		// the segment starts at the enact time; one that waits for its time
		// is replaced
		__int64 nAt = EnactTime(nEnact, pFirst);
		delete m_pPending;
		m_pPending = new CMpSegState(pFirst, false, nAt, m_dTempo);
		Mp_Debug("LTDirectMusic: modern music player: intensity %d (from %d): %s in %d ms%s\n", nNewIntensity,
			m_nPrevIntensity, pFirst->m_sName, (int)((nAt - m_nTime) * 1000 / MP_RATE), bFromQueue ? " (queue)" : "");
	}
	return LT_OK;
}

// the end of the primary segment: the next command(s) of the queue
// (CLTDirectMusicMgr::HandleSegmentNotification_SegAlmostEnd)
void CModernMusicPlayer::RunQueue()
{
	int nCur = (m_nLastCommand >= 0) ? m_nLastCommand + 1 : 0;
	bool bDone = false;
	int nGuard = 0;
	while (!bDone && nCur < m_nCommands && nGuard++ < 1000)
	{
		CCommand& cmd = m_aCommands[nCur];
		switch (cmd.nType)
		{
			case MP_CMD_LOOP:
				if (cmd.nVal == 0) { nCur++; break; }
				if (cmd.nVal > 0) cmd.nVal--;
				if (cmd.nVal == 0) { nCur++; break; }
				nCur = 0;
				break;

			case MP_CMD_CHANGE:
				if (ChangeIntensityLocked(cmd.nVal, LTDMEnactInvalid, true) == LT_OK)
				{
					// the change made a new queue
					nCur = m_nLastCommand;
					bDone = true;
				}
				else
				{
					// (retail tries the same command again, without end)
					nCur++;
				}
				break;

			case MP_CMD_PLAY:
				delete m_pPending;
				m_pPending = new CMpSegState(cmd.pSegment, false, m_nTime, m_dTempo);
				bDone = true;
				break;

			default:
				nCur++;
				break;
		}
	}
	m_nLastCommand = (nCur < m_nCommands) ? nCur : -1;
}

void CModernMusicPlayer::StartPending()
{
	CMpSegState* p = m_pPending;
	m_pPending = LTNULL;
	if (m_pPrimary)
	{
		// the old segment's waves and curves go on (NOINVALIDATE)
		m_dTempo = m_pPrimary->m_pSegment->EndTempo(m_pPrimary->m_dTempo);
		delete m_pPrimary;
	}
	// the tempo before the segment's own tempo track
	p->m_dTempo = m_dTempo;
	p->BeginPass(p->m_pSegment->m_mtPlayStart, p->m_nStart);
	m_pPrimary = p;
	m_nPrevIntensity = 0;	// DMUS_NOTIFICATION_SEGSTART
	Mp_Debug("LTDirectMusic: modern music player: segment %s starts at %d.%03d s\n", p->m_pSegment->m_sName,
		(int)(p->m_nStart / MP_RATE), (int)(p->m_nStart % MP_RATE * 1000 / MP_RATE));
}

void CModernMusicPlayer::StopAllNow(bool bFade)
{
	CMpVoice* v;
	for (v = m_pVoices; v != LTNULL; v = v->m_pNext)
	{
		if (bFade) v->FadeOut();
		else v->m_bDone = true;
	}
	while (m_pCurves)
	{
		CMpCurveState* p = m_pCurves;
		m_pCurves = p->m_pNext;
		delete p;
	}
	while (m_pSecondaries)
	{
		CMpSegState* p = m_pSecondaries;
		m_pSecondaries = p->m_pNext;
		delete p;
	}
	if (m_pPrimary)
	{
		m_dTempo = m_pPrimary->m_pSegment->EndTempo(m_pPrimary->m_dTempo);
		delete m_pPrimary;
		m_pPrimary = LTNULL;
	}
	delete m_pPending;
	m_pPending = LTNULL;
	m_nStopTime = -1;
}

void CModernMusicPlayer::StopLocked(LTDMEnactTypes nStart)
{
	// (CLTDirectMusicMgr::Stop: the performance stops at the enact time,
	// except at the end of the segment: then the segment ends with nothing
	// after it; the queue is cleared)
	if (nStart != LTDMEnactNextSegment)
	{
		__int64 nAt = EnactTime(nStart, LTNULL);
		delete m_pPending;
		m_pPending = LTNULL;
		if (nAt <= m_nTime)
			StopAllNow(true);
		else
			m_nStopTime = nAt;
	}
	m_nPaused = 0;
	ClearCommands();
}

// ----------------------------------------------------------------------- //
//  The interface
// ----------------------------------------------------------------------- //

#define MP_CHECK(name) \
	if (!m_bInitialized) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::" name); return LT_ERROR; } \
	if (!m_bLevelInitialized) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::" name); return LT_ERROR; }

LTRESULT CModernMusicPlayer::Play()
{
	Mp_Debug("LTDirectMusic: modern music player: Play\n");
	MP_CHECK("Play");
	return ChangeIntensity(m_nInitialIntensity, LTDMEnactImmediately);
}

LTRESULT CModernMusicPlayer::Stop(const LTDMEnactTypes nStart)
{
	Mp_Debug("LTDirectMusic: modern music player: Stop %d\n", (int)nStart);
	MP_CHECK("Stop");
	EnterCriticalSection(&m_cs);
	StopLocked(nStart);
	LeaveCriticalSection(&m_cs);
	return LT_OK;
}

LTRESULT CModernMusicPlayer::Pause(const LTDMEnactTypes nStart)
{
	Mp_Debug("LTDirectMusic: modern music player: Pause %d\n", (int)nStart);
	MP_CHECK("Pause");
	EnterCriticalSection(&m_cs);
	// (retail fails when no segment plays)
	if (m_nPaused == 0 && m_pPrimary == LTNULL && m_pPending == LTNULL)
	{
		LeaveCriticalSection(&m_cs);
		return LT_ERROR;
	}
	m_nPaused++;
	LeaveCriticalSection(&m_cs);
	return LT_OK;
}

LTRESULT CModernMusicPlayer::UnPause()
{
	Mp_Debug("LTDirectMusic: modern music player: UnPause\n");
	if (!m_bInitialized || !m_bLevelInitialized)
		return LT_ERROR;
	EnterCriticalSection(&m_cs);
	if (!m_nPaused)
	{
		LeaveCriticalSection(&m_cs);
		return LT_OVERFLOW;
	}
	m_nPaused--;
	LeaveCriticalSection(&m_cs);
	return LT_OK;
}

LTRESULT CModernMusicPlayer::SetVolume(const long nVolume)
{
	long nSet = nVolume + m_nVolumeOffset;
	Mp_Debug("LTDirectMusic: modern music player: SetVolume %ld (%ld)\n", nVolume, nSet);
	MP_CHECK("SetVolume");
	// GUID_PerfMasterVolume: 1/100 dB, -200 dB to +20 dB
	if (nSet > 2000) nSet = 2000;
	if (nSet < -20000) nSet = -20000;
	EnterCriticalSection(&m_cs);
	m_fMasterTarget = Mp_Gain(nSet);
	LeaveCriticalSection(&m_cs);
	return LT_OK;
}

LTRESULT CModernMusicPlayer::ChangeIntensity(const int nNewIntensity, const LTDMEnactTypes nStart)
{
	Mp_Debug("LTDirectMusic: modern music player: ChangeIntensity %d %d\n", nNewIntensity, (int)nStart);
	MP_CHECK("ChangeIntensity");
	EnterCriticalSection(&m_cs);
	LTRESULT r = ChangeIntensityLocked(nNewIntensity, nStart, false);
	LeaveCriticalSection(&m_cs);
	return r;
}

LTRESULT CModernMusicPlayer::PlaySecondary(const char* sSecondarySegment, const LTDMEnactTypes nStart)
{
	MP_CHECK("PlaySecondary");
	if (sSecondarySegment == LTNULL)
		return LT_ERROR;
	CMpSegment* pSegment;
	for (pSegment = m_pSegments; pSegment != LTNULL; pSegment = pSegment->m_pNext)
	{
		if (stricmp(pSegment->m_sName, sSecondarySegment) == 0)
			break;
	}
	if (pSegment == LTNULL)
		return LT_ERROR;
	LTDMEnactTypes nEnact = (nStart == LTDMEnactDefault) ? pSegment->m_nDefaultEnact : nStart;
	EnterCriticalSection(&m_cs);
	// (at the end of the segment, retail plays it at the next measure
	// after the notification that the segment almost ends: here at the end
	// of the primary segment)
	CMpSegState* p = new CMpSegState(pSegment, true, EnactTime(nEnact, pSegment), m_dTempo);
	p->m_pNext = m_pSecondaries;
	m_pSecondaries = p;
	LeaveCriticalSection(&m_cs);
	return LT_OK;
}

LTRESULT CModernMusicPlayer::StopSecondary(const char* sSecondarySegment, const LTDMEnactTypes nStart)
{
	MP_CHECK("StopSecondary");
	EnterCriticalSection(&m_cs);
	for (CMpSegState* p = m_pSecondaries; p != LTNULL; p = p->m_pNext)
	{
		if (sSecondarySegment == LTNULL || stricmp(p->m_pSegment->m_sName, sSecondarySegment) == 0)
		{
			LTDMEnactTypes nEnact = (nStart == LTDMEnactDefault) ? p->m_pSegment->m_nDefaultEnact : nStart;
			p->m_nStopTime = EnactTime(nEnact, LTNULL);
		}
	}
	LeaveCriticalSection(&m_cs);
	return LT_OK;
}

LTRESULT CModernMusicPlayer::PlayMotif(const char* sStyleName, const char* sMotifName, const LTDMEnactTypes nStart)
{
	MP_CHECK("PlayMotif");
	// no styles: no motif is found (as retail without the motif)
	return LT_ERROR;
}

LTRESULT CModernMusicPlayer::StopMotif(const char* sStyleName, const char* sMotifName, const LTDMEnactTypes nStart)
{
	MP_CHECK("StopMotif");
	return LT_OK;
}

// ----------------------------------------------------------------------- //
//  The mix
// ----------------------------------------------------------------------- //

// start the voices and curves of a segment state that start before nUntil
void CModernMusicPlayer::Schedule(CMpSegState* p, __int64 nUntil)
{
	CMpSegment* s = p->m_pSegment;
	while (p->m_nNextItem < s->m_nItems)
	{
		CMpWaveItem& it = s->m_aItems[p->m_nNextItem];
		if (it.mtStart >= p->m_mtPassEnd)
			break;
		__int64 nAt = p->SampleOf(it.mtStart);
		if (nAt >= nUntil)
			break;
		p->m_nNextItem++;
		if (nAt < m_nTime)
			nAt = m_nTime;
		__int64 nEnd = p->SampleOf(it.mtStart + it.mtDuration);
		if (nEnd <= nAt)
			continue;
		CMpVoice* v = new CMpVoice(it.pWave, nAt, nEnd, Mp_Gain(it.nVolume), it.nPChannel, p->m_bSecondary ? p : LTNULL);
		v->m_pNext = m_pVoices;
		m_pVoices = v;
	}
	while (p->m_nNextCurve < s->m_nCurves)
	{
		CMpCurveItem& cv = s->m_aCurves[p->m_nNextCurve];
		if (cv.mtStart >= p->m_mtPassEnd)
			break;
		__int64 nAt = p->SampleOf(cv.mtStart);
		if (nAt >= nUntil)
			break;
		p->m_nNextCurve++;
		CMpCurveState* c = new CMpCurveState;
		c->m_nPChannel = cv.nPChannel & 255;
		c->m_nCC = cv.nCC;
		c->m_nStart = nAt;
		c->m_nEnd = p->SampleOf(cv.mtStart + cv.mtDuration);
		c->m_fStart = (float)cv.nStart;
		c->m_fEnd = (float)cv.nEnd;
		c->m_nShape = cv.nShape;
		c->m_pNext = m_pCurves;
		m_pCurves = c;
	}
}

void CModernMusicPlayer::Mix(short* pOut, uint32 nFrames)
{
	static float s_afAcc[MP_BLOCK * 2];
	uint32 nDone = 0;

	EnterCriticalSection(&m_cs);
	while (nDone < nFrames)
	{
		uint32 n = nFrames - nDone;
		if (n > MP_BLOCK)
			n = MP_BLOCK;
		memset(s_afAcc, 0, sizeof(s_afAcc));

		// paused: silence, and the time stands still
		bool bPlay = m_bLevelInitialized && m_nPaused == 0;
		if (bPlay)
		{
			// the events at this time
			int nGuard = 0;
			for (;;)
			{
				if (++nGuard > 64)
				{
					// (a queue that plays nothing: stop it)
					StopAllNow(true);
					ClearCommands();
					break;
				}
				if (m_nStopTime >= 0 && m_nStopTime <= m_nTime)
				{
					StopAllNow(true);
					continue;
				}
				if (m_pPending && m_pPending->m_nStart <= m_nTime)
				{
					StartPending();
					continue;
				}
				if (m_pPrimary && m_pPrimary->m_nPassEnd <= m_nTime)
				{
					CMpSegState* p = m_pPrimary;
					if (p->m_nRepeatsLeft)
					{
						if (p->m_nRepeatsLeft != MP_REPEAT_INFINITE)
							p->m_nRepeatsLeft--;
						p->BeginPass(p->m_pSegment->m_mtLoopStart, p->m_nPassEnd);
						continue;
					}
					RunQueue();
					if (m_pPending && m_pPending->m_nStart <= m_nTime)
						StartPending();
					else if (m_pPrimary == p)
					{
						m_dTempo = p->m_pSegment->EndTempo(p->m_dTempo);
						delete p;
						m_pPrimary = LTNULL;
					}
					continue;
				}
				break;
			}

			// the secondary segments that end
			CMpSegState** pp = &m_pSecondaries;
			while (*pp)
			{
				CMpSegState* p = *pp;
				if (p->m_nPassEnd <= m_nTime && p->m_nRepeatsLeft)
				{
					if (p->m_nRepeatsLeft != MP_REPEAT_INFINITE)
						p->m_nRepeatsLeft--;
					p->BeginPass(p->m_pSegment->m_mtLoopStart, p->m_nPassEnd);
				}
				bool bStop = p->m_nStopTime >= 0 && p->m_nStopTime <= m_nTime;
				if (bStop || p->m_nPassEnd <= m_nTime)
				{
					for (CMpVoice* v = m_pVoices; v != LTNULL; v = v->m_pNext)
					{
						if (v->m_pOwner == p)
						{
							if (bStop) v->FadeOut();
							v->m_pOwner = LTNULL;
						}
					}
					*pp = p->m_pNext;
					delete p;
					continue;
				}
				pp = &p->m_pNext;
			}

			// the block ends at the next event
			__int64 nEnd = m_nTime + n;
			if (m_nStopTime >= 0 && m_nStopTime < nEnd) nEnd = m_nStopTime;
			if (m_pPending && m_pPending->m_nStart < nEnd) nEnd = m_pPending->m_nStart;
			if (m_pPrimary && m_pPrimary->m_nPassEnd < nEnd) nEnd = m_pPrimary->m_nPassEnd;
			CMpSegState* q;
			for (q = m_pSecondaries; q != LTNULL; q = q->m_pNext)
			{
				if (q->m_nPassEnd < nEnd) nEnd = q->m_nPassEnd;
				if (q->m_nStopTime >= 0 && q->m_nStopTime < nEnd) nEnd = q->m_nStopTime;
			}
			if (nEnd <= m_nTime)
				nEnd = m_nTime + 1;
			n = (uint32)(nEnd - m_nTime);

			if (m_pPrimary)
				Schedule(m_pPrimary, nEnd);
			for (q = m_pSecondaries; q != LTNULL; q = q->m_pNext)
			{
				if (q->m_nStart < nEnd)
					Schedule(q, nEnd);
			}

			// the curves, at the end of the block
			CMpCurveState** pc = &m_pCurves;
			while (*pc)
			{
				CMpCurveState* c = *pc;
				if (c->m_nStart >= nEnd)
				{
					pc = &c->m_pNext;
					continue;
				}
				m_afCC[c->m_nPChannel][c->m_nCC] = c->ValueAt(nEnd);
				if (c->m_nEnd <= nEnd || c->m_nShape == DMUS_CURVES_INSTANT)
				{
					*pc = c->m_pNext;
					delete c;
					continue;
				}
				pc = &c->m_pNext;
			}

			// the voices
			CMpVoice** pv = &m_pVoices;
			while (*pv)
			{
				CMpVoice* v = *pv;
				bool bKeep = !v->m_bDone;
				if (bKeep && v->m_nStart < nEnd)
				{
					float fVol = m_afCC[v->m_nPChannel][0] * (1.0f / 127.0f);
					float fExp = m_afCC[v->m_nPChannel][1] * (1.0f / 127.0f);
					float g = v->m_fGain * fVol * fVol * fExp * fExp;
					float g0 = (v->m_fLastGain < 0.0f) ? g : v->m_fLastGain;
					uint32 nOff = (v->m_nStart > m_nTime) ? (uint32)(v->m_nStart - m_nTime) : 0;
					bKeep = v->Mix(s_afAcc + nOff * 2, n - nOff, g0, g, m_nTime + nOff);
					v->m_fLastGain = g;
				}
				if (!bKeep)
				{
					*pv = v->m_pNext;
					delete v;
					continue;
				}
				pv = &v->m_pNext;
			}
			m_nTime += n;
		}

		// the master volume and the output
		float fTarget = bPlay ? m_fMasterTarget : 0.0f;
		float fStep = 1.0f / MP_FADE_SAMPLES;
		short* pDst = pOut + nDone * 2;
		for (uint32 i = 0; i < n; i++)
		{
			if (m_fMasterGain < fTarget) { m_fMasterGain += fStep * (fTarget > 1.0f ? fTarget : 1.0f); if (m_fMasterGain > fTarget) m_fMasterGain = fTarget; }
			else if (m_fMasterGain > fTarget) { m_fMasterGain -= fStep * (m_fMasterGain > 1.0f ? m_fMasterGain : 1.0f); if (m_fMasterGain < fTarget) m_fMasterGain = fTarget; }
			for (int ch = 0; ch < 2; ch++)
			{
				float f = s_afAcc[i * 2 + ch] * m_fMasterGain;
				if (f > 32767.0f) f = 32767.0f;
				if (f < -32768.0f) f = -32768.0f;
				pDst[i * 2 + ch] = (short)f;
			}
		}
		nDone += n;
	}
	LeaveCriticalSection(&m_cs);
}

// ----------------------------------------------------------------------- //
//  The output: DirectSound
// ----------------------------------------------------------------------- //

bool CModernMusicPlayer::StartOutput()
{
	m_pMixBuffer = new short[MP_MIX_MAX_FRAMES * 2];
	if (m_bNoOutput)
		return true;

	HRESULT hr = CoCreateInstance(CLSID_DirectSound8, LTNULL, CLSCTX_INPROC_SERVER, IID_IDirectSound8, (void**)&m_pDirectSound);
	if (SUCCEEDED(hr))
		hr = m_pDirectSound->Initialize(LTNULL);
	if (SUCCEEDED(hr))
	{
		// the engine's window, as SndDrv.dll
		HWND hWnd = FindWindowA("LithTech", LTNULL);
		if (hWnd == LTNULL) hWnd = GetForegroundWindow();
		if (hWnd == LTNULL) hWnd = GetDesktopWindow();
		hr = m_pDirectSound->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
	}
	if (SUCCEEDED(hr))
	{
		WAVEFORMATEX wfx;
		memset(&wfx, 0, sizeof(wfx));
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = 2;
		wfx.nSamplesPerSec = MP_RATE;
		wfx.wBitsPerSample = 16;
		wfx.nBlockAlign = 4;
		wfx.nAvgBytesPerSec = MP_RATE * 4;
		m_nBufferBytes = MP_RATE * 4;	// one second
		DSBUFFERDESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.dwSize = sizeof(desc);
		desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
		desc.dwBufferBytes = m_nBufferBytes;
		desc.lpwfxFormat = &wfx;
		hr = m_pDirectSound->CreateSoundBuffer(&desc, &m_pBuffer, LTNULL);
	}
	if (SUCCEEDED(hr))
	{
		void* p1; void* p2; DWORD n1, n2;
		if (SUCCEEDED(m_pBuffer->Lock(0, m_nBufferBytes, &p1, &n1, &p2, &n2, 0)))
		{
			memset(p1, 0, n1);
			if (p2) memset(p2, 0, n2);
			m_pBuffer->Unlock(p1, n1, p2, n2);
		}
		hr = m_pBuffer->Play(0, 0, DSBPLAY_LOOPING);
	}
	if (SUCCEEDED(hr))
	{
		DWORD nPlay, nWrite;
		m_pBuffer->GetCurrentPosition(&nPlay, &nWrite);
		m_nWritePos = nWrite & ~3;
		m_hExit = CreateEvent(LTNULL, TRUE, FALSE, LTNULL);
		DWORD nId;
		m_hThread = CreateThread(LTNULL, 0, ThreadBootstrap, this, 0, &nId);
		if (m_hThread == LTNULL)
			hr = E_FAIL;
		else
			SetThreadPriority(m_hThread, THREAD_PRIORITY_ABOVE_NORMAL);
	}
	{
		char sLine[160];
		LTSNPrintF(sLine, sizeof(sLine), "LTDirectMusic: modern music player: DirectSound output %s (0x%08lx), %d Hz\n",
			SUCCEEDED(hr) ? "started" : "failed", (unsigned long)hr, MP_RATE);
		OutputDebugStringA(sLine);
		if (FAILED(hr))
			LTDMConOutError(sLine);
	}
	return SUCCEEDED(hr);
}

void CModernMusicPlayer::StopOutput()
{
	if (m_hThread)
	{
		SetEvent(m_hExit);
		WaitForSingleObject(m_hThread, 2000);
		FILETIME ftCreate, ftExit, ftKernel, ftUser;
		if (GetThreadTimes(m_hThread, &ftCreate, &ftExit, &ftKernel, &ftUser))
		{
			// (for a debugger, or Wine's +debugstr: the cost of the mix)
			__int64 nCpu = ((__int64)ftKernel.dwHighDateTime << 32 | ftKernel.dwLowDateTime) +
				((__int64)ftUser.dwHighDateTime << 32 | ftUser.dwLowDateTime);
			Mp_Debug("LTDirectMusic: modern music player: %d s mixed, %d ms of CPU, %u late writes\n",
				(int)(m_nTime / MP_RATE), (int)(nCpu / 10000), m_nUnderruns);
		}
		CloseHandle(m_hThread);
		m_hThread = LTNULL;
	}
	if (m_hExit)
	{
		CloseHandle(m_hExit);
		m_hExit = LTNULL;
	}
	if (m_pBuffer)
	{
		m_pBuffer->Stop();
		m_pBuffer->Release();
		m_pBuffer = LTNULL;
	}
	if (m_pDirectSound)
	{
		m_pDirectSound->Release();
		m_pDirectSound = LTNULL;
	}
	delete [] m_pMixBuffer;
	m_pMixBuffer = LTNULL;
}

DWORD WINAPI CModernMusicPlayer::ThreadBootstrap(void* pData)
{
	((CModernMusicPlayer*)pData)->Thread();
	return 0;
}

void CModernMusicPlayer::Thread()
{
	const uint32 nLead = (MP_RATE * MP_LEAD_MS / 1000) * 4;
	const uint32 nMargin = (MP_RATE * 20 / 1000) * 4;

	while (WaitForSingleObject(m_hExit, 5) == WAIT_TIMEOUT)
	{
		DWORD nPlay, nWrite;
		if (FAILED(m_pBuffer->GetCurrentPosition(&nPlay, &nWrite)))
			continue;
		uint32 nQueued = (m_nWritePos + m_nBufferBytes - nPlay) % m_nBufferBytes;
		uint32 nSafe = (nWrite + m_nBufferBytes - nPlay) % m_nBufferBytes;
		if (nQueued < nSafe || nQueued > m_nBufferBytes - nMargin)
		{
			// the play cursor passed what was written: start again after
			// the write cursor
			m_nWritePos = nWrite & ~3;
			nQueued = nSafe;
			m_nUnderruns++;
		}
		uint32 nTarget = nLead;
		if (nTarget < nSafe + nMargin)
			nTarget = nSafe + nMargin;
		if (nQueued >= nTarget)
			continue;
		uint32 nBytes = (nTarget - nQueued) & ~3;
		if (nBytes > MP_MIX_MAX_FRAMES * 4)
			nBytes = MP_MIX_MAX_FRAMES * 4;
		Mix(m_pMixBuffer, nBytes / 4);

		void* p1; void* p2; DWORD n1, n2;
		HRESULT hr = m_pBuffer->Lock(m_nWritePos, nBytes, &p1, &n1, &p2, &n2, 0);
		if (hr == DSERR_BUFFERLOST)
		{
			m_pBuffer->Restore();
			hr = m_pBuffer->Lock(m_nWritePos, nBytes, &p1, &n1, &p2, &n2, 0);
		}
		if (FAILED(hr))
			continue;
		memcpy(p1, m_pMixBuffer, n1);
		if (p2)
			memcpy(p2, (uint8*)m_pMixBuffer + n1, n2);
		m_pBuffer->Unlock(p1, n1, p2, n2);
		m_nWritePos = (m_nWritePos + nBytes) % m_nBufferBytes;

		// silence after what was written, so that a late mix does not play
		// old music again
		uint32 nClear = (MP_RATE / 10) * 4;
		if (SUCCEEDED(m_pBuffer->Lock(m_nWritePos, nClear, &p1, &n1, &p2, &n2, 0)))
		{
			memset(p1, 0, n1);
			if (p2) memset(p2, 0, n2);
			m_pBuffer->Unlock(p1, n1, p2, n2);
		}
	}
}
