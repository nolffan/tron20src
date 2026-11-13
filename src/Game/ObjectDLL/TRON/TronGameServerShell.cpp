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
#include "GameSpy/GameSpySDK/gcdkey/gcdkeys.h"

// RETAIL: Tron's GameSpy product: the game name, its secret key (built on
// the stack, retail 0x1005d452) and the CD key product id
#define GAMESPY_GAME_NAME		"tron20"
#define GAMESPY_CDKEY_PRODUCT	670

// RETAIL: the version GameSpy reports (retail 0x1005c245 formats two .data
// ints, 1 and 18, the second less one); not yet traced to their source
#define GAMESPY_VERSION_MAJOR	1
#define GAMESPY_VERSION_MINOR	18

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
	sprintf( m_szGameSpyVersion, "%d.%.3d", GAMESPY_VERSION_MAJOR, GAMESPY_VERSION_MINOR - 1 );

	m_bGameSpyInitialized = false;
	m_pTronUnknown19BBC = LTNULL;
	m_pTronUnknown19BC0 = LTNULL;
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

	return res;
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

	char szSecretKey[] = { 't', '9', 'D', '3', 'v', 'B', '\0' };

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

	switch (messageID)
	{
		case MID_COMPILE:	HandlePlayerCompile	(hSender, pMsg);	break;
		default: CGameServerShell::OnMessage(hSender,pMsg);
	}
}

void CTronGameServerShell::Update(LTFLOAT timeElapsed)
{
	// Base class first
	CGameServerShell::Update(timeElapsed);

	// Now update the light cycle manager
	m_LightCycleMgr.Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::HandlePlayerCompile
//
//	PURPOSE:	Message from the client
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::HandlePlayerCompile(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	// Handle the compile message
	CTronPlayerObj* pPlayer = (CTronPlayerObj*)g_pLTServer->GetClientUserData(hSender);

	if (pPlayer)
	{
		pPlayer->HandleCompile(pMsg);
	}
}

bool CTronGameServerShell::DropInventoryObject(Body* pBody)
{
	// Sanity check
	ASSERT(pBody);
	if(!pBody)
		return false;

	// Check the position of the object
	LTVector vPos;
	g_pLTServer->GetObjectPos(pBody->m_hObject,&vPos);

	// Now create a Core Dump object there
	ObjectCreateStruct ocs;
	INIT_OBJECTCREATESTRUCT(ocs);
	ocs.m_Rotation.Init();
	ocs.m_Pos = vPos;

	CoreDump* pObj;
	HCLASS hClass = g_pLTServer->GetClass("CoreDump");
	if(hClass)
	{
		// Here's where we actually create the object
		pObj = (CoreDump*)g_pLTServer->CreateObject(hClass, &ocs);
		if(pObj)
		{
			// PORT: retail gives it the body's energy and health and its
			// archive items (Tron's Body, not written yet)
			pObj->Init(pBody->GetEnergy(), 0.0f);
			return true;
		}
		else
		{
			// Couldn't create the object
			ASSERT(FALSE);
		}
	}
	else
	{
		// Couldn't find the class
		ASSERT(FALSE);
	}

	return false;
}