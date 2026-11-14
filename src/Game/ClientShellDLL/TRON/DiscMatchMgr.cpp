// ----------------------------------------------------------------------- //
//
// MODULE  : DiscMatchMgr.cpp
//
// PURPOSE : The client's disc game (arena and tournament) match manager
//
// RETAIL: Tron 2.0's, retail-only; written from retail CShell.dll
// 0x1000dd50..0x10010650 (see DiscMatchMgr.h). The functions are in
// retail's order.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initializers (0x1000dd50..0x1000ddb2)
#include "DiscMatchMgr.h"
#include "GameClientShell.h"
#include "InterfaceMgr.h"
#include "ClientInfoMgr.h"
#include "ClientSoundMgr.h"
#include "ClientMultiplayerMgr.h"
#include "ProfileMgr.h"
#include "HUDMgr.h"
#include "HUDTransmission.h"
#include "HUDMessageQueue.h"
#include "HUDDiscScores.h"
#include "VarTrack.h"
#include "MsgIDs.h"

extern CGameClientShell* g_pGameClientShell;
extern VarTrack g_vtInAirAccelMultiplier;	// CMoveMgr.cpp (RETAIL: 0x10270e68)

CDiscMatchMgr* g_pDiscMatchMgr = LTNULL;

// RETAIL: Tron's HUD update flags these messages queue (the names are ours)
#define DMM_HUD_TRONUNKNOWN2000000		((eHUDUpdateFlag)0x2000000)
#define DMM_HUD_TRONUNKNOWN8000000		((eHUDUpdateFlag)0x8000000)
#define DMM_HUD_TRONUNKNOWN10000000		((eHUDUpdateFlag)0x10000000)

// RETAIL: the voice lines of a team (GetTeamSound's bases; the names are ours)
#define DMM_SOUND_GAMEOVER_ARENA		20868
#define DMM_SOUND_MATCHOVER				20876
#define DMM_SOUND_GAMEOVER_TOURNAMENT	20892

// RETAIL: 0x10236d20, GetTeamSound's buffer (the name is ours)
static char s_szTeamSound[64];

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::CDiscMatchMgr
//
// ----------------------------------------------------------------------- //

CDiscMatchMgr::CDiscMatchMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::Init
//
// ----------------------------------------------------------------------- //

bool CDiscMatchMgr::Init()
{
	Reset();
	g_pDiscMatchMgr = this;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::Term
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::Term()
{
	g_pDiscMatchMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::Reset
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::Reset()
{
	if (g_pDiscScores)
	{
		g_pDiscScores->Show(false, false);
	}

	m_Matches.clear();
	m_nState = 0;
	m_nTronUnknown8 = (uint32)-1;
	m_fTronUnknownC = -1.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::OnMessage
//
//	PURPOSE:	Tron's message 200 in the disc game types: the sub-message
//				id is peeked, then skipped by the handler's case
//
// ----------------------------------------------------------------------- //

bool CDiscMatchMgr::OnMessage(ILTMessage_Read *pMsg)
{
	switch (pMsg->Peekuint8())
	{
		case 0:		pMsg->Readuint8(); HandleMsgSettings(pMsg);			return true;
		case 80:	pMsg->Readuint8(); HandleMsgTeams(pMsg);			return true;
		case 8:		pMsg->Readuint8(); HandleMsgTimer(pMsg);			return true;
		case 1:		pMsg->Readuint8(); HandleMsgAddMatch(pMsg);			return true;
		case 2:		pMsg->Readuint8(); HandleMsgMatchOver(pMsg);		return true;
		case 3:		pMsg->Readuint8(); HandleMsgRoundOver(pMsg);		return true;
		case 4:		pMsg->Readuint8(); HandleMsgMatchScore(pMsg);		return true;
		case 5:		pMsg->Readuint8(); HandleMsgGameOver(pMsg);			return true;
		case 6:		pMsg->Readuint8(); HandleMsgCountdown(pMsg);		return true;
		case 7:		pMsg->Readuint8(); HandleMsgWaiting(pMsg);			return true;
		case 81:	pMsg->Readuint8(); HandleMsgTeamChange(pMsg);		return true;
		case 9:		pMsg->Readuint8(); HandleMsgTeamMessage(pMsg);		return true;
		case 10:	pMsg->Readuint8(); HandleMsgTronUnknown10(pMsg);	return true;
		case 11:	pMsg->Readuint8(); HandleMsgTronUnknown11(pMsg);	return true;
		case 101:	pMsg->Readuint8(); HandleMsgTronUnknown101(pMsg);	return true;
		case 83:	pMsg->Readuint8(); HandleMsgTronUnknown83(pMsg);	return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::HandleMsgSettings
//
//	PURPOSE:	The game's air control and settings
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::HandleMsgSettings(ILTMessage_Read *pMsg)
{
	uint8 nAirControl = pMsg->Readuint8();
	m_nTronUnknown8 = pMsg->Readuint8();

	if (g_pGameClientShell->GetGameType() == eGameTypeDiscArena)
	{
		float fTime = pMsg->Readfloat();
		if (fTime > 0.0f)
		{
			m_fTronUnknownC = g_pLTClient->GetTime() + fTime;
		}
		else
		{
			m_fTronUnknownC = -1.0f;
		}
	}
	else
	{
		m_fTronUnknownC = -1.0f;
	}

	g_vtInAirAccelMultiplier.SetFloat((float)nAirControl * 0.01f);

	g_pHUDMgr->QueueUpdate(DMM_HUD_TRONUNKNOWN10000000);
	g_pHUDMgr->QueueUpdate(DMM_HUD_TRONUNKNOWN8000000);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::HandleMsgTeams
//
//	PURPOSE:	The number of teams and their size. RETAIL: the light cycle
//				match manager's message 80 is the same code (folded)
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::HandleMsgTeams(ILTMessage_Read *pMsg)
{
	CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();
	pTeamMgr->SetNumTeams(pMsg->Readuint8());
	pTeamMgr->SetTeamSize(pMsg->Readuint8());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::HandleMsgTimer
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::HandleMsgTimer(ILTMessage_Read *pMsg)
{
	if (GetGameType() == eGameTypeDiscArena)
	{
		float fTime = pMsg->Readfloat();
		if (fTime > 0.0f)
		{
			m_fTronUnknownC = g_pLTClient->GetTime() + fTime;
		}
		else
		{
			m_fTronUnknownC = -1.0f;
		}
	}
	else
	{
		m_nTronUnknown8 = pMsg->Readuint8();
		m_fTronUnknownC = -1.0f;
	}

	g_pHUDMgr->QueueUpdate(DMM_HUD_TRONUNKNOWN10000000);
	g_pHUDMgr->QueueUpdate(DMM_HUD_TRONUNKNOWN8000000);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::HandleMsgAddMatch
//
//	PURPOSE:	A match is added (and starts when the local team plays it)
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::HandleMsgAddMatch(ILTMessage_Read *pMsg)
{
	g_pDiscScores->Show(false, false);

	bool bTronUnknown = pMsg->Readbool();
	if (pMsg->Readbool())
	{
		m_Matches.clear();
	}

	int nTeamB;
	int nTeamA = pMsg->Readuint8();
	nTeamB = pMsg->Readuint8();

	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID(&nLocalID);

	CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();
	int nLocalTeam = pTeamMgr->GetTeamOfClient(nLocalID);

	// The local team goes second
	int iA = 0;
	int iB = 1;
	if (nLocalTeam == nTeamA)
	{
		iA = 1;
		iB = 0;
	}

	DiscMatch match;
	match.nTeam[iA] = nTeamB;
	match.nTeam[iB] = nTeamA;
	match.nWins[iA] = pMsg->Readuint8();
	match.nWins[iB] = pMsg->Readuint8();
	match.nOrder = pMsg->Readuint8();
	match.nTronUnknown5 = pMsg->Readuint8();
	match.nTronUnknown8 = pMsg->Readuint8();
	match.nTronUnknownC = (int8)pMsg->Readuint8();
	match.nTronUnknown10 = (int8)pMsg->Readuint8();
	match.fTronUnknown14 = g_pLTClient->GetTime() + pMsg->Readfloat();

	m_Matches.push_back(match);
	m_Matches.sort();

	if (!bTronUnknown && (nLocalTeam == nTeamB || nLocalTeam == nTeamA))
	{
		m_nState = 1;

		std::string sTeamA;
		std::string sTeamB;
		pTeamMgr->GetTeam(match.nTeam[0], false)->GetName(false, sTeamA, false);
		pTeamMgr->GetTeam(match.nTeam[1], false)->GetName(false, sTeamB, false);

		char szMsg[256];
		FormatString(3456, szMsg, sizeof(szMsg), sTeamA.c_str(), sTeamB.c_str());
		g_pTransmission->Show(szMsg);

		g_pClientSoundMgr->PlayInterfaceSound("snd\\event\\mp_matchstart.wav");
		g_pGameClientShell->m_bTronUnknown833A = true;

		if (g_pInterfaceMgr->GetGameState() == GS_SCREEN)
		{
			g_pInterfaceMgr->ChangeState(GS_PLAYING);
		}

		g_pInterfaceMgr->StartScreenFadeOut(0.01f, false);
		g_pInterfaceMgr->ForceScreenFadeIn(2.0f);
	}

	g_pHUDMgr->QueueUpdate(DMM_HUD_TRONUNKNOWN2000000);
	g_pHUDMgr->QueueUpdate(DMM_HUD_TRONUNKNOWN10000000);
	g_pHUDMgr->QueueUpdate(DMM_HUD_TRONUNKNOWN8000000);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::HandleMsgMatchOver
//
//	PURPOSE:	A team won its match: the match is removed
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::HandleMsgMatchOver(ILTMessage_Read *pMsg)
{
	uint8 nWinner = pMsg->Readuint8();

	DiscMatchList::iterator iter;
	if (FindMatch(nWinner, iter) < 0)
		return;

	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID(&nLocalID);

	CClientInfoMgr* pCIMgr = g_pGameClientShell->GetInterfaceMgr()->GetClientInfoMgr();
	if (!pCIMgr)
		return;

	uint32 nLocalState = pCIMgr->GetClientByID(nLocalID)->sScore.m_nTronUnknown18;

	CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();
	int nLocalTeam = pTeamMgr->GetTeamOfClient(nLocalID);

	if ((nLocalState == 0 || nLocalState == 3) &&
		((*iter).nTeam[0] == nLocalTeam || (*iter).nTeam[1] == nLocalTeam))
	{
		m_nState = 0;

		std::string sTeam;
		pTeamMgr->GetTeam(nWinner, false)->GetName(false, sTeam, false);

		char szMsg[128];
		FormatString(3457, szMsg, sizeof(szMsg), sTeam.c_str());
		g_pTransmission->Show(szMsg);

		const char* pszSound = GetTeamSound(DMM_SOUND_MATCHOVER, nWinner);
		if (pszSound)
		{
			g_pClientSoundMgr->PlayInterfaceSound(pszSound);
		}
		else if (nLocalTeam == nWinner)
		{
			g_pClientSoundMgr->PlayInterfaceSound("snd\\event\\mp_matchwin.WAV");
		}
		else
		{
			g_pClientSoundMgr->PlayInterfaceSound("snd\\event\\mp_matchloss.WAV");
		}
	}

	m_Matches.erase(iter);

	g_pHUDMgr->QueueUpdate(DMM_HUD_TRONUNKNOWN2000000);
	g_pHUDMgr->QueueUpdate(DMM_HUD_TRONUNKNOWN10000000);
	g_pHUDMgr->QueueUpdate(DMM_HUD_TRONUNKNOWN8000000);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::HandleMsgRoundOver
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::HandleMsgRoundOver(ILTMessage_Read *pMsg)
{
	m_nState = 2;
	g_pTransmission->Show(3458);
	g_pClientSoundMgr->PlayInterfaceSound("voice\\20980.wav");
	g_pGameClientShell->m_bTronUnknown833A = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::HandleMsgMatchScore
//
//	PURPOSE:	A match's wins
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::HandleMsgMatchScore(ILTMessage_Read *pMsg)
{
	uint8 nTeamA = pMsg->Readuint8();
	uint8 nWinsA = pMsg->Readuint8();
	uint8 nWinsB = pMsg->Readuint8();
	bool bShow = pMsg->Readbool();
	uint8 nTeam = pMsg->Readuint8();

	DiscMatchList::iterator iter;
	if (FindMatch(nTeam, iter) < 0)
		return;

	if (nTeamA == (*iter).nTeam[0])
	{
		(*iter).nWins[0] = nWinsA;
		(*iter).nWins[1] = nWinsB;
	}
	else
	{
		(*iter).nWins[1] = nWinsA;
		(*iter).nWins[0] = nWinsB;
	}

	g_pHUDMgr->QueueUpdate(DMM_HUD_TRONUNKNOWN2000000);
	g_pHUDMgr->QueueUpdate(DMM_HUD_TRONUNKNOWN10000000);
	g_pHUDMgr->QueueUpdate(DMM_HUD_TRONUNKNOWN8000000);

	if (!IsLocalInMatch(nTeam))
		return;

	if (bShow)
	{
		CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();

		std::string sTeam;
		pTeamMgr->GetTeam(nTeam, false)->GetName(false, sTeam, false);

		char szMsg[128];
		FormatString(3461, szMsg, sizeof(szMsg), sTeam.c_str());
		g_pTransmission->Show(szMsg);
	}

	g_pGameClientShell->m_bTronUnknown833A = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::HandleMsgCountdown
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::HandleMsgCountdown(ILTMessage_Read *pMsg)
{
	uint16 nCount = pMsg->Readuint16();
	if (nCount == 3)
	{
		g_pClientSoundMgr->PlayInterfaceSound("voice\\21013.wav");
	}
	else if (nCount > 3)
	{
		return;
	}

	char szMsg[128];
	FormatString(3459, szMsg, sizeof(szMsg), nCount);
	g_pTransmission->Show(szMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::HandleMsgWaiting
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::HandleMsgWaiting(ILTMessage_Read *pMsg)
{
	uint16 nCount = pMsg->Readuint16();
	if (nCount > 0)
	{
		char szMsg[128];
		if (g_pGameClientShell->GetGameType() == eGameTypeDiscArena)
		{
			FormatString(3462, szMsg, sizeof(szMsg), nCount);
		}
		else
		{
			FormatString(3460, szMsg, sizeof(szMsg), nCount);
		}
		g_pTransmission->Show(szMsg);

		if (nCount <= 5)
		{
			g_pClientSoundMgr->PlayInterfaceSound("snd\\event\\mp_countdown.WAV");
		}

		m_fTronUnknownC = -1.0f;
	}
	else
	{
		g_pTransmission->Show("");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::HandleMsgGameOver
//
//	PURPOSE:	The game's winners and the scores
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::HandleMsgGameOver(ILTMessage_Read *pMsg)
{
	uint8 nNumWinners = pMsg->Readuint8();
	uint8 nNumSecond = pMsg->Readuint8();
	uint8 nNumThird = pMsg->Readuint8();

	std::vector<uint8> aWinners;
	std::vector<uint8> aSecond;
	std::vector<uint8> aThird;
	aWinners.reserve(nNumWinners);
	aSecond.reserve(nNumSecond);
	aThird.reserve(nNumThird);

	uint32 i;
	for (i = 0; i < nNumWinners; i++)
	{
		aWinners.push_back(pMsg->Readuint8());
	}
	for (i = 0; i < nNumSecond; i++)
	{
		aSecond.push_back(pMsg->Readuint8());
	}
	for (i = 0; i < nNumThird; i++)
	{
		aThird.push_back(pMsg->Readuint8());
	}

	m_fTronUnknownC = -1.0f;
	g_pDiscScores->Show(true, true);

	CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();

	std::string sTeam;
	char szMsg[256];
	szMsg[0] = '\0';

	const char* pszSound;
	if (GetGameType() == eGameTypeDiscArena)
	{
		if (aWinners.size() > 1)
		{
			FormatString(3485, szMsg, sizeof(szMsg), aWinners.size());
		}
		else if (aWinners.size() == 1)
		{
			pTeamMgr->GetTeam(aWinners[0], false)->GetName(false, sTeam, false);
			FormatString(3484, szMsg, sizeof(szMsg), sTeam.c_str());

			pszSound = GetTeamSound(DMM_SOUND_GAMEOVER_ARENA, aWinners[0]);
			if (pszSound)
			{
				g_pClientSoundMgr->PlayInterfaceSound(pszSound);
			}
		}
	}
	else if (aWinners.size() == 1)
	{
		pTeamMgr->GetTeam(aWinners[0], false)->GetName(false, sTeam, false);
		FormatString(3486, szMsg, sizeof(szMsg), sTeam.c_str());

		pszSound = GetTeamSound(DMM_SOUND_GAMEOVER_TOURNAMENT, aWinners[0]);
		if (pszSound)
		{
			g_pClientSoundMgr->PlayInterfaceSound(pszSound);
		}
	}

	if (!szMsg[0])
	{
		g_pTransmission->Show(3380);
	}
	else
	{
		g_pTransmission->Show(szMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::HandleMsgTeamChange
//
//	PURPOSE:	A client joined a team
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::HandleMsgTeamChange(ILTMessage_Read *pMsg)
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
		CClientInfoMgr* pCIMgr = g_pGameClientShell->GetInterfaceMgr()->GetClientInfoMgr();

		std::string sTeam;
		pTeamMgr->GetTeam(nTeam, false)->GetName(false, sTeam, false);

		char szMsg[128];
		FormatString(3468, szMsg, sizeof(szMsg), pCIMgr->GetPlayerName(nClientID), sTeam.c_str());
		g_pChatMsgs->AddMessage(szMsg, kMsgTeam);
	}

	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID(&nLocalID);
	if (nLocalID != nClientID)
		return;

	// RETAIL: Tron's profile byte +0x6a (our m_nPlayerSkills[0] sits
	// there): the team's index, or the profile's +0x69 without a team
	CUserProfile* pProfile = g_pProfileMgr->GetCurrentProfile();
	pProfile->m_nPlayerSkills[0] = pProfile->m_nDMPlayerModel;
	if (nTeam < TRON_MAX_TEAMS && !bSolo)
	{
		pProfile->m_nPlayerSkills[0] = pTeamMgr->GetTeam(nTeam, false)->GetTronUnknownIndex();
	}

	g_pClientMultiplayerMgr->UpdateNetClientData();

	m_nState = 0;
	g_pGameClientShell->m_bTronUnknown833A = false;

	g_pHUDMgr->QueueUpdate(DMM_HUD_TRONUNKNOWN2000000);
	g_pHUDMgr->QueueUpdate(DMM_HUD_TRONUNKNOWN10000000);
	g_pHUDMgr->QueueUpdate(DMM_HUD_TRONUNKNOWN8000000);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::HandleMsgTeamMessage
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::HandleMsgTeamMessage(ILTMessage_Read *pMsg)
{
	CTronTeamMgr* pTeamMgr = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr();

	char szMsg[256];
	if (GetGameType() == eGameTypeDiscArena)
	{
		if (pMsg->Readbool())
		{
			uint8 nTeam = pMsg->Readuint8();

			std::string sTeam;
			pTeamMgr->GetTeam(nTeam, false)->GetName(false, sTeam, false);
			FormatString(3488, szMsg, sizeof(szMsg), sTeam.c_str());
		}
		else
		{
			uint8 nNumber = pMsg->Readuint8();
			FormatString(3490, szMsg, sizeof(szMsg), nNumber);
		}

		g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\chat.wav");
		g_pChatMsgs->AddMessage(szMsg, kMsgTeam);
	}
	else
	{
		uint8 nTeamA = pMsg->Readuint8();
		uint8 nTeamB = pMsg->Readuint8();
		uint8 nVoice = pMsg->Readuint8();

		std::string sTeamA;
		std::string sTeamB;
		pTeamMgr->GetTeam(nTeamA, false)->GetName(false, sTeamA, false);
		pTeamMgr->GetTeam(nTeamB, false)->GetName(false, sTeamB, false);

		FormatString(3489, szMsg, sizeof(szMsg), sTeamA.c_str(), sTeamB.c_str());
		g_pChatMsgs->AddMessage(szMsg, kMsgTeam);

		char szSound[64];
		sprintf(szSound, "voice\\%d.wav", nVoice + 20998);
		g_pClientSoundMgr->PlayInterfaceSound(szSound);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::HandleMsgTronUnknown10
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::HandleMsgTronUnknown10(ILTMessage_Read *pMsg)
{
	g_pTransmission->Show(3487);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::HandleMsgTronUnknown11
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::HandleMsgTronUnknown11(ILTMessage_Read *pMsg)
{
	g_pGameClientShell->m_bTronUnknown833A = pMsg->Readbool();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::HandleMsgTronUnknown101
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::HandleMsgTronUnknown101(ILTMessage_Read *pMsg)
{
	g_pTransmission->Show(571);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::HandleMsgTronUnknown83
//
//	PURPOSE:	A message box of string 335 with two numbers
//
// ----------------------------------------------------------------------- //

void CDiscMatchMgr::HandleMsgTronUnknown83(ILTMessage_Read *pMsg)
{
	uint16 nTronUnknownA = pMsg->Readuint16();
	uint16 nTronUnknownB = pMsg->Readuint16();

	char szMsg[256];
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
//	ROUTINE:	CDiscMatchMgr::GetNumMatches
//
// ----------------------------------------------------------------------- //

int CDiscMatchMgr::GetNumMatches()
{
	return m_Matches.size();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::GetLocalMatchIndex
//
// ----------------------------------------------------------------------- //

int CDiscMatchMgr::GetLocalMatchIndex()
{
	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID(&nLocalID);

	int nLocalTeam = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr()->GetTeamOfClient(nLocalID);

	DiscMatchList::iterator iter;
	return FindMatch(nLocalTeam, iter);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::GetMatch
//
// ----------------------------------------------------------------------- //

DiscMatch* CDiscMatchMgr::GetMatch(uint32 nIndex)
{
	DiscMatchList::iterator iter = m_Matches.begin();
	for (uint32 i = 0; iter != m_Matches.end() && i < nIndex; i++)
	{
		iter++;
	}

	return &(*iter);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::FindMatch
//
//	PURPOSE:	The match a team plays in: its index (-1 if none)
//
// ----------------------------------------------------------------------- //

int CDiscMatchMgr::FindMatch(uint8 nTeam, DiscMatchList::iterator& iter)
{
	int nIndex = 0;
	for (iter = m_Matches.begin(); iter != m_Matches.end(); iter++)
	{
		if ((*iter).nTeam[0] == nTeam || (*iter).nTeam[1] == nTeam)
			break;

		nIndex++;
	}

	if (iter == m_Matches.end())
		return -1;

	return nIndex;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::IsLocalInMatch
//
//	PURPOSE:	Does the local client play in the team's match
//
// ----------------------------------------------------------------------- //

bool CDiscMatchMgr::IsLocalInMatch(uint8 nTeam)
{
	DiscMatchList::iterator iter;
	if (FindMatch(nTeam, iter) < 0)
		return false;

	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID(&nLocalID);

	CClientInfoMgr* pCIMgr = g_pGameClientShell->GetInterfaceMgr()->GetClientInfoMgr();
	if (!pCIMgr)
		return false;

	uint32 nLocalState = pCIMgr->GetClientByID(nLocalID)->sScore.m_nTronUnknown18;
	if (nLocalState != 0 && nLocalState != 3)
		return false;

	int nLocalTeam = g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr()->GetTeamOfClient(nLocalID);
	if ((*iter).nTeam[0] != nLocalTeam && (*iter).nTeam[1] != nLocalTeam)
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDiscMatchMgr::GetTeamSound
//
// ----------------------------------------------------------------------- //

const char* CDiscMatchMgr::GetTeamSound(int nBase, int nTeam)
{
	// RETAIL: teams 6 and 7 have each other's lines
	if (nTeam == 6)
	{
		nTeam = 7;
	}
	else if (nTeam == 7)
	{
		nTeam = 6;
	}

	if (g_pGameClientShell->GetInterfaceMgr()->GetTeamMgr()->GetTeam(0, false)->GetMaxClients() > 1)
	{
		sprintf(s_szTeamSound, "voice\\%d.wav", nBase + nTeam);
		return s_szTeamSound;
	}

	return LTNULL;
}
