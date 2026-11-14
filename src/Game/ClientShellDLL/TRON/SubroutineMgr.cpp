// ----------------------------------------------------------------------- //
//
// MODULE  : SubroutineMgr.cpp
//
// PURPOSE : Attribute file manager for but-related subroutine info
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

// TODO
// Set up code to create a concatenated string detailing everything about a subroutine.
// to be used in the popup box
// load and save code.


#include "stdafx.h"
#include "SubroutineMgr.h"
#include "ScreenSubroutines.h"
#include "ScreenRatings.h"
#include "InterfaceMgr.h"
#include "TronPlayerStats.h"
#include "TronMissionButeMgr.h"
#include "TronPlayerMgr.h"
#include "GameClientShell.h"

#define SUBROUTINE_TAG				"Subroutine"

#define SUBROUTINE_NAME				"Name"
#define SUBROUTINE_NAME_ID			"NameId"
#define SUBROUTINE_DESC				"DescriptionId"
#define SUBROUTINE_SPRITE			"Sprite"
#define SUBROUTINE_FUNCTION			"Function"

// defense-specific fields
#define SUBROUTINE_DEFLECTION		"Deflection"
#define SUBROUTINE_ANTIVIRUS		"Antivirus"
#define SUBROUTINE_ARMORPIECE		"ArmorPiece"

// utility-specific fields
#define SUBROUTINE_DESC_ALPHA		"DescriptionAlpha"
#define SUBROUTINE_DESC_BETA		"DescriptionBeta"
#define SUBROUTINE_DESC_GOLD		"DescriptionGold"
#define SUBROUTINE_ICONMODEL		"IconModel"
#define SUBROUTINE_ICONSKIN			"IconSkin"
#define SUBROUTINE_CURSORFX			"CursorFX"
#define SUBROUTINE_ENERGYDRAIN		"EnergyDrain"
#define SUBROUTINE_ARMORTEX			"ArmorTex"
#define SUBROUTINE_CORRUPTTEX		"CorruptTex"
#define SUBROUTINE_HUDICON			"HUDIcon"
#define SUBROUTINE_PROFILER			"profiler"
#define SUBROUTINE_DISTANCE			"Distance"
#define SUBROUTINE_CORRUPTION_TAG	"Corruption"
#define SUBROUTINE_SPREADTIME		"SpreadTime"

#define PROCEDURAL_TAG				"Procedural"

#define PROCEDURAL_NAME				"Name"
#define PROCEDURAL_NAME_ID			"NameId"
#define PROCEDURAL_DESC				"DescriptionId"
#define PROCEDURAL_EFFECT			"Effect" // corresponds to subroutine "Status"
#define PROCEDURAL_TIME				"Time" // seconds to work
#define PROCEDURAL_SLOT				"Slot" // which of the 5 empty spots does it go in?
#define PROCEDURAL_INTROFX			"IntroFX"
#define PROCEDURAL_COMPLETION		"CompletionId" // string to use when procedural is done

#define PROCEDURAL_RADIUS			"Radius"
#define PROCEDURAL_CENTERPOS		"CenterPos"

#define PROCEDURAL_IDLESKIN			"IdleSkin"
#define PROCEDURAL_WORKSKIN			"WorkSkin"
#define PROCEDURAL_CONDITIONSKIN	"ConditionSkin"
#define PROCEDURAL_PROGRESSSKIN		"ProgressSkin"
#define PROCEDURAL_VERB				"VerbId"
#define PROCEDURAL_DECISIONICON		"DecisionIcon"
#define PROCEDURAL_TOOLTIPICON		"ToolTipIcon"
#define PROCEDURAL_INDICATORICON	"IndicatorIcon"
#define PROCEDURAL_WORKSOUND		"WorkSound"
#define PROCEDURAL_ADDITIONALTIME	"AdditionalTimeRequired"

#define PRIMITIVE_TAG				"Primitive"
#define PRIMITIVE_NAME				"Name"
#define PRIMITIVE_NAME_ID			"NameId"
#define PRIMITIVE_SPRITE			"Sprite"
#define PRIMITIVE_HANDS				"hands"
           
CSubroutineMgr* g_pSubroutineMgr = LTNULL;

// RETAIL: CRes.dll's strings (the names are ours)
#define IDS_SUBROUTINE_ACQUIRED			3247	// "Acquired subroutine: <color 165 233 255>%1!s!"
#define IDS_SUBROUTINE_ACQUIRED_UNKNOWN	3249	// "Acquired unknown subroutine"

static char s_aTagName[30];
static char s_aAttName[30];

char * szRingTex[] =
{
	"Interface\\Subroutines\\Sprtex\\sub_optring_alpha.dtx",
	"Interface\\Subroutines\\Sprtex\\sub_optring_beta.dtx",
	"Interface\\Subroutines\\Sprtex\\sub_optring_gold.dtx",
};

// Default constructors
TronSubroutine::TronSubroutine()
{
	// RETAIL: 0x10091e50
	szName[0]			= 0;
	nNameId				= 0;
	nDescriptionId		= 0;
	nDescriptionAlphaId	= 0;
	nDescriptionBetaId	= 0;
	nDescriptionGoldId	= 0;
	szSprite[0]			= 0;
	szIconModel[0]		= 0;
	szIconSkin[0]		= 0;
	szCursorFX[0]		= 0;
	vEnergyDrain.Init(15.0f, 10.0f, 5.0f);
	eFunction			= FUNCTION_COMBAT;

	// Defensive-only fields
	vDeflection.Init(0.0f, 0.0f, 0.0f);
	vAntivirus.Init(0.0f, 0.0f, 0.0f);
	nArmorPiece			= -1;
	szArmorTex[0]		= 0;

	// Utility-only fields
	szHUDIcon[0]		= 0;
}

PlayerSubroutine::PlayerSubroutine()
{
	// RETAIL: 0x10091ee0
	pTronSubroutine = LTNULL;
	eVersion		= VERSION_ALPHA;
	nSlot			= -1;
	nTempSlot		= -1;
	fPercentDone	= 0.0f;
	eState			= SUBSTATE_OKAY;
	pProcedural		= LTNULL;
	nTronUnknown1C	= 0;
	fTronUnknown20	= 0.0f;
	nTronUnknown24	= 0;

	// hack for base code
	bPrevious		= false;
	bNext			= false;

	// PORT: build 69's members (retail has none of these)
	bErased			= false;
	fPercentCorrupt = 0.0f;
	bWorking		= false;
	bActive			= true;
}

Procedural::Procedural()
{
	// RETAIL: 0x10091f10
	szName[0]		= 0;
	nNameId			= 0;
	nDescriptionId	= 0;
	nVerbId			= 0;
	nCompletionId	= 0;
	iProcSlot		= 0;
	bPlayerHasThis	= false;
	vTimeRequired.Init(10.0f, 10.0f, 10.0f);
	szWorkSound[0]	= 0;
	CenterPos.x		= 320;
	CenterPos.y		= 240;
	iRadius			= 16;
	szToolTipIcon[0]	= 0;
	szDecisionIcon[0]	= 0;
	szIndicatorIcon[0]	= 0;
	eAffectState	= SUBSTATE_OKAY;
	pSub			= LTNULL;

	// PORT: build 69's members (retail has none of these)
	fTimeRequired		= 1.0f;
	szIntroFX[0]		= 0;
	szIdleSkin[0]		= 0;
	szWorkSkin[0]		= 0;
	szConditionSkin[0]	= 0;
	szProgressSkin[0]	= 0;
}

Primitive::Primitive()
{
	// RETAIL: 0x10091f80
	szName[0]		= 0;
	nNameId			= 0;
	bPlayerHasThis	= false;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// RETAIL: 0x10091f90
CSubroutineMgr::CSubroutineMgr()
{
	m_fLastUpdateTime = 0.0f;
	m_SystemMemoryArray = LTNULL;
}

// RETAIL: 0x100920c0
CSubroutineMgr::~CSubroutineMgr()
{
	Term();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::Init()
//
//	PURPOSE:	Init mgr (RETAIL: 0x10092200, slot 0)
//
// ----------------------------------------------------------------------- //

LTBOOL CSubroutineMgr::Init(const char* szAttributeFile)
{
	if (g_pSubroutineMgr)
		return LTFALSE;

	g_pSubroutineMgr = this;

	m_PrimitiveArray.clear();

	m_fTronUnknown115C = 0.0f;
	m_SystemMemoryArray = LTNULL;
	m_nTronUnknown1160 = 0;
	m_fTronUnknown11A4 = 0.0f;
	m_pTronUnknown1180 = LTNULL;
	m_nTronUnknown114C = 0;

	if (!szAttributeFile) return LTFALSE;
	if (!Parse(szAttributeFile)) return LTFALSE;

	CRect &rcSpread = m_buteMgr.GetRect(SUBROUTINE_CORRUPTION_TAG, SUBROUTINE_SPREADTIME);
	m_afCorruptionSpreadTime[0] = (float)rcSpread.left;
	m_afCorruptionSpreadTime[1] = (float)rcSpread.top;
	m_afCorruptionSpreadTime[2] = (float)rcSpread.right;
	m_afCorruptionSpreadTime[3] = (float)rcSpread.bottom;

	// PORT: here retail makes its IGuestDataMgr (0x10095df0, +0x11a8; it
	// fails Init without one) and has it read the attributes; not written yet

	LoadSubroutines();
	LoadProcedurals();

	// Load the primitives
	uint16 nNumPrimitives = 0;
	sprintf(s_aTagName, "%s0", PRIMITIVE_TAG);
	while (m_buteMgr.Exist(s_aTagName))
	{
		Primitive * pNewPrim = debug_new(Primitive);

		m_buteMgr.GetString(s_aTagName, PRIMITIVE_NAME, "", pNewPrim->szName, sizeof(pNewPrim->szName));
		pNewPrim->nNameId = (uint16)m_buteMgr.GetInt(s_aTagName, PRIMITIVE_NAME_ID, 0);
		m_buteMgr.GetString(s_aTagName, PRIMITIVE_SPRITE, "", pNewPrim->szSprite, sizeof(pNewPrim->szSprite));

		m_PrimitiveArray.push_back(pNewPrim);

		// RETAIL: the player always has his hands
		if (!_strcmpi(pNewPrim->szName, PRIMITIVE_HANDS))
			pNewPrim->bPlayerHasThis = true;

		nNumPrimitives++;
		sprintf(s_aTagName, "%s%d", PRIMITIVE_TAG, nNumPrimitives);
	}

	m_buteMgr.Term();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::LoadSubroutines()
//
//	PURPOSE:	RETAIL: 0x10092410 (the name is ours)
//
// ----------------------------------------------------------------------- //

void CSubroutineMgr::LoadSubroutines()
{
	m_PlayerSubroutineArray.clear();
	m_SubroutineArray.clear();

	for (int i = 0; i < 100; i++)
	{
		sprintf(s_aTagName, "%s%d", SUBROUTINE_TAG, i);
		if (!m_buteMgr.Exist(s_aTagName))
			continue;

		TronSubroutine* pNew = debug_new(TronSubroutine);

		m_buteMgr.GetString(s_aTagName, SUBROUTINE_NAME, "", pNew->szName, sizeof(pNew->szName));
		pNew->nNameId = (uint16)m_buteMgr.GetInt(s_aTagName, SUBROUTINE_NAME_ID, 0);
		pNew->nDescriptionId = (uint16)m_buteMgr.GetInt(s_aTagName, SUBROUTINE_DESC, 0);
		pNew->nDescriptionAlphaId = (uint16)m_buteMgr.GetInt(s_aTagName, SUBROUTINE_DESC_ALPHA, 0);
		pNew->nDescriptionBetaId = (uint16)m_buteMgr.GetInt(s_aTagName, SUBROUTINE_DESC_BETA, 0);
		pNew->nDescriptionGoldId = (uint16)m_buteMgr.GetInt(s_aTagName, SUBROUTINE_DESC_GOLD, 0);
		m_buteMgr.GetString(s_aTagName, SUBROUTINE_SPRITE, "", pNew->szSprite, sizeof(pNew->szSprite));
		m_buteMgr.GetString(s_aTagName, SUBROUTINE_ICONMODEL, "", pNew->szIconModel, sizeof(pNew->szIconModel));
		m_buteMgr.GetString(s_aTagName, SUBROUTINE_ICONSKIN, "", pNew->szIconSkin, sizeof(pNew->szIconSkin));
		m_buteMgr.GetString(s_aTagName, SUBROUTINE_CURSORFX, "", pNew->szCursorFX, sizeof(pNew->szCursorFX));
		pNew->eFunction = (SubFunction)m_buteMgr.GetInt(s_aTagName, SUBROUTINE_FUNCTION, 0);

		CAVector &vDrain = m_buteMgr.GetVector(s_aTagName, SUBROUTINE_ENERGYDRAIN);
		pNew->vEnergyDrain = LTVector((float)vDrain.Geti(), (float)vDrain.Getj(), (float)vDrain.Getk());

		if (pNew->eFunction == FUNCTION_DEFENSE)
		{
			CAVector &vDeflect = m_buteMgr.GetVector(s_aTagName, SUBROUTINE_DEFLECTION);
			pNew->vDeflection = LTVector((float)vDeflect.Geti(), (float)vDeflect.Getj(), (float)vDeflect.Getk());

			CAVector &vAnti = m_buteMgr.GetVector(s_aTagName, SUBROUTINE_ANTIVIRUS);
			pNew->vAntivirus = LTVector((float)vAnti.Geti(), (float)vAnti.Getj(), (float)vAnti.Getk());

			int iTemp = m_buteMgr.GetInt(s_aTagName, SUBROUTINE_ARMORPIECE, 0);
			if (iTemp >= 0 && iTemp <= 100)
				pNew->nArmorPiece = (uint8)iTemp;

			m_buteMgr.GetString(s_aTagName, SUBROUTINE_ARMORTEX, "", pNew->szArmorTex, sizeof(pNew->szArmorTex));
			m_buteMgr.GetString(s_aTagName, SUBROUTINE_CORRUPTTEX, "", pNew->szCorruptTex, sizeof(pNew->szCorruptTex));
		}
		else if (pNew->eFunction == FUNCTION_UTILITY)
		{
			m_buteMgr.GetString(s_aTagName, SUBROUTINE_HUDICON, "", pNew->szHUDIcon, sizeof(pNew->szHUDIcon));

			if (!_strcmpi(pNew->szName, SUBROUTINE_PROFILER))
			{
				CAVector &vDist = m_buteMgr.GetVector(s_aTagName, SUBROUTINE_DISTANCE);
				m_afProfilerDistance[0] = (float)vDist.Geti();
				m_afProfilerDistance[1] = (float)vDist.Getj();
				m_afProfilerDistance[2] = (float)vDist.Getk();
			}
		}

		m_SubroutineArray.push_back(pNew);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::LoadProcedurals()
//
//	PURPOSE:	RETAIL: 0x100927d0 (the name is ours)
//
// ----------------------------------------------------------------------- //

void CSubroutineMgr::LoadProcedurals()
{
	m_ProceduralArray.clear();

	uint16 nNumProcedurals = 0;
	sprintf(s_aTagName, "%s0", PROCEDURAL_TAG);
	while (m_buteMgr.Exist(s_aTagName))
	{
		Procedural* pNew = debug_new(Procedural);

		m_buteMgr.GetString(s_aTagName, PROCEDURAL_NAME, "", pNew->szName, sizeof(pNew->szName));
		pNew->nNameId = (uint16)m_buteMgr.GetInt(s_aTagName, PROCEDURAL_NAME_ID, 0);
		pNew->nDescriptionId = (uint16)m_buteMgr.GetInt(s_aTagName, PROCEDURAL_DESC, 0);
		pNew->nVerbId = (uint16)m_buteMgr.GetInt(s_aTagName, PROCEDURAL_VERB, 0);
		pNew->nCompletionId = (uint16)m_buteMgr.GetInt(s_aTagName, PROCEDURAL_COMPLETION, 0);

		CPoint pos = m_buteMgr.GetPoint(s_aTagName, PROCEDURAL_CENTERPOS);
		pNew->CenterPos.x = pos.x;
		pNew->CenterPos.y = pos.y;
		pNew->iRadius = (uint16)m_buteMgr.GetInt(s_aTagName, PROCEDURAL_RADIUS, 16);

		m_buteMgr.GetString(s_aTagName, PROCEDURAL_DECISIONICON,	"", pNew->szDecisionIcon,	sizeof(pNew->szDecisionIcon));
		m_buteMgr.GetString(s_aTagName, PROCEDURAL_TOOLTIPICON,		"", pNew->szToolTipIcon,	sizeof(pNew->szToolTipIcon));
		m_buteMgr.GetString(s_aTagName, PROCEDURAL_INDICATORICON,	"", pNew->szIndicatorIcon,	sizeof(pNew->szIndicatorIcon));
		m_buteMgr.GetString(s_aTagName, PROCEDURAL_WORKSOUND,		"", pNew->szWorkSound,		sizeof(pNew->szWorkSound));

		int iTemp = m_buteMgr.GetInt(s_aTagName, PROCEDURAL_EFFECT, 0);
		if (iTemp >= (int)SUBSTATE_OKAY && iTemp < (int)SUBSTATE_DELETED)
			pNew->eAffectState = (SubState)iTemp;

		CAVector &vTime = m_buteMgr.GetVector(s_aTagName, PROCEDURAL_TIME);
		pNew->vTimeRequired = LTVector((float)vTime.Geti(), (float)vTime.Getj(), (float)vTime.Getk());

		CAVector &vAddTime = m_buteMgr.GetVector(s_aTagName, PROCEDURAL_ADDITIONALTIME);
		pNew->vAdditionalTimeRequired = LTVector((float)vAddTime.Geti(), (float)vAddTime.Getj(), (float)vAddTime.Getk());

		iTemp = m_buteMgr.GetInt(s_aTagName, PROCEDURAL_SLOT, 1);
		if (iTemp >= 1 && iTemp <= 5)
			pNew->iProcSlot = iTemp;

		m_ProceduralArray.push_back(pNew);

		nNumProcedurals++;
		sprintf(s_aTagName, "%s%d", PROCEDURAL_TAG, nNumProcedurals);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CSubroutineMgr::Term()
{
    g_pSubroutineMgr = LTNULL;
	CleanArrays();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::Pause()
//
//	PURPOSE:	Note the pausing and unpausing of the game.
//
// ----------------------------------------------------------------------- //

void CSubroutineMgr::Pause(LTBOOL bPause)
{
	m_bPause = bPause;
	if (!m_bPause)
	{
		m_fLastUpdateTime = g_pLTClient->GetTime();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::GetSubroutineInSector
//
//	PURPOSE:	RETAIL: 0x100938f0 (the name is ours): the installed
//				subroutine covering the system memory sector (an alpha
//				covers three sectors, a beta two, a gold one)
//
// ----------------------------------------------------------------------- //

PlayerSubroutine* CSubroutineMgr::GetSubroutineInSector(int iSector)
{
	PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
	PlayerSubroutineArray::iterator end = m_PlayerSubroutineArray.end();

	for (; iter != end; iter++)
	{
		PlayerSubroutine *pSub = *iter;
		int nSlot = pSub->nSlot;

		if (nSlot >= 0 && nSlot < 24)
		{
			if (nSlot == iSector)
				return pSub;

			if (pSub->eVersion != VERSION_GOLD)
			{
				if ((nSlot + 1) % 24 == iSector)
					return pSub;

				if (pSub->eVersion == VERSION_ALPHA && (nSlot + 2) % 24 == iSector)
					return pSub;
			}
		}
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::Update()
//
//	PURPOSE:	Note the passage of time when the game is not paused
//				Advance the progress on any active procedurals and handle
//				completion of any procedural events
//
// ----------------------------------------------------------------------- //

void CSubroutineMgr::Update()
{
	if (m_bPause)
		return;

	m_bProceduralActive = false;

	float fNow = g_pLTClient->GetTime();

	unsigned short iVersion;
	g_pTronPlayerStats->GetJetVersion(iVersion);
	float fPlayerLevel = (float)(iVersion - (iVersion % 100)) / 100.0f;

	// advance any timers.
	float fDeltaTime = fNow - m_fLastUpdateTime;
	m_fLastUpdateTime = fNow;

	ProceduralArray::iterator iter = m_ProceduralArray.begin();
	while (iter != m_ProceduralArray.end())
	{
		Procedural *pProc = *iter;
		if (pProc->bPlayerHasThis)
		{
			PlayerSubroutine * pSub = pProc->pSub;
			if (pSub)
			{
				m_bProceduralActive = true;
				// advance the progress based upon how much the local timer has advanced
				// compute the advancement rate.  Start with the base procedural speed.
				// then reduce by 10% for each level that the player has advanced?
				float fAdjustedTime = pProc->fTimeRequired * (1.0f - (0.1f * fPlayerLevel));
				float fAdvancementPerSecond = 1.0f / fAdjustedTime;

				pSub->fPercentDone += (fAdvancementPerSecond * fDeltaTime);

				if (pSub->fPercentDone >= 1.0f)
				{
					ProceduralFinished(pProc);
				}
			}
		}
		iter++;
	}
	if (m_bProceduralActive)
	{
		g_pHUDMgr->QueueUpdate(kHUDProcedurals);
	}
	// TODO advance any corruption UNLESS being worked on by a procedural
	// TODO subtract any energy for continuously draining subroutines
	// TODO write a function for burst-style energy drainage
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::ProceduralFinished()
//
//	PURPOSE:	Handle the specific details of a procedural's completion
//
// ----------------------------------------------------------------------- //

void CSubroutineMgr::ProceduralFinished(Procedural * pProc)
{
	PlayerSubroutine * pSub = pProc->pSub;
	pSub->bWorking = false;

	// Detach the subroutine from this procedural
	pProc->pSub = LTNULL;

	switch (pProc->eAffectState)
	{
	case SUBSTATE_FOREIGN:			// port
		{
			pSub->eState = SUBSTATE_OKAY;
			// put up a message that this subroutine has been successfully ported
		}
		break;

	case SUBSTATE_CORRUPT:			// disinfect
		{
			pSub->eState = SUBSTATE_OKAY;
			pSub->fPercentCorrupt = 0.0f;
			// put up a message that this subroutine has been successfully disinfected
		}
		break;

	case SUBSTATE_UNUSABLE:			// bad block
		{
			g_pLTClient->CPrint("Bad Block Removed\n");
			// REMOVE the subroutine from system memory
			PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
			while (iter != m_PlayerSubroutineArray.end())
			{
				if (*iter == pSub)
				{
					m_PlayerSubroutineArray.erase(iter);
					break;
				}
				++iter;
			}
			// put up a message that this block has been repaired
		}
		break;

	case SUBSTATE_DELETED:			// undeleter
		{
			pSub->eState = SUBSTATE_OKAY;
			// put up a message that this subroutine has been restored
		}
		break;

	case SUBSTATE_OKAY:				// this was a "shredder", and should never happen
	default:
		break;
	}
}

void CSubroutineMgr::CleanArrays()
{
	SubroutineArray::iterator iter = m_SubroutineArray.begin();

	while (iter != m_SubroutineArray.end())
	{
		debug_delete(*iter);
		iter++;
	}

	m_SubroutineArray.clear();

	PlayerSubroutineArray::iterator iter2 = m_PlayerSubroutineArray.begin();

	while (iter2 != m_PlayerSubroutineArray.end())
	{
		debug_delete(*iter2);
		iter2++;
	}

	m_PlayerSubroutineArray.clear();

	ProceduralArray::iterator iter3 = m_ProceduralArray.begin();

	while (iter3 != m_ProceduralArray.end())
	{
		debug_delete(*iter3);
		iter3++;
	}

	m_ProceduralArray.clear();

	PrimitiveArray::iterator iter4 = m_PrimitiveArray.begin();

	while (iter4 != m_PrimitiveArray.end())
	{
		debug_delete(*iter4);
		iter4++;
	}
}

TronSubroutine* CSubroutineMgr::GetSubroutine(const char *pszName)
{
	if (!pszName || !pszName[0]) return LTNULL;
	SubroutineArray::iterator iter = m_SubroutineArray.begin();
	while (iter != m_SubroutineArray.end() && stricmp(pszName,(*iter)->szName) != 0)
	{
		iter++;
	}

	if (iter != m_SubroutineArray.end())
		return (*iter);

	return LTNULL;
}

// RETAIL: 0x10094720 (the name is ours)
TronSubroutine* CSubroutineMgr::GetSubroutineByNameId(uint32 nNameId)
{
	if (nNameId)
	{
		SubroutineArray::iterator iter = m_SubroutineArray.begin();
		while (iter != m_SubroutineArray.end() && (*iter)->nNameId != nNameId)
			iter++;

		if (iter != m_SubroutineArray.end())
			return *iter;
	}

	return LTNULL;
}

// RETAIL: 0x10094760 (the name is ours)
Primitive* CSubroutineMgr::GetPrimitiveByNameId(uint32 nNameId)
{
	if (nNameId)
	{
		PrimitiveArray::iterator iter = m_PrimitiveArray.begin();
		while (iter != m_PrimitiveArray.end() && (*iter)->nNameId != nNameId)
			iter++;

		if (iter != m_PrimitiveArray.end())
			return *iter;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::GetPlayerSubroutine
//
//	PURPOSE:	RETAIL: 0x100947a0 (the name is ours)
//
// ----------------------------------------------------------------------- //

PlayerSubroutine* CSubroutineMgr::GetPlayerSubroutine(const char *pszName, SubState *pExcludeState)
{
	if (!pszName || !pszName[0])
		return LTNULL;

	PlayerSubroutineArray::iterator iter;
	for (iter = m_PlayerSubroutineArray.begin(); iter != m_PlayerSubroutineArray.end(); iter++)
	{
		if ((*iter)->pTronSubroutine && !stricmp(pszName, (*iter)->pTronSubroutine->szName))
		{
			if (!pExcludeState || (*iter)->eState != *pExcludeState)
				return (*iter);
		}
	}

	return LTNULL;
}

void CSubroutineMgr::ClearPlayerSubroutines()
{
	// RETAIL: 0x10094870
	PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
	while (iter != m_PlayerSubroutineArray.end())
	{
		debug_delete(*iter);
		iter++;
	}

	m_PlayerSubroutineArray.clear();
}

// RETAIL: 0x100948d0 (the name is ours): the player keeps only his hands
void CSubroutineMgr::ResetPrimitives()
{
	PrimitiveArray::iterator iter = m_PrimitiveArray.begin();
	while (iter != m_PrimitiveArray.end())
	{
		if (!_strcmpi((*iter)->szName, PRIMITIVE_HANDS))
			(*iter)->bPlayerHasThis = true;
		else
			(*iter)->bPlayerHasThis = false;
		iter++;
	}
}

// RETAIL: 0x10094920 (the name is ours)
void CSubroutineMgr::ResetProcedurals()
{
	ProceduralArray::iterator iter = m_ProceduralArray.begin();
	while (iter != m_ProceduralArray.end())
	{
		(*iter)->bPlayerHasThis = false;
		(*iter)->pSub = LTNULL;
		iter++;
	}
}

// RETAIL: 0x10094810 (the name is ours)
PlayerSubroutine* CSubroutineMgr::GetPlayerSubroutineByNameId(uint32 nNameId)
{
	PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
	while (iter != m_PlayerSubroutineArray.end())
	{
		TronSubroutine *pTronSub = (*iter)->pTronSubroutine;
		if (pTronSub && pTronSub->nNameId == nNameId)
			return *iter;
		iter++;
	}
	return LTNULL;
}

// PORT: build 69's (retail has no such function; its CScreenSubroutines
// code may have it inlined)
void CSubroutineMgr::AddPlayerSubroutine(PlayerSubroutine * pSub)
{
	// assume that all duplicates have been removed
	if (pSub)
		m_PlayerSubroutineArray.push_back(pSub);
}


// RETAIL: 0x10094950. Tron's takes the version and the state (Build 69's
// took the state and the condition), replaces the copy the player has (the
// foreign one with a foreign one, the other with any other) keeping its
// slot when the new one is no worse, and tells the player.
void CSubroutineMgr::GivePlayerSubroutine(char * name, char * version, char * state, bool bNotify)
{
	int nOldSlot = -1;
	int nOldVersion = VERSION_ALPHA;

	// Get a pointer to the subroutine template, if any
	TronSubroutine * pSub = GetSubroutine(name);
	if (pSub == LTNULL)
		return;

	// bad blocks and base code are never given
	if (pSub->eFunction == FUNCTION_BADBLOCK || pSub->eFunction == FUNCTION_BASECODE)
		return;

	bool bForeign = false;
	if (!strcmpi(state, "noncompatible") || !strcmpi(state, "foreign"))
		bForeign = true;

	// take away the copy the player has
	bool bDone = false;
	PlayerSubroutineArray::iterator playerIter = m_PlayerSubroutineArray.begin();
	while (playerIter != m_PlayerSubroutineArray.end() && !bDone)
	{
		PlayerSubroutine *pPlayerSub = *playerIter;
		if (pPlayerSub->pTronSubroutine == pSub &&
			((pPlayerSub->eState == SUBSTATE_FOREIGN && bForeign) ||
			 (pPlayerSub->eState != SUBSTATE_FOREIGN && !bForeign)))
		{
			if (pPlayerSub->pProcedural)
				pPlayerSub->pProcedural->pSub = LTNULL;

			if ((*playerIter)->nSlot >= 0 && (*playerIter)->nSlot < 100)
			{
				nOldSlot = (*playerIter)->nSlot;
				nOldVersion = (*playerIter)->eVersion;
			}

			m_PlayerSubroutineArray.erase(playerIter);
			g_pHUDMgr->QueueUpdate(0x80000);	// RETAIL: Tron's flag
			bDone = true;
		}
		else
		{
			playerIter++;
		}
	}

	// Create a new PlayerSubroutine and link it to the TronSubroutine
	PlayerSubroutine *pPlayerSub = debug_new(PlayerSubroutine);
	pPlayerSub->pTronSubroutine		= pSub;

	// Set the version based on parameters passed to this function
	pPlayerSub->eVersion			= VERSION_ALPHA;

	if (!strcmpi(version,"beta"))
		pPlayerSub->eVersion = VERSION_BETA;
	else if (!strcmpi(version, "gold"))
		pPlayerSub->eVersion = VERSION_GOLD;

	pPlayerSub->eState				= SUBSTATE_OKAY;

	if (!strcmpi(state,"infected"))
	{
		pPlayerSub->eState = SUBSTATE_CORRUPT;
		pPlayerSub->nTronUnknown1C = (LAST_VERSION - pPlayerSub->eVersion) * 3;
	}
	else if (bForeign)
	{
		pPlayerSub->eState = SUBSTATE_FOREIGN;
	}

	// back in the old copy's slot, unless it is worse
	pPlayerSub->nSlot				= -1;
	if (nOldSlot != -1 &&
		(pPlayerSub->eState == SUBSTATE_FOREIGN || pPlayerSub->eState == SUBSTATE_OKAY) &&
		pPlayerSub->eVersion >= nOldVersion)
	{
		pPlayerSub->nSlot = nOldSlot;
	}

	m_PlayerSubroutineArray.push_back(pPlayerSub);

	// send a message that this subroutine has been successfully acquired
	g_pLTClient->CPrint("Acquired %s subroutine '%s' (%s)", version, name, state);

	if (IsMultiplayerGame())
		return;

	if (bNotify)
	{
		char szMsg[1024];
		if (bForeign)
		{
			FormatString(IDS_SUBROUTINE_ACQUIRED_UNKNOWN, szMsg, sizeof(szMsg));
		}
		else
		{
			FormatString(IDS_SUBROUTINE_ACQUIRED, szMsg, sizeof(szMsg), LoadTempString(pPlayerSub->pTronSubroutine->nNameId));
		}
		g_pPickupMsgs->AddMessage(szMsg, LTNULL);

		g_pTronPlayerStats->AddPing("subroutines");

		// RETAIL: the first of each function (combat, defense, utility)
		if (pPlayerSub->pTronSubroutine->eFunction < FUNCTION_BADBLOCK)
		{
			m_aTronUnknown119C[pPlayerSub->pTronSubroutine->eFunction] = 1;
		}
	}

	Compile();
}

void CSubroutineMgr::PopulateSubroutineScreen()
{
	// Wipe the subroutine screen
	CScreenSubroutines *pSubScreen = (CScreenSubroutines *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_SUBROUTINES);
	pSubScreen->ClearScreen();

	ProceduralArray::iterator iter = m_ProceduralArray.begin();
	while (iter != m_ProceduralArray.end())
	{
			pSubScreen->AddProcedural(*iter);
		iter++;
	}

	if (m_PlayerSubroutineArray.size())
	{
		PlayerSubroutineArray::iterator playerIter = m_PlayerSubroutineArray.begin();
		while (playerIter != m_PlayerSubroutineArray.end())
		{
			pSubScreen->AddSubroutine(*playerIter);
			playerIter++;
		}
	}
}

bool CSubroutineMgr::IsUtilitySubroutineActive(char * name)
{
	// RETAIL: 0x10094e10: an installed subroutine of that name
	PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
	while (iter != m_PlayerSubroutineArray.end())
	{
		PlayerSubroutine * pSub = *iter;
		// Look at installed subs
		if (pSub->nSlot >= 0 && pSub->nSlot < 100)
		{
			// look for a name match.  Should only be one.
			if (!_strcmpi(name, pSub->pTronSubroutine->szName))
				return true;
		}
		iter++;
	}
	return false;
}

// RETAIL: 0x10094e70 (the name is ours): whether the player's copy of the
// subroutine is installed
bool CSubroutineMgr::IsSubroutineInstalled(TronSubroutine *pTronSub)
{
	PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
	while (iter != m_PlayerSubroutineArray.end())
	{
		PlayerSubroutine * pSub = *iter;
		if (pSub->pTronSubroutine == pTronSub)
		{
			if (pSub->nSlot < 0 || pSub->nSlot >= 100)
				return false;
			return true;
		}
		iter++;
	}
	return false;
}

SubVersion CSubroutineMgr::GetActiveSubroutineVersion(char * name)
{
	// RETAIL: 0x10094eb0
	PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
	while (iter != m_PlayerSubroutineArray.end())
	{
		PlayerSubroutine * pSub = *iter;
		// Look at installed subs
		if (pSub->nSlot >= 0 && pSub->nSlot < 100)
		{
			// look for a name match.  Should only be one.
			if (!_strcmpi(name, pSub->pTronSubroutine->szName))
				return (pSub->eVersion);
		}
		iter++;
	}
	return VERSION_ALPHA;
}

// RETAIL: 0x10094f10 (the name is ours): how far the corruption has got
// through the subroutine (0..1): its whole steps of (3 - version) * 3, and
// the part of the current step's spread time gone
float CSubroutineMgr::GetSubroutineCorruption(PlayerSubroutine *pSub)
{
	if (!pSub)
		return 0.0f;

	int nSteps = pSub->nTronUnknown1C;
	if (!nSteps)
		return 0.0f;

	int nMaxSteps = (3 - pSub->eVersion) * 3;
	if (nSteps >= nMaxSteps)
		return 1.0f;

	float fMaxSteps = (float)nMaxSteps;

	return (float)nSteps / fMaxSteps +
		(GetCorruptionSpreadTime() - pSub->fTronUnknown20) / GetCorruptionSpreadTime() * (1.0f / fMaxSteps);
}

// RETAIL: 0x10094fb0 (the name is ours): whether the installed subroutine
// of that name is corrupt
bool CSubroutineMgr::IsSubroutineCorrupt(const char *name)
{
	if (name)
	{
		PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
		while (iter != m_PlayerSubroutineArray.end())
		{
			PlayerSubroutine * pSub = *iter;
			if (!_strcmpi(name, pSub->pTronSubroutine->szName) &&
				pSub->nSlot >= 0 && pSub->nSlot < 100)
			{
				return (pSub->eState == SUBSTATE_CORRUPT);
			}
			iter++;
		}
	}
	return false;
}

float CSubroutineMgr::GetActiveSubroutineCorruption(char * name)
{
	PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
	while (iter != m_PlayerSubroutineArray.end())
	{
		PlayerSubroutine * pSub = *iter;
		// Look at installed subs
		if (pSub->nSlot >= 0 && pSub->nSlot < 100)
		{
			// look for a name match.  Should only be one.
			if (!strcmpi(name, pSub->pTronSubroutine->szName))
			{
				if (pSub->bActive)
				{
					if (pSub->eState == SUBSTATE_CORRUPT)
					{
						return (pSub->fPercentCorrupt);
					}
					else
					{
						return 0.0f;
					}
				}
			}
		}
		iter++;
	}
	return 0.0f;
}

Procedural* CSubroutineMgr::GetProcedural(const char *pszName)
{
	if (!pszName || !pszName[0]) return LTNULL;
	ProceduralArray::iterator iter = m_ProceduralArray.begin();
	while (iter != m_ProceduralArray.end() && stricmp(pszName,(*iter)->szName) != 0)
	{
		iter++;
	}

	if (iter != m_ProceduralArray.end())
		return (*iter);

	return LTNULL;
}

void CSubroutineMgr::GivePlayerProcedural(char * name)
{
	// Get a pointer to the procedural, if any
	Procedural * pProc = GetProcedural(name);
	if (pProc == LTNULL)
		return;
	// see if we have this proc already
	if (pProc->bPlayerHasThis)
	{
		// Put up a message that the player already has this procedural
		return;
	}

	pProc->bPlayerHasThis = true;
	// Put up a message that the player has acquired the procedural
}

bool CSubroutineMgr::CanProcHookSub(Procedural *pProc, PlayerSubroutine *pSub)
{
	// Each procedural can fix one kind of effect.
	// Each subroutine can suffer from one kind of effect.
	// When we have a match, then we're good.
	if (pProc->eAffectState == pSub->eState)
		return true;

	return false;
}

void CSubroutineMgr::HookProcToSub(Procedural *pProc, PlayerSubroutine *pSub)
{
	// RETAIL: 0x100950c0
	// Clear out any previous hook
	if (pProc->pSub)
	{
		pProc->pSub->fPercentDone = 0.0f;
		pProc->pSub->pProcedural = LTNULL;
		pProc->pSub = LTNULL;
	}

	if (pSub && CanProcHookSub(pProc, pSub))
	{
		pSub->pProcedural = pProc;
		pProc->pSub = pSub;
	}
}

Procedural * CSubroutineMgr::WhoHookedToSub(PlayerSubroutine *pSub)
{
	// Iterate through the array of procs and determine who, if anyone, is hooked
	// to this sub
	ProceduralArray::iterator iter = m_ProceduralArray.begin();
	while (iter != m_ProceduralArray.end())
	{
		if ((*iter)->pSub == pSub)
			return (*iter);
		iter++;
	}
	return LTNULL;
}

void CSubroutineMgr::ComputeArmorCoverage()
{
	// RETAIL: 0x10095110: the installed defense subroutines' deflection and
	// antivirus (by version, in percent; halved while corrupt)
	float fArmorCoverage	= 0.0f;
	float fVirusCoverage	= 0.0f;

	PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
	while (iter != m_PlayerSubroutineArray.end())
	{
		PlayerSubroutine *pSub = *iter;

		// For each installed subroutine that's DEFENSE
		if (pSub->nSlot >= 0 && pSub->nSlot < 100)
		{
			TronSubroutine *pTronSub = pSub->pTronSubroutine;
			if (pTronSub->eFunction == FUNCTION_DEFENSE)
			{
				if (pSub->eState == SUBSTATE_CORRUPT)
					fArmorCoverage += (&pTronSub->vDeflection.x)[pSub->eVersion] * 0.5f;
				else
					fArmorCoverage += (&pTronSub->vDeflection.x)[pSub->eVersion];

				if (pSub->eState == SUBSTATE_CORRUPT)
					fVirusCoverage += (&pTronSub->vAntivirus.x)[pSub->eVersion] * 0.5f;
				else
					fVirusCoverage += (&pTronSub->vAntivirus.x)[pSub->eVersion];
			}
		}
		iter++;
	}

	g_pTronPlayerStats->UpdateSubroutineArmor(fArmorCoverage * 0.01f, fVirusCoverage * 0.01f);
}

PlayerSubroutine* CSubroutineMgr::HaveArmorPiece(int iPiece)
{
	PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
	while (iter != m_PlayerSubroutineArray.end())
	{
		if ((*iter)->nSlot >= 0 && (*iter)->nSlot < 100)
		{
			TronSubroutine *pTronSub = (*iter)->pTronSubroutine;
			if (pTronSub->eFunction == FUNCTION_DEFENSE)
			{
				if (pTronSub->nArmorPiece == iPiece)
				{
					return *iter;
				}
			}
		}
		iter++;
	}
	return LTNULL;
}

// RETAIL: 0x10095210 (the name is ours): the defense subroutine giving the
// armor piece
TronSubroutine* CSubroutineMgr::GetArmorSubroutine(int iPiece)
{
	SubroutineArray::iterator iter = m_SubroutineArray.begin();
	while (iter != m_SubroutineArray.end())
	{
		if ((*iter)->eFunction == FUNCTION_DEFENSE && (*iter)->nArmorPiece == iPiece)
			return *iter;
		iter++;
	}
	return LTNULL;
}

void CSubroutineMgr::GivePlayerPrimitive(char * name)
{
	if (!name)
		return;
	if (!name[0])
		return;

	PrimitiveArray::iterator iter = m_PrimitiveArray.begin();
	while (iter != m_PrimitiveArray.end())
	{
		if (!strcmpi((*iter)->szName, name))
		{
			if ((*iter)->bPlayerHasThis)
			{
				g_pLTClient->CPrint("Player already has primitive: %s\n",(*iter)->szName);
			}
			else
			{
				(*iter)->bPlayerHasThis = true;
				g_pLTClient->CPrint("Player acquired primitive: %s\n",(*iter)->szName);
			}
			return;
		}
		iter++;
	}
}

bool CSubroutineMgr::DoesPlayerHavePrimitive(char * name)
{
	// RETAIL: 0x100952e0
	PrimitiveArray::iterator iter = m_PrimitiveArray.begin();
	while (iter != m_PrimitiveArray.end())
	{
		Primitive *pPrim = *iter;
		if (!_strcmpi(pPrim->szName, name) && pPrim->bPlayerHasThis)
		{
			return true;
		}
		iter++;
	}
	return false;
}

// RETAIL: 0x10095340 (the name is ours): whether the player has the named
// subroutine installed
bool CSubroutineMgr::IsSubroutineInstalled(const char *name)
{
	TronSubroutine *pTronSub = GetSubroutine(name);
	if (pTronSub)
	{
		PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
		while (iter != m_PlayerSubroutineArray.end())
		{
			if ((*iter)->pTronSubroutine == pTronSub &&
				(*iter)->nSlot >= 0 && (*iter)->nSlot < 100)
			{
				return true;
			}
			iter++;
		}
	}
	return false;
}

void CSubroutineMgr::Compile()
{
	// RETAIL: 0x10095390
	// Compute armor, armor efficiency and sent to PlayerStats
	ComputeArmorCoverage();

	// the utility subroutines' energy drain, and the player manager told
	// of each of them
	m_fTronUnknown11A4 = 0.0f;

	PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
	while (iter != m_PlayerSubroutineArray.end())
	{
		PlayerSubroutine *pSub = *iter;

		if (pSub->pTronSubroutine->eFunction == FUNCTION_UTILITY)
		{
			bool bInstalled = true;

			if (pSub->nSlot >= 0 && pSub->nSlot <= 24)
			{
				if (pSub->eVersion == VERSION_ALPHA)
					m_fTronUnknown11A4 += pSub->pTronSubroutine->vEnergyDrain.x;
				else if (pSub->eVersion == VERSION_BETA)
					m_fTronUnknown11A4 += pSub->pTronSubroutine->vEnergyDrain.y;
				else if (pSub->eVersion == VERSION_GOLD)
					m_fTronUnknown11A4 += pSub->pTronSubroutine->vEnergyDrain.z;
			}
			else
			{
				bInstalled = false;
			}

			g_pHUDMgr->QueueUpdate(0x80000);	// RETAIL: Tron's flag

			if (pSub->eState != SUBSTATE_FOREIGN)
			{
				g_pTronPlayerMgr->OnSubroutineChanged(pSub->pTronSubroutine->szName, bInstalled,
					pSub->eState == SUBSTATE_CORRUPT, pSub->eVersion);
			}
		}

		iter++;
	}

	g_pHUDMgr->QueueUpdate(0x80000);	// RETAIL: Tron's flag
	g_pHUDMgr->QueueUpdate(kHUDProcedurals);
	g_pHUDMgr->QueueUpdate(kHUDEnergy);

	g_pTronPlayerStats->Compile();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::SetSystemMemoryConfiguration
//
//	PURPOSE:	To set the system memory configuration
//				(bad blocks, base code, etc...)
//
//	   NOTE:	The pSystemMemoryConfig is made up of:
//				'.'			= a normal block
//				'b' or 'B'	= a bad block
//				'c' or 'C'	= a base code block
//
// ----------------------------------------------------------------------- //
bool CSubroutineMgr::SetSystemMemoryConfiguration(const char* pSystemMemoryConfig, int nTronUnknown)
{
	// RETAIL: 0x100954b0
	if (nTronUnknown != -1)
		m_nTronUnknown114C = nTronUnknown;

	int iLen = 0;

	// RETAIL: the old bad blocks and base code go (not deleted: see
	// RETAIL_BUGS.md), the other subroutines are uninstalled
	PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
	while (iter != m_PlayerSubroutineArray.end())
	{
		if ((*iter)->eState == SUBSTATE_UNUSABLE)
		{
			iter = m_PlayerSubroutineArray.erase(iter);
		}
		else
		{
			(*iter)->nSlot = -1;
			iter++;
		}
	}

	if (pSystemMemoryConfig)
	{
		iLen = strlen(pSystemMemoryConfig);
		if (iLen > 24) iLen = 24;
	}

	// loop through the string and create the bad blocks.
	
	for (int i = 0; i < iLen; i++)
	{
		char c = pSystemMemoryConfig[i];

		if (c == 'b' || c == 'B')
		{
			AddBadBlock(i);
		}
		else if (c == 'C' || c == 'c')
		{
			char cPrev = pSystemMemoryConfig[(i+23) % 24];
			char cNext = pSystemMemoryConfig[(i+1) % 24];

			bool bPrev = (cPrev == 'c' || cPrev == 'C');
			bool bNext = (cNext == 'c' || cNext == 'C');

			AddBaseCodeBlock(i, bPrev, bNext);
		}
	}
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::AddBadBlock
//
//	PURPOSE:	Create a new subroutine that's just a Bad Block
// ----------------------------------------------------------------------- //

void CSubroutineMgr::AddBadBlock(int iSectorNumber)
{
	// Get a pointer to the subroutine template, if any
	TronSubroutine * pSub = GetSubroutine("bad_block");

	if (pSub == LTNULL)
		return;

	// Create a new PlayerSubroutine and link it to the TronSubroutine
	PlayerSubroutine *pPlayerSub = debug_new(PlayerSubroutine);

	pPlayerSub->pTronSubroutine		= pSub;
	pPlayerSub->eVersion			= VERSION_GOLD;
	pPlayerSub->eState				= SUBSTATE_UNUSABLE;

	int iRealSector = ((5 - iSectorNumber) + 24) % 24;
	pPlayerSub->nSlot				= iRealSector;

	m_PlayerSubroutineArray.push_back(pPlayerSub);

	// send a message that this subroutine has been successfully acquired
//	g_pLTClient->CPrint("Acquired bad block on sector (%d)", iSectorNumber);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::AddBaseCodeBlock
//
//	PURPOSE:	Create a new subroutine that's Base Code
// ----------------------------------------------------------------------- //

void CSubroutineMgr::AddBaseCodeBlock(int iSectorNumber, bool bPrev, bool bNext)
{
	// Get a pointer to the subroutine template for base code
	TronSubroutine * pSub = GetSubroutine("base_code");

	if (pSub == LTNULL)
		return;

	// Create a new PlayerSubroutine and link it to the TronSubroutine
	PlayerSubroutine *pPlayerSub = debug_new(PlayerSubroutine);

	pPlayerSub->pTronSubroutine		= pSub;
	pPlayerSub->eVersion			= VERSION_GOLD;
	pPlayerSub->eState				= SUBSTATE_UNUSABLE;

	int iRealSector = ((5 - iSectorNumber) + 24) % 24;
	pPlayerSub->nSlot				= iRealSector;

	pPlayerSub->bPrevious = bPrev;
	pPlayerSub->bNext = bNext;

	m_PlayerSubroutineArray.push_back(pPlayerSub);

	// send a message that this subroutine has been successfully acquired
//	g_pLTClient->CPrint("Acquired base code on sector (%d)", iSectorNumber);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::GetProfilerDistance
//
//	PURPOSE:	RETAIL: 0x10095730 (the name is ours): the profiler's
//				"Distance" for the player's version of it
//
// ----------------------------------------------------------------------- //

float CSubroutineMgr::GetProfilerDistance()
{
	PlayerSubroutine *pSub = GetPlayerSubroutine(SUBROUTINE_PROFILER, LTNULL);
	if (!pSub)
		return 100000.0f;

	return m_afProfilerDistance[pSub->eVersion];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::GetCorruptionSpreadTime
//
//	PURPOSE:	RETAIL: 0x10095760 (the name is ours): the time corruption
//				takes to spread a step, by difficulty (at least a second)
//
// ----------------------------------------------------------------------- //

float CSubroutineMgr::GetCorruptionSpreadTime()
{
	float fTime = m_afCorruptionSpreadTime[g_pGameClientShell->GetDifficulty()];
	if (fTime <= 1.0f)
		return 1.0f;
	return fTime;
}
