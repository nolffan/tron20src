// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerMgr.cpp
//
// PURPOSE : Implementation of class to handle managment of missions and worlds.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronPlayerMgr.h"
#include "TronPlayerStats.h" // needed for energy transfer data
#include "HUDMgr.h"
#include "SubroutineMgr.h"
#include "ClientSoundMgr.h"
#include "LayoutMgr.h"

CTronPlayerMgr* g_pTronPlayerMgr = NULL;

// RETAIL: two globals set up here and used nowhere (retail 0x1026ef4c,
// 0x1026ef48: 15 degrees, and a third of it). The names are ours
float g_fTronUnknownAngle = MATH_DEGREES_TO_RADIANS(15.0f);
float g_fTronUnknownAngleThird = g_fTronUnknownAngle / 3.0f;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::CTronPlayerMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTronPlayerMgr::CTronPlayerMgr() : CPlayerMgr()
{
	// RETAIL: retail 0x100a4350
	m_nTronUnknown1F4 = 0;
	m_bTransferringEnergy = false;
	m_hTronUnknownSound22C = NULL;
	m_bTronUnknown1E9 = false;
	m_hTronUnknownSound224 = NULL;
	m_bTronUnknown1EA = false;
	m_hTronUnknownSound228 = NULL;
	m_bTronUnknown1EB = false;
	m_bTronUnknown234 = false;
	m_fTronUnknownBC = 0.0f;
	m_bTronUnknownB8 = false;
	m_nTronUnknown230 = 0;
	m_bTronUnknown235 = false;

	m_fFOVZoomLOL = 30.0f;
	m_fFOVZoomTriangulate1 = 20.0f;
	m_fFOVZoomTriangulate2 = 7.0f;
	m_fFOVZoomTriangulate3 = 2.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::~CTronPlayerMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CTronPlayerMgr::~CTronPlayerMgr()
{
	g_pTronPlayerMgr = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CTronPlayerMgr::Init()
{
	g_pTronPlayerMgr = this;

	// RETAIL: retail 0x100a4460
	m_nTronUnknown1F4 = 0;
	m_bTransferringEnergy = false;
	m_hTronUnknownSound22C = NULL;
	m_bTronUnknown1E9 = false;
	m_hTronUnknownSound224 = NULL;
	m_bTronUnknown1EA = false;
	m_hTronUnknownSound228 = NULL;
	m_bTronUnknown1EB = false;

	m_fFOVZoomLOL = g_pLayoutMgr->GetFloat("hudlayout0", "FOVZoomLOL", 30.0f);
	m_fFOVZoomTriangulate1 = g_pLayoutMgr->GetFloat("hudlayout0", "FOVZoomTriangulate1", 20.0f);
	m_fFOVZoomTriangulate2 = g_pLayoutMgr->GetFloat("hudlayout0", "FOVZoomTriangulate2", 7.0f);
	m_fFOVZoomTriangulate3 = g_pLayoutMgr->GetFloat("hudlayout0", "FOVZoomTriangulate3", 2.0f);

	// PORT: retail copies a string of a global (retail 0x10270864, +0x1d4)
	// into CPlayerMgr's +0x1b5 here; not yet identified

	m_bTronUnknown234 = false;
	m_bTronUnknown236 = true;

	return CPlayerMgr::Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::Term
//
//	PURPOSE:	Term the mgr
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::Term()
{
	// RETAIL: retail 0x100a4570
	m_nTronUnknown1F4 = 0;

	m_bTransferringEnergy = false;
	if (m_hTronUnknownSound22C)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound22C);
		m_hTronUnknownSound22C = NULL;
	}

	m_bTronUnknown1E9 = false;
	if (m_hTronUnknownSound224)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound224);
		m_hTronUnknownSound224 = NULL;
	}

	m_bTronUnknown1EA = false;
	if (m_hTronUnknownSound228)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound228);
		m_hTronUnknownSound228 = NULL;
	}

	m_bTronUnknown1EB = false;

	g_pTronPlayerMgr = NULL;

	CPlayerMgr::Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::OnCommandOn()
//
//	PURPOSE:	Handle client commands
//
// ----------------------------------------------------------------------- //

LTBOOL CTronPlayerMgr::OnCommandOn(int command)
{
	return CPlayerMgr::OnCommandOn(command);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::OnCommandOff()
//
//	PURPOSE:	Handle command off notification
//
// ----------------------------------------------------------------------- //

LTBOOL CTronPlayerMgr::OnCommandOff(int command)
{
	// RETAIL: retail 0x100a4960
	switch (command)
	{
		case COMMAND_ID_ACTIVATE :
		{
            if (IsTransferringEnergy())
			{
				// PORT: Tron's PlaySoundLocal has a seventh argument (1.0f)
				g_pClientSoundMgr->PlaySoundLocal("get_done", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);

				// Clear the transfer flags
				m_bTransferringEnergy = false;
				if (m_hTronUnknownSound22C)
				{
					g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound22C);
					m_hTronUnknownSound22C = NULL;
				}

				m_bTronUnknown1E9 = false;
				if (m_hTronUnknownSound224)
				{
					g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound224);
					m_hTronUnknownSound224 = NULL;
				}

				m_bTronUnknown1EA = false;
				if (m_hTronUnknownSound228)
				{
					g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound228);
					m_hTronUnknownSound228 = NULL;
				}

				m_bTronUnknown1EB = false;

				HOBJECT hObj = m_pTargetMgr->GetLockedTarget();
				hObj = m_hTronUnknown200;

				// release the lock on the target
				m_pTargetMgr->LockTarget(NULL);
				m_hTronUnknown200 = NULL;

				// Insufficient transfer.  Abort
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_OBJECT_MESSAGE);
				cMsg.WriteObject(hObj);
				cMsg.Writeuint32(MID_START_ENERGY_TRANSFER);	// RETAIL: 155 (0x9b) here
				cMsg.Writeuint8(1);
				cMsg.Writefloat(m_fEnergyTransferred);
				g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
				return LTTRUE;
			}
		}
		break;

        default : 
			break;
	}

	return CPlayerMgr::OnCommandOff(command);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::OnMessage()
//
//	PURPOSE:	Handle client messages specific to TRON
//
// ----------------------------------------------------------------------- //

LTBOOL CTronPlayerMgr::OnMessage(uint8 messageID, ILTMessage_Read *pMsg)
{
	// RETAIL: Tron's overrides CPlayerMgr::OnMessage (retail 0x100a4b80),
	// for six messages: 136, 152, 154, 155, 193 and 200. PORT: only build
	// 69's energy transfer start (155) is ours yet; the rest are left to
	// CPlayerMgr
	switch(messageID)
	{
		case MID_START_ENERGY_TRANSFER:		HandleMsgStartEnergyTransfer	(pMsg);			break;
		default:							return CPlayerMgr::OnMessage(messageID, pMsg);	break;
	}
	
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::Pause()
//
//	PURPOSE:	Note the pausing and unpausing of the game.
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::Pause(LTBOOL bPause)
{
	// RETAIL: retail 0x100a4c90
	m_bPause = bPause;
	if (!bPause)
	{
		m_fLastUpdateTime = g_pLTClient->GetTime();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::Update()
//
//	PURPOSE:	Handle client updates
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::Update()
{
	CPlayerMgr::Update();

	if (m_bPause)	// if paused, that's all we do.
		return;

	if (m_bTransferringEnergy)
	{
		// advance any timers.
		float fNow = g_pLTClient->GetTime();
		float fDeltaTime = fNow - m_fLastUpdateTime;
		m_fLastUpdateTime = fNow;

		// Increment the amount of energy transferred based upon some fixed rate
		float fEnergyPerSecond = 5.0f;

		float fEnergy = fEnergyPerSecond * fDeltaTime;

		m_fEnergyTransferred += fEnergy;
		g_pHUDMgr->QueueUpdate(kHUDEnergyTrans);
		g_pHUDMgr->QueueUpdate(kHUDEnergy);
		if (m_fEnergyTransferred > m_fEnergyRequired)
		{
			// TRANSFER COMPLETE
			m_fEnergyTransferred = 0;
			m_bTransferringEnergy = false;
			HOBJECT hObj = m_pTargetMgr->GetLockedTarget();
			m_pTargetMgr->LockTarget(NULL);
			int iEnergy = g_pTronPlayerStats->GetEnergy() - (int)m_fEnergyRequired;

			if (iEnergy < 0)
				iEnergy = 0;

			// Tell the playerstatsmgr about the new energy balance
			g_pTronPlayerStats->UpdateEnergy(iEnergy);

			// Notify the binary bit that the energy transfer is complete
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_OBJECT_MESSAGE);
			cMsg.WriteObject(hObj);
			cMsg.Writeuint32(MID_ENERGY_TRANSFER_COMPLETE);
			g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
			// TODO send a message to the HUD
		}
	}
}

void CTronPlayerMgr::InitTargetMgr()
{
	m_pTargetMgr = debug_new( CTronTargetMgr );				 
	ASSERT( 0 != m_pTargetMgr );
}

void CTronPlayerMgr::HandleMsgStartEnergyTransfer(ILTMessage_Read * pMsg)
{
	int iEnergyRequired = pMsg->Readuint32();
	int iEnergyAvailable = g_pTronPlayerStats->GetEnergy();

	HOBJECT hObj = m_pTargetMgr->GetTargetObject();

	if (!hObj) // this should be impossible
		return;

	// Exit out with a message to the screen that you don't have enough energy
	// On the exit, send back the FAILED message to the BinaryBit.
	if (iEnergyRequired > iEnergyAvailable)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_OBJECT_MESSAGE);
		cMsg.WriteObject(hObj);
		cMsg.Writeuint32(MID_STOP_ENERGY_TRANSFER);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
		// TODO put a message on the HUD about insufficient energy
		return;
	}

	// Set the transfer flag
	m_bTransferringEnergy = true;

	m_fEnergyTransferred = 0.0f;
	m_fEnergyRequired = (float)iEnergyRequired;

	// lock targetmgr on this object until the player releases the activate key.
	m_pTargetMgr->LockTarget(hObj);

	m_fLastUpdateTime = g_pLTClient->GetTime();
}

float CTronPlayerMgr::GetPercentEnergyTransferred()
{
	// RETAIL: retail 0x100a6f30
	if (!IsTransferringEnergy())
		return 0.0f;

	if (!(int)m_fEnergyRequired)
		return 0.0f;

	return (m_fEnergyTransferred / m_fEnergyRequired);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::GetJumpVelocity
//
//	PURPOSE:	Gets the player's jump velocity which is dependent
//				on certain subroutines
//
// ----------------------------------------------------------------------- //
float CTronPlayerMgr::GetJumpVelocity(float fJumpVel, float fSuperJumpVel)
{
	// Check for existence of the jump enhancing subroutine
	if(g_pSubroutineMgr->IsUtilitySubroutineActive("YAmp"))
	{
		float fDelta = fSuperJumpVel - fJumpVel;
		
		// Check the state of it
		switch(g_pSubroutineMgr->GetActiveSubroutineVersion("YAmp"))
		{
			case VERSION_ALPHA:
			{
				return(fJumpVel + (fDelta * 0.33f));
			}
			case VERSION_BETA:
			{
				return(fJumpVel + (fDelta * 0.66f));
			}
			case VERSION_GOLD:
			{
				return(fSuperJumpVel);
			}
			break;
		}
	}

	return fJumpVel;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::GetTronUnknown204 / GetTronUnknown20C
//
//	PURPOSE:	RETAIL: Tron's accessors (retail 0x100a5530, 0x100a5540;
//				the names are ours)
//
// ----------------------------------------------------------------------- //

bool CTronPlayerMgr::GetTronUnknown204()
{
	return m_bTronUnknown204;
}

float CTronPlayerMgr::GetTronUnknown20C()
{
	return m_fTronUnknown20C;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::Save / Load
//
//	PURPOSE:	RETAIL: Tron's (retail 0x100a7ba0, 0x100a7c00): three of its
//				flags before CPlayerMgr's
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState)
{
	pMsg->Writebool(m_bTronUnknown210);
	pMsg->Writebool(m_bTronUnknown211);
	pMsg->Writebool(m_bTronUnknown234);

	CPlayerMgr::Save(pMsg, eSaveDataState);
}

void CTronPlayerMgr::Load(ILTMessage_Read *pMsg, SaveDataState eLoadDataState)
{
	m_bTronUnknown210 = pMsg->Readbool();
	m_bTronUnknown211 = pMsg->Readbool();
	m_bTronUnknown234 = pMsg->Readbool();

	CPlayerMgr::Load(pMsg, eLoadDataState);

	m_bTronUnknown1D5 = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtual3C
//
//	PURPOSE:	RETAIL: Tron's (retail 0x100a8870): +0xb8 set and, when asked,
//				+0x230 too. Not yet named
//
// ----------------------------------------------------------------------- //

bool CTronPlayerMgr::TronUnknownVirtual3C(bool b)
{
	return (m_bTronUnknownB8 && (!b || m_nTronUnknown230));
}
