// ----------------------------------------------------------------------- //
//
// MODULE  : TronMissionMgr.cpp
//
// PURPOSE : Tron's mission manager: the light cycle circuits' end
//
// RETAIL: written from the retail CShell.dll 0x100a32e0..0x100a4113 (see
// TronMissionMgr.h). The names are ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronMissionMgr.h"
#include "TronGameClientShell.h"
#include "InterfaceMgr.h"
#include "ScreenMgr.h"
#include "ProfileMgr.h"
#include "CircuitButeMgr.h"
#include "LightCycleGameDataMgr.h"
#include "TRON/LightCycleButeMgr.h"
#include "BaseScreenLightCycle.h"
#include "ClientSoundMgr.h"

// RETAIL: the cycle colours a circuit count unlocks: two circuits
// completed unlock the first row, four the second, and so on
// (0x101e9094, read at 0x101e9088 + (count / 2) * 12)
static const char* s_aszUnlockedCycles[][3] =
{
	{ "LightCycleYellow_1", "LightCycleYellow_2", "LightCycleYellow_3" },
	{ "LightCycleRed_1", "LightCycleRed_2", "LightCycleRed_3" },
	{ "SuperCycle_Yellow_1", "SuperCycle_Yellow_2", "SuperCycle_Yellow_3" },
	{ "LightCycleGreen_1", "LightCycleGreen_2", "LightCycleGreen_3" },
	{ "SuperCycle_red_1", "SuperCycle_red_2", "SuperCycle_red_3" },
};

// RETAIL: names compared without case, as LCNameEquals, but out of line
// here (0x100a4250, 0x100a42a0): file-static helpers not marked inline.
// The names are ours.
static bool NameEquals(const std::string &sName, const char *pszName)
{
	uint32 nLen = !pszName ? 0 : strlen(pszName);
	return (sName.size() == nLen && !_memicmp(sName.data(),pszName,nLen));
}

static bool NameEquals(const std::string &sName, const std::string &sOther)
{
	uint32 nLen = sName.size();
	return (nLen == sOther.size() && !_memicmp(sName.data(),sOther.data(),nLen));
}

// RETAIL: inline: whether a circuit's unlock is a power-up's name
inline bool IsPowerupName(const std::string &sName)
{
	for (int i = 0; i < kNumLCPowerupTypes; i++)
	{
		const char *pszName = g_aszLCPowerupNames[i];
		uint32 nLen = !pszName ? 0 : strlen(pszName);
		if (sName.size() == nLen && !_memicmp(sName.data(), pszName, nLen))
			return true;
	}
	return false;
}

// RETAIL: the message text's concatenation: the length of the text that
// is there is taken first (Build 69's LTStrCat without its assert)
inline void TronStrCat(char *pDest, const char *pSrc, uint32 destBytes)
{
	uint32 destLen = strlen(pDest);
	uint32 catLen = destBytes - destLen - 1;
	strncat(pDest, pSrc, catLen);
	pDest[destLen + catLen] = '\0';
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronMissionMgr::TronUnknownVirtual4
//
//	PURPOSE:	RETAIL: 0x100a32e0: a light cycle circuit is won: its
//				unlocks (power-ups, maps), its completion and the cycles
//				the completed circuits unlock go into the profile, the
//				circuit screen gets their text, and the circuit's sound
//				plays
//
// ----------------------------------------------------------------------- //

bool CTronMissionMgr::TronUnknownVirtual4( ILTMessage_Read& msg )
{
	CircuitButeMgr *pCircuitMgr = g_pTronGameClientShell->GetCircuitButeMgr();
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	CLightCycleGameData *pGameData = g_pLightCycleGameDataMgr->GetCurrent();
	StringArray &aUnlockedPowerups = pProfile->m_UnlockedPowerups;
	StringArray &aUnlockedMaps = pProfile->m_UnlockedMaps;

	StringArray aNewCycles;
	StringArray aNewPowerups;
	StringArray aNewMaps;

	Circuit &circuit = pGameData->m_Circuit;
	StringArray::iterator it, itEnd;

	CircuitStringArray::iterator iter;
	for (iter = circuit.aUnlocks.begin(); iter != circuit.aUnlocks.end(); ++iter)
	{
		if (IsPowerupName(*iter))
		{
			itEnd = aUnlockedPowerups.end();
			for (it = aUnlockedPowerups.begin(); it != itEnd; ++it)
			{
				if (NameEquals(*it, *iter))
					break;
			}

			if (it == aUnlockedPowerups.end())
			{
				aUnlockedPowerups.push_back(*iter);
				aNewPowerups.push_back(*iter);
			}
		}
		else if (pCircuitMgr->IsCircuitMap(*iter))
		{
			itEnd = aUnlockedMaps.end();
			for (it = aUnlockedMaps.begin(); it != itEnd; ++it)
			{
				if (NameEquals(*it, *iter))
					break;
			}

			if (it == aUnlockedMaps.end())
			{
				aUnlockedMaps.push_back(*iter);
				aNewMaps.push_back(*iter);
			}
		}
	}

	// the circuit's completion
	StringArray &aCompleted = pProfile->m_CompletedCircuits;
	itEnd = aCompleted.end();
	for (it = aCompleted.begin(); it != itEnd; ++it)
	{
		if (NameEquals(*it, circuit.sNameKey))
			break;
	}

	bool bNewlyCompleted = false;
	if (it == aCompleted.end())
	{
		bNewlyCompleted = true;
		aCompleted.push_back(circuit.sNameKey);

		// every second circuit completed (the training's not counted)
		// unlocks three cycles
		int nCompleted = aCompleted.size();
		itEnd = aCompleted.end();
		for (it = aCompleted.begin(); it != itEnd; ++it)
		{
			if (NameEquals(*it, "Training"))
				break;
		}
		if (it != aCompleted.end())
			nCompleted--;

		if (nCompleted > 0 && (nCompleted % 2) == 0)
		{
			const char **ppszCycles = s_aszUnlockedCycles[nCompleted / 2 - 1];
			for (int i = 0; i < 3; i++, ppszCycles++)
			{
				const char *pszCycle = *ppszCycles;
				itEnd = pProfile->m_UnlockedCycleColors.end();
				for (it = pProfile->m_UnlockedCycleColors.begin(); it != itEnd; ++it)
				{
					if (NameEquals(*it, pszCycle))
						break;
				}

				if (it == pProfile->m_UnlockedCycleColors.end())
				{
					pProfile->m_UnlockedCycleColors.push_back(pszCycle);
					aNewCycles.push_back(pszCycle);
				}
			}
		}
	}

	pProfile->Save();

	// are all the circuits (but the training) completed?
	bool bAllCompleted = true;
	int nCircuits = pCircuitMgr->GetNumCircuits();
	for (int nCircuit = 0; nCircuit < nCircuits; nCircuit++)
	{
		const Circuit &other = pCircuitMgr->GetCircuit(nCircuit);
		// RETAIL: a compare without case (0x100a37a2)
		if ((const CaselessString&)other.sNameKey == "Training")
			continue;

		itEnd = aCompleted.end();
		for (it = aCompleted.begin(); it != itEnd; ++it)
		{
			if (NameEquals(*it, other.sNameKey))
				break;
		}

		if (it == aCompleted.end())
		{
			bAllCompleted = false;
			break;
		}
	}

	// the text of the unlocks
	char szMsg[1024] = "";
	char szTmp[1024] = "";
	bool bMessage = false;

	TronStrCat(szMsg, "\n", sizeof(szMsg));
	if (circuit.nCompleteId)
	{
		FormatString(circuit.nCompleteId, szTmp, sizeof(szTmp));
		TronStrCat(szMsg, szTmp, sizeof(szMsg));
		bMessage = true;
	}
	TronStrCat(szMsg, "\n", sizeof(szMsg));
	TronStrCat(szMsg, "\n", sizeof(szMsg));

	if (aNewCycles.size())
	{
		bMessage = true;
		FormatString(0x2614, szTmp, sizeof(szTmp));
		TronStrCat(szMsg, szTmp, sizeof(szMsg));
		TronStrCat(szMsg, "\n", sizeof(szMsg));

		for (it = aNewCycles.begin(); it != aNewCycles.end(); ++it)
		{
			TronStrCat(szMsg, "     ", sizeof(szMsg));

			LIGHTCYCLECOLOR *pColor = g_pLightCycleButeMgr->GetCycleColor(it->c_str());
			uint8 nStyle, nShade;
			g_pLightCycleButeMgr->GetColorShade(pColor, nStyle, nShade);
			LIGHTCYCLESHADE *pShade = g_pLightCycleButeMgr->GetShade(nShade);

			if (nStyle == 0)
				FormatString(0x259a, szTmp, sizeof(szTmp));
			else if (nStyle == 1)
				FormatString(0x259b, szTmp, sizeof(szTmp));
			else
				szTmp[0] = '\0';

			TronStrCat(szMsg, szTmp, sizeof(szMsg));
			TronStrCat(szMsg, " ", sizeof(szMsg));
			FormatString(pShade->nNameId, szTmp, sizeof(szTmp));
			TronStrCat(szMsg, szTmp, sizeof(szMsg));
			TronStrCat(szMsg, "\n", sizeof(szMsg));
		}
		TronStrCat(szMsg, "\n", sizeof(szMsg));
	}

	if (aNewPowerups.size())
	{
		bMessage = true;
		FormatString(0x2615, szTmp, sizeof(szTmp));
		TronStrCat(szMsg, szTmp, sizeof(szMsg));
		TronStrCat(szMsg, "\n", sizeof(szMsg));

		for (it = aNewPowerups.begin(); it != aNewPowerups.end(); ++it)
		{
			TronStrCat(szMsg, "     ", sizeof(szMsg));
			FormatString(g_pLightCycleButeMgr->GetPowerupData(CaselessString(it->c_str()))->nNameId, szTmp, sizeof(szTmp));
			TronStrCat(szMsg, szTmp, sizeof(szMsg));
			TronStrCat(szMsg, "\n", sizeof(szMsg));
		}
	}
	TronStrCat(szMsg, "\n", sizeof(szMsg));

	LTGUIWiden(m_sCircuitMessage, szMsg);
	if (!bMessage)
	{
		m_sCircuitMessage = L"";
	}

	// back to the circuit screen (or the victory screen, all done)
	CScreenMgr *pScreenMgr = g_pInterfaceMgr->GetScreenMgr();
	g_pInterfaceMgr->ChangeState(GS_SCREEN);
	pScreenMgr->ClearHistory();
	g_pInterfaceMgr->RequestInterfaceSound(IS_PAGE);
	pScreenMgr->AddScreenToHistory(SCREEN_ID_MAIN);
	pScreenMgr->AddScreenToHistory(SCREEN_ID_SINGLE);

	if (bNewlyCompleted && bAllCompleted)
	{
		pScreenMgr->AddScreenToHistory(SCREEN_ID_LIGHTCYCLE_CIRCUIT);
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_LIGHTCYCLE_VICTORY);
	}
	else
	{
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_LIGHTCYCLE_CIRCUIT);
	}

	g_pClientSoundMgr->PlaySoundLocal(circuit.sCompleteSound.c_str(), SOUNDPRIORITY_PLAYER_HIGH, 0, 100, 1.0f, 0, 1.0f);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronMissionMgr::GetCircuitMessage
//
//	PURPOSE:	RETAIL: 0x100a3fd0
//
// ----------------------------------------------------------------------- //

std::wstring& CTronMissionMgr::GetCircuitMessage()
{
	return m_sCircuitMessage;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronMissionMgr::ClearCircuitMessage
//
//	PURPOSE:	RETAIL: 0x100a3fe0
//
// ----------------------------------------------------------------------- //

void CTronMissionMgr::ClearCircuitMessage()
{
	m_sCircuitMessage = L"";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronMissionMgr::TronUnknownVirtual8
//
//	PURPOSE:	RETAIL: 0x100a4080: the next map of the custom game by
//				index, or (-1) back to the custom map screen
//
// ----------------------------------------------------------------------- //

bool CTronMissionMgr::TronUnknownVirtual8( ILTMessage_Read& msg )
{
	int8 nMap = msg.Readint8();
	if (nMap != -1)
	{
		StringArray *pMaps = &g_pLightCycleGameDataMgr->GetCurrent()->m_Custom.aMaps;
		g_pMissionMgr->StartGameFromLevel((*pMaps)[nMap].c_str());
		return true;
	}

	g_pInterfaceMgr->ChangeState(GS_SCREEN);
	CScreenMgr *pScreenMgr = g_pInterfaceMgr->GetScreenMgr();
	pScreenMgr->ClearHistory();
	g_pInterfaceMgr->RequestInterfaceSound(IS_PAGE);
	pScreenMgr->AddScreenToHistory(SCREEN_ID_MAIN);
	pScreenMgr->AddScreenToHistory(SCREEN_ID_SINGLE);
	g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_LIGHTCYCLE_CUSTOMMAP);

	return true;
}
