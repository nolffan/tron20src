// ----------------------------------------------------------------------- //
//
// MODULE  : ClientFXAndActivateTypeFX.h
//
// PURPOSE : A model's client fx (CAttachClientFX) with an activate type -
//           Definition
//
// RETAIL: Tron 2.0's (RTTI CClientFXAndActivateTypeFX, CShell.dll vtable
// 0x101ec9b0; functions 0x101bffb0..0x101c02b0). SFX id 61. The member
// names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENTFX_AND_ACTIVATETYPE_FX_H__
#define __CLIENTFX_AND_ACTIVATETYPE_FX_H__

#include "SpecialFX.h"
#include "SharedFXStructs.h"
#include "AttachClientFX.h"
#include "ActivateObjectFX.h"

class CClientFXAndActivateTypeFX : public CSpecialFX
{
	public :

		CClientFXAndActivateTypeFX();
		// RETAIL: no destructor of its own (the compiler's, 0x101c0070)

		virtual LTBOOL	Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
		virtual LTBOOL	Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL	CreateObject(ILTClient* pClientDE);
		virtual void	OnModelKey(HLOCALOBJ hObj, ArgList *pArgs);
		virtual LTBOOL	OnServerMessage(ILTMessage_Read *pMsg);

		virtual uint32	GetSFXID() { return SFX_CLIENTFXANDACTIVATETYPE_ID; }

	protected :

		CLIENTFXUSERCREATESTRUCT	m_cs;						// +0x38
		CAttachClientFX				m_AttachClientFX;			// +0x44
		CActivateObjectHandler		m_ActivateObjectHandler;	// +0x54
};

#endif // __CLIENTFX_AND_ACTIVATETYPE_FX_H__
