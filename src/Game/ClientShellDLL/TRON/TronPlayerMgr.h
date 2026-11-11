// ----------------------------------------------------------------------- //
//
// MODULE  : TronPlayerMgr.h
//
// PURPOSE : Definition of class to handle managment of player and camera.
//			 Derived from CPlayerMgr
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TRON_PLAYER_MGR_H__
#define __TRON_PLAYER_MGR_H__

#include "PlayerMgr.h"
#include "TronTargetMgr.h"

class CTronPlayerMgr;
extern CTronPlayerMgr* g_pTronPlayerMgr;

// RETAIL: Tron's (retail CShell.dll's CTronPlayerMgr, vtable 0x101e9104,
// 0x238 bytes or more: constructor 0x100a4350, Init 0x100a4460). Build 69's
// (May 2002) energy transfer is there, as four transfers (+0x1e8..+0x1eb),
// each of the first three with a looping sound (+0x22c, +0x224, +0x228).
// Most of its virtuals are still CPlayerMgr's stand-ins or not yet written
// from retail (PlayerMgr.h has the slots).

class CTronPlayerMgr : public CPlayerMgr
{
public:
	CTronPlayerMgr();
	virtual ~CTronPlayerMgr();

	LTBOOL Init();
	void Term();

	virtual float GetJumpVelocity(float fJumpVel, float fSuperJumpVel);

    LTBOOL	OnMessage(uint8 messageID, ILTMessage_Read *pMsg);

	LTBOOL	OnCommandOn(int command);
	LTBOOL	OnCommandOff(int command);

	void	OnEnterWorld();

	void	Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState);
	void	Load(ILTMessage_Read *pMsg, SaveDataState eLoadDataState);

	void	Pause(LTBOOL bPause);
	void	Update();

	void	HandleMsgStartEnergyTransfer(ILTMessage_Read * pMsg);

	CTronTargetMgr * GetTronTargetMgr() {return (CTronTargetMgr*)m_pTargetMgr;}
	virtual void InitTargetMgr();

	// RETAIL: CPlayerMgr's pure slots (0x3c, 0x40, 0x88..0x98), Tron's: stand-ins
	// until written from retail CTronPlayerMgr's (vtable 0x101e9104)
	virtual bool TronUnknownVirtual3C(bool b);
	virtual void TronUnknownVirtual40() {}
	virtual bool TronUnknownVirtual88() { return m_bTronUnknown236; }	// RETAIL: 0x100a4410
	virtual void TronUnknownVirtual8C() {}
	virtual void TronUnknownVirtual90() {}
	virtual void TronUnknownVirtual94() {}
	virtual void TronUnknownVirtual98() {}

	int		GetEnergyRequired() {return (int)m_fEnergyRequired;}
	float	GetPercentEnergyTransferred();

	// RETAIL: any of the four transfers
	bool	IsTransferringEnergy() {return (m_bTransferringEnergy || m_bTronUnknown1E9 || m_bTronUnknown1EA || m_bTronUnknown1EB);}

	// RETAIL: Tron's EndZoom (retail 0x100a54d0, slot 0xb4)
	void	EndZoom();

	// RETAIL: Tron's accessors (retail 0x100a5530, 0x100a5540; the names
	// are ours)
	bool	GetTronUnknown204();
	float	GetTronUnknown20C();

private:
	// RETAIL: the offsets are retail's; names ours where not build 69's
	bool	m_bTransferringEnergy;		// +0x1e8 (with m_hTronUnknownSound22C)
	bool	m_bTronUnknown1E9;			// +0x1e9 (with m_hTronUnknownSound224)
	bool	m_bTronUnknown1EA;			// +0x1ea (with m_hTronUnknownSound228)
	bool	m_bTronUnknown1EB;			// +0x1eb
	float	m_fEnergyTransferred;		// +0x1ec
	float	m_fEnergyRequired;			// +0x1f0
	uint32	m_nTronUnknown1F4;			// +0x1f4

	LTBOOL				m_bPause;			// +0x1f8
	float				m_fLastUpdateTime;	// +0x1fc

	HOBJECT	m_hTronUnknown200;			// +0x200: the transfer's target
	bool	m_bTronUnknown204;			// +0x204
	bool	m_bTronUnknown205;			// +0x205
	uint32	m_nTronUnknown208;			// +0x208
	float	m_fTronUnknown20C;			// +0x20c
	bool	m_bTronUnknown210;			// +0x210: zoomed with the LOL
	bool	m_bTronUnknown211;			// +0x211: the triangulate subroutine is on
	bool	m_bTronUnknown212;			// +0x212: its version
	float	m_fFOVZoomLOL;				// +0x214 (layout "FOVZoomLOL")
	float	m_fFOVZoomTriangulate1;		// +0x218
	float	m_fFOVZoomTriangulate2;		// +0x21c
	float	m_fFOVZoomTriangulate3;		// +0x220
	HLTSOUND	m_hTronUnknownSound224;	// +0x224
	HLTSOUND	m_hTronUnknownSound228;	// +0x228
	HLTSOUND	m_hTronUnknownSound22C;	// +0x22c
	uint32	m_nTronUnknown230;			// +0x230
	bool	m_bTronUnknown234;			// +0x234
	bool	m_bTronUnknown235;			// +0x235
	bool	m_bTronUnknown236;			// +0x236
};

#endif
