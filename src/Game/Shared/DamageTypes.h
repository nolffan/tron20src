// ----------------------------------------------------------------------- //
//
// MODULE  : DamageTypes.h
//
// PURPOSE : Definition of damage types
//
// CREATED : 11/26/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DAMAGE_TYPES_H__
#define __DAMAGE_TYPES_H__

#include "ltbasedefs.h"
#include "GameButeMgr.h"
#include <bitset>

#define DTMGR_DEFAULT_FILE		"Attributes\\DamageTypes.txt"

typedef uint64 DamageFlags;

struct DTINFO;
extern DTINFO DTInfoArray[];

// IMPORTANT NOTE:  If you add a new DamageType, make sure you update
// the DTInfoArray in DamageTypes.cpp!!!

enum DamageType
{
	DT_INVALID = -1,
#define INCLUDE_AS_ENUM
#include "DamageTypesEnum.h"
#undef INCLUDE_AS_ENUM
	kNumDamageTypes
};

// PORT: NOLF2's tracker gadget: Tron has no such damage type. NOLF2 code
// that still names it compares with a type no damage has.
#define DT_GADGET_TRACKER	DT_INVALID

static const char* pDTNames[kNumDamageTypes] =
{
#define INCLUDE_AS_STRING
#include "DamageTypesEnum.h"
#undef INCLUDE_AS_STRING
};

// RETAIL: Tron's DTINFO is 0x28 bytes (DTInfoArray's initializer, Object.lto
// 0x10125050, CShell.dll 0x1017b150): one name (the upper case one; NOLF2's
// bute name is pDTNames') and a "ForceWalk" value; none of NOLF2 1.3's
// score tag, grief, hurt animation and damage sounds
struct DTINFO
{
    DTINFO(DamageType type, const char* name)
	{
		eType		= type;
		nDamageFlag	= 0;
		pName		= name;
		bJarCamera	= LTFALSE;
		bGadget		= LTFALSE;
		bAccuracy	= LTFALSE;
		bForceWalk	= LTFALSE;
		fSlowMove	= 0.0f;
	}

	DamageType	eType;
	DamageFlags	nDamageFlag;
	const char*	pName;
	LTBOOL		bJarCamera;
	LTBOOL		bGadget;
	LTBOOL		bAccuracy;
	LTBOOL		bForceWalk;		// RETAIL: Tron's (+0x20). The name is ours
	LTFLOAT		fSlowMove;
};


class CDTButeMgr : public CGameButeMgr
{
public :

	CDTButeMgr();
	~CDTButeMgr();

    LTBOOL      Init(const char* szAttributeFile=DTMGR_DEFAULT_FILE);
	void		Term();

};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecificObjectFilterFn()
//
//	PURPOSE:	Filter a specific object out of CastRay and/or
//				IntersectSegment calls.
//
// ----------------------------------------------------------------------- //

inline bool SpecificObjectFilterFn(HOBJECT hObj, void *pUserData)
{
    if (!hObj) return false;

	return (hObj != (HOBJECT)pUserData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsJarCameraType()
//
//	PURPOSE:	Does this damage type cause the camera to be "jarred"
//
// ----------------------------------------------------------------------- //

LTBOOL IsJarCameraType(DamageType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsGadgetType()
//
//	PURPOSE:	Does this damage type a gadget damage type?
//
// ----------------------------------------------------------------------- //

LTBOOL IsGadgetType(DamageType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsAccuracyType()
//
//	PURPOSE:	Does this damage type count towards player accuracy
//
// ----------------------------------------------------------------------- //

LTBOOL IsAccuracyType(DamageType eType);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SlowMovementDuration()
//
//	PURPOSE:	For how long does this damage type slow movement
//
// ----------------------------------------------------------------------- //

LTFLOAT SlowMovementDuration(DamageType eType);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeToFlag()
//
//	PURPOSE:	Convert the damage type to its flag equivillent
//
// ----------------------------------------------------------------------- //

DamageFlags DamageTypeToFlag(DamageType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeToString()
//
//	PURPOSE:	Convert the damage type to its string equivillent
//
// ----------------------------------------------------------------------- //

const char* DamageTypeToString(DamageType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StringToDamageType()
//
//	PURPOSE:	Convert the string to its damage type equivillent
//
// ----------------------------------------------------------------------- //

DamageType StringToDamageType(const char* pDTName);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDTINFO()
//
//	PURPOSE:	Gets the damagetype structure.
//
// ----------------------------------------------------------------------- //

DTINFO const* GetDTINFO( DamageType eType );

#endif // __DAMAGE_TYPES_H__