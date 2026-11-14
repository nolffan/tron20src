// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDecision.cpp
//
// PURPOSE : Implementation of CHUDDecision to display messages
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// TRON: Tron's decision window, written from retail CShell.dll
// (0x1013f660..0x10140be0; HUDDecision.h). NOLF2's dialog of column
// controls is gone; its sounds too.
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDDecision.h"
#include "InterfaceMgr.h"
#include "InterfaceResMgr.h"
#include "LayoutMgr.h"
#include "PlayerMgr.h"
#include "MsgIDs.h"
#include "ClientUtilities.h"
#include "CMoveMgr.h"
#include "ltquaduvutils.h"
#include "../TRON/TronLayoutMgr.h"

// RETAIL: the results Tron's decision objects are sent besides a choice
#define DECISION_ABORTED		0xFF	// a new decision replaced the window
#define DECISION_OUT_OF_RANGE	0xFE	// the player walked away
#define DECISION_ALL_CHOSEN		0xFC	// no choice is left

//******************************************************************************************
//** CHUDDecisionChoice
//******************************************************************************************

// RETAIL: 0x1013f660
CHUDDecisionChoice::CHUDDecisionChoice()
{
	m_pNumber = LTNULL;
	m_pText = LTNULL;
	Reset();
}

// RETAIL: 0x1013f6d0
void CHUDDecisionChoice::Reset()
{
	Term();
	m_nWidth = 200;
	m_bVisible = false;
	m_pNumber = LTNULL;
	m_pText = LTNULL;
}

// RETAIL: 0x1013f700
void CHUDDecisionChoice::Term()
{
	if (m_pNumber)
	{
		g_pFontManager->DestroyPolyString(m_pNumber);
		m_pNumber = LTNULL;
	}
	if (m_pText)
	{
		g_pFontManager->DestroyPolyString(m_pText);
		m_pText = LTNULL;
	}
}

// RETAIL: 0x1013f750
void CHUDDecisionChoice::Init(int nNumber, uint32 nStringID, int nFontSize, int nWidth)
{
	Reset();
	if (!nStringID) return;

	m_Frame.Init("chooserbox");

	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(20);
	m_nWidth = nWidth;
	float fYRatio = g_pInterfaceResMgr->GetYRatio();

	std::wstring sText;
	char szTmp[1024];

	sprintf(szTmp, "%d", nNumber);
	sText.erase(sText.begin(), sText.end());
	uint32 nLen = strlen(szTmp);
	for (uint32 i = 0; i < nLen; i++)
		sText += (wchar_t)(uint8)szTmp[i];

	m_pNumber = g_pFontManager->CreateFormattedPolyString(pFont, sText, 0.0f, 0.0f, CUI_HALIGN_CENTER);
	m_pNumber->SetCharScreenHeight(20);

	FormatString(nStringID, szTmp, sizeof(szTmp));
	sText.erase(sText.begin(), sText.end());
	nLen = strlen(szTmp);
	for (i = 0; i < nLen; i++)
		sText += (wchar_t)(uint8)szTmp[i];

	pFont = g_pInterfaceResMgr->GetFontForHeight((int)((float)nFontSize * fYRatio));
	m_pText = g_pFontManager->CreateFormattedPolyString(pFont, sText, 0.0f, 0.0f, CUI_HALIGN_CENTER);
	m_pText->SetCharScreenHeight(nFontSize);
	m_pText->SetWrapWidth(m_nWidth - 38);

	m_hNumberTex = g_pInterfaceResMgr->GetTexture("interface\\hud\\chooser_insert_number_on.dtx");
	g_pDrawPrim->SetRGBA(&m_Poly, argbWhite);
	SetupQuadUVs(m_Poly, m_hNumberTex, 0.0f, 0.0f, 1.0f, 1.0f);

	m_bVisible = true;
}

// RETAIL: 0x1013fa70
void CHUDDecisionChoice::Init(int nNumber, HSTRING hString, int nFontSize, int nWidth)
{
	Reset();
	const char* pString = g_pLTClient->GetStringData(hString);
	if (!pString || !pString[0]) return;

	m_Frame.Init("chooserbox");

	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(20);
	m_nWidth = nWidth;
	float fYRatio = g_pInterfaceResMgr->GetYRatio();

	std::wstring sText;
	char szTmp[1024];

	sprintf(szTmp, "%d", nNumber);
	sText.erase(sText.begin(), sText.end());
	uint32 nLen = strlen(szTmp);
	for (uint32 i = 0; i < nLen; i++)
		sText += (wchar_t)(uint8)szTmp[i];

	m_pNumber = g_pFontManager->CreateFormattedPolyString(pFont, sText, 0.0f, 0.0f, CUI_HALIGN_CENTER);
	m_pNumber->SetCharScreenHeight(20);

	pString = g_pLTClient->GetStringData(hString);
	sText.erase(sText.begin(), sText.end());
	nLen = strlen(pString);
	for (i = 0; i < nLen; i++)
		sText += (wchar_t)(uint8)pString[i];

	pFont = g_pInterfaceResMgr->GetFontForHeight((int)((float)nFontSize * fYRatio));
	m_pText = g_pFontManager->CreateFormattedPolyString(pFont, sText, 0.0f, 0.0f, CUI_HALIGN_CENTER);
	m_pText->SetCharScreenHeight(nFontSize);
	m_pText->SetWrapWidth(m_nWidth - 38);

	m_hNumberTex = g_pInterfaceResMgr->GetTexture("interface\\hud\\chooser_insert_number_on.dtx");
	g_pDrawPrim->SetRGBA(&m_Poly, argbWhite);
	SetupQuadUVs(m_Poly, m_hNumberTex, 0.0f, 0.0f, 1.0f, 1.0f);

	m_bVisible = true;
}

// RETAIL: 0x1013fd90
void CHUDDecisionChoice::GetSize(int &nWidth, int &nHeight)
{
	LTIntPt size = m_Frame.GetSize();
	nWidth = size.x;
	nHeight = size.y;
}

// RETAIL: 0x1013fdc0
void CHUDDecisionChoice::SetPosition(int x, int y)
{
	int32 nWidth, nHeight, nNumberHeight;
	m_pText->GetDims(&nWidth, &nHeight);
	m_pNumber->GetDims(&nWidth, &nNumberHeight);

	// RETAIL: the frame's height is the number's width and height
	m_Frame.SetRect(LTRect(x, y, x + m_nWidth, y + nWidth + nNumberHeight));

	g_pDrawPrim->SetXYWH(&m_Poly, (float)x, (float)y + 2.0f, 32.0f, 32.0f);
	m_pNumber->SetPosition(x + 14, y + 6);
	m_pText->SetPosition(x + 38, y + 6);
}

// RETAIL: 0x1013fe90
void CHUDDecisionChoice::Render()
{
	if (!m_bVisible) return;

	m_Frame.Render();

	g_pDrawPrim->SetTexture(m_hNumberTex);
	g_pDrawPrim->DrawPrim(&m_Poly);

	if (m_pNumber)
		m_pNumber->Render();
	if (m_pText)
		m_pText->Render();
}

//******************************************************************************************
//** CHUDDecision
//******************************************************************************************

// the choices a decision message carries
struct DecisionChoiceData
{
	DecisionChoiceData();

	bool	bString;
	HSTRING	hString;
	uint32	nStringID;
};

// RETAIL: 0x1013fef0
CHUDDecision::CHUDDecision()
{
	m_UpdateFlags = kHUDFrame;
	m_bTronUnknown4d = true;
	m_bRemoveWindow = LTTRUE;
	m_bVisible = LTFALSE;
	m_hObject = LTNULL;
	m_hRangeObject = LTNULL;
	m_fRadius = 0.0f;
	m_nLayout = 0xFF;
	m_bRemoveItems = LTFALSE;
	m_bTronUnknown4c = false;
}


// RETAIL: 0x1013ffc0
LTBOOL CHUDDecision::Init()
{
	m_pTitle = LTNULL;
	return LTTRUE;
}

// RETAIL: 0x1013ffd0
void CHUDDecision::Term()
{
	m_bTronUnknown4c = false;
	m_Frame.Term();
	if (m_pTitle)
	{
		g_pFontManager->DestroyPolyString(m_pTitle);
	}
	m_pTitle = LTNULL;
}

// RETAIL: 0x10140000
void CHUDDecision::Render()
{
	if (!IsVisible() || !m_bVisible) return;

	m_Frame.Render();
	for (int i = 0; i < MAX_DECISION_CHOICES; i++)
	{
		m_Choices[i].Render();
	}

	if (m_pTitle)
		m_pTitle->Render();
}

// RETAIL: 0x10140050
void CHUDDecision::Update()
{
	// Sanity checks...
	if (!IsVisible()) return;

	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
		SetScale(g_pInterfaceResMgr->GetXRatio());

	LTVector vObjPos, vPos;
	g_pLTClient->GetObjectPos(m_hRangeObject, &vObjPos);
	g_pLTClient->GetObjectPos(g_pPlayerMgr->GetMoveMgr()->GetObject(), &vPos);
	float fDist = vPos.Dist(vObjPos);
	if (m_fRadius > 0.0f && fDist > m_fRadius)
	{
		Hide(DECISION_OUT_OF_RANGE);
	}
}

// RETAIL: 0x10140120
void CHUDDecision::Hide(uint8 nChoice)
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_OBJECT_MESSAGE);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint32(MID_DECISION);
	cMsg.Writeuint8(nChoice);
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	m_hObject = LTNULL;
	m_hRangeObject = LTNULL;
	m_bVisible = LTFALSE;
	m_bTronUnknown4c = false;
	m_bTronUnknown4d = true;
}

// RETAIL: 0x10140230
void CHUDDecision::Show(ILTMessage_Read *pMsg)
{
	switch (pMsg->Readuint8())
	{
		case 2:
			ShowDecision(pMsg);
			break;
	}
}

// RETAIL: 0x10140260
void CHUDDecision::ShowDecision(ILTMessage_Read *pMsg)
{
	LTBOOL bShow = (LTBOOL)pMsg->Readuint8();
	if (bShow)
	{
		uint8 nLayout = pMsg->Readuint8();
		LTBOOL bForce = (LTBOOL)pMsg->Readuint8();
		g_pLTClient->CPrint("CHUDDecision::Show() : show");

		//read in the message
		DecisionChoiceData aChoices[MAX_DECISION_CHOICES];
		for (int i=0; i < MAX_DECISION_CHOICES; i++)
		{
			aChoices[i].bString = !!pMsg->Readuint8();
			if (aChoices[i].bString)
				aChoices[i].hString = pMsg->ReadHString();
			else
				aChoices[i].nStringID = pMsg->Readuint32();
		}

		HOBJECT hObj = pMsg->ReadObject();
		HOBJECT hRangeObj = pMsg->ReadObject();
		float fRad = pMsg->Readfloat();
		bool bTronUnknown4d = pMsg->Readbool();

		LTVector vObjPos, vPos;
		g_pLTClient->GetObjectPos(hRangeObj, &vObjPos);
		g_pLTClient->GetObjectPos(g_pPlayerMgr->GetMoveMgr()->GetObject(), &vPos);
		float fDist = (vPos - vObjPos).Length();

		//if we're out of range, bail right away
		if (!bForce && fRad > 0.0f && fDist > fRad)
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

		if (m_nLayout != nLayout)
		{
			m_nLayout = nLayout;
			UpdateLayout();
		}

		//fill the new window
		for (i=0; i < MAX_DECISION_CHOICES; i++)
		{
			if (aChoices[i].bString)
				m_Choices[i].Init(i+1, aChoices[i].hString, m_nFontSize, m_nWidth);
			else
				m_Choices[i].Init(i+1, aChoices[i].nStringID, m_nFontSize, m_nWidth);

			if (m_Choices[i].IsVisible())
				m_Choices[i].SetPosition(0, 0);
		}

		m_hObject = hObj;
		m_bTronUnknown4c = true;
		m_fRadius = fRad;
		m_hRangeObject = hRangeObj;
		m_bTronUnknown4d = bTronUnknown4d;

		// stack the choices up from above the progress box, the last lowest
		int x = 12;
		int y = g_pInterfaceResMgr->GetScreenHeight() - ((CTronLayoutMgr*)g_pLayoutMgr)->GetProgressHUDOffset();
		int nHeight = 0;
		for (i = MAX_DECISION_CHOICES - 1; i >= 0; i--)
		{
			if (m_Choices[i].IsVisible())
			{
				int nChoiceWidth, nChoiceHeight;
				m_Choices[i].GetSize(nChoiceWidth, nChoiceHeight);
				y -= nChoiceHeight;
				m_Choices[i].SetPosition(x, y);
				y -= 4;
				nHeight += nChoiceHeight + 4;
			}
		}

		if (!m_pTitle)
		{
			CUIFont* pFont = g_pInterfaceResMgr->GetFont(0);
			m_pTitle = g_pFontManager->CreatePolyString(pFont, LoadTempWString(16), 0.0f, 0.0f);
			m_pTitle->SetColor(argbWhite);
			m_pTitle->SetDropShadow(true);
			m_pTitle->SetCharScreenHeight(20);
		}
		else
		{
			m_pTitle->SetText(LoadTempWString(16));
		}
		m_pTitle->SetPosition(32, y - 34);

		m_Frame.SetPosition(x, y);
		LTRect rcMargins = m_Frame.GetMargins();
		m_Frame.SetSize(m_nWidth - rcMargins.right, nHeight);

		m_bVisible = LTTRUE;
	}
	else
	{
		HOBJECT hObject = pMsg->ReadObject();
		if (m_hObject == hObject)
		{
			g_pLTClient->CPrint("CHUDDecision::Show() : hide");
			m_hObject = LTNULL;
			m_hRangeObject = LTNULL;
			m_bVisible = LTFALSE;
			m_bTronUnknown4d = true;
		}
	}
}

// RETAIL: 0x10140800
DecisionChoiceData::DecisionChoiceData() :
	bString(false),
	hString(LTNULL),
	nStringID(0)
{
}

// RETAIL: 0x10140810
void CHUDDecision::Choose(uint8 nChoice)
{
	if (!m_hObject) return;

	if (nChoice < MAX_DECISION_CHOICES)
	{
		if (!m_Choices[nChoice].IsVisible()) return;
	}
	else
	{
		nChoice = MAX_DECISION_CHOICES;
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_OBJECT_MESSAGE);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint32(MID_DECISION);
	cMsg.Writeuint8(nChoice);
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	// RETAIL: resets m_Choices[6] (the frame after the choices) when the
	// window is cancelled (docs/RETAIL_BUGS.md)
	if (m_bRemoveItems)
		m_Choices[nChoice].Reset();

	for (int i = 0; i < MAX_DECISION_CHOICES; i++)
	{
		if (m_Choices[i].IsVisible()) goto Found;
	}

	{
		CAutoMessage cAllMsg;
		cAllMsg.Writeuint8(MID_OBJECT_MESSAGE);
		cAllMsg.WriteObject(m_hObject);
		cAllMsg.Writeuint32(MID_DECISION);
		cAllMsg.Writeuint8(DECISION_ALL_CHOSEN);
		g_pLTClient->SendToServer(cAllMsg.Read(), MESSAGE_GUARANTEED);
	}
	goto Close;

Found:
	// keep the window for the choices left
	if (!m_bRemoveWindow && nChoice != MAX_DECISION_CHOICES)
		goto Done;

Close:
	m_bVisible = LTFALSE;
	m_hObject = LTNULL;
	m_hRangeObject = LTNULL;
	m_bTronUnknown4c = false;
	m_bTronUnknown4d = true;

Done:
	;
}

// RETAIL: 0x10140a20
void CHUDDecision::SetScale(float fScale)
{
	m_fScale = fScale;
}

// RETAIL: 0x10140a30
void CHUDDecision::UpdateLayout()
{
	char szTag[64];
	sprintf(szTag, "DecisionWindow%d", m_nLayout);

	m_BasePos = g_pLayoutMgr->GetPoint(szTag,"Pos");
	int nNumColumns = g_pLayoutMgr->GetInt(szTag,"NumColumns");

	uint8 nFont = (uint8)g_pLayoutMgr->GetInt(szTag,"Font");
	m_pFont = g_pInterfaceResMgr->GetFont(nFont);
	m_nFontSize = 20;

	m_bRemoveItems = g_pLayoutMgr->GetInt(szTag,"RemoveItems");
	m_bRemoveWindow = g_pLayoutMgr->GetInt(szTag,"RemoveWindow");
	m_nWidth = (uint16)g_pLayoutMgr->GetInt(szTag,"Width");
	m_Offset = g_pLayoutMgr->GetPoint(szTag,"TextOffset");
	LTVector vCol = g_pLayoutMgr->GetVector(szTag,"TextColor");

	char szFrame[128] = "";
	g_pLayoutMgr->GetString(szTag,"Frame",szFrame,sizeof(szFrame));

	// RETAIL: the frame named is not used
	m_Frame.Init("chooserframe");
}



// RETAIL: 0x10140bb0
void CHUDDecision::OnObjectRemove(HLOCALOBJ hObj)
{
	if (!hObj) return;
	if (m_hObject != hObj && m_hRangeObject != hObj) return;

	m_hRangeObject = LTNULL;
	m_hObject = LTNULL;
	m_bVisible = LTFALSE;
	m_bTronUnknown4d = true;
}
