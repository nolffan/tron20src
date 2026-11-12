// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDiscScores.cpp
//
// PURPOSE : HUDItem showing the disc games' scores
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (HUDDiscScores.h).
// Only its small virtuals are written yet.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "HUDDiscScores.h"

// RETAIL: set by CTronHUDMgr to its scores (0x1009b9f0); the name is ours
CHUDDiscScores*		g_pDiscScores = LTNULL;

// RETAIL: 0x10027680
void CHUDDiscScores::Show(bool bShow, bool bTronUnknown)
{
	if (m_bTronUnknown6D78)
	{
		m_bShow = bShow;
		m_bTronUnknown6D51 = bTronUnknown;
		Update();
	}
}

// RETAIL: 0x10028d50
bool CHUDDiscScores::IsVisible()
{
	if (m_bEnabled && m_bShow)
		return true;
	return false;
}
