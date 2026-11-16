// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPreload.h
//
// PURPOSE : Interface screen to be displayed  before loading a level
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_SCREEN_PRELOAD_H_)
#define _SCREEN_PRELOAD_H_

#include "BaseScreen.h"

class CScreenPreload : public CBaseScreen
{
public:
	CScreenPreload();
	virtual ~CScreenPreload();

	// Build the screen
    virtual LTBOOL	Build();
    virtual void	OnFocus(LTBOOL bFocus);
	virtual void	Escape();

	virtual bool	UpdateInterfaceSFX();

	//sets the name of the world being loaded
	void	SetWaitingToExit(bool bWaitingToExit) {m_bWaitingToExit = bWaitingToExit;}

protected:

	virtual void	CreateInterfaceSFX();


	std::string		m_layout;
	bool			m_bWaitingToExit;

};

#endif // !defined(_SCREEN_PRELOAD_H_)