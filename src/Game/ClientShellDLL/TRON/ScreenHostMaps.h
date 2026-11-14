// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostMaps.h
//
// PURPOSE : Tron's host screen: the maps of the game type to play
//
// RETAIL: Tron's (RTTI CScreenHostMaps, vtable 0x101e7ae0, sizeof 0xa48;
// retail CShell.dll 0x10060c20..0x10062ad0). Written from the retail code
// after NOLF2's CScreenHostLevels: the member names are NOLF2's or ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENHOSTMAPS_H__
#define __SCREENHOSTMAPS_H__

#include "BaseScreenHost.h"
#include "CommonUtilities.h"

class CScreenHostMaps : public CBaseScreenHost
{
public:
	CScreenHostMaps();
	virtual ~CScreenHostMaps();

	// Build the screen
	virtual LTBOOL	Build();
	virtual void	Escape();

	virtual void	OnFocus(LTBOOL bFocus);
	virtual uint16	SetSelection(uint16 select, LTBOOL bFindSelectable = LTFALSE);
	virtual void	ScreenDimsChanged();

	virtual void	LoadOptions();
	virtual void	SaveOptions();

protected:
	virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	// RETAIL: 0x100618b0: the base screen's edit results
	void	HandleCallback(uint32 dwParam1, uint32 dwParam2);

	// The maps of the game type's folder, sorted (retail 0x100618d0,
	// 0x10061d90)
	LTBOOL	FillAvailList();
	void	FindMaps(const std::string &sDir);

	void	UpdateButtons();					// retail 0x10061fa0
	void	AddMapToList(uint32 nMap);			// retail 0x10062040

	CLTGUITextCtrl*	m_pAdd;					// +0xa14
	CLTGUITextCtrl*	m_pRemove;				// +0xa18
	CLTGUITextCtrl*	m_pAddAll;				// +0xa1c
	CLTGUITextCtrl*	m_pRemoveAll;			// +0xa20
	CLTGUITextCtrl*	m_pMoveDown;			// +0xa24
	CLTGUITextCtrl*	m_pMoveUp;				// +0xa28
	CLTGUIListCtrl*	m_pAvailMissions;		// +0xa2c
	CLTGUIListCtrl*	m_pSelMissions;			// +0xa30
	StringArray		m_saMaps;				// +0xa34: the folder's maps (no extension)
	int				m_nAvailWidth;			// +0xa40
	int				m_nSelWidth;			// +0xa44
};

#endif // __SCREENHOSTMAPS_H__
