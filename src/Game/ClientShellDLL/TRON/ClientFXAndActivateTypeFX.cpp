// ----------------------------------------------------------------------- //
//
// MODULE  : ClientFXAndActivateTypeFX.cpp
//
// PURPOSE : A model's client fx (CAttachClientFX) with an activate type -
//           Implementation
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x101bffb0..
// 0x101c02b0).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientFXAndActivateTypeFX.h"

// RETAIL: Tron's message to the model's client fx (0x101c0267); the name
// is ours
#define CFXATFX_ATTACHFX_MSG	35

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientFXAndActivateTypeFX::CClientFXAndActivateTypeFX
//
//	PURPOSE:	Constructor (RETAIL: 0x101bffb0)
//
// ----------------------------------------------------------------------- //

CClientFXAndActivateTypeFX::CClientFXAndActivateTypeFX()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientFXAndActivateTypeFX::Init
//
//	PURPOSE:	Init the fx from the server's message (RETAIL: 0x101c00f0)
//
// ----------------------------------------------------------------------- //

LTBOOL CClientFXAndActivateTypeFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
	if (!CSpecialFX::Init(hServObj, pMsg)) return LTFALSE;

	CLIENTFXUSERCREATESTRUCT cs;
	cs.hServerObj = hServObj;
	cs.Read(pMsg);

	if (!m_ActivateObjectHandler.Init(hServObj)) return LTFALSE;

	m_ActivateObjectHandler.m_nId		= pMsg->Readuint8();
	m_ActivateObjectHandler.m_bDisabled	= pMsg->Readbool();
	m_ActivateObjectHandler.m_eState	= (ACTIVATETYPE::State)pMsg->Readuint8();

	return Init(&cs);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientFXAndActivateTypeFX::Init
//
//	PURPOSE:	Init the fx (RETAIL: Tron's folds with 0x101b94a0)
//
// ----------------------------------------------------------------------- //

LTBOOL CClientFXAndActivateTypeFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	m_cs = *((CLIENTFXUSERCREATESTRUCT*)psfxCreateStruct);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientFXAndActivateTypeFX::CreateObject
//
//	PURPOSE:	Start the model's client fx on the object
//				(RETAIL: 0x101c01b0)
//
// ----------------------------------------------------------------------- //

LTBOOL CClientFXAndActivateTypeFX::CreateObject(ILTClient* pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return LTFALSE;

	m_AttachClientFX.Init(m_hServerObject, m_cs.eModelId);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientFXAndActivateTypeFX::OnModelKey
//
//	PURPOSE:	Pass the model keys to the client fx (RETAIL: 0x101c0200)
//
// ----------------------------------------------------------------------- //

void CClientFXAndActivateTypeFX::OnModelKey(HLOCALOBJ hObj, ArgList *pArgs)
{
	if (!hObj || !pArgs || !pArgs->argv || !pArgs->argc) return;

	char* pKey = pArgs->argv[0];
	if (!pKey) return;

	m_AttachClientFX.OnModelKey(hObj, pArgs);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientFXAndActivateTypeFX::OnServerMessage
//
//	PURPOSE:	Handle a message from the server (RETAIL: 0x101c0240)
//
// ----------------------------------------------------------------------- //

LTBOOL CClientFXAndActivateTypeFX::OnServerMessage(ILTMessage_Read *pMsg)
{
	if (!CSpecialFX::OnServerMessage(pMsg)) return LTFALSE;

	uint8 nMsgId = pMsg->Readuint8();

	switch (nMsgId)
	{
		case ACTIVATEFX_DISABLED :
		{
			m_ActivateObjectHandler.m_bDisabled = pMsg->Readbool();
		}
		return LTTRUE;

		case ACTIVATEFX_STATE :
		{
			m_ActivateObjectHandler.m_eState = (ACTIVATETYPE::State)pMsg->Readuint8();
		}
		return LTTRUE;

		// RETAIL: Tron's returns LTFALSE after this message too
		case CFXATFX_ATTACHFX_MSG :
		{
			m_AttachClientFX.OnServerMessage(pMsg);
		}
		break;
	}

	return LTFALSE;
}
