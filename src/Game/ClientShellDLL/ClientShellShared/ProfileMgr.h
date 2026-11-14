// ----------------------------------------------------------------------- //
//
// MODULE  : ProfileMgr.h
//
// PURPOSE : Manages player profiles
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef PROFILE_MGR_H
#define PROFILE_MGR_H

#pragma warning( disable : 4786 )
#include <string>			// For strings

#include "ClientUtilities.h"
#include "PerformanceMgr.h"
#include "ButeMgr.h"
#include "SkillsButeMgr.h"
#include "NetDefs.h"
#include "ProfileUtils.h"

class CProfileMgr;
extern CProfileMgr* g_pProfileMgr;


const int NUM_DEVICES = 3;
const int MAX_NUM_COMMANDS = 50;
const int kMaxDeviceAxis = 6;
const int kMaxDevicePOV = 4;

enum JoystickAxisBindings
{
	eJoystick_None,
	eJoystick_Look,
	eJoystick_Move,
	eJoystick_LookInvert,
};

const int MUSIC_MIN_VOL		= -2500;
const int MUSIC_MAX_VOL		= 5000;
const int MUSIC_SLIDER_INC	= 250;
const int MUSIC_DEFAULT_VOL	= 2500;

const int SOUND_MIN_VOL		= 0;
const int SOUND_MAX_VOL		= 100;
const int SOUND_SLIDER_INC	= 10;
const int SOUND_DEFAULT_VOL	= 75;
const int SPEECH_MIN_VOL	= 0;
const int SPEECH_MAX_VOL	= 100;
const int SPEECH_SLIDER_INC	= 10;
const int SPEECH_DEFAULT_VOL = 100;

const float WEAPONS_DEFAULT_MULTIPLIER = 1.0f;
const float SPEECH_DEFAULT_MULTIPLIER = 1.0f;
const float DEFAULT_DEFAULT_MULTIPLIER = 0.75f;

const float	kMinGamma		= 0.5f;
const float	kMaxGamma		= 6.0f;

extern const int g_kNumCommands;

//section flags
const uint8 PROFILE_CONTROLS	= 0x01;
const uint8 PROFILE_MULTI		= 0x02;
const uint8 PROFILE_GAME		= 0x04;
const uint8 PROFILE_CROSSHAIR	= 0x08;
const uint8 PROFILE_SOUND		= 0x10;
const uint8 PROFILE_ALL			= 0xFF;


enum CommandType
{
	COM_MOVE,
	COM_INV,
	COM_VIEW,
	COM_MISC,
	kNumCommandTypes
};

struct CommandData
{
	int		nStringID;
	int		nCommandID;
	int		nActionStringID;
	int		nCommandType;
};

char*				GetCommandName(int nCommand);
const CommandData*	GetCommandData(int nIndex);
inline uint16 GetMinimumBandwidth() {return (g_BandwidthServer[0]/2);}
inline uint16 GetMaximumBandwidth() {return (g_BandwidthClient[eBandwidth_Custom-1]);}
inline bool IsValidBandwidth(uint16 nBandwidth) 
{
	return (nBandwidth >= GetMinimumBandwidth() && nBandwidth <= GetMaximumBandwidth());
}



class CBindingData
{
public:
	CBindingData()	{ nStringID = 0; nAction = 0; memset(strRealName, 0, sizeof(strRealName));
						memset(strTriggerName, 0, sizeof(strTriggerName));	}

    uint32      nStringID;
	int			nAction;
	char		strTriggerName[NUM_DEVICES][64];
	char		strRealName[NUM_DEVICES][64];
	uint32		nDeviceObjectId[NUM_DEVICES];
	uint16		nCode;
};


class CDeviceAxisData
{
public:
	CDeviceAxisData()	{Init("",0,0.0f,0.0f);}
	void Init(char *sName, uint32 nType, float fRangeLow, float fRangeHigh)
		{ SAFE_STRCPY(m_sName,sName); m_nType = nType; m_fRangeLow = fRangeLow; m_fRangeHigh = fRangeHigh; }
	char	m_sName[INPUTNAME_LEN];
    uint32  m_nType;
	float	m_fRangeLow;
	float	m_fRangeHigh;
};

class CDevicePOVData
{
public:
	CDevicePOVData()	{Init("");}
	void Init(char *sName)	{ SAFE_STRCPY(m_sName,sName); }
	char	m_sName[INPUTNAME_LEN];
};



class CUserProfile
{
public:
	CUserProfile();

	LTBOOL		Init(const std::string& profileName, LTBOOL bCreate);
	LTBOOL		RestoreDefaults(uint8 nFlags);
	void		Term();

	//take data from profile and apply them to the game
	void		ApplyControls();
	void		ApplyBindings();
	void		ApplyMouse();
	void		ApplyKeyboard();
	void		ApplyJoystick();

	void		ApplyMultiplayer(bool bLAN);

	void		ApplyGameOptions();
	void		ApplyCrosshair();

	void		ApplySound();
	void		ImplementSoundVolume();
	void		ImplementMusicVolume();

	void		ApplyDisplay();

	void		SetPerformanceCfg(char *pszCfg);
	void		ApplyPerformance(bool bForceDisplay);  //this may affect sound and display settings as well
	void		SendPerformanceMsg(); 

	//read data from the game and save them in the profile
	void		SetControls();
	void		SetBindings();
	void		SetMouse();
	void		SetKeyboard();
	void		SetJoystick();

	void		SetMultiplayer();

	void		SetGameOptions();
	void		SetCrosshair();

	void		SetSound();

	void		SetDisplay();

	void		SetPerformance();

	// load/save profile to file
	void		Load( );
	// RETAIL: no argument in Tron (its callers push none: CScreenJoystick's
	// OnFocus, retail 0x10065787); NOLF2 1.x added bForceClose
	void		Save();

	LTBOOL		IsInitted() {return m_bInitted;}

	CBindingData* FindBinding(int commandIndex);
	char const* GetDeviceName( uint32 nDeviceIndex );

	std::string		m_sName;

	//****************** controls
	//mouse
    LTBOOL		m_bInvertY;
    LTBOOL		m_bMouseLook;
	int			m_nInputRate;
	int			m_nSensitivity;

	//keyboard
    LTBOOL      m_bAutoCenter;
	int			m_nTronUnknown20;	// RETAIL: Tron's (+0x20, 0 in the constructor 0x100ec8a0)
	int			m_nNormalTurn;
	int	  		m_nFastTurn;
	int			m_nLookUp;

	//shared
	int	  		m_nVehicleTurn;

	//joystick
	LTBOOL				m_bUseJoystick;
	uint8				m_nAxis[kMaxDeviceAxis];
	uint8				m_nPOV[kMaxDevicePOV];


	//****************** multiplayer
	std::string		m_sPlayerName;
	std::string		m_sPlayerGuid;
	// RETAIL: the name of the host option set in use ("ServerConfig"; retail
	// +0x5c): Tron keeps the sets in CServerOptionMgr, not in the profile
	std::string		m_sServerOptions;
	uint8			m_nCPPlayerModel;
	uint8			m_nDMPlayerModel;
	uint8			m_nPlayerSkills[kNumSkills];
	uint8			m_nBandwidthClient;
	uint16			m_nBandwidthClientCustom;
	bool			m_bMPRadar;
	uint16			m_nClientPort;

	// RETAIL: Tron's light cycle settings (the light cycle screens',
	// CBaseScreenLightCycle 0x1000a3a0, 0x1000a560, 0x1000a600, 0x1000a660;
	// the names are ours). Retail keeps them at +0x200 (the light cycle
	// player name), +0x20c and +0x210 (the cycle's shade style and shade)
	// +0x214 (the names of the circuits completed), +0x220 (the names of
	// the cycle colors unlocked) and +0x22c (the names of the power-ups
	// unlocked). PORT: ours sit
	// here, in the space below, until the profile is laid out as retail's
	// (they are not loaded or saved yet; two bytes keep them aligned)
	char			m_aTronPadAlign[2];
	std::string		m_sLightCycleName;
	int				m_nLightCycleStyle;
	int				m_nLightCycleShade;
	StringArray		m_CompletedCircuits;
	StringArray		m_UnlockedCycleColors;
	StringArray		m_UnlockedPowerups;

	// PORT: what retail keeps where NOLF2 had its server game options is
	// not yet laid out (the constructor, 0x100ec8a0, has two more strings,
	// two vectors and six ints); this keeps the size retail's
	char			m_aTronPadServerOptions[sizeof(ServerGameOptions) - sizeof(std::string) - sizeof(int)
						- 2 - sizeof(std::string) - 2 * sizeof(int) - 3 * sizeof(StringArray)];

	//****************** game options
    uint8	            m_nSubtitles;
    bool				m_bGore;
	uint8				m_nDifficulty;
	LTBOOL				m_bAlwaysRun;
	uint8				m_nLayout;
	int					m_nHeadBob;
	int					m_nWeaponSway;
	int					m_nMsgDur;
	bool				m_bSPRadar;
	LTBOOL				m_bAutoWeaponSwitch;
	LTBOOL				m_bLoadScreenTips;
	LTBOOL				m_bVehicleContour;

	//crosshair
    LTBOOL				m_bCrosshair;
	uint8				m_CrosshairR;
	uint8				m_CrosshairG;
	uint8				m_CrosshairB;
	uint8				m_CrosshairA;
	uint8				m_nStyle;
    LTBOOL				m_bDynamic;



	//****************** sound
	int		m_nSoundVolume;
	int		m_nMusicVolume;
    LTBOOL  m_bSoundQuality;
    LTBOOL  m_bMusicQuality;
	float	m_fDefaultSoundMultiplier;
	float	m_fWeaponsSoundMultiplier;
	float	m_fSpeechSoundMultiplier;

	//****************** display
	LTBOOL	m_bHardwareCursor;
	LTBOOL	m_bVSync;
	uint32	m_nScreenWidth;
	uint32	m_nScreenHeight;
	uint32	m_nScreenDepth;
	float	m_fGamma;

	//****************** display
	sPerformCfg m_sPerformance;

	// RETAIL: Tron's profile has no weapon bindings (build 51's
	// m_nWpnActions): its CUserProfile is 0x6310 bytes, its bindings at
	// +0x248 (constructor 0x100ec8a0)

private:
	//load parts of profile to file (file must already have been parsed by m_buteMgr)
	void		LoadControls();
	void		LoadMultiplayer();
	void		LoadGameOptions();
	void		LoadSound(bool bApply); //optionlly defer applying these settings
	void		LoadPerformance(bool bApply); //optionlly defer applying these settings

	//save parts of profile to file
	void		SaveControls();
	void		SaveMultiplayer();
	void		SaveGameOptions();
	void		SaveSound();
	void		SavePerformance();



private:
	int				m_nNumBindings;
	CBindingData	m_bindings[MAX_NUM_COMMANDS];


	LTBOOL		m_bInitted;
	CButeMgr	m_buteMgr;
	char*		m_pCryptKey;
	// RETAIL: Tron's profile keeps no save version (NOLF2's m_nSaveVersion
	// and its "Version" section are later): CUserProfile ends at +0x630c,
	// where CProfileMgr's axis count is (0x100f00d3)
};




class CProfileMgr
{
public:
	LTBOOL			Init();
	void			Term();

	void			GetProfileList(StringSet& profileList);

	//saves current profile and loads specified one (creating it if necessary)
	void			NewProfile(const std::string& profileName);
	void			DeleteProfile(const std::string& profileName);
	void			RenameProfile(const std::string& oldName,const std::string& newName);
	CUserProfile*	GetCurrentProfile() {return &m_profile;}
	const char*		GetCurrentProfileName() {return m_profile.m_sName.c_str();}

	void			ClearBindings();
	uint32			GetControlType(uint32 deviceType, uint32 nObjectId );

	int				GetNumAxis() {return m_nNumDeviceAxis;}
	CDeviceAxisData* GetAxisData(int ndx);

	int				GetNumPOV() {return m_nNumDevicePOV;}
	CDevicePOVData* GetPOVData(int ndx);

	void			ImplementSoundVolume() {m_profile.ImplementSoundVolume();}
	void			ImplementMusicVolume() {m_profile.ImplementMusicVolume();}

private:
	void			SetDeviceData();

	CUserProfile	m_profile;

	// Information about each joystick axis
	int				m_nNumDeviceAxis;
	CDeviceAxisData	m_aDeviceAxisData[kMaxDeviceAxis];

	// Information about each joystick POV
	int				m_nNumDevicePOV;
	CDevicePOVData	m_aDevicePOVData[kMaxDevicePOV];
};


LTBOOL IsRendererEqual(RMode *pRenderer1, RMode *pRenderer2);

// Gets the default session name.
bool GetDefaultSessionName( GameType eGameType, char* pszSessionName, uint32 nSessionNameLen );

#endif	// PROFILE_MGR_H
