// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenJoinServerList.h
//
// PURPOSE : Tron's server list: the GameSpy internet and LAN browser
//
// RETAIL: Tron's (RTTI CScreenJoinServerList, vtable 0x101e7b84, sizeof
// 0xb7c; retail CShell.dll 0x10062af0..0x10064fb0). It replaces NOLF2's
// CScreenJoin/CScreenJoinLAN (retail has neither). Written from the retail
// code: the member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENJOINSERVERLIST_H__
#define __SCREENJOINSERVERLIST_H__

#include "BaseScreenJoin.h"

class CScreenJoinServerList : public CBaseScreenJoin
{
public:
	CScreenJoinServerList();
	virtual ~CScreenJoinServerList();

	virtual void	Term();
	virtual LTBOOL	Build();
	virtual void	Escape();
	virtual LTBOOL	Render(HSURFACE hDestSurf);
	virtual void	OnFocus(LTBOOL bFocus);
	virtual LTBOOL	HandleKeyDown(int key, int rep);
	virtual void	ScreenDimsChanged();
	virtual void	HandleCallback(uint32 dwParam1, uint32 dwParam2);

	// the GameSpy browser's news (retail 0x10062af0)
	void	AddServer(SBServer hServer);
	void	UpdateState();

protected:
	virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	virtual LTBOOL	OnEnter();

	void	FindServers();		// retail 0x10064220
	void	StopUpdate();		// retail 0x100643c0
	void	SortServers();		// retail 0x10064400: refills the list
	bool	FilterServer(SBServer hServer);	// retail 0x10064aa0
	void	SetSort(int nField);

	CLTGUIButton*		m_pRefresh;			// +0xa74
	CLTGUITextCtrl*		m_pNameHeader;		// +0xa78
	CLTGUITextCtrl*		m_pPingHeader;
	CLTGUITextCtrl*		m_pPlayersHeader;
	CLTGUITextCtrl*		m_pTypeHeader;
	CLTGUITextCtrl*		m_pMapHeader;
	CLTGUIListCtrl*		m_pServerListCtrl;	// +0xa8c
	CLTGUIColumnCtrl*	m_pAddressCtrl;		// +0xa90: join an address (internet)
	CLTGUIColumnCtrl*	m_pPortCtrl;		// +0xa94: the LAN port (LAN)
	CString				m_sAddress;			// +0xa98
	CString				m_sPort;			// +0xa9c
	CLTGUITextCtrl*		m_pFilterLabel;		// +0xaa0
	CLTGUICycleCtrl*	m_pPingFilter;		// +0xaa4
	uint8				m_nPingFilter;
	CLTGUICtrl*			m_pDedicatedFilter;	// +0xaac: not made in Tron
	uint8				m_nDedicatedFilter;
	CLTGUICycleCtrl*	m_pPlayersFilter;	// +0xab4
	uint8				m_nPlayersFilter;
	CLTGUICycleCtrl*	m_pTypeFilter;		// +0xabc
	uint8				m_nTypeFilter;
	CLTGUICtrl*			m_pTronUnknownAC4;
	uint8				m_nTronUnknownAC8;
	char				m_szVersion[19];	// +0xac9: "major.minor"
	int					m_nStatus;			// +0xadc: the browser's state
	int					m_nLastStatus;
	CLTGUITextCtrl*		m_pStatus;			// +0xae4
	int					m_nSortField;		// +0xae8
	int					m_nSortDir;			// 0 up, 1 down, 2 none
	int					m_nPort;			// +0xaf0
	int					m_nLockColumn;		// +0xaf4
	int					m_nDedicatedColumn;
	int					m_nNameColumn;
	int					m_nPingColumn;
	int					m_nPlayersColumn;
	int					m_nTypeColumn;
	int					m_nMapColumn;
	float				m_fRefreshTimer;	// +0xb10
	float				m_fLastTime;
	HTEXTURE			m_hLocked;			// +0xb18
	HTEXTURE			m_hDedicated;
	HTEXTURE			m_hSortUp;
	HTEXTURE			m_hSortDown;
	LT_POLYFT4			m_SortPoly;			// +0xb28
};

#endif // __SCREENJOINSERVERLIST_H__
