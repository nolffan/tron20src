// ----------------------------------------------------------------------- //
//
// MODULE  : HUDPaused.h
//
// PURPOSE : Definition of CHUDPaused to display a paused message
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_PAUSED_H
#define __HUD_PAUSED_H

#include "HUDItem.h"
#include "HUDMessage.h"

//******************************************************************************************
//** HUD Message Queue
//******************************************************************************************
class CHUDPaused : public CHUDItem
{
public:
	// RETAIL: eGameTypeSingle, Cooperative, Deathmatch and the disc games (retail's commonest slot 3)
	virtual bool	IsUsedInGameType(GameType eGameType) { return eGameTypeSingle == eGameType || (eGameType > eGameTypeSingleLightCycleCustom && eGameType <= eGameTypeDiscTournament); }

	CHUDPaused();
	

    virtual LTBOOL      Init();
	virtual void		Term();

    virtual void        Render();
    virtual	void        Update();

	virtual void        UpdateLayout();

	virtual	void		Show(LTBOOL bShow);

protected:
	CHUDMessage		m_Msg;

    LTIntPt		m_BasePos;
	MsgCreate	m_MsgFormat;
	float		m_fScale;


};



#endif