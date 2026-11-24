// ----------------------------------------------------------------------- //
//
// MODULE  : TronTargetMgr.cpp
//
// PURPOSE : Implementation of class to handle tracking whjat the player is aimed at.
//			 Derived from TargetMgr.cpp
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "clientres.h"
#include "TronTargetMgr.h"
#include "TronPlayerMgr.h"
#include "GameClientShell.h"
#include "CharacterFX.h"
#include "BodyFX.h"
#include "PickupItemFX.h"
#include "PlayerMgr.h"
#include "HUDMgr.h"
// RETAIL: its SLICE_SIZE and PRIM_SIZE are set up in this TU too (retail
// 0x100afbb0, 0x100afbd0)
#include "ScreenSubroutines.h"

#include "ActivateObjectFX.h"
#include "ClientInfoMgr.h"
#include "InterfaceMgr.h"

// RETAIL: TargetMgr.cpp's console variables (retail 0x102755a0 and
// 0x10275590), and Tron's long activation distance (0x1026ef88; the name
// is ours)
extern VarTrack	g_vtActivationDistance;
extern VarTrack	g_vtTargetDistance;
VarTrack		g_vtActivationDistanceLong;

// RETAIL: the user flags that Tron's Update tests (0x100aea8b, 0x100aed87,
// 0x100aeeb1). Their names in Tron's ClientServerShared.h are not known:
// the names are ours
#define USRFLG_TRON_ACTIVATE_LONG		(1<<4)	// with USRFLG_CAN_ACTIVATE: ActivationDistanceLong
#define USRFLG_TRON_SUBROUTINE_PROPS	(1<<10)	// ask for the subroutine's properties (9)
#define USRFLG_TRON_PERMISSION_ANY		((1<<1) | USRFLG_PSET_1 | USRFLG_PSET_2 | USRFLG_PSET_3 | USRFLG_PSET_4 | \
										 USRFLG_PSET_5 | USRFLG_PSET_6 | USRFLG_PSET_7 | USRFLG_PSET_8)

// RETAIL: the target properties that only Tron's Update asks for, beside
// MsgIDs.h's (HandleTargetPropertiesMessage reads them). The names are ours
#define TARGET_PROP_TRON_3				3	// a third need (string 3807)
#define TARGET_PROP_TRON_4				4	// a fourth need (string 3809)
#define TARGET_PROP_TRON_KIND			7	// a person's kind (strings 3259, 3261, 3262)
#define TARGET_PROP_TRON_SUBROUTINE		9	// a subroutine's details (strings 3288..3310)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTargetMgr::CTronTargetMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTronTargetMgr::CTronTargetMgr() : CTargetMgr()
{
	// RETAIL: 0x100ae410: only m_Props's constructor
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTargetMgr::~CTronTargetMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CTronTargetMgr::~CTronTargetMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTargetMgr::Update
//
//	PURPOSE:	See what the crosshair is pointed at this frame
//
// ----------------------------------------------------------------------- //

// RETAIL: 0x100ae5f0. Tron's own: the four transfers, the console variables
// (with a long activation distance), the target's hit box and properties,
// and queries to the server for the target's properties (answered in
// HandleTargetPropertiesMessage). The "innocent" and enemy reticles are
// flags (+0x876, +0x877) for the crosshair, not target strings.
void CTronTargetMgr::Update()
{
	if (m_hLockedTarget && m_hTarget == m_hLockedTarget)
	{
		// are we transferring energy?
		// RETAIL: one string for each transfer (0x100ae622), in this order
		if (g_pTronPlayerMgr->m_bTransferringEnergy)
		{
			// don't change anything.
			SetTargetStringID(IDS_TRANSFERRING_ENERGY);
			return;
		}
		if (g_pTronPlayerMgr->m_bTronUnknown1EA)
		{
			SetTargetStringID(3808);	// RETAIL: "Downloading Health"
			return;
		}
		if (g_pTronPlayerMgr->m_bTronUnknown1E9)
		{
			SetTargetStringID(3806);	// RETAIL: "Downloading Energy"
			return;
		}
		if (g_pTronPlayerMgr->m_bTronUnknown1EB)
		{
			SetTargetStringID(3810);	// RETAIL: "Taking a dump"
			return;
		}

		// RETAIL: no gadget disabling and no searching in Tron's
	}

	// RETAIL: the old target and whether its properties were kept
	// (0x100ae66f)
	bool bOldProps = m_Props.bTronUnknown424;
	HOBJECT hOldTarget = m_hTarget;

	//clear our targeting info
	m_hTarget = NULL;
	m_Props.bTronUnknown424 = false;
	m_fTargetRange = 10000.0f;
	m_szString[0] = 0;
	m_szDebugString[0] = 0;
	m_nString = 0;

	// RETAIL: the console variables, each time (0x100ae6a5), as
	// CTargetMgr::Update does
	if (!g_vtActivationDistance.IsInitted())
	{
		g_vtActivationDistance.Init(g_pLTClient, "ActivationDistance", LTNULL, 100.0f);
	}

	if (!g_vtActivationDistanceLong.IsInitted())
	{
		g_vtActivationDistanceLong.Init(g_pLTClient, "ActivationDistanceLong", LTNULL, 200.0f);
	}

	if (!g_vtTargetDistance.IsInitted())
	{
		g_vtTargetDistance.Init(g_pLTClient, "TargetDistance", LTNULL, 1000.0f);
	}
	m_fActivationDistance = g_vtActivationDistance.GetFloat();
	m_fTargetDistance = g_vtTargetDistance.GetFloat();

	m_ActivationData.Init();

	m_bEnergyRequired = false;
	m_nEnergyRequired = 0;
	m_bTronUnknown85D = false;
	m_nTronUnknown868 = 0;
	m_bTronUnknown85E = false;
	m_nTronUnknown86C = 0;
	m_bTronUnknown85F = false;
	m_nTronUnknown870 = 0;
	m_bPermissionRequired = false;
	m_nPermissionRequired = 0;
	m_bTronUnknown876 = false;
	m_bTronUnknown877 = false;

	m_bSearchTarget = false;
	m_bMoveTarget = false;
	m_bCanActivate = true;
	m_bTronUnknown860 = false;

	//see what we've looking at
	float fDistAway = 10000.0f;
	CheckForIntersect(fDistAway);

	// RETAIL: the target's properties go when the player manager no
	// longer wants them (0x100ae97a), when there is no target (but there
	// was one) and when the target is out of their range
	if (!g_pTronPlayerMgr->GetTronUnknown204())
		ClearTargetProperties();
	if (!m_hTarget) //nothing to see here
	{
		if (hOldTarget || bOldProps)
			ClearTargetProperties();
		return;
	}
	if (g_pTronPlayerMgr->GetTronUnknown20C() < fDistAway)
		ClearTargetProperties();
	m_fTargetRange = fDistAway;
	m_Props.nTronUnknown420 = 0;


	//if its a body's hitbox, check the body instead
	CBodyFX* pBody = g_pGameClientShell->GetSFXMgr()->GetBodyFromHitBox(m_hTarget);
	if (pBody)
	{
		m_hTarget = pBody->GetServerObj();
		m_Props.bTronUnknown424 = true;
		m_ActivationData.m_hTarget = m_hTarget;
		if (!m_hTarget) return;
	}

	//if its a Character's hitbox, check the Character instead
	CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFromHitBox(m_hTarget);
	if (pCharacter)
	{
		m_hTarget = pCharacter->GetServerObj();
		m_Props.bTronUnknown424 = true;
		m_ActivationData.m_hTarget = m_hTarget;
		if (!m_hTarget) return;
	}


	uint32 dwUserFlags = 0;
    g_pCommonLT->GetObjectFlags(m_hTarget, OFT_User, dwUserFlags);

	// RETAIL: an object that can be activated from further away (0x100aea8b;
	// the flag has no name in Tron's ClientServerShared.h that we know)
	if ((dwUserFlags & USRFLG_CAN_ACTIVATE) && (dwUserFlags & USRFLG_TRON_ACTIVATE_LONG))
	{
		m_fActivationDistance = g_vtActivationDistanceLong.GetFloat();
	}

	//are we aiming at a person?
	if (dwUserFlags & USRFLG_CHARACTER)
	{
		CCharacterFX* const pFX = (CCharacterFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_CHARACTER_ID, m_hTarget);

		//display debug info if we have any 
		if( pFX && pFX->GetInfoString() && *pFX->GetInfoString() )
		{
			SAFE_STRCPY(m_szDebugString,pFX->GetInfoString());			
		}
		else
		{
			m_szDebugString[0] = NULL;
		}

		// is this a person we can talk to?
		// RETAIL: the server tells what kind of person it is (property 7,
		// 0x100aeb4c); the string stays "talk" until it answers
		if ((dwUserFlags & USRFLG_CAN_ACTIVATE) && (fDistAway <= m_fActivationDistance) &&
			!(dwUserFlags & USRFLG_REQUIRES_ENERGY))
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_OBJECT_MESSAGE);
			cMsg.WriteObject(m_hTarget);
			cMsg.Writeuint32(MID_QUERY_TARGET_PROPERTIES);
			cMsg.Writeuint8(1);
			cMsg.Writeuint8(TARGET_PROP_TRON_KIND);
			g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
			SetTargetStringID(kTronIDS_TargetTalk);
		}

		if (IsMultiplayerGame() && pFX && pFX->m_cs.bIsPlayer )
		{
			uint32 nId = pFX->m_cs.nClientID;
			CClientInfoMgr* pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
			CLIENT_INFO* pCI = pCIMgr->GetClientByID(nId);

			if (pCI)
			{
				m_nString = 0;
				SAFE_STRCPY(m_szString,pCI->sName.c_str());
			}
			return;
		}

		if ((fDistAway <= m_fTargetDistance) && pFX)
		{
			WEAPON const* pWeaponData = g_pWeaponMgr->GetWeapon(g_pPlayerStats->GetCurrentWeapon());
			if (pWeaponData && pWeaponData->bLooksDangerous)
			{
				// RETAIL: the crosshair's reticles (0x100aec9d): an enemy, or
				// a friend whom the weapon could hurt
				if (pFX->m_cs.eCrosshairCharacterClass == BAD)
					m_bTronUnknown877 = true;
				else
					m_bTronUnknown876 = true;
			}
		}
	}

	// RETAIL: an InfoString target: ask for its string (0x100aecb7)
	if ((dwUserFlags & USRFLG_INFO_STRING) && (fDistAway <= m_fTargetDistance))
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_OBJECT_MESSAGE);
		cMsg.WriteObject(m_hTarget);
		cMsg.Writeuint32(MID_QUERY_TARGET_PROPERTIES);
		cMsg.Writeuint8(1);
		cMsg.Writeuint8(TARGET_PROP_INFO_STRING);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
	}

	// RETAIL: a subroutine target: ask for its details while the player
	// manager wants them, in their range (0x100aed87)
	if (dwUserFlags & USRFLG_TRON_SUBROUTINE_PROPS)
	{
		if (g_pTronPlayerMgr->GetTronUnknown204() && (fDistAway < g_pTronPlayerMgr->GetTronUnknown20C()))
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_OBJECT_MESSAGE);
			cMsg.WriteObject(m_hTarget);
			cMsg.Writeuint32(MID_QUERY_TARGET_PROPERTIES);
			cMsg.Writeuint8(1);
			cMsg.Writeuint8(TARGET_PROP_TRON_SUBROUTINE);
			g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
		}
	}

	if (fDistAway <= m_fActivationDistance)
	{
		// RETAIL: the permission sets the target needs (0x100aeea9): bit 1
		// is in the test, but sets no permission
		if ((dwUserFlags & USRFLG_CAN_ACTIVATE) && (dwUserFlags & (USRFLG_TRON_PERMISSION_ANY)))
		{
			m_bPermissionRequired = true;
			if (dwUserFlags & USRFLG_PSET_1) m_nPermissionRequired |= (1<<0);
			if (dwUserFlags & USRFLG_PSET_2) m_nPermissionRequired |= (1<<1);
			if (dwUserFlags & USRFLG_PSET_3) m_nPermissionRequired |= (1<<2);
			if (dwUserFlags & USRFLG_PSET_4) m_nPermissionRequired |= (1<<3);
			if (dwUserFlags & USRFLG_PSET_5) m_nPermissionRequired |= (1<<4);
			if (dwUserFlags & USRFLG_PSET_6) m_nPermissionRequired |= (1<<5);
			if (dwUserFlags & USRFLG_PSET_7) m_nPermissionRequired |= (1<<6);
			if (dwUserFlags & USRFLG_PSET_8) m_nPermissionRequired |= (1<<7);
		}

		// See if this object is part of the activate object list with it's own string ID's...
		CActivateObjectHandler *pActivateObj = LTNULL;
		CActivateObjectHandler::ActivateObjList::const_iterator iter = CActivateObjectHandler::GetActivateObjectList().begin();
		while( iter != CActivateObjectHandler::GetActivateObjectList().end() )
		{
			pActivateObj = *iter;
			
			if( pActivateObj->GetHOBJECT() == m_hTarget )
			{
				ACTIVATETYPE *pType = g_pActivateTypeMgr->GetActivateType( pActivateObj->m_nId );
				if( pType )
				{
					// RETAIL: an object in Tron's third state uses the
					// strings below (0x100aef71)
					if (pActivateObj->m_eState == ACTIVATETYPE::eTronUnknown2)
						break;

					// Set whether or not it's disabled and set the string based on the state...

					m_bCanActivate = !pActivateObj->m_bDisabled;
					uint32 dwStringID = pType->dwStateID[pActivateObj->m_eState];
					
					if( dwStringID != (uint32)-1 )
					{
						SetTargetStringID( dwStringID );
					}

					return;
				}
			}

			++iter;
		}

		//can we pick up or activate it?
		if ((dwUserFlags & USRFLG_CAN_ACTIVATE) && (fDistAway <= m_fActivationDistance))
		{
			CPickupItemFX* const pFX = (CPickupItemFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_PICKUPITEM_ID, m_hTarget);
			if (pFX)
			{
				SetTargetStringID(kTronIDS_TargetTake);
				return;
			}

			if (dwUserFlags & USRFLG_REQUIRES_ENERGY)
			{
				// This object requires energy! Ask what it needs
				// RETAIL: five properties (0x100aefe0); the strings come
				// with the answer
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_OBJECT_MESSAGE);
				cMsg.WriteObject(m_hTarget);
				cMsg.Writeuint32(MID_QUERY_TARGET_PROPERTIES);
				cMsg.Writeuint8(5);
				cMsg.Writeuint8(TARGET_PROP_ENERGY_REQUIRED);
				cMsg.Writeuint8(TARGET_PROP_HEALTH);
				cMsg.Writeuint8(TARGET_PROP_TRON_3);
				cMsg.Writeuint8(TARGET_PROP_TRON_4);
				cMsg.Writeuint8(TARGET_PROP_INFO_STRING);
				g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
			}
			else
			{
				// RETAIL: only the target's own string (0x100af0fb)
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_OBJECT_MESSAGE);
				cMsg.WriteObject(m_hTarget);
				cMsg.Writeuint32(MID_QUERY_TARGET_PROPERTIES);
				cMsg.Writeuint8(1);
				cMsg.Writeuint8(TARGET_PROP_INFO_STRING);
				g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
			}
			return;
		}


		//we can't do anything with out target
		SetTargetStringID(NULL);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTargetMgr::HandleTargetPropertiesMessage
//
//	PURPOSE:	Special handler that processes additional information
//				about an object under the crosshair
//
// ----------------------------------------------------------------------- //
// RETAIL: the target text's concatenation (as the engine's LTStrCat, the
// destination's length taken first). The name is ours.
static inline void TronTargetStrCat(char *pDest, const char *pSrc, uint32 destBytes)
{
	uint32 destLen = strlen(pDest);
	uint32 catLen = destBytes - destLen - 1;
	strncat(pDest, pSrc, catLen);
	pDest[destLen + catLen] = '\0';
}

// RETAIL: 0x100af220. Tron's properties: what the target needs (energy,
// permissions and two others: strings 3801, 3805, 3807, 3809 unless the
// target set its own string), its info string, a subroutine's or a
// bin's kind (strings 3259, 3261, 3262) and Tron's subroutine details, the
// text of which the HUD shows (strings 3288..3310)
void CTronTargetMgr::HandleTargetPropertiesMessage(ILTMessage_Read *pMsg)
{
	// discard props if the crosshair is not over anything anymore
	if (!m_hTarget)
		return;

	HOBJECT hObj = pMsg->ReadObject();

	if (!hObj)
		return;

	// see if this is still a legitimate target object.
	if (hObj != m_hTarget)
		return;

	// if it is, then see what useful information we can grab from it
	int nProps = (int)pMsg->Readuint8();

	for (int i = nProps; i > 0; i--)
	{
		uint16 iThrowaway = pMsg->Readuint16(); // length of submessage was passed.  Ignore.

		uint8 iProp = pMsg->Readuint8();

		switch(iProp)
		{
		case TARGET_PROP_ENERGY_REQUIRED:
			{
				m_bEnergyRequired = true;
				m_nEnergyRequired = pMsg->Readuint32();
				if (!m_bTronUnknown860)
					SetTargetStringID(IDS_TRANSFER_ENERGY);
				break;
			}

		case TARGET_PROP_HEALTH:
			{
				m_bTronUnknown85D = true;
				m_nTronUnknown868 = pMsg->Readuint32();
				if (!m_bTronUnknown860)
					SetTargetStringID(3805);
				break;
			}

		case TARGET_PROP_TRON_3:
			{
				m_bTronUnknown85E = true;
				m_nTronUnknown86C = pMsg->Readuint32();
				if (!m_bTronUnknown860)
					SetTargetStringID(3807);
				break;
			}

		case TARGET_PROP_TRON_4:
			{
				m_bTronUnknown85F = true;
				m_nTronUnknown870 = pMsg->Readuint32();
				if (!m_bTronUnknown860)
					SetTargetStringID(3809);
				break;
			}

		case TARGET_PROP_TRON_SUBROUTINE:
			{
				// RETAIL: the properties come in this order (a pair's
				// second half first)
				LTVector vEntries(0.0f, 0.0f, 0.0f);
				uint16* pEntries = (uint16*)&vEntries;
				uint16 n0 = pMsg->Readuint16();
				uint16 n4 = pMsg->Readuint16();
				uint16 n5 = pMsg->Readuint16();
				uint16 n6 = pMsg->Readuint16();
				uint16 n7 = pMsg->Readuint16();
				uint16 n2 = pMsg->Readuint16();
				uint16 n1 = pMsg->Readuint16();
				uint16 n3 = pMsg->Readuint16();
				pEntries[1] = pMsg->Readuint16();
				pEntries[0] = pMsg->Readuint16();
				pEntries[3] = pMsg->Readuint16();
				pEntries[2] = pMsg->Readuint16();
				pEntries[5] = pMsg->Readuint16();
				pEntries[4] = pMsg->Readuint16();

				uint16* an = m_Props.anTronUnknown0;
				if (an[0] == n0 && an[1] == n1 && an[2] == n2 && an[3] == n3 &&
					an[4] == n4 && an[5] == n5 && an[6] == n6 && an[7] == n7)
					break;

				an[1] = n1;
				an[2] = n2;
				an[5] = n5;
				an[3] = n3;
				an[6] = n6;
				an[7] = n7;
				an[0] = n0;
				m_Props.vTronUnknown10 = vEntries;
				an[4] = n4;
				g_pHUDMgr->QueueUpdate(kHUDScores);	// RETAIL: 0x100000

				char* pszText = m_Props.szText;
				pszText[0] = 0;

				char szTmp[128];
				if (an[0])
				{
					const char* pszName = LoadTempString(an[0]);
					if (!pszName[0])
						FormatString(3289, szTmp, sizeof(szTmp), an[0]);
					else
						FormatString(3288, szTmp, sizeof(szTmp), LoadTempString(an[0]));
					TronTargetStrCat(pszText, szTmp, sizeof(m_Props.szText));
				}

				if (an[4] && an[5])
				{
					if (an[4] == 0xFFFF)
					{
						TronTargetStrCat(pszText, LoadTempString(3309), sizeof(m_Props.szText));
					}
					else
					{
						FormatString(3290, szTmp, sizeof(szTmp), an[4], an[5]);
						TronTargetStrCat(pszText, szTmp, sizeof(m_Props.szText));
					}
				}

				if (g_pTronPlayerMgr->m_nTronUnknown208 > 0)
				{
					if (an[6] && an[7])
					{
						FormatString(3291, szTmp, sizeof(szTmp), an[6], an[7]);
						TronTargetStrCat(pszText, szTmp, sizeof(m_Props.szText));
					}

					if (an[1])
					{
						FormatString(3296, szTmp, sizeof(szTmp), LoadTempString(an[1]));
						TronTargetStrCat(pszText, szTmp, sizeof(m_Props.szText));
					}

					if (g_pTronPlayerMgr->m_nTronUnknown208 > 1)
					{
						uint16* pEntry = (uint16*)&m_Props.vTronUnknown10;
						for (int j = 3; j > 0; j--, pEntry += 2)
						{
							if (pEntry[1] == 0)
							{
								FormatString(3293, szTmp, sizeof(szTmp), LoadTempString(pEntry[0]));
								TronTargetStrCat(pszText, szTmp, sizeof(m_Props.szText));
							}
							else if (pEntry[1] == 3)
							{
								FormatString(3294, szTmp, sizeof(szTmp), pEntry[0]);
								TronTargetStrCat(pszText, szTmp, sizeof(m_Props.szText));
							}
						}
					}
				}
				break;
			}

		case TARGET_PROP_INFO_STRING:
			{
				m_Props.nTronUnknown420 = pMsg->Readuint32();
				SetTargetStringID((uint16)m_Props.nTronUnknown420);
				m_bTronUnknown860 = true;
				break;
			}

		case TARGET_PROP_TRON_KIND:
			{
				uint8 nKind = pMsg->Readuint8();
				if (nKind == 1)
				{
					m_nString = (uint16)pMsg->Readuint32();
					if (m_nString)
					{
						FormatString(3259, m_szString, sizeof(m_szString));
						TronTargetStrCat(m_szString, LoadTempString(m_nString), sizeof(m_szString));
					}
					else
						FormatString(3259, m_szString, sizeof(m_szString));
				}
				else if (nKind == 2)
				{
					m_nString = (uint16)pMsg->Readuint32();
					if (m_nString)
					{
						FormatString(3261, m_szString, sizeof(m_szString));
						TronTargetStrCat(m_szString, LoadTempString(m_nString), sizeof(m_szString));
					}
					else
						FormatString(3261, m_szString, sizeof(m_szString));
				}
				else if (nKind == 3)
				{
					m_nString = (uint16)pMsg->Readuint32();
					if (m_nString)
					{
						FormatString(3262, m_szString, sizeof(m_szString));
						TronTargetStrCat(m_szString, LoadTempString(m_nString), sizeof(m_szString));
					}
					else
						FormatString(3262, m_szString, sizeof(m_szString));
				}
				else
				{
					m_szString[0] = 0;
					m_nString = 0;
				}
				break;
			}

		default:
			{
				break;
			}
		}
	}
}
/*
Andy
let the playermgr lock the target when a transfer starts
let the crosshair know when a a transfer starts

	IDS_REQUIRES_ENERGY				"Requires %d energy"
	IDS_INSUFFICIENT_ENERGY			"You do not have enough energy to complete this transfer."
	IDS_TRANSFER_ENERGY_SUCCESS		"Energy transfer successful."

*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTargetMgr::ClearTargetProperties
//
//	PURPOSE:	RETAIL: retail 0x100afa90 (the name is ours)
//
// ----------------------------------------------------------------------- //

void CTronTargetMgr::ClearTargetProperties()
{
	if (m_Props.szText[0])
	{
		m_Props.anTronUnknown0[0] = 0;
		m_Props.anTronUnknown0[1] = 0;
		m_Props.anTronUnknown0[2] = 0;
		m_Props.anTronUnknown0[3] = 0;
		m_Props.anTronUnknown0[4] = 0;
		m_Props.anTronUnknown0[5] = 0;
		m_Props.anTronUnknown0[6] = 0;
		m_Props.anTronUnknown0[7] = 0;
		m_Props.vTronUnknown10.Init();
		m_Props.szText[0] = 0;

		g_pHUDMgr->QueueUpdate(kHUDScores);	// RETAIL: 0x100000
	}
}
