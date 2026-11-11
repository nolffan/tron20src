// ----------------------------------------------------------------------- //
//
// MODULE  : HUDLightCycleLives.h
//
// PURPOSE : HUDItem showing the light cycle lives left (n/m)
//
// RETAIL: Tron 2.0's, in no source release; written from retail CShell.dll
// (0x1002d2b0..0x1002dab0, its vtable 0x101e6ed4). The class name is
// retail's (its RTTI); the rest are ours. Its text is built as a wide
// string with Tron's font manager, which this tree has not yet: those
// parts stand in with NOLF2's API.
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_LIGHTCYCLE_LIVES_H
#define __HUD_LIGHTCYCLE_LIVES_H

#include "HUDItem.h"

class CHUDLightCycleLives : public CHUDItem
{
public:
	CHUDLightCycleLives();

    LTBOOL      Init();
	void		Term();

	// RETAIL: the single player light cycle circuit only (retail 0x1002d540)
	virtual bool	IsUsedInGameType(GameType eGameType)
	{
		switch (eGameType)
		{
			case eGameTypeSingleLightCycleCircuit:
				return true;
		}
		return false;
	}

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
	void		UpdateText();

	HTEXTURE		m_hBack;
	LT_POLYFT4		m_Poly;
	CUIPolyString*	m_pText;
	LTFLOAT			m_fAlpha;
	int				m_nFlashCount;
	int				m_nFlashDir;
};

#endif
