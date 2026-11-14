// ----------------------------------------------------------------------- //
//
// MODULE  : TronInterfaceMgr.h
//
// PURPOSE : Manage all interface related functionality
//
// CREATED : 11/5/01
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TRONINTERFACE_MGR_H__
#define __TRONINTERFACE_MGR_H__

#include "ServerOptionMgr.h"
#include "LightCycleGameDataMgr.h"
#include "InterfaceMgr.h"
#include "TRONHUDMgr.h"
#include "TRONScreenMgr.h"
#include "TRONPlayerStats.h"
#include "SubroutineMgr.h"
#include "TronLayoutMgr.h"
#include "ScreenSpriteMgr.h"
#include "DuplicateSubroutineDlg.h"
#include <list>
#include <vector>

// RETAIL: the records CTronInterfaceMgr keeps in a list at +0x28b20 and
// saves/loads with it (Save 0x1009eee0, Load 0x1009efb0; their own Save
// 0x1009f620 and Load 0x1009f710). Unidentified; the names are ours.
struct TronInterfaceRecord
{
	// RETAIL: inlined in the allocation (0x1009fec0)
	TronInterfaceRecord()
	{
		m_szName[0] = 0;
		m_bTronUnknown40 = false;
		m_bTronUnknown64 = false;
		m_fTronUnknown44 = 0;
		m_fTronUnknown4C = 0;
		m_fTronUnknown50 = 0;
		m_fTronUnknown54 = 0;
		m_fTronUnknown58 = 0;
		m_fTronUnknown5C = 0;
		m_fTronUnknown60 = 0;
		m_fTronUnknown68 = 0;
		m_fTronUnknown6C = 0;
	}

	void	Save(ILTMessage_Write *pMsg);
	void	Load(ILTMessage_Read *pMsg);

	char	m_szName[64];		// +0x0
	bool	m_bTronUnknown40;	// +0x40
	float	m_fTronUnknown44;	// +0x44
	uint32	m_nTronUnknown48;	// +0x48, not saved
	float	m_fTronUnknown4C;	// +0x4c
	float	m_fTronUnknown50;	// +0x50
	float	m_fTronUnknown54;	// +0x54
	float	m_fTronUnknown58;	// +0x58
	float	m_fTronUnknown5C;	// +0x5c
	float	m_fTronUnknown60;	// +0x60
	bool	m_bTronUnknown64;	// +0x64
	float	m_fTronUnknown68;	// +0x68
	float	m_fTronUnknown6C;	// +0x6c
};

// RETAIL: the 0x180-byte elements of CTronInterfaceMgr's vector at
// +0x29484 (pushed by 0x1009f0c0, taken by UpdatePlayingState 0x1009e6f0;
// deallocated by 0x1009fba0): a subroutine picked up while the duplicate
// subroutine box was up. The names are ours
struct TronPendingSubroutine
{
	char	szName[128];
	char	szVersion[128];
	char	szState[128];
};

class CTronInterfaceMgr : public CInterfaceMgr
{
public:
	CTronInterfaceMgr();
	~CTronInterfaceMgr();

    LTBOOL  Init();
	void	Term();

	// Accessors
	CHUDMgr*		GetHUDMgr()			{return &m_HUD;}
	CScreenMgr*		GetScreenMgr()			{return &m_ScreenMgr;}
	CPlayerStats*	GetPlayerStats()	{ return &m_stats; }
	CWeaponChooser*	GetWeaponChooser()	{ return &m_WeaponChooser; }

	// RETAIL: slots 0x50 and 0x54 (see CInterfaceMgr)
	virtual bool	TronUnknownVirtual50();
	virtual bool	TronUnknownVirtual54() { return !m_DuplicateDlg.IsVisible(); }

	virtual void	TronUnknownVirtual7C(void* pTronUnknown);

    LTBOOL			OnCommandOn(int command);
    LTBOOL			OnCommandOff(int command);
	LTBOOL			OnKeyDown(int key, int rep);
	void			OnExitWorld();

	virtual void    OnEnterWorld(LTBOOL bRestoringGame=LTFALSE);

	virtual void	OnLButtonUp(int x, int y);
	virtual void	OnRButtonUp(int x, int y);
	virtual void	OnMouseMove(int x, int y);

	virtual LTBOOL	Update();
	virtual LTBOOL	Draw();

	virtual void	Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState);
	virtual void	Load(ILTMessage_Read *pMsg);

	virtual void	ScreenDimsChanged();
	virtual void	UpdateCursorState();

	virtual void	UpdatePlayingState();
	virtual void	UpdatePlayerStats(uint8 nThing, uint8 nType1, uint8 nType2, LTFLOAT fAmount);

	bool			IsDisplayingProgress() {return m_bDisplayProgress;}

	// RETAIL: Tron's flags (the names are ours). +0x28ae8: set from a
	// server message (retail 0x1009dc1a), cleared on entering a world; the
	// I/O node camera HUD shows while it is (CHUDIOCamera::Render, retail
	// 0x1002b8fc). +0x28b14: set on entering a world (0x1009d938; cleared
	// for some multiplayer games); most HUD items draw only while it is
	// (CHUDArmor::Render, 0x10022098)
	bool			GetTronUnknown28AE8() {return m_bTronUnknown28AE8;}
	bool			GetTronUnknown28B14() {return m_bTronUnknown28B14;}

	// RETAIL: +0x28b18, the id of the string CHUDSingleString shows (set
	// from server messages, 0x1009d805..; the name is ours)
	int				GetTronUnknown28B18() {return m_nTronUnknown28B18;}

protected:
	// Message function
	virtual LTBOOL OnMessage(uint8 messageID, ILTMessage_Read *pMsg);

	virtual LTBOOL	PostPlayingState(GameState eNewState);
	virtual void	UpdateLoadingLevelState();

	// RETAIL: helpers (the names are ours)
	void			ShowChatInput(uint8 nChatType);	// 0x1009d2e0
	void			UpdateIOCamera();				// 0x1009e240
	void			CreateIOCamera();				// 0x1009e390
	void			ClearTronRecords();				// 0x1009f050
	void			QueueSubroutinePickup(const char* szName, const char* szVersion, const char* szState);	// 0x1009f0c0
	void			HandleSubroutinePickup(char* szName, char* szVersion, char* szState);	// 0x1009f200
	void			OnDuplicateDlgClick(int x, int y);	// 0x1009f510

private:
	// RETAIL: layout from the constructor (0x1009ca20) and destructor
	// (0x1009ce40). The pads stand in for the difference between retail's
	// sizes of our member classes and ours (checked in the constructor);
	// they go when those classes are fixed.
	CTronHUDMgr			m_HUD;					// +0x10108 Heads-Up Display
	char				m_aTronPadHUD[0x11eec - sizeof(CTronHUDMgr)];
	CTronScreenMgr		m_ScreenMgr;			// +0x21ff4
	CTronPlayerStats	m_stats;				// +0x2206c Player statistics (health, ammo, armor, etc.)
	CSubroutineMgr		m_SubroutineMgr;		// +0x24474
	CTronLayoutMgr		m_TronLayoutMgr;		// +0x25620, same as g_pLayoutMgr
	// (our CTronLayoutMgr is smaller than retail's 0x1170 bytes)
	char				m_aTronPadLayout[0x1170 - sizeof(CTronLayoutMgr)];
	// PORT: retail's is a CTronWeaponChooser (0x118 bytes, vtable
	// 0x101e95b0; not yet written)
	CWeaponChooser		m_WeaponChooser;		// +0x26790
	char				m_aTronPadWeaponChooser[0x118 - sizeof(CWeaponChooser)];
	CServerOptionMgr	m_ServerOptionMgr;		// +0x268a8 same as g_pServerOptionMgr
	CLightCycleGameDataMgr	m_LightCycleGameDataMgr;	// +0x279c4 same as g_pLightCycleGameDataMgr
	CScreenSprite*		m_pTurnArrowLeft;		// +0x28ae0 "Interface\Spr\TurnArrowLeft.spr"
	CScreenSprite*		m_pTurnArrowRight;		// +0x28ae4 "Interface\Spr\TurnArrowRight.spr"
	bool				m_bTronUnknown28AE8;	// +0x28ae8
	// RETAIL: the I/O node camera (CreateIOCamera 0x1009e390,
	// UpdateIOCamera 0x1009e240; the names are ours)
	HLOCALOBJ			m_hIOCamera;			// +0x28aec
	uint32				m_nTronUnknown28AF0;	// +0x28af0
	LTRect				m_rcIOCamera;			// +0x28af4 the layout's rect
	LTRect				m_rcIOCameraScaled;		// +0x28b04 at the screen's size
	bool				m_bTronUnknown28B14;	// +0x28b14
	int					m_nTronUnknown28B18;	// +0x28b18
	bool				m_bTronUnknown28B1C;	// +0x28b1c, loaded by Load
	bool				m_bDisplayProgress;		// +0x28b1d flag for displaying objectives and procedurals
	std::list<TronInterfaceRecord*> m_TronRecords;	// +0x28b20
	// PORT: retail's is a CTronLoadingScreen (0x124 bytes, vtable
	// 0x101e904c, CInterfaceMgr's +0xf8e4 points at it); not yet written
	uint8				m_aTronLoadingScreen[0x124];	// +0x28b24
	// RETAIL: the duplicate subroutine box (CTronInterfaceMgr's use of it:
	// 0x1009f200, 0x1009f470..0x1009f5e0)
	CDuplicateSubroutineDlg	m_DuplicateDlg;		// +0x28c48
	// RETAIL: the duplicate subroutine box's two copies, and the new one's
	// pickup (0x1009f200; the names are ours)
	PlayerSubroutine*	m_pDuplicateOld;		// +0x293b8 the player's
	PlayerSubroutine*	m_pDuplicateNew;		// +0x293bc, deleted by Term
	char				m_szDuplicateName[64];		// +0x293c0
	char				m_szDuplicateVersion[64];	// +0x29400
	char				m_szDuplicateState[64];		// +0x29440
	eHUDRenderLevel		m_eDuplicateRenderLevel;	// +0x29480 the HUD's, while the box is up
	std::vector<TronPendingSubroutine> m_PendingSubroutines;	// +0x29484
};
#endif // __TRONINTERFACE_MGR_H__