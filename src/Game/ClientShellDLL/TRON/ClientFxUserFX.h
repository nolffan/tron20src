// ----------------------------------------------------------------------- //
//
// MODULE  : ClientFxUserFX.h
//
// PURPOSE : RETAIL: Tron 2.0's, retail-only (RTTI .?AVCClientFxUserFX@@,
//           vtable 0x101eb188; no source anywhere): the client side of an
//           object that plays its model's attached client fx
//           (CLIENTFXUSERCREATESTRUCT, SFX_CLIENTFXUSER_ID). The class is
//           inline: its code is CSFXMgr's TU's (CShell.dll 0x10113920..
//           0x10113ab0, after its banked list's Delete and New). The
//           member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_FX_USER_FX_H__
#define __CLIENT_FX_USER_FX_H__

#include "SpecialFX.h"
#include "AttachClientFX.h"

class CClientFxUserFX : public CSpecialFX
{
	public :

		// RETAIL: inline (CBankedList<CClientFxUserFX>::New, 0x10113740)
		CClientFxUserFX() {}

		// RETAIL: the destructor is the compiler's (0x10113ab0)

		// RETAIL: 0x10113920
		virtual LTBOOL Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
		{
			if (!CSpecialFX::Init(hServObj, pMsg)) return LTFALSE;

			CLIENTFXUSERCREATESTRUCT cs;
			cs.hServerObj = hServObj;
			cs.Read(pMsg);

			return Init(&cs);
		}

		// RETAIL: 0x101139a0
		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct)
		{
			if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

			m_cs = *((CLIENTFXUSERCREATESTRUCT*)psfxCreateStruct);

			return LTTRUE;
		}

		// RETAIL: 0x101139d0: the model's fx go on the server object
		virtual LTBOOL CreateObject(ILTClient* pClientDE)
		{
			if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

			if (m_hServerObject)
			{
				m_AttachFX.Init(m_hServerObject, m_cs.eModelId);
				return LTTRUE;
			}

			return LTFALSE;
		}

		// RETAIL: 0x10113a20
		virtual void OnModelKey(HLOCALOBJ hObj, ArgList *pArgs)
		{
			if (!hObj || !pArgs || !pArgs->argv || !pArgs->argc || !pArgs->argv[0]) return;

			m_AttachFX.OnModelKey(hObj, pArgs);
		}

		// RETAIL: 0x10113a60
		virtual LTBOOL OnServerMessage(ILTMessage_Read *pMsg)
		{
			if (!CSpecialFX::OnServerMessage(pMsg)) return LTFALSE;

			return m_AttachFX.OnServerMessage(pMsg);
		}

		// RETAIL: 0x10113a80
		virtual uint32 GetSFXID() { return SFX_CLIENTFXUSER_ID; }

	protected :

		CLIENTFXUSERCREATESTRUCT	m_cs;			// +0x38
		CAttachClientFX				m_AttachFX;		// +0x44
};

#endif // __CLIENT_FX_USER_FX_H__
