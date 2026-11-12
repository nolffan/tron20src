// ----------------------------------------------------------------------- //
//
// MODULE  : HUDHiding.h
//
// PURPOSE : HUDItem to display Jet's version number
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_VERSION_H
#define __HUD_VERSION_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD program version display
//******************************************************************************************
// RETAIL: Tron 2.0's, as retail CShell.dll has it (0x1003e340..0x1003e920,
// vtable 0x101e7208): the layout's "VersionNumber" places and colours it,
// and a change of version flashes it (a major change in one colour, a minor
// in another). The member names past build 69's are ours.
class CHUDVersion : public CHUDItem
{
public:
	// RETAIL: eGameTypeSingle, Cooperative, Deathmatch and the disc games (retail's commonest slot 3)
	virtual bool	IsUsedInGameType(GameType eGameType) { return eGameTypeSingle == eGameType || (eGameType > eGameTypeSingleLightCycleCustom && eGameType <= eGameTypeDiscTournament); }

	CHUDVersion();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
    LTIntPt		m_BasePos;				// +0x10
	LTVector	m_vColor;				// +0x18
	CUIPolyString * m_pString;			// +0x24
	uint16		m_iVersion;				// +0x28
	float		m_fUpdateTime;			// +0x2c
	float		m_fTimeRemaining;		// +0x30

	int			m_nFont;				// +0x34
	int			m_nFontSize;			// +0x38
	int			m_nMajorLoops;			// +0x3c
	int			m_nMinorLoops;			// +0x40
	float		m_fMajorLoopLength;		// +0x44
	float		m_fMinorLoopLength;		// +0x48
	LTVector	m_vMajorColor;			// +0x4c
	LTVector	m_vMinorColor;			// +0x58

	int			m_nLoops;				// +0x64: flashes left
	float		m_fLastTime;			// +0x68
	float		m_fLoopLength;			// +0x6c
	float		m_fLoopTime;			// +0x70: left of this flash
	LTVector	m_vFlashColor;			// +0x74
};

#endif