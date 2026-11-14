// ----------------------------------------------------------------------- //
//
// MODULE  : ProfileUtils.h
//
// PURPOSE : Profile utilities
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef PROFILEUTILS_H
#define PROFILEUTILS_H

#include "ltbasetypes.h"
#include "NetDefs.h"
#include "TeamMgr.h"
#include <string>

class CButeMgr;

#define DEFAULT_CAMPAIGN	".default"
#define DEFAULT_CAMPAIGN_FILE	"default_campaign.txt"
#define PROFILE_DIR "Profiles\\"
#define PROFILE_EXT ".txt"

#define MAX_PROFILE_NAME	16

// PORT: ours. NOLF2's per game type options, restriction sets and campaign
// files are only for the NOLF2 code in Object.lto and CShell.dll still standing
// in for Tron's; Tron's dedicated server (which defines neither build) has
// none of them (TRONSrv.exe has no static options to construct, and no
// campaign directories).
#if defined( _CLIENTBUILD ) || defined( _SERVERBUILD )
#define PROFILEUTILS_NOLF2_OPTIONS
#endif


class CoopGameOptions
{
	public:

		CoopGameOptions( );

		CoopGameOptions& Copy( CoopGameOptions const& other );

		void	Clear( );

		bool	LoadFromBute( CButeMgr& bute );
		bool	SaveToBute( CButeMgr& bute );

		// for transmitting options that may change mid game
		void	Write( ILTMessage_Write *pMsg );
		void	Read( ILTMessage_Read *pMsg );

		std::string		m_sSessionName;
		std::string		m_sCampaignName;

		uint8			m_nMaxPlayers;
		bool			m_bUseSkills;
		bool			m_bFriendlyFire;
		uint8			m_nDifficulty;
		float			m_fPlayerDiffFactor;

};

class DMGameOptions
{
	public:

		DMGameOptions( );

		DMGameOptions& Copy( DMGameOptions const& other );

		void	Clear( );

		bool	LoadFromBute( CButeMgr& bute );
		bool	SaveToBute( CButeMgr& bute );

		// for transmitting options that may change mid game
		void	Write( ILTMessage_Write *pMsg );
		void	Read( ILTMessage_Read *pMsg );

		std::string		m_sSessionName;
		std::string		m_sCampaignName;

		uint8			m_nMaxPlayers;
		uint8			m_nRunSpeed;
		uint8			m_nScoreLimit;
		uint8			m_nTimeLimit;
		uint8			m_nRounds;

		uint8			m_nFragScore;
		uint8			m_nTagScore;
};

class TeamDMGameOptions
{
	public:

		TeamDMGameOptions( );

		TeamDMGameOptions& Copy( TeamDMGameOptions const& other );

		void	Clear( );

		bool	LoadFromBute( CButeMgr& bute );
		bool	SaveToBute( CButeMgr& bute );

		// for transmitting options that may change mid game
		void	Write( ILTMessage_Write *pMsg );
		void	Read( ILTMessage_Read *pMsg );

		std::string		m_sSessionName;
		std::string		m_sCampaignName;

		uint8			m_nMaxPlayers;
		uint8			m_nRunSpeed;
		uint8			m_nScoreLimit;
		uint8			m_nTimeLimit;
		uint8			m_nRounds;
		bool			m_bFriendlyFire;

		uint8			m_nNumTeams;	// Will this ever be anything but 2?
		uint32			m_nTeamModel[MAX_TEAMS];
		std::string		m_sTeamName[MAX_TEAMS];

		uint8			m_nFragScore;
		uint8			m_nTagScore;
		uint8			m_nRevivingScore;
};

class DoomsdayGameOptions
{
	public:

		DoomsdayGameOptions( );

		DoomsdayGameOptions& Copy( DoomsdayGameOptions const& other );

		void	Clear( );

		bool	LoadFromBute( CButeMgr& bute );
		bool	SaveToBute( CButeMgr& bute );

		// for transmitting options that may change mid game
		void	Write( ILTMessage_Write *pMsg );
		void	Read( ILTMessage_Read *pMsg );

		std::string		m_sSessionName;
		std::string		m_sCampaignName;

		uint8			m_nMaxPlayers;
		uint8			m_nRunSpeed;
		uint8			m_nTimeLimit;
		uint8			m_nRounds;
		bool			m_bFriendlyFire;

		uint8			m_nNumTeams;	// Will this ever be anything but 2?
		uint32			m_nTeamModel[MAX_TEAMS];
		std::string		m_sTeamName[MAX_TEAMS];

		uint8			m_nFragScore;
		uint8			m_nTagScore;

		uint8			m_nDeviceCompletedScore;
		uint8			m_nLightPiecePlacedScore;
		uint8			m_nHeavyPiecePlacedScore;
		uint8			m_nPieceRemovedScore;

		uint8			m_nRevivingScore;
};


// RETAIL: Tron's ServerGameOptions is its own (0xd0 bytes; retail Object.lto's
// constructor 0x10114530, Clear 0x10114690, Copy 0x101149d0; CShell.dll's
// SaveToBute 0x100e9ab0 names the keys): the common settings, then one block
// per multiplayer game type (disc tournament, disc arena, light cycles), each
// with its map list. Member names are ours, after the bute keys.
// PORT: NOLF2's per game type options (and its restriction sets) are kept as
// static members, so the NOLF2 code still standing in for Tron's compiles
// without their taking room in the layout.
struct qr2_keybuffer_s;	// GameSpy SDK (qr2.h)
struct qr2_buffer_s;

class ServerGameOptions
{
	public:

		ServerGameOptions( );

		ServerGameOptions( ServerGameOptions const& other )
		{
			Copy( other );
		}

		ServerGameOptions& operator=( ServerGameOptions const& other )
		{
			return Copy( other );
		}

		ServerGameOptions& Copy( ServerGameOptions const& other );

		void	Clear( );

		bool	LoadFromBute( CButeMgr& bute );
		void	SaveToBute( CButeMgr& bute );

		// RETAIL: TRONSrv.exe 0x408130 and 0x408fd0 (the names are ours)
		bool	Validate( );
		bool	FixSessionName( );

		// "Settings"
		std::string		m_sOptionsName;				// +0x00 ".Default" (ours)
		std::string		m_sSessionName;				// +0x0c "ServerName"
		std::string		m_sPassword;				// +0x18 "Password"
		std::string		m_sScmdPassword;			// +0x24 "SCMDPassword"
		uint8			m_eGameType;				// +0x30 "GameType" (a GameType)
		uint32			m_nPort;					// +0x34 "Port"
		uint8			m_nBandwidthServer;			// +0x38 "Bandwidth"
		uint32			m_nBandwidthServerCustom;	// +0x3c "BandwidthCustom"
		bool			m_bUsePassword;				// +0x40 "Locked"
		bool			m_bDedicated;				// +0x41 "Dedicated"
		bool			m_bLANOnly;					// +0x42 "LANOnly"
		bool			m_bAllowScmdCommands;		// +0x43 "AllowSCMD"
		std::string		m_sModName;					// +0x44 not saved; not yet known

		// "DiscTournament"
		uint8			m_nTournMaxPlayers;			// +0x50
		uint8			m_nTournMaxTeamSize;
		bool			m_bTournFriendlyFire;
		bool			m_bTournDiscsInSpectator;
		bool			m_bTournAllowAutoAim;
		uint8			m_nTournPointsPerMatch;
		uint8			m_nTournMatchTimeLimit;
		uint8			m_nTournTeamsToStart;
		uint32			m_nTournMode;				// +0x58 "TournamentMode"
		uint8			m_nTournGamesPerMap;		// +0x5c
		uint8			m_nTournPreGameDelay;
		uint16			m_nTournMaxPing;			// +0x5e
		uint8			m_nTournAirControl;			// +0x60
		StringArray		m_saTournMaps;				// +0x64 "Map%02d"

		// "DiscArena"
		uint8			m_nArenaMaxPlayers;			// +0x70
		uint8			m_nArenaMaxTeamSize;
		bool			m_bArenaFriendlyFire;
		bool			m_bArenaDiscsInSpectator;
		bool			m_bArenaAllowAutoAim;
		uint8			m_nArenaPointsPerMatch;
		uint8			m_nArenaMatchTimeLimit;
		uint8			m_nArenaWinLimit;			// +0x77
		uint8			m_nArenaTimeLimit;
		uint8			m_nArenaGamesPerMap;
		uint8			m_nArenaPreGameDelay;		// +0x7a
		uint16			m_nArenaMaxPing;			// +0x7c
		uint8			m_nArenaAirControl;			// +0x7e
		StringArray		m_saArenaMaps;				// +0x80

		// "LightCycles" ("Lightcycles" for its MaxPlayers and maps)
		uint8			m_nLCMaxPlayers;			// +0x8c
		StringArray		m_saLCMaps;					// +0x90
		uint32			m_nLCAllowedCycleType;		// +0x9c
		uint32			m_nLCGameSpeed;				// +0xa0
		uint32			m_nLCTrailLength;			// +0xa4
		uint8			m_nLCMaxTeamSize;			// +0xa8
		uint8			m_nLCWaitForPlayersTime;
		uint8			m_nLCTotalRaces;
		uint16			m_nLCTotalPoints;			// +0xac
		uint16			m_nLCTotalTime;
		uint16			m_nLCMaxPing;				// +0xb0
		uint32			m_nLCPowerups;				// +0xb4 "PowerupAllow*", a bit each (ProfileUtils.cpp's LCPowerupBits)
		int8			m_nLCKillPointValue;		// +0xb8
		int8			m_nLCSelfKillPointValue;
		int8			m_nLCWinRacePointValue;

		// Unsaved data.
		std::string		m_sProfileName;				// +0xbc
		bool			m_bPreCacheAssets;			// +0xc8
		bool			m_bPerformanceTest;			// +0xc9
		uint32			m_nTronUnknownCC;			// +0xcc RETAIL: retail 0x10114e50 returns it

		//for the current game type
#ifdef PROFILEUTILS_NOLF2_OPTIONS
		bool		IsDefaultCampaign() const;
		const char*	GetCampaignName() const;
		void		SetCampaignName(const char*	pszName);
#endif // PROFILEUTILS_NOLF2_OPTIONS
		const char*	GetSessionName() const;
		void		SetSessionName(const char*	pszName);
		
		uint8		GetMaxPlayers() const;
		int			GetGameTypeStringID() const;

		StringArray*	GetMapList( );
		// RETAIL: CShell.dll 0x100eb4d0: true if the game type has no map
		// list or its first map is empty (the name is ours)
		bool		HasNoMaps( );
		uint32		GetTronUnknownCC( ) const;
		// RETAIL: CShell.dll 0x100eb5a0: the light cycle game data manager
		// stores its current settings here (CLightCycleGameDataMgr::
		// SetCurrent). The name is ours
		void		SetTronUnknownCC( uint32 nValue );

#ifndef _CLIENTBUILD
		// RETAIL: Object.lto 0x10114e60, 0x10114f50, 0x101150a0: the GameSpy
		// keys of the game type, registered, listed and reported for the
		// server shell's query callbacks (the names are ours; nKeyType is a
		// qr2_key_type)
		void		RegisterGameSpyKeys( );
		void		AddGameSpyKeys( int nKeyType, qr2_keybuffer_s* pKeyBuffer );
		bool		GetGameSpyKeyValue( int nKeyId, qr2_buffer_s* pOutBuf );
#endif // _CLIENTBUILD

#ifdef PROFILEUTILS_NOLF2_OPTIONS
		CoopGameOptions&		GetCoop() { return m_Coop;	}
		DMGameOptions&			GetDeathmatch() { return m_DM; }
		TeamDMGameOptions&		GetTeamDeathmatch() { return m_TeamDM; }
		DoomsdayGameOptions&	GetDoomsday() { return m_DD; }

		// PORT: NOLF2's, not Tron's (see above)
		static StringSet		m_setRestrictedWeapons;
		static StringSet		m_setRestrictedAmmo;
		static StringSet		m_setRestrictedGear;

	private:

		static CoopGameOptions		m_Coop;
		static DMGameOptions		m_DM;
		static TeamDMGameOptions	m_TeamDM;
		static DoomsdayGameOptions	m_DD;
#endif // PROFILEUTILS_NOLF2_OPTIONS
};



const char*	GetProfileDir( char const* pszProfileName );
const char*	GetProfileFile( char const* pszProfileName );
#ifdef PROFILEUTILS_NOLF2_OPTIONS
const char*	GetCampaignDir( char const* pszProfileName, GameType eGameType );
const char*	GetCampaignFile( ServerGameOptions const& serverGameOptions );
#endif // PROFILEUTILS_NOLF2_OPTIONS


#endif // PROFILEUTILS_H

