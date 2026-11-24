// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMulti.h
//
// PURPOSE : Interface screen for hosting and joining multi player games
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_MULTI_H_
#define _SCREEN_MULTI_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

// RETAIL: Tron's multiplayer screen (CShell.dll 0x1006fb90..0x1006fe20,
// vtable 0x101e8224): four items -- player, join (internet or LAN) and host
// -- and one member that none of its functions use (NOLF2's CD key and MOTD
// screen is not Tron's)
class CScreenMulti : public CBaseScreen
{
public:
	CScreenMulti();
	virtual ~CScreenMulti();

	// Build the screen
    LTBOOL   Build();
    void    OnFocus(LTBOOL bFocus);

protected:
    uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

private:
	// RETAIL: the screen is 0x794 bytes (debug_new_fn, 0x100ad5e0, allocates
	// 0x794; CBaseScreen is 0x790): the constructor does not set this member
	uint32	m_nTronUnknown790;	// +0x790
};

#endif // _SCREEN_MULTI_H_
