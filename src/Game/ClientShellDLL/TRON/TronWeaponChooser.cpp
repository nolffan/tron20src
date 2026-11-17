// ----------------------------------------------------------------------- //
//
// MODULE  : TronWeaponChooser.cpp
//
// PURPOSE : Tron's in-game weapon chooser
//
// ----------------------------------------------------------------------- //

// RETAIL: written from retail CShell.dll (0x100afc50..0x100b046c). The
// chooser keeps a list of the compiled weapons (Open) and steps through it
// (NextWeapon, PrevWeapon); while the player's holster is locked it does
// nothing.

#include "stdafx.h"
#include "TronWeaponChooser.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "SoundMgr.h"
#include "ClientWeaponBase.h"
#include "ClientWeaponMgr.h"
#include "CMoveMgr.h"
#include "VehicleMgr.h"
#include "TronPlayerMgr.h"
#include "TRONPlayerStats.h"
#include "TronHUDMgr.h"

namespace
{
	// RETAIL: 333.0f (0x101e95a8)
	const float kfDelayTime	= 333.0f;
	VarTrack	g_vtChooserAutoSwitchTime;
	VarTrack	g_vtChooserAutoSwitchFreq;
}

// RETAIL: 0x100afc50
CTronWeaponChooser::CTronWeaponChooser()
{
}

// RETAIL: 0x100afca0
void CTronWeaponChooser::Init()
{
	g_vtChooserAutoSwitchTime.Init(g_pLTClient, "ChooserAutoSwitchTime", NULL, 0.175f);
	g_vtChooserAutoSwitchFreq.Init(g_pLTClient, "ChooserAutoSwitchFreq", NULL, 0.1f);
}

// RETAIL: 0x100afe30
bool CTronWeaponChooser::Open(uint8 nClass)
{
	if (g_pTronPlayerMgr->IsHolsterLocked() ||
		g_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() == PPM_LIGHTCYCLE)
	{
		g_pInterfaceMgr->TronUnknownVirtual88();
		return false;
	}

	nClass++;
	if (m_bIsOpen)
	{
		m_nClass = nClass;
		return true;
	}

	if (g_pSubChooser)
		g_pSubChooser->m_bShow = false;
	if (g_pProcedurals)
		g_pProcedurals->m_bShow = false;

	CClientWeaponMgr *pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
	if (!pClientWeaponMgr->WeaponsEnabled())
		return false;

	IClientWeaponBase *pClientWeapon = pClientWeaponMgr->GetCurrentClientWeapon();
	if( !pClientWeapon )
		return false;

	m_nWeapon = pClientWeapon->GetWeaponId();

	uint8 nWeapon = pClientWeaponMgr->GetNextWeaponId( m_nWeapon, nClass );
	if (!g_pWeaponMgr->IsValidWeaponId(nWeapon) && g_pWeaponMgr->GetWeaponClass(m_nWeapon) != nClass )
	{
		m_nWeapon = -1;
		m_bIsOpen = false;
		return false;
	}

	m_nClass = nClass;
	m_bIsOpen = true;

	memset(m_aWeapons, 0, sizeof(m_aWeapons));
	memset(m_aClasses, 0, sizeof(m_aClasses));
	m_nNumWeapons = 0;
	m_nCurrent = -1;

	// RETAIL: from the second weapon command on
	uint8 nFirstWeaponCommandId = g_pWeaponMgr->GetFirstWeaponCommandId() + 1;
	uint8 nLastWeaponCommandId = g_pWeaponMgr->GetLastWeaponCommandId();
	for (uint8 nWpnCommandId = nFirstWeaponCommandId; nWpnCommandId <= nLastWeaponCommandId; nWpnCommandId++)
	{
		uint8 nWpn = g_pWeaponMgr->GetWeaponId( nWpnCommandId );

		if (g_pWeaponMgr->IsPlayerWeapon(nWpn) && g_pWeaponMgr->IsValidWeaponId(nWpn))
		{
			if (g_pTronPlayerStats->IsWeaponCompiled(nWpn))
			{
				m_aClasses[m_nNumWeapons] = g_pWeaponMgr->GetWeaponClass(nWpn);
				m_aWeapons[m_nNumWeapons] = nWpn;
				if (m_nWeapon == nWpn)
					m_nCurrent = m_nNumWeapons;
				m_nNumWeapons++;
			}
		}
	}

	m_AutoCloseTimer.Start(kfDelayTime);
	g_pHUDMgr->QueueUpdate(kHUDChooser);
	return true;
}

// RETAIL: 0x100afde0
void CTronWeaponChooser::Close(bool bSelect)
{
	if (g_pSubChooser)
		g_pSubChooser->m_bShow = true;
	if (g_pProcedurals)
		g_pProcedurals->m_bShow = true;

	CWeaponChooser::Close();

	// the HUD weapon shows the selection
	if (bSelect)
		g_pWeaponItem->SetWeapon(m_nWeapon);
}

// RETAIL: 0x100b0220
void CTronWeaponChooser::NextWeapon(uint8 nClass)
{
	if (g_pTronPlayerMgr->IsHolsterLocked())
	{
		g_pInterfaceMgr->TronUnknownVirtual88();
		return;
	}

	nClass++;
	if (nClass > g_pWeaponMgr->GetNumWeaponClasses())
		nClass = m_nClass;

	// get the next weapon the player can change to (of the class, unless
	// any class)
	int i = 0;
	while (i++ < m_nNumWeapons)
	{
		if (++m_nCurrent >= m_nNumWeapons)
			m_nCurrent = 0;

		if (g_pTronPlayerStats->CanChangeToWeapon(m_aWeapons[m_nCurrent]) &&
			(nClass == 1 || m_aClasses[m_nCurrent] == nClass))
			break;
	}
	m_nWeapon = m_aWeapons[m_nCurrent];

    g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());

   	m_AutoCloseTimer.Start(kfDelayTime);
	m_NextWeaponKeyDownTimer.Start(g_vtChooserAutoSwitchTime.GetFloat());
	g_pHUDMgr->QueueUpdate(kHUDChooser);
}

// RETAIL: 0x100b0350
void CTronWeaponChooser::PrevWeapon()
{
	if (g_pTronPlayerMgr->IsHolsterLocked())
	{
		g_pInterfaceMgr->TronUnknownVirtual88();
		return;
	}

	// get the previous weapon the player can change to
	int i = 0;
	do
	{
		if (i >= m_nNumWeapons)
			break;
		i++;
		if (--m_nCurrent < 0)
			m_nCurrent = m_nNumWeapons - 1;
	} while (!g_pTronPlayerStats->CanChangeToWeapon(m_aWeapons[m_nCurrent]));
	m_nWeapon = m_aWeapons[m_nCurrent];

    g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());

   	m_AutoCloseTimer.Start(kfDelayTime);
	m_PrevWeaponKeyDownTimer.Start(g_vtChooserAutoSwitchTime.GetFloat());
	g_pHUDMgr->QueueUpdate(kHUDChooser);
}

// RETAIL: 0x100b0450: no auto switch; the chooser closes when the holster
// locks
void CTronWeaponChooser::Update()
{
	if (g_pTronPlayerMgr->IsHolsterLocked())
		Close(false);
}
