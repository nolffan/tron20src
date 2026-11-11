// ----------------------------------------------------------------------- //
//
// MODULE  : BossButeMgr.cpp
//
// PURPOSE : The boss's (Thorne's) attributes
//
// RETAIL: Tron 2.0's, written from retail Object.lto (BossButeMgr.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BossButeMgr.h"
#include "ButeTools.h"

static const char* const s_aszThorneAnims[kNumThorneAnims] =
{
	"Disrupt",
	"Summon",
	"Angry",
	"CreateBall",
	"ChargeBall",
	"ThrowBall",
};

CBossButeMgr* g_pBossButeMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBossButeMgr::CBossButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CBossButeMgr::CBossButeMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBossButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CBossButeMgr::Init(const char* szAttributeFile)
{
	if (!szAttributeFile) return LTFALSE;
	if (!Parse(szAttributeFile)) return LTFALSE;

	ReadThorne();

	g_pBossButeMgr = this;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBossButeMgr::ReadThorne()
//
//	PURPOSE:	Read Thorne's attributes and animations
//
// ----------------------------------------------------------------------- //

void CBossButeMgr::ReadThorne()
{
	char szTag[] = "Thorne";

	m_Thorne.fBaseBallHealth	= CButeTools::GetValidatedDouble(m_buteMgr, szTag, "BaseBallHealth");
	m_Thorne.fAngerChance		= CButeTools::GetValidatedDouble(m_buteMgr, szTag, "AngerChance");
	m_Thorne.rngChargeTime		= CButeTools::GetValidatedRange(m_buteMgr, szTag, "ChargeTimeRange");
	m_Thorne.vFireOffset.x		= CButeTools::GetValidatedDouble(m_buteMgr, szTag, "XFireOffset");
	m_Thorne.vFireOffset.y		= CButeTools::GetValidatedDouble(m_buteMgr, szTag, "YFireOffset");
	m_Thorne.vFireOffset.z		= CButeTools::GetValidatedDouble(m_buteMgr, szTag, "ZFireOffset");

	char szBuf[128];
	CButeTools::GetValidatedString(m_buteMgr, szTag, "FireSocketName", szBuf, sizeof(szBuf));
	m_Thorne.sFireSocketName = szBuf;

	CButeTools::GetValidatedString(m_buteMgr, szTag, "HittableNode", szBuf, sizeof(szBuf));
	m_Thorne.sHittableNode = szBuf;

	// Each animation's labels: <Anim>0, <Anim>1...
	std::string sAnim;
	for (int iAnim = 0; iAnim < kNumThorneAnims; iAnim++)
	{
		for (int i = 0; ; i++)
		{
			char szLabel[128];
			sprintf(szLabel, "%s%d", s_aszThorneAnims[iAnim], i);
			if (!m_buteMgr.Exist(szTag, szLabel)) break;

			char szValue[128];
			CButeTools::GetValidatedString(m_buteMgr, szTag, szLabel, szValue, sizeof(szValue));
			sAnim = szValue;

			m_Thorne.mapAnims.insert(ThorneAttributes::AnimMap::value_type(sAnim, iAnim));
		}
	}
}
