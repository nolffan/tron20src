//----------------------------------------------------------------------------
//              
//	MODULE:		IHitBoxUser.h
//              
//	PURPOSE:	IHitBoxUser declaration
//              
//	CREATED:	11.04.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __IHITBOXUSER_H__
#define __IHITBOXUSER_H__

#include "GameBase.h"

class CAttachments;
class CAnimator;
enum ModelNode;
enum ModelSkeleton;

//----------------------------------------------------------------------------
//              
//	STRUCT:	IHitBoxUser
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
struct IHitBoxUser
{
	public:
		virtual CAttachments* GetAttachments() = 0;
		virtual ModelSkeleton GetModelSkeleton() const = 0;
		virtual void SetModelNodeLastHit(ModelNode eModelNode ) = 0;
		virtual LTBOOL UsingHitDetection() const = 0;
		virtual HATTACHMENT GetAttachment() const = 0;
		virtual LTFLOAT ComputeDamageModifier(ModelNode eModelNode) = 0;
		virtual float GetNodeRadius( ModelSkeleton, ModelNode ) = 0;
		virtual void UpdateClientHitBox() = 0;
		virtual CAnimator*	GetAnimator() { return LTNULL; }

		// RETAIL: Tron's last two slots: one argument, empty in CCharacter and
		// Body; and the user's own object (both return their m_hObject, retail
		// 0x10062940, 0x1001b900). Not yet named
		virtual void	TronUnknownVirtual24( int nArg ) {}
		virtual HOBJECT	TronUnknownVirtual28() = 0;
};

#endif // __IHITBOXUSER_H__

