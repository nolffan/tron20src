// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalExitLevel.h
//
// PURPOSE : AIGoalExitLevel class definition
//
// CREATED : 3/16/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_EXIT_LEVEL_H__
#define __AIGOAL_EXIT_LEVEL_H__

#include "AIGoalGuard.h"
#include "AINode.h"


// RETAIL: Tron splits the goal: CAIGoalExitLevelAbstract (RTTI; ctor
// 0x10208870, vtable 0x1029d3d4) holds the timing, Save/Load and importance;
// CAIGoalExitLevel (vtable 0x1029d448) and CAIGoalExitLevelByRezOut (vtable
// 0x1029d4bc, not written yet) give the node type and handle reaching it.
class CAIGoalExitLevelAbstract : public CAIGoalGuard
{
	typedef CAIGoalGuard super;

	public:

		 CAIGoalExitLevelAbstract( );

		// Initialization

		virtual void	InitGoal(CAI* pAI);

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Updating.

		virtual void	RecalcImportance();

	protected:

		// State Handling.

		virtual void	HandleStateGoto() = 0;

		// RETAIL: slot 0x64, the type of node to exit at (0x11 in
		// CAIGoalExitLevel at 0x10208af0). The name is ours.
		virtual EnumAINodeType	GetNodeType() = 0;

	protected:

		LTFLOAT		m_fExitAllowedTime;
};

class CAIGoalExitLevel : public CAIGoalExitLevelAbstract
{
	typedef CAIGoalExitLevelAbstract super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalExitLevel, kGoal_ExitLevel);

		 CAIGoalExitLevel( ) {}

	protected:

		// State Handling.

		virtual void	HandleStateGoto();

		virtual EnumAINodeType	GetNodeType() { return kNode_ExitLevel; }
};

#endif
