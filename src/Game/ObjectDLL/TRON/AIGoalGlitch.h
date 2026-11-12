// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalGlitch.h
//
// PURPOSE : AIGoalGlitch class definition: a working AI that now and then
//           glitches (plays its Glitch animation) on its way to the node.
//
// RETAIL: Tron 2.0 only, written from retail Object.lto (RTTI CAIGoalGlitch,
//         vtable 0x1029c934; TU 0x101f8ef0..0x101f94f0). Names of members
//         and methods are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_GLITCH_H__
#define __AIGOAL_GLITCH_H__

#include "AIGoalWork.h"


class CAIGoalGlitch : public CAIGoalWork
{
	typedef CAIGoalWork super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalGlitch, kGoal_Glitch);

		// RETAIL: 0x101f91c0 (resets the vtables, then ~CAIGoalWork)
		virtual ~CAIGoalGlitch() {}

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		virtual void UpdateGoal();
		virtual void RecalcImportance();

	protected:

		bool		m_bGlitching;			// RETAIL: 0x108
		LTFLOAT		m_fNextGlitchTime;		// RETAIL: 0x10c
};


#endif
