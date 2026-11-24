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
#include "Timer.h"

enum DamageType;
enum EnumAIStimulusID;

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

		// RETAIL: Tron's (0x101927d0, 0x10192850; the names are ours): the
		// PRod's and the energy claw's attacker lets go of us
		void ReleasePRodAttacker();
		void ReleaseEnergyClawAttacker();

		// RETAIL: Tron's (0x10191f50, 0x10192390; the names are ours): true
		// when the attacker's PRod or energy claw still points at us.
		bool IsPRodAttackerAiming();
		bool IsEnergyClawAttackerAiming();

		// RETAIL: Tron's (0x101928d0; the name is ours): the damage is
		// over, the AI searches or becomes aware.
		void EndSpecialDamage();

		// RETAIL: Tron's (0x10192990; the name is ours): NOLF2's
		// ActivateGoal body. ActivateGoal and UpdateGoal call it.
		void StartSpecialDamage();

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
		// RETAIL: Tron has no m_bInfinite (no SLEEPFOREVER command).
		bool				m_bNeedsClearState;		// +0x10c
		// RETAIL: Tron's (UpdateGoal 0x10190683): start the damage again on
		// the next update (the name is ours).
		bool				m_bRestartDamage;		// +0x10d
		// RETAIL: Tron's (+0x110): the PRod and energy claw damage time
		// (HandleDamage starts it for 0.5 seconds; the name is ours).
		CTimer				m_DamageTimer;			// +0x110
};


#endif
