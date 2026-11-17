// ----------------------------------------------------------------------- //
//
// MODULE  : TronGameServerShell.cpp
//
// PURPOSE : The game's server shell - Implementation
//
// CREATED : 9/17/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronGameServerShell.h"
#include "GameStartPoint.h"
#include "TronPlayerObj.h"
#include "CoreDump.h"
#include "Body.h"
#include "LightCycleMatchMgr.h"
#include "ServerDiscMatchMgr.h"
#include "GameSpy/GameSpySDK/gcdkey/gcdkeys.h"
#include "DebugLineSystem.h"	// RETAIL: its colors are constructed here (retail 0x1005bdf0..0x1005bfa0)

// RETAIL: Tron's GameSpy product: the game name, its secret key (built on
// the stack, retail 0x1005d452) and the CD key product id
#define GAMESPY_GAME_NAME		"tron20"
#define GAMESPY_CDKEY_PRODUCT	670

// RETAIL: Tron's message ids for a GameSpy error (retail 0x1005da80); not in
// MsgIDs.h/NetDefs.h yet (the names are ours)
#define SERVERAPP_GAMESPYERROR	7	// to the server app
#define MID_TRON_SERVERMESSAGE	200	// to the host's client, with:
#define TRON_SERVERMESSAGE_GAMESPYERROR	100

// RETAIL: Tron's handshake answer to a bad CD key (retail 0x1005eb10; ours)
#define MID_HANDSHAKE_BADCDKEY	5

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::CTronGameServerShell()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //
CTronGameServerShell::CTronGameServerShell() : CGameServerShell()
{
	// RETAIL: the GameSpy version is the handshake version's (0x1005c231)
	sprintf( m_szGameSpyVersion, "%d.%.3d", GAME_HANDSHAKE_VER_MAJOR, GAME_HANDSHAKE_VER_MINOR - 1 );

	m_bGameSpyInitialized = false;
	m_pDiscMatchMgr = LTNULL;
	m_pLightCycleMatchMgr = LTNULL;
	m_bTronUnknown1D0E4 = false;
	m_nLevelSwitchArg = 0;
	m_bLevelSwitchRequested = false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::~CTronGameServerShell()
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //
CTronGameServerShell::~CTronGameServerShell()
{
	// RETAIL: retail 0x1005c698
	delete m_pDiscMatchMgr;
	delete m_pLightCycleMatchMgr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::OnServerInititalized
//
//	PURPOSE:	What to do when the server is initialized
//
// ----------------------------------------------------------------------- //
LTRESULT CTronGameServerShell::OnServerInitialized()
{
	LTRESULT res = CGameServerShell::OnServerInitialized();
	
	m_LightCycleMgr.Init();

	// RETAIL: retail 0x1005cb6e
	g_pTronTeamMgr = &m_TeamMgr;
	m_TeamMgr.Init();

	// RETAIL: retail 0x1005cb7f: the light cycle game type's manager
	if (g_pGameServerShell->GetGameType() == eGameTypeMultiplayerLightcycles)
	{
		delete m_pLightCycleMatchMgr;
		m_pLightCycleMatchMgr = new CLightCycleMatchMgr;
		m_pLightCycleMatchMgr->Init();
	}

	m_PingButeMgr.Init("Attributes/pings.txt");
	m_BossButeMgr.Init("Attributes\\bosses.txt");

	// RETAIL: retail 0x1005cc16: the disc game types' manager
	if (IsDiscGameType())
	{
		delete m_pDiscMatchMgr;
		m_pDiscMatchMgr = new CDiscMatchMgr;
		m_pDiscMatchMgr->Init();
	}

	return res;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::PreStartWorld
//
//	PURPOSE:	A world is about to start: the last one's light cycle
//				power-ups are gone
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::PreStartWorld(bool bSwitchingWorlds)
{
	CGameServerShell::PreStartWorld(bSwitchingWorlds);

	((CServerLightCycleMgr*)g_pLightCycleMgr)->RemoveAllPowerups();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::OnClientExitWorld
//
//	PURPOSE:	RETAIL: retail 0x1005d370: in multiplayer the player leaves
//				the game type's manager and the light cycles first
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::OnClientExitWorld(HCLIENT hClient)
{
	if( IsMultiplayerGame( ))
	{
		CPlayerObj* pPlayer = GetPlayerFromClientList( hClient );
		TronUnknownVirtual80( pPlayer );
		g_pLightCycleMgr->RemoveCyclist( pPlayer->m_hObject );
	}

	CGameServerShell::OnClientExitWorld( hClient );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::OnRemoveClient
//
//	PURPOSE:	A client is gone: RETAIL: retail 0x1005d3c0, only the base's
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::OnRemoveClient(HCLIENT hClient)
{
	CGameServerShell::OnRemoveClient( hClient );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::SendPlayerInfoMsgToClients
//
//	PURPOSE:	RETAIL: retail 0x1005d3d0, only the base's
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::SendPlayerInfoMsgToClients(HCLIENT hClients, CPlayerObj *pPlayer, uint8 nInfoType)
{
	CGameServerShell::SendPlayerInfoMsgToClients( hClients, pPlayer, nInfoType );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::TronUnknownVirtual4C
//
//	PURPOSE:	RETAIL: retail 0x1005d3f0: start GameSpy's query reporting on
//				the game's socket and, on the internet, its CD key checks
//
// ----------------------------------------------------------------------- //
bool CTronGameServerShell::TronUnknownVirtual4C()
{
	if( !m_bUseGameSpy || m_bGameSpyInitialized )
		return false;

	m_bUseGameSpy = LTFALSE;
	m_bGameSpyInitialized = false;

	bool bPublic;
	char szAddress[30];
	uint16 nPort = 0;
	SOCKET hSocket;
	g_pLTServer->GetTcpIpAddress( szAddress, sizeof( szAddress ), nPort );

	// RETAIL: Tron's ILTServer has its UDP socket's getter at 0x1a0
	if( g_pLTServer->GetUDPSocket( ( uint32* )&hSocket ) != LT_OK )
		return false;

	// RETAIL: a 9 byte buffer filled one byte at a time (0x1005d458), as
	// NOLF2 does
	char szSecretKey[9];
	szSecretKey[0] = 't';
	szSecretKey[1] = '9';
	szSecretKey[2] = 'D';
	szSecretKey[3] = '3';
	szSecretKey[4] = 'v';
	szSecretKey[5] = 'B';
	szSecretKey[6] = '\0';

	m_ServerGameOptions.RegisterGameSpyKeys( );

	bPublic = !m_ServerGameOptions.m_bLANOnly;

	if( qr2_init_socket( NULL, hSocket, nPort, GAMESPY_GAME_NAME, szSecretKey, bPublic, 0,
		GameSpyServerKeyCallback, GameSpyPlayerKeyCallback, GameSpyTeamKeyCallback,
		GameSpyKeyListCallback, GameSpyCountCallback, GameSpyAddErrorCallback, this ) != e_qrnoerror )
		return false;

	if( !m_ServerGameOptions.m_bLANOnly )
	{
		m_lstCDKeyClients.clear( );
		gcd_init_qr2( NULL, GAMESPY_CDKEY_PRODUCT );
	}

	m_bUseGameSpy = LTTRUE;
	m_bGameSpyInitialized = true;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::TronUnknownVirtual50
//
//	PURPOSE:	RETAIL: retail 0x1005d600: stop GameSpy
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::TronUnknownVirtual50()
{
	if( !m_bUseGameSpy || !m_bGameSpyInitialized )
		return;

	if( !m_ServerGameOptions.m_bLANOnly )
	{
		gcd_shutdown( );
		m_lstCDKeyClients.clear( );
	}

	qr2_shutdown( NULL );

	m_bUseGameSpy = LTFALSE;
	m_bGameSpyInitialized = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::TronUnknownVirtual54
//
//	PURPOSE:	RETAIL: retail 0x1005d680: GameSpy's think
//
// ----------------------------------------------------------------------- //
bool CTronGameServerShell::TronUnknownVirtual54()
{
	if( !m_bGameSpyInitialized )
		return false;

	qr2_think( NULL );

	if( !m_ServerGameOptions.m_bLANOnly )
		gcd_think( );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::TronUnknownVirtual58
//
//	PURPOSE:	RETAIL: retail 0x1005d6b0: a GameSpy query that came in on the
//				game's socket (CGameServerShell::ProcessPacket)
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::TronUnknownVirtual58( char* pData, uint32 nDataLen, uint8 senderAddr[4], uint16 senderPort )
{
	if( !m_bGameSpyInitialized )
		return;

	sockaddr_in saddr;
	memset( &saddr, 0, sizeof( saddr ));
	saddr.sin_family = AF_INET;
	uint32 nAddr = ( senderAddr[0] << 24 ) | ( senderAddr[1] << 16 ) | ( senderAddr[2] << 8 ) | senderAddr[3];
	saddr.sin_addr.s_addr = htonl( nAddr );
	saddr.sin_port = htons( senderPort );

	qr2_parse_query( NULL, pData, nDataLen, ( sockaddr* )&saddr );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::GameSpyServerKey
//
//	PURPOSE:	RETAIL: retail 0x1005d730: report a server key
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::GameSpyServerKey( int nKeyId, qr2_buffer_t pOutBuf )
{
	switch( nKeyId )
	{
		case HOSTNAME_KEY:
			qr2_buffer_add( pOutBuf, GetHostName( ));
			break;
		case GAMEVER_KEY:
			qr2_buffer_add( pOutBuf, m_szGameSpyVersion );
			break;
		case MAPNAME_KEY:
			qr2_buffer_add( pOutBuf, m_sGameSpyMapName.c_str( ));
			break;
		case GAMETYPE_KEY:
			qr2_buffer_add( pOutBuf, GetGameTypeName( ));
			break;
		case NUMPLAYERS_KEY:
			qr2_buffer_add_int( pOutBuf, GetNumPlayers( ));
			break;
		case MAXPLAYERS_KEY:
			qr2_buffer_add_int( pOutBuf, GetMaxPlayers( ));
			break;
		case GAMEMODE_KEY:
			qr2_buffer_add( pOutBuf, "openplaying" );
			break;
		case HOSTPORT_KEY:
			qr2_buffer_add_int( pOutBuf, m_ServerGameOptions.m_nPort );
			break;
		case PASSWORD_KEY:
			qr2_buffer_add_int( pOutBuf, m_ServerGameOptions.m_bUsePassword );
			break;
		case MODNAME_KEY:
			qr2_buffer_add( pOutBuf, m_ServerGameOptions.m_sModName.c_str( ));
			break;
		default:
			if( !m_ServerGameOptions.GetGameSpyKeyValue( nKeyId, pOutBuf ))
				qr2_buffer_add( pOutBuf, "" );
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::GameSpyCount
//
//	PURPOSE:	RETAIL: retail 0x1005d8c0: the number of players (no teams)
//
// ----------------------------------------------------------------------- //
int CTronGameServerShell::GameSpyCount( qr2_key_type eKeyType )
{
	switch( eKeyType )
	{
		case key_player:
			return GetNumPlayers( );
		case key_team:
			return 0;
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::GameSpyKeyList
//
//	PURPOSE:	RETAIL: retail 0x1005d8e0: the keys reported
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::GameSpyKeyList( qr2_key_type eKeyType, qr2_keybuffer_t pKeyBuffer )
{
	switch( eKeyType )
	{
		case key_server:
		{
			qr2_keybuffer_add( pKeyBuffer, HOSTNAME_KEY );
			qr2_keybuffer_add( pKeyBuffer, GAMEVER_KEY );
			qr2_keybuffer_add( pKeyBuffer, MAPNAME_KEY );
			qr2_keybuffer_add( pKeyBuffer, GAMETYPE_KEY );
			qr2_keybuffer_add( pKeyBuffer, NUMPLAYERS_KEY );
			qr2_keybuffer_add( pKeyBuffer, MAXPLAYERS_KEY );
			qr2_keybuffer_add( pKeyBuffer, GAMEMODE_KEY );
			qr2_keybuffer_add( pKeyBuffer, HOSTPORT_KEY );
			qr2_keybuffer_add( pKeyBuffer, MODNAME_KEY );
			if( m_ServerGameOptions.m_bUsePassword )
				qr2_keybuffer_add( pKeyBuffer, PASSWORD_KEY );
		}
		break;

		case key_player:
		{
			qr2_keybuffer_add( pKeyBuffer, PLAYER__KEY );
			qr2_keybuffer_add( pKeyBuffer, SCORE__KEY );
			qr2_keybuffer_add( pKeyBuffer, PING__KEY );
		}
		break;
	}

	m_ServerGameOptions.AddGameSpyKeys( eKeyType, pKeyBuffer );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::GameSpyPlayerKey
//
//	PURPOSE:	RETAIL: retail 0x1005d9a0: report a key of the nIndex'th player
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::GameSpyPlayerKey( int nKeyId, int nIndex, qr2_buffer_t pOutBuf )
{
	CPlayerObj* pPlayer = GetFirstNetPlayer( );
	int i = 0;
	while( pPlayer && i < nIndex )
	{
		i++;
		pPlayer = GetNextNetPlayer( );
	}

	if( !pPlayer )
	{
		qr2_buffer_add( pOutBuf, "" );
		return;
	}

	switch( nKeyId )
	{
		case PLAYER__KEY:
			qr2_buffer_add( pOutBuf, pPlayer->GetNetUniqueName( ));
			break;
		case SCORE__KEY:
			qr2_buffer_add_int( pOutBuf, pPlayer->GetPlayerScore( )->GetScore( ));
			break;
		case PING__KEY:
			qr2_buffer_add_int( pOutBuf, GetPlayerPing( pPlayer ));
			break;
		default:
			qr2_buffer_add( pOutBuf, "" );
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::GameSpyTeamKey
//
//	PURPOSE:	RETAIL: no team keys (folded with the other empty functions
//				of three arguments, retail 0x100e9d30)
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::GameSpyTeamKey( int nKeyId, int nIndex, qr2_buffer_t pOutBuf )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::GameSpyAddError
//
//	PURPOSE:	RETAIL: retail 0x1005da80: the master server would not list
//				us; tell the server app, or the host's client
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::GameSpyAddError( qr2_error_t eError, char* pszErrMsg )
{
	if( pszErrMsg )
		g_pLTServer->CPrint( "GameSpy error: %s", pszErrMsg );

	if( e_qrnochallengeerror == eError || e_qrconnerror == eError || e_qrdnserror == eError )
	{
		if( m_bGameServHosted )
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8( SERVERAPP_GAMESPYERROR );
			g_pLTServer->SendToServerApp( *cMsg.Read( ));
		}
		else
		{
			CPlayerObj* pPlayer = GetFirstNetPlayer( );
			if( pPlayer )
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8( MID_TRON_SERVERMESSAGE );
				cMsg.Writeuint8( TRON_SERVERMESSAGE_GAMESPYERROR );
				cMsg.WriteString( pszErrMsg );
				g_pLTServer->SendToClient( cMsg.Read( ), pPlayer->GetClient( ), MESSAGE_GUARANTEED );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::GameSpyAuth
//
//	PURPOSE:	RETAIL: retail 0x1005eb10: the CD key server's answer for a
//				client: let it in (and respawn it), or turn it away
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::GameSpyAuth( int nLocalId, int bAuthenticated, char* pszErrMsg )
{
	HCLIENT hClient = g_pLTServer->GetClientHandle( nLocalId );
	if( !hClient )
		return;

	if( !bAuthenticated )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_HANDSHAKE );
		cMsg.Writeuint8( MID_HANDSHAKE_BADCDKEY );
		g_pLTServer->SendToClient( cMsg.Read( ), hClient, MESSAGE_GUARANTEED );
		return;
	}

	CPlayerObj* pPlayer = GetPlayerFromClientList( hClient );
	if( !pPlayer )
	{
		g_pLTServer->CPrint( "Kicking Client %d: Handshake 13", g_pLTServer->GetClientID( hClient ));
		g_pLTServer->KickClient( hClient );
		return;
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_HANDSHAKE );
	cMsg.Writeuint8( MID_HANDSHAKE_DONE );
	g_pLTServer->SendToClient( cMsg.Read( ), hClient, MESSAGE_GUARANTEED );

	pPlayer->FinishHandshake( );
	m_lstCDKeyClients.push_back( nLocalId );

	RespawnPlayer( pPlayer, hClient );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::CreatePlayer()
//
//	PURPOSE:	Create the player object, and associated it with the client.
//
// ----------------------------------------------------------------------- //
CPlayerObj* CTronGameServerShell::CreatePlayer(HCLIENT hClient, ModelId ePlayerModelId) // PORT: NOLF2's interface
{
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    theStruct.m_Rotation.Init();
	VEC_INIT(theStruct.m_Pos);
	theStruct.m_Flags = 0;

    HCLASS hClass = g_pLTServer->GetClass("CTronPlayerObj");

	GameStartPoint* pStartPoint = FindStartPoint(LTNULL);
	if (pStartPoint)
	{
		g_pLTServer->GetObjectPos(pStartPoint->m_hObject, &(theStruct.m_Pos));
	}

	CTronPlayerObj* pPlayer = NULL;
	if (hClass)
	{
		// RETAIL: the model id goes to the player in the user data
		// (0x1005ddef), as in NOLF2
		theStruct.m_UserData = ePlayerModelId;
        pPlayer = (CTronPlayerObj*) g_pLTServer->CreateObject(hClass, &theStruct);
	}

	return pPlayer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::OnMessage
//
//	PURPOSE:	Message from the client
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::OnMessage(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint8 messageID = pMsg->Readuint8();

	// RETAIL: retail 0x1005e0c0
	switch (messageID)
	{
		case MID_COMPILE:				HandlePlayerCompile		(hSender, pMsg);	break;
		case MID_TRON_SERVERMESSAGE:	HandleTronServerMessage	(hSender, pMsg);	break;
		case MID_LIGHT_CYCLE_INFO:		HandleLightCycleInfo	(hSender, pMsg);	break;
		case MID_USE_ENERGY:			HandleUseEnergy			(hSender, pMsg);	break;
		case MID_DISC_HOLD_DEFENSE:		HandleDiscHoldDefense	(hSender, pMsg);	break;
		case MID_TRON_EXIT_REQUEST:		HandleExitRequest		(hSender, pMsg);	break;
		case MID_TRON_DISC_FIRED:		HandleDiscFired			(hSender, pMsg);	break;
		default: CGameServerShell::OnMessage(hSender,pMsg);
	}
}

void CTronGameServerShell::Update(LTFLOAT timeElapsed)
{
	// Base class first
	CGameServerShell::Update(timeElapsed);

	// Now update the light cycle manager
	m_LightCycleMgr.Update();

	// RETAIL: retail 0x1005e20d: the game type's manager
	if (IsDiscGameType())
	{
		m_pDiscMatchMgr->Update();
	}
	else if (g_pGameServerShell->GetGameType() == eGameTypeMultiplayerLightcycles)
	{
		m_pLightCycleMatchMgr->Update();
	}

	// RETAIL: retail 0x1005e23c: a level switch waits for every client
	if (m_bLevelSwitchRequested)
	{
		CPlayerObj::PlayerObjList::const_iterator iter;
		for (iter = CPlayerObj::GetPlayerObjList().begin(); iter != CPlayerObj::GetPlayerObjList().end(); iter++)
		{
			CPlayerObj* pPlayer = *iter;
			if (pPlayer && !pPlayer->IsClientLoaded())
			{
				return;
			}
		}

		uint32 nLevel = m_nLevelSwitchArg;
		m_nLevelSwitchArg = 0;
		m_bLevelSwitchRequested = false;

		g_pLTServer->CPrint("All clients connected, performing level switch...");

		if (IsDiscGameType())
		{
			m_pDiscMatchMgr->SetNextLevel(nLevel);
		}
		else if (g_pGameServerShell->GetGameType() == eGameTypeMultiplayerLightcycles)
		{
			m_pLightCycleMatchMgr->SetNextLevel(nLevel);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::PostStartWorld
//
//	PURPOSE:	RETAIL: retail 0x1005cef0: the game type's manager starts
//				the level
//
// ----------------------------------------------------------------------- //

void CTronGameServerShell::PostStartWorld()
{
	CGameServerShell::PostStartWorld();

	if (!IsMultiplayerGame())
		return;

	if (IsDiscGameType())
	{
		LoadLevelInfo();
		m_pDiscMatchMgr->Term();
		m_pDiscMatchMgr->Reset(m_MultiplayerLevelInfoMgr.GetMaxTeamSize());
	}
	else if (GetGameType() == eGameTypeMultiplayerLightcycles)
	{
		m_pLightCycleMatchMgr->OnStartWorld();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::SetCurLevel
//
//	PURPOSE:	RETAIL: retail 0x1005cf50: keep the world's file name,
//				without its path, as GameSpy's map name
//
// ----------------------------------------------------------------------- //

void CTronGameServerShell::SetCurLevel( char const* pszWorldName )
{
	int nLen = strlen( pszWorldName );
	if( nLen )
	{
		do
		{
			if( pszWorldName[nLen - 1] == '\\' ) break;
		}
		while( --nLen );

		m_sGameSpyMapName = pszWorldName + nLen;
	}
	else
	{
		m_sGameSpyMapName = "";
	}

	CGameServerShell::SetCurLevel( pszWorldName );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::LoadLevelInfo
//
//	PURPOSE:	RETAIL: retail 0x1005d050: the level's "info.txt"
//
// ----------------------------------------------------------------------- //

void CTronGameServerShell::LoadLevelInfo()
{
	if (!IsMultiplayerGame())
		return;

	char szFile[128];
	sprintf(szFile, "%s%s", (const char*)m_sWorld, "info.txt");
	if (!m_MultiplayerLevelInfoMgr.Init(szFile))
	{
		g_pLTServer->CPrint("ERROR - Could not initialize multiplayer level info file: %s!\n", szFile);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::SendSystemMemory
//
//	PURPOSE:	RETAIL: retail 0x1005d0c0: the level's subroutines to a
//				client of the disc game types
//
// ----------------------------------------------------------------------- //

void CTronGameServerShell::SendSystemMemory( HCLIENT hClient )
{
	if (!hClient || !IsDiscGameType())
		return;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SYSTEM_MEMORY);
	cMsg.WriteString(m_MultiplayerLevelInfoMgr.m_sSystemMemory.c_str());

	cMsg.Writeuint8(m_MultiplayerLevelInfoMgr.m_lstDefaultSubs.size());
	MPLevelSubroutineList::iterator iter;
	for (iter = m_MultiplayerLevelInfoMgr.m_lstDefaultSubs.begin(); iter != m_MultiplayerLevelInfoMgr.m_lstDefaultSubs.end(); iter++)
	{
		cMsg.Writeuint32(iter->nNameId);
		cMsg.Writeint8(iter->nVersion);
	}

	cMsg.Writeuint8(m_MultiplayerLevelInfoMgr.m_lstAvailableSubs.size());
	for (iter = m_MultiplayerLevelInfoMgr.m_lstAvailableSubs.begin(); iter != m_MultiplayerLevelInfoMgr.m_lstAvailableSubs.end(); iter++)
	{
		cMsg.Writeuint32(iter->nNameId);
		cMsg.Writeint8(iter->nVersion);
	}

	g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::TronUnknownVirtual6C
//
//	PURPOSE:	RETAIL: retail 0x1005de40: a player was killed
//
// ----------------------------------------------------------------------- //

void CTronGameServerShell::TronUnknownVirtual6C( uint32 nArg1, uint32 nArg2 )
{
	if (IsDiscGameType())
	{
		m_pDiscMatchMgr->OnPlayerKilled((CPlayerObj*)nArg1, (CPlayerObj*)nArg2);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::TronUnknownVirtual70
//
//	PURPOSE:	RETAIL: retail 0x1005de70: a player respawns
//
// ----------------------------------------------------------------------- //

void CTronGameServerShell::TronUnknownVirtual70( uint32 nArg )
{
	if (IsDiscGameType())
	{
		m_pDiscMatchMgr->OnPlayerRespawn((CPlayerObj*)nArg);
	}
	else if (g_pGameServerShell->GetGameType() == eGameTypeMultiplayerLightcycles)
	{
		m_pLightCycleMatchMgr->OnPlayerRespawn((CPlayerObj*)nArg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::TronUnknownVirtual74
//
//	PURPOSE:	RETAIL: retail 0x1005dec0: a new client starts without a
//				team
//
// ----------------------------------------------------------------------- //

void CTronGameServerShell::TronUnknownVirtual74( HCLIENT hClient )
{
	if (IsDiscGameType())
	{
		m_pDiscMatchMgr->MakeSpectator(hClient);
	}
	else if (g_pGameServerShell->GetGameType() == eGameTypeMultiplayerLightcycles)
	{
		m_pLightCycleMatchMgr->AddClient(hClient);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::TronUnknownVirtual78
//
//	PURPOSE:	RETAIL: retail 0x1005df10: a client is gone: its team, and
//				its CD key's authentication
//
// ----------------------------------------------------------------------- //

void CTronGameServerShell::TronUnknownVirtual78( HCLIENT hClient )
{
	bool bLocal = !!(g_pLTServer->GetClientInfoFlags(hClient) & CIF_LOCAL);

	if (IsDiscGameType())
	{
		m_pDiscMatchMgr->OnClientRemoved(hClient);
	}
	else if (g_pGameServerShell->GetGameType() == eGameTypeMultiplayerLightcycles)
	{
		m_pLightCycleMatchMgr->RemoveClient(hClient);
	}

	if (m_bGameSpyInitialized && !m_ServerGameOptions.m_bLANOnly && !bLocal)
	{
		gcd_disconnect_user(GAMESPY_CDKEY_PRODUCT, g_pLTServer->GetClientID(hClient));
	}

	m_lstCDKeyClients.remove(g_pLTServer->GetClientID(hClient));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::TronUnknownVirtual7C
//
//	PURPOSE:	RETAIL: retail 0x1005dfe0: a player joined
//
// ----------------------------------------------------------------------- //

void CTronGameServerShell::TronUnknownVirtual7C( CPlayerObj* pPlayer )
{
	if (IsDiscGameType())
	{
		SendSystemMemory(pPlayer->GetClient());
		m_pDiscMatchMgr->OnPlayerAdded(pPlayer);
	}
	else if (g_pGameServerShell->GetGameType() == eGameTypeMultiplayerLightcycles)
	{
		m_pLightCycleMatchMgr->AddPlayer(pPlayer);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::TronUnknownVirtual80
//
//	PURPOSE:	RETAIL: retail 0x1005e040: a player left
//
// ----------------------------------------------------------------------- //

void CTronGameServerShell::TronUnknownVirtual80( CPlayerObj* pPlayer )
{
	if (IsDiscGameType())
	{
		m_pDiscMatchMgr->OnPlayerRemoved(pPlayer);
	}
	else if (g_pGameServerShell->GetGameType() == eGameTypeMultiplayerLightcycles)
	{
		m_pLightCycleMatchMgr->RemovePlayer(pPlayer);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::TronUnknownVirtual84
//
//	PURPOSE:	RETAIL: retail 0x1005e090: switch levels once every client
//				is in (Update)
//
// ----------------------------------------------------------------------- //

void CTronGameServerShell::TronUnknownVirtual84( uint32 nLevel )
{
	g_pLTServer->CPrint("Received level switch request, waiting for all players to be connected to initiate switch...");
	m_nLevelSwitchArg = nLevel;
	m_bLevelSwitchRequested = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::HandlePlayerCompile
//
//	PURPOSE:	MID_COMPILE: the client's ratings (RETAIL: retail 0x1005ed20)
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::HandlePlayerCompile(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CTronPlayerObj* pPlayer = (CTronPlayerObj*)g_pLTServer->GetClientUserData(hSender);

	if (pPlayer)
	{
		pPlayer->HandleRatings(pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::HandleTronServerMessage
//
//	PURPOSE:	MID_TRON_SERVERMESSAGE: the game type's manager's (RETAIL:
//				retail 0x1005ed50)
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::HandleTronServerMessage(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	if (IsDiscGameType())
	{
		m_pDiscMatchMgr->OnMessage(pMsg, hSender);
	}
	else if (g_pGameServerShell->GetGameType() == eGameTypeMultiplayerLightcycles)
	{
		m_pLightCycleMatchMgr->OnMessage(pMsg, hSender);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::HandleLightCycleInfo
//
//	PURPOSE:	MID_LIGHT_CYCLE_INFO: the light cycle manager's (RETAIL:
//				retail 0x1005eda0)
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::HandleLightCycleInfo(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	((CServerLightCycleMgr*)g_pLightCycleMgr)->OnMessage(pMsg, hSender);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::HandleUseEnergy
//
//	PURPOSE:	MID_USE_ENERGY (RETAIL: retail 0x1005edc0)
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::HandleUseEnergy(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CTronPlayerObj* pPlayer = (CTronPlayerObj*)g_pLTServer->GetClientUserData(hSender);

	if (pPlayer)
	{
		pPlayer->HandleEnergy(pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::HandleDiscHoldDefense
//
//	PURPOSE:	MID_DISC_HOLD_DEFENSE (RETAIL: retail 0x1005edf0)
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::HandleDiscHoldDefense(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CTronPlayerObj* pPlayer = (CTronPlayerObj*)g_pLTServer->GetClientUserData(hSender);

	if (pPlayer)
	{
		pPlayer->HandleSwat(pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::HandleExitRequest
//
//	PURPOSE:	MID_TRON_EXIT_REQUEST (RETAIL: retail 0x1005ee20)
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::HandleExitRequest(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CTronPlayerObj* pPlayer = (CTronPlayerObj*)g_pLTServer->GetClientUserData(hSender);

	if (pPlayer)
	{
		pPlayer->HandleExitRequest(pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::HandleDiscFired
//
//	PURPOSE:	MID_TRON_DISC_FIRED (RETAIL: retail 0x1005ee50)
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::HandleDiscFired(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CTronPlayerObj* pPlayer = (CTronPlayerObj*)g_pLTServer->GetClientUserData(hSender);

	if (pPlayer)
	{
		pPlayer->HandleCompile(pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::ProcessCheat
//
//	PURPOSE:	RETAIL: retail 0x1005ee80: Tron's player cheats. The
//				procedurals and points cheats go on to the base's as well
//
// ----------------------------------------------------------------------- //

void CTronGameServerShell::ProcessCheat( CPlayerObj* pPlayer, CheatCode nCheatCode, uint32 nData )
{
	CTronPlayerObj* pTronPlayer = dynamic_cast< CTronPlayerObj* >( pPlayer );
	if( pTronPlayer )
	{
		switch( nCheatCode )
		{
			case CHEAT_TRON_ENERGY:
				CheatEnergy( pTronPlayer, nData );
				return;

			case CHEAT_TRON_IDEALWEAPONS:
				CheatIdealWeapons( pTronPlayer, nData );
				return;

			case CHEAT_TRON_IDEALSUBROUTINES:
				CheatIdealSubroutines( pTronPlayer, nData );
				return;

			case CHEAT_TRON_IDEALPROCEDURALS:
				CheatIdealProcedurals( pTronPlayer, nData );
				break;

			case CHEAT_TRON_IDEALPOINTS:
				CheatIdealPoints( pTronPlayer, nData );
				break;

			case CHEAT_TRON_IDEAL:
				CheatIdealWeapons( pTronPlayer, nData );
				CheatIdealSubroutines( pTronPlayer, nData );
				CheatIdealProcedurals( pTronPlayer, nData );
				CheatIdealPoints( pTronPlayer, nData );
				return;
		}
	}

	CGameServerShell::ProcessCheat( pPlayer, nCheatCode, nData );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::Cheat*
//
//	PURPOSE:	RETAIL: retail 0x1005ef90 to 0x1005efd0 (the names are
//				ours): "mpenergy", "mpidealweapons", "mpidealsubroutines",
//				"mpidealprocedurals" and "mpidealpoints"
//
// ----------------------------------------------------------------------- //

void CTronGameServerShell::CheatEnergy( CTronPlayerObj* pPlayer, uint32 nData )
{
	if( pPlayer )
	{
		pPlayer->EnergyCheat( );
	}
}

void CTronGameServerShell::CheatIdealWeapons( CTronPlayerObj* pPlayer, uint32 nData )
{
	if( pPlayer )
	{
		pPlayer->IdealWeaponsCheat( );
	}
}

void CTronGameServerShell::CheatIdealSubroutines( CTronPlayerObj* pPlayer, uint32 nData )
{
	if( pPlayer )
	{
		pPlayer->IdealSubroutinesCheat( );
	}
}

void CTronGameServerShell::CheatIdealProcedurals( CTronPlayerObj* pPlayer, uint32 nData )
{
	if( pPlayer )
	{
		pPlayer->IdealProceduralsCheat( );
	}
}

void CTronGameServerShell::CheatIdealPoints( CTronPlayerObj* pPlayer, uint32 nData )
{
	if( pPlayer )
	{
		pPlayer->IdealPointsCheat( );
	}
}

bool CTronGameServerShell::DropInventoryObject(CCharacter* pCharacter)
{
	if (!pCharacter)
	{
		return false;
	}

	LTVector vPos;
	g_pLTServer->GetObjectPos(pCharacter->m_hObject, &vPos);

	ObjectCreateStruct ocs;
	ocs.m_Pos = vPos;
	ocs.m_Rotation.Init();

	// RETAIL: CCharacter's three items at +0xc6c are ArchiveItems (their type
	// first, -1 for none; CoreDump::SetItems takes them)
	ArchiveItem* aItems = pCharacter->m_aCoreDumpItems;

	int nItems = 0;
	for (int i = 0; i < 3; i++)
	{
		if (aItems[i].m_eType != kAIT_None)
		{
			nItems++;
		}
	}

	// Nothing to drop
	if (!nItems && pCharacter->m_damage.GetEnergy() == 0.0f)
	{
		return false;
	}

	HCLASS hClass = g_pLTServer->GetClass("CoreDump");
	if (!hClass)
	{
		return false;
	}

	CoreDump* pObj = (CoreDump*)g_pLTServer->CreateObject(hClass, &ocs);
	if (!pObj)
	{
		return false;
	}

	// RETAIL: its health is a 16-bit value of the character's model (retail
	// 0x100c61e0, the record's +0x370; paired with GetModelNameId, not
	// verified)
	float fHealth = (float)g_pModelButeMgr->GetModelNameId(pCharacter->m_eModelId);
	if (pCharacter->m_damage.GetEnergy() != 0.0f)
	{
		pObj->Init(pCharacter->m_damage.GetEnergy(), fHealth);
	}

	pObj->SetItems(aItems);
	pCharacter->m_hTronUnknownCF0 = pObj->m_hObject;

	return true;
}
