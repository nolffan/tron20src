// ----------------------------------------------------------------------- //
//
// MODULE  : HUDLightCycleRoster.h
//
// PURPOSE : HUDItem listing the light cyclists (their color's icon and
//			 name), flashing those derezzed out of the list
//
// RETAIL: Tron 2.0's, in no source release; written from retail CShell.dll
// (0x1002deb0..0x1002f730, its vtable 0x101e6f48). The class name is
// retail's (its RTTI); the entry's name and the member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_LIGHTCYCLE_ROSTER_H
#define __HUD_LIGHTCYCLE_ROSTER_H

#include "HUDItem.h"

struct LIGHT_CYCLIST;

// RETAIL: one cyclist of the roster, 0x8c bytes (its list's nodes are 0x94)
class CLightCycleRosterEntry
{
public:
	CLightCycleRosterEntry(LIGHT_CYCLIST* pCyclist);			// RETAIL: 0x1002df20
	CLightCycleRosterEntry(const CLightCycleRosterEntry& other);	// RETAIL: 0x1002e160
	~CLightCycleRosterEntry();									// RETAIL: 0x1002e340

	void	Init();												// RETAIL: 0x1002e450
	bool	IsCyclist(LIGHT_CYCLIST* pCyclist);					// RETAIL: 0x1002e660
	// RETAIL: 0x1002e680: by their teams' scores (higher first), then the
	// higher team
	bool	operator<(const CLightCycleRosterEntry& other) const;
	// RETAIL: 0x1002e730, 0x1002e800: a result for mem_fun_ref (always false)
	LTBOOL	Update();
	LTBOOL	Render();
	// RETAIL: 0x1002f0a0: derezzed and done flashing
	bool	IsDone();

	LT_POLYFT4		m_Poly;				// +0x00
	LTFLOAT			m_fAlpha;			// +0x54
	int				m_nFlashCount;		// +0x58
	int				m_nFlashDir;		// +0x5c
	bool			m_bAlive;			// +0x60 (false once derezzed)
	HOBJECT			m_hObject;			// +0x64 the cyclist's
	std::string		m_sColor;			// +0x68 the cyclist's color
	std::string		m_sName;			// +0x74
	uint8			m_nTeam;			// +0x80 (0xff without the team manager)
	HTEXTURE		m_hIcon;			// +0x84 the color's HUDRosterIconTex
	CUIFormattedPolyString*	m_pText;	// +0x88 the name
};

typedef std::list<CLightCycleRosterEntry> LightCycleRosterList;

class CHUDLightCycleRoster : public CHUDItem
{
public:
	CHUDLightCycleRoster();

	LTBOOL		Init();
	void		Term();

	// RETAIL: the single player and light cycle games (retail 0x1002ea20,
	// folded with the power-up icon's and the speedometer's)
	virtual bool	IsUsedInGameType(GameType eGameType)
	{
		switch (eGameType)
		{
			case eGameTypeSingle:
			case eGameTypeSingleLightCycleCircuit:
			case eGameTypeSingleLightCycleCustom:
			case eGameTypeMultiplayerLightcycles:
				return true;
		}
		return false;
	}

	void		Render();
	void		Update();
	void		UpdateLayout();

	// RETAIL: the client light cycle manager's (0x1001106b, 0x10012421,
	// 0x1001219d, 0x100155e2, 0x10014611)
	void		DerezCyclist(LIGHT_CYCLIST* pCyclist);	// RETAIL: 0x1002ee50
	void		AddCyclist(LIGHT_CYCLIST* pCyclist);	// RETAIL: 0x1002eed0
	void		Reset();								// RETAIL: 0x1002efd0

private:
	void		UpdateEntries();						// RETAIL: 0x1002f020

	LightCycleRosterList	m_Entries;					// +0x10
};

// RETAIL: 0x1026ee18 (set by CTronHUDMgr)
extern CHUDLightCycleRoster* g_pLightCycleRoster;

#endif
