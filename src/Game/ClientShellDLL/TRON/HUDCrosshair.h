// ----------------------------------------------------------------------- //
//
// MODULE  : HUDCrosshair.h
//
// PURPOSE : HUDItem to display crosshair
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_CROSSHAIR_H
#define __HUD_CROSSHAIR_H

#include "HUDItem.h"

// RETAIL: Tron's three reticles (retail's table 0x102133d0)
typedef enum
{
	RETICLE_NORMAL = 0,
	RETICLE_INNOCENT,
	RETICLE_ACTIVATE,
	NUM_RETICLES
} eReticleType;

// RETAIL: the permission sets a target may need, each its own icon
#define NUM_PSET_ICONS	8

//******************************************************************************************
//** HUD crosshair
//**
//** RETAIL: Tron's, rewritten from retail CShell.dll (vtable 0x101e6cc0; its
//** TU 0x100240f0..0x10024f80): a reticle (spun while the target can be
//** activated), the permission sets the target needs, and the activation,
//** debug and energy texts placed by their justification. Member names are
//** ours where no string names them.
//******************************************************************************************
class CHUDCrosshair : public CHUDItem
{
public:
	// RETAIL: eGameTypeSingle, Cooperative, Deathmatch and the disc games (retail's commonest slot 3)
	virtual bool	IsUsedInGameType(GameType eGameType) { return eGameTypeSingle == eGameType || (eGameType > eGameTypeSingleLightCycleCustom && eGameType <= eGameTypeDiscTournament); }

	CHUDCrosshair();							// RETAIL: 0x100240f0

    LTBOOL	Init();								// RETAIL: 0x10024170
	void	Term();								// RETAIL: 0x100245a0

    void	Render();							// RETAIL: 0x10024600
    void	Update();							// RETAIL: 0x100246f0

	void	UpdateLayout();						// RETAIL: 0x10024cb0

	void	RenderArmed();						// RETAIL: 0x10024a10
	void	RenderPermissions();				// RETAIL: 0x10024c00

	// RETAIL: empty (folded, retail 0x100abb80); Render calls it while the
	// scope is up
	void	RenderScope();

	void	Toggle() {m_bEnabled = !m_bEnabled;}

	// RETAIL: the scores hide the crosshair while they are drawn (retail
	// CHUDScores::Render, 0x1003563d, writes the flag at +0x82 that the
	// constructor sets, 0x10024153). The name is ours
	void	Enable(bool bEnable) {m_bEnabled = bEnable;}

	// RETAIL: the energy the target needs (+0x74; CHUDHealthEnergy::Render
	// reads it, 0x1002a1ef). The name is ours
	int		GetEnergyRequired() {return m_nEnergyRequired;}

protected:

	// RETAIL: 0x10024e50: sizes the string for the scale and places it by its
	// justification (0 left, 1 right, 2 centred), its screen position out
	void	PositionString(CUIFormattedPolyString* pStr, uint8 nJust, LTIntPt pos, uint8 nSize, LTIntPt* pScreenPos);

	LTIntPt			m_ScreenCenter;			// +0x10

	CUIFormattedPolyString*	m_pStr;			// +0x18
	LTIntPt			m_StrPos;				// +0x1c
	LTIntPt			m_StrScreenPos;			// +0x24
	uint8			m_nStrSz;				// +0x2c
	uint8			m_nStrJust;				// +0x2d
	uint32			m_StrColor;				// +0x30
	uint32			m_StrDisColor;			// +0x34

	CUIFormattedPolyString*	m_pDbgStr;		// +0x38
	LTIntPt			m_DbgPos;				// +0x3c
	LTIntPt			m_DbgScreenPos;			// +0x44
	uint8			m_nDbgSz;				// +0x4c
	uint8			m_nDbgJust;				// +0x4d
	uint16			m_nDbgWidth;			// +0x4e
	uint32			m_DbgColor;				// +0x50

	// Additional information about the currently targeted object i.e. energy required
	CUIFormattedPolyString* m_pInfoStr;		// +0x54
	LTIntPt			m_InfoPos;				// +0x58
	LTIntPt			m_InfoScreenPos;		// +0x60
	uint8			m_nInfoSz;				// +0x68
	uint8			m_nInfoJust;			// +0x69
	uint16			m_nInfoWidth;			// +0x6a
	uint32			m_InfoColor;			// +0x6c
	uint32			m_InfoBadColor;			// +0x70

	int				m_nEnergyRequired;		// +0x74
	float			m_fScale;				// +0x78

	//activation data
	HOBJECT		m_hObj;						// +0x7c
	uint16		m_nString;					// +0x80

	bool		m_bEnabled;					// +0x82
	uint32		m_nTronUnknown84;			// +0x84 not used

	HTEXTURE	m_hReticleTex[NUM_RETICLES];	// +0x88
	LT_POLYFT4	m_ReticlePrim;				// +0x94
	HTEXTURE	m_hPSetEmptyTex[NUM_PSET_ICONS];	// +0xe8: not needed
	HTEXTURE	m_hPSetNeedTex[NUM_PSET_ICONS];		// +0x108: needed, not held
	HTEXTURE	m_hPSetFullTex[NUM_PSET_ICONS];		// +0x128: needed and held
};

#endif