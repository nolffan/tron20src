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

#include "InterfaceMgr.h"
#include "TRONHUDMgr.h"
#include "TRONScreenMgr.h"
#include "TRONPlayerStats.h"
#include "SubroutineMgr.h"
#include "TronLayoutMgr.h"

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

    LTBOOL			OnCommandOn(int command);
	void			OnExitWorld();

	virtual void    OnEnterWorld(LTBOOL bRestoringGame=LTFALSE);

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

private:
	CTronHUDMgr			m_HUD;					// Heads-Up Display
	CTronScreenMgr		m_ScreenMgr;
	CTronPlayerStats	m_stats;				// Player statistics (health, ammo, armor, etc.)
	CSubroutineMgr		m_SubroutineMgr;
	CTronLayoutMgr		m_TronLayoutMgr;		// same as g_pLayoutMgr
	bool				m_bDisplayProgress;		// flag for displaying objectives and procedurals

	// RETAIL: at retail's +0x28ae8 and +0x28b14 (where this class's other
	// members are not yet placed: see the accessors)
	bool				m_bTronUnknown28AE8;
	bool				m_bTronUnknown28B14;
	int					m_nTronUnknown28B18;
};
#endif // __TRONINTERFACE_MGR_H__