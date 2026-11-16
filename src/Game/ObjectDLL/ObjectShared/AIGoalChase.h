// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalChase.h
//
// PURPOSE : AIGoalChase class definition
//
// CREATED : 6/21/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_CHASE_H__
#define __AIGOAL_CHASE_H__

#include "AIGoalAbstractSearch.h"

enum  EnumAIStateStatus;
class AIVolumeJunction;


struct JunctionRecord
{
	AIVolume* pVolume;
	uint8 mskActionVolumes;
};

typedef std::vector<JunctionRecord> JunctionStack;


// RETAIL: Tron moves NOLF2's CAIGoalChase into CAIGoalChaseAbstract (RTTI;
// vtable 0x1029d928), the base of CAIGoalChase, CAIGoalPsychoChase and
// CAIGoalRezIn. It does not override ActivateGoal: the goals call its
// ActivateChase (0x1020e6a0; name ours) themselves.
class CAIGoalChaseAbstract : public CAIGoalAbstractSearch
{
	typedef CAIGoalAbstractSearch super;

	public:

		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC(Goal);

		CAIGoalChaseAbstract( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void DeactivateGoal();

		// Updating.

		void UpdateGoal();
		virtual void RecalcImportance();

		// Command Handling.

		virtual LTBOOL	HandleNameValuePair(const char *szName, const char *szValue);

		// Triggered AI Sounds.

		virtual LTBOOL SelectTriggeredAISound(EnumAISoundType* peAISoundType);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		void			ActivateChase();

		// State Handling.

		void			HandleStateLookAt();
		void			HandleStateAware();
		virtual void	HandleStateChase();
		void			HandleStateGoto();
		virtual void	HandleStateSearch();
		void			HandleStateDraw();
		void			SetStateChase(LTBOOL bClearState);
		virtual void	SetStateSearch();
		void			SetChaseStatusLost();
		void			GiveUpChase(LTBOOL bSearch);
		void			AwareOnce();

		// Junction Volume Handling.

		void			HandleJunctionVolume();
		JunctionRecord* HandleJunctionStack();

	protected:

		AIVolume*			m_pJunctionVolume;
		AIVolume*			m_pJunctionActionVolume;
		AIVolume*			m_pLastVolume;

		JunctionStack		m_stkJunctions;

		LTBOOL				m_bLost;
		LTBOOL				m_bGiveUpChase;
		LTBOOL				m_bSeekTarget;
		LTBOOL				m_bNeverGiveUp;
		LTBOOL				m_bKeepDistance;
		LTBOOL				m_bContinueLost;
};

// RETAIL: Tron's thin chase goal (CreateAIClassInstance 0x1020dde0,
// ActivateGoal 0x1020deb0).
class CAIGoalChase : public CAIGoalChaseAbstract
{
	typedef CAIGoalChaseAbstract super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalChase, kGoal_Chase);

		// Activation.

		virtual void ActivateGoal();
};

#endif
