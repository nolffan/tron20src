// ----------------------------------------------------------------------- //
//
// MODULE  : Destructible.cpp
//
// PURPOSE : Destructible class - Implementation
//
// CREATED : 9/23/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "Destructible.h"
#include "ParsedMsg.h"
#include "iltserver.h"
#include "MsgIds.h"
#include "Projectile.h"
#include "ServerUtilities.h"
#include "Character.h"
#include "PlayerObj.h"
#include "GameServerShell.h"
#include "ObjectMsgs.h"
#include "iltphysics.h"
#include "Camera.h"
#include "AIUtils.h"
#include "PlayerButes.h"
#include "AI.h"
#include "ServerMissionMgr.h"
#include "CharacterHitBox.h"

#define TRIGGER_MSG_DESTROY	"DESTROY"
#define TRIGGER_MSG_RESET	"RESET"

#define MAXIMUM_BLOCK_PRIORITY	255.0f
#define MINIMUM_FRICTION		5.0f
#define MAXIMUM_FRICTION		15.0f
#define MINIMUM_FORCE			0.0f

CVarTrack g_vtAIDamageAdjustEasy;
CVarTrack g_vtAIDamageAdjustNormal;
CVarTrack g_vtAIDamageAdjustHard;
CVarTrack g_vtAIDamageAdjustVeryHard;
CVarTrack g_vtAIDamageIncreasePerPlayer;
CVarTrack g_vtPlayerDamageDecreasePerPlayer;

extern CGameServerShell* g_pGameServerShell;
extern CVarTrack g_DamageScale;
extern CVarTrack g_HealScale;
const LTFLOAT DamageStruct::kInfiniteDamage = 100000.0f; // Infinite damage


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageStruct::InitFromMessage
//
//	PURPOSE:	Initialize from a MID_DAMAGE message
//
// ----------------------------------------------------------------------- //

LTBOOL DamageStruct::InitFromMessage(ILTMessage_Read *pMsg)
{
    if (!pMsg) return LTFALSE;

	vDir = pMsg->ReadLTVector();
    fDuration  = pMsg->Readfloat();
    fDamage    = pMsg->Readfloat() * g_DamageScale.GetFloat(1.0f);
    eType      = (DamageType)pMsg->Readuint8();
    hDamager   = pMsg->ReadObject();
    hContainer = pMsg->ReadObject();
    nAmmoId    = pMsg->Readuint8();
	bTronUnknown44 = pMsg->Readbool() ? LTTRUE : LTFALSE;	// RETAIL: Tron's (Destructible.h)

	// RETAIL: back to the start, not to where the damage began (NOLF2's)
	pMsg->SeekTo( 0 );

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageStruct::DoDamage
//
//	PURPOSE:	Send the MID_DAMAGE message to do the damage
//
// ----------------------------------------------------------------------- //

LTBOOL DamageStruct::DoDamage(LPBASECLASS pDamager, HOBJECT hVictim, HOBJECT hContainer)
{
    if (!pDamager || !hVictim) return LTFALSE;

	CAutoMessage cMsg;
	cMsg.Writeuint32(MID_DAMAGE);
	cMsg.WriteLTVector(vDir);
	cMsg.Writefloat(fDuration);
	cMsg.Writefloat(fDamage);
	cMsg.Writeuint8(eType);
	cMsg.WriteObject(hDamager);
	cMsg.WriteObject(hContainer);
	cMsg.Writeuint8(nAmmoId);
	cMsg.Writebool(bTronUnknown44 == LTTRUE);	// RETAIL: Tron's (Destructible.h)
	g_pLTServer->SendToObject(cMsg.Read(), pDamager->m_hObject, hVictim, MESSAGE_GUARANTEED);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::CDestructible
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CDestructible::CDestructible() : IAggregate()
{
    m_hObject                   = LTNULL;
    m_hLastDamager              = LTNULL;

    m_bDead                     = LTFALSE;
    m_bIsPlayer                 = LTFALSE;
    m_bApplyDamagePhysics       = LTFALSE;
	m_fMass						= 1.0f;
	m_fHitPoints				= 1.0f;
	m_fMaxHitPoints				= 1.0f;
	m_fArmorPoints				= 0.0f;
	m_fEnergy					= 1.0f;
	m_fMaxEnergy				= 1.0f;
	m_fMaxArmorPoints			= 1.0f;
	m_fDeathDamage				= 0.0f;
	m_fLastDamage				= 0.0f;
	m_fLastArmorAbsorb			= 0.0f;
	m_fTronUnknown78			= 1.0f;		// RETAIL: Tron's (Destructible.h)
	m_nCantDamageFlags			= DamageTypeToFlag(DT_GADGET_DECAYPOWDER);
	m_eLastDamageType			= DT_UNSPECIFIED;
	m_bTronUnknown30			= LTFALSE;
	m_eDeathType				= DT_UNSPECIFIED;
	m_bTronUnknown6D0			= false;
	m_fTronUnknown7C			= 0.0f;
	m_fTronUnknown6D4			= 0.0f;
	m_fTronUnknown6D8			= 0.0f;
	m_vTronUnknown6DC.Init();

	VEC_INIT(m_vDeathDir);
	VEC_INIT(m_vLastDamageDir);

	m_fDamagePercentCommand			= 0.0f;
	m_nDamageTriggerCounter			= 0;
	m_nDamageTriggerNumSends		= 1;

    m_bCanHeal                  = LTTRUE;
    m_bCanRepair                = LTTRUE;
    m_bCanDamage                = LTTRUE;
    m_bNeverDestroy             = LTFALSE;

	// damage filter function
    m_pDamageFilterFunction = 0;
    m_pDamageFilterObject = 0;

	memset(m_bGearOwned, 0, sizeof(m_bGearOwned));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::~CDestructible
//
//	PURPOSE:	Destructible
//
// ----------------------------------------------------------------------- //

CDestructible::~CDestructible()
{

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //

uint32 CDestructible::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_TOUCHNOTIFY:
		{
			HandleTouch(pObject, (HOBJECT)pData, fData);
		}
		break;

		case MID_CRUSH:
		{
			HandleCrush(pObject, (HOBJECT)pData);
		}
		break;

		case MID_PRECREATE:
		{
			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp(pObject, (ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate(pObject);
			}
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint8)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint8)fData);
		}
		break;
	}

    return IAggregate::EngineMessageFn(pObject, messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CDestructible::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();
	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HandleTrigger(pObject, hSender, pMsg);
		}
		break;

		case MID_DAMAGE:
		{
			HandleDamage(pObject, hSender, pMsg);
		}
		break;

		case MID_REPAIR:
		{
			if (m_bCanRepair)
			{
				HandleRepair(pObject, hSender, pMsg);
			}
		}
		break;

		case MID_HEAL:
		{
			if (m_bCanHeal)
			{
				HandleHeal(pObject, hSender, pMsg);
			}
		}
		break;

		case MID_ADDGEAR:
		{
			HandleAddGear(pObject, hSender, pMsg);
		}
		break;

		default : break;
	}

    return IAggregate::ObjectMessageFn(pObject, hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::ReadProp(LPBASECLASS pObject, ObjectCreateStruct *pStruct)
{
    if (!pStruct) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("Mass", &genProp) == LT_OK)
	{
		SetMass(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("HitPoints", &genProp) == LT_OK)
	{
		SetHitPoints(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("MaxHitPoints", &genProp) == LT_OK)
	{
		SetMaxHitPoints(genProp.m_Float);
	}

	if (g_pLTServer->GetPropGeneric("Energy", &genProp) == LT_OK)
	{
		SetEnergy(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("MaxEnergy", &genProp) == LT_OK)
	{
		SetMaxEnergy(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("Armor", &genProp) == LT_OK)
	{
		SetArmorPoints(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("MaxArmor", &genProp) == LT_OK)
	{
		SetMaxArmorPoints(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("DamagePercentForCommand", &genProp) == LT_OK)
	{
		// RETAIL: Tron takes a percentage (0x100e720e).
		SetDamagePercentCommand(genProp.m_Float * 0.01f);
	}
   
    if (g_pLTServer->GetPropGeneric("DamageCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_sDamageCommand = genProp.m_String;
		}
	}

    if (g_pLTServer->GetPropGeneric("PlayerDamageCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_sPlayerDamageCommand = genProp.m_String;
		}
	}

    if (g_pLTServer->GetPropGeneric("DamageTriggerCounter", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_nDamageTriggerCounter = genProp.m_Long;
		}
	}

    if (g_pLTServer->GetPropGeneric("DamageTriggerNumSends", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_nDamageTriggerNumSends = genProp.m_Long;
		}
	}

    if (g_pLTServer->GetPropGeneric("DamagerMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_sDamagerMessage = genProp.m_String;
		}
	}

    if (g_pLTServer->GetPropGeneric("DamagePercentCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_sDamagePercentCommand = genProp.m_String;
		}
	}

    if (g_pLTServer->GetPropGeneric("DeathCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
           	m_sDeathCommand = genProp.m_String;
		}
	}
    
	if (g_pLTServer->GetPropGeneric("PlayerDeathCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_sPlayerDeathCommand = genProp.m_String;
		}
	}

    if (g_pLTServer->GetPropGeneric("KillerMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_sKillerMessage = genProp.m_String;
		}
	}

    if (g_pLTServer->GetPropGeneric("CanHeal", &genProp) == LT_OK)
	{
		m_bCanHeal = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("CanRepair", &genProp) == LT_OK)
	{
		m_bCanRepair = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("CanDamage", &genProp) == LT_OK)
	{
		m_bCanDamage = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("NeverDestroy", &genProp) == LT_OK)
	{
		m_bNeverDestroy = genProp.m_Bool;
	}


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Init
//
//	PURPOSE:	Initialize object - no longer used (InitialUpdate() does this)
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::Init(HOBJECT hObject)
{
	if (!m_hObject) m_hObject = hObject;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::InitialUpdate
//
//	PURPOSE:	Handle object initial update
//
// ----------------------------------------------------------------------- //

void CDestructible::InitialUpdate(LPBASECLASS pObject)
{
	if (!pObject || !pObject->m_hObject) return;
	if (!m_hObject) m_hObject = pObject->m_hObject;

	// Set the mass, which will also set the blocking priority and friction coefficient...

	SetMass( m_fMass );
	
	// Determine if we are a player object or not

	if (IsPlayer(m_hObject))
	{
        m_bIsPlayer = LTTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Update
//
//	PURPOSE:	Handle object updates
//
// ----------------------------------------------------------------------- //

void CDestructible::Update()
{
	// Update progressive damage...Don't allow progressive damage to happen 
	// in cinematics (fixes bugs with the player breaking cinematics)...

	if (Camera::IsActive())
	{
		ClearProgressiveDamage();
	}
	else
	{
		UpdateProgressiveDamage();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ClearProgressiveDamage
//
//	PURPOSE:	Remove all, or a specific DamageType, progressive damage being done to the object.
//
// ----------------------------------------------------------------------- //

void CDestructible::ClearProgressiveDamage( DamageType DT /*= DT_INVALID*/, bool bUpdateProgressiveDamage /*= true*/ )
{
	for (int i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		if( (DT == DT_INVALID) || (DT == m_ProgressiveDamage[i].eType) )
		{
			m_ProgressiveDamage[i].fDuration = 0.0f;
		}
	}
	
	if ( bUpdateProgressiveDamage )
	{
		UpdateProgressiveDamage();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ClearProgressiveDamage
//
//	PURPOSE:	Remove all, or a specific DamageType, progressive damage being done to the object.
//
// ----------------------------------------------------------------------- //

void CDestructible::ClearProgressiveDamage( DamageFlags nDamageFlag )
{
	for ( DamageType i = static_cast< DamageType >( DT_INVALID + 1 );
	      i < kNumDamageTypes;
	      i = static_cast< DamageType >( i + 1 ) )
	{
		if ( DamageTypeToFlag( i ) & nDamageFlag )
		{
			ClearProgressiveDamage( i, false );
		}
	}

	UpdateProgressiveDamage();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::UpdateProgressiveDamage
//
//	PURPOSE:	Update any progressive damage being done to the object.
//
// ----------------------------------------------------------------------- //

void CDestructible::UpdateProgressiveDamage()
{
	// RETAIL: Tron's (0x100e7890) is Build 51's: a flag per progressive
	// damage type, with Tron's slippery and corrode types added. A spent
	// damage is cleared, and the power armor of a character's hit box takes
	// the damage first.

	DamageStruct damage;

    LTFLOAT fFrameTime = g_pLTServer->GetFrameTime();

    LTBOOL bBleeding = LTFALSE, bPoisoned = LTFALSE, bStunned = LTFALSE, bSleeping = LTFALSE;
    LTBOOL bBurned = LTFALSE, bElectrocuted = LTFALSE, bGlue = LTFALSE, bLaughing = LTFALSE;
	LTBOOL bASSS = LTFALSE, bFreeze = LTFALSE, bBearTrap = LTFALSE, bSlippery = LTFALSE;
	LTBOOL bCorrode = LTFALSE;

	CCharacterHitBox* pHitBox = LTNULL;
	if ( IsCharacter( m_hObject ))
	{
		CCharacter* pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject( m_hObject ));
		if ( pChar )
		{
			pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject( pChar->GetHitBoxObject() ));
		}
	}

	// Loop over all the progressive damage done to us...

	for (int i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		if (m_ProgressiveDamage[i].fDuration > 0.0f)
		{
			m_ProgressiveDamage[i].fDuration -= fFrameTime;

			// Process the damage for this frame...

			damage = m_ProgressiveDamage[i];
			damage.fDamage = damage.fDamage * fFrameTime;

			if ( pHitBox )
			{
				pHitBox->AbsorbDamage( &damage );
			}

			DoDamage(damage, false);

			// The damage is spent: forget it.

			if (m_ProgressiveDamage[i].fDuration <= 0.0f)
			{
				m_ProgressiveDamage[i].Clear();
			}

			switch( damage.eType )
			{
				case DT_BLEEDING	:	bBleeding		= LTTRUE;	break;
				case DT_POISON		:	bPoisoned		= LTTRUE;	break;
				case DT_STUN		:	bStunned		= LTTRUE;	break;
				case DT_SLEEPING	:	bSleeping		= LTTRUE;	break;
				case DT_BURN		:	bBurned			= LTTRUE;	break;
				case DT_ELECTROCUTE	:	bElectrocuted	= LTTRUE;	break;
				case DT_GLUE		:	bGlue			= LTTRUE;	break;
				case DT_BEAR_TRAP	:	bBearTrap		= LTTRUE;	break;
				case DT_LAUGHING	:	bLaughing		= LTTRUE;	break;
				case DT_ASSS		:	bASSS			= LTTRUE;	break;
				case DT_FREEZE		:	bFreeze			= LTTRUE;	break;
				case DT_SLIPPERY	:	bSlippery		= LTTRUE;	break;
				case DT_CORRODE		:	bCorrode		= LTTRUE;	break;
				default : break;
			}
		}
	}


	// See if we have protection from bleeding damage...

	if (bBleeding)
	{
        LTFLOAT fProtection = 0.0f;

        GEAR const *pGear = LTNULL;

		for (int nGearId=0; nGearId < MAX_GEAR_ITEMS; nGearId++)
		{
			if (m_bGearOwned[nGearId])
			{
				pGear = g_pWeaponMgr->GetGear(nGearId);

				if (pGear && pGear->eProtectionType == DT_BLEEDING)
				{
					fProtection += pGear->fProtection;

					if (fProtection >= 1.0f)
					{
                        bBleeding = LTFALSE;
						break;
					}
				}
			}
		}
	}


	// Set our damage flags if we're bleeding/poisoned/stunned/sleeping...

	if (IsCharacter(m_hObject))
	{
        // Get the character...

		CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject( m_hObject ));
		if( !pChar ) return;

		// Set the damage flags ...

		DamageFlags nDmgFlags = pChar->GetDamageFlags();

		bBleeding ?
			nDmgFlags |= DamageTypeToFlag( DT_BLEEDING ) :
			nDmgFlags &= ~DamageTypeToFlag( DT_BLEEDING );

		bPoisoned ?
			nDmgFlags |= DamageTypeToFlag( DT_POISON ) :
			nDmgFlags &= ~DamageTypeToFlag( DT_POISON );

		bStunned ?
			nDmgFlags |= DamageTypeToFlag( DT_STUN ) :
			nDmgFlags &= ~DamageTypeToFlag( DT_STUN );

		bSleeping ?
			nDmgFlags |= DamageTypeToFlag( DT_SLEEPING ) :
			nDmgFlags &= ~DamageTypeToFlag( DT_SLEEPING );

		bBurned ?
			nDmgFlags |= DamageTypeToFlag( DT_BURN ) :
			nDmgFlags &= ~DamageTypeToFlag( DT_BURN );

		bElectrocuted ?
			nDmgFlags |= DamageTypeToFlag( DT_ELECTROCUTE ) :
			nDmgFlags &= ~DamageTypeToFlag( DT_ELECTROCUTE );

		bGlue ?
			nDmgFlags |= DamageTypeToFlag( DT_GLUE ) :
			nDmgFlags &= ~DamageTypeToFlag( DT_GLUE );

		bBearTrap ?
			nDmgFlags |= DamageTypeToFlag( DT_BEAR_TRAP ) :
			nDmgFlags &= ~DamageTypeToFlag( DT_BEAR_TRAP );

		bLaughing ?
			nDmgFlags |= DamageTypeToFlag( DT_LAUGHING ) :
			nDmgFlags &= ~DamageTypeToFlag( DT_LAUGHING );

		bASSS ?
			nDmgFlags |= DamageTypeToFlag( DT_ASSS ) :
			nDmgFlags &= ~DamageTypeToFlag( DT_ASSS );

		bFreeze ?
			nDmgFlags |= DamageTypeToFlag( DT_FREEZE ) :
			nDmgFlags &= ~DamageTypeToFlag( DT_FREEZE );

		bSlippery ?
			nDmgFlags |= DamageTypeToFlag( DT_SLIPPERY ) :
			nDmgFlags &= ~DamageTypeToFlag( DT_SLIPPERY );

		bCorrode ?
			nDmgFlags |= DamageTypeToFlag( DT_CORRODE ) :
			nDmgFlags &= ~DamageTypeToFlag( DT_CORRODE );

		pChar->SetInstantDamageFlags( nDmgFlags );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::AddProgressiveDamage
//
//	PURPOSE:	Add progressive damage to the object
//
// ----------------------------------------------------------------------- //

void CDestructible::AddProgressiveDamage(DamageStruct & damage)
{
    LTFLOAT fLeastDuration = 100000.0f;
	int nIndex = -1;
	
	HOBJECT hDamager	= damage.hDamager;
    LPBASECLASS pObject = g_pLTServer->HandleToObject(m_hObject);

	// RETAIL: Tron has no test of m_bDead here (0x100e7f70).

	// If the new damage is from a container, see if this container
	// is already damaging us...

    int i;
	if (damage.hContainer)
	{
        for (i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
		{
			if (m_ProgressiveDamage[i].hContainer == damage.hContainer)
			{
				m_ProgressiveDamage[i] = damage;
				return;
			}
		}
	}

	// RETAIL: Tron keeps one PRod, EnergyClaw or Corrode damage at a time:
	// the longer one stays (0x100e7fb4).

	if (damage.eType == DT_PROD || damage.eType == DT_ENERGYCLAW || damage.eType == DT_CORRODE)
	{
		for (i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
		{
			if (m_ProgressiveDamage[i].eType == damage.eType)
			{
				if (m_ProgressiveDamage[i].fDuration < damage.fDuration)
				{
					m_ProgressiveDamage[i] = damage;
				}
				return;
			}
		}
	}

	// RETAIL: Tron scores no tags (retail CDestructible::AddProgressiveDamage,
	// 0x100e7f70, has none of NOLF2's tag scoring; nor has Tron's
	// CPlayerScore tags, SharedScoring.h)




    for (i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		if (m_ProgressiveDamage[i].fDuration <= 0.0f)
		{
			m_ProgressiveDamage[i] = damage;
			return;
		}
		else
		{
			if (m_ProgressiveDamage[i].fDuration < fLeastDuration)
			{
				fLeastDuration = m_ProgressiveDamage[i].fDuration;
				nIndex = i;
			}
		}
	}

	// If we got to here there were no empty slots...Replace the slot
	// with the least amount of time if the new progressive damage item
	// would do more damage...

	if (nIndex != -1)
	{
        LTFLOAT fDamageLeft = m_ProgressiveDamage[nIndex].fDuration * m_ProgressiveDamage[nIndex].fDamage;
        LTFLOAT fNewDamage = damage.fDuration * damage.fDamage;

		// This assumes that all damage is equal.  In the future we may
		// want to check for powerups or other items that could make this
		// assumption false...

		if (fNewDamage > fDamageLeft)
		{
			m_ProgressiveDamage[nIndex] = damage;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::IsTakingProgressiveDamage
//
//	PURPOSE:	returns TRUE if currently affected by specified type of progressive damage.
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::IsTakingProgressiveDamage( DamageType DT )
{
	for (int i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		if( ( DT == m_ProgressiveDamage[i].eType ) && 
			( m_ProgressiveDamage[i].fDuration > 0.0f ) )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::SetMass
//
//	PURPOSE:	Set the blocking priority for this object
//
// ----------------------------------------------------------------------- //

void CDestructible::SetMass(LTFLOAT fMass)
{
	// Make sure we don't try and go above infinite mass as that will mess with the blocking priority...

	m_fMass = Clamp( fMass, 0.0f, INFINITE_MASS - 1.0f );

	SetBlockingPriority();

	// Set the friction based on the mass of the object...

	if (!m_hObject) return;

    g_pPhysicsLT->SetMass(m_hObject, m_fMass);

    LTFLOAT fFricCoeff = MINIMUM_FRICTION + (m_fMass * MAXIMUM_FRICTION / INFINITE_MASS);
	g_pPhysicsLT->SetFrictionCoefficient(m_hObject, fFricCoeff);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::SetBlockingPriority
//
//	PURPOSE:	Set the blocking priority for this object
//
// ----------------------------------------------------------------------- //

void CDestructible::SetBlockingPriority()
{
	if (!m_hObject) return;

    uint8 nPriority = (uint8)(m_fMass * MAXIMUM_BLOCK_PRIORITY / INFINITE_MASS);
	if (nPriority <= 0) nPriority = 1;

    g_pLTServer->SetBlockingPriority(m_hObject, nPriority);
    g_pPhysicsLT->SetForceIgnoreLimit(m_hObject, MINIMUM_FORCE);

// BEP 7/9/02 Why is this setting touch notify??  Removed.
//	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_TOUCH_NOTIFY, FLAG_TOUCH_NOTIFY);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::GetMaxHitPoints
//
//	PURPOSE:	Returns the maximum hit points for this object
//
// ----------------------------------------------------------------------- //

LTFLOAT CDestructible::GetMaxHitPoints() const
{
    LTFLOAT fAdjustedMax = m_fMaxHitPoints;

	if (m_bIsPlayer)
	{
        CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hObject);
		if (pPlayer)
		{
			// See if we should increase/decrease the max...
		}
	}

	return fAdjustedMax;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::GetMaxEnergy
//
//	PURPOSE:	Returns the maximum energy points for this object
//
// ----------------------------------------------------------------------- //

LTFLOAT CDestructible::GetMaxEnergy() const
{
    LTFLOAT fAdjustedMax = m_fMaxEnergy;

	if (m_bIsPlayer)
	{
        CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hObject);
		if (pPlayer)
		{
			// See if we should increase/decrease the max...
		}
	}

	return fAdjustedMax;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::GetMaxArmorPoints
//
//	PURPOSE:	Returns the maximum armor points for this object
//
// ----------------------------------------------------------------------- //

LTFLOAT CDestructible::GetMaxArmorPoints() const
{
    LTFLOAT fAdjustedMax = m_fMaxArmorPoints;

	if (m_bIsPlayer)
	{
        CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hObject);
		if (pPlayer)
		{
			// See if we should increase/decrease the max...
		}
	}

	return fAdjustedMax;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::GetStealthModifier
//
//	PURPOSE:	Returns footstep sound volume multiplier for the current gear
//
// ----------------------------------------------------------------------- //

LTFLOAT CDestructible::GetStealthModifier()
{
    LTFLOAT fMult = 1.0f;
    GEAR const *pGear = LTNULL;

	for (int nGearId=0; nGearId < MAX_GEAR_ITEMS; nGearId++)
	{
		if (m_bGearOwned[nGearId])
		{
			pGear = g_pWeaponMgr->GetGear(nGearId);
			if (pGear)
			{
				fMult *= (1.0f - pGear->fStealth);
			}
		}
	}

	if (m_bIsPlayer)
	{
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hObject);
		if (pPlayer)
		{
			// RETAIL: Tron reads the stealth table directly (0x100e83cb).
			fMult *= g_pSkillsButeMgr->GetStealth(pPlayer->GetPlayerSkills()->GetLevel(SKL_STEALTH))->fModifier[StealthModifiers::eRadius];
		}
	}

	return fMult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HasAirSupply
//
//	PURPOSE:	Returns true if current gear provides air supply
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::HasAirSupply()
{
    LTBOOL bAir = LTFALSE;
    GEAR const *pGear = LTNULL;

	for (int nGearId=0; nGearId < MAX_GEAR_ITEMS && !bAir; nGearId++)
	{
		if (m_bGearOwned[nGearId])
		{
			pGear = g_pWeaponMgr->GetGear(nGearId);
			if (pGear)
			{
				bAir = ( (pGear->eProtectionType == DT_CHOKE) && (pGear->fProtection >= 1.0f) );
			}
		}
	}
	return bAir;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::RegisterFilterFunction
//
//	PURPOSE:	Register the filter function
//
// ----------------------------------------------------------------------- //

LTBOOL  CDestructible::RegisterFilterFunction( DamageFilterFunction pFn, GameBase *pObject )
{
	if ( ( 0 == pFn ) || ( 0 == pObject ) )
	{
		// if one parameter is non-null, the both must be nonnull
		ASSERT( 0 != pFn );
		ASSERT( 0 != pObject );
	}

	m_pDamageFilterFunction = pFn;
	m_pDamageFilterObject = pObject;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleDamage
//
//	PURPOSE:	Handle damage message
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleDamage(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	DamageStruct damage;
	damage.InitFromMessage(pMsg);

	// Send things through the filer function, if any...
	if( m_pDamageFilterFunction )
	{
		bool bResult = m_pDamageFilterFunction( m_pDamageFilterObject, &damage );
		if ( !bResult )
		{
			// the filter return false, which means to disregard
			// processing this damage
			return;
		}
	}

	// Check for progressive damage...

	if (damage.fDuration > 0.0f)
	{
		AddProgressiveDamage(damage);
	}
	else
	{
		DoDamage(damage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::DoDamage
//
//	PURPOSE:	Do the damage
//
// ----------------------------------------------------------------------- //

// RETAIL: Object.lto 0x100e7f10.  Tron applies the hit's flags to a
// character after the damage.
void CDestructible::DoDamage(DamageStruct & damage)
{
	DoDamage(damage, true);

	if (IsCharacter(m_hObject))
	{
		CCharacter *pChar = (CCharacter*)g_pLTServer->HandleToObject(m_hObject);
		if (pChar)
		{
			pChar->AddDamageFlags(DamageTypeToFlag(damage.eType));
		}
	}
}

// RETAIL: Object.lto 0x100e85c0.  The bool is ours: when it is false, the
// damage triggers are not sent.
void CDestructible::DoDamage(DamageStruct & damage, bool bSendTriggers)
{
    LTVector vDir		= damage.vDir;
    LTFLOAT fDamage		= damage.fDamage;
	DamageType eType	= damage.eType;
	HOBJECT hDamager	= damage.hDamager;
	LTBOOL bTronUnknown	= damage.bTronUnknown44;

    LPBASECLASS pObject = g_pLTServer->HandleToObject(m_hObject);

	// See if we should actually process damage...

	if (m_bDead) 
		return;


	// See if this type of damage applies to us...

	if (m_nCantDamageFlags)
	{
		if (m_nCantDamageFlags & DamageTypeToFlag(eType)) 
			return;
	}

	if (!IsPlayer(m_hObject) && eType == DT_PLAYERONLY)
		return;

	if (m_bTronUnknown6D0 && !IsPlayer(damage.hDamager))
	{
		if (!bSendTriggers)
			return;

	// If this is supposed to send a damage trigger, send it now...

		if (m_nDamageTriggerCounter <= 0)
		{
			if (m_nDamageTriggerNumSends != 0)
			{
				m_nDamageTriggerNumSends--;

				if(m_sDamageCommand.length() )
				{
					const char *pCmd = m_sDamageCommand.c_str();

					if( g_pCmdMgr->IsValidCmd( pCmd ) )
					{
						g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
					}
				}

				if( m_sPlayerDamageCommand.size( ) != 0 && IsPlayer( hDamager ))
				{
					const char *pCmd = m_sPlayerDamageCommand.c_str( );

					if( g_pCmdMgr->IsValidCmd( pCmd ) )
					{
						g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
					}
				}
			}
		}

		// If we ought to send a percentage based damage command, do it now.

		if ( m_sDamagePercentCommand.length() && (m_fHitPoints < m_fMaxHitPoints * m_fDamagePercentCommand) )
		{
			const char *pCmd = m_sDamagePercentCommand.c_str();

			if( g_pCmdMgr->IsValidCmd( pCmd ) )
			{
				g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
			}
		}
		
		// Send message to object that damaged us...

		if (hDamager && m_sDamagerMessage.length())
		{
			SendTriggerMsgToObject(pObject, hDamager, LTTRUE, m_sDamagerMessage.c_str());
		}
	
		return;
	}

	// If the damage is only for worldmodels make sure we are a world model...

	if ( eType == DT_WORLDONLY && !IsWorldModel( m_hObject ) )
		return;
	

	// Adjust the damage based on difficulty, who I am and who I damaged...

	fDamage *= m_fTronUnknown78;
	fDamage = AdjustDamage(fDamage, hDamager);
	

	// Process damage through any gear items...

	fDamage = ProcessGear(fDamage, eType, hDamager, &vDir);

    LTFLOAT fAbsorb = 0.0f;

	// Do rag-doll, and adjust damage if it was greater than 0...(if the damage
	// was zero, we still want to process it)...

	if (fDamage > 0.0f)
	{
		// Rag-doll time...

		if (CanRagDoll(vDir, eType))
		{
            LTVector vImpactVel, vTemp, vVel;

            g_pPhysicsLT->GetVelocity(m_hObject, &vVel);

			VEC_COPY(vTemp, vDir);
			VEC_NORM(vTemp);

			if (m_fMass <= 0.0f)
			{
				m_fMass = 100.0f;
			}

            LTFLOAT fMultiplier = fDamage * (INFINITE_MASS/(m_fMass*5.0f));

			// If not on ground, scale down velocity...

			CollisionInfo standingInfo;
            g_pLTServer->GetStandingOn(m_hObject, &standingInfo);

			if (!standingInfo.m_hObject)
			{
				fMultiplier /= 10.0f;
			}

			VEC_MULSCALAR(vTemp, vTemp, fMultiplier);
			VEC_ADD(vImpactVel, vTemp, vVel);

            g_pPhysicsLT->SetVelocity(m_hObject, &vImpactVel);
		}


		// Basically armor just absorbs as much of the damage
		// as possible...

		if (m_fArmorPoints > 0.0 && ArmorCanAbsorb(eType))
		{
			LTFLOAT fAdjustedArmor = m_fArmorPoints;
			fAbsorb = (fDamage > fAdjustedArmor ? fAdjustedArmor : fDamage);
			fDamage	-= fAbsorb;
		}
	}

	// [KLS 7/15/02] Negative damage will actually give us hit points, so make sure
	// we don't do that ;)
	if (fDamage < 0.0)
	{
		fDamage = 0.0f;
	}

	// [KLS 7/15/02] If we're a player, now that we have processed gear and armor, 
	// adjust the actual damage done based on our skill level...

	if (m_bIsPlayer)
	{
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hObject);
		if (pPlayer)
		{
			// RETAIL: Tron reads the stamina table directly (0x100e8ac4).
			fDamage *= g_pSkillsButeMgr->GetStamina(pPlayer->GetPlayerSkills()->GetLevel(SKL_STAMINA))->fModifier[StaminaModifiers::eDamage];
		}
	}


	// Save these for later...

	m_fLastArmorAbsorb	= fAbsorb;
	m_fLastDamage		= fDamage;
	m_eLastDamageType	= eType;
	m_bTronUnknown30	= bTronUnknown;
	m_vLastDamageDir	= vDir;
	SetLastDamager(hDamager);


	// Actually apply the damage

	if (!m_bCanDamage || DebugDamageOn())
	{
		return;
	}

	m_fArmorPoints -= fAbsorb;

	if (m_fArmorPoints < 0.0f)
	{
		m_fArmorPoints = 0.0f;
	}

	// RETAIL: the test is made before the subtraction (0x100e8b55).
	if (m_fTronUnknown7C != 0.0f)
	{
		m_fHitPoints -= fDamage;
		LTFLOAT fMax = GetMaxHitPoints();
		m_fHitPoints = LTMIN(fMax, LTMAX(m_fHitPoints, m_fTronUnknown7C));
	}
	else
	{
		m_fHitPoints -= fDamage;
	}

	if (bSendTriggers)
	{
		if ( m_nDamageTriggerCounter > 0 )
		{
			m_nDamageTriggerCounter--;
		}

	// If this is supposed to send a damage trigger, send it now...

		if (m_nDamageTriggerCounter <= 0)
		{
			if (m_nDamageTriggerNumSends != 0)
			{
				m_nDamageTriggerNumSends--;

				if(m_sDamageCommand.length() )
				{
					const char *pCmd = m_sDamageCommand.c_str();

					if( g_pCmdMgr->IsValidCmd( pCmd ) )
					{
						g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
					}
				}

				if( m_sPlayerDamageCommand.size( ) != 0 && IsPlayer( hDamager ))
				{
					const char *pCmd = m_sPlayerDamageCommand.c_str( );

					if( g_pCmdMgr->IsValidCmd( pCmd ) )
					{
						g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
					}
				}
			}
		}

		// If we ought to send a percentage based damage command, do it now.

		if ( m_sDamagePercentCommand.length() && (m_fHitPoints < m_fMaxHitPoints * m_fDamagePercentCommand) )
		{
			const char *pCmd = m_sDamagePercentCommand.c_str();

			if( g_pCmdMgr->IsValidCmd( pCmd ) )
			{
				g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
			}
		}
		
		// Send message to object that damaged us...

		if (hDamager && m_sDamagerMessage.length())
		{
			SendTriggerMsgToObject(pObject, hDamager, LTTRUE, m_sDamagerMessage.c_str());
		}
	
	}

	// Send instant damage flags for characters...

	if( IsCharacter( m_hObject ) && !(damage.fDuration > 0.0f) )
	{
		CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject( m_hObject ));
		if( pChar )
		{
			pChar->AddDamageFlags( DamageTypeToFlag( eType ));
		}
	}


	// See if we're dead...

	if (m_fHitPoints <= 1.0f && !m_bNeverDestroy && ( fDamage > 0.f ) )
	{
		m_fHitPoints	= 0.0f;
		m_fDeathDamage	= fDamage;
		m_eDeathType	= eType;
		m_vDeathDir		= vDir;

		HandleDestruction(hDamager);

		if (m_bIsPlayer)
		{
			ProcessPlayerDeath(hDamager);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::SetLastDamager
//
//	PURPOSE:	Save our last damager
//
// ----------------------------------------------------------------------- //

void CDestructible::SetLastDamager(HOBJECT hDamager)
{
	// Save info on the guy who hit us...
	m_hLastDamager = hDamager;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ProcessPlayerDeath
//
//	PURPOSE:	Process the death of a player
//
// ----------------------------------------------------------------------- //

void CDestructible::ProcessPlayerDeath(HOBJECT hDamager)
{
	// RETAIL: Tron's (retail 0x100e8e70): only a player killed by another
	// player in a disc game type scores, a frag to the killer (one off for
	// a team mate), and the message has the death type, not NOLF2's flag

	if (!m_bIsPlayer) return;

	// Stop player movement if dead...

    LTVector vZero;
	vZero.Init();
    g_pPhysicsLT->SetVelocity(m_hObject, &vZero);

    CPlayerObj* pVictim = (CPlayerObj*)g_pLTServer->HandleToObject( m_hObject );
	if (!pVictim) return;

	pVictim->GetMissionStats()->dwNumTimesKilled++;

	// If another player killed us, give them the score.
    CPlayerObj* pPlayerKiller = IsPlayer( hDamager ) ? (CPlayerObj*)g_pLTServer->HandleToObject( hDamager ) : LTNULL;
	if( IsDiscGameType( ) && pPlayerKiller && pVictim != pPlayerKiller )
	{
		bool bTeamKill = false;

		uint32 nKillerID = g_pLTServer->GetClientID( pPlayerKiller->GetClient( ));
		uint32 nVictimID = g_pLTServer->GetClientID( pVictim->GetClient( ));

		int nKillerTeam = g_pTronTeamMgr->GetTeamOfClient( nKillerID );
		int nVictimTeam = g_pTronTeamMgr->GetTeamOfClient( nVictimID );

		if( nKillerTeam == nVictimTeam && nKillerTeam < TRON_UNASSIGNED_TEAM )
			bTeamKill = true;

		uint32 nKillerUnknown18 = pPlayerKiller->GetPlayerScore()->GetTronUnknown18();
		uint32 nVictimUnknown18 = pVictim->GetPlayerScore()->GetTronUnknown18();
		if( nKillerUnknown18 == 0 && nVictimUnknown18 == 0 )
		{
			// A team mate's kill costs a frag
			if( bTeamKill )
				pPlayerKiller->GetPlayerScore()->AddFrags( -1 );
			else
				pPlayerKiller->GetPlayerScore()->AddFrags( 1 );

			pPlayerKiller->GetPlayerScore()->UpdateClients();
		}
	}

	if (g_pGameServerShell)
	{
		g_pGameServerShell->SetUpdateGameServ();
	}

	HCLIENT hClientVictim = pVictim->GetClient();
	HCLIENT hClientKiller = ( pPlayerKiller ) ? pPlayerKiller->GetClient() : NULL;
	if (hClientVictim )
	{
		uint32 nVictimID = g_pLTServer->GetClientID (hClientVictim);
		uint32 nKillerID = ( hClientKiller ) ? g_pLTServer->GetClientID (hClientKiller) : -1;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_SCORED);
		cMsg.Writeuint32(nVictimID);
		cMsg.Writeuint32(nKillerID);
		cMsg.Writeuint8(GetDeathType());
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ProcessGear
//
//	PURPOSE:	Adjust damage according to what gear items we have
//
// ----------------------------------------------------------------------- //

LTFLOAT CDestructible::ProcessGear(LTFLOAT fDamage, DamageType eDamageType, HOBJECT hDamager, LTVector* pvDir)
{
	for (int nGearId=0; nGearId < MAX_GEAR_ITEMS; nGearId++)
	{
		if (m_bGearOwned[nGearId])
		{
			GEAR const *pGear = g_pWeaponMgr->GetGear(nGearId);
			if (pGear && eDamageType == pGear->eProtectionType)
			{
				fDamage *= (1.0f - pGear->fProtection);
			}
		}
	}

	return fDamage;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleDestruction
//
//	PURPOSE:	Handle destruction
//
// ----------------------------------------------------------------------- //

// RETAIL: Object.lto 0x100e9150 takes a third argument and stores it at +0x30.
void CDestructible::HandleDestruction(HOBJECT hDamager, DamageType eDamageType, LTBOOL bTronUnknown30)
{
	m_eLastDamageType = eDamageType;
	m_bTronUnknown30 = bTronUnknown30;
	HandleDestruction( hDamager );
}

void CDestructible::HandleDestruction(HOBJECT hDamager)
{
	if (m_bDead) return;

    m_bDead      = LTTRUE;
	m_fHitPoints = 0.0f;	// RETAIL: Tron sets it (0x100e9187)

    LPBASECLASS pD = g_pLTServer->HandleToObject(m_hObject);

	if (m_sDeathCommand.length())
	{
		const char *pCmd = m_sDeathCommand.c_str();

		if( g_pCmdMgr->IsValidCmd( pCmd ) )
		{
			g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
		}
	}

	if (m_sPlayerDeathCommand.length() && IsPlayer(hDamager))
	{
		const char *pCmd = m_sPlayerDeathCommand.c_str();

		if( g_pCmdMgr->IsValidCmd( pCmd ) )
		{
			g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
		}
	}

	// Send message to object that killed us...

	if(hDamager && m_sKillerMessage.length())
	{
		SendTriggerMsgToObject(pD, hDamager, LTFALSE, m_sKillerMessage.c_str());
	}

	if (IsPlayer(hDamager) && IsCharacter(m_hObject) && !m_bIsPlayer)
	{
        CCharacter* pChar = (CCharacter*) g_pLTServer->HandleToObject(m_hObject);
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(hDamager);
		if (pPlayer && pChar)
		{
			//TODO: verify alignment
			pPlayer->GetMissionStats()->dwNumEnemyKills++;
		}
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleHeal
//
//	PURPOSE:	Handle heal message.
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleHeal(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	if (m_bDead) return;

    LTFLOAT fAmount = pMsg->Readfloat() * g_HealScale.GetFloat(1.0f);

	// See if we can heal

 	LTBOOL bHealed = Heal(fAmount);

	CAutoMessage cMsg;
	cMsg.Writeuint32(MID_PICKEDUP);
	cMsg.Writeuint8((uint8)bHealed);
	g_pLTServer->SendToObject(cMsg.Read(), pObject->m_hObject, hSender, MESSAGE_GUARANTEED);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleAddGear
//
//	PURPOSE:	Handle add gear message.
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleAddGear(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	if (m_bDead) return;

    uint8 nGearId = pMsg->Readuint8();

 	LTBOOL bAdded = AddGear(nGearId);

	CAutoMessage cMsg;
	cMsg.Writeuint32(MID_PICKEDUP);
	cMsg.Writeuint8((uint8)bAdded);
	g_pLTServer->SendToObject(cMsg.Read(), pObject->m_hObject, hSender, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::AddGear
//
//	PURPOSE:	Add the specified gear to the owned list
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::AddGear(uint8 nGearId)
{
	// This may change after the new item is added...

    LTFLOAT fOldStealth = GetStealthModifier();

    LTBOOL bGearAdded = LTFALSE;
	// RETAIL: Tron's tests fall through to the stealth test (0x100e9550).
	CPlayerObj* pPlayer;
	if( m_bIsPlayer && nGearId != WMGR_INVALID_ID &&
		(pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hObject)) != LTNULL )
	{
			if (nGearId >= 0 && nGearId < MAX_GEAR_ITEMS)
			{
				if (!m_bGearOwned[nGearId])
				{
					GEAR const *pGear = g_pWeaponMgr->GetGear(nGearId);
					// RETAIL: Tron has no bServerRestricted test, and it tests
					// pGear only for bExclusive (0x100e95c2; docs/RETAIL_BUGS.md).
					if ( pGear && pGear->bExclusive)
					{
						m_bGearOwned[nGearId] = LTTRUE;
					}

#ifdef TRON20SRC_MODERN
					// GAME-81: no gear record, nothing added
					if (!pGear)
					{
						bGearAdded = LTFALSE;
					}
					else
#endif
					if (pGear->fArmor)
					{
						LTFLOAT fRepair = pGear->fArmor;
						// RETAIL: Tron reads the skill tables directly (0x100e95fc).
						fRepair *= g_pSkillsButeMgr->GetArmor(pPlayer->GetPlayerSkills()->GetLevel(SKL_ARMOR))->fModifier[ArmorModifiers::eArmorPickup];
						bGearAdded = Repair(fRepair);
					}
					else if (pGear->fHealth)
					{
						LTFLOAT fHealth = pGear->fHealth;
						fHealth *= g_pSkillsButeMgr->GetStamina(pPlayer->GetPlayerSkills()->GetLevel(SKL_STAMINA))->fModifier[StaminaModifiers::eHealthPickup];
						bGearAdded = Heal(fHealth);
					}
					else
					{
						bGearAdded = LTTRUE;
					}
				}
			}
			else
			{
				g_pLTServer->CPrint("Gear id out of range! : %d", nGearId);
			}
	}

	// If our stealth modifier has changed, notify the clients...

	if (fOldStealth != GetStealthModifier())
	{
		if (IsCharacter(m_hObject))
		{
            CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(m_hObject);
			if (pChar)
			{
				pChar->SendStealthToClients();
			}
		}
	}

	return bGearAdded;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Heal
//
//	PURPOSE:	Add some value to hit points
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::Heal(LTFLOAT fAmount)
{
    LTFLOAT fMax = GetMaxHitPoints();
    if (m_fHitPoints >= fMax) return LTFALSE;

	// only heal what we need to (cap hit points at maximum)

	if (m_fHitPoints + fAmount > fMax)
	{
		fAmount = fMax - m_fHitPoints;
	}

	// now actually heal the object

	DoActualHealing (fAmount);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::DoActualHealing()
//
//	PURPOSE:	Simply adds a value to the hit points variable
//
// ----------------------------------------------------------------------- //

void CDestructible::DoActualHealing(LTFLOAT fAmount)
{
	// NOTE: This function should only be called directly from the ultra heal
	//		 powerup code, as it does not do any bounds checking on the hit points.
	//       All other healing should be done through the HandleHeal() function above.

	if(!m_bCanHeal) return;

    m_bDead = LTFALSE;

	m_fHitPoints += fAmount;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleRepair()
//
//	PURPOSE:	Handle Repair message
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleRepair(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
    LTFLOAT fAmount = pMsg->Readfloat();

 	LTBOOL bRepaired = Repair(fAmount);

	CAutoMessage cMsg;
	cMsg.Writeuint32(MID_PICKEDUP);
	cMsg.Writeuint8((uint8)bRepaired);
	g_pLTServer->SendToObject(cMsg.Read(), pObject->m_hObject, hSender, MESSAGE_GUARANTEED);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Repair()
//
//	PURPOSE:	Add some value to armor
//
// ----------------------------------------------------------------------- //
LTBOOL CDestructible::Repair(LTFLOAT fAmount)
{
    if (!m_bCanRepair) return LTFALSE;

    LTFLOAT fMax = GetMaxArmorPoints();
    if (m_fArmorPoints >= fMax) return LTFALSE;

	m_fArmorPoints += fAmount;

	if (m_fArmorPoints > fMax)
	{
		m_fArmorPoints = fMax;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Reset
//
//	PURPOSE:	Reset
//
// ----------------------------------------------------------------------- //

void CDestructible::Reset(LTFLOAT fHitPts, LTFLOAT fArmorPts, LTFLOAT fEnergy)
{
    LTFLOAT fMaxHitPoints = GetMaxHitPoints();
    LTFLOAT fMaxArmorPoints = GetMaxArmorPoints();
	LTFLOAT fMaxEnergy = GetMaxEnergy();

	m_fHitPoints	= fHitPts <= fMaxHitPoints ? fHitPts : fMaxHitPoints;
	m_fArmorPoints  = fArmorPts <= fMaxArmorPoints ? fArmorPts : fMaxArmorPoints;
	m_fEnergy		= fEnergy <= fMaxEnergy ? fEnergy : fMaxEnergy;
	m_eDeathType	= DT_UNSPECIFIED;
	m_fDeathDamage	= 0.0f;
    m_bDead         = LTFALSE;

	m_eLastDamageType = DT_UNSPECIFIED;
	m_bTronUnknown30 = LTFALSE;	// RETAIL: Object.lto 0x100e9930
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleTrigger
//
//	PURPOSE:	Handle trigger message.
//
// ----------------------------------------------------------------------- //

// RETAIL: Object.lto 0x100e9940.  Tron parses the message and adds
// "DAMAGESCALAR <float>" and "MINHITPOINTS <float>".
void CDestructible::HandleTrigger(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	if (m_bDead) return;

	const char* szMsg = (const char*)pMsg->Readuint32();
	if( !szMsg )
		return;

	ConParse parse;
	parse.Init( szMsg );

	if( g_pCommonLT->Parse( &parse ) != LT_OK )
		return;

	CParsedMsg cMsg( parse.m_nArgs, parse.m_Args );

	static CParsedMsg::CToken s_cTok_Destroy( "DESTROY" );
	static CParsedMsg::CToken s_cTok_Reset( "RESET" );
	static CParsedMsg::CToken s_cTok_DamageScalar( "DAMAGESCALAR" );
	static CParsedMsg::CToken s_cTok_MinHitPoints( "MINHITPOINTS" );

	// See if we should destroy ourself...

	if( cMsg.GetArg(0) == s_cTok_Destroy )
	{
		DamageStruct damage;

		damage.fDamage	= damage.kInfiniteDamage;
		damage.hDamager = hSender;

		damage.DoDamage(pObject, pObject->m_hObject);
	}

	// See if we should reset ourself...

	if( cMsg.GetArg(0) == s_cTok_Reset )
	{
		Reset(GetMaxHitPoints(), GetMaxArmorPoints(), GetMaxEnergy());
	}

	if( cMsg.GetArg(0) == s_cTok_DamageScalar )
	{
		if( cMsg.GetArgCount() < 2 )
		{
			g_pLTServer->CPrint( "DamageScalar: DamageScalar <float>" );
		}
		else
		{
			m_fTronUnknown78 = (LTFLOAT)atof( cMsg.GetArg(1) );
		}
	}

	if( cMsg.GetArg(0) == s_cTok_MinHitPoints )
	{
		m_fTronUnknown7C = (LTFLOAT)atof( cMsg.GetArg(1) );
		if( m_fTronUnknown7C < 0.01f )
		{
			m_fTronUnknown7C = 0.0f;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleCrush
//
//	PURPOSE:	Handle crush message
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleCrush(LPBASECLASS pObject, HOBJECT hSender)
{
	if (!pObject || !hSender || m_bDead) return;

	// Don't do crush damage...

	return;


    LTFLOAT fMyMass, fHisMass;
    g_pPhysicsLT->GetMass(m_hObject, &fMyMass);
    g_pPhysicsLT->GetMass(hSender, &fHisMass);

	// If we're within 15% of each other, don't do anything rash...

	if (fHisMass <= fMyMass*1.15) return;

    uint32 dwUsrFlg;
	g_pCommonLT->GetObjectFlags(hSender, OFT_User, dwUsrFlg);
	if (dwUsrFlg & USRFLG_CANT_CRUSH) return;

    LTFLOAT fDamage = 20.0f;

	DamageStruct damage;

	damage.eType	= DT_CRUSH;
	damage.fDamage	= fDamage;
	damage.hDamager = hSender;

	damage.DoDamage(pObject, pObject->m_hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleTouch
//
//	PURPOSE:	Handle object contact
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleTouch(LPBASECLASS pObject, HOBJECT hSender, LTFLOAT fForce)
{
	if (!pObject || !hSender || m_bDead || fForce < 0.0f) return;

	// Don't do touch damage...

	return;


    LTFLOAT fMyMass, fHisMass;
    g_pPhysicsLT->GetMass(pObject->m_hObject, &fMyMass);
    g_pPhysicsLT->GetMass(hSender, &fHisMass);

    LTBOOL bIsWorld = IsMainWorld(hSender);

	if (bIsWorld)
	{
		fHisMass = INFINITE_MASS;

		// Don't allow the world to damage the player...

		if (IsPlayer(pObject->m_hObject)) return;
	}

    uint32 dwUsrFlg;
	g_pCommonLT->GetObjectFlags(hSender, OFT_User, dwUsrFlg);
	if (dwUsrFlg & USRFLG_CANT_CRUSH) return;

	if (fHisMass <= 0.1f || fMyMass <= 0.1f) return;

	// If we're within 15% of each other, don't do anything rash...

	if (fHisMass <= fMyMass*1.15 || fMyMass > fHisMass) return;


	// Not enough force to do anything...

    LTFLOAT fVal = bIsWorld ? fMyMass * 15.0f : fMyMass * 10.0f;
	if (fForce < fVal) return;


	// Okay, one last check.  Make sure that he is moving, if I run into him
	// that shouldn't cause anything to happen.  It is only when he runs into
	// me that I can take damage...(well, unless it is the world...

	if (!bIsWorld)
	{
        LTVector vVel;
		g_pPhysicsLT->GetVelocity(hSender, &vVel);
		if (VEC_MAG(vVel) < 1.0f) return;
	}


	// Calculate damage...

    LTFLOAT fDamage = (fForce / fMyMass);
	fDamage *= bIsWorld ? 5.0f : 10.0f;

    //g_pLTServer->BPrint("%.2f, %.2f, %.2f, %.2f",
	//				  fHisMass, fMyMass, fForce, fDamage);

    LTVector vDir;
	VEC_INIT(vDir);

	if (bIsWorld)
	{
        g_pPhysicsLT->GetVelocity(pObject->m_hObject, &vDir);
		VEC_NEGATE(vDir, vDir);
	}
	else
	{
        LTVector vMyPos, vHisPos;
		g_pLTServer->GetObjectPos(pObject->m_hObject, &vMyPos);
        g_pLTServer->GetObjectPos(hSender, &vHisPos);

		VEC_SUB(vDir, vMyPos, vHisPos);
	}

	vDir.Normalize();

	DamageStruct damage;

	damage.eType	= DT_CRUSH;
	damage.fDamage	= fDamage;
	damage.hDamager = hSender;
	damage.vDir		= vDir;

	damage.DoDamage(pObject, pObject->m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::DebugDamageOn
//
//	PURPOSE:	See if the object can be damaged
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::DebugDamageOn()
{
    if (!m_hObject) return LTFALSE;

    LTBOOL bRet = LTFALSE;

    HCONVAR hVar  = g_pLTServer->GetGameConVar("SetDamage");
    const char* pVal = g_pLTServer->GetVarValueString(hVar);

    if (!pVal) return LTFALSE;

    return ((LTBOOL) !atoi(pVal));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::DebugDamageOn
//
//	PURPOSE:	See if the object can be damaged
//
// ----------------------------------------------------------------------- //

float CDestructible::AdjustDamage( float fDamage, HOBJECT hDamager )
{
	// Only do following adjustments for characters damaging characters...

	if( !IsCharacter( m_hObject ) || !IsCharacter( hDamager ) )
	{
		return fDamage;
	}

	// AI damaging a player...

	if( IsAI( hDamager ) && IsPlayer( m_hObject ))
	{

		float fDifficultyFactor = 1.0f;
    		
		// If we're an AI damage is based on the current difficutly setting...

		switch (g_pGameServerShell->GetDifficulty())
		{
			case GD_EASY:
				fDifficultyFactor = g_vtAIDamageAdjustEasy.GetFloat();
			break;

			case GD_NORMAL:
				fDifficultyFactor = g_vtAIDamageAdjustNormal.GetFloat();
			break;

			case GD_HARD:
				fDifficultyFactor = g_vtAIDamageAdjustHard.GetFloat();
			break;

			case GD_VERYHARD:
				fDifficultyFactor = g_vtAIDamageAdjustVeryHard.GetFloat();
			break;

			default :
			break;
		}

		// Adjust for the number of players...
		
		float fPlayerMod = 0.0f;
		uint32 nPlayersInGame = CPlayerObj::GetNumberPlayersWithClients( );

		if( nPlayersInGame > 1 )
		{
			// Increase the difficulty by an amount per player, factoring out the first player...
			
			float fPlayerInc = g_vtAIDamageIncreasePerPlayer.GetFloat();
			fPlayerMod = fPlayerInc * (nPlayersInGame-1);
		}

		return fDamage * (fDifficultyFactor + fPlayerMod);
	}

	// Player damaging an AI

	if( IsPlayer( hDamager ) && IsAI( m_hObject ) )
	{
		// Leave the damage the way it was if in SP game...

		if( IsMultiplayerGame() )
		{
			uint32 nPlayersInGame = CPlayerObj::GetNumberPlayersWithClients( );
			if( nPlayersInGame > 1 )
			{
				float fDifficultyFactor = 1.0f;
				float fPlayerInc = g_vtPlayerDamageDecreasePerPlayer.GetFloat();
				float fPlayerMod = fPlayerInc * (nPlayersInGame-1);

				return fDamage * (fDifficultyFactor - fPlayerMod);
			}
		}
	}

	return fDamage;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CDestructible::Save(ILTMessage_Write *pMsg, uint8 nType)
{
	if (!pMsg) return;

    SAVE_HOBJECT(m_hObject);
    SAVE_HOBJECT(m_hLastDamager);

	SAVE_FLOAT(m_fHitPoints);
    SAVE_FLOAT(m_fMaxHitPoints);
	SAVE_FLOAT(m_fEnergy);
    SAVE_FLOAT(m_fMaxEnergy);
    SAVE_FLOAT(m_fArmorPoints);
    SAVE_FLOAT(m_fMaxArmorPoints);
    SAVE_FLOAT(m_fMass);
    SAVE_FLOAT(m_fLastDamage);
    SAVE_FLOAT(m_fLastArmorAbsorb);
    SAVE_FLOAT(m_fDeathDamage);
    SAVE_BOOL(m_bDead);
    SAVE_BOOL(m_bApplyDamagePhysics);
    SAVE_BOOL(m_bIsPlayer);
    SAVE_BOOL(m_bCanHeal);
    SAVE_BOOL(m_bCanRepair);
    SAVE_BOOL(m_bCanDamage);
    SAVE_BOOL(m_bNeverDestroy);
    SAVE_BYTE(m_eDeathType);
    SAVE_BYTE(m_eLastDamageType);
    SAVE_BOOL(m_bTronUnknown30);	// RETAIL: Tron's, here (retail 0x100e9fa0)
    SAVE_DWORD(m_nDamageTriggerCounter);
    SAVE_DWORD(m_nDamageTriggerNumSends);
    SAVE_QWORD(m_nCantDamageFlags);
    SAVE_BOOL(m_bTronUnknown6D0);
    SAVE_FLOAT(m_fTronUnknown6D4);
    SAVE_FLOAT(m_fTronUnknown6D8);
    SAVE_VECTOR(m_vTronUnknown6DC);
	
	SAVE_CHARSTRING( m_sDamageCommand.c_str() );
    SAVE_CHARSTRING( m_sDamagerMessage.c_str() );
    SAVE_CHARSTRING( m_sDeathCommand.c_str() );
    SAVE_CHARSTRING( m_sPlayerDeathCommand.c_str() );
    SAVE_CHARSTRING( m_sKillerMessage.c_str() );
    SAVE_CHARSTRING( m_sDamagePercentCommand.c_str() );

	SAVE_CHARSTRING( m_sPlayerDamageCommand.c_str( ));
 
	SAVE_VECTOR(m_vDeathDir);
    SAVE_VECTOR(m_vLastDamageDir);
	SAVE_FLOAT( m_fDamagePercentCommand );
	SAVE_FLOAT( m_fTronUnknown78 );
	SAVE_FLOAT( m_fTronUnknown7C );

    int i;
    for (i=0; i < MAX_GEAR_ITEMS; i++)
	{
        SAVE_BOOL(m_bGearOwned[i]);
	}

	for (i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		m_ProgressiveDamage[i].Save(pMsg);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CDestructible::Load(ILTMessage_Read *pMsg, uint8 nType)
{
	if (!pMsg) return;

    LOAD_HOBJECT(m_hObject);
    LOAD_HOBJECT(m_hLastDamager);

    LOAD_FLOAT(m_fHitPoints);
    LOAD_FLOAT(m_fMaxHitPoints);
	LOAD_FLOAT(m_fEnergy);
    LOAD_FLOAT(m_fMaxEnergy);
    LOAD_FLOAT(m_fArmorPoints);
    LOAD_FLOAT(m_fMaxArmorPoints);
    LOAD_FLOAT(m_fMass);
    LOAD_FLOAT(m_fLastDamage);
    LOAD_FLOAT(m_fLastArmorAbsorb);
    LOAD_FLOAT(m_fDeathDamage);
    LOAD_BOOL(m_bDead);
    LOAD_BOOL(m_bApplyDamagePhysics);
    LOAD_BOOL(m_bIsPlayer);
    LOAD_BOOL(m_bCanHeal);
    LOAD_BOOL(m_bCanRepair);
    LOAD_BOOL(m_bCanDamage);
    LOAD_BOOL(m_bNeverDestroy);
    LOAD_BYTE_CAST(m_eDeathType, DamageType);
    LOAD_BYTE_CAST(m_eLastDamageType, DamageType);
    LOAD_BOOL(m_bTronUnknown30);	// RETAIL: as Save
    LOAD_DWORD(m_nDamageTriggerCounter);
    LOAD_DWORD(m_nDamageTriggerNumSends);
    LOAD_QWORD(m_nCantDamageFlags);
    LOAD_BOOL(m_bTronUnknown6D0);
    LOAD_FLOAT(m_fTronUnknown6D4);
    LOAD_FLOAT(m_fTronUnknown6D8);
    LOAD_VECTOR(m_vTronUnknown6DC);

	char szString[1024] = {0};
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sDamageCommand = szString;
    LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sDamagerMessage = szString;
    LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sDeathCommand = szString;
    LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sPlayerDeathCommand = szString;
    LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sKillerMessage = szString;
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sDamagePercentCommand = szString;
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sPlayerDamageCommand = szString;
	
	LOAD_VECTOR(m_vDeathDir);
    LOAD_VECTOR(m_vLastDamageDir);
 	LOAD_FLOAT( m_fDamagePercentCommand );
	LOAD_FLOAT( m_fTronUnknown78 );
	LOAD_FLOAT( m_fTronUnknown7C );
	
    int i;
    for (i=0; i < MAX_GEAR_ITEMS; i++)
	{
        LOAD_BOOL(m_bGearOwned[i]);
	}

	for (i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		m_ProgressiveDamage[i].Load(pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDestructiblePlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CDestructiblePlugin::PreHook_PropChanged( const char *szObjName,
													   const char *szPropName, 
													   const int  nPropType, 
													   const GenericProp &gpPropValue,
													   ILTPreInterface *pInterface,
													   const char *szModifiers)
{
	// Check if the props are our commands and then just send it to the CommandMgr..

	if( !_stricmp( "DamageCommand", szPropName ) ||
		!_stricmp( "DeathCommand", szPropName ) ||
		!_stricmp( "PlayerDamageCommand", szPropName ) ||
		!_stricmp( "PlayerDeathCommand", szPropName ) )
	{
		if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
													szPropName, 
													nPropType, 
													gpPropValue,
													pInterface,
													szModifiers ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}
