// ----------------------------------------------------------------------- //
//
// MODULE  : HUDWeapon.h
//
// PURPOSE : HUDItem to display Jet's current Weapon
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_WEAPON_H
#define __HUD_WEAPON_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD Weapon display
//******************************************************************************************
class CHUDWeapon : public CHUDItem
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

	CHUDWeapon();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

	// RETAIL: 0x1003f100: the icon of the weapon (also inlined in Update;
	// CTronPlayerStats calls it, 0x100afe1a). The name is ours.
	void		SetWeapon(uint8 nWeaponID);

private:
	// RETAIL: CHUDWpnChooser reads m_WeaponPrim (0x100233a5)
	friend class CHUDWpnChooser;

	// RETAIL: 0x1003eae0: the fade state of the indicator (it fades out while
	// the weapon chooser is open). The name is ours.
	void		UpdateFade();

	// RETAIL: Tron's layout (0x1003e9a0 constructor; 0xf0 bytes). The names
	// are ours.
	float		m_fScale;			// +0x10
	LTBOOL		m_bVisible;			// +0x14: the weapon chooser is closed
	uint8		m_nWeaponID;		// +0x18

	LT_POLYFT4	m_BasePrim;			// +0x1c
	LT_POLYFT4	m_WeaponPrim;		// +0x70

	HTEXTURE	m_hHUDTex;			// +0xc4
	HTEXTURE	m_hCorruptTex;		// +0xc8
	HTEXTURE	m_hWeaponTex;		// +0xcc

	CUIPolyString*	m_pAmmoStr;		// +0xd0
	int			m_nAmmo;			// +0xd4

	int			m_eFadeState;		// +0xd8: 0 in, 1 shown, 2 out, 3 hidden
	float		m_fLastTime;		// +0xdc
	float		m_fFadeTime;		// +0xe0
	float		m_fFadeDuration;	// +0xe4

	LTIntPt		m_AmmoOffset;		// +0xe8
};

// RETAIL: 0x1026da80, set by Init (CHUDWpnChooser reads its weapon prim).
// The name is ours.
extern CHUDWeapon* g_pHUDWeapon;

#endif
