// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponContinuousFireBase.cpp
//
// PURPOSE : Tron's base of its continuous fire weapons (retail only,
//           written from CShell.dll 0x10018330..0x100189b0; see
//           ClientWeaponContinuousFireBase.h).
//
//           Not yet written: PlayFireAnimation (0x10018590),
//           HandleFireKeyUpTransition (0x10018470; it clears two bools of
//           Tron's CPlayerMgr, +0x1a8/+0x1a9, ours lacks),
//           SendFireMessage (0x10018810) and TronUnknownVirtual164
//           (0x10018990; it needs CTronPlayerStats' float energy, 0x100a9c90).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponContinuousFireBase.h"

// RETAIL: 0x10018360
CClientWeaponContinuousFireBase::CClientWeaponContinuousFireBase()
	: CTronClientWeapon()
{
	m_bFiring = false;
	m_bKeyUpEarly = false;
}

// RETAIL: 0x10018330
CClientWeaponContinuousFireBase::~CClientWeaponContinuousFireBase()
{
}

// RETAIL: 0x10018380.  Retail's second argument (a bool) becomes
// TronUnknownVirtual164() while firing; ours has no such argument.
WeaponState CClientWeaponContinuousFireBase::Update( bool bFire, FireType eFireType )
{
	if ( bFire )
	{
		TronUnknownVirtual164();
	}

	return CTronClientWeapon::Update( bFire, eFireType );
}

// RETAIL: 0x100183c0
void CClientWeaponContinuousFireBase::Load( ILTMessage_Read *pMsg )
{
	m_bFiring = pMsg->Readbool();
	m_bKeyUpEarly = pMsg->Readbool();
	CTronClientWeapon::Load( pMsg );
}

// RETAIL: 0x10018400
void CClientWeaponContinuousFireBase::Save( ILTMessage_Write *pMsg )
{
	pMsg->Writebool( m_bFiring );
	pMsg->Writebool( m_bKeyUpEarly );
	CTronClientWeapon::Save( pMsg );
}

// RETAIL: 0x10018450
void CClientWeaponContinuousFireBase::HandleFireKeyDownTransition()
{
	m_bFiring = false;
	m_bKeyUpEarly = false;
	CTronClientWeapon::HandleFireKeyDownTransition();
}
