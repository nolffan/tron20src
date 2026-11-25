// ----------------------------------------------------------------------- //
//
// MODULE  : HUDTronDecision.h
//
// PURPOSE : Tron's decision window: the bins of a download (subroutines,
//           procedurals, primitives, permission sets, e-mail, help files
//           and video archives) that the player can take
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1003be20..
// 0x1003e2c0; CHUDTronDecision's vtable 0x101e71cc). The class name is
// retail's (RTTI); the bin class and every member name are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_TRON_DECISION_H
#define __HUD_TRON_DECISION_H

#include "HUDDecision.h"

#define MAX_DECISION_BINS	6

// RETAIL: what the server sends for each bin (retail ShowBins builds it on
// the stack and passes it by value, 0x20 bytes). The name is ours.
struct DecisionBinData
{
	DecisionBinData() :
		eType(-1),
		nNameId(0),
		nCost(0),
		nPset(0),
		nVersion(0),
		bWarn(false),
		bForeign(false),
		bUpgrade(false),
		nPlayerVersion(0)
	{
	}

	int		nNumber;		// +0x0: the key that takes it (1..6)
	int		eType;			// +0x4: a RatingTransferType
	uint32	nNameId;		// +0x8
	int		nCost;			// +0xc: energy
	char	nPset;			// +0x10: the permission set
	int		nVersion;		// +0x14: alpha, beta or gold
	bool	bWarn;			// +0x18: the virus scan found it bad
	bool	bForeign;		// +0x19
	bool	bUpgrade;		// +0x1a: the player has it in another version
	int		nPlayerVersion;	// +0x1c: the version the player has
};

// RETAIL: one bin of the window (0x550 bytes; constructor 0x1003be90)
class CHUDDecisionBin
{
public:
	CHUDDecisionBin();
	~CHUDDecisionBin() { Term(); }

	void	Reset();
	void	Term();

	void	Init(DecisionBinData data);

	void	GetSize(int &nWidth, int &nHeight);
	void	SetPosition(int x, int y);

	void	Render();
	void	Update();

	void	StartDownload();
	void	StopDownload();

	uint32	GetTypeName(int eType);
	uint32	GetVersionName(int nVersion);

	// can the player take it (the name is ours)
	bool	IsAvailable() { return m_bActive && !m_bDownloading && !m_bDone; }
	bool	IsDownloading() { return m_bDownloading; }
#ifdef TRON20SRC_MODERN
	void	PadHighlight(bool bOn);		// MODERN.md CL-22
#endif

	bool			m_bActive;			// +0x0
	bool			m_bDownloading;		// +0x1
	bool			m_bDone;			// +0x2
	float			m_fPercent;			// +0x4
	float			m_fDownloadTime;	// +0x8
	float			m_fElapsed;			// +0xc
	int				m_nTronUnknown10;	// +0x10: not used
	float			m_fFlash;			// +0x14
	int				m_nCost;			// +0x18
	HTEXTURE		m_hIcon;			// +0x1c
	HTEXTURE		m_hProgressTex;		// +0x20
	HTEXTURE		m_hNumberTex;		// +0x24
	LT_POLYFT4		m_IconPoly;			// +0x28
	LT_POLYFT4		m_ProgressPoly;		// +0x7c
	LT_POLYFT4		m_NumberPoly;		// +0xd0
	CTexturedFrame	m_Frame;			// +0x124
	CUIFormattedPolyString*	m_pTypeStr;		// +0x53c
	CUIFormattedPolyString*	m_pNameStr;		// +0x540
	CUIFormattedPolyString*	m_pCostStr;		// +0x544
	CUIFormattedPolyString*	m_pUpgradeStr;	// +0x548
	CUIFormattedPolyString*	m_pNumberStr;	// +0x54c
};

class CHUDTronDecision : public CHUDDecision
{
public:
	CHUDTronDecision();

	virtual void	Term();
	virtual void	Render();
	virtual void	UpdateLayout();
	virtual void	Hide(uint8 nChoice);

	// RETAIL: not virtual (CTronInterfaceMgr calls them directly)
	void	Show(ILTMessage_Read *pMsg);
	void	Choose(uint8 nChoice);

	// RETAIL: read by CMenuMgr::OnCommandOff (0x10171c18); the name is ours
	bool	GetTronUnknown4D() { return m_bTronUnknown4d; }
#ifdef TRON20SRC_MODERN
	// MODERN.md CL-22: the controller's highlight, over the bins (or the
	// choices, when the window is a plain decision)
	bool	PadIsChoice(int nChoice);
	void	PadHighlight(int nChoice);
#endif

protected:
	void	ShowBins(ILTMessage_Read *pMsg, bool bFree);
	void	UpdateBins();

	float			m_fCost[MAX_DECISION_BINS];		// +0x1f6c
	CHUDDecisionBin	m_Bins[MAX_DECISION_BINS];		// +0x1f84
	bool			m_bDownloading;					// +0x3f64
	HLTSOUND		m_hDownloadSnd;					// +0x3f68
	bool			m_bWarn;						// +0x3f6c
	CTexturedFrame	m_WarnFrame;					// +0x3f70
	CUIPolyString*	m_pWarnStr;						// +0x4388
};

extern CHUDTronDecision* g_pTronDecision;

#endif
