// ----------------------------------------------------------------------- //
//
// MODULE  : HUDSubtitles.h
//
// PURPOSE : Definition of CHUDSubtitles to display subtitles
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_SUBTITLES_H
#define __HUD_SUBTITLES_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD Message Queue
//******************************************************************************************
class CHUDSubtitles : public CHUDItem
{
public:
	// RETAIL: every game type
	virtual bool	IsUsedInGameType(GameType eGameType) { return eGameType >= eGameTypeSingle && eGameType <= eGameTypeMultiplayerLightcycles; }

	CHUDSubtitles();
	

    virtual LTBOOL      Init();
	virtual void		Term();

    virtual void        Render();
    virtual void        Update();

	virtual void        UpdateLayout();

	LTBOOL	Show(int nStringId, LTVector vSpeakerPos, LTFLOAT fRadius=0.0f, LTFLOAT fDuration=-1.0f, bool bSubtitlePriority = false);
	void	Clear();

	void	SetScale(float fScale);

	// RETAIL: Tron's: its Render is empty (retail 0x100abb80), and NOLF2's
	// Render is this, slot 12 (retail 0x100fd0e0). The name is ours, and what
	// calls it is still to be found
	virtual void	Draw();

	// RETAIL: virtual in Tron, and only while the item is enabled (its last slot)
	virtual LTBOOL	IsVisible() {return m_bVisible && m_bEnabled;}

protected:

    LTIntPt					m_BasePos;
	CUIFormattedPolyString*	m_pText;

	CUIRECT		m_DisplayRect;

	CUIFont*	m_pFont;				// The font for this control
	uint8		m_nFontSize;			// The scaled font size to use.
	uint8		m_nBaseFontSize;		// The font size before scaling
	uint32		m_nTextColor;
	float		m_fScale;
	float		m_fElapsedTime;			// The amount of time that has elapsed while this subtitle is displayed
	LTBOOL		m_bVisible;

	LTIntPt			m_CinematicPos;
	uint16			m_nCinematicWidth;
	LTIntPt			m_FullScreenPos;
	uint16			m_nFullScreenWidth;
	uint8			m_nMaxLines;

	LTVector		m_vSpeakerPos;
	bool			m_bSubtitlePriority;

	int				m_nCursorPos;
	LTBOOL			m_bOverflow;
	LTFLOAT			m_fRadius;
	LTFLOAT			m_fDuration;

	LTFLOAT			m_fScrollStartTime;
	LTFLOAT			m_fScrollSpeed;
	LTFLOAT			m_fOffset;
	LTFLOAT			m_fMaxOffset;

	LTFLOAT			m_fEndTime;


};



#endif