// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalRezIn.h
//
// PURPOSE : AIGoalRezIn class definition
//
// RETAIL: Tron 2.0 only, written from retail Object.lto (TU 0x101f6a10..
//         0x101f6f00, vtable 0x1029c49c). The class name is retail's (RTTI);
//         the names of methods that are not virtuals of the base are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_REZ_IN_H__
#define __AIGOAL_REZ_IN_H__

#include "AIGoalChase.h"

// An AI that rezzes in: it plays its rez in animation hidden from the
// player's view, then chases.
class CAIGoalRezIn : public CAIGoalChaseAbstract
{
	typedef CAIGoalChaseAbstract super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalRezIn, kGoal_RezIn);

		CAIGoalRezIn( );

		// RETAIL: 0x101f6b60 (resets the vtables, then the chase members)
		virtual ~CAIGoalRezIn() {}

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		virtual void UpdateGoal();

	protected:

		// State Setting.

		void SetStateRezIn();

		// State Handling.

		void HandleStateAnimate();
};

#endif
