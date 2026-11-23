// ----------------------------------------------------------------------- //
//
// MODULE  : PickupItem.h
//
// PURPOSE : PickupItem - Definition
//
// CREATED : 8/20/98
//
// ----------------------------------------------------------------------- //

#ifndef __PICKUPITEM_FX_H__
#define __PICKUPITEM_FX_H__

#include "SpecialFX.h"

struct PICKUPITEMCREATESTRUCT : public SFXCREATESTRUCT
{
    PICKUPITEMCREATESTRUCT();

    LTBOOL		bRotate;
    LTBOOL		bBounce;
};

inline PICKUPITEMCREATESTRUCT::PICKUPITEMCREATESTRUCT()
{
    bRotate = LTFALSE;
    bBounce = LTFALSE;
}

// RETAIL: Tron's is Build 51's (0x40 bytes, vtable 0x101eaf80: no
// client fx, no team id, no OnServerMessage): CSFXMgr's PickupItem bank
// shares CObjSpriteFX's Delete (0x101115c0) for the same size
class CPickupItemFX : public CSpecialFX
{
	public :

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_PICKUPITEM_ID; }

	private :

        LTBOOL m_bRotate;
        LTBOOL m_bBounce;
};

#endif // __PICKUPITEM_FX_H__