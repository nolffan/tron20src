// ----------------------------------------------------------------------- //
//
// MODULE  : AmmoBox.cpp
//
// PURPOSE : AmmoBox object implementation
//
// CREATED : 10/28/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AmmoBox.h"
#include "MsgIds.h"
#include "iltserver.h"
#include "PlayerObj.h"
#include "ObjectMsgs.h"

#define UPDATE_DELTA			0.1f
#define UNUSED_STRING			"<none>"
#define AMMOBOX_PICKUP_SOUND	"AmmoBoxPickup"
#define AMMOBOX_RESPAWN_SOUND	"AmmoBoxRespawn"

		
 
#define ADD_AMMO_PROP(num) \
		ADD_STRINGPROP_FLAG(AmmoType##num##, UNUSED_STRING, PF_STATICLIST)\
		ADD_LONGINTPROP(AmmoCount##num##, 0)

LINKFROM_MODULE( AmmoBox );


#pragma force_active on
BEGIN_CLASS(AmmoBox)
	ADD_STRINGPROP_FLAG(Filename, "Props\\Models\\PuAmmoBox.ltb", PF_DIMS | PF_FILENAME | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, "Props\\Skins\\PuAmmoBox.dtx", PF_FILENAME)
	ADD_BOOLPROP_FLAG(Rotate, 0, PF_HIDDEN)
	ADD_REALPROP(RespawnTime, 30.0f)
	ADD_STRINGPROP_FLAG(RespawnSound, AMMOBOX_RESPAWN_SOUND, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(PickupSound, AMMOBOX_PICKUP_SOUND, PF_HIDDEN)
	ADD_AMMO_PROP(1)
	ADD_AMMO_PROP(2)
	ADD_AMMO_PROP(3)
	ADD_AMMO_PROP(4)
	ADD_AMMO_PROP(5)
	ADD_AMMO_PROP(6)
	ADD_AMMO_PROP(7)
	ADD_AMMO_PROP(8)
	ADD_AMMO_PROP(9)
	ADD_AMMO_PROP(10)
END_CLASS_DEFAULT_FLAGS_PLUGIN(AmmoBox, PickupItem, NULL, NULL, 0, CAmmoBoxPlugin)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( AmmoBox )
CMDMGR_END_REGISTER_CLASS( AmmoBox, PickupItem )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::AmmoBox
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AmmoBox::AmmoBox() : PickupItem()
{
	for (int i = 0; i < AB_MAX_TYPES; i++)
	{
		m_nAmmoId[i] = m_nOriginalAmmoId[i] = WMGR_INVALID_ID;
		m_nAmmoCount[i]	= m_nOriginalAmmoCount[i] = 0;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 AmmoBox::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = PickupItem::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			return dwRet;
		}
		break;

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
//	ROUTINE:	AmmoBox::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //

void AmmoBox::ReadProp(ObjectCreateStruct *pStruct)
{
	// RETAIL: 0x101d2d70: Tron's is build 51's, with the ammo's spawned
	// amount when the count is 0
	GenericProp genProp;

	for (int i=0; i < AB_MAX_TYPES; i++)
	{
		char key[40];

		sprintf(key, "AmmoType%d", i+1);
        if (g_pLTServer->GetPropGeneric(key, &genProp) == LT_OK)
		{
			if (_stricmp(genProp.m_String, UNUSED_STRING) != 0)
			{
				AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(genProp.m_String);
				if (pAmmo)
				{
					m_nAmmoId[i] = m_nOriginalAmmoId[i] = pAmmo->nId;
				}
			}
		}

		sprintf(key, "AmmoCount%d", i+1);
        if (g_pLTServer->GetPropGeneric(key, &genProp) == LT_OK)
		{
			m_nAmmoCount[i] = m_nOriginalAmmoCount[i] = genProp.m_Long;
			if (m_nAmmoCount[i] == 0)
			{
				AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmoId[i]);
				if (pAmmo)
				{
					m_nAmmoCount[i] = m_nOriginalAmmoCount[i] = pAmmo->nSpawnedAmount;
				}
			}
		}
	}


	// Set up the appropriate pick up and respawn sounds...

	FREE_HSTRING(m_hstrSoundFile);
    m_hstrSoundFile = g_pLTServer->CreateString(AMMOBOX_PICKUP_SOUND);

	FREE_HSTRING(m_hstrRespawnSoundFile);
	m_hstrRespawnSoundFile = g_pLTServer->CreateString( AMMOBOX_RESPAWN_SOUND );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::ObjectTouch
//
//	PURPOSE:	Add weapon PickupItem to object
//
// ----------------------------------------------------------------------- //

void AmmoBox::ObjectTouch(HOBJECT hObject, bool bForcePickup/*=false*/)
{
	if (!hObject) return;

	// If we hit non-player objects, just ignore them...

	if (IsPlayer(hObject))
	{
        CCharacter* pCharObj = (CCharacter*)g_pLTServer->HandleToObject(hObject);

		if (pCharObj && !pCharObj->IsDead())
		{
			int nValidIds = 0;
			for (int i=0; i < AB_MAX_TYPES; i++)
			{
				if (m_nAmmoId[i] != WMGR_INVALID_ID && m_nAmmoCount[i] > 0)
				{
					nValidIds++;
				}
			}
			if (nValidIds)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint32(MID_AMMOBOX);
				cMsg.Writeuint8(nValidIds);
				for (int i=0; i < AB_MAX_TYPES; i++)
				{
					if (m_nAmmoId[i] != WMGR_INVALID_ID && m_nAmmoCount[i] > 0)
					{
						cMsg.Writeuint8(m_nAmmoId[i]);
						cMsg.Writeint32(m_nAmmoCount[i]);
					}
				}
				g_pLTServer->SendToObject(cMsg.Read(), m_hObject, hObject, MESSAGE_GUARANTEED);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void AmmoBox::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	for (int i=0; i < AB_MAX_TYPES; i++)
	{
		pMsg->Writeuint8(m_nAmmoId[i]);
		pMsg->Writeint32(m_nAmmoCount[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void AmmoBox::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	for (int i=0; i < AB_MAX_TYPES; i++)
	{
        m_nAmmoId[i] = pMsg->Readuint8();
        m_nAmmoCount[i] = (int)pMsg->Readint32();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 AmmoBox::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_AMMOBOX:
		{
			//figure out what's left
			Leftovers(pMsg);
		}
		break;

		default: break;
	}

	return PickupItem::ObjectMessageFn(hSender, pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::Leftovers
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

void AmmoBox::Leftovers(ILTMessage_Read *pMsg)
{
    uint8 numAmmoTypes = pMsg->Readuint8();

    int i;
    for (i = 0; i < numAmmoTypes; i++)
	{
        m_nAmmoId[i]    = pMsg->Readuint8();
        m_nAmmoCount[i] = (int) pMsg->Readint32();

		if (m_nAmmoCount[i] <= 0)
			m_nAmmoId[i] = WMGR_INVALID_ID;

	}

	for (i=numAmmoTypes; i < AB_MAX_TYPES; i++)
	{
		m_nAmmoId[i] = WMGR_INVALID_ID;
		m_nAmmoCount[i] = 0;
	}

	PlayPickedupSound();
	
	// If in a deathmatch game we should act like the ammobox was empty and pick it up
	// if anything from the box was taken.
	
	if( !IsCoopMultiplayerGameType() )
	{
		PickedUp( LTNULL );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAmmoBoxPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //
#ifndef __PSX2

LTRESULT CAmmoBoxPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( CWeaponMgrPlugin::PreHook_EditStringList(szRezPath, szPropName,
		aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK )
	{
		return LT_OK;
	}


	// See if we can handle the property...

	char key[40];
	for (int index=1; index <= AB_MAX_TYPES; index++)
	{
		sprintf(key, "AmmoType%d", index);

        if (_stricmp(key, szPropName) == 0)
		{
            uint32 dwAmmoTypes = g_pWeaponMgr->GetNumAmmoIds();

			_ASSERT(cMaxStrings >= dwAmmoTypes);
			*pcStrings = 1;

			// Make sure the first string is the unused slot...

			strcpy(aszStrings[0], UNUSED_STRING);

            AMMO const *pAmmo = LTNULL;

            for (uint32 i=1; i <= dwAmmoTypes; i++)
			{
				pAmmo = g_pWeaponMgr->GetAmmo(i-1);

				if (pAmmo && pAmmo->szName[0] && strlen(pAmmo->szName) < cMaxStringLength)
				{
					strcpy(aszStrings[i], pAmmo->szName);
					(*pcStrings)++;
				}
				else
				{
					return LT_UNSUPPORTED;
				}
			}

			return LT_OK;
		}
	}
	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAmmoBoxPlugin::PreHook_EditStringList
//
//	PURPOSE:	Check the changed prop value
//
// ----------------------------------------------------------------------- //

LTRESULT CAmmoBoxPlugin::PreHook_PropChanged( const char *szObjName,
											  const char *szPropName,
											  const int nPropType,
											  const GenericProp &gpPropValue,
											  ILTPreInterface *pInterface,
											  const char *szModifiers )
{
	// Just pass it to the PickUpPlugin...
	
	if( m_PickupItemPlugin.PreHook_PropChanged( szObjName,
												szPropName,
												nPropType,
												gpPropValue,
												pInterface,
												szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
#endif  // !__PSX2
