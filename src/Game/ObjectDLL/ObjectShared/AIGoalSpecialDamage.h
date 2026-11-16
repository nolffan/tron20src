// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSpecialDamage.h
//
// PURPOSE : AIGoalSpecialDamage class definition
//
// CREATED : 3/21/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_SPECIAL_DAMAGE_H__
#define __AIGOAL_SPECIAL_DAMAGE_H__

#include "AIGoalAbstractSearch.h"
#include "AnimationMgr.h"

enum DamageType;
enum EnumAIStimulusID;

// RETAIL: Tron's (+0x110 in CAIGoalSpecialDamage): a start time, a
// duration (HandleDamage sets 0.5), a time and a flag; built before the
// vtable is set (0x1018fd45), so it has its own constructor. Names ours.
struct CAITronSpecialDamageTimer
{
	CAITronSpecialDamageTimer()
	{
		m_fStartTime = 0.f;
		m_fDuration = 0.f;
		m_fTime = 0.f;
		m_bOn = false;
	}

	LTFLOAT		m_fStartTime;	// +0x0 (saved as a time)
	LTFLOAT		m_fDuration;	// +0x4
	LTFLOAT		m_fTime;		// +0x8 (saved as a time)
	bool		m_bOn;			// +0xc
};

class CAIGoalSpecialDamage : public CAIGoalAbstractSearch
{
	typedef CAIGoalAbstractSearch super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalSpecialDamage, kGoal_SpecialDamage);

		CAIGoalSpecialDamage( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		virtual void UpdateGoal();

		// Damage Handling.

		virtual LTBOOL HandleDamage(const DamageStruct& damage);
		virtual HMODELANIM GetAlternateDeathAnimation();

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

		// Access.

		void InterruptSpecialDamage(LTBOOL bSearch);
		void PauseSpecialDamage(LTBOOL bPause);

	protected:

		// State Handling.

		void HandleStateGoto();
		void HandleStateAware();
		void HandleStatePanic();
		void HandleStateUseObject();
		void HandleStateIdle();

		// Activity.

		void HandleActivity();

		// Awareness.

		void RestoreAwareness();

	protected:

		DamageType			m_eDamageType;
		EnumAnimProp		m_eDamageAnim;
		EnumAIStimulusID	m_eSpecialDamageStimID;
		LTBOOL				m_bIncapacitated;
		LTBOOL				m_bProgressiveDamage;
		LTFLOAT				m_fProgressiveMinTime;
		LTObjRef			m_hDamager;				// +0xd0
		// RETAIL: Tron's (ctor 0x1018fcd0, Save 0x10190140): two more
		// objects, saved after the damager (names ours).
		LTObjRef			m_hTronUnknownE0;		// +0xe0
		LTObjRef			m_hTronUnknownF0;		// +0xf0
		LTBOOL				m_bFleeing;				// +0x100
		LTFLOAT				m_fRelaxTime;			// +0x104
		LTFLOAT				m_fTronUnknown108;		// +0x108, saved after the two bools
		bool				m_bInfinite;			// +0x10c, a bool in Tron
		bool				m_bNeedsClearState;		// +0x10d
		CAITronSpecialDamageTimer	m_TronUnknown110;	// +0x110
};


#endif
