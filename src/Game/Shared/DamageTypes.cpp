// ----------------------------------------------------------------------- //
//
// MODULE  : DamageTypes.cpp
//
// PURPOSE : Implementation of damage types
//
// CREATED : 01/11/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DamageTypes.h"

DTINFO DTInfoArray[kNumDamageTypes] =
{
#define INCLUDE_AS_STRUCT
#include "DamageTypesEnum.h"
#undef INCLUDE_AS_STRUCT
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDTButeMgr::CDTButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CDTButeMgr::CDTButeMgr()
{

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDTButeMgr::~CDTButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CDTButeMgr::~CDTButeMgr()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDTButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CDTButeMgr::Init(const char* szAttributeFile)
{
    if (!szAttributeFile || !Parse(szAttributeFile)) return LTFALSE;

	// RETAIL: Tron's reads the five values of each type and its "ForceWalk"
	// (Object.lto 0x10125c70, CShell.dll 0x1017bd40); it has no score
	// tag, grief, hurt animation or damage sounds
	for (int i = 0; i < kNumDamageTypes; i++)
	{
		DTInfoArray[i].nDamageFlag = ( 1 << ( uint64 )i );

		DTInfoArray[i].bJarCamera = (LTBOOL)m_buteMgr.GetInt(pDTNames[i],"JarCamera",0);
		DTInfoArray[i].bGadget = (LTBOOL)m_buteMgr.GetInt(pDTNames[i],"Gadget",0);
		DTInfoArray[i].bAccuracy = (LTBOOL)m_buteMgr.GetInt(pDTNames[i],"Accuracy",0);
		DTInfoArray[i].bForceWalk = (LTBOOL)m_buteMgr.GetInt(pDTNames[i],"ForceWalk",0);
		DTInfoArray[i].fSlowMove = (LTFLOAT)m_buteMgr.GetDouble(pDTNames[i],"SlowMovement",0.0);
	}
	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDTButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CDTButeMgr::Term()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageFlagToType()
//
//	PURPOSE:	Convert the damage flag to its type equivillent
//
// ----------------------------------------------------------------------- //

DamageType DamageFlagToType(DamageFlags nDamageFlag)
{
	for (int i=0; i < kNumDamageTypes; i++)
	{
		if (DTInfoArray[i].nDamageFlag == nDamageFlag)
		{
			return DTInfoArray[i].eType;
		}
	}

	return DT_INVALID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeToFlag()
//
//	PURPOSE:	Convert the damage type to its flag equivillent
//
// ----------------------------------------------------------------------- //

DamageFlags DamageTypeToFlag(DamageType eType)
{
	for (int i=0; i < kNumDamageTypes; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return DTInfoArray[i].nDamageFlag;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeToString()
//
//	PURPOSE:	Convert the damage type to its string equivillent
//
// ----------------------------------------------------------------------- //

const char* DamageTypeToString(DamageType eType)
{
	for (int i=0; i < kNumDamageTypes; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return DTInfoArray[i].pName;
		}
	}

	return "INVALID";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeToString()
//
//	PURPOSE:	Convert the damage type to its string equivillent
//
// ----------------------------------------------------------------------- //

DamageType StringToDamageType(const char* pDTName)
{
	for (int i=0; i < kNumDamageTypes; i++)
	{
		// Check if it matches the name.
		// RETAIL: Tron has one name only (Object.lto 0x10125d80)
		if (_stricmp(DTInfoArray[i].pName, pDTName) == 0)
		{
			return DTInfoArray[i].eType;
		}
	}

	return DT_INVALID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsJarCameraType()
//
//	PURPOSE:	Does this damage type cause the camera to be "jarred"
//
// ----------------------------------------------------------------------- //

LTBOOL IsJarCameraType(DamageType eType)
{
	for (int i=0; i < kNumDamageTypes; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return DTInfoArray[i].bJarCamera;
		}
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsGadgetType()
//
//	PURPOSE:	Is this damage type a gadget damage type?
//
// ----------------------------------------------------------------------- //

LTBOOL IsGadgetType(DamageType eType)
{
	for (int i=0; i < kNumDamageTypes; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return DTInfoArray[i].bGadget;
		}
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsAccuracyType()
//
//	PURPOSE:	Does this damage type count towards player accuracy
//
// ----------------------------------------------------------------------- //

LTBOOL IsAccuracyType(DamageType eType)
{
	for (int i=0; i < kNumDamageTypes; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return DTInfoArray[i].bAccuracy;
		}
	}

	return LTFALSE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SlowMovementDuration()
//
//	PURPOSE:	For how long does this damage type slow movement
//
// ----------------------------------------------------------------------- //

LTFLOAT SlowMovementDuration(DamageType eType)
{
	for (int i=0; i < kNumDamageTypes; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return DTInfoArray[i].fSlowMove;
		}
	}

	return 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDTINFO()
//
//	PURPOSE:	Gets the damagetype structure.
//
// ----------------------------------------------------------------------- //

DTINFO const* GetDTINFO( DamageType eType )
{
	for (int i=0; i < kNumDamageTypes; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return &DTInfoArray[i];
		}
	}

	return NULL;
}


