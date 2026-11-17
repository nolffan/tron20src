// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileClusterDisc.cpp
//
// PURPOSE : Projectile disc class - implementation
//
// CREATED : 4/15/2002
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
//
// RETAIL: written from Tron's code (0x100356b0..0x10035f61).
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "FXButeMgr.h"
#include "ProjectileDisc.h"
#include "ProjectileClusterDisc.h"
#include "WeaponFireInfo.h"
#include "Weapons.h"
#include "Weapon.h"

//
// LithTech Macros
//
BEGIN_CLASS(CClusterDisc)
END_CLASS_DEFAULT_FLAGS(CClusterDisc, CDisc, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClusterDisc::CClusterDisc()
//
//	PURPOSE:	RETAIL: 0x10035700
//
// ----------------------------------------------------------------------- //

CClusterDisc::CClusterDisc()
	: CDisc()
{
	m_fFireTime = 0.0f;
	m_fExplosionSuppressTime = 0.0f;
	m_fTronUnknownA30 = 0.0f;
	m_bTronUnknownA34 = false;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClusterDisc::~CClusterDisc()
//
//	PURPOSE:	RETAIL: 0x100356b0
//
// ----------------------------------------------------------------------- //

CClusterDisc::~CClusterDisc()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClusterDisc::Setup()
//
//	PURPOSE:	RETAIL: 0x10035740
//
// ----------------------------------------------------------------------- //

LTBOOL CClusterDisc::Setup( CWeapon const *pWeapon, WeaponFireInfo const &info )
{
	// setup the base class
	LTBOOL bResult = CDisc::Setup( pWeapon, info );

	AMMO const *pAmmoData = m_pAmmoData;
	CLUSTERDISCCLASSDATA *pClusterData = pAmmoData ? dynamic_cast< CLUSTERDISCCLASSDATA* >( pAmmoData->pProjectileFX->pClassData ) : LTNULL;

	m_fExplosionSuppressTime = pClusterData->fInitialExplosionSuppressTime;
	m_fFireTime = g_pLTServer->GetTime();
	m_bTronUnknownA34 = true;

	return bResult;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClusterDisc::EngineMessageFn()
//
//	PURPOSE:	RETAIL: 0x100357b0
//
// ----------------------------------------------------------------------- //

uint32 CClusterDisc::EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData )
{
	switch ( messageID )
	{
		case MID_SAVEOBJECT:
		{
			Save( ( ILTMessage_Write* )pData, ( uint32 )fData );
		}
		break;

		case MID_LOADOBJECT:
		{
			Load( ( ILTMessage_Read* )pData, ( uint32 )fData );
		}
		break;
	}

	return CDisc::EngineMessageFn( messageID, pData, fData );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClusterDisc::EnterReturnMode()
//
//	PURPOSE:	Burst when the disc goes back to its owner, if the
//				suppress time is over
//				RETAIL: 0x10035820
//
// ----------------------------------------------------------------------- //

void CClusterDisc::EnterReturnMode( HOBJECT hObj )
{
	if ( !IsMainWorld( hObj ) )
	{
		if ( hObj ||
		     ( ( m_fTronUnknownA30 > 0.0f ) ? ( m_fTronUnknownA30 - m_fFireTime ) >= m_fExplosionSuppressTime
		                                    : ( g_pLTServer->GetTime() - m_fFireTime ) >= m_fExplosionSuppressTime ) )
		{
			Burst();
		}
	}

	CDisc::EnterReturnMode( hObj );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClusterDisc::Burst()
//
//	PURPOSE:	RETAIL: 0x100358c0
//
// ----------------------------------------------------------------------- //

void CClusterDisc::Burst()
{
	CreateExplosion();
	PlayBurstFX();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClusterDisc::CreateExplosion()
//
//	PURPOSE:	Create the explosion of the weapon's cluster modifier
//				RETAIL: 0x100358e0
//
// ----------------------------------------------------------------------- //

void CClusterDisc::CreateExplosion()
{
	// RETAIL: the cluster modifier (CProjectile 0xf3) has the explosion's
	// ammo at 0x188
	if ( !m_bTronUnknownF3 ) return;

	int nExplosionAmmoId = *reinterpret_cast< int* >( &m_fTronUnknown118[ 0x1c ] );
	if ( WMGR_INVALID_ID == nExplosionAmmoId ) return;

	HCLASS hClass = g_pLTServer->GetClass( "Explosion" );
	if ( !hClass ) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT( theStruct );

	g_pLTServer->GetObjectPos( m_hObject, &theStruct.m_Pos );

	LTRotation rRot;
	g_pLTServer->GetObjectRotation( m_hObject, &rRot );

	theStruct.m_Rotation = LTRotation( rRot.Forward(), rRot.Up() );

	LPBASECLASS pExplosion = g_pLTServer->CreateObject( hClass, &theStruct );
	if ( !pExplosion ) return;

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( nExplosionAmmoId );
	uint32 nAmmoId = pAmmo ? pAmmo->nId : WMGR_INVALID_ID;

	// RETAIL: message 0x3f3 sets the explosion up (Explosion::ObjectMessageFn
	// 0x100ef790)
	CAutoMessage cMsg;
	cMsg.Writeuint32( 0x3f3 );
	cMsg.WriteObject( m_hFiredFrom );
	cMsg.Writeuint32( nAmmoId );
	cMsg.Writebool( true );
	cMsg.Writeuint32( m_nWeaponId );
	// PORT: Tron writes the projectile's fire data here (CProjectile
	// 0x10121c70, not yet written)
	g_pLTServer->SendToObject( cMsg.Read(), m_hObject, pExplosion->m_hObject, MESSAGE_GUARANTEED );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClusterDisc::PlayBurstFX()
//
//	PURPOSE:	RETAIL: 0x10035dc0
//
// ----------------------------------------------------------------------- //

void CClusterDisc::PlayBurstFX()
{
	// RETAIL: the cluster modifier's FX name is at 0x108
	char *szFX = reinterpret_cast< char* >( &m_fTronUnknown108 );
	if ( m_bTronUnknownF3 && szFX )
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos( m_hObject, &vPos );

		LTRotation rRot;
		g_pLTServer->GetObjectRotation( m_hObject, &rRot );

		PlayClientFX( szFX, LTNULL, &vPos, &rRot, 0 );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClusterDisc::Save()
//
//	PURPOSE:	RETAIL: 0x10035e40
//
// ----------------------------------------------------------------------- //

void CClusterDisc::Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	if ( !pMsg ) return;

	SAVE_FLOAT( m_fFireTime - g_pLTServer->GetTime() );
	SAVE_TIME( m_fTronUnknownA30 );
	SAVE_FLOAT( m_fExplosionSuppressTime );
	SAVE_bool( m_bTronUnknownA34 );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClusterDisc::Load()
//
//	PURPOSE:	RETAIL: 0x10035ee0
//
// ----------------------------------------------------------------------- //

void CClusterDisc::Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags )
{
	if ( !pMsg ) return;

	LOAD_TIME( m_fFireTime );
	LOAD_TIME( m_fTronUnknownA30 );
	LOAD_FLOAT( m_fExplosionSuppressTime );
	LOAD_bool( m_bTronUnknownA34 );
}
