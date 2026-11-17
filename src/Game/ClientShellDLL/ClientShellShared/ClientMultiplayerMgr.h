// ----------------------------------------------------------------------- //
//
// MODULE  : ClientMultiplayerMgr.h
//
// PURPOSE : Clientside multiplayer mgr - Declaration
//
// CREATED : 02/05/02
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENTMULTIPLAYERMGR_H_
#define __CLIENTMULTIPLAYERMGR_H_

#include "VarTrack.h"
#include "NetDefs.h"
#include "ProfileUtils.h"

class IServerDirectory;


enum eDisconnectCodes
{
	eDisconnect_None = 0,
	eDisconnect_NotSameGUID,
	eDisconnect_WrongPassword
};


class ClientMultiplayerMgr
{
public:
	
	ClientMultiplayerMgr( );
	~ClientMultiplayerMgr( );

	// Update called once per frame.
	void		Update( );

	bool		InitMultiPlayer();
	bool		UpdateMultiPlayer();
	bool		InitSinglePlayer();

	// Disconnect on the next update
	void		ForceDisconnect() {	m_bForceDisconnect = true;}	


	// RETAIL: Tron's takes no host name (0x100e87b0)
	bool		SetupClient(char const* pszIpAddress, char const* pszPassword);

	// Setup the server for singleplayer.
	bool		SetupServerSinglePlayer( );

	// Setup the server to host a multiplayer game.
	// RETAIL: Tron's takes the port only (0x100e8a00)
	bool		SetupServerHost( int nPort );

	// Handles engine event.
	void		OnEvent(uint32 dwEventID, uint32 dwParam);

	// Handles message from engine.
	bool		OnMessage(uint8 messageID, ILTMessage_Read *pMsg);




	// Sets the networking service.
	bool		SetService( );

	// Accessors to setup server information.
	StartGameRequest& GetStartGameRequest( ) { return m_StartGameRequest; }
	ServerGameOptions&	GetServerGameOptions( ) { return m_ServerGameOptions; }
	NetClientData&	GetNetClientData( ) { return m_NetClientData; }

	// Start a client of a server based on previously set startgamerequest.
	bool		StartClientServer( );


	// This client is connected to remote server.
	// RETAIL: Tron's tests the connection first (inlined in CPlayerMgr::
	// UpdatePlayerInfo, CShell 0x1012a3a0)
	bool		IsConnectedToRemoteServer( ) { return ( g_pLTClient->IsConnected() && m_StartGameRequest.m_Type == STARTGAME_CLIENTTCP ); }

	//returns true if the passed in address matches the current server address
	// Called when the engine wants to tell the game a disconnection code (a.k.a. hack)
	virtual void SetDisconnectCode(uint32 nCode, const char *pMsg);
	// Internal game-side support for the disconnection code
	void		ClearDisconnectCode();
	uint32		GetDisconnectCode() { return m_nDisconnectCode; }
	const char *GetDisconnectMsg() { return m_sDisconnectMsg; }

	const char *GetServerAddress() const {return m_sServerAddress;}
	const char *GetServerName() const {return m_sServerName;}

	// Server directory access
	// Note : The server directory is owned by this object once you pass it in.

	// Get/Set the source address of the message which is currently being processed
	void		SetCurMessageSource(const uint8 aAddr[4], uint16 nPort);

	LTRESULT	GetLastConnectionResult() {return m_nLastConnectionResult;}


	void		SetModName( const char *pszModName ) { m_sModName = pszModName; }
	const char*	GetModName( ) const { return m_sModName.c_str(); }
	
protected :


	// RETAIL: virtual in Tron (retail ClientMultiplayerMgr's vtable, slot 1:
	// 0x100e8dd0 reads the handshake's sub-message and switches on it)
	virtual bool	HandleMsgHandshake( ILTMessage_Read& msg );
	bool		HandleMsgMultiplayerData( ILTMessage_Read& pMsg );
	bool		HandleMsgPlayerMultiplayerInit( ILTMessage_Read& msg );
	bool		HandleMsgPlayerSingleplayerInit (ILTMessage_Read& msg);

	bool		StartServerAsSinglePlayer( );
	bool		StartServerAsHost( );
	bool		StartClient( );

private:

	// RETAIL: Tron's handshake (0x10096ac0) uses the key, the password and
	// the disconnect flag
	friend class TronClientMultiplayerMgr;

	// Must be filled out before StartLocalGame, StartNetworkGameAsXXX calls.
	StartGameRequest	m_StartGameRequest;
	ServerGameOptions	m_ServerGameOptions;
	NetClientData	m_NetClientData;


	//these refer to the server we are currently connected to
	CString		m_sServerAddress;
	int			m_nServerPort;
	CString		m_sServerName;

	//used for handshaking with server
	uint32		m_nServerKey;
	uint32		m_nClientPass;

	// Disconnection code/msg storage
	uint32		m_nDisconnectCode;
	CString		m_sDisconnectMsg;

	LTRESULT	m_nLastConnectionResult;

	// Connection handling
	bool			m_bForceDisconnect;	// Set this flag to disconnect on the next update


	// Current message source
	uint8		m_aCurMessageSourceAddr[4];
	uint16		m_nCurMessageSourcePort;

	
	// The name of our selected mod...
	std::string		m_sModName;
	
};

extern ClientMultiplayerMgr* g_pClientMultiplayerMgr;

#endif // __CLIENTMULTIPLAYERMGR_H_

