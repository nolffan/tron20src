// ----------------------------------------------------------------------- //
//
// MODULE  : TronWeaponChooser.h
//
// PURPOSE : Tron's in-game weapon chooser
//
// ----------------------------------------------------------------------- //

// RETAIL: written from retail CShell.dll (0x100afc50..0x100b046c; RTTI
// CTronWeaponChooser, vtable 0x101e95b0, 0x118 bytes: CTronInterfaceMgr
// +0x26790). The member names are ours.

#ifndef __TRON_WEAPON_CHOOSER_H__
#define __TRON_WEAPON_CHOOSER_H__

#include "WeaponChooser.h"

class CTronWeaponChooser : public CWeaponChooser
{
public:
	CTronWeaponChooser();

	virtual void	Init();

	virtual bool	Open(uint8 nClass);
	virtual void	Close(bool bSelect = true);

	virtual void	NextWeapon(uint8 nClass);
	virtual void	PrevWeapon();

	virtual void	Update();

private:

	enum Constants
	{
		kMaxWeapons = 100
	};

	// the compiled weapons, in command order (Open)
	uint8			m_aWeapons[kMaxWeapons];	// +0x48
	uint8			m_aClasses[kMaxWeapons];	// +0xac
	int				m_nNumWeapons;				// +0x110
	int				m_nCurrent;					// +0x114
};

#endif // __TRON_WEAPON_CHOOSER_H__
