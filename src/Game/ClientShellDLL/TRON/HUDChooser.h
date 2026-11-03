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

class CHUDWpnChooser : public CHUDItem
{
public:
	// RETAIL: eGameTypeSingle, Cooperative, Deathmatch and the disc games (retail's commonest slot 3)
	virtual bool	IsUsedInGameType(GameType eGameType) { return eGameTypeSingle == eGameType || (eGameType > eGameTypeSingleLightCycleCustom && eGameType <= eGameTypeDiscTournament); }

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
	float		m_fColumnXPos[kMaxClasses];
	float		m_fIconHt;
	float		m_fTextHeight;
	uint32		m_TextColor;
	float		m_fTexScale;


	
	float		m_fScale;
	uint32		m_nScreenWidth;

	LTBOOL		m_bDraw;
	uint8		m_nClass;

	uint8		m_nWeaponID[kMaxClasses][kMaxItemsPerClass];


	LT_POLYGT4	m_ClassPoly[kMaxClasses];
	HTEXTURE	m_hClassIcon[kMaxClasses];

	LT_POLYGT4	m_ItemPoly[kMaxItemsPerClass];
	HTEXTURE	m_hItemIcon[kMaxItemsPerClass];

	CUIFormattedPolyString *m_pStr;
	
};


class CHUDAmmoChooser : public CHUDItem
{
public:
	// TRON: not in retail; as most of its HUD items
	virtual bool	IsUsedInGameType(GameType eGameType) { return eGameTypeSingle == eGameType || (eGameType > eGameTypeSingleLightCycleCustom && eGameType <= eGameTypeDiscTournament); }

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