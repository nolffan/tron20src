// ----------------------------------------------------------------------- //
//
// MODULE  : DuplicateSubroutineDlg.h
//
// PURPOSE : The "DUPLICATE CODE DETECTED!" box: picking up a subroutine
//           the player already has shows both copies (version and
//           condition) side by side, each with a KEEP button.
//
// RETAIL: Tron's (retail CShell.dll 0x1001cb70..0x1001ea73: no vtable, no
// RTTI; the class, member and function names are ours). 0x770 bytes:
// CTronInterfaceMgr has one at +0x28c48 (for subroutines picked up in the
// game), CScreenSubroutines another.
//
// ----------------------------------------------------------------------- //

#ifndef __DUPLICATESUBROUTINEDLG_H__
#define __DUPLICATESUBROUTINEDLG_H__

#include "iltdrawprim.h"
#include "LightLine.h"
#include "ltrect.h"

struct PlayerSubroutine;
class CUIFont;
class CUIPolyString;

class CDuplicateSubroutineDlg
{
public:
	CDuplicateSubroutineDlg();
	~CDuplicateSubroutineDlg();

	void	Init();
	void	Term();

	// shows the two copies (the first is the one the player has)
	void	Show(PlayerSubroutine* pFirst, PlayerSubroutine* pSecond);
	void	Hide();

	void	Draw();

	// highlights the KEEP button under the cursor
	void	OnMouseMove(int x, int y);

	// the copy whose KEEP button is at x, y (LTNULL if none)
	PlayerSubroutine*	GetKeptSubroutine(int x, int y);

	bool	IsVisible() const { return m_bVisible; }

private:
	PlayerSubroutine*	m_pFirst;			// +0x0
	PlayerSubroutine*	m_pSecond;			// +0x4
	bool			m_bInit;				// +0x8
	bool			m_bVisible;				// +0x9
	bool			m_bOverFirstKeep;		// +0xa
	bool			m_bOverSecondKeep;		// +0xb
	LTRect			m_rcFirstKeep;			// +0xc
	LTRect			m_rcSecondKeep;			// +0x1c
	float			m_fKeepW;				// +0x2c the KEEP strings' size
	float			m_fKeepH;				// +0x30
	float			m_fBuildW;				// +0x34 "build"
	float			m_fBuildH;				// +0x38
	float			m_fConditionW;			// +0x3c "condition"
	float			m_fConditionH;			// +0x40
	HTEXTURE		m_hIcon;				// +0x44 the subroutine's sprite
	HTEXTURE		m_hFirstRing;			// +0x48 the copies' version rings
	HTEXTURE		m_hSecondRing;			// +0x4c
	CLightLine		m_Frame[4];				// +0x50 top, bottom, left, right
	LT_POLYF4		m_Back;					// +0x140
	LT_POLYFT4		m_FirstIcon;			// +0x174
	LT_POLYFT4		m_SecondIcon;			// +0x1c8
	// the lines from each icon to its KEEP button
	LT_POLYF4		m_FirstLines[7];		// +0x21c
	LT_POLYF4		m_SecondLines[7];		// +0x388
	LT_POLYF4		m_FirstKeep[2];			// +0x4f4 the KEEP buttons
	LT_POLYF4		m_SecondKeep[2];		// +0x55c
	LT_POLYG4		m_Bars[6];				// +0x5c4 behind the build and condition rows
	CUIFont*		m_pFont;				// +0x744
	CUIPolyString*	m_pTitle;				// +0x748
	CUIPolyString*	m_pName;				// +0x74c
	CUIPolyString*	m_pBuild;				// +0x750
	CUIPolyString*	m_pCondition;			// +0x754
	CUIPolyString*	m_pFirstVersion;		// +0x758
	CUIPolyString*	m_pSecondVersion;		// +0x75c
	CUIPolyString*	m_pFirstState;			// +0x760
	CUIPolyString*	m_pSecondState;			// +0x764
	CUIPolyString*	m_pFirstKeepStr;		// +0x768
	CUIPolyString*	m_pSecondKeepStr;		// +0x76c
};

#endif // __DUPLICATESUBROUTINEDLG_H__
