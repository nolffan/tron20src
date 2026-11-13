// ----------------------------------------------------------------------- //
//
// MODULE  : HUDProcedurals.h
//
// PURPOSE : HUDItem showing the procedurals at work: each an icon in a
//           frame with a progress bar and its name
//
// RETAIL: Tron 2.0's, in no source release; written from retail CShell.dll
// (0x10031ad0..0x10032463, its vtable 0x101e704c). The class name is
// retail's (its RTTI); the member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_PROCEDURALS_H
#define __HUD_PROCEDURALS_H

#include "HUDItem.h"

class CUIPolyString;

#define HUDPROC_MAX		3

class CHUDProcedurals : public CHUDItem
{
public:
	CHUDProcedurals();

	// RETAIL: folded with CHUDDistance's (retail 0x101613a0)
	LTBOOL		Init() {UpdateLayout(); return LTTRUE;}
	void		Term();

	// RETAIL: the commonest slot 3 (retail 0x10023440)
	virtual bool	IsUsedInGameType(GameType eGameType)
	{
		switch (eGameType)
		{
			case eGameTypeSingle:
			case eGameTypeCooperative:
			case eGameTypeDeathmatch:
			case eGameTypeDiscArena:
			case eGameTypeDiscTournament:
				return true;
		}
		return false;
	}

	void		Render();
	void		Update();

	void		UpdateLayout();

private:
	LTFLOAT					m_fScale;					// +0x10
	bool					m_bShow;					// +0x14
	int						m_nNumActive;				// +0x18
	bool					m_bActive[HUDPROC_MAX];		// +0x1c
	LTFLOAT					m_fProgress[HUDPROC_MAX];	// +0x20
	int						m_nFont;					// +0x2c
	int						m_nFontSize;				// +0x30
	uint32					m_nColor;					// +0x34
	int						m_nSpacing;					// +0x38
	int						m_nTopBound;				// +0x3c
	int						m_nTop;						// +0x40
	int						m_nIconWidth;				// +0x44
	int						m_nIconHeight;				// +0x48
	int						m_nIconOffset;				// +0x4c
	int						m_nPBarXOffset;				// +0x50
	int						m_nPBarYOffset;				// +0x54
	int						m_nPBarWidth;				// +0x58
	int						m_nPBarHeight;				// +0x5c
	int						m_nStringXOffset;			// +0x60
	int						m_nStringYOffset;			// +0x64
	HTEXTURE				m_hFrameTex;				// +0x68
	HTEXTURE				m_hIconTex[HUDPROC_MAX];	// +0x6c
	HTEXTURE				m_hEmptyBarTex;				// +0x78
	HTEXTURE				m_hFullBarTex;				// +0x7c
	CUIPolyString*			m_pName[HUDPROC_MAX];		// +0x80
	LT_POLYFT4				m_Icon[HUDPROC_MAX];		// +0x8c
	LT_POLYFT4				m_Frame[HUDPROC_MAX];		// +0x188
	LT_POLYFT4				m_EmptyBar[HUDPROC_MAX];	// +0x284
	LT_POLYFT4				m_FullBar[HUDPROC_MAX];		// +0x380
};

#endif
