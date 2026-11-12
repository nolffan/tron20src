/****************************************************************************
;
;	MODULE:			PatchRoutine.cpp
;
;	PURPOSE:		Patch routine class for TRON
;
;	HISTORY:		02/12/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
;	RETAIL:			Tron 2.0's, rewritten from retail Object.lto
;					(PatchRoutine.h).
;
****************************************************************************/

#include "stdafx.h"
#include "PatchRoutine.h"
#include "TronPlayerObj.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "MsgIDs.h"
#include "ServerUtilities.h"

// RETAIL: Tron's message ids (retail 0x1003328d, 0x10032af5); the names
// are ours. We tell the player's client to start draining us
// (MID_PATCHROUTINE_BEGIN: the kind, us twice, the amount); it sends the
// amounts it drains (MID_PATCHROUTINE_DRAIN) and when it stops
// (MID_PATCHROUTINE_STOP), which we also send it to stop
#define MID_PATCHROUTINE_BEGIN		154
#define MID_PATCHROUTINE_STOP		155
#define MID_PATCHROUTINE_DRAIN		156

// RETAIL: the kinds (retail 0x10033217, 0x10032b8e): Tron's target
// properties; the names are ours
#define PATCHROUTINE_ENERGY			2
#define PATCHROUTINE_HEALTH			3

// RETAIL: a user flag we set on ourselves (retail 0x10032a20); not yet
// identified on the client. The name is ours
#define USRFLG_PATCHROUTINE			(1<<11)

BEGIN_CLASS(PatchRoutine)
	ADD_LONGINTPROP(Health,0)
	ADD_LONGINTPROP(Energy,0)
	ADD_LONGINTPROP(DistAboveFloor,60)
END_CLASS_DEFAULT(PatchRoutine, GameBase, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::PatchRoutine()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

PatchRoutine::PatchRoutine() : GameBase()
{
	m_nTotalHealth = 0;
	m_nTotalEnergy = 0;
	m_nCurrentHealth = 0;
	m_nCurrentEnergy = 0;
	m_fDistAboveFloor = 0;
	m_pPlayer = NULL;
	m_hIdleFX = LTNULL;
	m_hTransferFX = LTNULL;
	m_bActive = false;
	m_bFromWorld = true;
	m_bInfinite = false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::~PatchRoutine()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

PatchRoutine::~PatchRoutine()
{
	if (m_hIdleFX)
	{
		g_pLTServer->RemoveObject(m_hIdleFX);
		m_hIdleFX = LTNULL;
	}

	if (m_hTransferFX)
	{
		g_pLTServer->RemoveObject(m_hTransferFX);
		m_hTransferFX = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PatchRoutine::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			LTRESULT ret = GameBase::EngineMessageFn(messageID, pData, fData);

			if((fData == PRECREATE_WORLDFILE) || (fData == PRECREATE_STRINGPROP))
			{
				ReadProp((ObjectCreateStruct*)pData);
				m_bFromWorld = true;
			}
			else
			{
				m_bFromWorld = false;
			}

			return ret;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate();
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE | USRFLG_PATCHROUTINE, USRFLG_CAN_ACTIVATE | USRFLG_PATCHROUTINE);
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
			return GameBase::EngineMessageFn(messageID, pData, fData);
		}

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
			return GameBase::EngineMessageFn(messageID, pData, fData);
		}

		default: break;
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 PatchRoutine::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_QUERY_TARGET_PROPERTIES:
		{
			uint8 nNumProps = pMsg->Readuint8();
			uint8 nNumPropsProcessed = 0;

			// See what we need to send down
			if(nNumProps > 0)
			{
				// Start the message
				CAutoMessage cTempMsg;

				// Loop through the props
				uint8 iProp;
				for(int i=0;i<nNumProps;i++)
				{
					// Get the prop
					iProp = pMsg->Readuint8();
					if (m_nCurrentHealth > 0 && iProp == PATCHROUTINE_HEALTH)
					{
						cTempMsg.Writeuint8(iProp);
						cTempMsg.Writeint32(m_nCurrentHealth);
						nNumPropsProcessed++;
					}
					else if (m_nCurrentEnergy > 0 && iProp == PATCHROUTINE_ENERGY)
					{
						cTempMsg.Writeuint8(iProp);
						cTempMsg.Writeint32(m_nCurrentEnergy);
						nNumPropsProcessed++;
					}
				}

				// Build the actual message to send back down to the client
				if(nNumPropsProcessed > 0)
				{
					// Get the player
					CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hSender);
					if(pPlayer)
					{
						CAutoMessage cPropMsg;
						cPropMsg.Writeuint8(MID_QUERY_TARGET_PROPERTIES);
						cPropMsg.WriteObject(m_hObject);
						cPropMsg.Writeuint8(nNumPropsProcessed);
						cPropMsg.WriteMessage(cTempMsg.Read());
						g_pLTServer->SendToClient(cPropMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);
					}
				}
			}
		}
		break;

		case MID_PATCHROUTINE_DRAIN:
		{
			int nAmount = pMsg->Readint32();
			if (nAmount <= 0 || !m_pPlayer) break;

			CDestructible* pDest = m_pPlayer->GetDestructible();

			// RETAIL: what is given is the amount asked, not the amount
			// left (see RETAIL_BUGS.md)
			int nGive = nAmount;

			if (m_nCurrentHealth > 0)
			{
				if (nAmount > m_nCurrentHealth) nAmount = m_nCurrentHealth;

				int nHitPoints = (int)pDest->GetHitPoints();
				int nMaxHitPoints = (int)pDest->GetMaxHitPoints();
				if (nHitPoints + nAmount > nMaxHitPoints)
				{
					nGive = nMaxHitPoints - nHitPoints;
				}

				if (!nGive)
				{
					HandleStopActivate(true);
					break;
				}

				pDest->SetHitPoints((LTFLOAT)(nHitPoints + nGive));

				if (!m_bInfinite)
				{
					m_nCurrentHealth -= nGive;
				}
			}
			else if (m_nCurrentEnergy > 0)
			{
				if (nAmount > m_nCurrentEnergy) nAmount = m_nCurrentEnergy;

				int nEnergy = (int)pDest->GetEnergy();
				int nMaxEnergy = (int)pDest->GetMaxEnergy();
				if (nEnergy + nAmount > nMaxEnergy)
				{
					nGive = nMaxEnergy - nEnergy;
				}

				if (!nGive)
				{
					HandleStopActivate(true);
					break;
				}

				pDest->SetEnergy((LTFLOAT)(nEnergy + nGive));

				if (!m_bInfinite)
				{
					m_nCurrentEnergy -= nGive;
				}
			}
			else
			{
				g_pLTServer->CPrint("ERROR - PatchRoutine received an activate message and it had nothing to give!");
			}
		}
		break;

		case MID_PATCHROUTINE_STOP:
		{
			HandleStopActivate(false);

			pMsg->Readuint8();
			pMsg->Readuint32();

			// All given: we are done
			if (!m_nCurrentHealth && !m_nCurrentEnergy)
			{
				HandleStopActivate(true);
				Remove();
			}
		}
		break;

		default:
			return GameBase::ObjectMessageFn(hSender, pMsg);
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL PatchRoutine::ReadProp(ObjectCreateStruct *pData)
{
	// Sanity check
    if (!pData) return LTFALSE;

	pData->m_Flags |= FLAG_FORCECLIENTUPDATE | FLAG_SOLID | FLAG_RAYHIT;
	pData->m_Flags2 |= FLAG2_SERVERDIMS;

	// Read them in
	GenericProp genProp;
	if ( g_pLTServer->GetPropGeneric("Health", &genProp) == LT_OK )
	{
		if (genProp.m_Long == -1)
		{
			m_bInfinite = true;
			m_nTotalHealth = 0x7fffffff;
		}
		else
		{
			m_nTotalHealth = genProp.m_Long;
		}
	}

	if ( g_pLTServer->GetPropGeneric("Energy", &genProp) == LT_OK )
	{
		if (genProp.m_Long == -1)
		{
			m_bInfinite = true;
			m_nTotalEnergy = 0x7fffffff;
		}
		else
		{
			m_nTotalEnergy = genProp.m_Long;
		}
	}

	if ( g_pLTServer->GetPropGeneric("DistAboveFloor", &genProp) == LT_OK )
	{
		m_fDistAboveFloor = genProp.m_Float;
	}

	// Sanity check
	if(m_nTotalEnergy && m_nTotalHealth)
	{
		// Bad level deisgner... BAD!!!
		g_pLTServer->CPrint("ERROR - A PatchRoutine object was found that had both energy and health specified. Bad level designer. BAD!!!\n");
		ASSERT(FALSE);
	}

	// And set
	Reset();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PatchRoutine::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_INT(m_nTotalHealth);
	SAVE_INT(m_nTotalEnergy);
	SAVE_INT(m_nCurrentHealth);
	SAVE_INT(m_nCurrentEnergy);
	SAVE_FLOAT(m_fDistAboveFloor);
	SAVE_HOBJECT(m_hIdleFX);
	SAVE_HOBJECT(m_hTransferFX);
	SAVE_bool(m_bInfinite);

	// We specifically don't save the player because we don't support
	// saving/loading during a transfer
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PatchRoutine::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_INT(m_nTotalHealth);
	LOAD_INT(m_nTotalEnergy);
	LOAD_INT(m_nCurrentHealth);
	LOAD_INT(m_nCurrentEnergy);
	LOAD_FLOAT(m_fDistAboveFloor);
	LOAD_HOBJECT(m_hIdleFX);
	LOAD_HOBJECT(m_hTransferFX);
	LOAD_bool(m_bInfinite);

	// We specifically don't save the player because we don't support
	// saving/loading during a transfer
	KillTransferFX();
	m_bActive = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //
bool PatchRoutine::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Activate("ACTIVATE");

	if (cMsg.GetArg(0) == s_cTok_Activate)
	{
		HandleActivate(hSender);
		return true;
	}

	return GameBase::OnTrigger(hSender, cMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::HandleActivate
//
//	PURPOSE:	Start activating: the player's client drains us
//
// ----------------------------------------------------------------------- //
void PatchRoutine::HandleActivate(HOBJECT hSender)
{
	// Sanity checks
	if(m_bActive)
	{
		// We should never get an activate message when we're already activated
		g_pLTServer->CPrint("ERROR - PatchRoutine received an activate message when it was already being activated!\n");
		ASSERT(FALSE);
		return;
	}

	if(!IsPlayer(hSender))
	{
		g_pLTServer->CPrint("ERROR - PatchRoutine received an activate message from an object other than a player!\n");
		ASSERT(FALSE);
		return;
	}

	// Make sure the player can take it
	CTronPlayerObj* pPlayer = (CTronPlayerObj*) g_pLTServer->HandleToObject(hSender);
	CDestructible* pDest = pPlayer->GetDestructible();

	uint8 nKind;
	int nAmount;
	if(m_nCurrentHealth > 0)
	{
		// If the player is already at his max, we're done.
		if(pDest->GetHitPoints() >= pDest->GetMaxHitPoints())
			return;

		nAmount = m_nCurrentHealth;
		nKind = PATCHROUTINE_HEALTH;
	}
	else if(m_nCurrentEnergy > 0)
	{
		// If the player is already at his max, we're done.
		if(pDest->GetEnergy() >= pDest->GetMaxEnergy())
			return;

		nAmount = m_nCurrentEnergy;
		nKind = PATCHROUTINE_ENERGY;
	}
	else
	{
		g_pLTServer->CPrint("ERROR - PatchRoutine received an activate message and it had nothing to give!");
		return;
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PATCHROUTINE_BEGIN);
	cMsg.Writeuint8(nKind);
	cMsg.WriteObject(m_hObject);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeint32(nAmount);
	g_pLTServer->SendToClient(cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);

	// We're all good
	m_pPlayer = pPlayer;
	m_bActive = true;

	CreateTransferFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::HandleStopActivate
//
//	PURPOSE:	Stop activating
//
// ----------------------------------------------------------------------- //
void PatchRoutine::HandleStopActivate(bool bTellClient)
{
	KillTransferFX();

	if (bTellClient && m_pPlayer)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PATCHROUTINE_STOP);
		cMsg.Writeuint8(1);
		g_pLTServer->SendToClient(cMsg.Read(), m_pPlayer->GetClient(), MESSAGE_GUARANTEED);
	}

	m_pPlayer = NULL;
	m_bActive = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::Remove
//
//	PURPOSE:	All given: go away, with an fx
//
// ----------------------------------------------------------------------- //
void PatchRoutine::Remove()
{
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);

	if (m_hIdleFX)
	{
		g_pLTServer->RemoveObject(m_hIdleFX);
		m_hIdleFX = LTNULL;
	}

	LTVector vPos;
	LTRotation rRot;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	if (m_nTotalHealth > 0)
	{
		PlayClientFX("WORLDS_PatchRoutine_Health_Remove", LTNULL, &vPos, &rRot, 0);
	}
	else
	{
		PlayClientFX("WORLDS_PatchRoutine_Energy_Remove", LTNULL, &vPos, &rRot, 0);
	}

	g_pLTServer->RemoveObject(m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::KillTransferFX
//
//	PURPOSE:	Stop the transfer fx
//
// ----------------------------------------------------------------------- //
void PatchRoutine::KillTransferFX()
{
	if (m_hTransferFX)
	{
		g_pLTServer->RemoveObject(m_hTransferFX);
		m_hTransferFX = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::CreateTransferFX
//
//	PURPOSE:	Start the transfer fx
//
// ----------------------------------------------------------------------- //
void PatchRoutine::CreateTransferFX()
{
	if (m_hTransferFX) return;

	char szProps[1024];
	if (m_nCurrentHealth)
	{
		sprintf(szProps, "FxName %s;Loop 1;SmoothShutdown 0", !m_bInfinite ? "WORLDS_PatchRoutine_Health_Transfer" : "WORLDS_PatchRoutine_Health_Transfer_Infinite");
	}
	else
	{
		sprintf(szProps, "FxName %s;Loop 1;SmoothShutdown 0", !m_bInfinite ? "WORLDS_PatchRoutine_Energy_Transfer" : "WORLDS_PatchRoutine_Energy_Transfer_Infinite");
	}

	m_hTransferFX = CreateClientFXObject(szProps, m_hObject, LTNULL, LTNULL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::InitialUpdate
//
//	PURPOSE:	Our dims; to the floor; the idle fx
//
// ----------------------------------------------------------------------- //
void PatchRoutine::InitialUpdate()
{
	LTVector vDims;
	vDims.y = vDims.x = vDims.z = 20.0f;
	g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);

	if (!m_bFromWorld) return;
	if (!m_nCurrentHealth && !m_nCurrentEnergy) return;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vPos;
	IQuery.m_To = LTVector(vPos.x, vPos.y - 10000.0f, vPos.z);
	IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

	if (g_pLTServer->IntersectSegment(&IQuery, &IInfo))
	{
		vPos.y -= (vPos.y - IInfo.m_Point.y - m_fDistAboveFloor) - (vDims.y + 0.1f);
		g_pLTServer->SetObjectPos(m_hObject, &vPos);
	}

	char szProps[1024];
	if (m_nCurrentHealth)
	{
		sprintf(szProps, "FxName %s;Loop 1;SmoothShutdown 0", !m_bInfinite ? "WORLDS_PatchRoutine_Health_Idle" : "WORLDS_PatchRoutine_Health_Idle_Infinite");
	}
	else
	{
		sprintf(szProps, "FxName %s;Loop 1;SmoothShutdown 0", !m_bInfinite ? "WORLDS_PatchRoutine_Energy_Idle" : "WORLDS_PatchRoutine_Energy_Idle_Infinite");
	}

	m_hIdleFX = CreateClientFXObject(szProps, m_hObject, LTNULL, LTNULL);
}
