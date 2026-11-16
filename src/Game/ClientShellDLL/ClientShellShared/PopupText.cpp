// PopupText.cpp: implementation of the CPopupText class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PopupText.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "ClientWeaponMgr.h"
#include "PopupMgr.h"


namespace
{
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPopupText::CPopupText()
{
	m_bVisible			= LTFALSE;
	m_bUsePopupState	= true;
	// RETAIL: Tron's (retail 0x1014eb00)
	m_bTronUnknownDrawFrame2 = false;
	m_hFrameTex			= LTNULL;
	m_hTronUnknownFrame2Tex = LTNULL;
	m_bWeaponsEnabled	= false;
}



CPopupText::~CPopupText()
{
	Term();
}


void CPopupText::Init(bool bUsePopupState/*=true*/)
{
	m_bVisible		= LTFALSE;
	m_bUsePopupState = bUsePopupState;
	m_Text.Create(L" ",0,0,g_pInterfaceResMgr->GetFont(0),8,LTNULL);

	// RETAIL: Tron's keeps its own reference to the frame's texture
	// (retail 0x1014ec76), for both frames
	if (g_pTexInterface->FindTextureFromName(m_hFrameTex, "interface\\menu\\sprtex\\frame.dtx") != LT_OK)
	{
		g_pTexInterface->CreateTextureFromName(m_hFrameTex, "interface\\menu\\sprtex\\frame.dtx");
	}
	g_pTexInterface->AddRefTextureHandle(m_hFrameTex);

	m_Frame.Create(m_hFrameTex,200,320,LTTRUE);
	m_TronUnknownFrame2.Create(m_hFrameTex,200,320,LTTRUE);
}

void CPopupText::Term()
{
	if (m_bVisible)
	{
		Close();
	}

}


void CPopupText::Close()
{
	m_bVisible = LTFALSE;

	// RETAIL: Tron's (retail 0x1014ed10)
	g_pClientSoundMgr->PlayInterfaceSound("interface\\snd\\hidepopup.wav");

	//if we had our weapons out before, show them again now
	if (g_pInterfaceMgr && g_pPlayerMgr)
	{
		if (m_bWeaponsEnabled)
		{
			g_pPlayerMgr->GetClientWeaponMgr()->EnableWeapons();
		}
		else
		{
			g_pPlayerMgr->GetClientWeaponMgr()->DisableWeapons();
		}
	}

	if (m_hFrameTex)
	{
		g_pTexInterface->ReleaseTextureHandle(m_hFrameTex);
		m_hFrameTex = LTNULL;
		m_Frame.SetFrame(LTNULL);
	}

	if (m_hTronUnknownFrame2Tex)
	{
		g_pTexInterface->ReleaseTextureHandle(m_hTronUnknownFrame2Tex);
		m_hTronUnknownFrame2Tex = LTNULL;
		m_TronUnknownFrame2.SetFrame(LTNULL);
	}
}

void CPopupText::Draw()
{
	if (!m_bVisible) return;

	m_Frame.Render();
	// RETAIL: Tron's second frame (retail 0x1014edc3)
	if (m_bTronUnknownDrawFrame2)
	{
		m_TronUnknownFrame2.Render();
	}
	m_Text.Render();
}


void CPopupText::DisplayPopup(uint8 nPopupId, std::wstring sText, uint32 nTextId, const char* szImage)
{
	POPUP* pPopup = g_pPopupMgr->GetPopup(nPopupId);
	if (!pPopup) return;

	LTIntPt pos( (640 - pPopup->sSize.x) / 2, (480 - pPopup->sSize.y) / 2 );
	float fScale = g_pInterfaceResMgr->GetXRatio();

	// RETAIL: Tron's keeps its own reference to the frame's texture
	const char* szFrame = pPopup->szFrame;
	if (szFrame && szFrame[0])
	{
		if (m_hFrameTex)
		{
			g_pTexInterface->ReleaseTextureHandle(m_hFrameTex);
			m_hFrameTex = LTNULL;
		}
		if (g_pTexInterface->FindTextureFromName(m_hFrameTex, szFrame) != LT_OK)
		{
			g_pTexInterface->CreateTextureFromName(m_hFrameTex, szFrame);
		}
		g_pTexInterface->AddRefTextureHandle(m_hFrameTex);
		if (m_hFrameTex)
		{
			m_Frame.SetFrame(m_hFrameTex);
		}
	}

	m_Frame.SetSize(pPopup->sSize.x,pPopup->sSize.y);
	m_Frame.SetBasePos(pos);
	m_Frame.SetScale(fScale);

	// RETAIL: Tron's image, in the second frame
	if (m_hTronUnknownFrame2Tex)
	{
		g_pTexInterface->ReleaseTextureHandle(m_hTronUnknownFrame2Tex);
		m_hTronUnknownFrame2Tex = LTNULL;
		m_TronUnknownFrame2.SetFrame(LTNULL);
	}
	m_bTronUnknownDrawFrame2 = false;

	if (szImage && szImage[0])
	{
		if (g_pTexInterface->FindTextureFromName(m_hTronUnknownFrame2Tex, szImage) != LT_OK)
		{
			g_pTexInterface->CreateTextureFromName(m_hTronUnknownFrame2Tex, szImage);
		}
		g_pTexInterface->AddRefTextureHandle(m_hTronUnknownFrame2Tex);
		if (m_hTronUnknownFrame2Tex)
		{
			m_TronUnknownFrame2.SetFrame(m_hTronUnknownFrame2Tex);

			pos.x += pPopup->sImageOffset.x;
			pos.y += pPopup->sImageOffset.y;
			m_TronUnknownFrame2.SetBasePos(pos);
			pos.x -= pPopup->sImageOffset.x;
			pos.y -= pPopup->sImageOffset.y;

			if (pPopup->sImageSize.x > 0 && pPopup->sImageSize.y > 0)
			{
				m_TronUnknownFrame2.SetSize(pPopup->sImageSize.x, pPopup->sImageSize.y);
			}
			else
			{
				uint32 nWidth, nHeight;
				g_pTexInterface->GetTextureDims(m_hTronUnknownFrame2Tex, nWidth, nHeight);
				m_TronUnknownFrame2.SetSize(nWidth, nHeight);
			}
			m_TronUnknownFrame2.SetScale(fScale);
			m_bTronUnknownDrawFrame2 = true;
		}
	}

	pos.x += pPopup->sTextOffset.x;
	pos.y += pPopup->sTextOffset.y;

	m_Text.SetScale(1.0f);

	// If we were given a string id use it, otherwise use the string...
	if( nTextId )
	{
		char szString[1024] = "";
		std::wstring sString;
		LoadString(nTextId,szString,sizeof(szString));
		char *pBody = strchr(szString,'@');
		if (pBody)
		{
			++pBody;
			sString.erase(sString.begin(), sString.end());
			uint32 nLen = strlen(pBody);
			for (uint32 i = 0; i < nLen; i++)
				sString.push_back((wchar_t)(uint8)pBody[i]);
			m_Text.SetString(sString);
		}
		else
		{
			sString.erase(sString.begin(), sString.end());
			uint32 nLen = strlen(szString);
			for (uint32 i = 0; i < nLen; i++)
				sString.push_back((wchar_t)(uint8)szString[i]);
			m_Text.SetString(sString);
		}
	}
	else
	{
		m_Text.SetString(sText);
	}

	m_Text.SetFont(g_pInterfaceResMgr->GetFontForHeight(pPopup->nFontSize),pPopup->nFontSize);
	m_Text.SetColors(pPopup->argbTextColor,pPopup->argbTextColor,pPopup->argbTextColor);
	m_Text.SetFixedWidth(pPopup->nTextWidth);
	m_Text.SetBasePos(pos);
	m_Text.SetScale(g_pInterfaceResMgr->GetXRatio());

	m_bVisible = LTTRUE;

	if (m_bUsePopupState)
	{
		g_pInterfaceMgr->ChangeState(GS_POPUP);
	}

	// RETAIL: Tron's (retail 0x1014f3dc)
	g_pClientSoundMgr->PlayInterfaceSound("interface\\snd\\showpopup.wav");

	//remember whether our weapons were out
	m_bWeaponsEnabled = g_pPlayerMgr->GetClientWeaponMgr()->WeaponsEnabled();
}

void CPopupText::Update()
{
}

LTBOOL CPopupText::OnKeyDown(int key, int rep)
{
	// They pressed escape - close the popup
	if (key == VK_ESCAPE)
	{
		Close();
        return LTTRUE;
	}
	return LTFALSE;
}
