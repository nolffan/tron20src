// ----------------------------------------------------------------------- //
//
// MODULE  : HUDPopup.h
//
// PURPOSE : Definition of CHUDPopup to display Popups
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_POPUP_H
#define __HUD_POPUP_H

//
// Includes...
//
	
	#include "HUDItem.h"
	#include "LTGuiMgr.h"


class CHUDPopup : public CHUDItem
{
public:
	// RETAIL: eGameTypeSingle, Cooperative, Deathmatch and the disc games (retail's commonest slot 3)
	virtual bool	IsUsedInGameType(GameType eGameType) { return eGameTypeSingle == eGameType || (eGameType > eGameTypeSingleLightCycleCustom && eGameType <= eGameTypeDiscTournament); }

	CHUDPopup();
	

    virtual LTBOOL      Init();
	virtual void		Term();

    virtual void        Render();
    virtual void        Update();

	//hide or show 
	void	Show( uint8 nPopupID, const char *pText );

	//close window and tell server
	void	Hide();

	void	SetScale(float fScale);
	void	SetTextColor( uint8 nR, uint8 nG, uint8 nB, uint8 nA )
	{
		m_bColorOverride = LTTRUE;
		m_Text.SetColors( SET_ARGB(nA,nR,nG,nB), SET_ARGB(nA,nR,nG,nB), SET_ARGB(nA,nR,nG,nB) );
	}

	// RETAIL: virtual in Tron, and only while the item is enabled (its last slot)
	virtual LTBOOL	IsVisible() {return m_bVisible && m_bEnabled;}

protected:

    
	float		m_fScale;
	LTBOOL		m_bVisible;
	LTBOOL		m_bColorOverride;
	

	CLTGUIFrame			m_Frame;
	CLTGUITextCtrl		m_Text;

};

#endif // __HUD_POPUP_H