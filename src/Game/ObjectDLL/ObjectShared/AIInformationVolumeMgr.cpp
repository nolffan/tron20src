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
	// RETAIL: 0x101990f0: Tron casts with dynamic_cast
	return dynamic_cast<AIInformationVolume*>( CAISpatialRepresentationMgr::FindContainingVolume( hObject, vPos, iAxisMask, fVerticalThreshhold, pVolumeStart, bBruteForce));
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIVolumePlayerInfoMgr::CAIVolumePlayerInfoMgr()
//              
//	PURPOSE:	RETAIL: 0x10199130: Tron's; hooks up g_pAIVolumePlayerInfoMgr
//              
//----------------------------------------------------------------------------
CAIVolumePlayerInfoMgr::CAIVolumePlayerInfoMgr()
{
	g_pAIVolumePlayerInfoMgr = this;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIVolumePlayerInfoMgr::~CAIVolumePlayerInfoMgr()
//              
//	PURPOSE:	RETAIL: 0x10199170: unhooks g_pAIVolumePlayerInfoMgr
//              
//----------------------------------------------------------------------------
CAIVolumePlayerInfoMgr::~CAIVolumePlayerInfoMgr()
{
	g_pAIVolumePlayerInfoMgr = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIVolumePlayerInfoMgr::Init()
//              
//	PURPOSE:	RETAIL: 0x10199190: Inits the base class with the name of the
//				Volumes to be used
//              
//----------------------------------------------------------------------------
void CAIVolumePlayerInfoMgr::Init()
{
	CAISpatialRepresentationMgr::Init( "AIVolumePlayerInfo" );
}

AIVolumePlayerInfo* CAIVolumePlayerInfoMgr::FindContainingVolume(HOBJECT hObject, const LTVector& vPos, int iAxisMask, LTFLOAT fVerticalThreshhold, AISpatialRepresentation* pVolumeStart, LTBOOL bBruteForce)
{
	// RETAIL: 0x101991a0
	return dynamic_cast<AIVolumePlayerInfo*>( CAISpatialRepresentationMgr::FindContainingVolume( hObject, vPos, iAxisMask, fVerticalThreshhold, pVolumeStart, bBruteForce));
}