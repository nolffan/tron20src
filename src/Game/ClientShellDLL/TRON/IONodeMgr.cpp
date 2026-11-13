// ----------------------------------------------------------------------- //
//
// MODULE  : IONodeMgr.cpp
//
// PURPOSE : The pings: the characters (I/O nodes) that call the player
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1003f590..
// 0x1003feeb; IONodeMgr.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "IONodeMgr.h"
#include "InterfaceMgr.h"
#include "ClientSoundMgr.h"
#include "HUDMgr.h"
#include "HUDMessageQueue.h"

#define IONODEMGR_MAX_CHARACTERS	100

static char s_aTagName[30];

// RETAIL: inlined (the name is ours)
static inline char* CopyString(const char* szString)
{
	if (!szString) return LTNULL;

	char* szCopy = debug_newa(char, strlen(szString) + 1);
	if (szCopy)
	{
		strcpy(szCopy, szString);
	}
	return szCopy;
}

CIONodeMgr::CIONodeMgr()
{
	m_fNextPingSound = 0.0f;
}

LTBOOL CIONodeMgr::Init(const char* szAttributeFile)
{
	m_Pings.clear();
	m_Characters.clear();
	m_fNextPingSound = 0.0f;

	if (!szAttributeFile) return LTFALSE;
	if (!Parse(szAttributeFile)) return LTFALSE;

	for (int i = 0; i < IONODEMGR_MAX_CHARACTERS; i++)
	{
		sprintf(s_aTagName, "%s%d", "Character", i);
		if (m_buteMgr.Exist(s_aTagName))
		{
			IONODECHARACTER* pChar = debug_new(IONODECHARACTER);

			char szTmp[1024];
			m_buteMgr.GetString(s_aTagName, "Name", "", szTmp, sizeof(szTmp));
			pChar->szName = CopyString(szTmp);
			pChar->nNameId = (uint16)m_buteMgr.GetInt(s_aTagName, "NameID", 0);
			m_buteMgr.GetString(s_aTagName, "Portrait", "", szTmp, sizeof(szTmp));
			pChar->szPortrait = CopyString(szTmp);
			pChar->bIsCharacter = m_buteMgr.GetByte(s_aTagName, "IsCharacter");
			pChar->fStayTime = m_buteMgr.GetFloat(s_aTagName, "StayTime", 0.0f);

			m_Characters.push_back(pChar);
		}
	}

	m_buteMgr.Term();

	return LTTRUE;
}

bool CIONodeMgr::AddPing(const char* szName)
{
	if (IsMultiplayerGame()) return false;

	// a character pings once at a time
	RemovePing(szName);

	IONODECHARACTER* pChar = LTNULL;
	for (CharacterArray::iterator iter = m_Characters.begin(); iter != m_Characters.end(); iter++)
	{
		if (!_strcmpi(szName, (*iter)->szName))
		{
			pChar = *iter;
		}
	}

	if (!pChar)
	{
		g_pLTClient->CPrint("PING: no such character '%s'\n", szName);
		return false;
	}

	IONODEPING* pPing = debug_new(IONODEPING);
	pPing->szName = CopyString(szName);
	pPing->szDisplayName = CopyString(LoadTempString(pChar->nNameId));
	pPing->hPortrait = g_pInterfaceResMgr->GetTexture(pChar->szPortrait);
	pPing->fStayTime = pChar->fStayTime;
	pPing->bNew = true;

	if (!pPing->hPortrait)
	{
		debug_deletea(pPing->szName);
		debug_deletea(pPing->szDisplayName);
		debug_delete(pPing);
		return false;
	}

	m_Pings.push_back(pPing);
	g_pHUDMgr->QueueUpdate(0x20000);	// RETAIL: CHUDPings' flag

	if (g_pLTClient->GetTime() > m_fNextPingSound)
	{
		HLTSOUND hSnd = g_pClientSoundMgr->PlayInterfaceSound("interface\\snd\\newping.wav");

		LTFLOAT fDuration = 0.0f;
		g_pLTClient->SoundMgr()->GetSoundDuration(hSnd, fDuration);
		m_fNextPingSound = g_pLTClient->GetTime() + fDuration;
	}

	if (pChar->bIsCharacter)
	{
		char szMsg[1024];
		FormatString(3227, szMsg, sizeof(szMsg), pPing->szDisplayName);
		g_pChatMsgs->AddMessage(szMsg);
	}

	return true;
}

void CIONodeMgr::ClearPings()
{
	for (PingArray::iterator iter = m_Pings.begin(); iter != m_Pings.end(); iter++)
	{
		IONODEPING* pPing = *iter;
		if (pPing->szName)
		{
			debug_deletea(pPing->szName);
		}
		if (pPing->szDisplayName)
		{
			debug_deletea(pPing->szDisplayName);
		}
		debug_delete(pPing);
	}

	m_Pings.clear();
	g_pHUDMgr->QueueUpdate(0x20000);
}

bool CIONodeMgr::RemovePing(const char* szName)
{
	if (IsMultiplayerGame()) return false;

	for (PingArray::iterator iter = m_Pings.begin(); iter != m_Pings.end(); iter++)
	{
		IONODEPING* pPing = *iter;
		if (!_strcmpi(pPing->szName, szName))
		{
			if (pPing->szName)
			{
				debug_deletea(pPing->szName);
			}
			if (pPing->szDisplayName)
			{
				debug_deletea(pPing->szDisplayName);
			}
			debug_delete(pPing);

			m_Pings.erase(iter);
			g_pHUDMgr->QueueUpdate(0x20000);
			return true;
		}
	}

	return false;
}

IONODEPING* CIONodeMgr::GetPing(int nIndex)
{
	if (nIndex >= 0 && nIndex < (int)m_Pings.size())
	{
		return m_Pings[nIndex];
	}
	return LTNULL;
}

void CIONodeMgr::Save(ILTMessage_Write* pMsg)
{
	uint8 nNumPings = (uint8)m_Pings.size();
	pMsg->Writeuint8(nNumPings);
	if (!nNumPings) return;

	for (PingArray::iterator iter = m_Pings.begin(); iter != m_Pings.end(); iter++)
	{
		pMsg->WriteString((*iter)->szName);
	}
}

void CIONodeMgr::Load(ILTMessage_Read* pMsg)
{
	m_Pings.clear();

	int nNumPings = pMsg->Readuint8();
	if (!nNumPings) return;

	for (int i = 0; i < nNumPings; i++)
	{
		char szName[256];
		pMsg->ReadString(szName, sizeof(szName));
		AddPing(szName);
	}
}
