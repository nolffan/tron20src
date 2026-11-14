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
	void	OnExitWorld();	// RETAIL: 0x100a4d60

	void	Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState);
	void	Load(ILTMessage_Read *pMsg, SaveDataState eLoadDataState);

	void	Pause(LTBOOL bPause);
	void	Update();
	void	UpdatePlaying();	// RETAIL: 0x100a71b0 (slot 0x20)

	// RETAIL: Tron's energy transfer messages (retail OnMessage, 0x100a4b80:
	// 154 starts one, 155 ends it); the names are build 69's / ours
	void	HandleMsgStartEnergyTransfer(ILTMessage_Read * pMsg);
	void	HandleMsgStopEnergyTransfer(ILTMessage_Read * pMsg);

	CTronTargetMgr * GetTronTargetMgr() {return (CTronTargetMgr*)m_pTargetMgr;}
	virtual void InitTargetMgr();

	// RETAIL: CTronPlayerMgr's overrides (retail vtable 0x101e9104)
	virtual void	TronUnknownVirtual24(bool b1D7, bool b1D9);		// RETAIL: 0x100a7e60
	virtual bool	TronUnknownVirtual3C(bool b);					// RETAIL: 0x100a8870
	virtual void	TronUnknownVirtual40();							// RETAIL: 0x100a8270
	virtual bool	ChangeWeapon(uint8 nWeaponId, uint8 nAmmoId = WMGR_INVALID_ID, int dwAmmo = -1);	// RETAIL: 0x100a7b00
	virtual void	ToggleHolster(bool bPlayDeselect);				// RETAIL: 0x100a7b40
	virtual void	DoActivate();									// RETAIL: 0x100a7a20
	virtual void	TronUnknownVirtual50();							// RETAIL: 0x100a7030
	virtual void	TronUnknownVirtual54();							// RETAIL: 0x100a7130
	virtual void	TronUnknownVirtual5C() {}						// RETAIL: an empty one of its own
	virtual void	TronUnknownVirtual60();							// RETAIL: 0x100a7360
	virtual bool	TronUnknownVirtual64();							// RETAIL: 0x100a7390
	virtual void	TronUnknownVirtual68(int nFlags);				// RETAIL: 0x100a74d0
	virtual void	TronUnknownVirtual6C(int nArg);					// RETAIL: 0x100a8040
	virtual bool	TronUnknownVirtual70(HOBJECT hObj, char* pszModel, char* pszSkin);	// RETAIL: 0x100a7c60
	virtual bool	TronUnknownVirtual74(HOBJECT hObj, char* pszBuf, int nBufLen);	// RETAIL: 0x100a7d50
	virtual bool	TronUnknownVirtual78(HOBJECT hObj, char* pszBuf);	// RETAIL: 0x100a7e10
	virtual bool	TronUnknownVirtual7C(HOBJECT hObj, char* psz1, char* psz2, char* psz3);	// RETAIL: 0x100a7cd0
	virtual uint8	TronUnknownVirtual88() { return m_nTronUnknown236; }	// RETAIL: 0x100a4410
	virtual void	TronUnknownVirtual8C();							// RETAIL: 0x100a8050
	virtual void	TronUnknownVirtual90(const char* pszColor, bool bSend, bool bSave);	// RETAIL: 0x100a8080
	virtual bool	TronUnknownVirtual94();							// RETAIL: 0x100a8810
	virtual void	TronUnknownVirtual98(int nArg);					// RETAIL: 0x100a8860
	virtual bool	TronUnknownVirtual9C(void* pData, int nArg);	// RETAIL: 0x100a7660
	virtual int		TronUnknownVirtualA0();							// RETAIL: 0x100a7650
	virtual int		TronUnknownVirtualA4(int nArg);					// RETAIL: 0x100a7920
	virtual void	TronUnknownVirtualA8(bool bHide);				// RETAIL: 0x100a86b0

	int		GetEnergyRequired() {return (int)m_fEnergyRequired;}
	float	GetPercentEnergyTransferred();

	// RETAIL: any of the four transfers
	bool	IsTransferringEnergy() {return (m_bTransferringEnergy || m_bTronUnknown1E9 || m_bTronUnknown1EA || m_bTronUnknown1EB);}

	// RETAIL: the first or the fourth transfer: what CHUDEnergyTransfer's
	// Render shows (inlined there, retail 0x10029430; the name is ours)
	bool	IsShowingEnergyTransfer() {return (m_bTransferringEnergy || m_bTronUnknown1EB);}

	// RETAIL: Tron's EndZoom (retail 0x100a54d0, slot 0xb4)
	void	EndZoom();

	// RETAIL: Tron's accessors (retail 0x100a5530, 0x100a5540; the names
	// are ours)
	bool	GetTronUnknown204();
	float	GetTronUnknown20C();

	// RETAIL: a subroutine's state, as the subroutine manager tells it
	// (retail 0x100a7720; the name is ours)
	void	OnSubroutineChanged(const char* pszName, bool bActive, bool bVersion, int nVersion);	// RETAIL: the version is passed whole (0x10095441)

	// RETAIL: called by the light cycle manager (retail 0x100a7eb0,
	// 0x100a8000; the names are ours)
	bool	TronUnknownLightCycleSpectate();
	void	TronUnknownLightCycleRemoved(HOBJECT hObj);

	// RETAIL: inline: stops watching a light cycle (the light cycle match
	// manager, CShell.dll 0x10016d06, 0x10017190): +0xbc is the watched
	// object, +0xb8 whether one is watched, +0x230 its LIGHT_CYCLIST (as
	// TronUnknownLightCycleSpectate, 0x100a7eb0, sets them). The name is ours
	void	TronUnknownStopSpectating()
	{
		m_hTronUnknownBC = LTNULL;
		m_bTronUnknownB8 = false;
		m_nTronUnknown230 = 0;
	}

	// PORT: Tron overrides UpdateCameraZoom (retail 0x100a4da0, slot 0xac:
	// the LOL's and the triangulate subroutine's FOVs, +0x214..+0x220) and
	// UpdateCameraSway (0x100a52a0, slot 0xb0); not yet written, CPlayerMgr's
	// stand in

	// RETAIL: inlined in CHUDProfiler::Render (retail 0x100328b0): true
	// while the player is being profiled (the name is ours)
	bool	IsTronUnknown205() {return m_bTronUnknown205;}

private:

	// RETAIL: the four transfers' updates (retail 0x100a5690, 0x100a5860,
	// 0x100a5b20, 0x100a5de0; the names are ours)
	void	UpdateEnergyTransfer(float fDeltaTime);
	void	UpdateHealthGain(float fDeltaTime);
	void	UpdateEnergyGain(float fDeltaTime);
	void	UpdateTronUnknownTransfer1EB(float fDeltaTime);

	// RETAIL: messages 193, 200, 152 (retail 0x100a6740, 0x100a6e50,
	// 0x100a6ec0) and a message box (0x100a6ee0); the names are ours
	void	HandleMsgSystemMemory(ILTMessage_Read *pMsg);
	void	HandleMsgTronUnknown200(ILTMessage_Read *pMsg);
	void	HandleMsgTronUnknown152(ILTMessage_Read *pMsg);
	void	ShowTronUnknownMessageBox(ILTMessage_Read *pMsg);

	bool	IsHolsterLocked();	// RETAIL: 0x100a7b70 (the name is ours)

	// RETAIL: the offsets are retail's; names ours where not build 69's
	bool	m_bTransferringEnergy;		// +0x1e8 (with m_hTronUnknownSound22C)
	bool	m_bTronUnknown1E9;			// +0x1e9 (with m_hTronUnknownSound224)
	bool	m_bTronUnknown1EA;			// +0x1ea (with m_hTronUnknownSound228)
	bool	m_bTronUnknown1EB;			// +0x1eb
	float	m_fEnergyTransferred;		// +0x1ec
	float	m_fEnergyRequired;			// +0x1f0
	float	m_fTronUnknown1F4;			// +0x1f4: a rating, from its least to its surge (0..1)

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
	uint32	m_nTronUnknown230;			// +0x230: a light cycle (its +0x3c, an object)
	bool	m_bTronUnknown234;			// +0x234: message 152's
	bool	m_bTronUnknown235;			// +0x235
	uint8	m_nTronUnknown236;			// +0x236: a count (light cycle lives)
};

#endif
