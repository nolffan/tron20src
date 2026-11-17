// ----------------------------------------------------------------------- //
//
// MODULE  : TronLoadingScreen.h
//
// PURPOSE : Tron's loading screen
//
// ----------------------------------------------------------------------- //

// RETAIL: written from retail CShell.dll (0x100a25f0..0x100a2d18; RTTI
// CTronLoadingScreen, vtable 0x101e904c, 0x124 bytes: CTronInterfaceMgr
// +0x28b24). The names are ours.

#ifndef __TRON_LOADING_SCREEN_H__
#define __TRON_LOADING_SCREEN_H__

#include "LoadingScreen.h"

class CTronLoadingScreen : public CLoadingScreen
{
public:
	virtual void	UpdateMissionInfo();

protected:
	// RETAIL: 0x100a25f0: the mission strings from the layout of the world
	// (its file name is the layout tag)
	bool			ReadWorldLayout();
};

#endif // __TRON_LOADING_SCREEN_H__
