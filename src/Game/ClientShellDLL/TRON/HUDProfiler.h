// ----------------------------------------------------------------------- //
//
// MODULE  : HUDProfiler.h
//
// PURPOSE : HUDItem showing the name of what profiles the player, with a
//           flickering copy of it while the profiling goes on
//
// RETAIL: Tron 2.0's, in no source release; written from retail CShell.dll
// (0x10032670..0x10032d70, its vtable 0x101e7080). The class name is
// retail's (its RTTI); the member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_PROFILER_H
#define __HUD_PROFILER_H

#include "HUDItem.h"

class CUIFormattedPolyString;

class CHUDProfiler : public CHUDItem
{
public:
	CHUDProfiler();
	~CHUDProfiler();

	LTBOOL		Init();
	void		Term();

	// RETAIL: the commonest slot 3 (retail 0x10023440, folded with
	// CHUDVersion's)
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
	void		Flicker();
	void		Scramble();

	LTFLOAT					m_fScale;			// +0x10
	bool					m_bTronUnknown14;	// +0x14 (cleared only)
	CUIFormattedPolyString*	m_pText;			// +0x18
	CUIFormattedPolyString*	m_pFlicker;			// +0x1c
	LTFLOAT					m_fLastTime;		// +0x20
	LTFLOAT					m_fFlickerTimer;	// +0x24
	LTFLOAT					m_fFlickerTime;		// +0x28
	bool					m_bProfiling;		// +0x2c
	LTIntPt					m_Pos;				// +0x30
	LTFLOAT					m_fJitter;			// +0x38
	char					m_szName[1024];		// +0x3c
};

#endif
