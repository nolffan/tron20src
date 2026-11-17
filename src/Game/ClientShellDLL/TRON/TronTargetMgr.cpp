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

// PORT: build 51's TargetMgr.h had this, which Tron's code uses; NOLF2's
// TargetMgr reads it from the client butes instead (ActivationDistance)
const float kActivationDist = 100.0f;

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
void CTronTargetMgr::Update()
{
	if (m_hLockedTarget && m_hTarget == m_hLockedTarget)
	{

		// are we transferring energy?
		if (g_pTronPlayerMgr->IsTransferringEnergy())
		{
			// don't change anything.
			SetTargetStringID(IDS_TRANSFERRING_ENERGY);
			return;
		}

		//are we disabling a GadgetTarget?
		if (g_pPlayerMgr->IsDisabling())
		{
			SetGadgetTarget( true );
			return;
		}

		//are we searching something?
		if (g_pPlayerMgr->IsSearching())
		{
			m_bSearchTarget = true;
			SetTargetStringID(IDS_TARGET_SEARCHING);
			return;
		}
	}

	// RETAIL: the old target and whether its properties were kept
	// (0x100ae66f)
	bool bOldProps = m_Props.bTronUnknown424;
	HOBJECT hOldTarget = m_hTarget;

	//clear our targeting info
	m_hTarget = NULL;
	m_fTargetRange = 10000.0f;
	m_szString[0] = 0;
	m_szDebugString[0] = 0;
	m_nString = 0;

	m_ActivationData.Init();

	m_bEnergyRequired = false;
	m_nEnergyRequired = 0;

	m_bSearchTarget = false;
	m_bMoveTarget = false;
	m_bCanActivate = true;

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


	//if its a body's hitbox, check the body instead
	CBodyFX* pBody = g_pGameClientShell->GetSFXMgr()->GetBodyFromHitBox(m_hTarget);
	if (pBody)
	{
		m_hTarget = pBody->GetServerObj();
		m_ActivationData.m_hTarget = m_hTarget;
		if (!m_hTarget) return;
	}


	uint32 dwUserFlags = 0;
    g_pCommonLT->GetObjectFlags(m_hTarget, OFT_User, dwUserFlags);

	//special case handling for bodies
	if (pBody)
	{
		LTRotation	rCamRot;
		LTVector	vTargetPos, vCamPos, vDir, vCamR;

		HLOCALOBJ	hCamera = g_pPlayerMgr->GetCamera();

		g_pLTClient->GetObjectRotation( hCamera, &rCamRot );
		vCamR = rCamRot.Right();

		g_pLTClient->GetObjectPos( hCamera, &vCamPos );
		g_pLTClient->GetObjectPos( m_hTarget, &vTargetPos );
		

		vDir = vTargetPos - vCamPos;

		// See if we are looking at the target...

		vDir.Normalize();
		LTFLOAT	fCamAngle = vDir.Dot( vCamR );

		bool bCanSearch = !!(dwUserFlags & USRFLG_CAN_SEARCH);
		bool bCanMove = (pBody->CanBeCarried() && g_pPlayerMgr->CanDropCarriedObject());	// PORT: build 51's CanDropBody
		bool bCanRevive = pBody->CanBeRevived();

		if (bCanSearch && bCanMove)
		{
			if (fCamAngle > 0)
			{
				// we can search this body
				if (fDistAway <= kActivationDist)
				{
					m_bSearchTarget = true;
					m_ActivationData.m_nType = MID_ACTIVATE_SEARCH;
					SetTargetStringID(IDS_TARGET_SEARCH);
					return;
				}
			}
			else
			{
				// we can move this body
				if (fDistAway <= kActivationDist)
				{
					m_bMoveTarget = true;
					m_ActivationData.m_nType = MID_ACTIVATE_MOVE;
					SetTargetStringID(IDS_TARGET_MOVE);
					return;
				}
			}
		}
		else if (bCanSearch && fDistAway <= kActivationDist)
		{
			// we can search this body
			m_bSearchTarget = true;
			m_ActivationData.m_nType = MID_ACTIVATE_SEARCH;
			SetTargetStringID(IDS_TARGET_SEARCH);
			return;
		}
		else if (bCanMove && fDistAway <= kActivationDist)
		{
			// we can move this body
			m_bMoveTarget = true;
			m_ActivationData.m_nType = MID_ACTIVATE_MOVE;
			SetTargetStringID(IDS_TARGET_MOVE);
			return;
		}
		else if( bCanRevive && fDistAway <= kActivationDist )
		{
			// we can revive this body
			m_ActivationData.m_nType = MID_ACTIVATE_REVIVE;
			SetTargetStringID(IDS_TARGET_REVIVE);
			return;
		}
		else
		{
			SetTargetStringID(NULL);
			return;
		}

	}

	// is this a gadget target
	if (IsGadgetActivatable(m_hTarget) && (fDistAway <= kActivationDist))
	{
		// looks like we can use a gadget on it...
		SetGadgetTarget(false);
		return;
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
		if (dwUserFlags & USRFLG_CAN_ACTIVATE)
		{
			SetTargetStringID(IDS_TARGET_TALK);
			return;
		}

	
		WEAPON const* pWeaponData = g_pWeaponMgr->GetWeapon(g_pPlayerStats->GetCurrentWeapon());
		if (pWeaponData)
		{
			// warn the player if we have a dangerous weapon and are pointing at a friend...
			if (pWeaponData->bLooksDangerous && pFX && pFX->m_cs.eCrosshairCharacterClass != BAD)
			{
				if (fDistAway <= (LTFLOAT) pWeaponData->nRange)
				{
					SetTargetStringID(IDS_TARGET_INNOCENT);
					return;
				}
			}


		}
	}
/*	Don't allow SEARCHING in Tron
	//is this a searchable object?
	if (dwUserFlags & USRFLG_CAN_SEARCH && (fDistAway <= kActivationDist))
	{
		m_bSearchTarget = true;
		m_ActivationData.m_nType = MID_ACTIVATE_SEARCH;
		SetTargetStringID(IDS_TARGET_SEARCH);
		return;

	}
*/
	//can we pick up or activate it?
	if (dwUserFlags & USRFLG_CAN_ACTIVATE && (fDistAway <= kActivationDist))
	{

		CPickupItemFX* const pFX = (CPickupItemFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_PICKUPITEM_ID, m_hTarget);
		if (pFX)
			SetTargetStringID(IDS_TARGET_TAKE);
		else
		{
			if (dwUserFlags & USRFLG_REQUIRES_ENERGY)
			{
				// This object requires energy! Send a message off to find out how much it needs
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_OBJECT_MESSAGE);
				cMsg.WriteObject(m_hTarget);
				cMsg.Writeuint32(MID_QUERY_TARGET_PROPERTIES);
				cMsg.Writeuint8(1);
				cMsg.Writeuint8(TARGET_PROP_ENERGY_REQUIRED);
				g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
				SetTargetStringID(IDS_TRANSFER_ENERGY);
			}
			else
			{
				SetTargetStringID(IDS_TARGET_USE);
			}
		}
		return;
	}


	//we can't do anything with out target
	SetTargetStringID(NULL);
	return;

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

		case 3:
			{
				m_bTronUnknown85E = true;
				m_nTronUnknown86C = pMsg->Readuint32();
				if (!m_bTronUnknown860)
					SetTargetStringID(3807);
				break;
			}

		case 4:
			{
				m_bTronUnknown85F = true;
				m_nTronUnknown870 = pMsg->Readuint32();
				if (!m_bTronUnknown860)
					SetTargetStringID(3809);
				break;
			}

		case 9:
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

		case 7:
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
