// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttack.h
//
// PURPOSE : AIGoalAttack class definition
//
// CREATED : 7/17/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ATTACK_H__
#define __AIGOAL_ATTACK_H__

#include "AIGoalAbstractStimulated.h"
#include "AnimationProp.h"
#include "AINode.h"
#include "AITarget.h"

// Forward Declarations.

enum EnumAIWeaponType;


class CAIGoalAttack : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttack, kGoal_Attack);

		CAIGoalAttack( );

		// RETAIL: Tron's (0x10215e70): sets up the goal's own target.
		virtual void	InitGoal(CAI* pAI);

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		void UpdateGoal();

		// Attractor Handling.

		virtual AINode*	HandleGoalAttractors();
		virtual AINode* FindNearestAttractorNode(EnumAINodeType eNodeType, const LTVector& vPos, LTFLOAT fDistSqr);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

	protected:

		// State Setting.

		virtual void SetStateAttack();
		// RETAIL: Tron's (slot 0x5c, retail 0x10210f10): HandleGoalAttractors
		// asks it in place of NOLF2's !m_bForceSearch (0x1021662d). The name
		// is ours.
		virtual bool ShouldClearNode() { return !m_bForceSearch; }
				void SetStatePanic();

		void ActivateChangeWeapons();

		// State Handling.

		void HandleStateAttack();
		void HandleStateFlee();
		void HandleStatePanic();
		void HandleStateHolster();
		void HandleStateDraw();

		void HandlePotentialWeaponChange();
		AINodeChangeWeapons* GetNodeAsChangeWeapons() const;
		const char* const GetWeaponChangeDescription() const;


	protected:

		EnumAIWeaponType	m_eWeaponType;

		LTFLOAT				m_fChaseDelay;
		EnumAnimProp		m_ePosture;
		LTBOOL				m_bPanicCanActivate;

		// Some attacks are node-based (cover, vantage, view).

		LTBOOL				m_bNodeBased;
		LTBOOL				m_bForceSearch;
		LTObjRef			m_hNode;
		LTObjRef			m_hFailedNode;

		// Remember if we checked for a weapon change yet.

		LTBOOL				m_bCheckForWeaponChange;

		// RETAIL: Tron's (ctor 0x10215c60, InitGoal 0x10215e70): the goal's
		// own target (+0xdc) and whether the AI's RangeAttackIsRecessive
		// data is set (+0x19c). Both names are ours.
		CGenericTarget<CAIGoalAbstract>	m_Target;
		LTBOOL				m_bRangeAttackIsRecessive;
};


#endif
