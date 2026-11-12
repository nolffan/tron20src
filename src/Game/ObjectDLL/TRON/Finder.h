// ----------------------------------------------------------------------- //
//
// MODULE  : Finder.h
//
// PURPOSE : The Finder: a scanner (a turret on a keyframer) that opens,
//           turns toward the enemies it sees, locks on to them and fires
//           at them, lingers looking around when it loses them, and
//           closes again
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x1001adc0..0x10020090, its class def at 0x102d8640, vtables
// 0x102939f0 (main), 0x10293988 (IAISensing), 0x10293970
// (IObjectRelationMgrUser), the plugin's 0x10293ad0). It is NOLF2's
// SecurityCamera's sibling: a CScanner with IAISensing at +0x840 and
// IObjectRelationMgrUser at +0x844. The names of the classes, the
// properties, the commands and the animations are retail's; the names of
// the members, the functions and the states are ours.
//
// RETAIL: Tron's CScanner derives from PropType, not Prop (retail RTTI:
// Finder : CScanner : PropType : Prop), and CScannerPlugin from
// CPropTypePlugin. Ours (NOLF2's) derives from Prop, so every member here
// is 0x10 bytes before retail's (the offsets noted are retail's).
//
// ----------------------------------------------------------------------- //

#ifndef __FINDER_H__
#define __FINDER_H__

#include "Scanner.h"
#include "Timer.h"
#include "ServerSpecialFX.h"
#include <list>

class CWeapon;
class CWeapons;

// RETAIL: an enemy the Finder has locked on to and the time it stays
// locked on (0x20 bytes, kept in a list at +0x994; its destructor is
// 0x1001cb20); the name is ours
struct FinderTarget
{
	FinderTarget(const CTimer& timer, HOBJECT hObj)
	:	m_Timer(timer),
		m_hObj(hObj)
	{
	}

	CTimer		m_Timer;	// +0x00
	LTObjRef	m_hObj;		// +0x10
};

typedef std::list<FinderTarget> FinderTargetList;

class Finder : public CScanner
{
	public :

		Finder();
		virtual ~Finder();

		// RETAIL: 0x1001f700 (IObjectRelationMgrUser's slot 0)
		virtual LTBOOL	IsDead() const;

	protected :

		// RETAIL: the states (m_eState, +0x92c); the names are ours, the
		// animation each one loops is retail's (s_aszStateAnims)
		enum State
		{
			eStateNone,			// ""
			eStateClosing,		// "Idle": turns back to its rotation, then closes
			eStateClosed,		// "Idle"
			eStateFocusing,		// "Scan": turns toward an enemy until it locks on
			eStateLocked,		// "Fire_2" ("Alarm" without a weapon): fires
			eStateLingering,	// "Seek": looks around where it lost the enemy
			eStateSearching		// "Seek": turns toward where it was hit from
		};

		// RETAIL: which way the lingering turns (+0x980); the names are ours
		enum LingerState
		{
			eLingerTurningLeft,
			eLingerReturningFromRight,
			eLingerReturningFromLeft,
			eLingerTurningRight
		};

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual uint32	ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);
		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		virtual DetectState	UpdateDetect();

		// RETAIL: 0x1001ba20
		virtual LTFLOAT	GetFocusTime() { return m_fSecondsToLock; }

	private :

		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		bool	ReadProp(ObjectCreateStruct *pStruct);
		bool	PostPropRead(ObjectCreateStruct *pStruct);

		void	SetState(State eNewState);

		void	Restart();
		bool	InitialUpdate();
		bool	AllObjectsCreated();
		bool	Update();

		bool	UpdateClosing();
		bool	UpdateClosed();
		bool	UpdateFocusing();
		bool	UpdateLocked();
		bool	UpdateLingering();
		bool	UpdateSearching();

		void	ResumeKeyframer();
		void	PauseKeyframer();
		void	ResetKeyframer();

		LTFLOAT	TurnTo(const LTVector &vDir);

		bool	UpdateAnimation();
		bool	PlayAnimation(const char* szAnim);
		bool	PlayScanAnimation();

		void	CreateExplosion();
		void	HandleDeath();
		LTBOOL	Fire();
		bool	SetWeapon(int nWeaponId);
		void	UpdateScanVolume();

		LTBOOL	UpdateTargets();
		LTBOOL	AddTarget(HOBJECT hObj, LTFLOAT fTime);
		FinderTarget*	FindTarget(HOBJECT hObj);

		// RETAIL: an empty function retail calls at a few places
		// (0x1010c0c0, folded with all the empty ones); ours
		void	TronUnknownNoOp();

	private :

		CWeapon*	m_pWeapon;					// +0x8f8
		CWeapons*	m_pWeapons;					// +0x8fc
		HSTRING		m_hstrWeapon;				// +0x900, the weapon of its Type
		bool		m_bHasWeapon;				// +0x904
		LTObjRef	m_hKeyframer;				// +0x908
		HSTRING		m_hstrKeyframerName;		// +0x918
		uint32		m_xTronUnknown91c;			// +0x91c, never touched
		HSTRING		m_hstrClearAlarmCommand;	// +0x920
		HSTRING		m_hstrDeathCommand;			// +0x924
		bool		m_bFakeDeath;				// +0x928
		State		m_eState;					// +0x92c
		LTFLOAT		m_fTurnRate;				// +0x930, radians a second
		int			m_nLastTurnDir;				// +0x934, 0 left, 1 right
		LTRotation	m_rStartRot;				// +0x938
		LTFLOAT		m_fSecondsToLock;			// +0x948
		LTFLOAT		m_fSecondsToLinger;			// +0x94c
		LTFLOAT		m_fLastUpdateTime;			// +0x950
		CTimer		m_LingerTimer;				// +0x954
		LTFLOAT		m_fLingerMaxTurn;			// +0x964, radians
		LTVector	m_vEnemyPos;				// +0x968
		LTVector	m_vLingerDir;				// +0x974
		LingerState	m_eLingerState;				// +0x980
		LTFLOAT		m_fDisturbanceRadius;		// +0x984
		uint32		m_nDisturbanceAlarmLevel;	// +0x988
		EnumAIStimulusID	m_eAlarmStimulusID;	// +0x98c
		HSTRING		m_hstrDeathFX;				// +0x990
		FinderTargetList	m_lstTargets;		// +0x994
		LTVector	m_vSearchDir;				// +0x998
		bool		m_bScanInfinite;			// +0x9a4
		LTVector	m_vScanInfiniteDir;			// +0x9a8
		uint32		m_xTronUnknown9b4;			// +0x9b4, never touched
};

// RETAIL: vtable 0x10293ad0 (0x24 bytes: a CScannerPlugin)
class FinderPlugin : public CScannerPlugin
{
	public :

		// RETAIL: 0x1001fa80
		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength)
		{
			if (!sm_bInitted)
			{
				sm_bInitted = true;
			}

			if (!_strcmpi("DeathFX", szPropName))
			{
				return sm_FXPlugin.PopulateStringList(szRezPath, aszStrings,
					pcStrings, cMaxStrings, cMaxStringLength);
			}

			return CScannerPlugin::PreHook_EditStringList(szRezPath, szPropName,
				aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		}

	private :

		static bool				sm_bInitted;	// RETAIL: 0x103397fc
		static SpecialFXPlugin	sm_FXPlugin;	// RETAIL: 0x1033951c
};

#endif // __FINDER_H__
