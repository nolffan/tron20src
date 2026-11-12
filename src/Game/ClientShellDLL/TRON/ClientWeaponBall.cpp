// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponBall.cpp
//
// PURPOSE : Tron's client-side ball weapon (retail only, written from
//           CShell.dll 0x10017670..0x10017c1f; see ClientWeaponBall.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponBall.h"

// RETAIL: 0x10017870
CClientWeaponBall::CClientWeaponBall()
	: CTronClientWeapon(),
	  m_fFireKeyDownTime( 0.0f )
{
}

// RETAIL: 0x10017670
CClientWeaponBall::~CClientWeaponBall()
{
}

// RETAIL: 0x10017890
void CClientWeaponBall::Load( ILTMessage_Read *pMsg )
{
	m_fFireKeyDownTime = pMsg->Readfloat() + g_pLTClient->GetTime();
	CTronClientWeapon::Load( pMsg );
}

// RETAIL: 0x100178d0
void CClientWeaponBall::Save( ILTMessage_Write *pMsg )
{
	pMsg->Writefloat( m_fFireKeyDownTime - g_pLTClient->GetTime() );
	CTronClientWeapon::Save( pMsg );
}

// RETAIL: 0x10017b70
void CClientWeaponBall::HandleFireKeyDownTransition()
{
	m_fFireKeyDownTime = g_pLTClient->GetTime();
}

