// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostConnection.h
//
// PURPOSE : Tron's host screen: the session's name, passwords, port and
//			 bandwidth
//
// RETAIL: Tron's (RTTI CScreenHostConnection, vtable 0x101e79a0, sizeof
// 0xa64; retail CShell.dll 0x1005b9c0..0x1005d9e0). Written from the retail
// code after NOLF2's CScreenHost: the member names are NOLF2's or ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENHOSTCONNECTION_H__
#define __SCREENHOSTCONNECTION_H__

#include "BaseScreenHost.h"

class CScreenHostConnection : public CBaseScreenHost
{
public:
	CScreenHostConnection();
	virtual ~CScreenHostConnection();

	// Build the screen
	virtual LTBOOL	Build();

	virtual void	OnFocus(LTBOOL bFocus);
	virtual void	ScreenDimsChanged();

	virtual LTBOOL	OnLButtonUp(int x, int y);
	virtual LTBOOL	OnRButtonUp(int x, int y);

	virtual void	LoadOptions();
	virtual void	SaveOptions();

protected:
	virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	virtual LTBOOL	OnLeft();
	virtual LTBOOL	OnRight();

	// The edit boxes' answers (retail 0x1005cb00)
	void	HandleEditResult(uint32 dwParam1, uint32 dwParam2);
	// The bandwidth's text (retail 0x1005c910)
	void	UpdateBandwidth();

	CString	m_sSessionName;					// +0xa14
	CString	m_sPassword;					// +0xa18
	CString	m_sScmdPassword;				// +0xa1c
	CString	m_sPort;						// +0xa20
	LTBOOL	m_bUsePassword;					// +0xa24
	LTBOOL	m_bLANOnly;						// +0xa28
	LTBOOL	m_bDedicated;					// +0xa2c
	LTBOOL	m_bAllowScmdCommands;			// +0xa30

	CLTGUIColumnCtrl*	m_pName;			// +0xa34
	CLTGUIToggle*		m_pLANToggle;		// +0xa38
	CLTGUIToggle*		m_pDedicatedToggle;	// +0xa3c
	CLTGUIColumnCtrl*	m_pPassword;		// +0xa40
	CLTGUIToggle*		m_pPassToggle;		// +0xa44
	CLTGUIColumnCtrl*	m_pScmdPassword;	// +0xa48
	CLTGUIToggle*		m_pScmdPassToggle;	// +0xa4c
	CLTGUIColumnCtrl*	m_pPort;			// +0xa50
	CLTGUICycleCtrl*	m_pBandwidthCycle;	// +0xa54
	CLTGUIColumnCtrl*	m_pBandwidth;		// +0xa58
	uint8				m_nBandwidth;		// +0xa5c
	CString				m_sBandwidth;		// +0xa60
};

#endif // __SCREENHOSTCONNECTION_H__
