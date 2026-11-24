//-------------------------------------------------------------------------
//
// MODULE  : HUDMessage.cpp
//
// PURPOSE : Base class for HUD text display
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------


#include "stdafx.h"
#include "GameClientShell.h"
#include "HUDMessage.h"
#include "LayoutMgr.h"

namespace
{
	float kMinDuration = -1.0f;
	float kMinFade = -1.0f;
}


CHUDMessage::CHUDMessage()
{
	m_hImage = LTNULL;
	m_nBaseImageSize = 0;

	m_pText = LTNULL;
	m_pSplitText = LTNULL;

	m_pFont = LTNULL;
	m_nFontSize = 0;
	m_nBaseFontSize = 0;
	m_nTextColor = argbWhite;

	m_fInitTime = -1.0f;
	m_fScrambleTime = 0.0f;
	m_eJustify = kMsgLeft;
	m_fDuration = 0.0f;
	m_fFadeDur  = 0.0f;
	m_fSplitDelay = 0.0f;

	m_fImageGap	= 4.0f;
	m_fAlpha = 1.0f;
}


// RETAIL: Tron's (0x101880f0): the split text, and only left or right
// alignment
LTBOOL CHUDMessage::Create(MsgCreate &mc)
{
	if (!mc.pFont) return LTFALSE; 

	if (kMinDuration < 0.0f)
		kMinDuration = g_pLayoutMgr->GetMessageMinTime();
	if (kMinFade < 0.0f)
		kMinFade = g_pLayoutMgr->GetMessageMinFade();


	m_pFont = mc.pFont;


	if (!m_pText)
	{
		m_pText = g_pFontManager->CreateFormattedPolyString(m_pFont,std::wstring(),0.0f,0.0f,CUI_HALIGN_LEFT);
		m_pText->SetDropShadow(true);
		if (!m_pText)
			return LTFALSE;
	}

	if (!m_pSplitText && mc.fDropShadow > 0.0f)
	{
		m_pSplitText = g_pFontManager->CreateFormattedPolyString(m_pFont,std::wstring(),0.0f,0.0f,CUI_HALIGN_LEFT);
		if (!m_pSplitText)
			mc.fDropShadow = 0.0f;
	}

	m_pText->SetText(mc.sString.c_str());
	if (m_pSplitText)
		m_pSplitText->SetText(mc.sString.c_str());


	m_nFontSize = mc.nFontSize;
	m_nBaseFontSize = mc.nFontSize;
	m_nTextColor = mc.nTextColor;
	m_nFixedWidth = mc.nWidth;

	m_pText->SetColor(m_nTextColor);
	if (m_pSplitText)
		m_pSplitText->SetColor(m_nTextColor);

	m_fInitTime = -1.0f;

	m_eJustify = mc.eJustify;
	m_fDuration = mc.fDuration * GetConsoleFloat("MessageDuration",1.0f);
	if (m_fDuration < kMinDuration)
		m_fDuration = kMinDuration;
	m_fFadeDur  = mc.fFadeDur * GetConsoleFloat("MessageDuration",1.0f);;
	if (m_fFadeDur < kMinFade)
		m_fFadeDur = kMinFade;
	m_fSplitDelay = mc.fDropShadow * GetConsoleFloat("MessageDuration",1.0f);
	if (m_fSplitDelay < 0.0f)
		m_fSplitDelay = 0.0f;
	m_fDuration = m_fSplitDelay + m_fDuration;

	m_hImage = mc.hImage;
	m_nBaseImageSize = mc.nImageSize;

	switch (m_eJustify)
	{
	case kMsgLeft:
		m_pText->TronUnknownVirtual30(CUI_HALIGN_LEFT);
		if (m_pSplitText)
			m_pSplitText->TronUnknownVirtual30(CUI_HALIGN_LEFT);
		break;
	case kMsgRight:
		m_pText->TronUnknownVirtual30(CUI_HALIGN_RIGHT);
		if (m_pSplitText)
			m_pSplitText->TronUnknownVirtual30(CUI_HALIGN_RIGHT);
		break;
	};


	InitPoly();

	SetBasePos(LTIntPt(0,0));

	SetScale(g_pInterfaceResMgr->GetXRatio());

	m_bVisible = LTTRUE;

	

	return LTTRUE;
}

// RETAIL: 0x101884c0
void CHUDMessage::Destroy ( )
{
	if (m_pText)
	{
		g_pFontManager->DestroyPolyString(m_pText);
		m_pText = LTNULL;
	}
	if (m_pSplitText)
	{
		g_pFontManager->DestroyPolyString(m_pSplitText);
		m_pSplitText = LTNULL;
	}
}

// RETAIL: 0x10188510. The split text fades in while the delay runs
void CHUDMessage::SetAlpha(float fAlpha)
{
	if (fAlpha < 0.0f) fAlpha = 0.0f;
	if (fAlpha > 1.0f) fAlpha = 1.0f;
	m_fAlpha = fAlpha;
	uint32 a,r,g,b;
	GET_ARGB(m_nTextColor,a,r,g,b);
	a = (uint8)(255.0f * fAlpha);
	m_nTextColor = SET_ARGB(a,r,g,b);
	if (m_pText)
		m_pText->SetColor(m_nTextColor);



	uint32 c  = SET_ARGB(a,255,255,255);
	g_pDrawPrim->SetRGBA(&m_Poly,c);

	if (m_pSplitText)
	{
		if (GetLifetime() <= m_fSplitDelay)
			a = 255 - a;
		m_pSplitText->SetColor(SET_ARGB(a,r,g,b));
	}
}


// RETAIL: 0x10188620
void CHUDMessage::Update()
{
	// Sanity checks...
	if (!IsVisible()) return;

	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
		SetScale(g_pInterfaceResMgr->GetXRatio());

	if (m_fInitTime < 0.0f)
	{
		m_fInitTime = g_pLTClient->GetTime();
		m_fScrambleTime = 0.0f;
	}

	if (m_fDuration > 0.0f)
	{
		float fTime = GetLifetime();
		if (m_fSplitDelay > 0.0f)
		{
			float fAlpha = fTime / m_fSplitDelay;
			if (fAlpha > 1.0f)
				fAlpha = 1.0f;
			SetAlpha(fAlpha);
		}
		if (fTime > m_fDuration)
		{
			fTime -= m_fDuration;
			float fAlpha = 1.0f - (fTime / m_fFadeDur);
			if (fAlpha <= 0.0f)
			{
				Show(LTFALSE);
				return;
			}
			else
				SetAlpha(fAlpha);
		}
	}
	else
		Show(LTFALSE);

}


// RETAIL: 0x10188750
void CHUDMessage::Render(LTBOOL bForceVisible)
{

	LTBOOL bVis = m_bVisible;
	if (!bVis) return;
	float fAlpha = m_fAlpha;
	if (bForceVisible)
	{
		m_bVisible = LTTRUE;
		SetAlpha(1.0f);
	}
	Render();
	if (bForceVisible)
	{
		m_bVisible = bVis;
		SetAlpha(fAlpha);
	}

}

// RETAIL: 0x101887b0. The split text shows while the delay runs (its
// letters scrambled), and again as the message fades
void CHUDMessage::Render()
{
	// Sanity checks...
	if (!IsVisible()) return;

	if (m_hImage)
	{
		g_pDrawPrim->SetTexture(m_hImage);
		// set up the render state	
		SetRenderState();

		// draw our button
		g_pDrawPrim->DrawPrim(&m_Poly);
	}

	float fTime = GetLifetime();
	if (fTime > m_fDuration)
	{
		UpdateSplitText();
	}

	if (m_pText)
	{
		m_pText->Render();
	}

	if (m_pSplitText)
	{
		if (fTime <= m_fSplitDelay)
		{
			ScrambleSplitText();
			m_pSplitText->Render();
		}
		else if (fTime > m_fDuration)
		{
			m_pSplitText->Render();
		}
	}
}


// RETAIL: 0x10188870. Every 0.05 seconds each letter of the split text
// shows a random part of its texture
void CHUDMessage::ScrambleSplitText()
{
	m_fScrambleTime += g_pLTClient->GetTime() - m_fScrambleTime;
	if (m_fScrambleTime < 0.05f) return;

	int nPolys = m_pSplitText->GetLength();
	LT_POLYGT4* pSrc = m_pText->GetPolys();
	LT_POLYGT4* pDest = m_pSplitText->GetPolys();

	for (int i = 0; i < nPolys; i++)
	{
		float fU = GetRandom(0.0f, 0.9f);
		float fV = GetRandom(0.0f, 0.9f);

		float fW = pSrc[i].verts[1].u - pSrc[i].verts[0].u;
		if (fW < 0.05f)
			fW = 0.05f;
		float fH = pSrc[i].verts[3].v - pSrc[i].verts[0].v;
		if (fH < 0.05f)
			fH = 0.05f;

		g_pDrawPrim->SetUVWH(&pDest[i], fU, fV, fW, fH);
	}
}


// RETAIL: 0x10188980. The two texts move apart as the message fades
void CHUDMessage::UpdateSplitText()
{
	if (!m_pSplitText) return;

	float fRatio = (GetLifetime() - m_fDuration) / m_fFadeDur;
	float fDisplacement = GetConsoleFloat("FadeDisplacement", 40.0f);
	int nOffset = (int)(fDisplacement * (fRatio * fRatio * g_pInterfaceResMgr->GetXRatio()));

	m_pText->SetPosition(m_nTextX - nOffset, m_nTextY);
	m_pSplitText->SetPosition(m_nTextX + nOffset, m_nTextY);
}


float CHUDMessage::GetLifetime()
{
	if (m_fInitTime < 0.0f)
		return 0.0f;

	return g_pLTClient->GetTime() - m_fInitTime;
}


// RETAIL: 0x10188a50
void CHUDMessage::SetBasePos ( LTIntPt pos )
{ 
	CLTGUICtrl::SetBasePos(pos);
	ScalePoly();
	if (m_pText)
	{
		float x = (float)m_pos.x;
		float y = (float)m_pos.y;

		if (m_hImage)
		{
			switch (m_eJustify)
			{
			case kMsgLeft:
			case kMsgCenter:
				//adjust for image size
				x += (float)m_imageSize.x + m_fImageGap;
				y += ((float)m_imageSize.y - m_pText->GetHeight()) / 2;
				break;
			case kMsgRight:
				break;
			};

		}

		if (m_eJustify == kMsgCenter)
		{
			int32 nWidth, nHeight;
			m_pText->GetDims(&nWidth, &nHeight);
			m_pText->SetPosition((int)x - nWidth / 2, (int)y);
			if (m_pSplitText)
				m_pSplitText->SetPosition((int)x - nWidth / 2, (int)y);
		}
		else
		{
			m_pText->SetPosition((int)x, (int)y);
			if (m_pSplitText)
				m_pSplitText->SetPosition((int)x, (int)y);
		}
	}
	JustifyPoly();


}

// RETAIL: 0x10188bc0
void CHUDMessage::SetScale(float fScale)
{
	CLTGUICtrl::SetScale(fScale);
	ScalePoly();
	m_fImageGap = 4.0f * m_fScale;
	m_nFontSize = (uint8)((float)m_nBaseFontSize * m_fScale);
	if (m_pText)
	{
		float x = (float)m_pos.x;
		float y = (float)m_pos.y;
		m_pFont = g_pInterfaceResMgr->GetFontForHeight(m_nFontSize);
		m_pText->SetFont(m_pFont);
		m_pText->SetCharScreenHeight(m_nFontSize);
		if (m_pSplitText)
		{
			m_pSplitText->SetFont(m_pFont);
			m_pSplitText->SetCharScreenHeight(m_nFontSize);
		}
		if (m_hImage)
		{
			switch (m_eJustify)
			{
			case kMsgLeft:
			case kMsgCenter:
				//adjust for image size
				x = (float)m_imageSize.x + x + m_fImageGap;
				y += ((float)m_imageSize.y - m_pText->GetHeight()) / 2;
				break;
			case kMsgRight:
				break;
			};			
		}

		if (m_eJustify == kMsgCenter)
		{
			int32 nWidth, nHeight;
			m_pText->GetDims(&nWidth, &nHeight);
			m_nTextX = (int)x - nWidth / 2;
			m_nTextY = (int)y;
			m_pText->SetPosition(m_nTextX, m_nTextY);
			if (m_pSplitText)
				m_pSplitText->SetPosition(m_nTextX, m_nTextY);
		}
		else
		{
			m_nTextX = (int)x;
			m_nTextY = (int)y;
			m_pText->SetPosition(m_nTextX, m_nTextY);

			uint16 nTextWidth = (uint16)((float)m_nFixedWidth * m_fScale - ((float)m_imageSize.x + m_fImageGap));
			m_pText->SetWrapWidth(nTextWidth);
			if (m_pSplitText)
			{
				m_pSplitText->SetPosition(m_nTextX, m_nTextY);
				m_pSplitText->SetWrapWidth(nTextWidth);
			}
		}

		m_nWidth = m_imageSize.x + (uint16)( m_pText->GetWidth() + m_fImageGap );
		m_nHeight = Max((uint16)m_imageSize.y,(uint16)m_pText->GetHeight());

	}
	else
	{
		m_nWidth = m_imageSize.x;
		m_nHeight = m_imageSize.y;
	}
	JustifyPoly();

}


void CHUDMessage::SetRenderState()
{
	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
}

void CHUDMessage::InitPoly()
{
	if (!m_hImage) return;
	ScalePoly();
	SetupQuadUVs(m_Poly, m_hImage, 0.0f,0.0f,1.0f,1.0f);
	g_pDrawPrim->SetRGBA(&m_Poly,0xFFFFFFFF);
	JustifyPoly();
}

void CHUDMessage::ScalePoly()
{
	if (!m_hImage) return;

	uint32 w,h;
	g_pTexInterface->GetTextureDims(m_hImage,w,h);

	if (m_nBaseImageSize > 0)
	{
		w = (uint32)( (float)w * (float)m_nBaseImageSize/ (float)h);
		h = m_nBaseImageSize;
	}


	m_imageSize.x = (int)((float)w * m_fScale);
	m_imageSize.y = (int)((float)h * m_fScale);

}


void CHUDMessage::JustifyPoly()
{
	if (!m_hImage) return;

	float x = (float)m_basePos.x * m_fScale;
	float y = (float)m_basePos.y * m_fScale;
	float fw = (float)m_imageSize.x;
	float fh = (float)m_imageSize.y;

	if (m_pText)
	{
		switch (m_eJustify)
		{
		case kMsgCenter:
			//adjust for text size
			x -= ( m_pText->GetWidth() + m_fImageGap ) / 2.0f;
			break;
		case kMsgRight:
			//adjust for text size
			x -= ( m_pText->GetWidth() + m_fImageGap );
			break;
		case kMsgLeft:
			break;
		};
	}

//	y += (m_pText->GetHeight() - (float)m_imageSize.y) / 2.0f;

	g_pDrawPrim->SetXYWH(&m_Poly,x,y,fw,fh);


}




