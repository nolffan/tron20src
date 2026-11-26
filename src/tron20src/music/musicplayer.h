/*
 * musicplayer.h -- the modern build's own music player (MODERN.md X-3).
 *
 * Tron 2.0's music is DirectMusic segments (MUSIC/<set>/*.sgt) that hold
 * wave tracks with MP3 waves, tempo tracks, time signature tracks and, in
 * some segments, volume and expression curves. Under Wine and Proton no
 * DirectMusic plays them (MODERN.md X-3). This player reads the level's
 * music control file, the segments and their waves itself, mixes the waves
 * at their times and plays the mix through a DirectSound streaming buffer.
 *
 * CLTDirectMusicMgr (ltdirectmusic_impl.cpp) gives each call of the
 * ILTDirectMusicMgr interface to this player when DirectMusic did not
 * start, or when the console variable MusicPlayer is 1. On Windows,
 * DirectMusic starts, and the player is not used.
 *
 * This file and musicplayer.cpp are #included in ltdirectmusic_impl.cpp,
 * in TRON20SRC_MODERN blocks, so that Lithtech.dsp does not change.
 *
 * Modern build only (TRON20SRC_MODERN).
 */
#ifndef __TRON20SRC_MUSICPLAYER_H__
#define __TRON20SRC_MUSICPLAYER_H__

struct IDirectSound8;
struct IDirectSoundBuffer;
class CMpWave;
class CMpSegment;
class CMpSegState;
class CMpVoice;
class CMpCurveState;

// the output of the player
#define MP_RATE		44100

class CModernMusicPlayer
{
public:
	CModernMusicPlayer();
	~CModernMusicPlayer();

	// the ILTDirectMusicMgr calls (iltdirectmusic.h), with the same
	// meanings and return values as CLTDirectMusicMgr's
	LTRESULT Init();
	LTRESULT Term();
	LTRESULT InitLevel(const char* sWorkingDirectory, const char* sControlFileName,
		const char* sDefine1, const char* sDefine2, const char* sDefine3);
	LTRESULT TermLevel();
	LTRESULT Play();
	LTRESULT Stop(const LTDMEnactTypes nStart);
	LTRESULT Pause(const LTDMEnactTypes nStart);
	LTRESULT UnPause();
	LTRESULT SetVolume(const long nVolume);
	LTRESULT ChangeIntensity(const int nNewIntensity, const LTDMEnactTypes nStart);
	LTRESULT PlaySecondary(const char* sSecondarySegment, const LTDMEnactTypes nStart);
	LTRESULT StopSecondary(const char* sSecondarySegment, const LTDMEnactTypes nStart);
	LTRESULT PlayMotif(const char* sStyleName, const char* sMotifName, const LTDMEnactTypes nStart);
	LTRESULT StopMotif(const char* sStyleName, const char* sMotifName, const LTDMEnactTypes nStart);
	int GetCurIntensity() { return m_nCurIntensity; }
	int GetNumIntensities() { return m_nNumIntensities; }
	int GetInitialIntensity() { return m_nInitialIntensity; }
	int GetInitialVolume() { return m_nInitialVolume; }
	int GetVolumeOffset() { return m_nVolumeOffset; }

	// mix nFrames stereo frames of 16-bit PCM at MP_RATE into pOut; the
	// mixer thread calls it, and the offline test (no DirectSound)
	void Mix(short* pOut, uint32 nFrames);

	// (the offline test: start without DirectSound)
	bool m_bNoOutput;

	// a wave of the level, loaded once (the segments' parser calls it)
	CMpWave* LoadWave(const char* sName);

private:
	// the command queue of an intensity (as CLTDirectMusicMgr's)
	enum { MP_CMD_PLAY = 1, MP_CMD_LOOP, MP_CMD_CHANGE };
	struct CCommand
	{
		int nType;
		CMpSegment* pSegment;
		int nVal;
	};
	struct CIntensity
	{
		CMpSegment** apSegments;
		int nSegments;
		int nLoops;
		int nNext;
	};
	struct CTransition
	{
		LTDMEnactTypes nEnact;
		bool bManual;
		CMpSegment* pSegment;
	};

	// level data
	bool ReadControlFile(CControlFileMgr& controlFile);
	CMpSegment* LoadSegment(const char* sName);
	bool ReadFile(const char* sName, uint8*& pData, uint32& nLen);
	void FreeLevel();
	CTransition* GetTransition(int nFrom, int nTo);

	// the timeline (all called with m_cs held)
	LTRESULT ChangeIntensityLocked(int nNewIntensity, LTDMEnactTypes nStart, bool bFromQueue);
	void StopLocked(LTDMEnactTypes nStart);
	__int64 EnactTime(LTDMEnactTypes nEnact, CMpSegment* pNew);
	void ClearCommands();
	void AddCommand(int nType, CMpSegment* pSegment, int nVal);
	void RunQueue();
	void StartPending();
	void StopAllNow(bool bFade);
	void Schedule(CMpSegState* pState, __int64 nUntil);
	void FreeStates();

	// the output
	bool StartOutput();
	void StopOutput();
	static DWORD WINAPI ThreadBootstrap(void* pData);
	void Thread();

	CRITICAL_SECTION m_cs;
	bool m_bInitialized;
	bool m_bLevelInitialized;
	bool m_bCOM;

	// the control file
	char m_sDirectory[_MAX_PATH];
	int m_nNumIntensities;
	int m_nInitialIntensity;
	int m_nInitialVolume;
	int m_nVolumeOffset;
	CIntensity* m_aIntensities;
	CTransition* m_aTransitions;
	int m_nNumTransitions;
	CMpSegment* m_pSegments;
	CMpWave* m_pWaves;

	// the state of the intensities (CLTDirectMusicMgr's)
	int m_nCurIntensity;
	int m_nPrevIntensity;
	CCommand* m_aCommands;
	int m_nCommands;
	int m_nCommandsAlloc;
	int m_nLastCommand;		// -1: none
	uint32 m_nPaused;

	// the timeline, in samples at MP_RATE from the start of the player
	__int64 m_nTime;
	CMpSegState* m_pPrimary;
	CMpSegState* m_pPending;	// the next primary segment, at its start time
	CMpSegState* m_pSecondaries;
	__int64 m_nStopTime;		// -1: none
	double m_dTempo;			// the tempo of the performance
	CMpVoice* m_pVoices;
	CMpCurveState* m_pCurves;
	float m_afCC[256][2];		// each PChannel's volume (CC 7) and expression (CC 11), 0 to 127
	float m_fMasterGain;
	float m_fMasterTarget;

	// the output
	IDirectSound8* m_pDirectSound;
	IDirectSoundBuffer* m_pBuffer;
	uint32 m_nBufferBytes;
	uint32 m_nWritePos;
	HANDLE m_hThread;
	HANDLE m_hExit;
	short* m_pMixBuffer;
	uint32 m_nUnderruns;
};

#endif // __TRON20SRC_MUSICPLAYER_H__
