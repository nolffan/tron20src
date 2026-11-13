// ----------------------------------------------------------------------- //
//
// MODULE  : LightLine.h
//
// PURPOSE : A thin line drawn on a menu screen with glints running along it
//
// RETAIL: Tron's (retail CShell.dll 0x100d7bc0..0x100d8460: no RTTI and no
// strings, so the class and member names are ours). The join and host
// screens frame their tabs with eight of these (CBaseScreenJoin +0x818),
// the light cycle screens use them too.
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTLINE_H__
#define __LIGHTLINE_H__

#include "iltdrawprim.h"

// RETAIL: the 0xc4 bytes the glint is built on (its constructor: retail
// 0x100d7bc0; its destructor is empty and out of line). Only the line's
// end points are used by the code we know.
class CLightGlintBase
{
public:
	CLightGlintBase();
	~CLightGlintBase();

	uint32		m_dwTronUnknown0;
	uint32		m_dwTronUnknown4;
	float		m_fX1;			// the line the glint runs along
	float		m_fY1;
	float		m_fX2;
	float		m_fY2;
	uint32		m_dwTronUnknown18;
	LT_POLYF4	m_TronUnknown1C[3];
	uint32		m_dwTronUnknownB8;
	uint32		m_dwTronUnknownBC;
	uint32		m_dwTronUnknownC0;
};

// RETAIL: one glint, sizeof 0x154 (retail's array new: 0x100d83d0)
class CLightGlint : public CLightGlintBase
{
public:
	CLightGlint();

	void	Init(float fX1, float fY1, float fX2, float fY2);
	void	Move(float fX1, float fY1, float fX2, float fY2);
	void	Reset();
	void	Render();

	bool		m_bVertical;	// the glint's quads are upright
	LT_POLYG4	m_Poly[2];		// the glint's two halves, fading outwards
	float		m_fTime;		// time left in this run
	float		m_fPeriod;		// the length of a run
	float		m_fPos;			// where on the line (0..1)
};

// RETAIL: sizeof 0x3c (CBaseScreenJoin's eight at +0x818)
class CLightLine
{
public:
	CLightLine();
	~CLightLine() { Term(); }

	void	Create(float fX1, float fY1, float fX2, float fY2);
	void	Move(float fX1, float fY1, float fX2, float fY2);
	void	Term();
	void	SetColor(uint32 nColor);
	void	Render();

private:
	LT_POLYF4		m_Poly;
	int				m_nGlints;
	CLightGlint*	m_pGlints;
};

#endif // __LIGHTLINE_H__
