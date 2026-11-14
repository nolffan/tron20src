// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleMatchMgr.cpp
//
// PURPOSE : The client's light cycle game (rounds and matches) manager
//
// RETAIL: Tron 2.0's, retail-only; written from retail CShell.dll
// 0x10016130..0x10017670 (see LightCycleMatchMgr.h). The functions are in
// retail's order.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initializers (0x10016130..0x10016192)
#include "LightCycleMatchMgr.h"
#include "GameClientShell.h"
#include "InterfaceMgr.h"
#include "ClientInfoMgr.h"
#include "ClientSoundMgr.h"
#include "ClientMultiplayerMgr.h"
#include "ProfileMgr.h"
#include "CMoveMgr.h"
#include "HUDMgr.h"
#include "HUDTransmission.h"
#include "HUDMessageQueue.h"
#include "HUDChatInput.h"
#include "HUDScores.h"
#include "TronPlayerMgr.h"
#include "LightCycleMgr.h"
#include "MsgIDs.h"

extern CGameClientShell* g_pGameClientShell;

CLightCycleMatchMgr* g_pLightCycleMatchMgr = LTNULL;

// RETAIL: Tron's HUD update flag these messages queue (the name is ours)
#define LCMM_HUD_TRONUNKNOWN1000000		((eHUDUpdateFlag)0x1000000)

// RETAIL: Tron's message 200 (CTronPlayerMgr::HandleMsgTronUnknown200) and
// the sub-message the client sends with it (the names are ours)
#define LCMM_MSG_TRONUNKNOWN200			200
#define LCMM_MSG_TRONUNKNOWN32			32

// RETAIL: the voice lines of a team (GetTeamSound's bases; the names are ours)
#define LCMM_SOUND_MATCHOVER			20924
#define LCMM_SOUND_ROUNDOVER			20948

// RETAIL: 0x10236da0, GetTeamSound's buffer (the name is ours)
static char s_szTeamSound[64];

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::CLightCycleMatchMgr
//
// ----------------------------------------------------------------------- //

CLightCycleMatchMgr::CLightCycleMatchMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::Init
//
// ----------------------------------------------------------------------- //

bool CLightCycleMatchMgr::Init()
{
	g_pLightCycleMatchMgr = this;
	Reset();
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::Term
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::Term()
{
	g_pLightCycleMatchMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::Reset
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::Reset()
{
	m_nTronUnknown8 = 2;
	m_nState = 0;
	m_nRound = 0;
	m_fTimeLeft = 0.0f;
	m_fStartTime = 0.0f;
	m_fTimeLimit = 0.0f;
	m_bTronUnknown1F = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::OnMessage
//
//	PURPOSE:	Tron's message 200 in the light cycle game type
//
// ----------------------------------------------------------------------- //

bool CLightCycleMatchMgr::OnMessage(ILTMessage_Read *pMsg)
{
	switch (pMsg->Readuint8())
	{
		case 20:	HandleMsgSettings(pMsg);		return true;
		case 21:	HandleMsgRoundStart(pMsg);		return true;
		case 22:	HandleMsgRound(pMsg);			return true;
		case 23:	HandleMsgTronUnknown23(pMsg);	return true;
		case 24:	HandleMsgRoundOver(pMsg);		return true;
		case 25:	HandleMsgGameOver(pMsg);		return true;
		case 30:	HandleMsgTronUnknown30(pMsg);	return true;
		case 33:	HandleMsgSpectate(pMsg);		return true;
		case 83:	HandleMsgTronUnknown83(pMsg);	return true;
		case 80:	HandleMsgTeams(pMsg);			return true;
		case 81:	HandleMsgTeamChange(pMsg);		return true;
		case 35:	HandleMsgTronUnknown35(pMsg);	return true;
		case 36:	HandleMsgMatchOver(pMsg);		return true;
		case 37:	HandleMsgTronUnknown37(pMsg);	return true;
		case 38:	HandleMsgTronUnknown38(pMsg);	return true;
		case 39:	HandleMsgTronUnknown39(pMsg);	return true;
	}

	// Not ours: let the next one read it from the start
	pMsg->SeekTo(0);
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleMsgSettings
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleMsgSettings(ILTMessage_Read *pMsg)
{
	float fTronUnknown = pMsg->Readfloat();
	g_pPlayerMgr->SetTronUnknown1DC(fTronUnknown);

	m_nTronUnknown8 = pMsg->Readuint8();
	m_nTronUnknown19 = pMsg->Readuint8();
	m_nTronUnknown18 = pMsg->Readuint8();
	m_nTronUnknown1A = pMsg->Readuint16();
	m_nTronUnknown1C = pMsg->Readuint8();
	m_nTronUnknown1D = pMsg->Readuint8();
	m_nTronUnknown1E = pMsg->Readuint8();
	m_fTimeLeft = m_fTimeLimit = pMsg->Readfloat();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleMsgRoundStart
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleMsgRoundStart(ILTMessage_Read *pMsg)
{
	m_nState = 2;
	m_bTronUnknown1F = false;
	m_fStartTime = g_pLTClient->TronUnknownVirtual44();
	m_fTimeLeft = m_fTimeLimit = pMsg->Readfloat();
	m_nTronUnknown19 = pMsg->Readuint8();

	g_pInterfaceMgr->ClearTronUnknownMessage();
	g_pTransmission->Show(3469);
	g_pClientSoundMgr->PlayInterfaceSound("snd\\event\\mp_pointstart.WAV");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleMsgRound
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleMsgRound(ILTMessage_Read *pMsg)
{
	// RETAIL: 0x1026ee04, Tron's scores HUD (our g_pScores)
	g_pScores->Show(false, false);

	if (m_nState != 1)
	{
		m_nState = 1;
		if (m_bTronUnknown1F && g_pInterfaceMgr->GetGameState() == GS_PLAYING)
		{
			g_pInterfaceMgr->StartScreenFadeOut(0.01f, false);
			g_pInterfaceMgr->ForceScreenFadeIn(2.0f);
		}

		g_pLightCycleMgr->RemoveAllCyclists();
	}

	m_nRound = pMsg->Readuint8();

	char szMsg[128];
	FormatString(3478, szMsg, sizeof(szMsg), m_nRound);
	g_pChatMsgs->AddMessage(szMsg, kMsgDefault);

	uint32 nTime = pMsg->Readuint16();
	g_pInterfaceMgr->SetTronUnknownMessage((float)nTime, 3470, LTNULL, true);

	g_pHUDMgr->QueueUpdate(LCMM_HUD_TRONUNKNOWN1000000);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleMsgRoundOver
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleMsgRoundOver(ILTMessage_Read *pMsg)
{
	if (m_nState != 3)
	{
		m_nState = 3;
	}

	uint32 nTime = pMsg->Readuint16();
	m_nTronUnknown19 = pMsg->Readuint8();
	int8 nWinner = pMsg->Readuint8();
	uint8 nNoChat = pMsg->Readuint8();

	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID(&nLocalID);

	CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();
	int nLocalTeam = pTeamMgr->GetTeamOfClient(nLocalID);

	int nTeam = nWinner;
	if (nTeam == nLocalTeam)
	{
		g_pClientSoundMgr->PlayInterfaceSound("snd\\event\\mp_matchwin.WAV");
	}
	else
	{
		g_pClientSoundMgr->PlayInterfaceSound("snd\\event\\mp_matchloss.WAV");
	}

	char szMsg[256];
	szMsg[0] = '\0';
	if (nWinner == -1)
	{
		FormatString(3474, szMsg, sizeof(szMsg));
	}
	else
	{
		const char* pszSound = GetTeamSound(1, nTeam);
		if (pszSound)
		{
			g_pClientSoundMgr->PlayInterfaceSound(pszSound);
		}

		std::string sTeam;
		pTeamMgr->GetTeam(nTeam, false)->GetName(false, sTeam, true);
		FormatString(3479, szMsg, sizeof(szMsg), sTeam.c_str(), m_nTronUnknown19);
	}

	g_pInterfaceMgr->SetTronUnknownMessage((float)nTime, 0, szMsg, false);

	if (!nNoChat)
	{
		FormatString(3473, szMsg, sizeof(szMsg));
		g_pChatMsgs->AddMessage(szMsg, kMsgDefault);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleMsgGameOver
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleMsgGameOver(ILTMessage_Read *pMsg)
{
	g_pScores->Show(true, true);

	if (m_nState != 4)
	{
		m_nState = 4;
		m_fTimeLeft = 0.0f;
		m_fStartTime = 0.0f;
		m_fTimeLimit = 0.0f;
	}

	char szMsg[256];
	szMsg[0] = '\0';

	uint32 nTime = pMsg->Readuint16();
	pMsg->Readuint8();
	int8 nWinner = pMsg->Readuint8();
	uint8 nTied = pMsg->Readuint8();

	if (nTied > 0)
	{
		FormatString(3485, szMsg, sizeof(szMsg), nTied + 1);
	}
	else
	{
		CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();

		// No winner sent: the team with the best score
		if (nWinner == -1)
		{
			int nBest = 0x80000000;
			for (uint32 i = 0; i < (uint32)pTeamMgr->GetNumTeams(); i++)
			{
				int16 nScore = pTeamMgr->GetTeam(i, false)->GetTronUnknown12();
				if (nScore > nBest)
				{
					nBest = nScore;
					nTied = 0;
					nWinner = i;
				}
				else if (nScore == nBest)
				{
					nTied++;
				}
			}
		}

		uint32 nLocalID = 0;
		g_pLTClient->GetLocalClientID(&nLocalID);
		int nLocalTeam = pTeamMgr->GetTeamOfClient(nLocalID);

		int nTeam = nWinner;
		if (nTeam != nLocalTeam && nWinner != -1)
		{
			g_pClientSoundMgr->PlayInterfaceSound("snd\\event\\mp_matchloss.WAV");
		}
		else
		{
			g_pClientSoundMgr->PlayInterfaceSound("snd\\event\\mp_matchwin.WAV");
		}

		const char* pszSound = GetTeamSound(2, nTeam);
		if (pszSound)
		{
			g_pClientSoundMgr->PlayInterfaceSound(pszSound);
		}

		std::string sTeam;
		pTeamMgr->GetTeam(nTeam, false)->GetName(false, sTeam, true);
		FormatString(3480, szMsg, sizeof(szMsg), sTeam.c_str());
	}

	g_pInterfaceMgr->SetTronUnknownMessage((float)nTime, 0, szMsg, false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleMsgTronUnknown30
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleMsgTronUnknown30(ILTMessage_Read *pMsg)
{
	g_pTransmission->Show(2566);
	m_nState = 2;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::Update
//
//	PURPOSE:	The time left while playing (CTronInterfaceMgr::Update)
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::Update()
{
	switch (m_nState)
	{
		case 2:
			m_fTimeLeft = m_fTimeLimit - (g_pLTClient->TronUnknownVirtual44() - m_fStartTime);
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::OnCyclistDerezzed
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::OnCyclistDerezzed(HOBJECT hObj, HOBJECT hOther)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::OnKeyDown
//
//	PURPOSE:	The space bar (outside the chat) watches another cycle
//				while the local player has none
//
// ----------------------------------------------------------------------- //

bool CLightCycleMatchMgr::OnKeyDown(int nKey, int nRep)
{
	if (nKey == VK_SPACE && !g_pChatInput->IsVisible())
	{
		uint32 nLocalID = 0;
		g_pLTClient->GetLocalClientID(&nLocalID);
		if (g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr()->GetTeamOfClient(nLocalID) != TRON_MAX_TEAMS)
		{
			SendTronUnknown32();
		}

		if (g_pTronPlayerMgr->TronUnknownVirtual3C(false))
		{
			g_pTronPlayerMgr->TronUnknownLightCycleSpectate();
			return true;
		}

		if (m_nState != 1 || !m_bTronUnknown1F)
		{
			if (!g_pLightCycleMgr->GetLivingCyclist(g_pMoveMgr->GetObject()))
			{
				g_pTronPlayerMgr->TronUnknownLightCycleSpectate();
				return true;
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::SendTronUnknown32
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::SendTronUnknown32()
{
	char szMsg[128];
	FormatString(3481, szMsg, sizeof(szMsg));
	strcpy(g_pInterfaceMgr->GetTronUnknownMessageText2(), szMsg);

	CAutoMessage cMsg;
	cMsg.Writeuint8(LCMM_MSG_TRONUNKNOWN200);
	cMsg.Writeuint8(LCMM_MSG_TRONUNKNOWN32);
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleMsgSpectate
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleMsgSpectate(ILTMessage_Read *pMsg)
{
	if (pMsg->Readbool())
	{
		m_bTronUnknown1F = false;
		if (!g_pTronPlayerMgr->TronUnknownVirtual3C(false))
		{
			g_pTronPlayerMgr->TronUnknownLightCycleSpectate();
		}
	}
	else
	{
		g_pTronPlayerMgr->TronUnknownStopSpectating();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleMsgTronUnknown83
//
//	PURPOSE:	A message box of string 335 with two numbers
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleMsgTronUnknown83(ILTMessage_Read *pMsg)
{
	uint16 nTronUnknownA = pMsg->Readuint16();
	uint16 nTronUnknownB = pMsg->Readuint16();

	char szMsg[128];
	szMsg[0] = '\0';
	FormatString(335, szMsg, sizeof(szMsg), nTronUnknownA, nTronUnknownB);

	// PORT: retail widens the text into a std::wstring for Tron's
	// CMessageBox::Show(std::wstring, ...) (through an out-of-line
	// CInterfaceMgr::ShowMessageBox, 0x1000fa80); our message box takes the
	// narrow string
	MBCreate mb;
	g_pInterfaceMgr->ShowMessageBox(szMsg, &mb);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleMsgTeams
//
//	PURPOSE:	The disc match manager's message 80 (the same code, folded)
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleMsgTeams(ILTMessage_Read *pMsg)
{
	CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();
	pTeamMgr->SetNumTeams(pMsg->Readuint8());
	pTeamMgr->SetTeamSize(pMsg->Readuint8());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleMsgTeamChange
//
//	PURPOSE:	A client joined a team
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleMsgTeamChange(ILTMessage_Read *pMsg)
{
	CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();

	uint32 nClientID = pMsg->Readuint32();
	uint8 nTeam = pMsg->Readuint8();

	bool bSolo = false;
	if (pTeamMgr->GetTeam(0, false)->GetMaxClients() <= 1)
	{
		bSolo = true;
	}

	if (nTeam == TRON_MAX_TEAMS || !bSolo)
	{
		const char* pszName = g_pGameClientShell->GetInterfaceMgr()->GetClientInfoMgr()->GetPlayerName(nClientID);
		if (pszName)
		{
			std::string sTeam;
			pTeamMgr->GetTeam(nTeam, false)->GetName(false, sTeam, true);

			char szMsg[128];
			FormatString(3468, szMsg, sizeof(szMsg), pszName, sTeam.c_str());
			g_pChatMsgs->AddMessage(szMsg, kMsgDefault);
		}
	}

	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID(&nLocalID);
	if (nLocalID != nClientID)
		return;

	// RETAIL: Tron's profile byte +0x6a (our m_nPlayerSkills[0] sits
	// there) takes the profile's +0x69
	CUserProfile* pProfile = g_pProfileMgr->GetCurrentProfile();
	pProfile->m_nPlayerSkills[0] = pProfile->m_nDMPlayerModel;

	// PORT: with a team, retail then gives the local cycle the profile's
	// cycle color (Tron's profile string at +0x1d4, which our CUserProfile
	// has not) in the team's shade (CTronTeamMgr 0x100fba70) through the
	// light cycle bute manager and the player manager's slot 0x90

	g_pClientMultiplayerMgr->UpdateNetClientData();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleMsgTronUnknown35
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleMsgTronUnknown35(ILTMessage_Read *pMsg)
{
	m_nState = 0;
	g_pInterfaceMgr->ClearTronUnknownMessage();
	m_fTimeLeft = 0.0f;
	m_fStartTime = 0.0f;
	m_fTimeLimit = 0.0f;
	m_bTronUnknown1F = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleMsgTronUnknown23
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleMsgTronUnknown23(ILTMessage_Read *pMsg)
{
	g_pTronPlayerMgr->TronUnknownStopSpectating();
	g_pGameClientShell->m_bTronUnknown833A = true;
	m_bTronUnknown1F = true;
	g_pInterfaceMgr->TronUnknownFirstPersonCamera(true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleMsgMatchOver
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleMsgMatchOver(ILTMessage_Read *pMsg)
{
	uint32 nWinner = pMsg->Readuint32();

	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID(&nLocalID);
	int nLocalTeam = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr()->GetTeamOfClient(nLocalID);

	// RETAIL: the win sound when the team sent is not the local team's
	// (0x10017215): the team sent may be the match's loser (not known yet)
	if (nWinner != nLocalTeam)
	{
		g_pClientSoundMgr->PlayInterfaceSound("snd\\event\\mp_matchwin.WAV");
	}
	else
	{
		g_pClientSoundMgr->PlayInterfaceSound("snd\\event\\mp_matchloss.WAV");
	}

	const char* pszSound = GetTeamSound(0, nWinner);
	if (pszSound)
	{
		g_pClientSoundMgr->PlayInterfaceSound(pszSound);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleMsgTronUnknown37
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleMsgTronUnknown37(ILTMessage_Read *pMsg)
{
	uint8 nNumber = pMsg->Readuint8();

	char szMsg[256];
	FormatString(3490, szMsg, sizeof(szMsg), nNumber);
	g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\chat.wav");
	g_pChatMsgs->AddMessage(szMsg, kMsgTeam);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleMsgTronUnknown38
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleMsgTronUnknown38(ILTMessage_Read *pMsg)
{
	int8 nTeam = pMsg->Readuint8();
	uint8 nCount = pMsg->Readuint8();

	char szMsg[256];
	if (!nCount && nTeam != -1)
	{
		std::string sTeam;
		CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();
		pTeamMgr->GetTeam(nTeam, false)->GetName(false, sTeam, true);
		FormatString(3476, szMsg, sizeof(szMsg), sTeam.c_str());
	}
	else
	{
		FormatString(3477, szMsg, sizeof(szMsg), nCount + 1);
	}

	g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\chat.wav");
	g_pChatMsgs->AddMessage(szMsg, kMsgTeam);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::HandleMsgTronUnknown39
//
// ----------------------------------------------------------------------- //

void CLightCycleMatchMgr::HandleMsgTronUnknown39(ILTMessage_Read *pMsg)
{
	int8 nTeam = pMsg->Readuint8();
	uint16 nScore = pMsg->Readuint16();

	std::string sTeam;
	CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();
	pTeamMgr->GetTeam(nTeam, false)->GetName(false, sTeam, true);

	char szMsg[256];
	FormatString(3472, szMsg, sizeof(szMsg), sTeam.c_str(), nScore);
	g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\chat.wav");
	g_pChatMsgs->AddMessage(szMsg, kMsgTeam);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMatchMgr::GetTeamSound
//
// ----------------------------------------------------------------------- //

const char* CLightCycleMatchMgr::GetTeamSound(int nKind, int nTeam)
{
	int nBase;
	switch (nKind)
	{
		case 1:		nBase = LCMM_SOUND_ROUNDOVER;	break;
		case 0:		nBase = LCMM_SOUND_MATCHOVER;	break;
		default:	return LTNULL;
	}

	// RETAIL: teams 3 and 4 have each other's lines
	if (nTeam == 3)
	{
		nTeam = 4;
	}
	else if (nTeam == 4)
	{
		nTeam = 3;
	}

	if (g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr()->GetTeam(0, false)->GetMaxClients() > 1)
	{
		sprintf(s_szTeamSound, "voice\\%d.wav", nBase + nTeam);
		return s_szTeamSound;
	}

	return LTNULL;
}
