// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponItems.cpp
//
// PURPOSE : Weapon items - Implementation
//
// CREATED : 10/7/97
//
// REVISED : 10/22/99 - jrg
//			 07/12/02 - kls
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponItems.h"
#include "MsgIds.h"
#include "iltserver.h"
#include "Character.h"
#include "ObjectMsgs.h"
#include "CVarTrack.h"
#include "GameServerShell.h"

LINKFROM_MODULE( WeaponItems );

extern CGameServerShell* g_pGameServerShell;

#define UPDATE_DELTA				0.1f
#define WEAPONITEM_PICKUP_SOUND		"WeaponItemPickup"
#define WEAPONITEM_RESPAWN_SOUND	"WeaponItemRespawn"

static const char s_szDefaultRS[] = "RS\\Default.ltb";

#pragma force_active on
BEGIN_CLASS(WeaponItem)
	// Hidden property overrides...
	ADD_STRINGPROP_FLAG(Filename, "", PF_FILENAME | PF_HIDDEN | PF_MODEL )
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Rotate, 0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(RespawnSound, WEAPONITEM_RESPAWN_SOUND, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(PickupSound, WEAPONITEM_PICKUP_SOUND, PF_HIDDEN)

	ADD_REALPROP(RespawnTime, 30.0f)
	ADD_STRINGPROP_FLAG(WeaponType, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS )
	ADD_LONGINTPROP_FLAG(WeaponTypeId, WMGR_INVALID_ID, PF_HIDDEN) // Only used by SpawnObject
	ADD_LONGINTPROP(AmmoAmount, -1)
END_CLASS_DEFAULT_FLAGS_PLUGIN(WeaponItem, PickupItem, NULL, NULL, 0, CWeaponItemPlugin)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( WeaponItem )
CMDMGR_END_REGISTER_CLASS( WeaponItem, PickupItem )


extern CVarTrack g_RespawnScaleTrack;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::WeaponItem
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

WeaponItem::WeaponItem() : PickupItem()
{
	m_nAmmo		= -1;
	m_nWeaponId = WMGR_INVALID_ID;
	m_nAmmoId	= WMGR_INVALID_ID;

    m_bBounce = LTFALSE;

	m_dwFlags |= FLAG_NOSLIDING;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 WeaponItem::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
		    uint32 dwRet = PickupItem::EngineMessageFn(messageID, pData, fData);
            SetNextUpdate(UPDATE_NEVER);

			return dwRet;
		}
		break;

		case MID_PRECREATE:
		{
            uint32 dwRet = PickupItem::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			if (fData != PRECREATE_SAVEGAME)
			{
				PostPropRead((ObjectCreateStruct*)pData);
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
            uint32 dwRet = PickupItem::EngineMessageFn(messageID, pData, fData);

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			return dwRet;
		}

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return PickupItem::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //

void WeaponItem::ReadProp(ObjectCreateStruct *pStruct)
{
	// RETAIL: 0x101d1b20: Tron's is build 51's
	GenericProp genProp;

 	g_pWeaponMgr->ReadWeaponProp("WeaponType", m_nWeaponId, m_nAmmoId);

	// If the WeaponType is invalid, see if the WeaponTypeId property
	// was set correctly...

	if (m_nWeaponId == WMGR_INVALID_ID)
	{
		if (g_pLTServer->GetPropGeneric("WeaponTypeId", &genProp) == LT_OK)
		{
			m_nWeaponId = (uint8) genProp.m_Long;

			// Set ammo type to default (could be changed by AmmoType
			// property...
			WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(m_nWeaponId);
			if (pWeapon)
			{
				m_nAmmoId = pWeapon->nDefaultAmmoId;
			}
		}
	}

	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(m_nWeaponId);
	if (pWeapon && pWeapon->bInfiniteAmmo)
	{
		m_nAmmo = 1000;
	}
	else
	{
		if (g_pLTServer->GetPropGeneric("AmmoAmount", &genProp) == LT_OK)
		{
			m_nAmmo = genProp.m_Long;
		}
	}

	// This allows ammo type to be specified without appending it
	// to the WeaponType attribute (e.g., a spawned weapon)...

    if (g_pLTServer->GetPropGeneric("AmmoType", &genProp) == LT_OK)
	{
		AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(genProp.m_String);
		if (pAmmo)
		{
			m_nAmmoId = pAmmo->nId;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::PostPropRead
//
//	PURPOSE:	Handle post property read engine messages
//
// ----------------------------------------------------------------------- //

void WeaponItem::PostPropRead(ObjectCreateStruct *pStruct)
{
	// RETAIL: 0x101d1c60: Tron's is build 51's with the override model and
	// the hand held scale
	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(m_nWeaponId);
	if (!pWeapon) return;

	if (pStruct)
	{
		SAFE_STRCPY(pStruct->m_Filename, pWeapon->szHHModel);

		pWeapon->blrHHSkins.CopyList(0, pStruct->m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
		pWeapon->blrHHRenderStyles.CopyList(0, pStruct->m_RenderStyleNames[0], MAX_CS_FILENAME_LEN+1);

		// See if our default model was changed...

		CheckForOverrideModel(pStruct);

		// Set up the appropriate pick up and respawn sounds...

		FREE_HSTRING(m_hstrSoundFile);
		m_hstrSoundFile = g_pLTServer->CreateString( WEAPONITEM_PICKUP_SOUND );

		FREE_HSTRING( m_hstrRespawnSoundFile );
		m_hstrRespawnSoundFile = g_pLTServer->CreateString( WEAPONITEM_RESPAWN_SOUND );

		WEAPON const *pScaleWeapon = g_pWeaponMgr->GetWeapon(m_nWeaponId);
		if (pScaleWeapon)
		{
			m_vScale = pScaleWeapon->vHHScale;
		}

		m_bBounce = LTFALSE;
		m_bRotate = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::InitialUpdate
//
//	PURPOSE:	Handle inital update engine messages
//
// ----------------------------------------------------------------------- //

void WeaponItem::InitialUpdate()
{
	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmoId);
	if (!pAmmo) return;
    
	// Set our default ammo if necessary...

	if (m_nAmmo < 0)
	{
		m_nAmmo = pAmmo->nSpawnedAmount;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::ObjectTouch
//
//	PURPOSE:	Add weapon powerup to object
//
// ----------------------------------------------------------------------- //

void WeaponItem::ObjectTouch(HOBJECT hObject, bool bForcePickup/*=false*/)
{
	if (!hObject) return;

	// If we hit non-player objects, just ignore them...

	if (IsPlayer(hObject))
	{
        CCharacter* pCharObj = (CCharacter*)g_pLTServer->HandleToObject(hObject);

		if (pCharObj && !pCharObj->IsDead())
		{
			CAutoMessage cMsg;
			cMsg.Writeuint32(MID_ADDWEAPON);
			cMsg.Writeuint8(m_nWeaponId);
            cMsg.Writeuint8(m_nAmmoId);
            cMsg.Writeint32(m_nAmmo);
			cMsg.Writebool(bForcePickup);
			g_pLTServer->SendToObject(cMsg.Read(), m_hObject, hObject, MESSAGE_GUARANTEED);

			// RETAIL: Tron's player also acquires the weapon's primitive
			// and its subroutine (0x101d1f21)
			WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(m_nWeaponId);
			if (!pWeapon) return;

			char szMsg[256];
			sprintf(szMsg, "ACQUIREPRIM %s", pWeapon->szPrimitiveType);
			SendTriggerMsgToObject(this, pCharObj, LTFALSE, szMsg);

			if (pWeapon->szSubroutineNeeded[0])
			{
				sprintf(szMsg, "ACQUIRESUB %s", pWeapon->szSubroutineNeeded);
				SendTriggerMsgToObject(this, pCharObj, LTFALSE, szMsg);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::PickedUp
//
//	PURPOSE:	Called when an object tells this item that the object
//				picked it up...
//
// ----------------------------------------------------------------------- //

void WeaponItem::PickedUp( ILTMessage_Read *pMsg )
{
	PickupItem::PickedUp( pMsg );

	// RETAIL: Tron's WEAPON has no respawn-wait skins, render styles or FX
	// (PORT: NOLF2 switched to them here while the item waited to respawn)
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void WeaponItem::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

    SAVE_BYTE(m_nWeaponId);
    SAVE_BYTE(m_nAmmoId);
    SAVE_INT(m_nAmmo);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void WeaponItem::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

    LOAD_BYTE(m_nWeaponId);
    LOAD_BYTE(m_nAmmoId);
    LOAD_INT(m_nAmmo);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponItemPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //
#ifndef __PSX2
LTRESULT CWeaponItemPlugin::PreHook_EditStringList(const char* szRezPath, 
												   const char* szPropName, 
												   char** aszStrings, 
												   uint32* pcStrings, 
												   const uint32 cMaxStrings, 
												   const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	if (_strcmpi("WeaponType", szPropName) == 0)
	{
		m_WeaponMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		// RETAIL: Tron lists the weapons with a primitive type (Object.lto 0x101d20aa)
		m_WeaponMgrPlugin.PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength, WeaponHasPrimitiveType);

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponItemPlugin::PreHook_Dims
//
//	PURPOSE:	Determine the dims for this item
//
// ----------------------------------------------------------------------- //

LTRESULT CWeaponItemPlugin::PreHook_Dims(const char* szRezPath,
										 const char* szPropValue,
										 char* szModelFilenameBuf,
										 int nModelFilenameBufLen,
										 LTVector & vDims )
{

	if (!szModelFilenameBuf || nModelFilenameBufLen < 1 || !g_pWeaponMgr ) return LT_UNSUPPORTED;

	szModelFilenameBuf[0] = '\0';

	// Remove the , that is put into some weapon names.
	char szModifiedPropValue[256];
	SAFE_STRCPY( szModifiedPropValue, szPropValue );
	strtok( szModifiedPropValue, "," );

	WEAPON const* pWeapon = g_pWeaponMgr->GetWeapon(( char* )szModifiedPropValue);
	if( !pWeapon || !pWeapon->szHHModel[0] )
	{
		return LT_UNSUPPORTED;
	}

	strcpy( szModelFilenameBuf, pWeapon->szHHModel );

	// Need to convert the .ltb filename to one that DEdit understands...
	
	ConvertLTBFilename( szModelFilenameBuf );

	return LT_OK;
}



#endif