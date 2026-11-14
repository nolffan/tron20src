//-------------------------------------------------------------------------
//
// MODULE  : ProceduralCtrl.cpp
//
// PURPOSE : GUI element for interacting with procedurals
//
// CREATED : 4/3/02 - for TRON
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// RETAIL: written from retail CShell.dll 0x1004e460..0x1004f760 (see the
// header). PORT: retail's PlaySoundLocal takes one more argument (1.0f)
// than ours, so the functions playing sounds stay near.
//
//-------------------------------------------------------------------------

#include "stdafx.h"
#include "ProceduralCtrl.h"
#include "InterfaceMgr.h"
#include "InterfaceResMgr.h"
#include "LayoutMgr.h"
#include "ClientSoundMgr.h"
#include "cuipolystring.h"

extern CSubroutineMgr* g_pSubroutineMgr;

// RETAIL: 0x10217570, the condition model of a corrupted subroutine, by
// version
char * g_szOptRingModels[LAST_VERSION] =
{
	"optring_alpha",
	"optring_beta",
	"optring_gold"
};

// RETAIL: 0x1021757c (unused in retail; the name is ours)
char * g_szAddSockets[3] =
{
	"skt_add_1",
	"skt_add_2",
	"skt_add_3"
};

// RETAIL: 0x10217588, true in retail, so the procedurals' work sounds are
// never played (the name is ours)
bool g_bNoProcWorkSound = true;


//////////////////////////////////////////////////////////////////////
// COptimizerCtrl
//////////////////////////////////////////////////////////////////////

COptimizerCtrl::COptimizerCtrl()
{
	m_Center.x = 100;
	m_Center.y = 100;
	m_bInit = false;
	m_bHot = false;
	m_bBusy = false;
	m_bDone = false;
	m_HitRect.left = 0;
	m_HitRect.top = 0;
	m_HitRect.right = 0;
	m_HitRect.bottom = 0;
}

COptimizerCtrl::~COptimizerCtrl()
{
	Term();
}

bool COptimizerCtrl::Init()
{
	if (m_bInit)
		return true;

	Term();

	m_bInit = true;
	m_bBusy = false;
	m_bDone = false;
	m_bCanAccept = false;

	m_Optimizer.Create("optimizer");
	m_Optimizer.PlayAnim("liveloopmenu", true, false);
	m_Optimizer.Show(true);

	m_DeadOptimizer.Create("dead_optimizer");
	m_DeadOptimizer.PlayAnim("deadloop", true, false);
	m_DeadOptimizer.Show(false);

	m_Background.Create("optimizer_bg");
	m_Background.PlayAnim("loop", true, false);
	m_Background.Show(true);

	ScreenDimsChanged();

	return true;
}

void COptimizerCtrl::ScreenDimsChanged()
{
	if (!m_bInit)
		return;

	float fXRatio = g_pInterfaceResMgr->GetXRatio();
	float fYRatio = g_pInterfaceResMgr->GetYRatio();

	m_HitRect = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_SUBROUTINES, "OptimizerHitRect");
	m_HitRect.left = (int)(m_HitRect.left * fXRatio);
	m_HitRect.right = (int)(m_HitRect.right * fXRatio);
	m_HitRect.top = (int)(m_HitRect.top * fYRatio);
	m_HitRect.bottom = (int)(m_HitRect.bottom * fYRatio);

	m_Center.x = (m_HitRect.left + m_HitRect.right) / 2;
	m_Center.y = (m_HitRect.top + m_HitRect.bottom) / 2;
}

void COptimizerCtrl::Term()
{
	if (!m_bInit)
		return;

	if (m_bBusy)
	{
		m_Sub.Term();
		m_bBusy = false;
	}

	m_Optimizer.Term();
	m_DeadOptimizer.Term();
	m_Background.Term();

	if (m_FXLink.IsValid())
	{
		g_pInterfaceMgr->RemoveInterfaceFX(&m_FXLink);
		m_FXLink.ClearLink();
	}

	m_bHot = false;
	m_bInit = false;
}

LTBOOL COptimizerCtrl::OnMouseMove(int x, int y, PlayerSubroutine * pCursorSub)
{
	if (!m_bInit || m_bBusy)
		return LTFALSE;

	bool bWasAccepting = m_bCanAccept;
	bool bWasHot = m_bHot;

	m_bCanAccept = CanAccept(pCursorSub);
	m_bHot = false;
	if (x >= m_HitRect.left && x <= m_HitRect.right &&
		y >= m_HitRect.top && y <= m_HitRect.bottom)
	{
		m_bHot = true;
	}

	if (m_bHot == bWasHot && m_bCanAccept == bWasAccepting)
		return LTTRUE;

	if (!m_bDone)
	{
		if (m_bHot && !bWasHot)
		{
			if (m_bCanAccept)
				m_Optimizer.PlayAnim("excitedloop", true, false);
		}
		else if (!m_bHot && bWasHot)
		{
			if (m_bCanAccept)
				m_Optimizer.PlayAnim("excitedloop_menu", true, false);
		}
		else if (m_bCanAccept)
		{
			if (!bWasAccepting)
			{
				if (m_bHot)
					m_Optimizer.PlayAnim("excitedloop", true, false);
				else
					m_Optimizer.PlayAnim("excitedloop_menu", true, false);
			}
		}
		else if (bWasAccepting)
		{
			m_Optimizer.PlayAnim("liveloopmenu", true, false);
		}
	}

	return LTTRUE;
}

bool COptimizerCtrl::CanAccept(PlayerSubroutine * pSub)
{
	if (!m_bBusy && !m_bDone && pSub && pSub->eVersion != VERSION_GOLD &&
		(pSub->eState == SUBSTATE_OKAY || pSub->eState == SUBSTATE_CORRUPT))
		return true;

	return false;
}

void COptimizerCtrl::Update()
{
	if (!m_bInit || m_bDone || !m_bBusy)
		return;

	if (!m_Optimizer.IsAnimDone())
		return;

	if (m_FXLink.IsValid())
	{
		g_pInterfaceMgr->RemoveInterfaceFX(&m_FXLink);
		m_FXLink.ClearLink();
	}

	m_bDone = true;
	m_bBusy = false;

	m_Sub.Term();
	m_Optimizer.Show(false);
	m_DeadOptimizer.Show(true);
	m_pSub = LTNULL;
}

void COptimizerCtrl::Optimize(PlayerSubroutine * pSub)
{
	if (!m_bInit || !pSub || m_bBusy || m_bDone)
		return;

	m_pSub = pSub;
	if (pSub->eVersion < VERSION_GOLD)
		pSub->eVersion = (SubVersion)(pSub->eVersion + 1);

	m_Optimizer.PlayAnim("optimize", false, false);
	g_pClientSoundMgr->PlaySoundLocal("optimize", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);

	INT_MENUPIECE * pPiece = ((CTronLayoutMgr *)g_pLayoutMgr)->GetSubroutinePiece(pSub);
	m_Sub.Create(pPiece);
	if (pPiece)
	{
		debug_delete(pPiece);
	}
	m_Sub.Attach("optimizer", "TopSckt");
	m_Sub.PlayAnim("loop", true, false);
	m_Sub.Show(true);

	AddFX();

	m_bBusy = true;
}

void COptimizerCtrl::AddFX()
{
	INT_FX * pFX = g_pLayoutMgr->GetFX("SUBS_optimize");
	if (pFX)
	{
		g_pInterfaceMgr->AddInterfaceFX(&m_FXLink, pFX->szFXName, pFX->vPos, LTTRUE);
	}
	else
	{
		m_FXLink.ClearLink();
	}
}


//////////////////////////////////////////////////////////////////////
// Arcs of text
//////////////////////////////////////////////////////////////////////

void ArcPolyString(CUIPolyString * pStr, int x, int y, float fRadius, float fAngle,
				   bool bOutside, int nJustify)
{
	if (!pStr)
		return;

	// lay the string out again, straight
	pStr->SetText(pStr->GetText());

	int32 nWidth, nHeight;
	pStr->GetDims(&nWidth, &nHeight);
	float fWidth = (float)nWidth;
	float fHeight = (float)nHeight;

	LT_POLYGT4 * pPoly = pStr->GetPolys();
	int nLen = pStr->GetLength();

	float fRad = MATH_DEGREES_TO_RADIANS(fAngle);
	float fArc = fWidth / (fRadius * MATH_CIRCLE) * MATH_CIRCLE;

	float fStart, fEnd;
	switch (nJustify)
	{
		case 0:
		{
			if (bOutside)
			{
				fStart = fArc * 0.5f + fRad;
				fEnd = fStart - fArc;
			}
			else
			{
				fStart = fRad - fArc * 0.5f;
				fEnd = fStart + fArc;
			}
		}
		break;

		case 1:
		{
			fStart = fRad;
			if (bOutside)
				fEnd = fRad - fArc;
			else
				fEnd = fRad + fArc;
		}
		break;

		case 2:
		{
			if (bOutside)
				fStart = fArc + fRad;
			else
				fStart = fRad - fArc;
			fEnd = fRad;
		}
		break;
	}

	while (fStart < 0.0f || fEnd < 0.0f)
	{
		fStart += MATH_CIRCLE;
		fEnd += MATH_CIRCLE;
	}

	float fDone = 0.0f;
	float fX = 0.0f;
	float fDelta = fEnd - fStart;
	float fCharStart = fStart;

	for (int i = 0; i < nLen; i++)
	{
		float fCharWidth = pPoly->verts[1].x - pPoly->verts[0].x;
		fX += fCharWidth;
		fDone += fCharWidth / fWidth;
		float fCharEnd = fDelta * fDone + fStart;

		SetArcPoly(pPoly, x, y, fRadius, fHeight, fCharStart, fCharEnd, bOutside);

		fCharStart = fCharEnd;
		pPoly++;
	}
}

void SetArcPoly(LT_POLYGT4 * pPoly, int x, int y, float fRadius, float fHeight,
				float fStartAngle, float fEndAngle, bool bOutside)
{
	if (!pPoly)
		return;

	float fInner = fRadius - fHeight * 0.5f;
	float fOuter = fInner + fHeight;

	float fCosStart = (float)cos(fStartAngle);
	float fCosEnd = (float)cos(fEndAngle);
	float fSinStart = (float)sin(fStartAngle);
	float fSinEnd = (float)sin(fEndAngle);

	float fX = (float)x;
	if (bOutside)
	{
		pPoly->verts[0].x = fCosStart * fOuter + fX;
		float fY = (float)y;
		pPoly->verts[0].y = fY - fSinStart * fOuter;
		pPoly->verts[1].x = fCosEnd * fOuter + fX;
		pPoly->verts[1].y = fY - fSinEnd * fOuter;
		pPoly->verts[2].x = fCosEnd * fInner + fX;
		pPoly->verts[2].y = fY - fSinEnd * fInner;
		pPoly->verts[3].x = fCosStart * fInner + fX;
		pPoly->verts[3].y = fY - fSinStart * fInner;
	}
	else
	{
		pPoly->verts[0].x = fCosStart * fInner + fX;
		float fY = (float)y;
		pPoly->verts[0].y = fY - fSinStart * fInner;
		pPoly->verts[1].x = fCosEnd * fInner + fX;
		pPoly->verts[1].y = fY - fSinEnd * fInner;
		pPoly->verts[2].x = fCosEnd * fOuter + fX;
		pPoly->verts[2].y = fY - fSinEnd * fOuter;
		pPoly->verts[3].x = fCosStart * fOuter + fX;
		pPoly->verts[3].y = fY - fSinStart * fOuter;
	}
}


//////////////////////////////////////////////////////////////////////
// CProceduralCtrl
//////////////////////////////////////////////////////////////////////

namespace
{
	void PlayMouseOverYesSound()
	{
		g_pClientSoundMgr->PlaySoundLocal("mouseover_procedural_yes", SOUNDPRIORITY_MISC_LOW, 0, 100, 1.0f, 0);
	}

	void PlayMouseOverNoSound()
	{
		g_pClientSoundMgr->PlaySoundLocal("mouseover_procedural_no", SOUNDPRIORITY_MISC_LOW, 0, 100, 1.0f, 0);
	}
}

CProceduralCtrl::CProceduralCtrl()
{
	m_CenterPos.x = 100;
	m_CenterPos.y = 100;
	m_bInit = false;
	m_pProc = LTNULL;
	m_iSlot = -1;
	m_iRadius = 16;
	m_bCanAccept = false;
	m_bWorking = false;
	m_bHot = false;
	m_hWorkSound = LTNULL;
}

CProceduralCtrl::~CProceduralCtrl()
{
	Term();
}

bool CProceduralCtrl::Init(Procedural * pProc)
{
	Term();

	m_bInit = true;
	m_bWorking = false;
	m_pProc = pProc;
	m_iSlot = pProc->iProcSlot;

	if (pProc->bPlayerHasThis)
	{
		m_Progress.Create("proc_progress");
		m_Working.Create("proc_working");

		if (m_iSlot == 0)
		{
			m_Base.Create("proc_left_base");
			m_Progress.Attach("proc_left_base", "skt_left");
			m_Working.Attach("proc_left_base", "skt_left");
		}
		else if (m_iSlot == 1)
		{
			m_Base.Create("proc_right_base");
			m_Progress.Attach("proc_right_base", "skt_right");
			m_Working.Attach("proc_right_base", "skt_right");
		}
		else
		{
			m_Base.Create("proc_bottom_base");
			m_Progress.Attach("proc_bottom_base", "skt_bottom");
			m_Working.Attach("proc_bottom_base", "skt_bottom");
		}
		m_Base.Show(true);

		m_CenterPos.x = (int)(g_pInterfaceResMgr->GetXRatio() * pProc->CenterPos.x);
		m_CenterPos.y = (int)(g_pInterfaceResMgr->GetYRatio() * pProc->CenterPos.y);
		m_iRadius = (int)(g_pInterfaceResMgr->GetXRatio() * pProc->iRadius);

		if (m_pProc->pSub)
		{
			m_Base.PlayAnim("loop", true, false);
			m_Working.PlayAnim("still", true, false);
			m_Working.Show(true);

			int nProgress = (int)(pProc->pSub->fPercentDone * 100.0f);
			if (nProgress < 0)
				nProgress = 0;
			else if (nProgress > 100)
				nProgress = 100;

			m_Progress.PlayAnim("progress", false, true);
			m_Progress.SetAnimTime(nProgress);
			m_Progress.Show(true);

			if (m_pProc->pSub->pTronSubroutine->eFunction != FUNCTION_BADBLOCK)
			{
				if (m_pProc->eAffectState == SUBSTATE_FOREIGN)
				{
					m_Sub.Create("foreign_sub");
				}
				else
				{
					INT_MENUPIECE * pPiece = ((CTronLayoutMgr *)g_pLayoutMgr)->GetSubroutinePiece(m_pProc->pSub);
					m_Sub.Create(pPiece);
					if (pPiece)
					{
						debug_delete(pPiece);
					}
				}
				m_Sub.Attach(m_Base.GetName(), "proc_bottom");
				m_Sub.PlayAnim("loop", true, false);
				m_Sub.Show(true);

				if (m_pProc->eAffectState == SUBSTATE_CORRUPT)
				{
					m_Condition.Create(g_szOptRingModels[m_pProc->pSub->eVersion]);
					m_Condition.Attach(m_Base.GetName(), "proc_bottom");
					m_Condition.PlayAnim("loop", true, false);
					m_Condition.Show(true);
				}
			}

			if (!g_bNoProcWorkSound)
			{
				m_hWorkSound = g_pClientSoundMgr->PlaySoundLocal(m_pProc->szWorkSound, SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
			}
			m_bWorking = true;
		}

		m_bInit = true;
	}
	else
	{
		m_pProc = LTNULL;
		m_CenterPos.x = 0;
		m_CenterPos.y = 0;
		m_iRadius = 0;
	}

	return true;
}

void CProceduralCtrl::ScreenDimsChanged()
{
	Procedural * pProc = m_pProc;
	if (pProc && pProc->bPlayerHasThis)
	{
		m_CenterPos.x = (int)((float)pProc->CenterPos.x * g_pInterfaceResMgr->GetXRatio());
		m_CenterPos.y = (int)((float)pProc->CenterPos.y * g_pInterfaceResMgr->GetYRatio());
		m_iRadius = (int)((float)pProc->iRadius * g_pInterfaceResMgr->GetXRatio());
	}
	else
	{
		m_CenterPos.x = 0;
		m_CenterPos.y = 0;
		m_iRadius = 0;
	}
}

void CProceduralCtrl::Term()
{
	if (!m_bInit)
		return;

	if (m_bWorking)
	{
		// keep the subroutine's progress: it stays hooked to the procedural
		float fPercentDone = 0.0f;
		PlayerSubroutine * pSub = LTNULL;
		if (m_pProc->pSub)
		{
			fPercentDone = m_pProc->pSub->fPercentDone;
			pSub = m_pProc->pSub;
		}

		RemoveSub();

		if (pSub)
		{
			g_pSubroutineMgr->HookProcToSub(m_pProc, pSub);
			pSub->fPercentDone = fPercentDone;
		}
	}

	m_pProc = LTNULL;

	if (m_hWorkSound)
	{
		g_pLTClient->SoundMgr()->KillSoundLoop(m_hWorkSound);
		m_hWorkSound = LTNULL;
	}

	m_Base.Term();
	m_Progress.Term();
	m_Working.Term();

	m_bInit = false;
}

void CProceduralCtrl::SetHighlight(bool bHighlight)
{
	m_bHighlight = bHighlight;

	if (bHighlight)
	{
		if (!m_bCanAccept)
		{
			m_Base.PlayAnim("hl_on", false, false);
			m_Base.SetNextAnim("hl_loop");
			m_Working.PlayAnim("hl", true, false);
		}
	}
	else if (m_bCanAccept)
	{
		m_Base.PlayAnim("hl_off", false, false);
		m_Base.SetNextAnim("loop");
		m_Working.PlayAnim("still", true, false);
	}
}

LTBOOL CProceduralCtrl::OnMouseMove(int mx, int my, PlayerSubroutine * pCursorSub)
{
	if (m_pProc)
	{
		bool bWasAccepting = m_bCanAccept;
		bool bWasHot = m_bHot;

		m_bCanAccept = true;
		m_bHot = true;

		if (!pCursorSub || pCursorSub->eState != m_pProc->eAffectState)
			m_bCanAccept = false;

		if (mx < m_CenterPos.x - m_iRadius || mx > m_CenterPos.x + m_iRadius)
		{
			m_bCanAccept = false;
			m_bHot = false;
		}

		if (my < m_CenterPos.y - m_iRadius || my > m_CenterPos.y + m_iRadius)
		{
			m_bCanAccept = false;
			m_bHot = false;
		}

		if (!bWasHot && m_bHot)
		{
			if (m_bCanAccept)
				PlayMouseOverYesSound();
			else
				PlayMouseOverNoSound();
		}

		if (!m_bHighlight)
		{
			if (m_bCanAccept && !bWasAccepting)
			{
				m_Base.PlayAnim("hl_on", false, false);
				m_Base.SetNextAnim("hl_loop");
				m_Working.PlayAnim("hl", true, false);
			}
			else if (!m_bCanAccept && bWasAccepting)
			{
				m_Base.PlayAnim("hl_off", false, false);
				m_Base.SetNextAnim("loop");
				m_Working.PlayAnim("still", true, false);
			}
		}
	}

	return LTTRUE;
}

void CProceduralCtrl::RemoveSub()
{
	if (!m_pProc || !m_pProc->pSub || !m_bWorking)
		return;

	g_pSubroutineMgr->HookProcToSub(m_pProc, LTNULL);

	if (m_hWorkSound)
	{
		g_pLTClient->SoundMgr()->KillSoundLoop(m_hWorkSound);
		m_hWorkSound = LTNULL;
	}

	m_Progress.Show(false);
	m_Working.Show(false);
	m_Sub.Term();
	m_Condition.Term();

	m_bWorking = false;
}

void CProceduralCtrl::AddSub(PlayerSubroutine * pSub)
{
	if (!m_pProc || !pSub)
		return;

	pSub->fPercentDone = 0.0f;
	g_pSubroutineMgr->HookProcToSub(m_pProc, pSub);

	if (m_bWorking)
		RemoveSub();

	m_Working.Show(true);
	m_Working.PlayAnim("hl", true, false);

	int nProgress = (int)(m_pProc->pSub->fPercentDone * 100.0f);
	if (nProgress < 0)
		nProgress = 0;
	else if (nProgress > 100)
		nProgress = 100;

	m_Progress.Show(true);
	m_Progress.SetAnimTime(nProgress);

	if (pSub->eState == SUBSTATE_FOREIGN)
	{
		m_Sub.Create("foreign_sub");
	}
	else
	{
		INT_MENUPIECE * pPiece = ((CTronLayoutMgr *)g_pLayoutMgr)->GetSubroutinePiece(pSub);
		m_Sub.Create(pPiece);
		if (pPiece)
		{
			debug_delete(pPiece);
		}

		if (pSub->eState == SUBSTATE_CORRUPT)
		{
			m_Condition.Create(g_szOptRingModels[m_pProc->pSub->eVersion]);
			m_Condition.PlayAnim("loop", true, false);
			m_Condition.Attach(m_Base.GetName(), "proc_bottom");
			m_Condition.Show(true);
		}
	}

	m_Sub.PlayAnim("loop", true, false);
	m_Sub.Attach(m_Base.GetName(), "proc_bottom");
	m_Sub.Show(true);

	if (!g_bNoProcWorkSound)
	{
		m_hWorkSound = g_pClientSoundMgr->PlaySoundLocal(m_pProc->szWorkSound, SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
	}
	m_bWorking = true;
}
