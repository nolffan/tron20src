// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIFrame.cpp
//
// PURPOSE : Simple resizeable frame control
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUIFrame::CLTGUIFrame()
{

	m_hFrame   = LTNULL;

	m_nWidth			= 0;
	m_nHeight			= 0;
	m_nBaseWidth		= 0;
	m_nBaseHeight		= 0;

	m_bEnabled = LTFALSE;

	m_bSimpleStretch = LTFALSE;

	m_fTextureScale = 1.0f;

	m_argbColor = 0xFFFFFFFF;

	memset(m_Poly,0,sizeof(m_Poly));
	memset(m_Border,0,sizeof(m_Border));

	m_nBorderWidth = 0;
	m_BorderColor = 0xFF000000;

	// TRON: (RETAIL: 0x100bdc80)
	m_bHollow = false;
	m_bScaleSourceRects = true;
	m_bUseSourceRects = false;

}

CLTGUIFrame::~CLTGUIFrame()
{
	Destroy();
}

// Create the control
LTBOOL CLTGUIFrame::Create (	HTEXTURE hFrame, uint16 nWidth, uint16 nHeight, LTBOOL bSimpleStretch)
{
	if (!hFrame || !nHeight || !nWidth) return LTFALSE;

	m_hFrame	= hFrame;

	m_nBaseWidth = nWidth;
	m_nBaseHeight = nHeight;
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	m_bSimpleStretch = bSimpleStretch;
	m_argbColor = 0xFFFFFFFF;

	InitPolies();
    m_bCreated=LTTRUE;

    return LTTRUE;
}

LTBOOL CLTGUIFrame::Create (uint32 argbColor, uint16 nWidth, uint16 nHeight)
{
	if (!nHeight || !nWidth) return LTFALSE;

	m_hFrame	= LTNULL;

	m_nBaseWidth = nWidth;
	m_nBaseHeight = nHeight;
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	m_bSimpleStretch = LTTRUE;

	m_argbColor = argbColor;


	InitPolies();
    m_bCreated=LTTRUE;

    return LTTRUE;

}

// Render the control
void CLTGUIFrame::Render()
{
	// Sanity checks...
	if (!IsVisible()) return;

	g_pDrawPrim->SetTexture(m_hFrame);

	// set up the render state	
	SetRenderState();

	// draw our button
	if (m_bSimpleStretch)
	{
		g_pDrawPrim->DrawPrim(m_Poly);
	}
	else if (m_bHollow)
	{
		// TRON: a hollow frame skips its centre cell (RETAIL: 0x100bdeb0)
		g_pDrawPrim->DrawPrim(m_Poly,4);
		g_pDrawPrim->SetTexture(m_hFrame);
		g_pDrawPrim->DrawPrim(&m_Poly[5],4);
	}
	else
	{
		g_pDrawPrim->DrawPrim(m_Poly,9);
	}

	if (m_nBorderWidth)
	{
		for (int f = 0;f < 4; ++f)
			g_pDrawPrim->SetRGBA(&m_Border[f],m_BorderColor);

		// draw our frames
		g_pDrawPrim->DrawPrim(m_Border,4);
	}


}

void CLTGUIFrame::SetBasePos ( LTIntPt pos )
{ 
	CLTGUICtrl::SetBasePos(pos);
	ScalePolies();
}

void CLTGUIFrame::SetScale(float fScale)
{
	CLTGUICtrl::SetScale(fScale);
	m_nWidth = (uint16)((float)m_nBaseWidth * m_fScale);
	m_nHeight = (uint16)((float)m_nBaseHeight * m_fScale);
	ScalePolies();

}

void CLTGUIFrame::SetTextureScale(float fScale)
{
	m_fTextureScale = fScale;
	ScalePolies();

}

void CLTGUIFrame::InitPolies()
{
	ScalePolies();

	// TRON: (RETAIL: 0x100be030)
	if (m_bUseSourceRects)
	{
		InitSourceRectPolies();
		return;
	}

	if (m_bSimpleStretch)
	{
		SetupQuadUVs(m_Poly[0], m_hFrame, 0.0f, 0.0f, 1.0f, 1.0f);
		g_pDrawPrim->SetRGBA(&m_Poly[0],m_argbColor);
	}
	else
	{
		float uv[4] = {0.0f, 0.25f, 0.75f, 1.0f};


		for (int r = 0; r < 3; r++)
		{
			for (int c = 0; c < 3; c++)
			{
				int i = r*3+c;
				g_pDrawPrim->SetRGBA(&m_Poly[i],0xFFFFFFFF);
				SetupQuadUVs(m_Poly[i], m_hFrame, uv[c], uv[r], uv[c+1] - uv[c], uv[r+1] - uv[r]);
			}
		}
	}


}

void CLTGUIFrame::ScalePolies()
{
	// TRON: (RETAIL: 0x100be2b0)
	if (m_bUseSourceRects)
	{
		ScaleSourceRectPolies();
		return;
	}

	uint32 tw,th;
	float fx = (float)m_pos.x;
	float fy = (float)m_pos.y;

	if (m_bSimpleStretch)
	{
		float fw = (float)m_nWidth;
		float fh = (float)m_nHeight;
		g_pDrawPrim->SetXYWH(&m_Poly[0],fx,fy,fw,fh);
	}
	else
	{
		if (!m_hFrame) return;
		g_pTexInterface->GetTextureDims(m_hFrame,tw,th);
		float fw = (float)tw;
		float fh = (float)th;

		float x[4] = {fx, fx+fw*0.25f, (fx+m_nWidth) - fw*0.25f, (fx+m_nWidth) };
		float y[4] = {fy, fy+fh*0.25f, (fy+m_nHeight) - fh*0.25f, (fy+m_nHeight) };


		for (int r = 0; r < 3; r++)
		{
			for (int c = 0; c < 3; c++)
			{
				int i = r*3+c;
				g_pDrawPrim->SetXY4(&m_Poly[i],x[c],y[r],x[c+1],y[r],x[c+1],y[r+1],x[c],y[r+1]);
			}
		}
	}

	float BorderW = ((float)m_nBorderWidth * m_fScale);

	//top
	fx = (float)m_pos.x - BorderW;
	fy = (float)m_pos.y - BorderW;
	float fw = (float)m_nWidth + BorderW;
	float fh = BorderW;
	g_pDrawPrim->SetXYWH(&m_Border[0],fx,fy,fw,fh);

	//right
	fx = (float)(m_pos.x + m_nWidth);
	fy = (float)m_pos.y - BorderW;
	fw = (float)BorderW;
	fh = (float)GetHeight() + BorderW;
	g_pDrawPrim->SetXYWH(&m_Border[1],fx,fy,fw,fh);

	//bottom
	fx = (float)m_pos.x;
	fy = (float)(m_pos.y + GetHeight());
	fw = (float)m_nWidth + BorderW;
	fh = BorderW;
	g_pDrawPrim->SetXYWH(&m_Border[2],fx,fy,fw,fh);

	//left
	fx = (float)m_pos.x - BorderW;
	fy = (float)m_pos.y;
	fw = (float)BorderW;
	fh = (float)GetHeight() + BorderW;
	g_pDrawPrim->SetXYWH(&m_Border[3],fx,fy,fw,fh);


}

void CLTGUIFrame::SetRenderState()
{
	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
		
}


void CLTGUIFrame::SetSize(uint16 nWidth, uint16 nHeight)
{
	m_nBaseWidth = nWidth;
	m_nBaseHeight = nHeight;
	m_nWidth = (uint16)((float)m_nBaseWidth * m_fScale);
	m_nHeight = (uint16)((float)m_nBaseHeight * m_fScale);
	ScalePolies();
}

void CLTGUIFrame::SetFrame(HTEXTURE hFrame)
{
	m_hFrame	= hFrame;
	m_argbColor = 0xFFFFFFFF;
	InitPolies();
	ScalePolies();
}

void CLTGUIFrame::SetColor(uint32 argbColor)
{
	m_hFrame	= LTNULL;
	m_argbColor = argbColor;
	// TRON: colours all nine cells (RETAIL: 0x100be710)
	for (int i = 0; i < 9; i++)
		g_pDrawPrim->SetRGBA(&m_Poly[i],argbColor);
	ScalePolies();
}

// TRON: places the cells of a frame cut from source rectangles: corner
// cells keep their texel size, edges and centre stretch (RETAIL:
// 0x100be760; name ours). The frame reaches one pixel outside the control.
void CLTGUIFrame::ScaleSourceRectPolies()
{
	if (!m_bScaleSourceRects) return;

	int nLeft = m_pos.x - 1;
	int nTop = m_pos.y - 1;
	float fTop = (float)nTop;

	int w0 = m_rcSource[0].right - m_rcSource[0].left + 1;
	int h0 = m_rcSource[0].bottom - m_rcSource[0].top + 1;
	g_pDrawPrim->SetXYWH(&m_Poly[0],(float)nLeft,fTop,(float)w0,(float)h0);

	int y3 = nTop + h0 - 1;
	int x1 = nLeft + w0 - 1;
	int w2 = m_rcSource[2].right - m_rcSource[2].left + 1;
	int h2 = m_rcSource[2].bottom - m_rcSource[2].top + 1;
	int x2 = m_pos.x + m_nWidth - w2 + 2;
	g_pDrawPrim->SetXYWH(&m_Poly[2],(float)x2,fTop,(float)w2,(float)h2);

	int y5 = nTop + h2;
	int h1 = m_rcSource[1].bottom - m_rcSource[1].top + 1;
	g_pDrawPrim->SetXYWH(&m_Poly[1],(float)x1,fTop,(float)(x2 - x1 + 1),(float)h1);

	nLeft = m_pos.x - 1;
	float fLeft = (float)nLeft;
	int w6 = m_rcSource[6].right - m_rcSource[6].left + 1;
	int h6 = m_rcSource[6].bottom - m_rcSource[6].top + 1;
	int y6 = m_pos.y + m_nHeight - h6 + 2;
	g_pDrawPrim->SetXYWH(&m_Poly[6],fLeft,(float)y6,(float)w6,(float)h6);

	// RETAIL: the middle left cell goes to m_Poly[6] (overwriting the
	// bottom left one) and m_Poly[3] is never placed
	int x7 = nLeft + w6 - 1;
	int w3 = m_rcSource[3].right - m_rcSource[3].left + 1;
	g_pDrawPrim->SetXYWH(&m_Poly[6],fLeft,(float)y3,(float)w3,(float)(y6 - y3 + 1));

	int w8 = m_rcSource[8].right - m_rcSource[8].left + 1;
	int h8 = m_rcSource[8].bottom - m_rcSource[8].top + 1;
	int x8 = m_pos.x + m_nWidth - w8 + 2;
	int y8 = m_pos.y + m_nHeight - h8 + 2;
	g_pDrawPrim->SetXYWH(&m_Poly[8],(float)x8,(float)y8,(float)w8,(float)h8);

	int h7 = m_rcSource[7].bottom - m_rcSource[7].top + 1;
	int y7 = m_pos.y + m_nHeight - h7 + 2;
	g_pDrawPrim->SetXYWH(&m_Poly[7],(float)x7,(float)y7,(float)(x8 - x7 + 1),(float)h7);

	int w5 = m_rcSource[5].right - m_rcSource[5].left + 1;
	int x5 = m_pos.x + m_nWidth - w5 + 2;
	g_pDrawPrim->SetXYWH(&m_Poly[5],(float)x5,(float)y5,(float)w5,(float)(y8 - y5));

	int xc = m_pos.x - 1 + (m_rcSource[3].right - m_rcSource[3].left);
	int yc = m_pos.y + (m_rcSource[1].bottom - m_rcSource[1].top) + 1;
	g_pDrawPrim->SetXYWH(&m_Poly[4],(float)xc,(float)yc,(float)(x5 - xc),(float)(y7 - yc));
}

// TRON: the UVs of a frame cut from source rectangles (RETAIL: 0x100beb70;
// name ours)
void CLTGUIFrame::InitSourceRectPolies()
{
	uint32 tw,th;
	g_pTexInterface->GetTextureDims(m_hFrame,tw,th);

	for (int i = 0; i < 9; i++)
	{
		float fu = (float)m_rcSource[i].left / (float)tw;
		float fw = (float)(m_rcSource[i].right - m_rcSource[i].left + 1) / (float)tw;
		float fv = (float)m_rcSource[i].top / (float)th;
		float fh = (float)(m_rcSource[i].bottom - m_rcSource[i].top + 1) / (float)th;
		SetupQuadUVs(m_Poly[i], m_hFrame, fu, fv, fw, fh);
		g_pDrawPrim->SetRGBA(&m_Poly[i],0xFFFFFFFF);
	}
}

// Sets the width of the frames's border, set to 0 to not show the frame
void CLTGUIFrame::SetBorder(uint8 nBorderWidth, uint32 nBorderColor)
{
	m_nBorderWidth = nBorderWidth;
	m_BorderColor = nBorderColor;
	ScalePolies();
}
