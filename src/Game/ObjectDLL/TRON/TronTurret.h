// ----------------------------------------------------------------------- //
//
// MODULE  : TronTurret.h
//
// PURPOSE : A turret that turns towards what it senses and fires a weapon
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x10068eb0..0x1006cdf0, its class def at 0x102dee90, its
// vtables 0x102953ec, 0x10295384 (IAISensing), 0x1029536c
// (IObjectRelationMgrUser)). The names of the class, its plugin and its
// properties are retail's (RTTI, class def); the rest are ours, most of
// them after CScanner's, which it follows closely.
//
// ----------------------------------------------------------------------- //

#ifndef __TRONTURRET_H__
#define __TRONTURRET_H__

#include "GameBase.h"
#include "AISensing.h"
#include "IObjectRelationMgrUser.h"
#include "Weapons.h"
#include "PropTypeMgr.h"

class CObjectRelationMgr;
class RelationSet;
class RelationData;
class CAIStimulusRecord;

// RETAIL: the seven string properties, kept until the objects are all
// created (retail's destructor of it, 0x10069e70, is out of line). Ours
// names.
struct TronTurretStrings
{
	std::string	sParent;
	std::string	sParentSocket;
	std::string	sAlignment;
	std::string	sSensingSocket;
	std::string	sFireSocket;
	std::string	sTarget;
	std::string	sWeapon;
};

class TronTurret : public GameBase, public IAISensing, public IObjectRelationMgrUser
{
	public :

		TronTurret();
		~TronTurret();

		// IAISensing members.

		virtual HOBJECT		GetSensingObject() { return m_hObject; }

		virtual LTBOOL		IsSensing() { return m_bSensing; }
		virtual LTBOOL		IsAlert() { return m_bAlert; }

		virtual LTFLOAT		GetSenseUpdateRate() const { return m_fSenseUpdateRate; }
		virtual LTFLOAT		GetNextSenseUpdate() const { return m_fNextSenseUpdate; }
		virtual void		SetNextSenseUpdate(LTFLOAT fTime) { m_fNextSenseUpdate = fTime; }

		virtual	void		UpdateSensingMembers() {}

		virtual	LTBOOL		GetDoneProcessingStimuli() const;
		virtual	void		SetDoneProcessingStimuli(LTBOOL bDone);
		virtual void		ClearProcessedStimuli();
		virtual LTBOOL		ProcessStimulus(CAIStimulusRecord* pRecord);

		virtual int			GetIntersectSegmentCount() const { return m_cIntersectSegmentCount; }
		virtual void		ClearIntersectSegmentCount() { m_cIntersectSegmentCount = 0; }
		virtual void		IncrementIntersectSegmentCount() { ++m_cIntersectSegmentCount; }

		virtual LTBOOL		HandleSenseRecord(CAIStimulusRecord* pStimulusRecord, uint32 nCycle);
		virtual	void		HandleSenses(uint32 nCycle) {}
		virtual void		HandleSenseTrigger(AISenseRecord* pSenseRecord) {}
		virtual LTFLOAT		GetSenseDistance(EnumAISenseType eSenseType);

		virtual uint32		GetCurSenseFlags() const { return m_dwSenses; }

		virtual const RelationSet& GetSenseRelationSet() const;

		virtual const LTVector& GetSensingPosition() const { return m_tSensing.m_Pos; }

		virtual CRange<int>& GetSightGridRangeX() { return m_rngSightGrid; }
		virtual CRange<int>& GetSightGridRangeY() { return m_rngSightGrid; }

		// IObjectRelationMgrUser members (Tron's).

		virtual LTBOOL			IsDead() const { return LTFALSE; }
		virtual const RelationSet&	GetRelationSet() const;
		virtual HOBJECT			GetUserObject() const { return m_hObject; }
		virtual const RelationData&	GetRelationData() const;
		virtual HOBJECT			GetRegionObject() { return LTNULL; }

        static bool DefaultFilterFn(HOBJECT hObj, void *pUserData);
        static bool BodyFilterFn(HOBJECT hObj, void *pUserData);

	protected :

        virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

		LTBOOL	ReadProp(ObjectCreateStruct *pStruct);
		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		void	HandleAllObjectsCreated();
		bool	InitWeapon();
		void	EnableSensing(bool bEnable);
		LTBOOL	CanSeeObject(ObjectFilterFn ofn, HOBJECT hObject);
		bool	InitialUpdate();
		LTFLOAT	GetTurnAmount(const LTRotation &rRot, const LTVector &vDir);
		bool	Update();
		HOBJECT	GetTarget();
		WeaponState	Fire();
		void	UpdateTarget();

		TronTurretStrings	m_Strings;			// RETAIL: +0x40

		uint32		m_eState;					// RETAIL: +0x94 (0 idle, 1 tracking, 2 firing, 3 returning)
		uint32		m_eLastState;				// +0x98

		HLTSOUND	m_hLoopSound;				// +0x9c
		HLTSOUND	m_hStartSound;				// +0xa0
		HLTSOUND	m_hStopSound;				// +0xa4
		HLTSOUND	m_hFireSound;				// +0xa8

		LTFLOAT		m_fLastUpdateTime;			// +0xac
		LTFLOAT		m_fNextFireTime;			// +0xb0

		HOBJECT		m_hParent;					// +0xb4
		HMODELSOCKET m_hParentSocket;			// +0xb8
		bool		m_bHasParent;				// +0xbc
		bool		m_bHasParentSocket;			// +0xbd

		LTransform	m_tInitial;					// +0xc0

		LTFLOAT		m_fYaw;						// +0xe8
		LTFLOAT		m_fLastYaw;					// +0xec

		CWeapons	m_Weapons;					// +0xf0
		CWeapon*	m_pWeapon;					// +0x128

		LTFLOAT		m_fCosViewFOV;				// +0x12c
		LTFLOAT		m_fSpotRange;				// +0x130
		LTFLOAT		m_fSpotRangeSqr;			// +0x134
		LTFLOAT		m_fTurnRate;				// +0x138 (radians a second)
		uint8		m_nRateOfFire;				// +0x13c (shots a minute)
		bool		m_bOn;						// +0x13d
		LTFLOAT		m_fCosFireFOV;				// +0x140
		LTFLOAT		m_fIgnoreDistSqr;			// +0x144

		HMODELSOCKET m_hSensingSocket;			// +0x148
		bool		m_bHasSensingSocket;		// +0x14c
		LTransform	m_tSensing;					// +0x150
		bool		m_bHasFireSocket;			// +0x178
		HMODELSOCKET m_hFireSocket;				// +0x17c
		LTransform	m_tFire;					// +0x180

		bool		m_bHasTarget;				// +0x1a8
		LTObjRef	m_hTarget;					// +0x1ac

		LTObjRef			m_hStimulus;		// +0x1bc
		EnumAIStimulusID	m_eStimulusID;		// +0x1cc
		EnumAISenseType		m_eStimulusSenseType;	// +0x1d0
		LTFLOAT				m_fStimulusTime;	// +0x1d4
		LTFLOAT				m_fLastResetTime;	// +0x1d8
		bool				m_bSensing;			// +0x1dc
		bool				m_bAlert;			// +0x1dd

		// IAISensing members.

		uint32						m_dwSenses;					// +0x1e0
		LTFLOAT						m_fSenseUpdateRate;			// +0x1e4
		LTFLOAT						m_fNextSenseUpdate;			// +0x1e8
		CRange<int>					m_rngSightGrid;				// +0x1ec
		LTBOOL						m_bDoneProcessingStimuli;	// +0x1f4
		AI_PROCESSED_STIMULI_MAP	m_mapProcessedStimuli;		// +0x1f8
		int							m_cIntersectSegmentCount;	// +0x204

		CObjectRelationMgr*			m_pObjectRelationMgr;		// +0x208
};

class TronTurretPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);

	protected :

		CPropTypeMgrPlugin m_PropTypeMgrPlugin;
};

#endif // __TRONTURRET_H__
