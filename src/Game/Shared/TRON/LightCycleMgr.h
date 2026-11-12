// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleMgr.h
//
// PURPOSE : The light cycle manager: the cyclists, their trails, power-ups
//           and collisions. The base of CClientLightCycleMgr (CShell.dll)
//           and of CServerLightCycleMgr (Object.lto).
//
// RETAIL: Tron 2.0's, retail-only (RTTI .?AVCLightCycleMgr@@, a virtual
// class; Build 69's ObjectDLL/TRON/LightCycleMgr is its early, non-virtual
// server-only ancestor). Written from retail CShell.dll 0x100fe3b0..
// 0x10101a40 (vtable 0x101e572c); the same TU is in Object.lto at
// 0x100bb200.. (vtable 0x10294bcc). The class, LIGHT_CYCLIST,
// LIGHT_CYCLE_TRAIL(_POINT), LightCycleCollisionInfo and the names Build 69
// has are Monolith's; the other names are ours.
//
// RETAIL: layout. CLightCycleButeMgr is 0x40 bytes bigger in our build
// (Tron's CButeMgr is smaller), so the members after m_ButeMgr are 0x40
// further than retail's offsets given below.
//
// ----------------------------------------------------------------------- //

#ifndef __TRON_LIGHTCYCLEMGR_H__
#define __TRON_LIGHTCYCLEMGR_H__

#include <vector>
#include <string>
#include "LightCycleButeMgr.h"

// Defines (Build 69)
#define CURRENT_TRAIL_ID -1

// The no-power-up value of LIGHT_CYCLIST::nPowerup (ours; RETAIL: 0xc)
#define LC_NO_POWERUP	12

// RETAIL: 12 bytes
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

// RETAIL: 0x1c bytes (the new function: CShell 0x10016070)
struct LIGHT_CYCLE_TRAIL
{
	LIGHT_CYCLE_TRAIL()
	{
		fLength = 0.0f;
		m_fTronUnknown14 = 500.0f;
		fDerezStartTime = 0.0f;
		m_bTronUnknownC = false;
	}

	// Collection of wall points
	std::vector<LIGHT_CYCLE_TRAIL_POINT> collTrailPoints;	// RETAIL: +0x00

	bool	m_bTronUnknownC;		// RETAIL: +0x0c
	float	fLength;				// RETAIL: +0x10 (summed by UpdateLightCycleTrail)
	float	m_fTronUnknown14;		// RETAIL: +0x14
	float	fDerezStartTime;		// RETAIL: +0x18 (set by Update)
};

// RETAIL: +0xd4 of LIGHT_CYCLIST: a member with a constructor that clears
// two dwords (the type and its name are ours)
struct LCTronUnknownPair
{
	LCTronUnknownPair() : m_nA(0), m_nB(0) {}
	uint32	m_nA;
	uint32	m_nB;
};

// RETAIL: 0x134 bytes (the new function and its inline constructor: CShell
// 0x10015dd0)
struct LIGHT_CYCLIST
{
	LIGHT_CYCLIST()
	{
		fStartDerezTime = 0.0f;
		bUpdating = true;
		bDerezRequested = false;
		pCurTrail = NULL;
		hObject = NULL;
		vForward.x = 0.0f;
		vForward.y = 0.0f;
		vForward.z = 0.0f;
		fMaxTrailLength = 0.0f;
		nCycleType = 0;
		szCycleColor[0] = '\0';
		nRed = 0;
		nGreen = 0;
		nBlue = 0;
		m_fTronUnknown6C = 0.0f;
		fBaseSpeed = 800.0f;
		fSpeed = 0.0f;
		m_fTronUnknown78 = 0.0f;
		fTurboAccelRate = 400.0f;
		fBrakeDecelRate = 400.0f;
		fMaxManualSpeedAdd = 1600.0f;
		fMaxExternalSpeedAdd = 2400.0f;
		fMaxTurboSpeed = 5000.0f;
		fMinSpeed = 400.0f;
		fNormalAccelRate = 100.0f;
		fNormalDecelRate = 100.0f;
		fMaxWallAccelRate = 400.0f;
		fTurnSpeedLossPercent = 0.5f;
		nMaxWallDist = 50;
		m_bTronUnknownAC = false;
		fHeight = 0.0f;
		fWidth = 0.0f;
		fLength = 0.0f;
		bFixedDims = false;
		nCycleDialog = 0;
		vLastPos.x = 0.0f;
		vLastPos.y = 0.0f;
		vLastPos.z = 0.0f;
		fTrailCheckTime = 0.0f;
		m_vTronUnknownE4.x = 0.0f;
		m_vTronUnknownE4.y = 0.0f;
		m_vTronUnknownE4.z = 0.0f;
		nPowerup = LC_NO_POWERUP;
		nFlags = 0;
		fPowerupStartTime = 0.0f;
		fPowerupDuration = 1.0f;
		pSpikeTrail = NULL;
		pSpikeTrail2 = NULL;
		m_nTronUnknown108 = 0;
		m_nTronUnknown10C = 0;
		m_nTronUnknown110 = 0;
		m_nTronUnknown114 = 0;
		m_nTronUnknown118 = 0;
		m_nTronUnknown11C = 0;
		m_vTronUnknown120.Init(0.0f, 0.0f, 0.0f);
		m_nTronUnknown12C = 0;
		m_bTronUnknownD1 = false;
		m_bTronUnknown130 = false;
	}
	~LIGHT_CYCLIST();						// RETAIL: 0x100fe3c0

	// Removes and frees the trail spike's trails. RETAIL: 0x100fe4e0
	void ClearTrailSpike();

	// Collection of trails (normally just 1 except in special cases)
	std::vector<LIGHT_CYCLE_TRAIL*> collTrails;	// RETAIL: +0x00

	float	fStartDerezTime;				// RETAIL: +0x0c
	bool	bUpdating;						// RETAIL: +0x10
	bool	bDerezRequested;				// RETAIL: +0x11 (set by DerezCyclist)
	LIGHT_CYCLE_TRAIL* pCurTrail;			// RETAIL: +0x14
	LTVector vForward;						// RETAIL: +0x18
	LTVector vLastPos;						// RETAIL: +0x24
	LTObjRef hObject;						// RETAIL: +0x30
	float	fMaxTrailLength;				// RETAIL: +0x40
	char	szCycleColor[32];				// RETAIL: +0x44
	int		nCycleType;						// RETAIL: +0x64
	uint8	nRed;							// RETAIL: +0x68
	uint8	nGreen;							// RETAIL: +0x69
	uint8	nBlue;							// RETAIL: +0x6a
	float	m_fTronUnknown6C;				// RETAIL: +0x6c
	float	fBaseSpeed;						// RETAIL: +0x70
	float	fSpeed;							// RETAIL: +0x74
	float	m_fTronUnknown78;				// RETAIL: +0x78
	float	fMaxManualSpeedAdd;				// RETAIL: +0x7c
	float	fMaxExternalSpeedAdd;			// RETAIL: +0x80
	float	fMaxTurboSpeed;					// RETAIL: +0x84
	float	fMinSpeed;						// RETAIL: +0x88
	float	fTurboAccelRate;				// RETAIL: +0x8c
	float	fBrakeDecelRate;				// RETAIL: +0x90
	float	fNormalAccelRate;				// RETAIL: +0x94
	float	fNormalDecelRate;				// RETAIL: +0x98
	float	fMaxWallAccelRate;				// RETAIL: +0x9c
	float	fTurnSpeedLossPercent;			// RETAIL: +0xa0
	int		nWallDist;						// RETAIL: +0xa4
	int		nMaxWallDist;					// RETAIL: +0xa8
	bool	m_bTronUnknownAC;				// RETAIL: +0xac
	float	fLength;						// RETAIL: +0xb0 (dims along the travel)
	float	fWidth;							// RETAIL: +0xb4
	float	fHeight;						// RETAIL: +0xb8
	float	fPlayerNoticeDist;				// RETAIL: +0xbc
	float	fFrontSensorDist;				// RETAIL: +0xc0
	float	fRightSensorDist;				// RETAIL: +0xc4
	float	m_fTronUnknownC8;				// RETAIL: +0xc8
	float	fLeftSensorDist;				// RETAIL: +0xcc
	bool	bFixedDims;						// RETAIL: +0xd0 (AddCyclist's pvDims)
	bool	m_bTronUnknownD1;				// RETAIL: +0xd1
	LCTronUnknownPair m_TronUnknownD4;		// RETAIL: +0xd4
	int		nCycleDialog;					// RETAIL: +0xdc
	float	fTrailCheckTime;				// RETAIL: +0xe0 (UpdateLightCycleTrail)
	LTVector m_vTronUnknownE4;				// RETAIL: +0xe4
	int		nPowerup;						// RETAIL: +0xf0 LCPowerupType, LC_NO_POWERUP
	// RETAIL: +0xf4, bit fields (retail tests them with shifts); the names
	// are ours
	union
	{
		uint32	nFlags;
		struct
		{
			uint32	bShield : 1;			// bit 0, ActivatePowerup(Shield)
			uint32	bTurbo : 1;				// bit 1, ActivatePowerup(Turbo)
			uint32	bTrailSpike : 1;		// bit 2, ActivatePowerup(TrailSpike)
			uint32	bTronUnknown3 : 1;
			uint32	bTronUnknown4 : 1;
			uint32	bTurboCursed : 1;		// bit 5, set on the others by TurboCurse
			uint32	bTronUnknown6 : 1;
			uint32	bTronUnknown7 : 1;
			uint32	bTronUnknown8 : 1;
			uint32	bTronUnknown9 : 1;
			uint32	bTurboCurse : 1;		// bit 10, set on the curser by TurboCurse
		};
	};
	float	fPowerupStartTime;				// RETAIL: +0xf8
	float	fPowerupDuration;				// RETAIL: +0xfc
	LIGHT_CYCLE_TRAIL* pSpikeTrail;			// RETAIL: +0x100
	LIGHT_CYCLE_TRAIL* pSpikeTrail2;		// RETAIL: +0x104
	uint32	m_nTronUnknown108;				// RETAIL: +0x108
	uint32	m_nTronUnknown10C;				// RETAIL: +0x10c
	uint32	m_nTronUnknown110;				// RETAIL: +0x110
	uint32	m_nTronUnknown114;				// RETAIL: +0x114
	uint32	m_nTronUnknown118;				// RETAIL: +0x118
	uint32	m_nTronUnknown11C;				// RETAIL: +0x11c
	LTVector m_vTronUnknown120;				// RETAIL: +0x120
	uint32	m_nTronUnknown12C;				// RETAIL: +0x12c
	bool	m_bTronUnknown130;				// RETAIL: +0x130
};

// RETAIL: 0x18 bytes, filled by CheckForCollision (0x100ff360)
struct LightCycleCollisionInfo
{
	LIGHT_CYCLIST* pCyclist;	// RETAIL: +0x00 the cyclist hit (or whose trail was hit)
	int		nTrail;				// RETAIL: +0x04 index of the trail hit, -1: the cycle
	int		nSegment;			// RETAIL: +0x08 index of the trail's segment hit
	LTVector vCollisionPoint;	// RETAIL: +0x0c
};

class CLightCycleMgr
{
	public:

		CLightCycleMgr();						// RETAIL: 0x100fe5c0
		virtual ~CLightCycleMgr() {}			// RETAIL: 0x10010850 (inline)

		virtual bool Init();					// RETAIL: 0x100fe640
		virtual void Term();					// RETAIL: 0x100fe670

		// Adds (or resets) the cyclist of the object. RETAIL: 0x100fe680
		virtual LIGHT_CYCLIST* AddCyclist(HOBJECT hObj, const char* szCycleColor, bool bUpdate,
			LTVector* pvDims, uint32 nTronUnknown4, uint32 nTronUnknown5);
		virtual bool DerezCyclist(HOBJECT hObj);	// RETAIL: 0x100fec30

		// A cyclist crashed (into pOther or its trail). RETAIL: slot 5, pure
		virtual void CyclistCrash(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLIST* pOther, bool bTronUnknown) = 0;
		// Two cycles met head on. RETAIL: slot 6, 0x100991a0 (empty, folded)
		virtual void HeadOnCollision(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLIST* pOther) {}
		// RETAIL: slot 7, pure
		virtual void TronUnknownVirtual7() = 0;

		// A shielded cyclist's hit. RETAIL: 0x10100960
		virtual bool HandleShieldCollision(LIGHT_CYCLIST* pCyclist, LightCycleCollisionInfo* pInfo, bool bTronUnknown, uint32 nTronUnknown);
		// Drops the first points of the current trail up to nPoint (or the
		// first if two are left, snapping the next). RETAIL: 0x10100df0
		virtual bool TrimTrail(LIGHT_CYCLIST* pCyclist, int nTrail, int nPoint, uint32 nTronUnknown);
		virtual bool TronUnknownVirtual10(LIGHT_CYCLIST* pCyclist, uint32);	// RETAIL: 0x10101280
		virtual bool TronUnknownVirtual11(LIGHT_CYCLIST* pCyclist);			// RETAIL: 0x10101520
		virtual bool TronUnknownVirtual12(LIGHT_CYCLIST* pCyclist, uint32);	// RETAIL: 0x100fffd0

		// Sets the cycle's dims, turned to its forward. RETAIL: 0x10100520
		virtual void UpdateCyclistDims(LIGHT_CYCLIST* pCyclist, LTVector & vDims, bool bTronUnknown);
		// Frees all the cyclists. RETAIL: 0x10100580
		virtual void RemoveAllCyclists();
		virtual bool SetCyclistPowerup(LIGHT_CYCLIST* pCyclist, int nPowerup);	// RETAIL: 0x10100770
		virtual bool ActivatePowerup(LIGHT_CYCLIST* pCyclist, uint32 nTronUnknown);	// RETAIL: 0x101007a0
		virtual void DeactivateShield(LIGHT_CYCLIST* pCyclist, uint32 nTronUnknown);	// RETAIL: 0x10100950
		virtual void Update();					// RETAIL: 0x10101910

		// RETAIL: slot 19, 0x10010970 (inline; returns a string made from
		// a global char array, 0x10236c98)
		virtual std::string TronUnknownVirtual19(uint32) { return std::string(""); }
		// RETAIL: slot 20, 0x10010a30 (inline, empty)
		virtual void TronUnknownVirtual20(uint32, uint32, uint32, uint32, uint32) {}
		// RETAIL: slot 21, 0x100a9310 (inline, empty, folded)
		virtual void TronUnknownVirtual21(uint32) {}

		LIGHT_CYCLIST* FindCyclist(HOBJECT hObj);	// RETAIL: 0x100ff280

		// Snaps the position to the bute grid. RETAIL: 0x100ffec0
		void SnapToGrid(LTVector & vPos);

		// Frees the cyclists flagged for a derez and the trails whose derez
		// is over. RETAIL: 0x101005e0
		void UpdateDerez();

		// The next living cyclist after hObj's (the first if NULL); wraps
		// to the first if bWrap. RETAIL: 0x100ff2b0
		LIGHT_CYCLIST* GetNextCyclist(HOBJECT hObj, bool bWrap);

		// The cyclist of the object if it is not derezzing. RETAIL: 0x10101250
		LIGHT_CYCLIST* GetLivingCyclist(HOBJECT hObj);	// (uses g_pLightCycleMgr)

		LIGHT_CYCLE_TRAIL* BeginLightCycleTrail(LIGHT_CYCLIST* pCyclist, LTVector& vPos);	// RETAIL: 0x100fec60
		bool UpdateLightCycleTrail(LIGHT_CYCLIST* pCyclist, bool bCheckCollision, LTVector* pvForward, bool bTronUnknown);	// RETAIL: 0x100fedd0
		// Checks the move from the trail's last point to vPos for a hit.
		// RETAIL: 0x100ff360
		bool CheckForCollision(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLE_TRAIL_POINT* pLast, LTVector* pvPos,
			LightCycleCollisionInfo* pInfo, bool bCheckCollision, LIGHT_CYCLIST* pOwner);
		// Cuts the trail's tail to the cyclist's max trail length.
		// RETAIL: 0x100ffc00
		void TrimTrailLength(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLE_TRAIL* pTrail, bool bToTrailEnd);
		uint32 AddLightCycleTrailPoint(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLE_TRAIL* pTrail, LTVector& vPos);	// RETAIL: 0x100ff1f0

		// The wall acceleration of the cyclist's wall distance. RETAIL: 0x101004d0
		float GetWallAccelRate(LIGHT_CYCLIST* pCyclist);

		// The index of the GameSpeedScalars value of m_fGameSpeedScalar.
		// RETAIL: 0x10101990
		int GetGameSpeedIndex();
		// The index of the TrailLengthValues value of m_fTrailLength.
		// RETAIL: 0x101019e0
		int GetTrailLengthIndex();

		int GetTronUnknown1338();

		// Turns the cycle's forward to the axis nearest its rotation's.
		// RETAIL: 0x10101a40
		void UpdateCyclistForward(LIGHT_CYCLIST* pCyclist);					// RETAIL: 0x10101a30

		void SetUpdating(bool bUpdating) { m_bUpdating = bUpdating; }

	protected:

		void CreateTrailPointFromVector(LIGHT_CYCLE_TRAIL_POINT &pt, LTVector& vPos);	// RETAIL: 0x100ff340

		// Collision handler. RETAIL: 0x100ffbe0
		void HandleCollision(LIGHT_CYCLIST* pCyclist, LightCycleCollisionInfo &info);

		// The game's difficulty. RETAIL: 0x10101980
		int GetDifficulty();

		// Collection of light cyclist data
		std::vector<LIGHT_CYCLIST*> m_collCyclists;		// RETAIL: +0x04
		std::vector<uint32>			m_collTronUnknown10;// RETAIL: +0x10 (4-byte elements)

		// Are we updating or not
		bool				m_bUpdating;				// RETAIL: +0x1c
		float				m_fTronUnknown20;			// RETAIL: +0x20 (500)
		CLightCycleButeMgr	m_ButeMgr;					// RETAIL: +0x24
		float				m_fGameSpeedScalar;			// RETAIL: +0x1330
		float				m_fTrailLength;				// RETAIL: +0x1334
		int					m_nTronUnknown1338;			// RETAIL: +0x1338
};

extern CLightCycleMgr* g_pLightCycleMgr;	// RETAIL: CShell 0x10236d98

#endif // __TRON_LIGHTCYCLEMGR_H__
