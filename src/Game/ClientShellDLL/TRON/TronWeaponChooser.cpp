// ----------------------------------------------------------------------- //
//
// MODULE  : TronWeaponChooser.cpp
//
// PURPOSE : Tron's in-game weapon chooser - Implementation
//
// RETAIL: written from retail CShell.dll 0x100afc70..0x100b0450 (no source
// of it exists). The names of the members are ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronWeaponChooser.h"
#include "TronInterfaceMgr.h"
#include "TronPlayerMgr.h"
#include "TRONPlayerStats.h"
#include "TronHUDMgr.h"
#include "HUDWeapon.h"
#include "HUDSubChooser.h"
#include "HUDProcedurals.h"
#include "ClientWeaponBase.h"
#include "ClientWeaponMgr.h"
#include "CMoveMgr.h"
#include "VehicleMgr.h"
#include "ClientSoundMgr.h"

namespace
{
	const float kfDelayTime	= 333.0f;	// RETAIL: 0x101e95a8

	// RETAIL: this TU's own copies (0x1026efa0..0x1026efb0)
	VarTrack	g_vtChooserAutoSwitchTime;
	VarTrack	g_vtChooserAutoSwitchFreq;
}

// RETAIL: 0x100afc50: the constructor is out of line
CTronWeaponChooser::CTronWeaponChooser()
{
}

// RETAIL: 0x100afca0
void CTronWeaponChooser::Init()
{
	g_vtChooserAutoSwitchTime.Init(g_pLTClient, "ChooserAutoSwitchTime", NULL, 0.175f);
	g_vtChooserAutoSwitchFreq.Init(g_pLTClient, "ChooserAutoSwitchFreq", NULL, 0.1f);
}

// RETAIL: 0x100afe30. The classes count from 1 here: the caller's class is
// one less.
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
		g_pSubChooser->Show(false);
	if (g_pProcedurals)
		g_pProcedurals->Show(false);

	CClientWeaponMgr const *pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
	if (!pClientWeaponMgr->WeaponsEnabled())
		return false;

	IClientWeaponBase const *pClientWeapon = pClientWeaponMgr->GetCurrentClientWeapon();
	if (!pClientWeapon)
		return false;

	m_nWeapon = pClientWeapon->GetWeaponId();

	uint8 nWeapon = pClientWeaponMgr->GetNextWeaponId(m_nWeapon, nClass);
	if (!g_pWeaponMgr->IsValidWeaponId(nWeapon) && g_pWeaponMgr->GetWeaponClass(m_nWeapon) != nClass)
	{
		m_nWeapon = -1;
		m_bIsOpen = false;
		return false;
	}

	m_nClass = nClass;
	m_bIsOpen = true;

	memset(m_nWeapons, 0, sizeof(m_nWeapons));
	memset(m_nClasses, 0, sizeof(m_nClasses));
	m_nNumWeapons = 0;
	m_nCurrent = -1;

	// The player's weapons after the first, in the order of their commands
	uint8 nLast = g_pWeaponMgr->GetLastWeaponCommandId();
	for (uint8 nCommand = g_pWeaponMgr->GetFirstWeaponCommandId() + 1; nCommand <= nLast; nCommand++)
	{
		uint8 nWeaponId = g_pWeaponMgr->GetWeaponId(nCommand);
		if (!g_pWeaponMgr->IsPlayerWeapon(nWeaponId))
			continue;

		if (!g_pWeaponMgr->IsValidWeaponId(nWeaponId))
			continue;

		if (!g_pTronPlayerStats->IsWeaponCompiled(nWeaponId))
			continue;

		m_nClasses[m_nNumWeapons] = g_pWeaponMgr->GetWeaponClass(nWeaponId);
		m_nWeapons[m_nNumWeapons] = nWeaponId;
		if (m_nWeapon == nWeaponId)
			m_nCurrent = m_nNumWeapons;
		m_nNumWeapons++;
	}

	m_AutoCloseTimer.Start(kfDelayTime);
	g_pHUDMgr->QueueUpdate(kHUDChooser);
	return true;
}

// RETAIL: 0x100afde0
void CTronWeaponChooser::Close(bool bShowSelection)
{
	if (g_pSubChooser)
		g_pSubChooser->Show(true);
	if (g_pProcedurals)
		g_pProcedurals->Show(true);

	CWeaponChooser::Close(true);

	if (bShowSelection)
		g_pHUDWeapon->SetWeapon(m_nWeapon);
}

// RETAIL: 0x100b0220. Class 1 takes any weapon.
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

	bool bFound = false;
	int nTries = 0;
	while (!bFound && nTries < m_nNumWeapons)
	{
		m_nCurrent++;
		nTries++;
		if (m_nCurrent >= m_nNumWeapons)
			m_nCurrent = 0;

		if (g_pTronPlayerStats->CanChangeToWeapon(m_nWeapons[m_nCurrent]))
		{
			bFound = (nClass == 1 || m_nClasses[m_nCurrent] == nClass);
		}
	}

	m_nWeapon = m_nWeapons[m_nCurrent];

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

	bool bFound = false;
	int nTries = 0;
	while (!bFound && nTries < m_nNumWeapons)
	{
		m_nCurrent--;
		nTries++;
		if (m_nCurrent < 0)
			m_nCurrent = m_nNumWeapons - 1;

		bFound = g_pTronPlayerStats->CanChangeToWeapon(m_nWeapons[m_nCurrent]);
	}

	m_nWeapon = m_nWeapons[m_nCurrent];

    g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());

   	m_AutoCloseTimer.Start(kfDelayTime);
	m_PrevWeaponKeyDownTimer.Start(g_vtChooserAutoSwitchTime.GetFloat());
	g_pHUDMgr->QueueUpdate(kHUDChooser);
}

// RETAIL: 0x100b0450. Tron's chooser closes itself only when the weapons
// are locked: the keys and the auto switch are not used.
void CTronWeaponChooser::Update()
{
	if (g_pTronPlayerMgr->IsHolsterLocked())
		Close(false);
}
