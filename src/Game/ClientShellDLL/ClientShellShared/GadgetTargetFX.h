// ----------------------------------------------------------------------- //
//
// MODULE  : GadgetTargetFX.h
//
// PURPOSE : GadgetTargetFX - Definition
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GADGETTARGET_FX_H__
#define __GADGETTARGET_FX_H__

#include "SpecialFX.h"
#include "GadgetTargetTypes.h"

// RETAIL: Tron's keeps the type only (CSFXMgr::HandleSFXMsg reads one byte,
// 0x101061f1; CGadgetTargetFX::Init 0x101b7d20; the fx is 0x3c bytes)
struct GADGETTARGETCREATESTRUCT : public SFXCREATESTRUCT
{
    GADGETTARGETCREATESTRUCT();

    GadgetTargetType	eType;
};

inline GADGETTARGETCREATESTRUCT::GADGETTARGETCREATESTRUCT()
{
	eType = eINVALID;
}

class CGadgetTargetFX : public CSpecialFX
{
	public :

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);

		virtual uint32 GetSFXID() { return SFX_GADGETTARGET_ID; }

		virtual GadgetTargetType	GetType() const {return m_eType;}

	private :

	    GadgetTargetType	m_eType;		// +0x38
};

#endif // __GADGETTARGET_FX_H__