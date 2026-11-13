// ----------------------------------------------------------------------- //
//
// MODULE  : TronPlayerObj.h
//
// PURPOSE : Player Object
//
// CREATED : 01/28/01
//
// ----------------------------------------------------------------------- //

#ifndef __TRON_GAME_SERVER_SHELL_H__
#define __TRON_GAME_SERVER_SHELL_H__

#include "GameServerShell.h"
#include "TronVersionMgr.h"
#include "TronMissionButeMgr.h"
#include "ServerLightCycleMgr.h"
#include "PingButeMgr.h"
#include "BossButeMgr.h"
#include "SubroutineDataMgr.h"
#include "MultiplayerLevelInfoMgr.h"
#include "GameSpy/GameSpySDK/qr2/qr2.h"
#include <list>

class Body;

class CTronGameServerShell : public CGameServerShell
{
	public :

		declare_interface(CTronGameServerShell);

        CTronGameServerShell();
		~CTronGameServerShell();

		virtual bool DropInventoryObject(Body* pBody);

		// RETAIL: CGameServerShell's four pure slots (GameServerShell.h): Tron's
		// GameSpy reporting, retail 0x1005d3f0 (start the query and CD key
		// SDKs), 0x1005d600 (stop them), 0x1005d680 (their think) and
		// 0x1005d6b0 (a query that came in on the game's socket). The base's
		// names are kept
		virtual bool	TronUnknownVirtual4C();
		virtual void	TronUnknownVirtual50();
		virtual bool	TronUnknownVirtual54();
		virtual void	TronUnknownVirtual58( char* pData, uint32 nDataLen, uint8 senderAddr[4], uint16 senderPort );

		// RETAIL: overrides that only call the base (retail 0x1005d3d0)
		virtual void	SendPlayerInfoMsgToClients(HCLIENT hClients, CPlayerObj *pPlayer, uint8 nInfoType);

	protected:
		virtual LTRESULT OnServerInitialized();

		// RETAIL: retail 0x1005d3c0, only the base's
		void			OnRemoveClient(HCLIENT hClient);
		virtual void OnMessage(HCLIENT hSender, ILTMessage_Read *pMsg);

		virtual void Update(LTFLOAT timeElapsed);

		// Tron specific MID_ messages
		void HandlePlayerCompile(HCLIENT hSender, ILTMessage_Read *pMsg);

	private:

		// RETAIL: the GameSpy query SDK's callbacks (retail 0x1005d550 to
		// 0x1005d5e0, after TronUnknownVirtual4C, which hands them to
		// qr2_init_socket with the shell as the user data); each calls the
		// shell's own. The names are ours
		static void	GameSpyServerKeyCallback( int nKeyId, qr2_buffer_t pOutBuf, void* pUserData )
		{
			( ( CTronGameServerShell* )pUserData )->GameSpyServerKey( nKeyId, pOutBuf );
		}
		static void	GameSpyPlayerKeyCallback( int nKeyId, int nIndex, qr2_buffer_t pOutBuf, void* pUserData )
		{
			( ( CTronGameServerShell* )pUserData )->GameSpyPlayerKey( nKeyId, nIndex, pOutBuf );
		}
		static void	GameSpyTeamKeyCallback( int nKeyId, int nIndex, qr2_buffer_t pOutBuf, void* pUserData )
		{
			( ( CTronGameServerShell* )pUserData )->GameSpyTeamKey( nKeyId, nIndex, pOutBuf );
		}
		static int	GameSpyCountCallback( qr2_key_type eKeyType, void* pUserData )
		{
			return ( ( CTronGameServerShell* )pUserData )->GameSpyCount( eKeyType );
		}
		static void	GameSpyKeyListCallback( qr2_key_type eKeyType, qr2_keybuffer_t pKeyBuffer, void* pUserData )
		{
			( ( CTronGameServerShell* )pUserData )->GameSpyKeyList( eKeyType, pKeyBuffer );
		}
		static void	GameSpyAddErrorCallback( qr2_error_t eError, char* pszErrMsg, void* pUserData )
		{
			( ( CTronGameServerShell* )pUserData )->GameSpyAddError( eError, pszErrMsg );
		}

		// RETAIL: the CD key SDK's authentication callback (retail 0x1005eaf0;
		// the name is ours)
		static void	GameSpyAuthCallback( int nGameId, int nLocalId, int bAuthenticated, char* pszErrMsg, void* pInstance )
		{
			( ( CTronGameServerShell* )pInstance )->GameSpyAuth( nLocalId, bAuthenticated, pszErrMsg );
		}

		// RETAIL: retail 0x1005d730, 0x1005d9a0, (an empty one, folded),
		// 0x1005d8c0, 0x1005d8e0, 0x1005da80 and 0x1005eb10. The names are ours
		void		GameSpyServerKey( int nKeyId, qr2_buffer_t pOutBuf );
		int			GameSpyCount( qr2_key_type eKeyType );
		void		GameSpyKeyList( qr2_key_type eKeyType, qr2_keybuffer_t pKeyBuffer );
		void		GameSpyPlayerKey( int nKeyId, int nIndex, qr2_buffer_t pOutBuf );
		void		GameSpyTeamKey( int nKeyId, int nIndex, qr2_buffer_t pOutBuf );
		void		GameSpyAddError( qr2_error_t eError, char* pszErrMsg );
		void		GameSpyAuth( int nLocalId, int bAuthenticated, char* pszErrMsg );

		virtual CPlayerObj* CreatePlayer(HCLIENT hClient, ModelId ePlayerModelId); // PORT: NOLF2's interface

		// RETAIL: the members in the order the constructor (retail 0x1005bfc0)
		// builds them, at retail's offsets. Ours are larger until CVersionMgr
		// (0x264 bytes in Tron), CButeMgr (0x40 smaller) and the light cycle
		// manager are Tron's
		CTronVersionMgr		m_VersionMgr;		// +0x17494 Same as g_pVersionMgr
		CTronMissionButeMgr	m_MissionButeMgr;	// +0x176f8 Same as g_pMissionButeMgr
		CServerLightCycleMgr	m_LightCycleMgr;	// +0x18840 Same as g_pLightCycleMgr, g_pServerLightCycleMgr (0x137c bytes in retail)
		void*				m_pTronUnknown19BBC;	// +0x19bbc RETAIL: the revive game types' manager; not yet known
		void*				m_pTronUnknown19BC0;	// +0x19bc0 RETAIL: the light cycle game type's; not yet known
		CPingButeMgr		m_PingButeMgr;		// +0x19bc4
		CBossButeMgr		m_BossButeMgr;		// +0x1ace8
		CSubroutineDataMgr	m_SubroutineDataMgr;	// +0x1be50
		CMultiplayerLevelInfoMgr	m_MultiplayerLevelInfoMgr;	// +0x1bf64
		bool				m_bGameSpyInitialized;	// +0x1d0c0 (ours)
		std::string			m_sGameSpyMapName;		// +0x1d0c4 GameSpy's "mapname" (ours)
		char				m_szGameSpyVersion[16];	// +0x1d0d0 GameSpy's "gamever", "%d.%.3d" (ours)
		std::list<int>		m_lstCDKeyClients;		// +0x1d0e0 the client ids whose CD key passed (ours)
		bool				m_bTronUnknown1D0E4;	// +0x1d0e4
		bool				m_bLevelSwitchRequested;	// +0x1d0e5 (ours; retail 0x1005e090)
		uint32				m_nLevelSwitchArg;		// +0x1d0e8 (ours; retail 0x1005e090)
		// RETAIL: +0x1d0ec, an object of a class not yet written (its
		// constructor, retail 0x1005c2c0, builds 17 entries of 0x2c bytes)
};


#endif  // __TRON_GAME_SERVER_SHELL_H__
