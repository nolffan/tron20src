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
// server-only ancestor). One TU in both DLLs: CShell.dll 0x100fe3b0..
// 0x10101b60 (vtable 0x101e572c) and Object.lto 0x100baf70..0x100be640
// (vtable 0x10294bcc), function for function in the same order and, but
// for what _CLIENTBUILD marks (AddCyclist's SetObjectPos, the end of
// UpdateLightCycleTrail, GetDifficulty), the same code. Object.lto's link
// drops the three functions only the client calls (GetGameSpeedIndex,
// GetTrailLengthIndex, GetTronUnknown1338). The class,
// LIGHT_CYCLIST, LIGHT_CYCLE_TRAIL(_POINT), LightCycleCollisionInfo and the
// names Build 69 has are Monolith's; the other names are ours.
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

// UpdateCyclistMovement's flags (the names are ours)
#define LCMF_TURNLEFT		0x00000010
#define LCMF_TURNRIGHT		0x00000100
#define LCMF_ACCELERATE		0x00001000
#define LCMF_BRAKE			0x00010000
#define LCMF_SPEEDZONE		0x00100000
#define LCMF_SLOWZONE		0x01000000

// The no-power-up value of LIGHT_CYCLIST::nPowerup (ours; RETAIL: 0xc)
#define LC_NO_POWERUP	12

// LIGHT_CYCLIST::nFlags as masks, for the server's code that tests them so
// (names ours): bTurbo, bTrailSpike and bTurboCursed, and what a timed
// power-up's end clears (bTurbo, bTurboCursed, bTurboCurse; RETAIL:
// Object.lto 0x10050278)
#define LCF_TURBO					(1<<1)
#define LCF_TRAILSPIKE				(1<<2)
#define LCF_TURBOCURSED				(1<<5)
#define LCF_TIMED_MASK				(~0x422)

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

// RETAIL: 0x1c bytes (the new function: CShell 0x10016070, Object.lto
// 0x100bf020)
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
	float	fDerezStartTime;		// RETAIL: +0x18 (set by DerezOldTrails)
};

// RETAIL: 0x134 bytes (the new function and its inline constructor: CShell
// 0x10015dd0, Object.lto 0x100bee20)
struct LIGHT_CYCLIST
{
	LIGHT_CYCLIST() : nClientID(0), nNameId(0)
	{
		fStartDerezTime = 0.0f;
		bUpdating = true;
		bRemove = false;
		pCurTrail = NULL;
		hObject = NULL;
		vForward.Init(0.0f, 0.0f, 0.0f);
		fMaxTrailLength = 0.0f;
		nCycleType = 0;
		szCycleColor[0] = '\0';
		nRed = 0;
		nGreen = 0;
		nBlue = 0;
		fAccelRate = 0.0f;
		fBaseSpeed = 800.0f;
		fSpeed = 0.0f;
		fMaxSpeed = 0.0f;
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
		bIsPlayer = false;
		nCycleDialog = 0;
		vLastPos.Init(0.0f, 0.0f, 0.0f);
		fTrailCheckTime = 0.0f;
		m_vTronUnknownE4.Init(0.0f, 0.0f, 0.0f);
		nPowerup = LC_NO_POWERUP;
		nFlags = 0;
		fPowerupStartTime = 0.0f;
		fPowerupDuration = 1.0f;
		pSpikeTrail = NULL;
		pSpikeTrail2 = NULL;
		vSpikeDir.Init(0.0f, 0.0f, 0.0f);
		vSpikeDir2.Init(0.0f, 0.0f, 0.0f);
		vSpikePos.Init(0.0f, 0.0f, 0.0f);
		fSpikeFullTime = 0.0f;
		m_bTronUnknownD1 = false;
		m_bTronUnknown130 = false;
	}

	// RETAIL: Object.lto only, out of line at 0x10029fd0 (LightCycleMissile's
	// stand-in cyclist). Its "LIGHT_CYCLIST();" builds and destroys a
	// temporary: it does not initialize this cyclist (see
	// docs/RETAIL_BUGS.md). The parameters' names are ours.
	LIGHT_CYCLIST(float fNewLength, float fNewWidth, bool bTronUnknownD1)
	{
		LIGHT_CYCLIST();
		fLength = fNewLength;
		fWidth = fNewWidth;
		m_bTronUnknownD1 = bTronUnknownD1;
	}

	~LIGHT_CYCLIST();						// RETAIL: CShell 0x100fe3c0, Object.lto 0x100baf80

	// Removes and frees the trail spike's trails. RETAIL: CShell 0x100fe4e0,
	// Object.lto 0x100bb0a0
	void ClearTrailSpike();

	// Collection of trails (normally just 1 except in special cases)
	std::vector<LIGHT_CYCLE_TRAIL*> collTrails;	// RETAIL: +0x00

	float	fStartDerezTime;				// RETAIL: +0x0c
	bool	bUpdating;						// RETAIL: +0x10
	bool	bRemove;						// RETAIL: +0x11 (set by RemoveCyclist, freed by UpdateDerez)
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
	float	fAccelRate;						// RETAIL: +0x6c (UpdateCyclistMovement)
	float	fBaseSpeed;						// RETAIL: +0x70
	float	fSpeed;							// RETAIL: +0x74
	float	fMaxSpeed;						// RETAIL: +0x78 (UpdateCyclistMovement)
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
	bool	bIsPlayer;						// RETAIL: +0xd0 (AddCyclist's pvDims: players')
	bool	m_bTronUnknownD1;				// RETAIL: +0xd1 (LightCycleMissile's: hits its own trails, Object.lto 0x100bbf5c)
	uint32	nClientID;						// RETAIL: +0xd4 (the player's client id; -1 for AI)
	uint32	nNameId;						// RETAIL: +0xd8 (the AI's name string id)
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
	LTVector vSpikeDir;						// RETAIL: +0x108 (BeginTrailSpike)
	LTVector vSpikeDir2;					// RETAIL: +0x114
	LTVector vSpikePos;						// RETAIL: +0x120
	float	fSpikeFullTime;					// RETAIL: +0x12c (0: still growing)
	bool	m_bTronUnknown130;				// RETAIL: +0x130
};

// RETAIL: 0x18 bytes, filled by CheckForCollision (CShell 0x100ff360)
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

		// RETAIL: the addresses are CShell's; the Object.lto ones are in
		// the .cpp
		CLightCycleMgr();						// RETAIL: 0x100fe5c0
		virtual ~CLightCycleMgr() {}			// RETAIL: 0x10010850 (inline)

		virtual bool Init();					// RETAIL: 0x100fe640
		virtual void Term();					// RETAIL: 0x100fe670

		// Adds (or resets) the cyclist of the object; pvDims is given for
		// players. The base does not use the last two (the server's sends
		// them to the clients). RETAIL: 0x100fe680
		virtual LIGHT_CYCLIST* AddCyclist(HOBJECT hObj, const char* szCycleColor, bool bUpdate,
			const LTVector* pvDims, uint32 nNameId, bool bTronUnknown);
		// Flags the cyclist for removal (UpdateDerez frees it; Build 69's
		// name, which removed it at once). RETAIL: 0x100fec30
		virtual bool RemoveCyclist(HOBJECT hObj);

		// Starts the derez of a cyclist that crashed (into pOther or its
		// trail); false if it was already derezzing (Build 69's name).
		// RETAIL: slot 5, pure
		virtual bool DerezCyclist(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLIST* pOther, bool bNotify) = 0;
		// Two cycles met head on. RETAIL: slot 6, 0x100991a0 (empty, folded)
		virtual void HeadOnCollision(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLIST* pOther) {}
		// Moves the cycles and their trails (Build 69's name). RETAIL: slot
		// 7, pure
		virtual void Update() = 0;

		// A shielded cyclist's hit. RETAIL: 0x10100960
		virtual bool HandleShieldCollision(LIGHT_CYCLIST* pCyclist, LightCycleCollisionInfo* pInfo, bool bNotify, bool bCurrentTrail);
		// Drops the first points of the current trail up to nPoint (or the
		// first if two are left, snapping the next). RETAIL: 0x10100df0
		virtual bool TrimTrail(LIGHT_CYCLIST* pCyclist, int nTrail, int nPoint, bool bNotify);
		// Starts the trail spike's two trails at vPos. RETAIL: 0x10101280
		virtual bool BeginTrailSpike(LIGHT_CYCLIST* pCyclist, LTVector* pvPos);
		// Grows the trail spike, then removes it. RETAIL: 0x10101520
		virtual void UpdateTrailSpike(LIGHT_CYCLIST* pCyclist);
		// Turns and accelerates the object's cycle from the LCMF_ flags and
		// sets its velocity. RETAIL: 0x100fffd0
		virtual bool UpdateCyclistMovement(HOBJECT hObj, uint32 nFlags);

		// Sets the cycle's dims, turned to its forward. RETAIL: 0x10100520
		virtual void UpdateCyclistDims(LIGHT_CYCLIST* pCyclist, LTVector & vDims, bool bTronUnknown);
		// Frees all the cyclists. RETAIL: 0x10100580
		virtual void RemoveAllCyclists();
		virtual bool SetCyclistPowerup(LIGHT_CYCLIST* pCyclist, int nPowerup);	// RETAIL: 0x10100770
		virtual bool ActivatePowerup(LIGHT_CYCLIST* pCyclist, LTVector* pvPos);	// RETAIL: 0x101007a0
		virtual void DeactivateShield(LIGHT_CYCLIST* pCyclist, bool bNotify);	// RETAIL: 0x10100950
		// Starts the derez of every living cyclist's trails but the current
		// one (the name is ours). RETAIL: 0x10101910
		virtual void DerezOldTrails();

		// The cyclist's name (the name is ours). RETAIL: slot 19, 0x10010970
		// (inline; returns a string made from a global char array,
		// 0x10236c98; the client's: 0x10012dd0)
		virtual std::string GetCyclistName(HOBJECT hObj) { return std::string(""); }
		// The cycle turned or its acceleration or max speed changed
		// (UpdateCyclistMovement). RETAIL: slot 20, 0x10010a30 (inline, empty)
		virtual void CyclistMovementChanged(LIGHT_CYCLIST* pCyclist, bool bTurned, float fAccel, float fWallAccel, uint32 nFlags) {}
		// A trail was cut by a shielded cycle (HandleShieldCollision).
		// RETAIL: slot 21, 0x100a9310 (inline, empty, folded)
		virtual void TrailCut(LightCycleCollisionInfo* pInfo) {}

		LIGHT_CYCLIST* FindCyclist(HOBJECT hObj);	// RETAIL: 0x100ff280

		// Snaps the position to the bute grid. RETAIL: 0x100ffec0
		void SnapToGrid(LTVector & vPos);

		// Frees the cyclists flagged for removal and the trails whose derez
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
		// Pulls the trail point back from a (shield) hit point, dropping
		// the point if it passes its neighbour. RETAIL: 0x10100fb0
		bool CutTrailPoint(std::vector<LIGHT_CYCLE_TRAIL_POINT> & collPoints, LIGHT_CYCLE_TRAIL_POINT* pPoint,
			LIGHT_CYCLE_TRAIL_POINT* pNeighbour, LightCycleCollisionInfo* pInfo);
		uint32 AddLightCycleTrailPoint(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLE_TRAIL* pTrail, LTVector& vPos);	// RETAIL: 0x100ff1f0

		// The wall acceleration of the cyclist's wall distance. RETAIL: 0x101004d0
		float GetWallAccelRate(LIGHT_CYCLIST* pCyclist);

		// The index of the GameSpeedScalars value of m_fGameSpeedScalar.
		// RETAIL: 0x10101990
		int GetGameSpeedIndex();
		// The index of the TrailLengthValues value of m_fTrailLength.
		// RETAIL: 0x101019e0
		int GetTrailLengthIndex();

		int GetTronUnknown1338();				// RETAIL: 0x10101a30 (m_nTronUnknown1338)

		// Turns the cycle's forward to the axis nearest its rotation's.
		// RETAIL: 0x10101a40
		void UpdateCyclistForward(LIGHT_CYCLIST* pCyclist);

		void SetUpdating(bool bUpdating) { m_bUpdating = bUpdating; }

		// RETAIL: public (Object.lto's LightCycleMissile::Update calls it)
		void CreateTrailPointFromVector(LIGHT_CYCLE_TRAIL_POINT &pt, LTVector& vPos);	// RETAIL: 0x100ff340

	protected:

		// Collision handler. RETAIL: 0x100ffbe0
		void HandleCollision(LIGHT_CYCLIST* pCyclist, LightCycleCollisionInfo &info);

		// The game's difficulty. RETAIL: 0x10101980
		int GetDifficulty();

		// Collection of light cyclist data
		std::vector<LIGHT_CYCLIST*> m_collCyclists;		// RETAIL: +0x04
		std::vector<LIGHT_CYCLIST*>	m_collTronUnknown10;// RETAIL: +0x10 (4-byte elements; the type makes ~CLightCycleMgr identical)

		// Are we updating or not
		bool				m_bUpdating;				// RETAIL: +0x1c
		float				m_fTronUnknown20;			// RETAIL: +0x20 (500)
		CLightCycleButeMgr	m_ButeMgr;					// RETAIL: +0x24
		float				m_fGameSpeedScalar;			// RETAIL: +0x1330
		float				m_fTrailLength;				// RETAIL: +0x1334
		int					m_nTronUnknown1338;			// RETAIL: +0x1338
};

extern CLightCycleMgr* g_pLightCycleMgr;	// RETAIL: CShell 0x10236d98, Object.lto 0x1035a54c

#endif // __TRON_LIGHTCYCLEMGR_H__
