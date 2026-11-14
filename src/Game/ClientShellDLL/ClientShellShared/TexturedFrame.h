// ----------------------------------------------------------------------- //
//
// MODULE  : TexturedFrame.h
//
// PURPOSE : A nine-piece frame drawn from one texture (a layout.txt
//           "TexturedFrame%d" record: INT_TEXTUREDFRAME)
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1013e130..
// 0x1013f660). The class's and its members' names are ours. Size 0x418
// (CHUDDecision holds one at +0x1b54).
//
// ----------------------------------------------------------------------- //

#ifndef __TEXTUREDFRAME_H__
#define __TEXTUREDFRAME_H__

#include "LayoutMgr.h"

class CUIPolyString;

// one piece of the frame, in texels: its corner and size
struct TexturedFramePiece
{
	TexturedFramePiece() : x(0), y(0), w(0), h(0) {}

	int		x, y, w, h;
};

class CTexturedFrame
{
public:
	// the pieces, in INT_TEXTUREDFRAME's order (the center is last, so a
	// hollow frame draws the first eight)
	enum Piece
	{
		kTopLeft = 0,
		kTop,
		kTopRight,
		kLeft,
		kRight,
		kBottomLeft,
		kBottom,
		kBottomRight,
		kCenter,
		kNumPieces
	};

	CTexturedFrame();
	~CTexturedFrame();

	// RETAIL: empty (CHUDDecision::Term calls it: folded, 0x100abb80)
	void	Term();

	// the layout's frame of that name (CLayoutMgr::GetTexturedFrame); it
	// becomes the one the "Frame"/"FrameUV" console programs adjust
	void	Init(const char* szName);
	void	Init(INT_TEXTUREDFRAME* pFrame);

	// the console programs' tuning: move one edge of a piece's screen quad,
	// or of its texture rect, by nDelta (edges 0..3: left, top, right,
	// bottom)
	void	AdjustPiece(int nPiece, int nEdge, int nDelta);
	void	AdjustPieceUV(int nPiece, int nEdge, int nDelta);

	void	Render();

	LTIntPt	GetPosition();
	LTIntPt	GetSize();
	LTRect	GetRect();

	void	SetPosition(int x, int y);
	void	SetSize(int w, int h);
	void	SetRect(LTRect rect);

	// RETAIL: inline (CHUDDecision::ShowDecision copies all four)
	LTRect	GetMargins() { return m_rcMargins; }

private:
	void	Update();

	bool		m_bShowPieces;			// +0x0: draws each piece's numbers
	bool		m_bDirty;				// +0x1
	LTIntPt		m_Pos;					// +0x4
	LTIntPt		m_Size;					// +0xc
	LTRect		m_xTronUnknown14;		// +0x14: cleared only
	LTRect		m_xTronUnknown24;		// +0x24: cleared only
	bool		m_bInscribe;			// +0x34
	bool		m_bHollow;				// +0x35
	LTRect		m_rcMargins;			// +0x38
	TexturedFramePiece	m_Piece[kNumPieces];	// +0x48
	LT_POLYFT4	m_Poly[kNumPieces];		// +0xd8
	HTEXTURE	m_hTexture;				// +0x3cc
	CUIPolyString*	m_pPieceStr[kNumPieces];	// +0x3d0: screen rects
	CUIPolyString*	m_pUVStr[kNumPieces];		// +0x3f4: texture rects
};

// RETAIL: 0x1027337c: the frame last initialised by name (the name is ours)
extern CTexturedFrame* g_pTexturedFrame;

#endif // __TEXTUREDFRAME_H__
