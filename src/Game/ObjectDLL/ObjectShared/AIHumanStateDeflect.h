//----------------------------------------------------------------------------
//              
//	MODULE:		AIHumanStateDeflect.h
//              
//	PURPOSE:	CAIHumanStateDeflect declaration
//              
//	CREATED:	12.11.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __AIHUMANSTATEDEFLECT_H__
#define __AIHUMANSTATEDEFLECT_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#include "AIHumanState.h"

// Forward declarations

// Globals

// Statics

//----------------------------------------------------------------------------
//              
//	CLASS:		CAIHumanStateDeflect
//              
//	PURPOSE:	Handle deflecting an incoming projectile by raising a weapon
//				To protect ones self.  A deflection can be relative to an 
//				object or a direction.  It can terminate on either a timeout
//				or on being hit by an object.
//              
//----------------------------------------------------------------------------
// RETAIL: Tron's CAIHumanStateDeflect (0xa0 bytes, ctor 0x1022b990, vtable
// 0x1029eb90) holds a deflection (the AI's defense) for its
// MaxDeflectDuration or until it touches its object; a DEFLECT model
// string turns the defense on or off. No stop conditions or three-phase
// animation sequence (NOLF2's).
class CAIHumanStateDeflect : public CAIHumanState
{
	typedef CAIHumanState super;

	public :

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateDeflect, kState_HumanDeflect);

		CAIHumanStateDeflect( );
		virtual ~CAIHumanStateDeflect( );

		// Ctors/dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Handlers

		virtual void HandleModelString(ArgList* pArgList);
		virtual void HandleTouch(HOBJECT hObject);

		// Update

		void Update(void);
		void UpdateAnimation(void);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood(void) { return CMusicMgr::eMoodAggressive; }

		// RETAIL: inline in CAIGoalDeflect::ActivateGoal (0x1020b900).
		void SetObjectToDeflect( HOBJECT hObject ) { m_hObjectToDeflect = hObject; }

	protected:

		void SetupDeflection();
		void ShouldDropDeflection(void);

	protected:

		// RETAIL: +0x7c, +0x80 (names ours): when the deflection may end
		// early, and the time before which the defense stays on.
		LTFLOAT			m_fDeflectEndTime;
		LTFLOAT			m_fDefendTime;

		// if we Deflected the object we have a handle to, then set this to true
		bool			m_bDeflectedNamedObject;

		// If we have a specific object we mean to deflect, keep a link to it
		LTObjRef		m_hObjectToDeflect;

		// RETAIL: Tron's posture and direction props (+0x98, +0x9c). The
		// names are ours.
		EnumAnimProp	m_ePosture;
		EnumAnimProp	m_eDirection;
};

#endif // __AIHUMANSTATEDEFLECT_H__
