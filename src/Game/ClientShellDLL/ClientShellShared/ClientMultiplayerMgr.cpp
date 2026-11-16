// ----------------------------------------------------------------------- //
//
// MODULE  : ClientMultiplayerMgr.cpp
//
// PURPOSE : Clientside multiplayer mgr - Definition
//
// CREATED : 02/05/02
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ServerOptionMgr.h"
#include "ClientMultiplayerMgr.h"
#include "GameClientShell.h"
#include "MsgIds.h"
#include "CharacterFx.h"
#include "CMoveMgr.h"
#include "MissionMgr.h"
#include "MissionButeMgr.h"
#include "InterfaceMgr.h"
#include "ProfileMgr.h"
#include "WeaponMgr.h"
#include "CRC32.h"

ClientMultiplayerMgr* g_pClientMultiplayerMgr = NULL;


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::ClientMultiplayerMgr
//
//	PURPOSE:	Constructor
//
// --------------------------------------------------------------------------- //

ClientMultiplayerMgr::ClientMultiplayerMgr( )
{
	m_nServerPort = -1;
	m_nServerKey = 0;
	m_nDisconnectCode = 0;
	m_bForceDisconnect = false;
	m_aCurMessageSourceAddr[0] = 0;
	m_aCurMessageSourceAddr[1] = 0;
	m_aCurMessageSourceAddr[2] = 0;
	m_aCurMessageSourceAddr[3] = 0;
	m_nCurMessageSourcePort = 0;

	memset( &m_StartGameRequest, 0, sizeof( m_StartGameRequest ));
	m_StartGameRequest.m_Type = GAMEMODE_NONE;
	memset( &m_NetClientData, 0, sizeof( m_NetClientData ));

	m_nLastConnectionResult = LT_OK;

	g_pClientMultiplayerMgr = this;

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::~ClientMultiplayerMgr
//
//	PURPOSE:	Destructor
//
// --------------------------------------------------------------------------- //

ClientMultiplayerMgr::~ClientMultiplayerMgr( )
{
	g_pClientMultiplayerMgr = NULL;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::InitSinglePlayer
//
//	PURPOSE:	Send the server the initial single player info
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::InitSinglePlayer()
{

	g_pWeaponMgr->LoadOverrideButes( WEAPON_DEFAULT_FILE );

	//force server to update difficulty
	GameDifficulty eDiff = g_pGameClientShell->GetDifficulty();
	g_pGameClientShell->SetDifficulty(eDiff);

	//force server to update performance settings
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	pProfile->SendPerformanceMsg();


	return true;
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::InitMultiPlayer
//
//	PURPOSE:	Send the server the initial multiplayer info
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::InitMultiPlayer()
{
	if (!IsMultiplayerGame())
		return false;

	// RETAIL: Tron's (0x100e84f0) sends the profile's model, colour and
	// colour choice, and has no team
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (pProfile->m_nCPPlayerModel >= g_pModelButeMgr->GetNumMPModels())
		pProfile->m_nCPPlayerModel = 0;

	CAutoMessage cMsg;
  
	cMsg.Writeuint8( MID_PLAYER_MULTIPLAYER_INIT );
	cMsg.Writeuint8( pProfile->m_nCPPlayerModel );
	cMsg.Writeuint8( pProfile->m_nPlayerColor );
	cMsg.Writeuint8( pProfile->m_nDMPlayerModel );

    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	//force server to update difficulty
	g_pGameClientShell->SetDifficulty(GD_NORMAL);

	//force server to update performance settings
	pProfile->SendPerformanceMsg();

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::UpdateMultiPlayer
//
//	PURPOSE:	Send the server the updated multiplayer info
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::UpdateMultiPlayer()
{
	if (!IsMultiplayerGame())
		return false;

	// RETAIL: Tron's (0x100e8650)
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (pProfile)
	{
		if (pProfile->m_nCPPlayerModel >= g_pModelButeMgr->GetNumMPModels())
			pProfile->m_nCPPlayerModel = 0;

		CAutoMessage cMsg;
  
		cMsg.Writeuint8( MID_PLAYER_INFOCHANGE );
		cMsg.WriteString( pProfile->m_sPlayerName.c_str() );
		cMsg.Writeuint8( pProfile->m_nCPPlayerModel );
		cMsg.Writeuint8( pProfile->m_nPlayerColor );
		cMsg.Writeuint8( pProfile->m_nDMPlayerModel );

		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
	}

	return true;
}

// returns true if the passed in address matches the current server address





bool ClientMultiplayerMgr::SetupClient(char const* pszIpAddress, char const* pszPassword)
{
	// Check inputs.
	if( !pszIpAddress || !pszIpAddress[0] )
	{
		ASSERT( !"ClientMultiplayerMgr::SetupClient: Invalid inputs." );
		return false;
	}

	// Start the game...

	memset( &m_StartGameRequest, 0, sizeof( m_StartGameRequest ));
	memset( &m_NetClientData, 0, sizeof( m_NetClientData ));
	m_ServerGameOptions.Clear( );

	// RETAIL: Tron has no UpdateNetClientData: SetupClient and
	// SetupServerHost fill the client data themselves
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	// Check if the user overrode the player name on the command line.
	char szPlayerNameOverride[MAX_PLAYER_NAME] = "";
	LTStrCpy( szPlayerNameOverride, GetConsoleTempString( "playername", "" ), ARRAY_LEN( szPlayerNameOverride ));
	if( szPlayerNameOverride[0] )
	{
		pProfile->m_sPlayerName = szPlayerNameOverride;
	}

	LTStrCpy( m_NetClientData.m_szName, pProfile->m_sPlayerName.c_str(), ARRAY_LEN( m_NetClientData.m_szName ));
	LTStrCpy( m_NetClientData.m_szPlayerGuid, pProfile->m_sPlayerGuid.c_str(), ARRAY_LEN( m_NetClientData.m_szPlayerGuid ));

	// RETAIL: Tron's model is the profile's multiplayer model; the colour
	// follows the profile's colour choice
	m_NetClientData.m_ePlayerModelId = g_pModelButeMgr->GetMPModel( pProfile->m_nCPPlayerModel )->m_eModelId;
	pProfile->m_nPlayerColor = pProfile->m_nDMPlayerModel;
	m_NetClientData.m_nPlayerColor = pProfile->m_nPlayerColor;

	m_StartGameRequest.m_Type = STARTGAME_CLIENTTCP;
	strncpy( m_StartGameRequest.m_TCPAddress, pszIpAddress, MAX_SGR_STRINGLEN);

	if (pszPassword)
	{
		m_nClientPass = str_Hash(pszPassword);
	}
	else
	{
		m_nClientPass = 0;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::SetupServerSinglePlayer
//
//	PURPOSE:	Setup server for singleplayer game.
//
// ----------------------------------------------------------------------- //
bool ClientMultiplayerMgr::SetupServerSinglePlayer( )
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	memset(&m_StartGameRequest, 0, sizeof(StartGameRequest));
	m_ServerGameOptions.Clear( );
	memset( &m_NetClientData, 0, sizeof( m_NetClientData ));

	// Set the game type to normal (single player).
	m_StartGameRequest.m_Type = STARTGAME_NORMAL;

	m_ServerGameOptions = (*g_pServerOptionMgr->GetCurrent());
	m_ServerGameOptions.m_bPreCacheAssets = (pProfile->m_sPerformance.nSettings[kPerform_PreCacheAssets] > 0);
	m_ServerGameOptions.m_bPerformanceTest = g_pGameClientShell->IsRunningPerformanceTest();

	return true;
}
// --------------------------------------------------------------------------- //
//
//  ROUTINE:    ClientMultiplayerMgr::SetupServerHost()
//
//  PURPOSE:    Host a game.
//
//	PARAMETERS:	int nPort - Port to use, 0 to use default
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::SetupServerHost( int nPort )
{
	memset( &m_StartGameRequest, 0, sizeof( m_StartGameRequest ));
	m_ServerGameOptions.Clear( );
	memset( &m_NetClientData, 0, sizeof( m_NetClientData ));
	
	ServerGameOptions *pOptions = g_pServerOptionMgr->GetCurrent();
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	m_ServerGameOptions = *pOptions;
	m_ServerGameOptions.m_bPreCacheAssets = (pProfile->m_sPerformance.nSettings[kPerform_PreCacheAssets] > 0);

	// Check if the user overrode the player name on the command line.
	char szPlayerNameOverride[MAX_PLAYER_NAME] = "";
	LTStrCpy( szPlayerNameOverride, GetConsoleTempString( "playername", "" ), ARRAY_LEN( szPlayerNameOverride ));
	if( szPlayerNameOverride[0] )
	{
		pProfile->m_sPlayerName = szPlayerNameOverride;
	}

	LTStrCpy( m_NetClientData.m_szName, pProfile->m_sPlayerName.c_str(), ARRAY_LEN( m_NetClientData.m_szName ));
	LTStrCpy( m_NetClientData.m_szPlayerGuid, pProfile->m_sPlayerGuid.c_str(), ARRAY_LEN( m_NetClientData.m_szPlayerGuid ));

	// RETAIL: Tron's model is the profile's multiplayer model; the colour
	// follows the profile's colour choice
	m_NetClientData.m_ePlayerModelId = g_pModelButeMgr->GetMPModel( pProfile->m_nCPPlayerModel )->m_eModelId;
	pProfile->m_nPlayerColor = pProfile->m_nDMPlayerModel;
	m_NetClientData.m_nPlayerColor = pProfile->m_nPlayerColor;

	m_StartGameRequest.m_Type = STARTGAME_HOST;
	m_StartGameRequest.m_HostInfo.m_Port = nPort;

	// RETAIL: Tron's game types (0x100e8a00)
	switch (pOptions->m_eGameType)
	{
	default:
	    m_StartGameRequest.m_HostInfo.m_dwMaxConnections = 1;
		break;
	case eGameTypeMultiplayerLightcycles:
	    m_StartGameRequest.m_HostInfo.m_dwMaxConnections = pOptions->m_nLCMaxPlayers-1;
		break;
	case eGameTypeDiscTournament:
	    m_StartGameRequest.m_HostInfo.m_dwMaxConnections = pOptions->m_nTournMaxPlayers-1;
		break;
	case eGameTypeDiscArena:
	    m_StartGameRequest.m_HostInfo.m_dwMaxConnections = pOptions->m_nArenaMaxPlayers-1;
		break;
	};

	LTStrCpy( m_StartGameRequest.m_HostInfo.m_sName, pOptions->m_sSessionName.c_str( ),
				sizeof(m_StartGameRequest.m_HostInfo.m_sName));

	m_StartGameRequest.m_HostInfo.m_bHasPassword = pOptions->m_bUsePassword;
	
	// Set the name of the mod we are using...
	m_ServerGameOptions.m_sModName = GetModName();

    return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::SetDisconnectCode
//
//	PURPOSE:	Sets the disconnection code and message
//
// --------------------------------------------------------------------------- //

void ClientMultiplayerMgr::SetDisconnectCode(uint32 nCode, const char *pMsg)
{
	// Don't override what someone already told us
	if (m_nDisconnectCode)
		return;

	m_nDisconnectCode = nCode;

	if( pMsg )
		m_sDisconnectMsg = pMsg;
	else
		m_sDisconnectMsg.Empty( );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::ClearDisconnectCode
//
//	PURPOSE:	Clears the disconnection code and message
//
// --------------------------------------------------------------------------- //

void ClientMultiplayerMgr::ClearDisconnectCode()
{
	m_nDisconnectCode = 0;
	m_sDisconnectMsg.Empty( );
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::SetCurMessageSource
//
//	PURPOSE:	Set the source address of the message which is currently being processed
//
// --------------------------------------------------------------------------- //

void ClientMultiplayerMgr::SetCurMessageSource(const uint8 aAddr[4], uint16 nPort)
{
	m_aCurMessageSourceAddr[0] = aAddr[0];
	m_aCurMessageSourceAddr[1] = aAddr[1];
	m_aCurMessageSourceAddr[2] = aAddr[2];
	m_aCurMessageSourceAddr[3] = aAddr[3];
	m_nCurMessageSourcePort = nPort;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::OnMessage()
//
//	PURPOSE:	Handle client messages
//
// ----------------------------------------------------------------------- //

bool ClientMultiplayerMgr::OnMessage(uint8 messageID, ILTMessage_Read *pMsg)
{
	// Check inputs.
	if( !pMsg )
	{
		ASSERT( !"ClientMultiplayerMgr::OnMessage: Invalid msg." );
		return false;
	}

	switch(messageID)
	{
		case MID_HANDSHAKE:
			return HandleMsgHandshake( *pMsg );
			break;
		case MID_PLAYER_SINGLEPLAYER_INIT:	
			return HandleMsgPlayerSingleplayerInit	( *pMsg);	
			break;
		case MID_PLAYER_MULTIPLAYER_INIT:	
			return HandleMsgPlayerMultiplayerInit( *pMsg );
			break;
		case MID_MULTIPLAYER_DATA:
			return HandleMsgMultiplayerData( *pMsg );
			break;
		// RETAIL: no MID_MULTIPLAYER_SERVERDIR in Tron (retail 0x100e8cd0)
		default:
			break;
		
	}

	return false;
}


bool ClientMultiplayerMgr::HandleMsgHandshake( ILTMessage_Read & msg )
{
	int nHandshakeSub = (int)msg.Readuint8();
	switch (nHandshakeSub)
	{
		case MID_HANDSHAKE_HELLO :
		{
			int nHandshakeVer = (int)msg.Readuint16();
			if (nHandshakeVer != GAME_HANDSHAKE_VER)
			{
				// Disconnect
				m_bForceDisconnect = true;
				SetDisconnectCode(eDisconnect_NotSameGUID,NULL);

				return true;
			}

			// Send back a hello response
			CAutoMessage cResponse;
			cResponse.Writeuint8(MID_HANDSHAKE);
			cResponse.Writeuint8(MID_HANDSHAKE_HELLO);
		    cResponse.Writeuint16(GAME_HANDSHAKE_VER);
			// Send them our secret key
			cResponse.Writeuint32( GAME_HANDSHAKE_PASSWORD );
			g_pLTClient->SendToServer(cResponse.Read(), MESSAGE_GUARANTEED);
		}
		break;
		case MID_HANDSHAKE_PASSWORD:
		{
			// Read in their key
			m_nServerKey = msg.Readuint32();
			bool   bNeedPassword = msg.Readbool();

			uint32 nPassword = GAME_HANDSHAKE_PASSWORD;
			uint32 nXORMask = GAME_HANDSHAKE_MASK;

			nPassword ^= nXORMask;

			// Game password.
			uint32 nHashedPassword = 0;
			if (bNeedPassword)
			{
				nHashedPassword = m_nClientPass;
			}

			// Get the weapons file CRC
			uint32 nWeaponCRC = g_pWeaponMgr->GetFileCRC();
			// Mask that up too
			nWeaponCRC ^= nXORMask;

			// CRC the modelbutes.txt
			static uint32 nModelButesCRC = CRC32::CalcRezFileCRC( g_pModelButeMgr->GetAttributeFile( ));
			uint32 nModelButesMaskedCRC = nModelButesCRC ^ nXORMask;

			// CRC the surface.txt
			static uint32 nSurfaceCRC = CRC32::CalcRezFileCRC( g_pSurfaceMgr->GetAttributeFile( ));
			uint32 nSurfaceMaskedCRC = nSurfaceCRC ^ nXORMask;

			// CRC the damagefx.txt
			static uint32 nDamageFxCRC = CRC32::CalcRezFileCRC( "attributes\\damagefx.txt" );
			uint32 nDamageFxMaskedCRC = nDamageFxCRC ^ nXORMask;

			// Get the client shell file CRC
			char aClientShellName[MAX_PATH + 1];
			// Just in case getting the file name fails
			aClientShellName[0] = 0; 
			// Get the client shell handle from the engine
			HMODULE hClientShell;
			g_pLTClient->GetEngineHook("cshell_hinstance", (void**)&hClientShell);
			DWORD nResult = GetModuleFileName(hClientShell, aClientShellName, sizeof(aClientShellName));
			uint32 nClientCRC = CRC32::CalcFileCRC(aClientShellName);
			
			// Mask that up too
			nClientCRC ^= nXORMask;

			// Send it back their direction
			CAutoMessage cResponse;
			cResponse.Writeuint8(MID_HANDSHAKE);
			cResponse.Writeuint8(MID_HANDSHAKE_LETMEIN);
			cResponse.Writeuint32(nPassword);
			cResponse.Writeuint32(nWeaponCRC);
			cResponse.Writeuint32(nClientCRC);
			cResponse.Writeuint32(nModelButesMaskedCRC);
			cResponse.Writeuint32(nSurfaceMaskedCRC);
			cResponse.Writeuint32(nDamageFxMaskedCRC);
			cResponse.Writeuint32(nHashedPassword);
			g_pLTClient->SendToServer(cResponse.Read(), MESSAGE_GUARANTEED);

		}
		break;
		case MID_HANDSHAKE_DONE:
		{

			// This just means the server validated us...
		}
		break;
		case MID_HANDSHAKE_WRONGPASS:
		{
			// Oops... wrong password, disconnect
			m_bForceDisconnect = true;
			SetDisconnectCode(eDisconnect_WrongPassword, NULL);
		}
		break;
		default :
		{
			// Disconnect
			m_bForceDisconnect = true;
			SetDisconnectCode(eDisconnect_NotSameGUID, NULL);
		}
		break;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::HandleMsgPlayerSingleplayerInit()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool ClientMultiplayerMgr::HandleMsgPlayerSingleplayerInit (ILTMessage_Read& msg)
{
	return InitSinglePlayer();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::HandleMsgPlayerMultiplayerInit()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool ClientMultiplayerMgr::HandleMsgPlayerMultiplayerInit(ILTMessage_Read& msg)
{
	return InitMultiPlayer();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::HandleMsgMultiplayerData()
//
//	PURPOSE:	Read multiplayer data sent from server.
//
// ----------------------------------------------------------------------- //

bool ClientMultiplayerMgr::HandleMsgMultiplayerData( ILTMessage_Read& msg )
{
    GameType eGameType = ( GameType )msg.Readuint8();
	g_pGameClientShell->SetGameType( eGameType );

	// Check if some joker set us to single player.
	if( eGameType == eGameTypeSingle )
	{
		ASSERT( !"ClientMultiplayerMgr::HandleMsgMultiplayerData: Invalid game type." );
		return false;
	}

	msg.ReadString( m_sServerAddress.GetBuffer( 256 ), 256 );
	m_sServerAddress.ReleaseBuffer( );
	uint32 tmp = msg.Readuint32();
	m_nServerPort = (int)tmp;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::OnEvent()
//
//	PURPOSE:	Called for asynchronous errors that cause the server
//				to shut down
//
// ----------------------------------------------------------------------- //

void ClientMultiplayerMgr::OnEvent(uint32 dwEventID, uint32 dwParam)
{
	if( !IsMultiplayerGame( ))
		return;

	switch(dwEventID)
	{
		// Client disconnected from server.  dwParam will
		// be a error flag found in de_codes.h.
		case LTEVENT_DISCONNECT :
		{
			m_sServerAddress.Empty( );
			m_nServerPort = -1;
			m_sServerName.Empty( );
			m_nServerKey = 0;
		} 
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::Update()
//
//	PURPOSE:	Frame update.
//
// ----------------------------------------------------------------------- //

void ClientMultiplayerMgr::Update( )
{
	// This will happen when something wanted to disconnect, but wasn't
	// in a valid location to do so.  (e.g. when processing packets..)
	if (m_bForceDisconnect)
	{
		g_pLTClient->Disconnect();
		m_bForceDisconnect = false;
		return;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::SetService
//
//	PURPOSE:	Selects the connection service for hosting/joining internet games.
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::SetService( )
{
	NetService *pCur, *pListHead;
	HNETSERVICE hNetService;

	pCur      = NULL;
	pListHead = NULL;
	hNetService = NULL;

    if( g_pLTClient->GetServiceList( pListHead ) != LT_OK || !pListHead )
        return false;

	// Find the service specified.
	pCur = pListHead;
	while( pCur )
	{
		if( pCur->m_dwFlags & NETSERVICE_TCPIP )
		{
			hNetService = pCur->m_handle;
			break;
		}

		pCur = pCur->m_pNext;
	}

	// Free the service list.
    g_pLTClient->FreeServiceList( pListHead );

	// Check if tcp not found.
	if( !hNetService )
        return false;

	// Select it.
    if( g_pLTClient->SelectService( hNetService ) != LT_OK )
        return false;

    return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::StartClient
//
//	PURPOSE:	Start a client of a remote server.
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::StartClient( )
{

	// Initialize the networking.  Always start a new server with hosted games.
    m_nLastConnectionResult = g_pLTClient->InitNetworking(NULL, 0);
	if (m_nLastConnectionResult != LT_OK)
	{
        return false;
	}

	// Initialize our protocol.
	if (!SetService())
        return false;

	// Hook up the netgame and clientinfo.
	m_StartGameRequest.m_pClientData = &m_NetClientData;
	m_StartGameRequest.m_ClientDataLen = sizeof( m_NetClientData );

	int nRetries = 0;
	while (nRetries >= 0)
	{
		// If successful, then we're done.
		m_nLastConnectionResult = g_pLTClient->StartGame( const_cast< StartGameRequest * >( &m_StartGameRequest ));
		if( m_nLastConnectionResult == LT_OK )
		{
			return true;
		}

		// If we didn't timeout, then there's no reason to try again.
		if( m_nLastConnectionResult != LT_TIMEOUT )
			break;

		// Wait a sec and try again.
		Sleep(1000);
		nRetries--;
	}

	
	return false;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::StartServerHost
//
//	PURPOSE:	Start a hosted game.
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::StartServerAsHost( )
{
	// RETAIL: Tron's tells the server the profile's name (0x100e9370)
	m_ServerGameOptions.m_sProfileName = g_pProfileMgr->GetCurrentProfileName( );

	// If they want a dedicated server, then launch the serverapp.
	if( m_ServerGameOptions.m_bDedicated )
	{
		// RETAIL: the name of the option set, not of the profile
		if( !g_pGameClientShell->LauncherServerApp( m_ServerGameOptions.m_sOptionsName.c_str( )))
			return false;

		return true;
	}

	// Check if we're already connected to a server.
  	if( g_pLTClient->IsConnected( ))
	{
  		// Check if we are already hosting mp.
  		if( IsMultiplayerGame( ) && m_StartGameRequest.m_Type == STARTGAME_HOST )
  		{
  			// Don't need to restart a server.
  			return true;
		}
	}

	// Initialize the networking.  Always start a new server with hosted games.
    m_nLastConnectionResult = g_pLTClient->InitNetworking(NULL, 0);
	if (m_nLastConnectionResult != LT_OK)
	{
        return false;
	}

	// Initialize our protocol.
	if (!SetService())
        return false;

	// Hook up the netgame and clientinfo.
	ServerGameOptions* pServerGameOptions = &m_ServerGameOptions;
	m_StartGameRequest.m_pGameInfo = &pServerGameOptions;
	m_StartGameRequest.m_GameInfoLen = sizeof( pServerGameOptions );
	m_StartGameRequest.m_pClientData = &m_NetClientData;
	m_StartGameRequest.m_ClientDataLen = sizeof( m_NetClientData );

	// Start the server.
	m_nLastConnectionResult = g_pLTClient->StartGame( const_cast< StartGameRequest * >( &m_StartGameRequest ));
	return ( m_nLastConnectionResult == LT_OK );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::StartServerSinglePlayer
//
//	PURPOSE:	Starts the single player server.
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::StartServerAsSinglePlayer( )
{
	// Check if we're already connected to a server.
  	if( g_pLTClient->IsConnected( ))
	{
  		// Check if we are already running sp server.
  		if( !IsMultiplayerGame( ) && m_StartGameRequest.m_Type == STARTGAME_NORMAL )
  		{
  			// Don't need to restart a server.
  			return true;
		}
	}

	// Make sure the profile is set.
	m_ServerGameOptions.m_eGameType = g_pGameClientShell->GetGameType( );
	// RETAIL: Tron's tells the server the profile's name (0x100e94c0)
	m_ServerGameOptions.m_sProfileName = g_pProfileMgr->GetCurrentProfileName( );
	ServerGameOptions* pServerGameOptions = &m_ServerGameOptions;
	m_StartGameRequest.m_pGameInfo = &pServerGameOptions;
	m_StartGameRequest.m_GameInfoLen = sizeof( pServerGameOptions );


	// Start with clean slate
	m_StartGameRequest.m_Type = STARTGAME_NORMAL;

    m_nLastConnectionResult = g_pLTClient->StartGame(&m_StartGameRequest);
	return ( m_nLastConnectionResult == LT_OK );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::StartClientServer
//
//	PURPOSE:	Starts a client/server based on previously set startgamerequest.
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::StartClientServer( )
{
	// RETAIL: Tron's (0x100e95b0) keeps the team data here
	switch( m_StartGameRequest.m_Type )
	{
		case STARTGAME_NORMAL:
			return StartServerAsSinglePlayer( );
			break;
		case STARTGAME_HOST:
			return StartServerAsHost( );
			break;
		case STARTGAME_CLIENTTCP:
			return StartClient( );
			break;
		default:
			ASSERT( !"ClientMultiplayerMgr::StartClientServer: Invalid gamerequest type." );
			return false;
			break;
	}
}
