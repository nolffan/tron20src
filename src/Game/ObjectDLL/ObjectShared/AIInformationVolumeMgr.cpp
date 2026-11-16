//----------------------------------------------------------------------------
//              
//	MODULE:		AIInformationVolumeMgr.cpp
//              
//	PURPOSE:	CAIInformationVolumeMgr implementation
//              
//	CREATED:	18.01.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#ifndef __AIINFORMATIONVOLUMEMGR_H__
#include "AIInformationVolumeMgr.h"		
#endif

#include "AIVolume.h"

// Forward declarations

// Globals

// Statics
						 
CAIInformationVolumeMgr* g_pAIInformationVolumeMgr = LTNULL;
CAIVolumePlayerInfoMgr* g_pAIVolumePlayerInfoMgr = LTNULL;	// RETAIL: Tron's (0x1038ca98)

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIInformationVolumeMgr::~CAIInformationVolumeMgr()
//              
//	PURPOSE:	Hooks the g_ppInformationVolumeMgr up to the InformationVolumeMgr
//              
//----------------------------------------------------------------------------
CAIInformationVolumeMgr::CAIInformationVolumeMgr()
{
	g_pAIInformationVolumeMgr = this;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIInformationVolumeMgr::~CAIInformationVolumeMgr()
//              
//	PURPOSE:	Unhooks the g_pAIInformationVolumeMgr
//              
//----------------------------------------------------------------------------
CAIInformationVolumeMgr::~CAIInformationVolumeMgr()
{
	g_pAIInformationVolumeMgr = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIInformationVolumeMgr::Init()
//              
//	PURPOSE:	Inits the base class with the name of the Volumes to be used
//              
//----------------------------------------------------------------------------
void CAIInformationVolumeMgr::Init()
{
	CAISpatialRepresentationMgr::Init( "AIInformationVolume" );
}

AIInformationVolume* CAIInformationVolumeMgr::FindContainingVolume(HOBJECT hObject, const LTVector& vPos, int iAxisMask, LTFLOAT fVerticalThreshhold, AISpatialRepresentation* pVolumeStart, LTBOOL bBruteForce)
{
	return (AIInformationVolume*)CAISpatialRepresentationMgr::FindContainingVolume( hObject, vPos, iAxisMask, fVerticalThreshhold, pVolumeStart, bBruteForce);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIVolumePlayerInfoMgr
//              
//	PURPOSE:	RETAIL: Tron's mgr of the AIVolumePlayerInfo volumes
//				(Object.lto 0x10199130..0x101991da)
//              
//----------------------------------------------------------------------------

CAIVolumePlayerInfoMgr::CAIVolumePlayerInfoMgr()
{
	g_pAIVolumePlayerInfoMgr = this;
}

CAIVolumePlayerInfoMgr::~CAIVolumePlayerInfoMgr()
{
	g_pAIVolumePlayerInfoMgr = NULL;
}

void CAIVolumePlayerInfoMgr::Init()
{
	CAISpatialRepresentationMgr::Init( "AIVolumePlayerInfo" );
}

AIVolumePlayerInfo* CAIVolumePlayerInfoMgr::FindContainingVolume(HOBJECT hObject, const LTVector& vPos, int iAxisMask, LTFLOAT fVerticalThreshhold, AISpatialRepresentation* pVolumeStart, LTBOOL bBruteForce)
{
	// RETAIL: a dynamic_cast (Object.lto 0x101991d2)
	return dynamic_cast<AIVolumePlayerInfo*>( CAISpatialRepresentationMgr::FindContainingVolume( hObject, vPos, iAxisMask, fVerticalThreshhold, pVolumeStart, bBruteForce) );
}
