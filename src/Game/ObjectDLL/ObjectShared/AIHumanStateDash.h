//----------------------------------------------------------------------------
//
//	MODULE:		AIHumanStateDash.h
//
//	PURPOSE:	CAIHumanStateDash declaration
//
//	RETAIL:		Tron 2.0 only (no source anywhere): written from the retail
//				Object.lto, 0x1022cc90..0x1022d490 (vtable 0x1029ec98,
//				state type 26). The AI runs at its target, then dashes
//				past it. Member names ours.
//
//----------------------------------------------------------------------------

#ifndef __AIHUMANSTATEDASH_H__
#define __AIHUMANSTATEDASH_H__

#include "AIHumanState.h"

class CAIHumanStateDash : public CAIHumanState
{
	typedef CAIHumanState super;

	public :

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateDash, kState_HumanDash);

		CAIHumanStateDash( );
		~CAIHumanStateDash( );

		// Ctors/Dtors/etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

	protected :

		// RETAIL: a squared distance (+0x7c) under which the AI does not
		// dash; nothing in retail sets it but Load.
		LTFLOAT		m_fTronUnknown7C;
		bool		m_bDashing;
};

#endif // __AIHUMANSTATEDASH_H__
