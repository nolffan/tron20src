// ----------------------------------------------------------------------- //
//
// MODULE  : TronWeaponChooser.h
//
// PURPOSE : Tron's in-game weapon chooser - Definition
//
// RETAIL: Tron 2.0's (RTTI CTronWeaponChooser, CShell.dll vtable
// 0x101e95b0; functions 0x100afc70..0x100b0450; CTronInterfaceMgr
// +0x26790). Open makes a list of the player's compiled weapons; Next and
// Prev step through it. 0x118 bytes. The member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __TRON_WEAPON_CHOOSER_H__
#define __TRON_WEAPON_CHOOSER_H__

#include "WeaponChooser.h"

#define TRONWEAPONCHOOSER_MAX	100

class CTronWeaponChooser : public CWeaponChooser
{
public:

	CTronWeaponChooser();	// RETAIL: 0x100afc50, out of line

	virtual void	Init();

	virtual bool	Open(uint8 nClass);
	virtual void	Close(bool bShowSelection = true);

	virtual void	NextWeapon(uint8 nClass);
	virtual void	PrevWeapon();

	virtual void	Update();

protected:

	uint8	m_nWeapons[TRONWEAPONCHOOSER_MAX];	// +0x48: the weapons in the list
	uint8	m_nClasses[TRONWEAPONCHOOSER_MAX];	// +0xac: their classes
	int		m_nNumWeapons;						// +0x110
	int		m_nCurrent;							// +0x114: the selection's index
};

#endif // __TRON_WEAPON_CHOOSER_H__
