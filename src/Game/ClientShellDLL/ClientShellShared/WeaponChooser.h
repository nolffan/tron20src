// WeaponChooser.h: interface for the CWeaponChooser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_WEAPONCHOOSER_H_)
#define _WEAPONCHOOSER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LTGUIMgr.h"
#include "Timer.h"

// RETAIL: Tron's chooser is virtual (vtable 0x101eb9f4: the destructor,
// Init, Term, Open, Close, NextWeapon, PrevWeapon, EndAutoSwitch, Update);
// CTronWeaponChooser (TRON/TronWeaponChooser.h) overrides most of them.
// Close takes a flag (unused here; CTronWeaponChooser's shows the closing
// selection on the HUD weapon when it is set).
class CWeaponChooser
{
public:
	CWeaponChooser();
	virtual ~CWeaponChooser();

	virtual void	Init();
	virtual void	Term();

	virtual bool	Open(uint8 nClass);
	virtual void	Close(bool bShowSelection = true);
    bool	IsOpen()    {return m_bIsOpen;}

	virtual void	NextWeapon(uint8 nClass);
	virtual void	PrevWeapon();
	virtual void	EndAutoSwitch(bool bNextWeaponKey = true);

	virtual void	Update();

    uint8   GetCurrentSelection() {return m_nWeapon;}
    uint8   GetCurrentClass() {return m_nClass;}

protected:

	uint8			m_nWeapon;		// +0x4
	uint8			m_nClass;		// +0x5
    bool			m_bIsOpen;		// +0x6

	CTimer		m_NextWeaponKeyDownTimer;	// +0x8
	CTimer		m_PrevWeaponKeyDownTimer;	// +0x18
	CTimer		m_AutoSwitchTimer;			// +0x28
	CTimer		m_AutoCloseTimer;			// +0x38
};

class CAmmoChooser
{
public:
	CAmmoChooser();
	virtual ~CAmmoChooser();

	void	Init();
	void	Term();

    bool	Open();
	void	Close();
    bool	IsOpen()    {return m_bIsOpen;}

	void	NextAmmo();
	void	EndAutoSwitch();

	void	Update();

    uint8   GetCurrentSelection() {return m_nAmmo;}

private:
	uint8			m_nAmmo;
    bool			m_bIsOpen;

	CTimer		m_NextAmmoKeyDownTimer;
	CTimer		m_AutoSwitchTimer;
	CTimer		m_AutoCloseTimer;

};

#endif // !defined(_WEAPONCHOOSER_H_)