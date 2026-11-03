// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDecision.h
//
// PURPOSE : Definition of CHUDDecision to display transmission messages
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_DECISION_H
#define __HUD_DECISION_H

#include "HUDItem.h"
#include "ClientServerShared.h"

//******************************************************************************************
//** HUD Message Queue
//******************************************************************************************
class CHUDDecision : public CHUDItem
{
public:
	// RETAIL: eGameTypeSingle, Cooperative, Deathmatch and the disc games (retail's commonest slot 3)
	virtual bool	IsUsedInGameType(GameType eGameType) { return eGameTypeSingle == eGameType || (eGameType > eGameTypeSingleLightCycleCustom && eGameType <= eGameTypeDiscTournament); }

	CHUDDecision();
	

    virtual LTBOOL      Init();
	virtual void		Term();

    virtual void        Render();
    virtual void        Update();

	virtual void        UpdateLayout();

	void	OnObjectRemove(HLOCALOBJ hObj);

	//hide or show based on messages from the server
	void	Show(ILTMessage_Read *pMsg);

	//close window and tell server
	void	Hide();

	void	SetScale(float fScale);

	// RETAIL: both virtual in Tron (its vtable's last two slots: Choose's body
	// is retail 0x10140120)
	virtual void	Choose(uint8 nChoice);

	// RETAIL: virtual in Tron, and only while the item is enabled (its last slot)
	virtual LTBOOL	IsVisible() {return m_bVisible && m_bEnabled;}

	HOBJECT GetObject() { return m_hObject; }
	float	GetRadius() { return m_fRadius; }

protected:

    LTIntPt					m_BasePos;

	CUIFont*	m_pFont;				// The font for this control
	uint8		m_nFontSize;			// The scaled font size to use.
	uint8		m_nBaseFontSize;		// The font size before scaling
	uint32		m_nTextColor;
	float		m_fScale;
	LTBOOL		m_bVisible;
	uint16		m_nWidth;
	LTIntPt		m_Offset;

	HOBJECT		m_hObject;
	LTVector	m_vObjPos;
	float		m_fRadius;

	CLTGUIWindow	    m_Dlg;
	CLTGUIColumnCtrl*	m_pText[MAX_DECISION_CHOICES];

};



#endif