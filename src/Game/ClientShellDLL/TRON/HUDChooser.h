// ----------------------------------------------------------------------- //
//
// MODULE  : HUDChooser.h
//
// PURPOSE : HUDItem to display weapon and ammo choices
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_CHOOSER_H
#define __HUD_CHOOSER_H

#include "HUDItem.h"

// RETAIL: Tron's weapon chooser (vtable 0x101e6c84; constructor 0x10022a90,
// 0x4518 bytes: CTronHUDMgr +0x6e10). It is a grid: one row for each weapon
// class that has weapons, one box for each weapon of the class. The box
// names come from HUDLayout's Chooser* attributes. The member names are ours.
class CHUDWpnChooser : public CHUDItem
{
public:
	// RETAIL: eGameTypeSingle, Cooperative, Deathmatch and the disc games (retail's commonest slot 3)
	virtual bool	IsUsedInGameType(GameType eGameType)
	{
		// RETAIL: a switch (0x10023440, folded with the other HUD items')
		switch (eGameType)
		{
			case eGameTypeSingle:
			case eGameTypeCooperative:
			case eGameTypeDeathmatch:
			case eGameTypeDiscArena:
			case eGameTypeDiscTournament:
				return true;
		}
		return false;
	}

	CHUDWpnChooser();
	~CHUDWpnChooser();

    LTBOOL      Init();

    void        Render();
    void        Update();

    void        UpdateLayout();

	enum Constants 
	{ 
		kMaxClasses = 6,
		kMaxItemsPerClass = 16
	};
	

private:
	// RETAIL: 0x100232c0: the fade in and out of the chooser (as
	// CHUDWeapon::UpdateFade). The name is ours.
	void		UpdateFade();

	float		m_fTextHeight;			// +0x10
	uint32		m_TextColor;			// +0x14
	LTIntPt		m_BoxSize;				// +0x18 ChooserBoxSize
	LTIntPt		m_Spacing;				// +0x20 ChooserSpacing
	LTIntPt		m_IconOffset;			// +0x28 ChooserIconOffset
	LTIntPt		m_Margin;				// +0x30 ChooserMargin
	LTIntPt		m_Size;					// +0x38 x: three boxes wide, y: ChooserTitleHeight
	float		m_fScale;				// +0x40
	uint32		m_nScreenWidth;			// +0x44
	float		m_fIntroTime;			// +0x48 ChooserIntroTime
	float		m_fOutroTime;			// +0x4c ChooserOutroTime
	LTBOOL		m_bDraw;				// +0x50 the weapon chooser is open
	uint8		m_nClass;				// +0x54

	uint8		m_nWeaponID[kMaxClasses][kMaxItemsPerClass];	// +0x55

	CUIFormattedPolyString *m_pStr;		// +0xb8
	uint8		m_nSelWeapon;			// +0xbc the chooser's selection
	uint8		m_nCurWeapon;			// +0xbd the weapon when the chooser opened
	uint8		m_nWeaponCount[kMaxClasses];	// +0xbe

	LT_POLYFT4	m_BoxPoly[kMaxClasses][kMaxItemsPerClass];	// +0xc4

	HTEXTURE	m_hChooserTex;			// +0x2044
	HTEXTURE	m_hChooserTexHi;		// +0x2048
	HTEXTURE	m_hCorruptTex;			// +0x204c
	HTEXTURE	m_hCorruptTexHi;		// +0x2050
	HTEXTURE	m_hEmptyTex;			// +0x2054
	HTEXTURE	m_hEmptyTexHi;			// +0x2058
	HTEXTURE	m_hClassNumber[kMaxClasses];	// +0x205c
	HTEXTURE	m_hClassNumberHi[kMaxClasses];	// +0x2074

	LT_POLYFT4	m_ClassPoly[kMaxClasses];						// +0x208c
	LT_POLYFT4	m_IconPoly[kMaxClasses][kMaxItemsPerClass];	// +0x2284

	HTEXTURE	m_hDisabledIcon[kMaxClasses][kMaxItemsPerClass];	// +0x4204 (the selected weapon's)
	HTEXTURE	m_hNormalIcon[kMaxClasses][kMaxItemsPerClass];		// +0x4384

	float		m_fLastTime;			// +0x4504
	int			m_eFadeState;			// +0x4508 0 in, 1 shown, 2 out, 3 hidden
	float		m_fFadeTime;			// +0x450c
	float		m_fFadeDuration;		// +0x4510
	LT_POLYFT4*	m_pWeaponPrim;			// +0x4514 the HUD weapon's icon (the icons fly from it)
};


class CHUDAmmoChooser : public CHUDItem
{
public:
	// TRON: not in retail; as most of its HUD items
	virtual bool	IsUsedInGameType(GameType eGameType)
	{
		// RETAIL: a switch (0x10023440, folded with the other HUD items')
		switch (eGameType)
		{
			case eGameTypeSingle:
			case eGameTypeCooperative:
			case eGameTypeDeathmatch:
			case eGameTypeDiscArena:
			case eGameTypeDiscTournament:
				return true;
		}
		return false;
	}

	CHUDAmmoChooser();
	~CHUDAmmoChooser();

    LTBOOL      Init();

    void        Render();
    void        Update();

    void        UpdateLayout();

	enum Constants 
	{ 
		kMaxAmmoTypes = 10,
	};
	

private:
	float		m_fIconHt;
	float		m_fTextHeight;
	uint32		m_TextColor;
	float		m_fTexScale;
	
	float		m_fScale;
	uint32		m_nScreenWidth;

	LTBOOL		m_bDraw;

	LT_POLYGT4	m_WeaponPoly;
	HTEXTURE	m_hWeaponIcon;

	LT_POLYGT4	m_AmmoPoly[kMaxAmmoTypes];
	HTEXTURE	m_hAmmoIcon[kMaxAmmoTypes];

	CUIFormattedPolyString *m_pWpnStr;
	CUIFormattedPolyString *m_pStr;
	
};

#endif