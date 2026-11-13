#include "stdafx.h"
#include "ProfileUtils.h"
#include "ButeMgr.h"
#include "ltbasedefs.h"
#include "ModelButeMgr.h"
#include "CommonUtilities.h"
#include "ResShared.h"
#ifndef _CLIENTBUILD
#include "GameSpy/GameSpySDK/qr2/qr2.h"
#endif // _CLIENTBUILD
#ifdef _CLIENTBUILD
#include "ClientUtilities.h"
#include "ClientResShared.h"
#endif //_CLIENTBUILD


#define ARRAY_LEN(array) (sizeof((array)) / sizeof((array)[0]))
#define DEFAULT_SESSION ""

// RETAIL: a table nothing in the retail code reads, the first data of this
// file in every image that links it (TRONSrv.exe 0x41eee0, CShell.dll
// 0x1022ac8c, Object.lto 0x102f7608): the worlds of the disc and the light
// cycle games. The name is ours.
char const* g_apszTronUnknownWorlds[] =
{
	"Worlds\\RetailMultiPlayer\\DiscArena",
	"Worlds\\RetailLightCycle",
};

namespace
{

#ifdef _CLIENTBUILD
	int nDefTeamID[2] = {IDS_TEAM1_NAME,IDS_TEAM2_NAME};
#endif
}

#ifdef PROFILEUTILS_NOLF2_OPTIONS
// PORT: NOLF2's per game type options and restriction sets (see ProfileUtils.h)
StringSet				ServerGameOptions::m_setRestrictedWeapons;
StringSet				ServerGameOptions::m_setRestrictedAmmo;
StringSet				ServerGameOptions::m_setRestrictedGear;
CoopGameOptions			ServerGameOptions::m_Coop;
DMGameOptions			ServerGameOptions::m_DM;
TeamDMGameOptions		ServerGameOptions::m_TeamDM;
DoomsdayGameOptions		ServerGameOptions::m_DD;
#endif // PROFILEUTILS_NOLF2_OPTIONS

// RETAIL: Object.lto 0x10114530
ServerGameOptions::ServerGameOptions( )
{
	Clear( );
}

// RETAIL: Object.lto 0x10114690, CShell.dll 0x100e9740
void ServerGameOptions::Clear( )
{
	m_sOptionsName = ".Default";
	m_sSessionName = "";
	m_sPassword = "";
	m_sScmdPassword = "reindeerflotilla";

	m_eGameType = eGameTypeDiscArena;
	m_nPort = DEFAULT_PORT;
	m_nBandwidthServer = 3;
	m_nBandwidthServerCustom = g_BandwidthServer[m_nBandwidthServer];
	m_bUsePassword = false;
	m_bDedicated = false;
	m_bLANOnly = false;
	m_bAllowScmdCommands = false;
	m_sModName = "";

	m_nTournMaxPlayers = 8;
	m_nTournMaxTeamSize = 2;
	m_bTournFriendlyFire = false;
	m_bTournDiscsInSpectator = true;
	m_bTournAllowAutoAim = true;
	m_nTournPointsPerMatch = 3;
	m_nTournMatchTimeLimit = 5;
	m_nTournTeamsToStart = 2;
	m_nTournMode = 0;
	m_nTournGamesPerMap = 1;
	m_nTournPreGameDelay = 30;
	m_nTournMaxPing = 0;
	m_nTournAirControl = 50;
	m_saTournMaps.clear( );

	m_nArenaMaxPlayers = 8;
	m_nArenaMaxTeamSize = 2;
	m_bArenaFriendlyFire = false;
	m_bArenaDiscsInSpectator = true;
	m_bArenaAllowAutoAim = true;
	m_nArenaPointsPerMatch = 3;
	m_nArenaMatchTimeLimit = 5;
	m_nArenaWinLimit = 5;
	m_nArenaTimeLimit = 20;
	m_nArenaGamesPerMap = 1;
	m_nArenaPreGameDelay = 0;
	m_nArenaMaxPing = 0;
	m_nArenaAirControl = 50;
	m_saArenaMaps.clear( );

	m_nLCMaxPlayers = 8;
	m_saLCMaps.clear( );
	m_nLCTrailLength = 1;
	m_nLCGameSpeed = 1;
	m_nLCKillPointValue = 1;
	m_nLCMaxTeamSize = 2;
	m_nLCAllowedCycleType = 0;
	m_nLCWaitForPlayersTime = 0;
	m_nLCTotalRaces = 5;
	m_nLCTotalPoints = 0;
	m_nLCTotalTime = 0;
	m_nLCMaxPing = 80;
	m_nLCPowerups = 0xFFFFFFFF;
	m_nLCSelfKillPointValue = -1;
	m_nLCWinRacePointValue = 6;

	m_sProfileName = "";
	m_bPreCacheAssets = true;
	m_bPerformanceTest = false;
	m_nTronUnknownCC = 0;
}

static void StringSetToDelimitedString( StringSet const& sSet, std::string& sDelimitedString, char const* pszDelimiter )
{
	sDelimitedString.clear( );

	// Save set to a string delimted by commas.
	std::string sValue = "";
	StringSet::const_iterator iter = sSet.begin( );
	while( iter != sSet.end( ))
	{
		std::string const& sItem = *iter;
		sDelimitedString += sItem;
		sDelimitedString += pszDelimiter;

		iter++;
	}
}

static void DelimitedStringToStringSet( std::string const& sDelimitedString, StringSet& sSet, char const* pszDelimiter )
{
	sSet.clear( );

	// Find all items delimited by commas.
	uint32 nPos = 0;
	while( nPos < sDelimitedString.size( ))
	{
		uint32 nNextPos = sDelimitedString.find( ',', nPos );
		if( nNextPos == std::string::npos )
			nNextPos = sDelimitedString.size( );

		uint32 nNumChars = nNextPos - nPos;
		std::string sItem = sDelimitedString.substr( nPos, nNumChars );

		// Add the item to the set.
		if( !sItem.empty( ))
		{
			sSet.insert( sItem );
		}


		nPos = nNextPos + 1;
	}
}

#define MAX_MAPS_IN_LIST	32

// Reads a map list ("Map00", "Map01"... while they exist, at most
// MAX_MAPS_IN_LIST). RETAIL: TRONSrv.exe 0x408160, CShell.dll 0x100ea210: the
// loop is inline in LoadFromBute, once per game type (VC6 inlines no function
// with destructors to unwind, so it is a macro here, on LoadFromBute's szKey
// and nMap; the name is ours).
#define LOAD_MAP_LIST( pszTag, saMaps ) \
	{ \
		nMap = 0; \
		sprintf( szKey, "Map%02d", nMap ); \
		saMaps.clear( ); \
		while( nMap < MAX_MAPS_IN_LIST && bute.Exist( pszTag, szKey )) \
		{ \
			std::string sMap = bute.GetString( pszTag, szKey, "" ); \
			if( !sMap.empty( )) \
				saMaps.push_back( sMap ); \
			nMap++; \
			sprintf( szKey, "Map%02d", nMap ); \
		} \
	}

// Writes a map list, blanking the rest of the MAX_MAPS_IN_LIST keys.
// RETAIL: CShell.dll 0x100e9cfa: inline in SaveToBute, like LOAD_MAP_LIST.
#define SAVE_MAP_LIST( pszTag, saMaps ) \
	{ \
		nMap = 0; \
		StringArray::const_iterator iter = saMaps.begin( ); \
		for( ; iter != saMaps.end( ) && nMap < MAX_MAPS_IN_LIST; iter++, nMap++ ) \
		{ \
			sprintf( szKey, "Map%02d", nMap ); \
			bute.SetString( pszTag, szKey, iter->c_str( )); \
		} \
		for( ; nMap < MAX_MAPS_IN_LIST; nMap++ ) \
		{ \
			sprintf( szKey, "Map%02d", nMap ); \
			bute.SetString( pszTag, szKey, "" ); \
		} \
	}

// RETAIL: TRONSrv.exe 0x408130, CShell.dll 0x100e9a80: makes the options
// usable, true if they already were: no backslash in the session name and no
// more than 8 light cycle players. The name is ours.
bool ServerGameOptions::Validate( )
{
	bool bChanged = FixSessionName( );
	if( m_nLCMaxPlayers > 8 )
	{
		m_nLCMaxPlayers = 8;
		bChanged = true;
	}

	return !bChanged;
}

// ServerGameOptions::m_nLCPowerups's bits. RETAIL: LoadFromBute sets them as
// bit fields (TRONSrv.exe 0x408b22). The names are ours. PORT: the struct is
// here and m_nLCPowerups is read through it, where Tron's header more likely
// had the bit fields in the class: any new type in ProfileUtils.h moves VC6's
// stack slots in unrelated code that includes it (Object.lto's CSoundButeMgr
// destructor, 0x101b7e20, through the precompiled header).
struct LCPowerupBits
{
	uint32	bShield : 1;
	uint32	bTurbo : 1;
	uint32	bTrailSpike : 1;
	uint32	bMissile : 1;
	uint32	bWallExtender : 1;
	uint32	bTurboCurse : 1;
	uint32	bGrab : 1;
	uint32	bTrailReset : 1;
	uint32	bForceUse : 1;
};
#define m_LCPowerups ( ( LCPowerupBits& )m_nLCPowerups )

// RETAIL: CShell.dll 0x100ea210, TRONSrv.exe 0x408160. Tron's reads no
// "TournamentMode" (SaveToBute writes it; m_nTournMode keeps its default),
// and rebuilds the light cycle powerups from nothing: each flag's default is
// the bit just cleared, so a missing key leaves its powerup off. The result is
// whether the options read were already valid.
bool ServerGameOptions::LoadFromBute( CButeMgr& bute )
{
	char szKey[32];
	int nMap;

	m_sSessionName = bute.GetString( "Settings", "ServerName", m_sSessionName.c_str( ));
	m_sPassword = bute.GetString( "Settings", "Password", m_sPassword.c_str( ));
	m_sScmdPassword = bute.GetString( "Settings", "SCMDPassword", m_sScmdPassword.c_str( ));
	m_eGameType = ( uint8 )bute.GetInt( "Settings", "GameType", m_eGameType );
	m_nPort = bute.GetInt( "Settings", "Port", m_nPort );
	m_nBandwidthServer = ( uint8 )bute.GetInt( "Settings", "Bandwidth", m_nBandwidthServer );
	m_nBandwidthServerCustom = bute.GetInt( "Settings", "BandwidthCustom", m_nBandwidthServerCustom );
	m_bUsePassword = bute.GetInt( "Settings", "Locked", m_bUsePassword ) > 0;
	m_bDedicated = bute.GetInt( "Settings", "Dedicated", m_bDedicated ) > 0;
	m_bLANOnly = bute.GetInt( "Settings", "LANOnly", m_bLANOnly ) > 0;
	m_bAllowScmdCommands = bute.GetInt( "Settings", "AllowSCMD", m_bAllowScmdCommands ) > 0;

	m_nTournMaxPlayers = ( uint8 )bute.GetInt( "DiscTournament", "MaxPlayers", m_nTournMaxPlayers );
	m_nTournMaxTeamSize = ( uint8 )bute.GetInt( "DiscTournament", "MaxTeamSize", m_nTournMaxTeamSize );
	m_bTournFriendlyFire = bute.GetInt( "DiscTournament", "FriendlyFire", m_bTournFriendlyFire ) > 0;
	m_bTournDiscsInSpectator = bute.GetInt( "DiscTournament", "DiscsInSpectator", m_bTournDiscsInSpectator ) > 0;
	m_bTournAllowAutoAim = bute.GetInt( "DiscTournament", "AllowAutoAim", m_bTournAllowAutoAim ) > 0;
	m_nTournPointsPerMatch = ( uint8 )bute.GetInt( "DiscTournament", "PointsPerMatch", m_nTournPointsPerMatch );
	m_nTournMatchTimeLimit = ( uint8 )bute.GetInt( "DiscTournament", "MatchTimeLimit", m_nTournMatchTimeLimit );
	m_nTournTeamsToStart = ( uint8 )bute.GetInt( "DiscTournament", "TeamsToStart", m_nTournTeamsToStart );
	m_nTournGamesPerMap = ( uint8 )bute.GetInt( "DiscTournament", "GamesPerMap", m_nTournGamesPerMap );
	m_nTournPreGameDelay = ( uint8 )bute.GetInt( "DiscTournament", "PreGameDelay", m_nTournPreGameDelay );
	m_nTournMaxPing = ( uint16 )bute.GetInt( "DiscTournament", "MaxPing", m_nTournMaxPing );
	m_nTournAirControl = ( uint8 )bute.GetInt( "DiscTournament", "AirControl", m_nTournAirControl );
	LOAD_MAP_LIST( "DiscTournament", m_saTournMaps );

	m_nArenaMaxPlayers = ( uint8 )bute.GetInt( "DiscArena", "MaxPlayers", m_nArenaMaxPlayers );
	m_nArenaMaxTeamSize = ( uint8 )bute.GetInt( "DiscArena", "MaxTeamSize", m_nArenaMaxTeamSize );
	m_bArenaFriendlyFire = bute.GetInt( "DiscArena", "FriendlyFire", m_bArenaFriendlyFire ) > 0;
	m_bArenaDiscsInSpectator = bute.GetInt( "DiscArena", "DiscsInSpectator", m_bArenaDiscsInSpectator ) > 0;
	m_bArenaAllowAutoAim = bute.GetInt( "DiscArena", "AllowAutoAim", m_bArenaAllowAutoAim ) > 0;
	m_nArenaPointsPerMatch = ( uint8 )bute.GetInt( "DiscArena", "PointsPerMatch", m_nArenaPointsPerMatch );
	m_nArenaMatchTimeLimit = ( uint8 )bute.GetInt( "DiscArena", "MatchTimeLimit", m_nArenaMatchTimeLimit );
	m_nArenaWinLimit = ( uint8 )bute.GetInt( "DiscArena", "WinLimit", m_nArenaWinLimit );
	m_nArenaTimeLimit = ( uint8 )bute.GetInt( "DiscArena", "TimeLimit", m_nArenaTimeLimit );
	m_nArenaGamesPerMap = ( uint8 )bute.GetInt( "DiscArena", "GamesPerMap", m_nArenaGamesPerMap );
	m_nArenaPreGameDelay = ( uint8 )bute.GetInt( "DiscArena", "PreGameDelay", m_nArenaPreGameDelay );
	m_nArenaMaxPing = ( uint16 )bute.GetInt( "DiscArena", "MaxPing", m_nArenaMaxPing );
	m_nArenaAirControl = ( uint8 )bute.GetInt( "DiscArena", "AirControl", m_nArenaAirControl );
	LOAD_MAP_LIST( "DiscArena", m_saArenaMaps );

	// RETAIL: "Lightcycles" for the max players and maps, "LightCycles" for
	// the rest (the bute manager's tags are not case sensitive)
	m_nLCMaxPlayers = ( uint8 )bute.GetInt( "Lightcycles", "MaxPlayers", m_nLCMaxPlayers );
	m_nLCMaxTeamSize = ( uint8 )bute.GetInt( "LightCycles", "MaxTeamSize", m_nLCMaxTeamSize );
	m_nLCTrailLength = bute.GetInt( "LightCycles", "TrailLength", m_nLCTrailLength );
	m_nLCGameSpeed = bute.GetInt( "LightCycles", "GameSpeed", m_nLCGameSpeed );
	m_nLCAllowedCycleType = bute.GetInt( "LightCycles", "AllowedCycleType", m_nLCAllowedCycleType );
	m_nLCWaitForPlayersTime = ( uint8 )bute.GetInt( "LightCycles", "WaitForPlayersTime", m_nLCWaitForPlayersTime );
	m_nLCTotalRaces = ( uint8 )bute.GetInt( "LightCycles", "TotalRaces", m_nLCTotalRaces );
	m_nLCTotalPoints = ( uint16 )bute.GetInt( "LightCycles", "TotalPoints", m_nLCTotalPoints );
	m_nLCTotalTime = ( uint16 )bute.GetInt( "LightCycles", "TotalTime", m_nLCTotalTime );
	m_nLCMaxPing = ( uint16 )bute.GetInt( "LightCycles", "MaxPing", m_nLCMaxPing );
	m_nLCPowerups = 0;
	if( bute.GetBool( "LightCycles", "PowerupAllowShield", m_LCPowerups.bShield ))
		m_LCPowerups.bShield = 1;
	if( bute.GetBool( "LightCycles", "PowerupAllowTurbo", m_LCPowerups.bTurbo ))
		m_LCPowerups.bTurbo = 1;
	if( bute.GetBool( "LightCycles", "PowerupAllowTrailSpike", m_LCPowerups.bTrailSpike ))
		m_LCPowerups.bTrailSpike = 1;
	if( bute.GetBool( "LightCycles", "PowerupAllowMissile", m_LCPowerups.bMissile ))
		m_LCPowerups.bMissile = 1;
	if( bute.GetBool( "LightCycles", "PowerupAllowTurboCurse", m_LCPowerups.bTurboCurse ))
		m_LCPowerups.bTurboCurse = 1;
	if( bute.GetBool( "LightCycles", "PowerupAllowGrab", m_LCPowerups.bGrab ))
		m_LCPowerups.bGrab = 1;
	if( bute.GetBool( "LightCycles", "PowerupAllowTrailReset", m_LCPowerups.bTrailReset ))
		m_LCPowerups.bTrailReset = 1;
	if( bute.GetBool( "LightCycles", "PowerupAllowForceUse", m_LCPowerups.bForceUse ))
		m_LCPowerups.bForceUse = 1;
	if( bute.GetBool( "LightCycles", "PowerupAllowWallExtender", m_LCPowerups.bWallExtender ))
		m_LCPowerups.bWallExtender = 1;
	m_nLCKillPointValue = ( int8 )bute.GetInt( "LightCycles", "KillPointValue", m_nLCKillPointValue );
	m_nLCSelfKillPointValue = ( int8 )bute.GetInt( "LightCycles", "SelfKillPointValue", m_nLCSelfKillPointValue );
	m_nLCWinRacePointValue = ( int8 )bute.GetInt( "LightCycles", "WinRacePointValue", m_nLCWinRacePointValue );
	LOAD_MAP_LIST( "Lightcycles", m_saLCMaps );

	return Validate( );
}

// RETAIL: CShell.dll 0x100e9ab0 (the client's only): the options are made
// valid first (the result is not checked); Tron's returns nothing
void ServerGameOptions::SaveToBute( CButeMgr& bute )
{
	// The default options are not saved.
	if( stricmp( m_sOptionsName.c_str( ), ".Default" ) == 0 )
		return;

	Validate( );

	char szKey[32];
	int nMap;

	bute.SetString( "Settings", "ServerName", m_sSessionName.c_str( ));
	bute.SetString( "Settings", "Password", m_sPassword.c_str( ));
	bute.SetString( "Settings", "SCMDPassword", m_sScmdPassword.c_str( ));
	bute.SetInt( "Settings", "GameType", m_eGameType );
	bute.SetInt( "Settings", "Port", m_nPort );
	bute.SetInt( "Settings", "Bandwidth", m_nBandwidthServer );
	bute.SetInt( "Settings", "BandwidthCustom", m_nBandwidthServerCustom );
	bute.SetInt( "Settings", "Locked", m_bUsePassword );
	bute.SetInt( "Settings", "Dedicated", m_bDedicated );
	bute.SetInt( "Settings", "LANOnly", m_bLANOnly );
	bute.SetInt( "Settings", "AllowSCMD", m_bAllowScmdCommands );

	bute.SetInt( "DiscTournament", "MaxPlayers", m_nTournMaxPlayers );
	bute.SetInt( "DiscTournament", "MaxTeamSize", m_nTournMaxTeamSize );
	bute.SetInt( "DiscTournament", "FriendlyFire", m_bTournFriendlyFire );
	bute.SetInt( "DiscTournament", "DiscsInSpectator", m_bTournDiscsInSpectator );
	bute.SetInt( "DiscTournament", "AllowAutoAim", m_bTournAllowAutoAim );
	bute.SetInt( "DiscTournament", "PointsPerMatch", m_nTournPointsPerMatch );
	bute.SetInt( "DiscTournament", "MatchTimeLimit", m_nTournMatchTimeLimit );
	bute.SetInt( "DiscTournament", "TeamsToStart", m_nTournTeamsToStart );
	bute.SetInt( "DiscTournament", "TournamentMode", m_nTournMode );
	bute.SetInt( "DiscTournament", "GamesPerMap", m_nTournGamesPerMap );
	bute.SetInt( "DiscTournament", "PreGameDelay", m_nTournPreGameDelay );
	bute.SetInt( "DiscTournament", "MaxPing", m_nTournMaxPing );
	bute.SetInt( "DiscTournament", "AirControl", m_nTournAirControl );
	SAVE_MAP_LIST( "DiscTournament", m_saTournMaps );

	bute.SetInt( "DiscArena", "MaxPlayers", m_nArenaMaxPlayers );
	bute.SetInt( "DiscArena", "MaxTeamSize", m_nArenaMaxTeamSize );
	bute.SetInt( "DiscArena", "FriendlyFire", m_bArenaFriendlyFire );
	bute.SetInt( "DiscArena", "DiscsInSpectator", m_bArenaDiscsInSpectator );
	bute.SetInt( "DiscArena", "AllowAutoAim", m_bArenaAllowAutoAim );
	bute.SetInt( "DiscArena", "PointsPerMatch", m_nArenaPointsPerMatch );
	bute.SetInt( "DiscArena", "MatchTimeLimit", m_nArenaMatchTimeLimit );
	bute.SetInt( "DiscArena", "WinLimit", m_nArenaWinLimit );
	bute.SetInt( "DiscArena", "TimeLimit", m_nArenaTimeLimit );
	bute.SetInt( "DiscArena", "GamesPerMap", m_nArenaGamesPerMap );
	bute.SetInt( "DiscArena", "PreGameDelay", m_nArenaPreGameDelay );
	bute.SetInt( "DiscArena", "MaxPing", m_nArenaMaxPing );
	bute.SetInt( "DiscArena", "AirControl", m_nArenaAirControl );
	SAVE_MAP_LIST( "DiscArena", m_saArenaMaps );

	bute.SetInt( "Lightcycles", "MaxPlayers", m_nLCMaxPlayers );
	bute.SetInt( "LightCycles", "MaxTeamSize", m_nLCMaxTeamSize );
	bute.SetInt( "LightCycles", "TrailLength", m_nLCTrailLength );
	bute.SetInt( "LightCycles", "GameSpeed", m_nLCGameSpeed );
	bute.SetInt( "LightCycles", "AllowedCycleType", m_nLCAllowedCycleType );
	bute.SetInt( "LightCycles", "WaitForPlayersTime", m_nLCWaitForPlayersTime );
	bute.SetInt( "LightCycles", "TotalRaces", m_nLCTotalRaces );
	bute.SetInt( "LightCycles", "TotalPoints", m_nLCTotalPoints );
	bute.SetInt( "LightCycles", "TotalTime", m_nLCTotalTime );
	bute.SetInt( "LightCycles", "MaxPing", m_nLCMaxPing );
	bute.SetBool( "LightCycles", "PowerupAllowShield", ( m_nLCPowerups >> 0 ) & 1 );
	bute.SetBool( "LightCycles", "PowerupAllowTurbo", ( m_nLCPowerups >> 1 ) & 1 );
	bute.SetBool( "LightCycles", "PowerupAllowTrailSpike", ( m_nLCPowerups >> 2 ) & 1 );
	bute.SetBool( "LightCycles", "PowerupAllowMissile", ( m_nLCPowerups >> 3 ) & 1 );
	bute.SetBool( "LightCycles", "PowerupAllowTurboCurse", ( m_nLCPowerups >> 5 ) & 1 );
	bute.SetBool( "LightCycles", "PowerupAllowGrab", ( m_nLCPowerups >> 6 ) & 1 );
	bute.SetBool( "LightCycles", "PowerupAllowTrailReset", ( m_nLCPowerups >> 7 ) & 1 );
	bute.SetBool( "LightCycles", "PowerupAllowForceUse", ( m_nLCPowerups >> 8 ) & 1 );
	bute.SetBool( "LightCycles", "PowerupAllowWallExtender", ( m_nLCPowerups >> 4 ) & 1 );
	bute.SetInt( "LightCycles", "KillPointValue", m_nLCKillPointValue );
	bute.SetInt( "LightCycles", "SelfKillPointValue", m_nLCSelfKillPointValue );
	bute.SetInt( "LightCycles", "WinRacePointValue", m_nLCWinRacePointValue );
	SAVE_MAP_LIST( "Lightcycles", m_saLCMaps );
}

// RETAIL: Object.lto 0x101149d0
ServerGameOptions& ServerGameOptions::Copy( ServerGameOptions const& other )
{
	if( &other == this )
		return *this;

	m_sOptionsName = other.m_sOptionsName;
	m_sSessionName = other.m_sSessionName;
	m_sPassword = other.m_sPassword;
	m_sScmdPassword = other.m_sScmdPassword;
	m_eGameType = other.m_eGameType;
	m_nPort = other.m_nPort;
	m_nBandwidthServer = other.m_nBandwidthServer;
	m_nBandwidthServerCustom = other.m_nBandwidthServerCustom;
	m_bUsePassword = other.m_bUsePassword;
	m_bDedicated = other.m_bDedicated;
	m_bLANOnly = other.m_bLANOnly;
	m_bAllowScmdCommands = other.m_bAllowScmdCommands;
	m_sModName = other.m_sModName;

	m_nTournMaxPlayers = other.m_nTournMaxPlayers;
	m_nTournMaxTeamSize = other.m_nTournMaxTeamSize;
	m_bTournFriendlyFire = other.m_bTournFriendlyFire;
	m_bTournDiscsInSpectator = other.m_bTournDiscsInSpectator;
	m_bTournAllowAutoAim = other.m_bTournAllowAutoAim;
	m_nTournPointsPerMatch = other.m_nTournPointsPerMatch;
	m_nTournMatchTimeLimit = other.m_nTournMatchTimeLimit;
	m_nTournTeamsToStart = other.m_nTournTeamsToStart;
	m_nTournMode = other.m_nTournMode;
	m_nTournGamesPerMap = other.m_nTournGamesPerMap;
	m_nTournPreGameDelay = other.m_nTournPreGameDelay;
	m_nTournMaxPing = other.m_nTournMaxPing;
	m_nTournAirControl = other.m_nTournAirControl;
	m_saTournMaps.clear( );
	for( StringArray::const_iterator iterTournMaps = other.m_saTournMaps.begin( ); iterTournMaps != other.m_saTournMaps.end( ); iterTournMaps++ )
		m_saTournMaps.push_back( *iterTournMaps );

	m_nArenaMaxPlayers = other.m_nArenaMaxPlayers;
	m_nArenaMaxTeamSize = other.m_nArenaMaxTeamSize;
	m_bArenaFriendlyFire = other.m_bArenaFriendlyFire;
	m_bArenaDiscsInSpectator = other.m_bArenaDiscsInSpectator;
	m_bArenaAllowAutoAim = other.m_bArenaAllowAutoAim;
	m_nArenaPointsPerMatch = other.m_nArenaPointsPerMatch;
	m_nArenaMatchTimeLimit = other.m_nArenaMatchTimeLimit;
	m_nArenaWinLimit = other.m_nArenaWinLimit;
	m_nArenaTimeLimit = other.m_nArenaTimeLimit;
	m_nArenaGamesPerMap = other.m_nArenaGamesPerMap;
	m_nArenaPreGameDelay = other.m_nArenaPreGameDelay;
	m_nArenaMaxPing = other.m_nArenaMaxPing;
	m_nArenaAirControl = other.m_nArenaAirControl;
	m_saArenaMaps.clear( );
	for( StringArray::const_iterator iterArenaMaps = other.m_saArenaMaps.begin( ); iterArenaMaps != other.m_saArenaMaps.end( ); iterArenaMaps++ )
		m_saArenaMaps.push_back( *iterArenaMaps );

	m_nLCMaxPlayers = other.m_nLCMaxPlayers;
	m_nLCAllowedCycleType = other.m_nLCAllowedCycleType;
	m_nLCGameSpeed = other.m_nLCGameSpeed;
	m_nLCTrailLength = other.m_nLCTrailLength;
	m_nLCMaxTeamSize = other.m_nLCMaxTeamSize;
	m_nLCWaitForPlayersTime = other.m_nLCWaitForPlayersTime;
	m_nLCTotalRaces = other.m_nLCTotalRaces;
	m_nLCTotalPoints = other.m_nLCTotalPoints;
	m_nLCTotalTime = other.m_nLCTotalTime;
	m_nLCMaxPing = other.m_nLCMaxPing;
	m_nLCPowerups = other.m_nLCPowerups;
	m_nLCKillPointValue = other.m_nLCKillPointValue;
	m_nLCSelfKillPointValue = other.m_nLCSelfKillPointValue;
	m_nLCWinRacePointValue = other.m_nLCWinRacePointValue;
	m_saLCMaps.clear( );
	for( StringArray::const_iterator iterLCMaps = other.m_saLCMaps.begin( ); iterLCMaps != other.m_saLCMaps.end( ); iterLCMaps++ )
		m_saLCMaps.push_back( *iterLCMaps );

	m_sProfileName = other.m_sProfileName;
	m_bPreCacheAssets = other.m_bPreCacheAssets;
	m_bPerformanceTest = other.m_bPerformanceTest;
	m_nTronUnknownCC = other.m_nTronUnknownCC;

	return *this;
}

// RETAIL: Object.lto 0x10114e20: the map list of the game type
StringArray* ServerGameOptions::GetMapList( )
{
	switch( m_eGameType )
	{
		case eGameTypeDiscArena:
			return &m_saArenaMaps;
		case eGameTypeDiscTournament:
			return &m_saTournMaps;
		case eGameTypeMultiplayerLightcycles:
			return &m_saLCMaps;
	}

	return NULL;
}

// RETAIL: Object.lto 0x10114e50 (the name is ours)
uint32 ServerGameOptions::GetTronUnknownCC( ) const
{
	return m_nTronUnknownCC;
}

// RETAIL: TRONSrv.exe 0x408fd0, CShell.dll 0x100eb530: turns the session
// name's backslashes into slashes, true if there were any. The name is ours.
bool ServerGameOptions::FixSessionName( )
{
	int nPos = m_sSessionName.find( '\\' );
	bool bChanged = false;
	while( nPos != std::string::npos )
	{
		bChanged = true;
		m_sSessionName[nPos] = '/';
		nPos = m_sSessionName.find( '\\', nPos + 1 );
	}
	return bChanged;
}

#ifndef _CLIENTBUILD

// RETAIL: Tron's own GameSpy keys (qr2_register_key ids; the names are ours,
// after the key strings)
enum GameSpyKey
{
	kGSKeyFriendlyFire	= 50,	// "friendlyfire"
	kGSKeyMatchPoints	= 51,	// "matchpoints"
	kGSKeyMatchTime		= 52,	// "matchtime"
	kGSKeyGamesPerMap	= 53,	// "gamespermap"
	kGSKeyTournMode		= 54,	// reported for the disc tournament; registered nowhere seen
	kGSKeyAirControl	= 55,	// "aircontrol"
	kGSKeyDedicated		= 56,	// "dedicated"
	kGSKeyWaitWeaps		= 57,	// "waitweaps"
	kGSKeyAutoAim		= 58,	// "autoaim"
	kGSKeyCycleType		= 59,	// "cycletype"
	kGSKeyTrailLength	= 60,	// "traillength"
	kGSKeyMaxPing		= 61,	// "maxping"
	kGSKeyKillPoints	= 62,	// "killpoints"
	kGSKeyDeathPoints	= 63,	// "deathpoints"
	kGSKeyWinPoints		= 64,	// "winpoints"
	kGSKeyPowerups		= 65,	// "powerups"
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerGameOptions::RegisterGameSpyKeys
//
//	PURPOSE:	RETAIL: Object.lto 0x10114e60: register the game type's keys
//
// ----------------------------------------------------------------------- //
void ServerGameOptions::RegisterGameSpyKeys( )
{
	qr2_register_key( kGSKeyDedicated, "dedicated" );

	switch( m_eGameType )
	{
		case eGameTypeMultiplayerLightcycles:
		{
			qr2_register_key( kGSKeyCycleType, "cycletype" );
			qr2_register_key( kGSKeyTrailLength, "traillength" );
			qr2_register_key( kGSKeyMaxPing, "maxping" );
			qr2_register_key( kGSKeyKillPoints, "killpoints" );
			qr2_register_key( kGSKeyDeathPoints, "deathpoints" );
			qr2_register_key( kGSKeyWinPoints, "winpoints" );
			qr2_register_key( kGSKeyPowerups, "powerups" );
			qr2_register_key( kGSKeyGamesPerMap, "gamespermap" );
		}
		break;

		case eGameTypeDiscArena:
		{
			qr2_register_key( kGSKeyFriendlyFire, "friendlyfire" );
			qr2_register_key( kGSKeyWaitWeaps, "waitweaps" );
			qr2_register_key( kGSKeyAutoAim, "autoaim" );
			qr2_register_key( kGSKeyMatchPoints, "matchpoints" );
			qr2_register_key( kGSKeyMatchTime, "matchtime" );
			qr2_register_key( kGSKeyGamesPerMap, "gamespermap" );
			qr2_register_key( kGSKeyMaxPing, "maxping" );
			qr2_register_key( kGSKeyAirControl, "aircontrol" );
		}
		break;

		case eGameTypeDiscTournament:
		{
			qr2_register_key( kGSKeyFriendlyFire, "friendlyfire" );
			qr2_register_key( kGSKeyWaitWeaps, "waitweaps" );
			qr2_register_key( kGSKeyAutoAim, "autoaim" );
			qr2_register_key( kGSKeyMatchPoints, "matchpoints" );
			qr2_register_key( kGSKeyMatchTime, "matchtime" );
			qr2_register_key( kGSKeyGamesPerMap, "gamespermap" );
			qr2_register_key( kGSKeyMaxPing, "maxping" );
			qr2_register_key( kGSKeyAirControl, "aircontrol" );
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerGameOptions::AddGameSpyKeys
//
//	PURPOSE:	RETAIL: Object.lto 0x10114f50: the server keys the game type
//				reports, after the shell's own
//
// ----------------------------------------------------------------------- //
void ServerGameOptions::AddGameSpyKeys( int nKeyType, qr2_keybuffer_s* pKeyBuffer )
{
	if( nKeyType != key_server )
		return;

	qr2_keybuffer_add( pKeyBuffer, kGSKeyDedicated );

	switch( m_eGameType )
	{
		case eGameTypeMultiplayerLightcycles:
		{
			qr2_keybuffer_add( pKeyBuffer, TEAMPLAY_KEY );
			qr2_keybuffer_add( pKeyBuffer, FRAGLIMIT_KEY );
			qr2_keybuffer_add( pKeyBuffer, TIMELIMIT_KEY );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyCycleType );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyTrailLength );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyGamesPerMap );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyMaxPing );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyKillPoints );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyDeathPoints );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyWinPoints );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyPowerups );
		}
		break;

		case eGameTypeDiscTournament:
		{
			qr2_keybuffer_add( pKeyBuffer, TEAMPLAY_KEY );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyFriendlyFire );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyWaitWeaps );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyAutoAim );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyMatchPoints );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyMatchTime );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyGamesPerMap );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyMaxPing );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyAirControl );
		}
		break;

		case eGameTypeDiscArena:
		{
			qr2_keybuffer_add( pKeyBuffer, TEAMPLAY_KEY );
			qr2_keybuffer_add( pKeyBuffer, FRAGLIMIT_KEY );
			qr2_keybuffer_add( pKeyBuffer, TIMELIMIT_KEY );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyFriendlyFire );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyWaitWeaps );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyAutoAim );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyMatchPoints );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyMatchTime );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyGamesPerMap );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyMaxPing );
			qr2_keybuffer_add( pKeyBuffer, kGSKeyAirControl );
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerGameOptions::GetGameSpyKeyValue
//
//	PURPOSE:	RETAIL: Object.lto 0x101150a0: report the value of one of
//				the game type's keys; false if it is not one of them
//
// ----------------------------------------------------------------------- //
bool ServerGameOptions::GetGameSpyKeyValue( int nKeyId, qr2_buffer_s* pOutBuf )
{
	if( nKeyId == kGSKeyDedicated )
	{
		qr2_buffer_add_int( pOutBuf, m_bDedicated );
		return true;
	}

	switch( m_eGameType )
	{
		case eGameTypeMultiplayerLightcycles:
		{
			switch( nKeyId )
			{
				case TEAMPLAY_KEY:		qr2_buffer_add_int( pOutBuf, ( m_nLCMaxTeamSize > 1 ) ? m_nLCMaxTeamSize : 0 ); return true;
				case kGSKeyCycleType:	qr2_buffer_add_int( pOutBuf, m_nLCAllowedCycleType ); return true;
				case kGSKeyTrailLength:	qr2_buffer_add_int( pOutBuf, m_nLCTrailLength ); return true;
				case kGSKeyGamesPerMap:	qr2_buffer_add_int( pOutBuf, m_nLCTotalRaces ); return true;
				case FRAGLIMIT_KEY:		qr2_buffer_add_int( pOutBuf, m_nLCTotalPoints ); return true;
				case TIMELIMIT_KEY:		qr2_buffer_add_int( pOutBuf, m_nLCTotalTime ); return true;
				case kGSKeyMaxPing:		qr2_buffer_add_int( pOutBuf, m_nLCMaxPing ); return true;
				case kGSKeyKillPoints:	qr2_buffer_add_int( pOutBuf, m_nLCKillPointValue ); return true;
				case kGSKeyDeathPoints:	qr2_buffer_add_int( pOutBuf, m_nLCSelfKillPointValue ); return true;
				case kGSKeyWinPoints:	qr2_buffer_add_int( pOutBuf, m_nLCWinRacePointValue ); return true;
				case kGSKeyPowerups:	qr2_buffer_add_int( pOutBuf, m_nLCPowerups ); return true;
			}
		}
		break;

		case eGameTypeDiscTournament:
		{
			switch( nKeyId )
			{
				case TEAMPLAY_KEY:		qr2_buffer_add_int( pOutBuf, ( m_nTournMaxTeamSize > 1 ) ? m_nTournMaxTeamSize : 0 ); return true;
				case kGSKeyTournMode:	qr2_buffer_add_int( pOutBuf, m_nTournMode ); return true;
				case kGSKeyFriendlyFire:	qr2_buffer_add_int( pOutBuf, m_bTournFriendlyFire ); return true;
				case kGSKeyWaitWeaps:	qr2_buffer_add_int( pOutBuf, m_bTournDiscsInSpectator ); return true;
				case kGSKeyAutoAim:		qr2_buffer_add_int( pOutBuf, m_bTournAllowAutoAim ); return true;
				case kGSKeyMatchPoints:	qr2_buffer_add_int( pOutBuf, m_nTournPointsPerMatch ); return true;
				case kGSKeyMatchTime:	qr2_buffer_add_int( pOutBuf, m_nTournMatchTimeLimit ); return true;
				case kGSKeyGamesPerMap:	qr2_buffer_add_int( pOutBuf, m_nTournGamesPerMap ); return true;
				case kGSKeyMaxPing:		qr2_buffer_add_int( pOutBuf, m_nTournMaxPing ); return true;
				case kGSKeyAirControl:	qr2_buffer_add_int( pOutBuf, m_nTournAirControl ); return true;
			}
		}
		break;

		case eGameTypeDiscArena:
		{
			switch( nKeyId )
			{
				case TEAMPLAY_KEY:		qr2_buffer_add_int( pOutBuf, ( m_nArenaMaxTeamSize > 1 ) ? m_nArenaMaxTeamSize : 0 ); return true;
				case FRAGLIMIT_KEY:		qr2_buffer_add_int( pOutBuf, m_nArenaWinLimit ); return true;
				case TIMELIMIT_KEY:		qr2_buffer_add_int( pOutBuf, m_nArenaTimeLimit ); return true;
				case kGSKeyFriendlyFire:	qr2_buffer_add_int( pOutBuf, m_bArenaFriendlyFire ); return true;
				case kGSKeyWaitWeaps:	qr2_buffer_add_int( pOutBuf, m_bArenaDiscsInSpectator ); return true;
				case kGSKeyAutoAim:		qr2_buffer_add_int( pOutBuf, m_bArenaAllowAutoAim ); return true;
				case kGSKeyMatchPoints:	qr2_buffer_add_int( pOutBuf, m_nArenaPointsPerMatch ); return true;
				case kGSKeyMatchTime:	qr2_buffer_add_int( pOutBuf, m_nArenaMatchTimeLimit ); return true;
				case kGSKeyGamesPerMap:	qr2_buffer_add_int( pOutBuf, m_nArenaGamesPerMap ); return true;
				case kGSKeyMaxPing:		qr2_buffer_add_int( pOutBuf, m_nArenaMaxPing ); return true;
				case kGSKeyAirControl:	qr2_buffer_add_int( pOutBuf, m_nArenaAirControl ); return true;
			}
		}
		break;
	}

	return false;
}

#endif // _CLIENTBUILD

#ifdef PROFILEUTILS_NOLF2_OPTIONS
bool ServerGameOptions::IsDefaultCampaign() const
{
	return (stricmp(GetCampaignName(), DEFAULT_CAMPAIGN ) == 0);
}

const char*	ServerGameOptions::GetCampaignName() const
{

	switch (m_eGameType)
	{
	case eGameTypeCooperative:
		return m_Coop.m_sCampaignName.c_str();
		break;
	case eGameTypeDeathmatch:
		return m_DM.m_sCampaignName.c_str();
		break;
	case eGameTypeTeamDeathmatch:
		return m_TeamDM.m_sCampaignName.c_str();
		break;
	case eGameTypeDoomsDay:
		return m_DD.m_sCampaignName.c_str();
		break;
		
	};

	return DEFAULT_CAMPAIGN;
}
void ServerGameOptions::SetCampaignName(const char*	pszName)
{
	switch (m_eGameType)
	{
	case eGameTypeCooperative:
		m_Coop.m_sCampaignName = pszName;
		break;
	case eGameTypeDeathmatch:
		m_DM.m_sCampaignName = pszName;
		break;
	case eGameTypeTeamDeathmatch:
		m_TeamDM.m_sCampaignName = pszName;
		break;
	case eGameTypeDoomsDay:
		m_DD.m_sCampaignName = pszName;
		break;
	};
}
#endif // PROFILEUTILS_NOLF2_OPTIONS



const char*	ServerGameOptions::GetSessionName() const
{
	return m_sSessionName.c_str( );
}
void ServerGameOptions::SetSessionName(const char*	pszName)
{
	m_sSessionName = pszName;
}


// PORT: Tron's game types' max players (retail's own not yet located)
uint8 ServerGameOptions::GetMaxPlayers() const
{
	switch (m_eGameType)
	{
		case eGameTypeDiscTournament:
			return m_nTournMaxPlayers;
		case eGameTypeDiscArena:
			return m_nArenaMaxPlayers;
		case eGameTypeMultiplayerLightcycles:
			return m_nLCMaxPlayers;
		default:
			return 0;
	};
}

int ServerGameOptions::GetGameTypeStringID() const
{
	switch (m_eGameType)
	{
		case eGameTypeCooperative:
		{
			return IDS_COOPERATIVE;
		}
		break;

		case eGameTypeDeathmatch:
		{	
			return IDS_DEATHMATCH;
		}
		break;
		
		case eGameTypeTeamDeathmatch:
		{
			return IDS_TEAMDEATHMATCH;
		}
		break;

		case eGameTypeDoomsDay:
		{
			return IDS_DOOMSDAY;
		}
		break;
		
		default:
		{
			return 0;
		}
		break;
	};
}

/************************************************************************************
**** Cooperative game options
************************************************************************************/

CoopGameOptions::CoopGameOptions( )
{
}

void CoopGameOptions::Clear( )
{
	m_sSessionName = "";
	m_sCampaignName = DEFAULT_CAMPAIGN;

	m_nMaxPlayers = 4;
	m_bUseSkills = true;
	m_bFriendlyFire = true;
	m_nDifficulty = 1;
	m_fPlayerDiffFactor = 1.0f;
}

bool CoopGameOptions::LoadFromBute( CButeMgr& bute )
{
	char const szTagName[] = "Cooperative";
	std::string sAttName = "";

	char szString[256];
	bute.GetString( szTagName, "SessionName", m_sSessionName.c_str( ), szString, ARRAY_LEN( szString ));
	m_sSessionName = szString;

	bute.GetString( szTagName, "CampaignName", DEFAULT_CAMPAIGN, szString, ARRAY_LEN( szString ));
	m_sCampaignName = szString;

	m_nMaxPlayers = (uint8)bute.GetInt( szTagName, "MaxPlayers", 4 );
	m_bUseSkills = bute.GetBool(szTagName,"UseSkills",true);
	m_bFriendlyFire = bute.GetBool(szTagName,"FriendlyFire",true);
	m_nDifficulty = (uint8)bute.GetInt(szTagName,"MPDifficulty",1);
	m_fPlayerDiffFactor = bute.GetFloat(szTagName,"MPPlayerDiffFactor",0.1f);

	return true;
}

bool CoopGameOptions::SaveToBute( CButeMgr& bute )
{
	char const szTagName[] = "Cooperative";

	bute.SetString( szTagName, "SessionName", m_sSessionName.c_str());
	bute.SetString( szTagName, "CampaignName", m_sCampaignName.c_str( ));

	bute.SetInt(szTagName,"MaxPlayers",(int)m_nMaxPlayers);
	bute.SetBool(szTagName,"UseSkills",m_bUseSkills);
	bute.SetBool(szTagName,"FriendlyFire",m_bFriendlyFire);
	bute.SetInt(szTagName,"MPDifficulty",(int)m_nDifficulty);
	bute.SetFloat(szTagName,"MPPlayerDiffFactor",m_fPlayerDiffFactor);

	return true;
}

CoopGameOptions& CoopGameOptions::Copy( CoopGameOptions const& other )
{
	if( &other == this )
		return *this;

	m_sSessionName = other.m_sSessionName;
	m_sCampaignName = other.m_sCampaignName;

	m_nMaxPlayers = other.m_nMaxPlayers;
	m_bUseSkills = other.m_bUseSkills;
	m_bFriendlyFire = other.m_bFriendlyFire;
	m_nDifficulty = other.m_nDifficulty;
	m_fPlayerDiffFactor = other.m_fPlayerDiffFactor;

	return *this;
}


/************************************************************************************
**** Deathmatch game options
************************************************************************************/

DMGameOptions::DMGameOptions( )
{
}

void DMGameOptions::Clear( )
{
	m_sSessionName = "";
	m_sCampaignName = DEFAULT_CAMPAIGN;

	m_nMaxPlayers = 8;
	m_nRunSpeed = 130;
	m_nScoreLimit = 25;
	m_nTimeLimit = 10;
	m_nRounds = 1;

	m_nFragScore = 2;
	m_nTagScore = 1;
}


bool DMGameOptions::LoadFromBute( CButeMgr& bute )
{
	char const szTagName[] = "Deathmatch";
	std::string sAttName = "";

	char szString[256];
	bute.GetString( szTagName, "SessionName", m_sSessionName.c_str( ), szString, ARRAY_LEN( szString ));
	m_sSessionName = szString;

	bute.GetString( szTagName, "CampaignName", DEFAULT_CAMPAIGN, szString, ARRAY_LEN( szString ));
	m_sCampaignName = szString;

	m_nMaxPlayers = (uint8)bute.GetInt( szTagName, "MaxPlayers", 16 );
	m_nRunSpeed = (uint8)bute.GetInt(szTagName,"RunSpeed",130);
	m_nScoreLimit = (uint8)bute.GetInt(szTagName,"ScoreLimit",25);
	m_nTimeLimit = (uint8)bute.GetInt(szTagName,"TimeLimit",10);
	m_nRounds = (uint8)bute.GetInt( szTagName, "Rounds", 0 );

	m_nFragScore = bute.GetInt(szTagName,"FragScore", 2);
	m_nTagScore = bute.GetInt(szTagName,"TagScore", 1);

	return true;
}

bool DMGameOptions::SaveToBute( CButeMgr& bute )
{
	char const szTagName[] = "Deathmatch";

	bute.SetString( szTagName, "SessionName", m_sSessionName.c_str());
	bute.SetString( szTagName, "CampaignName", m_sCampaignName.c_str( ));

	bute.SetInt(szTagName,"MaxPlayers",(int)m_nMaxPlayers);
	bute.SetInt(szTagName,"RunSpeed",m_nRunSpeed);
	bute.SetInt(szTagName,"ScoreLimit",m_nScoreLimit);
	bute.SetInt(szTagName,"TimeLimit",m_nTimeLimit);
	bute.SetInt(szTagName,"Rounds",m_nRounds);

	bute.SetInt(szTagName,"FragScore",m_nFragScore);
	bute.SetInt(szTagName,"TagScore",m_nTagScore);

	return true;
}

// for transmitting options that may change mid game
void DMGameOptions::Write( ILTMessage_Write *pMsg )
{

	pMsg->Writeuint8(m_nRunSpeed);
	pMsg->Writeuint8(m_nScoreLimit);
	pMsg->Writeuint8(m_nTimeLimit);
	pMsg->Writeuint8(m_nRounds );

	pMsg->Writeuint8(m_nFragScore);
	pMsg->Writeuint8(m_nTagScore);

}

// for transmitting options that may change mid game
void DMGameOptions::Read( ILTMessage_Read *pMsg )
{
	m_nRunSpeed = pMsg->Readuint8();
	m_nScoreLimit = pMsg->Readuint8();
	m_nTimeLimit = pMsg->Readuint8();
	m_nRounds = pMsg->Readuint8();

	m_nFragScore = pMsg->Readuint8();
	m_nTagScore = pMsg->Readuint8();

}


DMGameOptions& DMGameOptions::Copy( DMGameOptions const& other )
{
	if( &other == this )
		return *this;

	m_sSessionName = other.m_sSessionName;
	m_sCampaignName = other.m_sCampaignName;

	m_nMaxPlayers = other.m_nMaxPlayers;
	m_nRunSpeed = other.m_nRunSpeed;
	m_nScoreLimit = other.m_nScoreLimit;
	m_nTimeLimit = other.m_nTimeLimit;
	m_nRounds = other.m_nRounds;
	m_nFragScore = other.m_nFragScore;
	m_nTagScore = other.m_nTagScore;
	
	return *this;
}



/************************************************************************************
**** TeamDeathmatch game options
************************************************************************************/

TeamDMGameOptions::TeamDMGameOptions( )
{

}

void TeamDMGameOptions::Clear( )
{
	m_sSessionName = "";
	m_sCampaignName = DEFAULT_CAMPAIGN;

	m_nMaxPlayers = 16;
	m_nRunSpeed = 130;
	m_nScoreLimit = 50;
	m_nTimeLimit = 10;
	m_nRounds = 1;
	m_bFriendlyFire = true;

	m_nNumTeams = 2;

	char szTemp[32] = {0};
	for(int i = 0; i < MAX_TEAMS; ++i )
	{
		m_nTeamModel[i] = i;
		sprintf( szTemp, "Team %i", i+1 );
		m_sTeamName[i] = szTemp;
	}

	m_nFragScore = 2;
	m_nTagScore = 1;
	m_nRevivingScore = 1;
}

bool TeamDMGameOptions::LoadFromBute( CButeMgr& bute )
{
	char const szTagName[] = "TeamDM";
	std::string sAttName = "";

	char szString[256];
	bute.GetString( szTagName, "SessionName", m_sSessionName.c_str( ), szString, ARRAY_LEN( szString ));
	m_sSessionName = szString;

	bute.GetString( szTagName, "CampaignName", DEFAULT_CAMPAIGN, szString, ARRAY_LEN( szString ));
	m_sCampaignName = szString;

	m_nMaxPlayers = (uint8)bute.GetInt( szTagName, "MaxPlayers", 16 );
	m_nRunSpeed = (uint8)bute.GetInt(szTagName,"RunSpeed",130);
	m_nScoreLimit = (uint8)bute.GetInt(szTagName,"ScoreLimit",100);
	m_nTimeLimit = (uint8)bute.GetInt(szTagName,"TimeLimit",10);
	m_nRounds = (uint8)bute.GetInt( szTagName, "Rounds", 1 );
	m_bFriendlyFire = bute.GetBool(szTagName,"FriendlyFire",true);	

	m_nNumTeams = bute.GetInt( szTagName, "NumTeams", 2 );

	char szAttName[32] = {0};
	char szTemp[32] = {0};
	for( int i = 0; i < MAX_TEAMS; ++i )
	{
		int defModel = i;
		sprintf( szTemp, "Team %i", i+1 );
#ifdef _CLIENTBUILD
		defModel = g_pModelButeMgr->GetTeamDefaultModel(i);
		LoadString(nDefTeamID[i],szTemp,sizeof(szTemp));
#endif

		sprintf( szAttName, "Team%iModel", i );
		m_nTeamModel[i] = bute.GetInt( szTagName, szAttName, defModel );

		sprintf( szAttName, "Team%iName", i );
		bute.GetString( szTagName, szAttName, szTemp, szString, ARRAY_LEN( szString ));
		m_sTeamName[i] = szString;
	}

	m_nFragScore = bute.GetInt(szTagName,"FragScore", 2);
	m_nTagScore = bute.GetInt(szTagName,"TagScore", 1);

	m_nRevivingScore = ( uint8 )bute.GetInt( szTagName, "ReviveScore", 1 );

	return true;
}

bool TeamDMGameOptions::SaveToBute( CButeMgr& bute )
{
	char const szTagName[] = "TeamDM";

	bute.SetString( szTagName, "SessionName", m_sSessionName.c_str());
	bute.SetString( szTagName, "CampaignName", m_sCampaignName.c_str( ));


	bute.SetInt(szTagName,"MaxPlayers",(int)m_nMaxPlayers);
	bute.SetInt(szTagName,"RunSpeed",m_nRunSpeed);
	bute.SetInt(szTagName,"ScoreLimit",m_nScoreLimit);
	bute.SetInt(szTagName,"TimeLimit",m_nTimeLimit);
	bute.SetInt(szTagName,"Rounds", m_nRounds );
	bute.SetBool(szTagName,"FriendlyFire",m_bFriendlyFire);

	
	bute.SetInt( szTagName, "NumTeams", m_nNumTeams );
	
	char szAttName[32] = {0};
	for( int i = 0; i < MAX_TEAMS; ++i )
	{
		sprintf( szAttName, "Team%iModel", i );
		bute.SetInt( szTagName, szAttName, m_nTeamModel[i] );

		sprintf( szAttName, "Team%iName", i );
		bute.SetString( szTagName, szAttName, m_sTeamName[i].c_str() );
	}

	bute.SetInt(szTagName,"FragScore",m_nFragScore);
	bute.SetInt(szTagName,"TagScore",m_nTagScore);

	bute.SetInt( szTagName, "ReviveScore", m_nRevivingScore );

	return true;

}


// for transmitting options that may change mid game
void TeamDMGameOptions::Write( ILTMessage_Write *pMsg )
{

	pMsg->Writeuint8(m_nRunSpeed);
	pMsg->Writeuint8(m_nTimeLimit);
	pMsg->Writeuint8(m_nScoreLimit);
	pMsg->Writeuint8(m_nRounds );
	pMsg->Writebool(m_bFriendlyFire);

	pMsg->Writeuint8(m_nFragScore);
	pMsg->Writeuint8(m_nTagScore);

}

// for transmitting options that may change mid game
void TeamDMGameOptions::Read( ILTMessage_Read *pMsg )
{
	m_nRunSpeed = pMsg->Readuint8();
	m_nTimeLimit = pMsg->Readuint8();
	m_nScoreLimit = pMsg->Readuint8();
	m_nRounds = pMsg->Readuint8();
	m_bFriendlyFire = pMsg->Readbool();

	m_nFragScore = pMsg->Readuint8();
	m_nTagScore = pMsg->Readuint8();

}


TeamDMGameOptions& TeamDMGameOptions::Copy( TeamDMGameOptions const& other )
{
	if( &other == this )
		return *this;

	m_sSessionName = other.m_sSessionName;
	m_sCampaignName = other.m_sCampaignName;

	m_nMaxPlayers = other.m_nMaxPlayers;
	m_nRunSpeed = other.m_nRunSpeed;
	m_nScoreLimit = other.m_nScoreLimit;
	m_nTimeLimit = other.m_nTimeLimit;
	m_nRounds = other.m_nRounds;
	m_bFriendlyFire = other.m_bFriendlyFire;

	m_nNumTeams = other.m_nNumTeams;

	for(int i = 0; i < MAX_TEAMS; ++i )
	{
		m_nTeamModel[i] = other.m_nTeamModel[i];
		m_sTeamName[i] = other.m_sTeamName[i];
	}

	m_nFragScore = other.m_nFragScore;
	m_nTagScore = other.m_nTagScore;

	m_nRevivingScore = other.m_nRevivingScore;

	return *this;
}


/************************************************************************************
**** Cooperative game options
************************************************************************************/

DoomsdayGameOptions::DoomsdayGameOptions( )
{
}


void DoomsdayGameOptions::Clear( )
{
	m_sSessionName = "";
	m_sCampaignName = DEFAULT_CAMPAIGN;

	m_nMaxPlayers = 16;
	m_nRunSpeed = 130;
	m_nTimeLimit = 20;
	m_nRounds = 1;
	m_bFriendlyFire = true;

	m_nNumTeams = 2;

	char szTemp[32] = {0};
	for(int i = 0; i < MAX_TEAMS; ++i )
	{
		m_nTeamModel[i] = i;
		sprintf( szTemp, "Team %i", i+1 );
		m_sTeamName[i] = szTemp;
	}

	m_nFragScore = 2;
	m_nTagScore = 1;
	m_nDeviceCompletedScore = 10;
	m_nLightPiecePlacedScore = 3;
	m_nHeavyPiecePlacedScore = 5;
	m_nPieceRemovedScore = 2;
	m_nRevivingScore = 1;
}

bool DoomsdayGameOptions::LoadFromBute( CButeMgr& bute )
{
	char const szTagName[] = "Doomsday";
	std::string sAttName = "";

	char szString[256];
	bute.GetString( szTagName, "SessionName", m_sSessionName.c_str( ), szString, ARRAY_LEN( szString ));
	m_sSessionName = szString;

	bute.GetString( szTagName, "CampaignName", DEFAULT_CAMPAIGN, szString, ARRAY_LEN( szString ));
	m_sCampaignName = szString;

	m_nMaxPlayers = (uint8)bute.GetInt( szTagName, "MaxPlayers", 16 );
	m_nRunSpeed = (uint8)bute.GetInt(szTagName,"RunSpeed",130);
	m_nTimeLimit = (uint8)bute.GetInt(szTagName,"TimeLimit",20);
	m_nRounds = (uint8)bute.GetInt(szTagName, "Rounds", 1 );
	m_bFriendlyFire = bute.GetBool(szTagName,"FriendlyFire",true);	

	m_nNumTeams = bute.GetInt( szTagName, "NumTeams", 2 );

	char szAttName[32] = {0};
	char szTemp[32] = {0};
	for( int i = 0; i < MAX_TEAMS; ++i )
	{
		int defModel = i;
		sprintf( szTemp, "Team %i", i+1 );
#ifdef _CLIENTBUILD
		defModel = g_pModelButeMgr->GetTeamDefaultModel(i);
		LoadString(nDefTeamID[i],szTemp,sizeof(szTemp));
#endif

		sprintf( szAttName, "Team%iModel", i );
		m_nTeamModel[i] = bute.GetInt( szTagName, szAttName, defModel );

		sprintf( szAttName, "Team%iName", i );
		bute.GetString( szTagName, szAttName, szTemp, szString, ARRAY_LEN( szString ));
		m_sTeamName[i] = szString;
	}

	m_nFragScore = bute.GetInt(szTagName,"FragScore", 2);
	m_nTagScore = bute.GetInt(szTagName,"TagScore", 1);

	m_nDeviceCompletedScore = ( uint8 )bute.GetInt( szTagName, "DeviceCompletedScore", 10 );
	m_nLightPiecePlacedScore = ( uint8 )bute.GetInt( szTagName, "LightPiecePlacedScore", 3 );
	m_nHeavyPiecePlacedScore = ( uint8 )bute.GetInt( szTagName, "HeavyPiecePlacedScore", 5 );
	m_nPieceRemovedScore = ( uint8 )bute.GetInt( szTagName, "PieceRemovedScore", 2 );
	m_nRevivingScore = ( uint8 )bute.GetInt( szTagName, "ReviveScore", 1 );

	return true;
}


// for transmitting options that may change mid game
void DoomsdayGameOptions::Write( ILTMessage_Write *pMsg )
{

	pMsg->Writeuint8(m_nRunSpeed);
	pMsg->Writeuint8(m_nTimeLimit);
	pMsg->Writeuint8(m_nRounds );
	pMsg->Writebool(m_bFriendlyFire);

	pMsg->Writeuint8(m_nFragScore);
	pMsg->Writeuint8(m_nTagScore);

	pMsg->Writeuint8( m_nDeviceCompletedScore );
	pMsg->Writeuint8( m_nLightPiecePlacedScore );
	pMsg->Writeuint8( m_nHeavyPiecePlacedScore );
	pMsg->Writeuint8( m_nPieceRemovedScore );
	pMsg->Writeuint8( m_nRevivingScore );

}

// for transmitting options that may change mid game
void DoomsdayGameOptions::Read( ILTMessage_Read *pMsg )
{
	m_nRunSpeed = pMsg->Readuint8();
	m_nTimeLimit = pMsg->Readuint8();
	m_nRounds = pMsg->Readuint8();
	m_bFriendlyFire = pMsg->Readbool();

	m_nFragScore = pMsg->Readuint8();
	m_nTagScore = pMsg->Readuint8();

	m_nDeviceCompletedScore = pMsg->Readuint8();
	m_nLightPiecePlacedScore = pMsg->Readuint8();
	m_nHeavyPiecePlacedScore = pMsg->Readuint8();
	m_nPieceRemovedScore = pMsg->Readuint8();
	m_nRevivingScore = pMsg->Readuint8();

}

bool DoomsdayGameOptions::SaveToBute( CButeMgr& bute )
{
	char const szTagName[] = "Doomsday";

	bute.SetString( szTagName, "SessionName", m_sSessionName.c_str());
	bute.SetString( szTagName, "CampaignName", m_sCampaignName.c_str( ));

	bute.SetInt(szTagName,"MaxPlayers",(int)m_nMaxPlayers);
	bute.SetInt(szTagName,"RunSpeed",m_nRunSpeed);
	bute.SetInt(szTagName,"TimeLimit",m_nTimeLimit);
	bute.SetInt(szTagName,"Rounds", m_nRounds );
	bute.SetBool(szTagName,"FriendlyFire",m_bFriendlyFire);

	bute.SetInt( szTagName, "NumTeams", m_nNumTeams );
	
	char szAttName[32] = {0};
	for( int i = 0; i < MAX_TEAMS; ++i )
	{
		sprintf( szAttName, "Team%iModel", i );
		bute.SetInt( szTagName, szAttName, m_nTeamModel[i] );

		sprintf( szAttName, "Team%iName", i );
		bute.SetString( szTagName, szAttName, m_sTeamName[i].c_str() );
	}

	bute.SetInt(szTagName,"FragScore",m_nFragScore);
	bute.SetInt(szTagName,"TagScore",m_nTagScore);

	bute.SetInt( szTagName, "DeviceCompletedScore", m_nDeviceCompletedScore );
	bute.SetInt( szTagName, "LightPiecePlacedScore", m_nLightPiecePlacedScore );
	bute.SetInt( szTagName, "HeavyPiecePlacedScore", m_nHeavyPiecePlacedScore );
	bute.SetInt( szTagName, "PieceRemovedScore", m_nPieceRemovedScore );
	bute.SetInt( szTagName, "ReviveScore", m_nRevivingScore );

	return true;

}


DoomsdayGameOptions& DoomsdayGameOptions::Copy( DoomsdayGameOptions const& other )
{
	if( &other == this )
		return *this;

	m_sSessionName = other.m_sSessionName;
	m_sCampaignName = other.m_sCampaignName;

	m_nMaxPlayers = other.m_nMaxPlayers;
	m_nRunSpeed = other.m_nRunSpeed;
	m_nTimeLimit = other.m_nTimeLimit;
	m_nRounds = other.m_nRounds;
	m_bFriendlyFire = other.m_bFriendlyFire;

	m_nNumTeams = other.m_nNumTeams;

	for(int i = 0; i < MAX_TEAMS; ++i )
	{
		m_nTeamModel[i] = other.m_nTeamModel[i];
		m_sTeamName[i] = other.m_sTeamName[i];
	}

	m_nFragScore = other.m_nFragScore;
	m_nTagScore = other.m_nTagScore;

	m_nDeviceCompletedScore = other.m_nDeviceCompletedScore;
	m_nLightPiecePlacedScore = other.m_nLightPiecePlacedScore;
	m_nHeavyPiecePlacedScore = other.m_nHeavyPiecePlacedScore;
	m_nPieceRemovedScore = other.m_nPieceRemovedScore;
	m_nRevivingScore = other.m_nRevivingScore;

	return *this;

}


/***********************************************************************************/

const char*	GetProfileDir( char const* pszProfileName ) 
{
	static std::string directory;
	directory = PROFILE_DIR;
	directory += pszProfileName;
	return directory.c_str();
}

const char*	GetProfileFile( char const* pszProfileName ) 
{
	static std::string sProfileFile;
	sProfileFile = PROFILE_DIR;
	sProfileFile += pszProfileName;
	sProfileFile += PROFILE_EXT;
	return sProfileFile.c_str();
}

#ifdef PROFILEUTILS_NOLF2_OPTIONS
static const char szCampaignDirs[g_knNumGameTypes][32] = 
{
	// PORT: Tron has no campaign directories (none of NOLF2's "\\Campaigns\\"
	// and "\\DM_Campaigns\\" is in any retail image), so every game type's
	// campaign lives in the profile's own directory; the table keeps NOLF2's
	// shape for the NOLF2 mission code still standing in for Tron's
	"\\",
	"\\",
	"\\",
	"\\",
	"\\",
	"\\",
	"\\",
	"\\",
};


const char*	GetCampaignDir( char const* pszProfileName, GameType eGameType ) 
{
	ASSERT( eGameType >= 0 && eGameType < g_knNumGameTypes );
	
	static std::string sCampaignDir;
	sCampaignDir = GetProfileDir( pszProfileName );
	sCampaignDir += szCampaignDirs[eGameType]; 

	return sCampaignDir.c_str();
}

const char*	GetCampaignFile( ServerGameOptions const& serverGameOptions ) 
{
	static std::string sCampaignFile;

	if (serverGameOptions.m_eGameType == eGameTypeSingle)
		return "";

	if( serverGameOptions.IsDefaultCampaign() )
	{
		sCampaignFile = GetCampaignDir( serverGameOptions.m_sProfileName.c_str( ), ( GameType )serverGameOptions.m_eGameType );
		sCampaignFile += DEFAULT_CAMPAIGN_FILE;
	}
	else
	{
		sCampaignFile = GetCampaignDir( serverGameOptions.m_sProfileName.c_str( ), ( GameType )serverGameOptions.m_eGameType );
		sCampaignFile += serverGameOptions.GetCampaignName(); 
		sCampaignFile += ".txt";
	}

	return sCampaignFile.c_str();
}
#endif // PROFILEUTILS_NOLF2_OPTIONS
