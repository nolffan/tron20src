// ----------------------------------------------------------------------- //
//
// MODULE  : HUDItem.h
//
// PURPOSE : Definition of CHUDItem base class
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_ITEM_H
#define __HUD_ITEM_H

#include "ltbasedefs.h"
#include "NetDefs.h"


enum eHUDRenderLevel
{
	kHUDRenderNone = 0,		// always render
	kHUDRenderText,			// render when text is allowed (i.e. cinematics, etc.)
	kHUDRenderFull,			// render only when full HUD is rendered

	// RETAIL: Tron's levels are build 51's (kHUDRenderFull is 2: CHUDItem's
	// constructor, retail CShell.dll 0x10021e10); NOLF2's dead level, which
	// its code still names, stands in as the text level (PORT)
	kHUDRenderDead = kHUDRenderText
};


//******************************************************************************************
//** Base HUD item
//******************************************************************************************
class CHUDItem
{
public:
	// RETAIL: an initializer list in Tron (the members are set before the
	// vtable: retail CShell.dll 0x10021e10)
	CHUDItem() : m_UpdateFlags(0), m_eLevel(kHUDRenderFull), m_bEnabled(true) {}
	virtual ~CHUDItem() {Term();}
	

    virtual LTBOOL      Init() {return LTTRUE;}
	virtual void		Term() {}

	// RETAIL: Tron's, pure here and in CHUDMeter (retail's vtables, slot 3):
	// whether the item belongs to the HUD of a game type. CHUDMgr builds its
	// list of items from it (retail CHUDMgr's vtable slot 2)
	virtual bool		IsUsedInGameType(GameType eGameType) = 0;

    virtual void        Render() = 0;
    virtual void        Update() = 0;

    virtual void        UpdateLayout() {}

	virtual uint32		GetUpdateFlags() { return m_UpdateFlags; }

	virtual	eHUDRenderLevel	GetRenderLevel() { return m_eLevel; }

	virtual void		SetRenderState();

	// RETAIL: Tron's, the last two slots of every HUD item (the flag at +0xc,
	// set true by the constructor; CHUDVersion's Render reads it first). The
	// names are ours: not IsVisible, which items such as CHUDDecision have of
	// their own, and do not override with it
	virtual void		SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }
	virtual bool		IsEnabled() { return m_bEnabled; }

protected:
	uint32				m_UpdateFlags;
	eHUDRenderLevel		m_eLevel;
	bool				m_bEnabled;	// RETAIL: Tron's

};


#endif