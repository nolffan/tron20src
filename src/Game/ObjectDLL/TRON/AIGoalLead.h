// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalLead.h
//
// PURPOSE : AIGoalLead class definition: the AI walks to a node ahead of
//           the player, stopping to wait (and beckon) when the player
//           falls behind.
//
// RETAIL: Tron 2.0 only, written from retail Object.lto (RTTI CAIGoalLead,
//         vtable 0x1029c8c8; TU 0x101f8780..0x101f8eb0). Names of members
//         and methods are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_LEAD_H__
#define __AIGOAL_LEAD_H__

#include "AIGoalGoto.h"


class CAIGoalLead : public CAIGoalGoto
{
	typedef CAIGoalGoto super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalLead, kGoal_Lead);

		CAIGoalLead( );

		// RETAIL: no Save/Load of its own (slots 3/4 are CAIGoalGoto's).

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		virtual void UpdateGoal();

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

	protected:

		// State Handling.

		void	HandleStateAnimate();
		LTFLOAT	GetPlayerDistance();

	protected:

		LTFLOAT		m_fWaitDist;		// RETAIL: 0x74, "WAIT_DIST"
		LTFLOAT		m_fContinueDist;	// RETAIL: 0x78, "CONTINUE_DIST"
		bool		m_bLookAround;		// RETAIL: 0x7c
};


#endif
