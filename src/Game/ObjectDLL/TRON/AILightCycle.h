// ----------------------------------------------------------------------- //
//
// MODULE  : AILightCycle.h
//
// PURPOSE : An AI controlled light cycle of the light cycle game
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x10001250..0x1000a1c0: its class def at 0x102d6780, vtable
// 0x1029331c, 0x130 bytes; its plugin CAILCPlugin, vtable 0x102932d0). The
// names of the classes, the properties and the behaviors are retail's
// (RTTI, strings); the rest are ours. The layout is retail's (constructor
// 0x10001cd0, Save 0x10006f20, Load 0x10006fa0).
//
// ----------------------------------------------------------------------- //

#ifndef __AILIGHTCYCLE_H__
#define __AILIGHTCYCLE_H__

#include "LightCycle.h"
#include "LightCycleMgr.h"
#include <list>
#include <vector>
#include <string>

class LightCyclePowerup;

// RETAIL: what AILightCycle::Probe and ProbeAll fill (0x1c bytes, copied by
// its out of line operator= at 0x10003030): what a probe hit and how far.
// The names are ours.
struct AILCProbeResult
{
	LightCycleCollisionInfo	info;		// RETAIL: +0x00 (pCyclist NULL: a light cycle volume)
	float					fDist;		// RETAIL: +0x18
};

typedef std::vector<AILCProbeResult>	AILCProbeResultList;

// RETAIL: the kinds of things a probe hits (Probe, GetHitType; bit flags,
// the names are ours)
#define AILC_HIT_NONE			0x00
#define AILC_HIT_OWNTRAIL		0x02
#define AILC_HIT_AITRAIL		0x04
#define AILC_HIT_PLAYERTRAIL	0x08
#define AILC_HIT_VOLUME			0x10
#define AILC_HIT_OUTOFBOUNDS	0x20

// RETAIL: the movement flags the AI gives the light cycle manager's
// movement update (slot 12, m_dwMoveFlags); as the client's LCMF_ flags
// (Shared/TRON/LightCycleMgr.h). 1 is set by every AI update (0x100046ce);
// the names are ours.
#define AILC_MOVE_GO			0x00000001
#define AILC_MOVE_TURNLEFT		0x00000010
#define AILC_MOVE_TURNRIGHT		0x00000100
#define AILC_MOVE_ACCELERATE	0x00001000
#define AILC_MOVE_BRAKE			0x00010000
#define AILC_MOVE_SPEEDZONE		0x00100000
#define AILC_MOVE_SLOWZONE		0x01000000

// RETAIL: the turns of GetTurnPoint (0x10003790); the names are ours
enum AILCTurn
{
	kAILCTurn_Forward = 0,
	kAILCTurn_Right,
	kAILCTurn_Back,
	kAILCTurn_Left,
};

// RETAIL: the values of m_eBehavior (+0x84). 0 and 1 are what ReadProp sets
// for the Behavior property (0x1000253b); 4 is chasing a power-up
// (SetBehavior, 0x10007290). The names are ours.
enum AILCBehavior
{
	kAILCBehavior_Wander = 0,
	kAILCBehavior_Aggressive = 1,
	kAILCBehavior_Avoid = 2,
	kAILCBehavior_TronUnknown3 = 3,
	kAILCBehavior_Powerup = 4,
};

// RETAIL: a timed state of the AI (+0xf8 and +0x108, 0x10 bytes,
// constructed inline by AILightCycle's constructor). The names are ours.
struct AILCTimer
{
	AILCTimer()
	{
		fStartTime = 0.0f;
		fDuration = 0.0f;
		fTime = 0.0f;
		bOn = false;
	}

	float	fStartTime;		// RETAIL: +0x00
	float	fDuration;		// RETAIL: +0x04
	float	fTime;			// RETAIL: +0x08
	bool	bOn;			// RETAIL: +0x0c
};

class AILightCycle : public LightCycle
{
	public :

		AILightCycle();									// RETAIL: 0x10001cd0
		virtual ~AILightCycle();						// RETAIL: 0x10001e60

	protected :

		typedef void (AILightCycle::*StateFn)();

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);	// RETAIL: 0x10001f80
		virtual uint32	ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);		// RETAIL: 0x10002000

		virtual void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);	// RETAIL: 0x10006f20 (slot 26)
		virtual void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);	// RETAIL: 0x10006fa0 (slot 27)
		virtual LTBOOL	InitialUpdate();									// RETAIL: 0x100025e0 (slot 28)

		LTBOOL	ReadProp(ObjectCreateStruct *pStruct);		// RETAIL: 0x10002320
		void	Update();									// RETAIL: 0x10006ed0

		// Adds the cycle to the light cycle manager and picks the state.
		// RETAIL: 0x10002760
		void	Start();

		// The states (m_pfnState). RETAIL: UpdateScripted is empty (folded
		// at 0x10264ac0)
		void	UpdateScripted() {}
		void	UpdateGoForward();							// RETAIL: 0x100046a0
		void	UpdateAI();									// RETAIL: 0x100049f0

		// The direction (0..3: +z, +x, -z, -x) from the cycle to vPos, by
		// its larger axis. RETAIL: 0x10002860
		int		GetDirectionTo(const LTVector & vPos);

		// The direction the player's cycle travels. RETAIL: 0x100028f0
		int		GetPlayerDirection();

		// Looks fDist ahead in direction nDir for trails and volumes; fills
		// pResult with the nearest. Returns an AILC_HIT_ flag.
		// RETAIL: 0x10002970
		int		Probe(int nDir, AILCProbeResult* pResult, float fDist);

		// As Probe, but adds all the hits to pResults, nearest first, and
		// returns how many it added. RETAIL: 0x10003070
		int		ProbeAll(int nDir, AILCProbeResultList* pResults, float fDist);

		// The distance to the player ahead of us and where. RETAIL: 0x100034b0
		bool	GetPlayerAhead(float* pfDist, LTVector* pvPos);

		// Is the segment vStart..vEnd blocked by a trail or a volume?
		// Its distance in pfDist, the nearest hit in pvHit.
		// RETAIL: 0x10003630
		bool	CheckPath(const LTVector & vStart, const LTVector & vEnd, float* pfDist, LTVector* pvHit);

		// The corner of a turn toward vTarget. RETAIL: 0x10003790
		void	GetTurnPoint(int nTurn, const LTVector & vTarget, LTVector & vPoint);

		// The unit vectors of the cycle's direction. RETAIL: GetLeft
		// 0x10003a90 and GetBack 0x10003bc0 are also called out of line
		LTVector	GetRight()
		{
			static LTVector s_avRight[4] =
			{
				LTVector(1.0f, 0.0f, 0.0f),
				LTVector(0.0f, 0.0f, -1.0f),
				LTVector(-1.0f, 0.0f, 0.0f),
				LTVector(0.0f, 0.0f, 1.0f),
			};
			return s_avRight[m_nDirection];
		}

		LTVector	GetLeft()
		{
			static LTVector s_avLeft[4] =
			{
				LTVector(-1.0f, 0.0f, 0.0f),
				LTVector(0.0f, 0.0f, 1.0f),
				LTVector(1.0f, 0.0f, 0.0f),
				LTVector(0.0f, 0.0f, -1.0f),
			};
			return s_avLeft[m_nDirection];
		}

		LTVector	GetForward()
		{
			static LTVector s_avForward[4] =
			{
				LTVector(0.0f, 0.0f, 1.0f),
				LTVector(1.0f, 0.0f, 0.0f),
				LTVector(0.0f, 0.0f, -1.0f),
				LTVector(-1.0f, 0.0f, 0.0f),
			};
			return s_avForward[m_nDirection];
		}

		LTVector	GetBack()
		{
			static LTVector s_avBack[4] =
			{
				LTVector(0.0f, 0.0f, -1.0f),
				LTVector(-1.0f, 0.0f, 0.0f),
				LTVector(0.0f, 0.0f, 1.0f),
				LTVector(1.0f, 0.0f, 0.0f),
			};
			return s_avBack[m_nDirection];
		}

		// Remembers a turn (the last four). RETAIL: 0x10006e40
		void	AddTurn(uint32 nTurn)
		{
			m_anTurns[3] = m_anTurns[2];
			m_anTurns[2] = m_anTurns[1];
			m_anTurns[1] = m_anTurns[0];
			m_anTurns[0] = nTurn;
		}

		// RETAIL: 0x10003cf0
		bool	IsToLeft(LTVector vPos);
		// RETAIL: 0x10003e90
		bool	IsInFront(const LTVector & vPos);

		// Goes for the nearest power-up, if it's worth it. RETAIL: 0x10004030
		bool	FindPowerup();
		// Plans the path to vTarget, turning to nDir. RETAIL: 0x10004230
		bool	PlanPath(int nDir, const LTVector & vTarget);
		// Plans the path to the power-up again. RETAIL: 0x10004460
		bool	ReplanPowerupPath();
		// Gives up the power-up. RETAIL: 0x100044f0
		void	DropPowerup();
		// Is the planned path still clear? RETAIL: 0x10004560
		bool	IsPathClear();
		// The distance traveled since the last turn. RETAIL: 0x100046f0
		float	GetDistSinceTurn();
		// Fires the power-up. RETAIL: 0x10004990
		void	UsePowerup();

		// RETAIL: out of line, after Load (0x10007130..0x100073b0)
		bool			IsPlayerDead();						// RETAIL: 0x10007130
		LIGHT_CYCLIST*	GetPlayerCyclist();					// RETAIL: 0x10007140
		LTVector		GetPlayerPos();						// RETAIL: 0x10007150
		bool			IsTrailTight();						// RETAIL: 0x100071c0
		int				SetBehavior(int eBehavior);			// RETAIL: 0x10007290
		int				GetHitType(LightCycleCollisionInfo* pInfo);	// RETAIL: 0x10007330
		int				GetPowerup();						// RETAIL: 0x10007380
		bool			HasFlag(uint32 nFlag);				// RETAIL: 0x10007390

		bool		m_bFirstUpdate;					// RETAIL: +0x6c (Start on the next update)
		bool		m_bScripted;					// RETAIL: +0x6d Scripted
		bool		m_bPaused;						// RETAIL: +0x6e StartPaused
		bool		m_bMoved;						// RETAIL: +0x6f (cleared each update)
		bool		m_bTronUnknown70;				// RETAIL: +0x70
		float		m_fUpdateDelta;					// RETAIL: +0x74
		LIGHT_CYCLIST*	m_pCyclist;					// RETAIL: +0x78
		uint32		m_nTronUnknown7C;				// RETAIL: +0x7c (unused)
		int			m_nTronUnknown80;				// RETAIL: +0x80
		int			m_eBehavior;					// RETAIL: +0x84 AILCBehavior
		int			m_eDefaultBehavior;				// RETAIL: +0x88 Behavior
		int			m_eTronUnknown8C;				// RETAIL: +0x8c
		LTVector	m_vStartPos;					// RETAIL: +0x90
		LTVector	m_vTronUnknown9C;				// RETAIL: +0x9c
		float		m_fCycleLength;					// RETAIL: +0xa8
		uint32		m_anTurns[4];					// RETAIL: +0xac (0xff: none)
		uint32		m_dwMoveFlags;					// RETAIL: +0xbc (LCMF_, cleared each update)
		int			m_nDirection;					// RETAIL: +0xc0 (0..3: +z, +x, -z, -x)
		int			m_nTronUnknownC4;				// RETAIL: +0xc4
		StateFn		m_pfnState;						// RETAIL: +0xc8
		std::string	m_sCrashCmd;					// RETAIL: +0xd0 CrashCmd
		std::string	m_sRemoveCmd;					// RETAIL: +0xdc RemoveCmd
		bool		m_bWaitingForClient;			// RETAIL: +0xe8 (until MID_LIGHT_CYCLE_INFO 15)
		std::list<LTVector>	m_lstPath;				// RETAIL: +0xec (to the power-up)
		std::list<LTVector>	m_lstTronUnknownF0;		// RETAIL: +0xf0
		LightCyclePowerup*	m_pPowerup;				// RETAIL: +0xf4 (the one we go for)
		AILCTimer	m_TronUnknownF8;				// RETAIL: +0xf8
		AILCTimer	m_TronUnknown108;				// RETAIL: +0x108
		int			m_nLastPowerup;					// RETAIL: +0x118 (LC_NO_POWERUP)
		LTVector	m_vTronUnknown11C;				// RETAIL: +0x11c
		uint32		m_nNameId;						// RETAIL: +0x128 NameId
		bool		m_bGoForward;					// RETAIL: +0x12c GoForward
};

// RETAIL: the 2D (x, z) intersection of the segments p1..p2 and p3..p4:
// 0 none, 1 at *pvHit, 2 overlapping from *pvHit to *pvHit2. Called by the
// light cycle volumes' code too (0x1000a711). RETAIL: 0x100073d0
int IntersectSegments(LTVector p1, LTVector p2, LTVector p3, LTVector p4, LTVector* pvHit, LTVector* pvHit2);

// RETAIL: the light cycle volumes' code (0x1000a1f0.., not written yet):
// is vPos in one (0x1000a5b0); does vStart..vEnd cross one, where and how
// far (0x1000a620). The names are ours.
bool IsPointInLightCycleVolume(const LTVector & vPos);
bool IntersectLightCycleVolumes(const LTVector & vStart, const LTVector & vEnd, float* pfDist, LTVector* pvHit, LTVector* pvNormal);

#endif // __AILIGHTCYCLE_H__
