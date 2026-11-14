// ----------------------------------------------------------------------- //
//
// MODULE  : DuplicateSubroutineDlg.cpp
//
// PURPOSE : The "DUPLICATE CODE DETECTED!" box
//
// RETAIL: Tron's, written from retail CShell.dll 0x1001cb70..0x1001ea73
// (the names are ours)
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DuplicateSubroutineDlg.h"
#include "InterfaceMgr.h"
#include "SubroutineMgr.h"

// RETAIL: the string ids (CRes.dll; the names are ours)
#define IDS_DUPLICATE_TITLE		3253	// "        DUPLICATE CODE DETECTED!        "
#define IDS_DUPLICATE_BUILD		3254	// "build"
#define IDS_DUPLICATE_CONDITION	3255	// "condition"
#define IDS_DUPLICATE_KEEP		3256	// "KEEP"
#define IDS_DUPLICATE_CLEAN		3257	// "clean"
#define IDS_DUPLICATE_CORRUPT	3258	// "corrupt"
#define IDS_SUBROUTINE_ALPHA	3412	// "Alpha", "Beta", "Gold" (by version)

// RETAIL: 0x102121d0, the version rings by SubVersion (retail's table has
// the gold ring twice more after these)
static char* s_aRingTex[] =
{
	"Interface\\Subroutines\\icons\\skins\\opt_ring_alpha.dtx",
	"Interface\\Subroutines\\icons\\skins\\opt_ring_beta.dtx",
	"Interface\\Subroutines\\icons\\skins\\opt_ring_gold.dtx",
	"Interface\\Subroutines\\icons\\skins\\opt_ring_gold.dtx",
	"Interface\\Subroutines\\icons\\skins\\opt_ring_gold.dtx",
};

// RETAIL: 0x1001cb70
CDuplicateSubroutineDlg::CDuplicateSubroutineDlg()
{
	m_bInit = false;
}

// RETAIL: 0x1001ccc0
CDuplicateSubroutineDlg::~CDuplicateSubroutineDlg()
{
	Term();
}

// RETAIL: 0x1001cd20
void CDuplicateSubroutineDlg::Init()
{
	if (m_bInit)
		return;

	m_bVisible = false;
	m_bInit = true;
	m_pSecond = LTNULL;
	m_pFirst = LTNULL;

	m_pFont = g_pInterfaceResMgr->GetFont(0);
	float fScale = g_pInterfaceResMgr->GetYRatio();

	m_pTitle = g_pFontManager->CreatePolyString(m_pFont, LoadTempWString(IDS_DUPLICATE_TITLE));
	m_pTitle->SetColor(0xff4081f3);
	m_pTitle->SetCharScreenHeight((uint32)(fScale * 14.0f));

	int w, h;
	m_pBuild = g_pFontManager->CreatePolyString(m_pFont, LoadTempWString(IDS_DUPLICATE_BUILD));
	m_pBuild->SetColor(0xff5aafff);
	m_pBuild->SetCharScreenHeight((uint32)(fScale * 16.0f));
	m_pBuild->GetDims(&w, &h);
	m_fBuildW = (float)w;
	m_fBuildH = (float)h;

	m_pCondition = g_pFontManager->CreatePolyString(m_pFont, LoadTempWString(IDS_DUPLICATE_CONDITION));
	m_pCondition->SetColor(0xff5aafff);
	m_pCondition->SetCharScreenHeight((uint32)(fScale * 16.0f));
	m_pCondition->GetDims(&w, &h);
	m_fConditionW = (float)w;
	m_fConditionH = (float)h;

	m_pFirstKeepStr = g_pFontManager->CreatePolyString(m_pFont, LoadTempWString(IDS_DUPLICATE_KEEP));
	m_pFirstKeepStr->SetColor(0xff5aafff);
	m_pFirstKeepStr->SetCharScreenHeight((uint32)(fScale * 16.0f));
	m_pFirstKeepStr->GetDims(&w, &h);
	m_fKeepW = (float)w;
	m_fKeepH = (float)h;

	m_pSecondKeepStr = g_pFontManager->CreatePolyString(m_pFont, LoadTempWString(IDS_DUPLICATE_KEEP));
	m_pSecondKeepStr->SetColor(0xff5aafff);
	m_pSecondKeepStr->SetCharScreenHeight((uint32)(fScale * 16.0f));

	m_pName = g_pFontManager->CreatePolyString(m_pFont, std::wstring(L" "));
	m_pName->SetColor(0xffffffff);
	m_pName->SetCharScreenHeight((uint32)(fScale * 18.0f));

	m_pFirstVersion = g_pFontManager->CreateFormattedPolyString(m_pFont, std::wstring(L" "));
	m_pFirstVersion->SetColor(0xff5aafff);
	m_pFirstVersion->SetCharScreenHeight((uint32)(fScale * 16.0f));

	m_pSecondVersion = g_pFontManager->CreateFormattedPolyString(m_pFont, std::wstring(L" "));
	m_pSecondVersion->SetColor(0xff5aafff);
	m_pSecondVersion->SetCharScreenHeight((uint32)(fScale * 16.0f));

	m_pFirstState = g_pFontManager->CreatePolyString(m_pFont, std::wstring(L" "));
	m_pFirstState->SetCharScreenHeight((uint32)(fScale * 16.0f));

	m_pSecondState = g_pFontManager->CreatePolyString(m_pFont, std::wstring(L" "));
	m_pSecondState->SetCharScreenHeight((uint32)(fScale * 16.0f));

	g_pDrawPrim->SetRGBA(&m_Back, 0xff000000);

	g_pDrawPrim->SetRGBA(&m_FirstIcon, 0xffffffff);
	g_pDrawPrim->SetUVWH(&m_FirstIcon, 0.0f, 0.0f, 1.0f, 1.0f);
	g_pDrawPrim->SetRGBA(&m_SecondIcon, 0xffffffff);
	g_pDrawPrim->SetUVWH(&m_SecondIcon, 0.0f, 0.0f, 1.0f, 1.0f);

	for (int i = 0; i < 7; i++)
	{
		g_pDrawPrim->SetRGBA(&m_FirstLines[i], 0xff004bee);
		g_pDrawPrim->SetRGBA(&m_SecondLines[i], 0xff004bee);
	}

	// the inner discs of the rings and their ends
	g_pDrawPrim->SetRGBA(&m_FirstLines[1], 0xff000000);
	g_pDrawPrim->SetRGBA(&m_SecondLines[1], 0xff000000);
	g_pDrawPrim->SetRGBA(&m_FirstLines[6], 0xff000000);
	g_pDrawPrim->SetRGBA(&m_SecondLines[6], 0xff000000);

	g_pDrawPrim->SetRGBA(&m_FirstKeep[0], 0xff004bee);
	g_pDrawPrim->SetRGBA(&m_SecondKeep[0], 0xff004bee);
	g_pDrawPrim->SetRGBA(&m_FirstKeep[1], 0xff002677);
	g_pDrawPrim->SetRGBA(&m_SecondKeep[1], 0xff002677);

	g_pDrawPrim->SetRGBA4(&m_Bars[0], 0xff000000, 0xff002474, 0xff002474, 0xff000000);
	g_pDrawPrim->SetRGBA(&m_Bars[1], 0xff002474);
	g_pDrawPrim->SetRGBA4(&m_Bars[2], 0xff002474, 0xff000000, 0xff000000, 0xff002474);
	g_pDrawPrim->SetRGBA4(&m_Bars[3], 0xff000000, 0xff002474, 0xff002474, 0xff000000);
	g_pDrawPrim->SetRGBA(&m_Bars[4], 0xff002474);
	g_pDrawPrim->SetRGBA4(&m_Bars[5], 0xff002474, 0xff000000, 0xff000000, 0xff002474);
}

// RETAIL: 0x1001d640
void CDuplicateSubroutineDlg::Term()
{
	if (!m_bInit)
		return;

	g_pFontManager->DestroyPolyString(m_pTitle);
	g_pFontManager->DestroyPolyString(m_pName);
	g_pFontManager->DestroyPolyString(m_pBuild);
	g_pFontManager->DestroyPolyString(m_pCondition);
	g_pFontManager->DestroyPolyString(m_pFirstVersion);
	g_pFontManager->DestroyPolyString(m_pSecondVersion);
	g_pFontManager->DestroyPolyString(m_pFirstState);
	g_pFontManager->DestroyPolyString(m_pSecondState);
	g_pFontManager->DestroyPolyString(m_pFirstKeepStr);
	g_pFontManager->DestroyPolyString(m_pSecondKeepStr);

	m_pTitle = LTNULL;
	m_pName = LTNULL;
	m_pBuild = LTNULL;
	m_pCondition = LTNULL;
	m_pFirstVersion = LTNULL;
	m_pSecondVersion = LTNULL;
	m_pFirstState = LTNULL;
	m_pSecondState = LTNULL;
	m_pFirstKeepStr = LTNULL;
	m_pSecondKeepStr = LTNULL;

	m_bInit = false;
}

// RETAIL: 0x1001d750: lays the box out around the two copies. The icons
// sit left and right of the screen's centre, a line runs from each ring to
// its KEEP button, the build and condition rows are between them.
void CDuplicateSubroutineDlg::Show(PlayerSubroutine* pFirst, PlayerSubroutine* pSecond)
{
	if (m_bVisible)
		Hide();

	m_pFirst = pFirst;
	m_pSecond = pSecond;

	if (!pFirst || !pSecond)
		return;

	m_hIcon = g_pInterfaceResMgr->GetTexture(pFirst->pTronSubroutine->szSprite);
	m_hFirstRing = g_pInterfaceResMgr->GetTexture(s_aRingTex[pFirst->eVersion]);
	m_hSecondRing = g_pInterfaceResMgr->GetTexture(s_aRingTex[pSecond->eVersion]);

	int w, h;
	m_pName->SetText(LoadTempWString(pFirst->pTronSubroutine->nNameId));
	m_pName->GetDims(&w, &h);
	float fNameW = (float)w;

	m_pFirstVersion->SetText(LoadTempWString(pFirst->eVersion + IDS_SUBROUTINE_ALPHA));
	m_pFirstVersion->GetDims(&w, &h);
	float fFirstVersionW = (float)w;

	m_pSecondVersion->SetText(LoadTempWString(pSecond->eVersion + IDS_SUBROUTINE_ALPHA));
	m_pSecondVersion->GetDims(&w, &h);
	float fSecondVersionW = (float)w;

	m_pFirstState->SetText(LoadTempWString(pFirst->eState == SUBSTATE_CORRUPT ? IDS_DUPLICATE_CORRUPT : IDS_DUPLICATE_CLEAN));
	m_pFirstState->SetColor(pFirst->eState == SUBSTATE_CORRUPT ? 0xffc5f925 : 0xff5aafff);
	m_pFirstState->GetDims(&w, &h);
	float fFirstStateW = (float)w;

	m_pSecondState->SetText(LoadTempWString(pSecond->eState == SUBSTATE_CORRUPT ? IDS_DUPLICATE_CORRUPT : IDS_DUPLICATE_CLEAN));
	m_pSecondState->SetColor(pSecond->eState == SUBSTATE_CORRUPT ? 0xffc5f925 : 0xff5aafff);
	m_pSecondState->GetDims(&w, &h);
	float fSecondStateW = (float)w;

	// the widest of the strings under each icon
	float fMaxW = 50.0f;
	if (m_fKeepW > fMaxW)
		fMaxW = m_fKeepW;
	if (fFirstVersionW > fMaxW)
		fMaxW = fFirstVersionW;
	if (fSecondVersionW > fMaxW)
		fMaxW = fSecondVersionW;
	if (fFirstStateW > fMaxW)
		fMaxW = fFirstStateW;
	if (fSecondStateW > fMaxW)
		fMaxW = fSecondStateW;

	float fHalfW = (fMaxW + 64.0f) * 0.5f;
	float fLabelW = m_fBuildW;
	if (fLabelW < m_fConditionW)
		fLabelW = m_fConditionW;
	float fLabelHalfW = fLabelW * 0.5f;
	float fOffset = fLabelHalfW + fHalfW;

	float fCenterX = g_pInterfaceResMgr->GetXRatio() * 320.0f;
	float fTop = g_pInterfaceResMgr->GetYRatio() * 140.0f;

	m_pTitle->GetDims(&w, &h);
	m_pTitle->SetPosition((int)(fCenterX - (float)w * 0.5f), (int)fTop);
	float fTitleW = (float)w;

	float fY = fTop + 2.0f * (float)h;
	m_pName->SetPosition((int)(fCenterX - fNameW * 0.5f), (int)fY);
	fY += 2.0f * (float)h;

	// the first copy, on the left
	float fLeftX = fCenterX - fOffset;
	float fX = fCenterX - fOffset;
	float fIconY = fY + 16.0f;
	g_pDrawPrim->SetXYWH(&m_FirstIcon, fX - 16.0f, fY, 33.0f, 33.0f);
	float fRingY = fIconY - 25.0f;
	g_pDrawPrim->SetXYWH(&m_FirstLines[0], fX - 25.0f, fRingY, 50.0f, 50.0f);
	g_pDrawPrim->SetXYWH(&m_FirstLines[1], fX - 23.0f, fIconY - 23.0f, 46.0f, 46.0f);

	float fLineW = fCenterX - fLabelHalfW - (fLeftX + 25.0f) - 16.0f;
	float fLineEnd = fLineW + fX + 25.0f;
	g_pDrawPrim->SetXYWH(&m_FirstLines[2], fX + 25.0f, fIconY, fLineW, 2.0f);
	g_pDrawPrim->SetXYWH(&m_FirstLines[5], fLineEnd - 4.0f, fIconY - 4.0f, 10.0f, 10.0f);
	g_pDrawPrim->SetXYWH(&m_FirstLines[6], fLineEnd - 2.0f, fIconY - 2.0f, 6.0f, 6.0f);

	// the second, on the right
	float fRightX = fCenterX + fOffset;
	fX = fCenterX + fOffset;
	g_pDrawPrim->SetXYWH(&m_SecondIcon, fX - 16.0f, fY, 33.0f, 33.0f);
	g_pDrawPrim->SetXYWH(&m_SecondLines[0], fX - 25.0f, fRingY, 50.0f, 50.0f);
	g_pDrawPrim->SetXYWH(&m_SecondLines[1], fX - 23.0f, fIconY - 23.0f, 46.0f, 46.0f);
	fX = fX - 25.0f - fLineW;
	g_pDrawPrim->SetXYWH(&m_SecondLines[2], fX, fIconY, fLineW, 2.0f);
	g_pDrawPrim->SetXYWH(&m_SecondLines[5], fX - 4.0f, fIconY - 4.0f, 10.0f, 10.0f);
	g_pDrawPrim->SetXYWH(&m_SecondLines[6], fX - 2.0f, fIconY - 2.0f, 6.0f, 6.0f);

	// the build row
	fY += 48.0f;
	m_pBuild->SetPosition((int)(fCenterX - m_fBuildW * 0.5f), (int)fY);
	m_pFirstVersion->SetPosition((int)(fLeftX - fFirstVersionW * 0.5f), (int)fY);
	m_pSecondVersion->GetDims(&w, &h);
	m_pSecondVersion->SetPosition((int)(fRightX - fSecondVersionW * 0.5f), (int)fY);

	g_pDrawPrim->SetXYWH(&m_Bars[0], fLeftX - 25.0f - 16.0f, fY - 3.0f, (fLineEnd - 4.0f) - (fLeftX - 25.0f) + 18.0f, m_fBuildH + 6.0f);
	g_pDrawPrim->SetXYWH(&m_Bars[1], fLineEnd + 4.0f, fY - 3.0f, fX - 6.0f - fLineEnd, m_fBuildH + 6.0f);
	g_pDrawPrim->SetXYWH(&m_Bars[2], fX + 4.0f, fY - 3.0f, (fLineEnd - 4.0f) - (fLeftX - 25.0f) + 18.0f, m_fBuildH + 6.0f);

	// the condition row
	fY += (float)h * 1.5f;
	m_pCondition->SetPosition((int)(fCenterX - m_fConditionW * 0.5f), (int)fY);
	m_pFirstState->SetPosition((int)(fLeftX - fFirstStateW * 0.5f), (int)fY);
	m_pSecondState->SetPosition((int)(fRightX - fSecondStateW * 0.5f), (int)fY);

	g_pDrawPrim->SetXYWH(&m_Bars[3], fLeftX - 25.0f - 16.0f, fY - 3.0f, (fLineEnd - 4.0f) - (fLeftX - 25.0f) + 18.0f, m_fBuildH + 6.0f);
	g_pDrawPrim->SetXYWH(&m_Bars[4], fLineEnd + 4.0f, fY - 3.0f, fX - 6.0f - fLineEnd, m_fConditionH + 6.0f);
	g_pDrawPrim->SetXYWH(&m_Bars[5], fX + 4.0f, fY - 3.0f, (fLineEnd - 4.0f) - (fLeftX - 25.0f) + 18.0f, m_fBuildH + 6.0f);

	// the KEEP buttons, at the ends of the lines
	fY += (float)h * 2.5f;

	int nLeft = (int)(fLineEnd - m_fKeepW - 10.0f) + 3;
	m_rcFirstKeep.left = nLeft;
	m_rcFirstKeep.right = (int)fLineEnd + 3;
	int nTop = (int)(fY - 5.0f);
	m_rcFirstKeep.top = nTop;
	m_rcFirstKeep.bottom = (int)(fY + m_fKeepH + 5.0f);
	m_pFirstKeepStr->SetPosition((int)((float)nLeft + 5.0f), (int)((float)nTop + 5.0f));

	float fBoxX = (float)m_rcFirstKeep.left;
	float fBoxY = (float)m_rcFirstKeep.top;
	float fBoxW = (float)(m_rcFirstKeep.right - m_rcFirstKeep.left);
	float fBoxH = (float)(m_rcFirstKeep.bottom - m_rcFirstKeep.top) - 2.0f;
	g_pDrawPrim->SetXYWH(&m_FirstKeep[0], fBoxX, fBoxY, fBoxW, fBoxH);
	g_pDrawPrim->SetXYWH(&m_FirstKeep[1], fBoxX + 2.0f, fBoxY + 2.0f, fBoxW - 4.0f, fBoxH - 4.0f);

	float fLineY = fBoxY - 10.0f;
	g_pDrawPrim->SetXYWH(&m_FirstLines[3], fLineEnd, fIconY, 2.0f, fLineY - fIconY);
	g_pDrawPrim->SetXYWH(&m_FirstLines[4], fLineEnd - 14.0f, fLineY, 16.0f, 2.0f);

	nLeft = (int)fX + 1;
	m_rcSecondKeep.left = nLeft;
	m_rcSecondKeep.right = (int)(fX + m_fKeepW + 11.0f);
	m_rcSecondKeep.top = nTop;
	m_rcSecondKeep.bottom = (int)((float)h + fY + 5.0f);
	m_pSecondKeepStr->SetPosition((int)((float)nLeft + 5.0f), (int)((float)nTop + 5.0f));

	fBoxX = (float)m_rcSecondKeep.left;
	fBoxY = (float)m_rcSecondKeep.top;
	fBoxW = (float)(m_rcSecondKeep.right - m_rcSecondKeep.left);
	fBoxH = (float)(m_rcSecondKeep.bottom - m_rcSecondKeep.top) - 2.0f;
	g_pDrawPrim->SetXYWH(&m_SecondKeep[0], fBoxX, fBoxY, fBoxW, fBoxH);
	g_pDrawPrim->SetXYWH(&m_SecondKeep[1], fBoxX + 2.0f, fBoxY + 2.0f, fBoxW - 4.0f, fBoxH - 4.0f);

	fLineY = fBoxY - 10.0f;
	g_pDrawPrim->SetXYWH(&m_SecondLines[3], fX, fIconY, 2.0f, fLineY - fIconY);
	g_pDrawPrim->SetXYWH(&m_SecondLines[4], fX, fLineY, 16.0f, 2.0f);

	// the box, and its frame
	float fBackY = fTop - 20.0f;
	float fBackW = fTitleW + 40.0f;
	float fBackX = fCenterX - fBackW * 0.5f;
	float fBackH = fY + 30.0f - fTop + m_fKeepH + 16.0f;
	g_pDrawPrim->SetXYWH(&m_Back, fBackX, fBackY, fBackW, fBackH);

	float fBackR = fBackX + fBackW;
	float fBackB = fBackH + fBackY;
	m_Frame[0].Create(fBackX, fBackY, fBackR, fBackY);
	m_Frame[1].Create(fBackX, fBackB, fBackR, fBackB);
	m_Frame[2].Create(fBackX, fBackY, fBackX, fBackB);
	m_Frame[3].Create(fBackR, fBackY, fBackR, fBackB);

	m_bVisible = true;
}

// RETAIL: 0x1001e780
void CDuplicateSubroutineDlg::Hide()
{
	m_bVisible = false;
}

// RETAIL: 0x1001e790
void CDuplicateSubroutineDlg::Draw()
{
	if (!m_bVisible)
		return;

	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ);
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

	g_pDrawPrim->DrawPrim(&m_Back, 1);
	g_pDrawPrim->DrawPrim(m_FirstLines, 7);
	g_pDrawPrim->DrawPrim(m_SecondLines, 7);
	g_pDrawPrim->DrawPrim(m_FirstKeep, 2);
	g_pDrawPrim->DrawPrim(m_SecondKeep, 2);
	g_pDrawPrim->DrawPrim(m_Bars, 6);

	g_pDrawPrim->SetTexture(m_hIcon);
	g_pDrawPrim->DrawPrim(&m_FirstIcon, 1);
	g_pDrawPrim->SetTexture(m_hIcon);
	g_pDrawPrim->DrawPrim(&m_SecondIcon, 1);
	g_pDrawPrim->SetTexture(m_hFirstRing);
	g_pDrawPrim->DrawPrim(&m_FirstIcon, 1);
	g_pDrawPrim->SetTexture(m_hSecondRing);
	g_pDrawPrim->DrawPrim(&m_SecondIcon, 1);

	m_pTitle->Render();
	m_pName->Render();
	m_pBuild->Render();
	m_pCondition->Render();
	m_pFirstVersion->Render();
	m_pSecondVersion->Render();
	m_pFirstState->Render();
	m_pSecondState->Render();
	m_pFirstKeepStr->Render();
	m_pSecondKeepStr->Render();

	for (int i = 0; i < 4; i++)
	{
		m_Frame[i].Render();
	}
}

// RETAIL: 0x1001e9b0
void CDuplicateSubroutineDlg::OnMouseMove(int x, int y)
{
	bool bWasOverSecond = m_bOverSecondKeep;
	bool bWasOverFirst = m_bOverFirstKeep;
	m_bOverSecondKeep = false;
	m_bOverFirstKeep = false;

	if (x >= m_rcFirstKeep.left && x <= m_rcFirstKeep.right &&
		y >= m_rcFirstKeep.top && y < m_rcFirstKeep.bottom)
	{
		m_bOverFirstKeep = true;
	}
	else if (x >= m_rcSecondKeep.left && x <= m_rcSecondKeep.right &&
		y >= m_rcSecondKeep.top && y < m_rcSecondKeep.bottom)
	{
		m_bOverSecondKeep = true;
	}

	if (bWasOverFirst != m_bOverFirstKeep)
	{
		m_pFirstKeepStr->SetColor(m_bOverFirstKeep ? 0xffffffff : 0xff5aafff);
	}
	else if (bWasOverSecond != m_bOverSecondKeep)
	{
		m_pSecondKeepStr->SetColor(m_bOverSecondKeep ? 0xffffffff : 0xff5aafff);
	}
}

// RETAIL: 0x1001ea40
PlayerSubroutine* CDuplicateSubroutineDlg::GetKeptSubroutine(int x, int y)
{
	OnMouseMove(x, y);

	if (m_bOverFirstKeep)
		return m_pFirst;

	if (m_bOverSecondKeep)
		return m_pSecond;

	return LTNULL;
}
