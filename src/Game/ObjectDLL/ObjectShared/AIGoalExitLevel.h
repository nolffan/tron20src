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
// 0x1029d4bc) give the node type and handle reaching it.
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

// RETAIL: Tron's (RTTI .?AVCAIGoalExitLevelByRezOut@@, vtable 0x1029d4bc,
// sizeof 0x94): at its AINodeExitLevelByRezOut the AI holsters its weapon
// and uses the node's smart object, then is removed. Object.lto
// 0x10208bc0..0x10208fc2. The names of the new virtuals are ours.

class CAIGoalExitLevelByRezOut : public CAIGoalExitLevelAbstract
{
	typedef CAIGoalExitLevelAbstract super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalExitLevelByRezOut, kGoal_ExitLevelByRezOut);

		 CAIGoalExitLevelByRezOut( ) {}

		// Updating.

		virtual void	UpdateGoal();						// RETAIL: 0x10208cf0

	protected:

		// State Handling.

		virtual void	HandleStateGoto();					// RETAIL: 0x10208cd0
		virtual EnumAINodeType	GetNodeType() { return kNode_ExitLevelByRezOut; }	// RETAIL: 0x10208cc0

		virtual void	StartRezOut();						// RETAIL: 0x10208d30
		virtual void	HandleStateUseObject();				// RETAIL: 0x10208e10
		virtual void	HandleStateHolster();				// RETAIL: 0x10208fa0
};

#endif
