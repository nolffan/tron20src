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
#include "LightCycleMgr.h"

class Body;

class CTronGameServerShell : public CGameServerShell
{
	public :

		declare_interface(CTronGameServerShell);

        CTronGameServerShell();
		~CTronGameServerShell();

		virtual bool DropInventoryObject(Body* pBody);

		// RETAIL: CGameServerShell's four pure slots (GameServerShell.h): Tron's
		// GameSpy reporting, retail 0x1005d3f0, 0x1005d600, 0x1005d680 and
		// 0x1005d6b0. Stand-ins until written from the retail code (GameSpy is
		// not yet linked)
		virtual void	TronUnknownVirtual4C() {}
		virtual void	TronUnknownVirtual50() {}
		virtual bool	TronUnknownVirtual54() { return false; }
		virtual void	TronUnknownVirtual58( uint8 senderAddr[4], uint16 senderPort, void* pData, uint32 nDataLen ) {}

	protected:
		virtual LTRESULT OnServerInitialized();
		virtual void OnMessage(HCLIENT hSender, ILTMessage_Read *pMsg);

		virtual void Update(LTFLOAT timeElapsed);

		// Tron specific MID_ messages
		void HandlePlayerCompile(HCLIENT hSender, ILTMessage_Read *pMsg);

	private:

		virtual CPlayerObj* CreatePlayer(HCLIENT hClient, ModelId ePlayerModelId); // PORT: NOLF2's interface
		CTronVersionMgr		m_VersionMgr; // Same as g_pVersionMgr
		CTronMissionButeMgr	m_MissionButeMgr;	// Same as g_pMissionButeMgr
		CLightCycleMgr m_LightCycleMgr; // Same as g_pLightCycleMgr
};


#endif  // __TRON_GAME_SERVER_SHELL_H__