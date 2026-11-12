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
// -- and no members of its own (NOLF2's CD key and MOTD screen is not Tron's)
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
};

#endif // _SCREEN_MULTI_H_
