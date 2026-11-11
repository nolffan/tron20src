// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUITextItemCtrl.cp
//
// PURPOSE : Simple text control which may be used as a menu item.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUITextCtrl::CLTGUITextCtrl()
{
    m_pFont             = LTNULL;
	m_nFontSize			= 0;
	m_nBaseFontSize		= 0;
    m_pCommandHandler   = LTNULL;
	m_nWidth			= 0;
	m_nHeight			= 0;
    m_nFixedWidth		= 0;
    m_nIndent			= 0;
	m_bClip				= LTFALSE;
	m_pString			= LTNULL;
	m_pPulseString		= LTNULL;
	m_fPulse			= 0.0f;

}

CLTGUITextCtrl::~CLTGUITextCtrl()
{
	Destroy();
}

// Create the control
LTBOOL CLTGUITextCtrl::Create (  	std::wstring sText,
                                    uint32      nCommandID,
                                    uint32      nHelpID,
									CUIFont*	pFont,
									uint8		nFontSize,
									CLTGUICommandHandler *pCommandHandler,
                                    uint32      nParam1,
                                    uint32      nParam2)
{
	m_pCommandHandler	= pCommandHandler;


	if (!SetFont(pFont,nFontSize))
	{
        return LTFALSE;
	}

	// Add the string
	if (sText.length())
	{
		SetString(sText);
	}

	CLTGUICtrl::Create(nCommandID, nHelpID, nParam1,nParam2);

    return LTTRUE;
}


// Destroys the control
void CLTGUITextCtrl::Destroy ( )
{
	if (m_pString)
	{
		g_pFontManager->DestroyPolyString(m_pString);
		m_pString = LTNULL;
	}
	if (m_pPulseString)
	{
		g_pFontManager->DestroyPolyString(m_pPulseString);
		m_pPulseString = LTNULL;
	}

}


// Add more strings to the control.  These are cycled when OnLeft() and OnRight() are called
void CLTGUITextCtrl::SetString(std::wstring sText)
{
	int nLen = sText.length();
	if ( !m_pFont || !m_nFontSize)
		return;

	ASSERT(nLen < 1024);
	if (nLen >= 1024)
		return;


	if (!m_pString)
		m_pString = g_pFontManager->CreateFormattedPolyString(m_pFont,sText,(float)m_pos.x,(float)m_pos.y);
	else
		m_pString->SetText(sText);


	if (m_pString)
	{
		if (m_nFixedWidth && !m_bClip)
		{
			uint16 nWidth = (uint16)(m_fScale * (float)(m_nFixedWidth-m_nIndent));
			m_pString->SetWrapWidth(nWidth);
		}

		m_pString->SetCharScreenHeight(m_nFontSize);
		CalculateSize();

	}
}


// Set the font
LTBOOL CLTGUITextCtrl::SetFont(CUIFont *pFont, uint8 nFontSize)
{
	if (!pFont && !nFontSize)
	{
        return LTFALSE;
	}

	if (pFont)
		m_pFont = pFont;
	if (nFontSize)
		m_nBaseFontSize = nFontSize;

	m_nFontSize = (uint8)(m_fScale * (float)m_nBaseFontSize);

	if (m_pString)
	{
		m_pString->SetFont(m_pFont);
		m_pString->SetCharScreenHeight(m_nFontSize);
		CalculateSize();
	}


    return LTTRUE;
}


void CLTGUITextCtrl::CalculateSize()
{
	if (!m_pFont || !m_pString)
	{
		if (m_nFixedWidth)
			m_nWidth=m_nFixedWidth;
		else
			m_nWidth=0;
		m_nHeight=0;
		return;
	}
	if (m_nFixedWidth)
	{
		m_nWidth = (uint16)(m_fScale * (float)m_nFixedWidth);
	}
	else
	{
		float fi = (float)m_nIndent * m_fScale;
		m_nWidth = (uint16)m_pString->GetWidth() + (uint16)fi;
	}

	m_nHeight = (uint16)m_pString->GetHeight();
}

// TRON: makes (or removes) the pulse's copy of the text (RETAIL: 0x100bf430)
void CLTGUITextCtrl::SetPulse(bool bPulse)
{
	if (!m_pString)
		return;

	if (m_pPulseString)
	{
		g_pFontManager->DestroyPolyString(m_pPulseString);
		m_pPulseString = LTNULL;
	}

	if (!bPulse)
		return;

	std::wstring sText = m_pString->GetText();
	m_pPulseString = g_pFontManager->CreateFormattedPolyString(m_pFont,sText,(float)m_pos.x,(float)m_pos.y);
	m_pPulseString->SetCharScreenHeight(m_nFontSize);
	m_pPulseString->SetColor(0xFFFFFFFF);
}

// TRON: starts the pulse when selected (RETAIL: 0x100bf580)
void CLTGUITextCtrl::Pulse()
{
	if (m_bSelected)
		m_fPulse = 1.0f;
}

// Render the control
void CLTGUITextCtrl::Render()
{
	// Sanity checks...
	if (!IsVisible() || !m_pString) return;

    uint32 argbColor=GetCurrentColor();
	m_pString->SetColor(argbColor);

	// TRON: the pulse: the text's copy grows from it and fades out over a
	// second (RETAIL: 0x100bf590)
	if (m_pPulseString && m_fPulse > 0.0f)
	{
		float fGrowth = 1.0f - m_fPulse;
		m_fPulse -= (1.0f / 60.0f);
		uint8 nAlpha = (uint8)(m_fPulse * 192.0f);
		m_pPulseString->SetColor((nAlpha << 24) | (m_argbNormal & 0x00FFFFFF));

		int nLen = m_pString->GetLength();
		LT_POLYGT4* pSrc = m_pString->GetPolys();
		LT_POLYGT4* pDst = m_pPulseString->GetPolys();
		for (int i = 0; i < nLen; i++)
		{
			float fHeight = pSrc[i].verts[3].y - pSrc[i].verts[0].y;
			float fGrow = fHeight * fGrowth;
			g_pDrawPrim->SetXYWH(&pDst[i], pSrc[i].verts[0].x, pSrc[i].verts[0].y - fGrow,
				pSrc[i].verts[1].x - pSrc[i].verts[0].x, fGrow + fGrow + fHeight);
		}

		if (m_fPulse > 0.0f)
			m_pPulseString->Render();
	}

	if (m_nFixedWidth && m_bClip)
	{
		CUIRECT rc;
		rc.x = (float)m_pos.x;
		rc.y = (float)m_pos.y;
		rc.width = (float)m_nWidth;
		rc.height = (float)m_nHeight;
		m_pString->RenderClipped(&rc);
	}
	else
		m_pString->Render();
}


// Enter was pressed
LTBOOL CLTGUITextCtrl::OnEnter ( )
{
	// Send the command
	if ( m_pCommandHandler && m_pCommandHandler->SendCommand(m_nCommandID, m_nParam1, m_nParam2) )
		return LTTRUE;
    return LTFALSE;
}

void CLTGUITextCtrl::SetBasePos ( LTIntPt pos )
{ 
	CLTGUICtrl::SetBasePos(pos);
	if (m_pString)
	{
		float fi = (float)m_nIndent * m_fScale;
		m_pString->SetPosition(m_pos.x+(int)fi,m_pos.y);	// TRON: integers
	}
}

void CLTGUITextCtrl::SetScale(float fScale)
{
	CLTGUICtrl::SetScale(fScale);
	m_nFontSize = (uint8)(m_fScale * (float)m_nBaseFontSize);
	uint16 nWidth = 0;
	if (m_nFixedWidth && !m_bClip)
	{
		nWidth = (uint16)(m_fScale * (float)(m_nFixedWidth-m_nIndent));
	}
	if (m_pString)
	{
		float fi = (float)m_nIndent * m_fScale;
		m_pString->SetPosition(m_pos.x+(int)fi,m_pos.y);	// TRON: integers
		m_pString->SetCharScreenHeight(m_nFontSize);
		m_pString->SetWrapWidth(nWidth);
	}
	CalculateSize();

}


void CLTGUITextCtrl::SetFixedWidth(uint16 nWidth, LTBOOL bClip)
{
	uint16 nMinWidth = m_nBaseFontSize + m_nIndent + 1;
	ASSERT(nWidth == 0 || nWidth >= nMinWidth);
	if (nWidth > 0 && nWidth < nMinWidth)
		nWidth = nMinWidth;

	m_nFixedWidth = nWidth;
	m_bClip = bClip;

	uint16 nTrueWidth = 0;
	if (m_nFixedWidth && !m_bClip)
	{
		nTrueWidth = (uint16)(m_fScale * (float)(m_nFixedWidth-m_nIndent));
	}
	if (m_pString)
	{
		m_pString->SetWrapWidth(nTrueWidth);
		// TRON: a clipped string clips itself (RETAIL: 0x100bf870)
		if (m_bClip)
			m_pString->EnableClipping(true);
	}
	CalculateSize();

}

void CLTGUITextCtrl::SetIndent(uint16 nIndent)
{
	if (m_nFixedWidth)
	{
		ASSERT(m_nFixedWidth > nIndent);
		if(m_nFixedWidth <= nIndent)
			return;
	}

	m_nIndent = nIndent;

	if (m_nFixedWidth && !m_bClip)
	{
		uint16 nTrueWidth = (uint16)(m_fScale * (float)(m_nFixedWidth-m_nIndent));
		if (m_pString)
		{
			m_pString->SetWrapWidth(nTrueWidth);
		}

	}

	if (m_pString)
	{
		float fi = (float)m_nIndent * m_fScale;
		m_pString->SetPosition(m_pos.x+(int)fi,m_pos.y);	// TRON: integers
	}

	CalculateSize();

}
