// ----------------------------------------------------------------------- //
//
// MODULE  : AttachedExplosion.h
//
// PURPOSE : An explosion that follows the object it is attached to
//
// RETAIL: Tron 2.0's, written from retail Object.lto (RTTI
//         .?AVAttachedExplosion@@, vtable 0x10297cb8, ClassDef 0x102f4d10,
//         0x16c bytes; code 0x100f0c20..0x100f0fc8). Member and function
//         names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __ATTACHED_EXPLOSION_H__
#define __ATTACHED_EXPLOSION_H__

#include "Explosion.h"

class AttachedExplosion : public Explosion
{
	public :

		AttachedExplosion();				// RETAIL: 0x100f0c90

	protected :

		// RETAIL: slot 24 (0x100f0dc0): "ATTACH" from the object this
		// explosion is attached to makes it the target
		virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		virtual void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);	// slot 27 (0x100f0f60)
		virtual void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);		// slot 28 (0x100f0f90)
		virtual void Update();											// slot 29 (0x100f0da0)

		// RETAIL: slot 30 (0x100f0eb0): the position of the attachment
		virtual LTRESULT GetAttachedPos(LTVector *pvPos);

		LTObjRef		m_hTarget;			// 0x15c
};

#endif // __ATTACHED_EXPLOSION_H__
