// ----------------------------------------------------------------------- //
//`
// MODULE  : ClientWeaponClusterDisc.cpp
//
// PURPOSE : Tron specific client-side cluster version of the disc
//
// CREATED : 4/12/02
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponClusterDisc.h"
#include "FXButeMgr.h"
#include "RatingMgr.h"
#include "SubroutineMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its statics' initializers (0x100180d0..)

// RETAIL: 0x10018190
CClientWeaponClusterDisc::CClientWeaponClusterDisc()
	: CClientWeaponDisc()
{
}



// RETAIL: 0x10018140
CClientWeaponClusterDisc::~CClientWeaponClusterDisc()
{
}



void CClientWeaponClusterDisc::AddExtraFireMessageInfo( bool bFire, ILTMessage_Write *pMsg )
{
	// first add the standard disc extra info
	CClientWeaponDisc::AddExtraFireMessageInfo( bFire, pMsg );

	// get the cluster specific data
	CLUSTERDISCCLASSDATA const *pClusterData = dynamic_cast< CLUSTERDISCCLASSDATA const * >( m_pAmmo->pProjectileFX->pClassData );

	// write the horizontal spread
	float fHorizontalSpread; 
	fHorizontalSpread = InterpolateRating< float >(
			Rating_Accuracy,
			pClusterData->fShardHorizontalSpreadMin,
			pClusterData->fShardHorizontalSpreadMax,
			pClusterData->fShardHorizontalSpreadSurge
		);
	pMsg->Writeuint16( CompressAngleToShort( fHorizontalSpread ) );

	// write the horizontal perturb
	float fHorizontalPerturb;
	fHorizontalPerturb = InterpolateRating< float >(
			Rating_Accuracy,
			pClusterData->fShardHorizontalPerturbMin,
			pClusterData->fShardHorizontalPerturbMax,
			pClusterData->fShardHorizontalPerturbSurge
		);
	pMsg->Writeuint8( CompressAngleToByte( fHorizontalPerturb ) );

	// write the vertical spread
	float fVerticalSpread;
	fVerticalSpread = InterpolateRating< float >(
			Rating_Accuracy,
			pClusterData->fShardVerticalSpreadMin,
			pClusterData->fShardVerticalSpreadMax,
			pClusterData->fShardVerticalSpreadSurge
		);
	pMsg->Writeuint8( CompressAngleToByte( fVerticalSpread ) );

	//
	// number of shards
	//

	// get the base number of shards
	int nNumberOfShards;
	nNumberOfShards = InterpolateRating< int >(
			Rating_Accuracy,
			pClusterData->nShardsTotalMin,
			pClusterData->nShardsTotalMax,
			pClusterData->nShardsTotalSurge
		);

	// get the perturb
	int nNumberOfShardsPreturb;
	nNumberOfShardsPreturb = InterpolateRating< int >(
			Rating_Accuracy,
			pClusterData->nShardsTotalPerturbMin,
			pClusterData->nShardsTotalPerturbMax,
			pClusterData->nShardsTotalPerturbSurge
		);

	// get the total number of shards
	int nTotalNumberOfShards = GetRandom( 
			( nNumberOfShards - nNumberOfShardsPreturb ),
			( nNumberOfShards + nNumberOfShardsPreturb )
		);
	ASSERT( ( 0 <= nTotalNumberOfShards ) && ( UCHAR_MAX > nTotalNumberOfShards ) );

	// write the total number of shards to the message
	pMsg->Writeuint8( static_cast< uint8 >( nTotalNumberOfShards ) );
}





// RETAIL: 0x100181b0
float CClientWeaponClusterDisc::GetEnergyUse()
{
	float fEnergy = CTronClientWeapon::GetEnergyUse();
	float fBase = GetBaseEnergyUse();

	if ( W_BEGIN_FIRING == GetState() )
	{
		if ( m_abTronUnknown13C[1][0] )
		{
			return fBase * m_fTronUnknown1E4 + fEnergy;
		}
	}
	else if ( g_pSubroutineMgr->IsUtilitySubroutineActive( "Cluster" ) )
	{
		ClusterData data = g_pSubroutineDataMgr->GetClusterData( GetSubroutineDataKey( "Cluster" ) );
		return data.fEnergyUseMultiplier * fBase + fEnergy;
	}

	return fEnergy;
}

// RETAIL: 0x1001bc00
bool CClientWeaponClusterDisc::TronUnknownVirtual18C()
{
	if ( TronUnknownVirtual184() && TronUnknownVirtual164() && TronUnknownVirtual19C() )
	{
		return false;
	}

	if ( W_FIRING_NOAMMO == GetState() ||
	     W_RELOADING == GetState() ||
	     W_SWAT_DEFENSE == GetState() ||
	     W_FIRED == GetState() ||
	     TronUnknownVirtual1A0() )
	{
		return false;
	}

	return true;
}
