// ----------------------------------------------------------------------- //
//
// MODULE  : HUDTronDecision.cpp
//
// PURPOSE : Tron's decision window: the bins of a download
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1003be20..
// 0x1003e2c0; HUDTronDecision.h). The server sends the bins (MID_DECISION,
// sub-message 1 or 3); the player takes one with COMMAND_ID_CHOOSE_1..6,
// pays its energy cost and waits while it downloads.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "HUDTronDecision.h"
#include "InterfaceMgr.h"
#include "InterfaceResMgr.h"
#include "TronInterfaceMgr.h"
#include "TronLayoutMgr.h"
#include "PlayerMgr.h"
#include "CMoveMgr.h"
#include "MsgIDs.h"
#include "CommandIDs.h"
#include "ClientUtilities.h"
#include "ClientSoundMgr.h"
#include "GameClientShell.h"
#include "SubroutineMgr.h"
#include "RatingMgr.h"
#include "TRONPlayerStats.h"

extern CGameClientShell* g_pGameClientShell;

// RETAIL: the results Tron's decision objects are sent besides a choice
#define DECISION_ABORTED		0xFF	// a new decision replaced the window
#define DECISION_OUT_OF_RANGE	0xFE	// the player walked away
#define DECISION_ALL_CHOSEN		0xFC	// no choice is left

// RETAIL: three ints of this TU's data (retail 0x1021580c, 0x10215810,
// 0x10215814). The names are ours.
static int s_nTitleHeight = 20;
static int s_nTextHeight = 20;
static int s_nSmallHeight = 16;

// PORT: LTGUIWiden written out where it is used, the character appended by
// push_back (retail has the loop inline at each place in this TU). The
// name is ours.
#define WIDEN_TEXT(sOut, pText) \
	{ \
		sOut.erase(sOut.begin(), sOut.end()); \
		uint32 nLen = strlen(pText); \
		for (uint32 nChar = 0; nChar < nLen; nChar++) \
			sOut.push_back((wchar_t)(uint8)pText[nChar]); \
	}

// ----------------------------------------------------------------------- //
//	CHUDDecisionBin
// ----------------------------------------------------------------------- //

// RETAIL: 0x1003be90
CHUDDecisionBin::CHUDDecisionBin()
{
	Reset();
}

// RETAIL: 0x1003bf20
void CHUDDecisionBin::Reset()
{
	Term();

	m_bActive = false;
	m_bDownloading = false;
	m_bDone = false;
	m_pTypeStr = LTNULL;
	m_pNameStr = LTNULL;
	m_pCostStr = LTNULL;
	m_pUpgradeStr = LTNULL;
	m_pNumberStr = LTNULL;
	m_fPercent = 0.0f;
}

// RETAIL: 0x1003bf60
void CHUDDecisionBin::Term()
{
	if (m_pNumberStr)
	{
		g_pFontManager->DestroyPolyString(m_pNumberStr);
		m_pNumberStr = LTNULL;
	}
	if (m_pTypeStr)
	{
		g_pFontManager->DestroyPolyString(m_pTypeStr);
		m_pTypeStr = LTNULL;
	}
	if (m_pNameStr)
	{
		g_pFontManager->DestroyPolyString(m_pNameStr);
		m_pNameStr = LTNULL;
	}
	if (m_pCostStr)
	{
		g_pFontManager->DestroyPolyString(m_pCostStr);
		m_pCostStr = LTNULL;
	}
	if (m_pUpgradeStr)
	{
		g_pFontManager->DestroyPolyString(m_pUpgradeStr);
		m_pUpgradeStr = LTNULL;
	}
}

// RETAIL: 0x1003c000
void CHUDDecisionBin::Init(DecisionBinData data)
{
	Reset();
	m_bActive = true;

	std::wstring sText;
	char szText[128];
	char szVersion[128];
	char szType[128];

	CUIFont* pSmallFont = g_pInterfaceResMgr->GetFontForHeight(s_nSmallHeight);
	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(s_nTextHeight);

	if (data.bWarn)
		m_Frame.Init("chooserboxwarn");
	else
		m_Frame.Init("chooserbox");

	m_fDownloadTime = g_pRatingMgr->GetTransferRate((RatingTransferType)data.eType, data.nVersion);

	m_hIcon = LTNULL;
	switch (data.eType)
	{
		case kTransfer_Primitive:
		{
			Primitive* pPrim = g_pSubroutineMgr->GetPrimitiveByNameId(data.nNameId);
			if (pPrim)
			{
				m_hIcon = g_pInterfaceResMgr->GetTexture(pPrim->szSprite);
			}
		}
		break;

		case kTransfer_Subroutine:
		{
			TronSubroutine* pSub = g_pSubroutineMgr->GetSubroutineByNameId(data.nNameId);
			if (data.bForeign)
				m_hIcon = g_pInterfaceResMgr->GetTexture("Interface\\Subroutines\\Sprtex\\foreign_0.dtx");
			else
				m_hIcon = g_pInterfaceResMgr->GetTexture(pSub->szSprite);

			sprintf(szVersion, LoadTempString(GetVersionName(data.nVersion)));
			sprintf(szType, LoadTempString(GetTypeName(kTransfer_Subroutine)));
			sprintf(szText, "%s <color 255 255 255>%s", szVersion, szType);

			WIDEN_TEXT(sText, szText);
			m_pTypeStr = g_pFontManager->CreateFormattedPolyString(pFont, sText, 0.0f, 0.0f, CUI_HALIGN_LEFT);
			m_pTypeStr->SetCharScreenHeight(s_nSmallHeight);

			if (data.bUpgrade)
			{
				FormatString(GetVersionName(data.nPlayerVersion), szVersion, sizeof(szVersion));
				FormatString(3952, szText, 256, szVersion);

				WIDEN_TEXT(sText, szText);
				m_pUpgradeStr = g_pFontManager->CreateFormattedPolyString(pFont, sText, 0.0f, 0.0f, CUI_HALIGN_LEFT);
				m_pUpgradeStr->SetColor(0xFFFFFFFF);
				m_pUpgradeStr->SetCharScreenHeight(s_nSmallHeight);
				m_fFlash = 1.0f;
			}
		}
		break;

		case kTransfer_Procedural:
		{
			m_hIcon = LTNULL;
			for (int i = 0; i < 3; i++)
			{
				Procedural* pProc = g_pSubroutineMgr->GetProcedural(i);
				if (pProc->nNameId == data.nNameId)
				{
					m_hIcon = g_pInterfaceResMgr->GetTexture(pProc->szDecisionIcon);
				}
			}
		}
		break;

		case kTransfer_Pset:
		{
			sprintf(szText, "interface\\hud\\pset_icon_%d.dtx", data.nPset);
			m_hIcon = g_pInterfaceResMgr->GetTexture(szText);

			// RETAIL: the size given is 1024, the buffer is 128 bytes
			FormatString(3953, szText, 1024, data.nPset);

			WIDEN_TEXT(sText, szText);
			m_pNameStr = g_pFontManager->CreateFormattedPolyString(pFont, sText, 0.0f, 0.0f, CUI_HALIGN_LEFT);
			m_pNameStr->SetCharScreenHeight(s_nTextHeight);
		}
		break;

		case kTransfer_Email:
		{
			m_hIcon = g_pInterfaceResMgr->GetTexture("interface\\hud\\email_icon.dtx");
			m_pNameStr = g_pFontManager->CreateFormattedPolyString(pFont, LoadTempWString(GetTypeName(kTransfer_Email)), 0.0f, 0.0f, CUI_HALIGN_LEFT);
			m_pNameStr->SetCharScreenHeight(s_nTextHeight);
		}
		break;

		case kTransfer_Custom:
		{
			m_hIcon = g_pInterfaceResMgr->GetTexture("interface\\hud\\helpfile_icon.dtx");
			m_pNameStr = g_pFontManager->CreateFormattedPolyString(pFont, LoadTempWString(GetTypeName(kTransfer_Custom)), 0.0f, 0.0f, CUI_HALIGN_LEFT);
			m_pNameStr->SetCharScreenHeight(s_nTextHeight);
		}
		break;

		case kTransfer_HelpFile:
		{
			m_hIcon = g_pInterfaceResMgr->GetTexture("interface\\hud\\videoarchive_icon.dtx");
		}
		break;
	}

	sprintf(szText, "%d", data.nNumber);
	WIDEN_TEXT(sText, szText);
	m_pNumberStr = g_pFontManager->CreateFormattedPolyString(pFont, sText, 0.0f, 0.0f, CUI_HALIGN_LEFT);
	m_pNumberStr->SetCharScreenHeight(s_nTextHeight);

	if (!m_pTypeStr)
	{
		m_pTypeStr = g_pFontManager->CreateFormattedPolyString(pSmallFont, LoadTempWString(GetTypeName(data.eType)), 0.0f, 0.0f, CUI_HALIGN_LEFT);
		m_pTypeStr->SetCharScreenHeight(s_nSmallHeight);
	}

	if (!m_pNameStr)
	{
		if (data.bForeign)
			m_pNameStr = g_pFontManager->CreateFormattedPolyString(pFont, LoadTempWString(3417), 0.0f, 0.0f, CUI_HALIGN_LEFT);
		else
			m_pNameStr = g_pFontManager->CreateFormattedPolyString(pFont, LoadTempWString(data.nNameId), 0.0f, 0.0f, CUI_HALIGN_LEFT);
		m_pNameStr->SetCharScreenHeight(s_nTextHeight);
	}

	m_nCost = data.nCost;
	if (data.nCost)
	{
		FormatString(3248, szText, sizeof(szText), data.nCost);
		WIDEN_TEXT(sText, szText);
		m_pCostStr = g_pFontManager->CreateFormattedPolyString(pFont, sText, 0.0f, 0.0f, CUI_HALIGN_LEFT);
		m_pCostStr->SetCharScreenHeight(s_nSmallHeight);
	}

	m_hProgressTex = g_pInterfaceResMgr->GetTexture("interface\\hud\\progressbar_alpha.dtx");
	m_hNumberTex = g_pInterfaceResMgr->GetTexture("interface\\hud\\chooser_insert_number_on.dtx");

	g_pDrawPrim->SetRGBA(&m_IconPoly, argbWhite);
	g_pDrawPrim->SetRGBA(&m_ProgressPoly, argbWhite);
	g_pDrawPrim->SetRGBA(&m_NumberPoly, argbWhite);
	g_pDrawPrim->SetUVWH(&m_IconPoly, 0.0f, 0.0f, 1.0f, 1.0f);
	g_pDrawPrim->SetUVWH(&m_ProgressPoly, 0.0f, 0.0f, 1.0f, 1.0f);
	g_pDrawPrim->SetUVWH(&m_NumberPoly, 0.0f, 0.0f, 1.0f, 1.0f);
}

// RETAIL: 0x1003cae0
void CHUDDecisionBin::GetSize(int &nWidth, int &nHeight)
{
	LTIntPt size = m_Frame.GetSize();
	nWidth = size.x;
	nHeight = size.y;
}

// RETAIL: 0x1003cb10
void CHUDDecisionBin::SetPosition(int x, int y)
{
	int nWidth, nHeight;
	m_pNameStr->GetDims(&nWidth, &nHeight);

	LTRect rcFrame(x, y, x + 200, y + nHeight * 3);
	m_Frame.SetRect(rcFrame);

	float fLeft = (float)x;
	float fTop = (float)y;
	float fRight = (float)rcFrame.right;
	float fBottom = (float)rcFrame.bottom;

	g_pDrawPrim->SetXYWH(&m_NumberPoly, fLeft, fTop + 2.0f, 32.0f, 32.0f);
	m_pNumberStr->SetPosition(x + 14, y + 6);

	g_pDrawPrim->SetXYWH(&m_IconPoly, fRight - 36.0f, fTop + 6.0f, 32.0f, 32.0f);

	m_pNameStr->SetPosition(x + 38, y + 6);
	m_pTypeStr->SetPosition(x + 38, nHeight + y + 6);

	if (m_pUpgradeStr)
	{
		int nTypeHeight;
		m_pTypeStr->GetDims(&nWidth, &nTypeHeight);
		m_pUpgradeStr->SetPosition(x + 38, nTypeHeight + y + nHeight + 6);
	}

	if (m_pCostStr)
	{
		int nCostWidth, nCostHeight;
		m_pCostStr->GetDims(&nCostWidth, &nCostHeight);
		m_pCostStr->SetPosition(rcFrame.right - nCostWidth - 4, y + 38);
	}

	if (m_bDownloading)
	{
		g_pDrawPrim->SetXYWH(&m_ProgressPoly, fLeft, fTop, (fRight - fLeft) * m_fPercent, fBottom - fTop);
	}
}

// RETAIL: 0x1003ccf0
void CHUDDecisionBin::Render()
{
	if (!m_bActive) return;

	m_fFlash -= g_pLTClient->GetFrameTime();
	if (m_fFlash < 0.0f)
		m_fFlash = 1.0f;

	float fAlpha;
	if (m_fFlash > 0.5f)
		fAlpha = m_fFlash;
	else
		fAlpha = 1.0f - m_fFlash;
	uint8 nAlpha = (uint8)(fAlpha * 255.0f);

	m_Frame.Render();

	if (m_bDownloading)
	{
		g_pDrawPrim->SetTexture(m_hProgressTex);
		g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_ADD);
		g_pDrawPrim->DrawPrim(&m_ProgressPoly, 1);
		g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
	}

	g_pDrawPrim->SetTexture(m_hNumberTex);
	g_pDrawPrim->DrawPrim(&m_NumberPoly, 1);
	if (m_pNumberStr)
		m_pNumberStr->Render();

	if (!m_bDone && m_hIcon)
	{
		g_pDrawPrim->SetTexture(m_hIcon);
		g_pDrawPrim->DrawPrim(&m_IconPoly, 1);
	}

	if (m_pNameStr)
		m_pNameStr->Render();

	if (m_bDone) return;

	if (m_pTypeStr)
		m_pTypeStr->Render();

	if (m_pUpgradeStr)
	{
		m_pUpgradeStr->SetColor(((uint32)nAlpha << 24) | 0x00FFFFFF);
		m_pUpgradeStr->Render();
	}

	if (m_pCostStr)
	{
		if (m_nCost > (int)g_pTronPlayerStats->GetEnergy())
			m_pCostStr->SetColor(((uint32)nAlpha << 24) | 0x00FFFFFF);
		else
			m_pCostStr->SetColor(argbWhite);
		m_pCostStr->Render();
	}
}

// RETAIL: 0x1003ced0
void CHUDDecisionBin::Update()
{
	if (!m_bDownloading) return;

	m_fElapsed += g_pGameClientShell->GetFrameTime();
	m_fPercent = m_fElapsed / m_fDownloadTime;

	if (m_fPercent >= 1.0f)
	{
		m_bDownloading = false;
		m_bDone = true;
		m_pNameStr->SetColor(0xFFB0B0B0);
	}
	else
	{
		LTRect rcFrame = m_Frame.GetRect();
		g_pDrawPrim->SetXYWH(&m_ProgressPoly, (float)rcFrame.left, (float)rcFrame.top,
			(float)(rcFrame.right - rcFrame.left) * m_fPercent, (float)(rcFrame.bottom - rcFrame.top));
	}
}

// RETAIL: 0x1003cfa0
void CHUDDecisionBin::StartDownload()
{
	m_bDownloading = true;
	m_bDone = false;
	m_fElapsed = 0.0f;
	m_fPercent = 0.0f;
}

// RETAIL: 0x1003cfb0
void CHUDDecisionBin::StopDownload()
{
	m_fPercent = 0.0f;
	m_fElapsed = 0.0f;
	m_bDownloading = false;
}

// RETAIL: 0x1003cfc0: the string of a RatingTransferType
uint32 CHUDDecisionBin::GetTypeName(int eType)
{
	switch (eType)
	{
		case kTransfer_Subroutine:	return 3423;
		case kTransfer_Procedural:	return 3424;
		case kTransfer_Primitive:	return 3425;
		case kTransfer_Pset:		return 3426;
		case kTransfer_Email:		return 3427;
		case kTransfer_Custom:		return 3434;
		case kTransfer_HelpFile:	return 3428;
	}
	return eType;
}

// RETAIL: 0x1003d030: alpha, beta or gold
uint32 CHUDDecisionBin::GetVersionName(int nVersion)
{
	switch (nVersion)
	{
		case 1:		return 3413;
		case 2:		return 3414;
		default:	return 3412;
	}
}

// ----------------------------------------------------------------------- //
//	CHUDTronDecision
// ----------------------------------------------------------------------- //

// RETAIL: 0x1003d060
CHUDTronDecision::CHUDTronDecision()
{
	for (int i = 0; i < MAX_DECISION_BINS; i++)
	{
		m_fCost[i] = 0.0f;
		m_Bins[i].Reset();
	}
	m_hDownloadSnd = LTNULL;
	m_bWarn = false;
}

// RETAIL: 0x1003d2e0
void CHUDTronDecision::Term()
{
	m_WarnFrame.Term();
	if (m_pWarnStr)
	{
		g_pFontManager->DestroyPolyString(m_pWarnStr);
	}
	m_pWarnStr = LTNULL;

	CHUDDecision::Term();
}

// RETAIL: 0x1003d320
void CHUDTronDecision::Hide(uint8 nChoice)
{
	if (m_hDownloadSnd)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hDownloadSnd);
		m_hDownloadSnd = LTNULL;
	}

	if (m_bDownloading)
	{
		g_pClientSoundMgr->PlayInterfaceSound("interface\\snd\\downloadaborted.wav");
		if (nChoice == DECISION_OUT_OF_RANGE)
		{
			g_pChatMsgs->AddMessage(3300);
		}
	}

	CHUDDecision::Hide(nChoice);
}

// RETAIL: 0x1003d3a0: sub-message 1 shows bins to pay for, 3 free ones;
// 2 is CHUDDecision's choices
void CHUDTronDecision::Show(ILTMessage_Read *pMsg)
{
	switch (pMsg->Readuint8())
	{
		case 1:
			ShowBins(pMsg, false);
			break;
		case 2:
			ShowDecision(pMsg);
			break;
		case 3:
			ShowBins(pMsg, true);
			break;
	}
}

// RETAIL: 0x1003d3f0
void CHUDTronDecision::ShowBins(ILTMessage_Read *pMsg, bool bFree)
{
	int y = 80;
	int nWidth = 200;
	int nHeight = 0;

	for (int i = 0; i < MAX_DECISION_BINS; i++)
	{
		m_Bins[i].Reset();
	}

	m_bTronUnknown4c = false;

	HOBJECT hObj = pMsg->ReadObject();
	float fRad = pMsg->Readfloat();

	LTVector vObjPos, vPos;
	g_pLTClient->GetObjectPos(hObj, &vObjPos);
	g_pLTClient->GetObjectPos(g_pPlayerMgr->GetMoveMgr()->GetObject(), &vPos);
	float fDist = (vPos - vObjPos).Length();

	//if we're out of range, bail right away
	if (fRad > 0.0f && fDist > fRad)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_OBJECT_MESSAGE);
		cMsg.WriteObject(hObj);
		cMsg.Writeuint32(MID_DECISION);
		cMsg.Writeuint8(DECISION_OUT_OF_RANGE);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

		g_pLTClient->CPrint("CHUDDecision::Show() : out of range");
		return;
	}

	//if a decision window is open and we get a new one, hide (and abort) the old one
	if (m_bVisible && m_hObject)
	{
		Hide(DECISION_ABORTED);
	}

	m_bWarn = false;
	m_hObject = hObj;
	m_hRangeObject = hObj;
	m_fRadius = fRad;

	uint8 nShow = pMsg->Readuint8();
	if (!nShow) return;

	uint8 nLayout = pMsg->Readuint8();
	if (m_nLayout != nLayout)
	{
		m_nLayout = nLayout;
		UpdateLayout();
	}

	bool bNoScan = pMsg->Readbool();

	PlayerSubroutine* pScan = g_pSubroutineMgr->GetPlayerSubroutine("virus_scan", LTNULL);
	bool bScan = false;
	if (pScan && pScan->nSlot >= 0 && pScan->nSlot < 100 && !bNoScan)
	{
		bScan = true;
	}

	int nCount = 0;
	for (i = 0; i < MAX_DECISION_BINS; i++)
	{
		DecisionBinData data;
		data.nNumber = i + 1;

		if (pMsg->Readbool())
		{
			nCount++;
			data.nNameId = pMsg->Readuint32();
			data.eType = pMsg->Readint8();

			if (data.eType == kTransfer_Subroutine)
			{
				data.nVersion = pMsg->Readint8();
				int nState = pMsg->Readint8();
				if (nState == 2)
				{
					data.bForeign = true;
				}

				if (bScan && nState == 1)
				{
					m_bWarn = true;
					if (pScan->eVersion == 1 || pScan->eVersion == 2)
					{
						data.bWarn = true;
					}
				}

				PlayerSubroutine* pSub = g_pSubroutineMgr->GetPlayerSubroutineByNameId(data.nNameId);
				if (pSub && nState != 2 && pSub->eState != 1)
				{
					data.bUpgrade = true;
					data.nPlayerVersion = pSub->eVersion;
				}
			}
			else if (data.eType == kTransfer_Pset)
			{
				data.nPset = pMsg->Readint8();
			}

			if (!bFree)
			{
				float fCost = pMsg->Readfloat();
				m_fCost[i] = fCost;
				data.nCost = (int)fCost;
			}
			else
			{
				data.nCost = 0;
				m_fCost[i] = 0.0f;
			}

			m_Bins[i].Init(data);
			m_Bins[i].SetPosition(12, y);

			int nBinWidth, nBinHeight;
			m_Bins[i].GetSize(nBinWidth, nBinHeight);
			nHeight += nBinHeight + 4;
			y += nBinHeight + 4;
		}
	}

	if (!nCount)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_OBJECT_MESSAGE);
		cMsg.WriteObject(hObj);
		cMsg.Writeuint32(MID_DECISION);
		cMsg.Writeuint8(DECISION_ALL_CHOSEN);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

		g_pLTClient->CPrint("CHUDDecision::Show() : empty bin");
		return;
	}

	// stack the bins up from above the progress box, the last lowest
	y = g_pInterfaceResMgr->GetScreenHeight() - ((CTronLayoutMgr*)g_pLayoutMgr)->GetProgressHUDOffset();
	for (i = MAX_DECISION_BINS - 1; i >= 0; i--)
	{
		CHUDDecisionBin* pBin = &m_Bins[i];
		if (pBin->m_bActive && !pBin->m_bDownloading && !pBin->m_bDone)
		{
			int nBinWidth, nBinHeight;
			pBin->GetSize(nBinWidth, nBinHeight);
			y -= nBinHeight;
			pBin->SetPosition(12, y);
			y -= 4;
		}
	}

	LTRect rcMargins = m_Frame.GetMargins();
	nWidth = 200 - rcMargins.right;
	m_Frame.SetSize(nWidth, nHeight);
	m_Frame.SetPosition(12, y);

	if (m_bWarn)
	{
		LTIntPt pos = m_Frame.GetPosition();
		int x = pos.x - m_Frame.GetMargins().left;
		int yWarn = pos.y - 36 - m_Frame.GetMargins().top;
		m_WarnFrame.SetPosition(x, yWarn);
		rcMargins = m_Frame.GetMargins();
		m_WarnFrame.SetSize(rcMargins.right + rcMargins.left + nWidth, 26);

		if (!m_pWarnStr)
		{
			m_pWarnStr = g_pFontManager->CreatePolyString(g_pInterfaceResMgr->GetFont(0), LoadTempWString(3237), 0.0f, 0.0f);
			m_pWarnStr->SetColor(argbWhite);
			m_pWarnStr->SetDropShadow(true);
			m_pWarnStr->SetCharScreenHeight(s_nTextHeight);
		}
		m_pWarnStr->SetPosition(x + 20, yWarn + 6);
	}

	std::wstring sTitle = LoadTempWString(3269);
	if (!m_pTitle)
	{
		m_pTitle = g_pFontManager->CreatePolyString(g_pInterfaceResMgr->GetFont(0), sTitle, 0.0f, 0.0f);
		m_pTitle->SetColor(argbWhite);
		m_pTitle->SetDropShadow(true);
		m_pTitle->SetCharScreenHeight(s_nTitleHeight);
	}
	else
	{
		m_pTitle->SetText(sTitle);
	}
	m_pTitle->SetPosition(32, y - 34);

	m_bVisible = LTTRUE;
}

// RETAIL: 0x1003dd20
void CHUDTronDecision::Render()
{
	if (!m_bVisible) return;
	if (g_pInterfaceMgr->GetGameState() != GS_PLAYING) return;
	if (!IsVisible()) return;
	if (g_pLTClient->IsCommandOn(COMMAND_ID_TOGGLE_PROGRESS)) return;

	if (m_bTronUnknown4c)
	{
		CHUDDecision::Render();
		return;
	}

	UpdateBins();

	m_Frame.Render();
	if (m_pTitle)
		m_pTitle->Render();

	for (int i = 0; i < MAX_DECISION_BINS; i++)
	{
		m_Bins[i].Render();
	}

	if (m_bWarn)
	{
		m_WarnFrame.Render();
		if (m_pWarnStr)
			m_pWarnStr->Render();
	}
}

// RETAIL: 0x1003dde0
void CHUDTronDecision::Choose(uint8 nChoice)
{
	if (!m_hObject) return;

	if (m_bTronUnknown4c)
	{
		CHUDDecision::Choose(nChoice);
		return;
	}

	if (nChoice < MAX_DECISION_BINS)
	{
		CHUDDecisionBin* pBin = &m_Bins[nChoice];
		if (!pBin->m_bActive || pBin->m_bDownloading || pBin->m_bDone) return;

		if (g_pTronPlayerStats->GetEnergy() < m_fCost[nChoice])
		{
			g_pChatMsgs->AddMessage(3823);
			g_pClientSoundMgr->PlayInterfaceSound("interface\\snd\\cantafford.wav");
			return;
		}

		pBin->StartDownload();
	}
}

// RETAIL: 0x1003de90
void CHUDTronDecision::UpdateBins()
{
	bool bAllDone = true;

	if (g_pGameClientShell->IsGamePaused()) return;

	for (int i = 0; i < MAX_DECISION_BINS; i++)
	{
		bool bWasDone = m_Bins[i].m_bDone;

		if (g_pTronPlayerStats->GetEnergy() < m_fCost[i])
		{
			m_Bins[i].StopDownload();
		}

		m_Bins[i].Update();

		if (m_Bins[i].IsDownloading() || m_Bins[i].IsAvailable())
		{
			bAllDone = false;
		}

		if (m_Bins[i].m_bDone && !bWasDone)
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_OBJECT_MESSAGE);
			cMsg.WriteObject(m_hObject);
			cMsg.Writeuint32(MID_DECISION);
			cMsg.Writeuint8((uint8)i);
			g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

			g_pClientSoundMgr->PlayInterfaceSound("interface\\snd\\actioncomplete.wav");
		}
	}

	if (bAllDone)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_OBJECT_MESSAGE);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint32(MID_DECISION);
		cMsg.Writeuint8(DECISION_ALL_CHOSEN);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

		m_bVisible = LTFALSE;
		m_hObject = LTNULL;
		m_hRangeObject = LTNULL;
		m_bTronUnknown4d = true;
	}

	bool bDownloading = false;
	for (i = 0; i < MAX_DECISION_BINS; i++)
	{
		if (m_Bins[i].m_bDownloading)
			bDownloading = true;
	}

	if (bDownloading && !m_bDownloading)
	{
		m_hDownloadSnd = g_pClientSoundMgr->PlaySoundLocal("download", SOUNDPRIORITY_MISC_MEDIUM, 0, 100);
		if (m_pTitle)
		{
			m_pTitle->SetText(LoadTempWString(3270));
		}
	}
	else if (m_bDownloading && !bDownloading)
	{
		if (m_hDownloadSnd)
		{
			g_pLTClient->SoundMgr()->KillSound(m_hDownloadSnd);
			m_hDownloadSnd = LTNULL;
		}
		g_pClientSoundMgr->PlaySoundLocal("download_done", SOUNDPRIORITY_MISC_MEDIUM, 0, 100);
		if (m_pTitle)
		{
			m_pTitle->SetText(LoadTempWString(3269));
		}
	}

	m_bDownloading = bDownloading;
}

// RETAIL: 0x1003e2a0
void CHUDTronDecision::UpdateLayout()
{
	CHUDDecision::UpdateLayout();
	m_pWarnStr = LTNULL;
	m_WarnFrame.Init("chooserwarn");
}
