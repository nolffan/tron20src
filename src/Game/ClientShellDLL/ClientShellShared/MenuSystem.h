// ----------------------------------------------------------------------- //
//
// MODULE  : MenuSystem.h
//
// PURPOSE : In-game system menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_MENU_SYSTEM_H_)
#define _MENU_SYSTEM_H_

#include "BaseMenu.h"


class CMenuSystem : public CBaseMenu
{
public:

	CMenuSystem( );

	virtual LTBOOL	Init();

	virtual void OnFocus(LTBOOL bFocus);

	// Handle a command
    virtual uint32 OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2);

	// RETAIL: CBaseMenu's last slot, false here (retail 0x101c8210)
	virtual bool TronUnknownVirtualC4() { return false; }

private:

	CLTGUITextCtrl*	m_pSaveCtrl;
	CLTGUITextCtrl*	m_pLoadCtrl;
	CLTGUITextCtrl*	m_pPlayerCtrl;
	CLTGUITextCtrl*	m_pTeamCtrl;
	// RETAIL: Tron's has no host options and no server controls (retail's
	// constructor 0x101c7d90 and Init 0x101c7df0: four controls; CMenuSystem
	// is 0xd6c bytes)
};

#endif //!defined(_MENU_SYSTEM_H_)