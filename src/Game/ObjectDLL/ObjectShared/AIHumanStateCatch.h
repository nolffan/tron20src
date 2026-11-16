//----------------------------------------------------------------------------
//              
//	MODULE:		AIHumanStateCatch.h
//              
//	PURPOSE:	CAIHumanStateCatch declaration
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

#ifndef __AIHUMANSTATECATCH_H__
#define __AIHUMANSTATECATCH_H__

//#if _MSC_VER >= 1000
//#pragma once
//#endif // _MSC_VER >= 1000

// Includes
#include "AIHumanState.h"

// Forward declarations

// Globals

// Statics


//----------------------------------------------------------------------------
//              
//	CLASS:		CAIHumanStateCatch
//              
//	PURPOSE:	Handle catching an incoming projectile, but raising hands, and
//				either waiting for it to show up, or timing out if it doesn't 
//				show up soon enough.
//              
//----------------------------------------------------------------------------
// RETAIL: Tron's CAIHumanStateCatch (0xa4 bytes, ctor 0x1022b180, vtable
// 0x1029eb0c) holds a catch until it touches its object; it has no
// three-phase animation sequence, no max catch duration and no catch speed
// (NOLF2's). It plays the AI's posture and, while holding, the direction
// it picked.
class CAIHumanStateCatch : public CAIHumanState
{
	public :

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateCatch, kState_HumanCatch);

		CAIHumanStateCatch( );

		// Handlers

		virtual void HandleTouch(HOBJECT hObject);

		// Ctors/dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update(void);
		void UpdateAnimation(void);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood(void) { return CMusicMgr::eMoodAggressive; }

		void SetObjectToCatch( HOBJECT hObject);

	protected:
		void HandleStartCatch();
		void HandleHoldCatch();

		LTBOOL ShouldDropCatch(void);

	protected:

		// Direction to the object we mean to Catch, NULL
		// if no direction is set or to be used.
		LTVector		m_vIncomingPosition;

		// If we have a specific object we mean to Catch, keep a link to it
		// BUT be sure that we are handling it if it fails
		LTObjRef		m_hObjectToCatch;

		// if we Catch the object we have a handle to, then set this to true
		bool			m_bCaughtNamedObject;

		// RETAIL: Tron's posture and direction props (+0x9c, +0xa0). The
		// names are ours.
		EnumAnimProp	m_ePosture;
		EnumAnimProp	m_eDirection;
};

#endif // __AIHUMANSTATECATCH_H__
