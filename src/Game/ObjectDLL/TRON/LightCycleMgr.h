/****************************************************************************
;
;	MODULE:			LightCycleMgr.h
;
;	PURPOSE:		Light Cycle Manager (SERVER) for TRON
;
;	HISTORY:		4/19/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
;	RETAIL: build 69's CLightCycleMgr was a plain class; retail's is the
;	virtual base (RTTI .?AVCLightCycleMgr@@, Object.lto vtable 0x10294bcc,
;	CShell.dll vtable 0x101e572c) of the server's CServerLightCycleMgr
;	(ServerLightCycleMgr.h) and the client's CClientLightCycleMgr. Its code
;	is Object.lto 0x100bb180..0x100be640. The layout below is retail's
;	(constructor 0x100bb180, destructor 0x1004fc60, the cyclist allocation in
;	0x100bb240); the names of the members, of the virtuals not in build 69
;	and of the cyclist's fields are ours. Our offsets are the retail ones
;	given here (CButeMgr has Tron's layout: libs/ButeMgr/butemgr.h).
;
****************************************************************************/

#ifndef _LIGHTCYCLEMGR_H_
#define _LIGHTCYCLEMGR_H_

// stl vector
#include <vector>
#include <string>
#include "LightCycleButeMgr.h"

// Defines
#define CURRENT_TRAIL_ID -1

// RETAIL: the power-up a cyclist has when it has none (LIGHT_CYCLIST::nPowerup;
// 0x100bb2d5, 0x100522a1). The name is ours.
#define LC_NO_POWERUP				12

struct LIGHT_CYCLE_TRAIL_POINT
{
	LTVector vPoint;

	// We're defining accessors for this just in case the underlying datatype
	// or architecture changes... That way, the calling code won't have to change
	float GetX() { return vPoint.x; }
	float GetY() { return vPoint.y; }
	float GetZ() { return vPoint.z; }
	void SetX(float x) { vPoint.x = x; }
	void SetY(float y) { vPoint.y = y; }
	void SetZ(float z) { vPoint.z = z; }
};

// RETAIL: 0x1c bytes, allocated and constructed inline at 0x100bf020
struct LIGHT_CYCLE_TRAIL
{
	LIGHT_CYCLE_TRAIL()
	{
		bTronUnknownC = false;
		nTronUnknown10 = 0;
		fTronUnknown14 = 500.0f;
		fDerezTime = 0.0f;
	}

	// Collection of wall points
	std::vector<LIGHT_CYCLE_TRAIL_POINT> collTrailPoints;	// RETAIL: +0x00

	bool	bTronUnknownC;			// RETAIL: +0x0c
	int		nTronUnknown10;			// RETAIL: +0x10
	float	fTronUnknown14;			// RETAIL: +0x14
	float	fDerezTime;				// RETAIL: +0x18 (set when its cyclist derezzes)
};

// RETAIL: 0x134 bytes, allocated in 0x100bb240 (AddCyclist). Only the fields
// the server's code reads are named; their names are ours.
struct LIGHT_CYCLIST
{
	// RETAIL: inline (debug_new_fn<LIGHT_CYCLIST> 0x100bee20, called by
	// AddCyclist)
	LIGHT_CYCLIST() : nClientID(0), nNameId(0)
	{
		fStartDerezTime = 0.0f;
		bUpdating = true;
		bRemove = false;
		pCurTrail = NULL;
		hObject = LTNULL;
		vForward.Init();
		fMaxTrailLength = 0.0f;
		nCycleType = 0;
		szCycleColor[0] = '\0';
		nRed = 0;
		nGreen = 0;
		nBlue = 0;
		fTronUnknown6C = 0.0f;
		fSpeed = 800.0f;
		fTronUnknown74 = 0.0f;
		fTronUnknown78 = 0.0f;
		fMaxManualSpeedAdd = 1600.0f;
		fMaxExternalSpeedAdd = 2400.0f;
		fMaxTurboSpeed = 5000.0f;
		fMinSpeed = 400.0f;
		fTurboAccelRate = 400.0f;
		fBrakeDecelRate = 400.0f;
		fNormalAccelRate = 100.0f;
		fNormalDecelRate = 100.0f;
		fMaxWallAccelRate = 400.0f;
		fTurnSpeedLossPercent = 0.5f;
		nMaxWallDist = 50;
		bTronUnknownAC = false;
		fHalfHeight = fHalfLength = fHalfWidth = 0.0f;
		bIsPlayer = false;
		nCycleDialog = 0;
		vPos.Init();
		fLastPosUpdateTime = 0.0f;
		vTronUnknownE4.Init();
		nPowerup = LC_NO_POWERUP;
		dwFlags = 0;
		fFlagsTime = 0.0f;
		fFlagsDuration = 1.0f;
		pTronUnknownTrail100 = NULL;
		pTronUnknownTrail104 = NULL;
		vTronUnknown108.Init();
		vTronUnknown114.Init();
		ptTronUnknown120.vPoint.Init(0.0f, 0.0f, 0.0f);
		fTronUnknown12C = 0.0f;
		bTronUnknownD1 = false;
		bTronUnknown130 = false;
	}

	// RETAIL: inline (out of line at 0x10029fd0, for LightCycleMissile's
	// stand-in cyclist). Its "LIGHT_CYCLIST();" builds and destroys a
	// temporary: it does not initialize this cyclist (see
	// docs/RETAIL_BUGS.md). The parameters' names are ours.
	LIGHT_CYCLIST(float fHeight, float fLength, bool bTronUnknown)
	{
		LIGHT_CYCLIST();
		fHalfHeight = fHeight;
		fHalfLength = fLength;
		bTronUnknownD1 = bTronUnknown;
	}

	LIGHT_CYCLE_TRAIL* FindTrail(uint16 wTrailID);

	// Collection of trails (normally just 1 except in special cases)
	std::vector<LIGHT_CYCLE_TRAIL*> collTrails;		// RETAIL: +0x00

	// If we die, here's the time we start
	float fStartDerezTime;							// RETAIL: +0x0c

	// Are we updating this cyclist?
	bool bUpdating;									// RETAIL: +0x10
	bool bRemove;									// RETAIL: +0x11 (RemoveCyclist)

	// Which trail we're currently "making"
	LIGHT_CYCLE_TRAIL* pCurTrail;					// RETAIL: +0x14

	// The cycle's current direction (axis aligned)
	LTVector vForward;								// RETAIL: +0x18
	LTVector vPos;									// RETAIL: +0x24

	// Object associated with this cyclist
	LTObjRef hObject;								// RETAIL: +0x30 (the handle at +0x3c)

	float	fMaxTrailLength;						// RETAIL: +0x40
	char	szCycleColor[32];						// RETAIL: +0x44
	int		nCycleType;								// RETAIL: +0x64
	uint8	nRed;									// RETAIL: +0x68
	uint8	nGreen;									// RETAIL: +0x69
	uint8	nBlue;									// RETAIL: +0x6a
	float	fTronUnknown6C;							// RETAIL: +0x6c
	float	fSpeed;									// RETAIL: +0x70
	float	fTronUnknown74;							// RETAIL: +0x74
	float	fTronUnknown78;							// RETAIL: +0x78
	float	fMaxManualSpeedAdd;						// RETAIL: +0x7c
	float	fMaxExternalSpeedAdd;					// RETAIL: +0x80
	float	fMaxTurboSpeed;							// RETAIL: +0x84
	float	fMinSpeed;								// RETAIL: +0x88
	float	fTurboAccelRate;						// RETAIL: +0x8c
	float	fBrakeDecelRate;						// RETAIL: +0x90
	float	fNormalAccelRate;						// RETAIL: +0x94
	float	fNormalDecelRate;						// RETAIL: +0x98
	float	fMaxWallAccelRate;						// RETAIL: +0x9c
	float	fTurnSpeedLossPercent;					// RETAIL: +0xa0
	int		nTronUnknownA4;							// RETAIL: +0xa4
	int		nMaxWallDist;							// RETAIL: +0xa8
	bool	bTronUnknownAC;							// RETAIL: +0xac
	float	fHalfHeight;							// RETAIL: +0xb0
	float	fHalfLength;							// RETAIL: +0xb4
	float	fHalfWidth;								// RETAIL: +0xb8
	float	fPlayerNoticeDist;						// RETAIL: +0xbc
	float	fFrontSensorDist;						// RETAIL: +0xc0
	float	fRightSensorDist;						// RETAIL: +0xc4
	float	fTronUnknownC8;							// RETAIL: +0xc8
	float	fLeftSensorDist;						// RETAIL: +0xcc
	bool	bIsPlayer;								// RETAIL: +0xd0
	bool	bTronUnknownD1;							// RETAIL: +0xd1 (LightCycleMissile's: hits its own trails, 0x100bbf5c)
	uint32	nClientID;								// RETAIL: +0xd4 (-1 for AI)
	uint32	nNameId;								// RETAIL: +0xd8
	uint32	nCycleDialog;							// RETAIL: +0xdc
	float	fLastPosUpdateTime;						// RETAIL: +0xe0
	LTVector vTronUnknownE4;						// RETAIL: +0xe4
	uint32	nPowerup;								// RETAIL: +0xf0 (LC_NO_POWERUP if none)
	uint32	dwFlags;								// RETAIL: +0xf4
	float	fFlagsTime;								// RETAIL: +0xf8
	float	fFlagsDuration;							// RETAIL: +0xfc
	LIGHT_CYCLE_TRAIL* pTronUnknownTrail100;		// RETAIL: +0x100
	LIGHT_CYCLE_TRAIL* pTronUnknownTrail104;		// RETAIL: +0x104
	LTVector vTronUnknown108;						// RETAIL: +0x108
	LTVector vTronUnknown114;						// RETAIL: +0x114
	LIGHT_CYCLE_TRAIL_POINT ptTronUnknown120;		// RETAIL: +0x120
	float	fTronUnknown12C;						// RETAIL: +0x12c
	bool	bTronUnknown130;						// RETAIL: +0x130
};

// RETAIL: the flags of LIGHT_CYCLIST::dwFlags the server tests (names ours)
#define LCF_TRONUNKNOWN_2			(1<<1)
#define LCF_TRONUNKNOWN_4			(1<<2)
#define LCF_TRONUNKNOWN_20			(1<<5)
#define LCF_TIMED_MASK				(~0x422)	// RETAIL: 0x10050278

// RETAIL: 0x14 bytes on the stack of the trail collision code (0x100524c0,
// 0x100520d0). The fields after pCyclist are ours.
struct LightCycleCollisionInfo
{
	LIGHT_CYCLIST* pCyclist;		// RETAIL: +0x00 (whose trail was hit)
	int nTrail;						// RETAIL: +0x04 (-1 for none)
	int nPoint;						// RETAIL: +0x08
	LTVector vCollisionPoint;		// RETAIL: +0x0c
};

class CLightCycleMgr
{
	public:

		CLightCycleMgr();										// RETAIL: 0x100bb180
		virtual ~CLightCycleMgr() {}							// RETAIL: 0x1004fc60

		virtual bool Init();									// RETAIL: 0x100bb200
		virtual void Term();									// RETAIL: 0x100bb230

		// Creates a new cyclist and adds it to the list. pvDims is given for
		// players. RETAIL: 0x100bb240
		virtual LIGHT_CYCLIST* AddCyclist(HOBJECT hObj, const char* szCycleColor, bool bUpdate,
			const LTVector* pvDims, uint32 nNameId, bool bTronUnknown);

		// Marks the cyclist for removal. RETAIL: 0x100bb7f0
		virtual bool RemoveCyclist(HOBJECT hObj);

		// RETAIL: pure in the base (slot 5)
		virtual bool DerezCyclist(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLIST* pKiller, bool bTronUnknown) = 0;

		// Two cycles ran into each other. RETAIL: 0x100e2870 (empty)
		virtual void HandleHeadOnCollision(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLIST* pOther) {}

		// RETAIL: pure in the base (slot 7)
		virtual void Update() = 0;

		// A cycle ran into a trail. RETAIL: 0x100bd4f0
		virtual bool HandleCollision(LIGHT_CYCLIST* pCyclist, LightCycleCollisionInfo* pInfo, bool bTronUnknown1, bool bTronUnknown2);

		virtual bool TronUnknownVirtual9(LIGHT_CYCLIST* pCyclist, uint32 nTronUnknown1, uint32 nTronUnknown2, bool bTronUnknown);	// RETAIL: 0x100bd980
		virtual bool UpdateLightCycleTrail(LIGHT_CYCLIST* pCyclist, LTVector& vPos);	// RETAIL: 0x100bde10
		virtual void TronUnknownVirtual11(LIGHT_CYCLIST* pCyclist);		// RETAIL: 0x100be0b0
		virtual bool TronUnknownVirtual12(LIGHT_CYCLIST* pCyclist, uint32 nTronUnknown);	// RETAIL: 0x100bcb60
		virtual bool BeginLightCycleTrail(LIGHT_CYCLIST* pCyclist, LTVector& vPos, bool bTronUnknown);	// RETAIL: 0x100bd0b0
		virtual void RemoveAllCyclists();								// RETAIL: 0x100bd110
		virtual bool SetPowerup(LIGHT_CYCLIST* pCyclist, int nPowerup);	// RETAIL: 0x100bd300
		virtual bool ActivatePowerup(LIGHT_CYCLIST* pCyclist, const LTVector& vPos);	// RETAIL: 0x100bd330
		virtual void TronUnknownVirtual17(LIGHT_CYCLIST* pCyclist, uint32 nTronUnknown);	// RETAIL: 0x100bd4e0
		virtual void TronUnknownVirtual18();							// RETAIL: 0x100be4a0

		// RETAIL: 0x1004fd80 (inline)
		virtual std::string TronUnknownVirtual19(uint32 nTronUnknown) { return ""; }

		// A cycle used its turbo. RETAIL: 0x1004fe40 (empty, inline)
		virtual void OnCyclistTurbo(LIGHT_CYCLIST* pCyclist, bool bTronUnknown, float fTronUnknown1, float fTronUnknown2, uint32 nTronUnknown) {}

		virtual void TronUnknownVirtual21(uint32 nTronUnknown) {}		// RETAIL: 0x100e2900 (empty)

		void SetUpdating(bool bUpdating) { m_bUpdating = bUpdating; }

		LIGHT_CYCLIST* FindCyclist(HOBJECT hObj);						// RETAIL: 0x100bbe10

		// Does the segment ptStart..ptEnd of pCyclist hit a cycle or a trail
		// (pIgnore's excepted)? Fills pInfo. RETAIL: 0x100bbef0
		bool CheckForCollision(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLE_TRAIL_POINT& ptStart, LIGHT_CYCLE_TRAIL_POINT& ptEnd,
			LightCycleCollisionInfo* pInfo, bool bTronUnknown, LIGHT_CYCLIST* pIgnore);

		// RETAIL: public (LightCycleMissile::Update calls it)
		void CreateTrailPointFromVector(LIGHT_CYCLE_TRAIL_POINT &pt, LTVector& vPos);	// RETAIL: 0x100bbed0
		LIGHT_CYCLIST* GetActiveCyclist(HOBJECT hObj);						// RETAIL: 0x100bdde0
		void TronUnknownUpdate();										// RETAIL: 0x100bd170
		bool AddTrailPoints(LIGHT_CYCLIST* pCyclist, bool bTronUnknown1, LTVector& vPos, bool bTronUnknown2);	// RETAIL: 0x100bb990
		uint32 AddLightCycleTrailPoint(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLE_TRAIL* pTrail, LTVector& vPos);	// RETAIL: 0x100bbd80

	protected:

		// Collection of light cyclist data
		std::vector<LIGHT_CYCLIST*> m_collCyclists;		// RETAIL: +0x04
		std::vector<LIGHT_CYCLIST*> m_collTronUnknown10;	// RETAIL: +0x10

		// Are we updating or not
		bool m_bUpdating;								// RETAIL: +0x1c
		float m_fTronUnknown20;							// RETAIL: +0x20 (500.0)

		CLightCycleButeMgr m_LightCycleButeMgr;			// RETAIL: +0x24 (g_pLightCycleButeMgr)

		float m_fGameSpeedScalar;						// RETAIL: +0x1330
		float m_fTrailLength;							// RETAIL: +0x1334
		int m_nDifficulty;								// RETAIL: +0x1338
};

extern CLightCycleMgr* g_pLightCycleMgr;	// RETAIL: 0x1035a54c

// RETAIL: what CServerLightCycleMgr::SetPowerup calls for a random power-up:
// 0x1013c350, which returns 0 (its TU is not identified; the name is ours)
int TronUnknownGetRandomPowerup();

#endif // _LIGHTCYCLEMGR_H_
