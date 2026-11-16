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


void CPopupText::DisplayPopup(uint8 nPopupId, char *pText, uint32 nTextId)
{
	POPUP* pPopup = g_pPopupMgr->GetPopup(nPopupId);
	if (!pPopup) return;

	CUIFont *pFont = g_pInterfaceResMgr->GetFont(pPopup->nFont);
	LTIntPt pos( (640 - pPopup->sSize.x) / 2, (480 - pPopup->sSize.y) / 2 );

	m_Frame.SetFrame(g_pInterfaceResMgr->GetTexture(pPopup->szFrame));
	m_Frame.SetSize(pPopup->sSize.x,pPopup->sSize.y);
	m_Frame.SetBasePos(pos);
	m_Frame.SetScale(g_pInterfaceResMgr->GetXRatio());


	pos.x += pPopup->sTextOffset.x;
	pos.y += pPopup->sTextOffset.y;

	m_Text.SetScale(1.0f);

	// If we were given a string use it, otherwise get the string from the ID...

	if( pText )
	{
		m_Text.SetString( pText );
	}
	else
	{
		char szString[1024] = "";
		LoadString(nTextId,szString,sizeof(szString));

		char *pBody = strchr(szString,'@');
		if (pBody)
		{
			++pBody;
			m_Text.SetString(pBody);
		}
		else
		{
			m_Text.SetString(szString);
		}
	}

	m_Text.SetFont(pFont,pPopup->nFontSize);
	m_Text.SetColors(pPopup->argbTextColor,pPopup->argbTextColor,pPopup->argbTextColor);
	m_Text.SetFixedWidth(pPopup->nTextWidth);
	m_Text.SetBasePos(pos);
	m_Text.SetScale(g_pInterfaceResMgr->GetXRatio());

	m_bVisible = LTTRUE;

	//remember whether our weapons were out
	m_bWeaponsEnabled = g_pPlayerMgr->GetClientWeaponMgr()->WeaponsEnabled();

	if (m_bUsePopupState)
	{
		g_pInterfaceMgr->ChangeState(GS_POPUP);
	}
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
