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

class CWeaponChooser
{
public:
	CWeaponChooser();
	virtual ~CWeaponChooser();

	// RETAIL: Tron's are virtual (vtable 0x101eb9f4, nine slots), for
	// CTronWeaponChooser (vtable 0x101e95b0)
	virtual void	Init();
	virtual void	Term();

    virtual bool	Open(uint8 nClass);
	// RETAIL: Tron's takes a flag (0x10166f10 ret 4) that only
	// CTronWeaponChooser reads (0x100afe08: the HUD weapon shows the
	// selection). The name is ours.
	virtual void	Close(bool bSelect = true);
    bool	IsOpen()    {return m_bIsOpen;}

	virtual void	NextWeapon(uint8 nClass);
	virtual void	PrevWeapon();
	virtual void	EndAutoSwitch(bool bNextWeaponKey = true);

	virtual void	Update();

    uint8   GetCurrentSelection() {return m_nWeapon;}
    uint8   GetCurrentClass() {return m_nClass;}

protected:

	uint8			m_nWeapon;
	uint8			m_nClass;
    bool			m_bIsOpen;

	CTimer		m_NextWeaponKeyDownTimer;
	CTimer		m_PrevWeaponKeyDownTimer;
	CTimer		m_AutoSwitchTimer;
	CTimer		m_AutoCloseTimer;
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