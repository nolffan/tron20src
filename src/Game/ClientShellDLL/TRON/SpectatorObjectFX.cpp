// ----------------------------------------------------------------------- //
//
// MODULE  : SpectatorObjectFX.cpp
//
// PURPOSE : Spectator object special fx - Implementation
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x101c02c0..
// 0x101c0387).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SpectatorObjectFX.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpectatorObjectFX::Init
//
//	PURPOSE:	Init the fx from the server's message (RETAIL: 0x101c02c0)
//
// ----------------------------------------------------------------------- //

LTBOOL CSpectatorObjectFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
	if (!hServObj) return LTFALSE;
	if (!CSpecialFX::Init(hServObj, pMsg)) return LTFALSE;

	m_bTronUnknown38 = pMsg->Readbool();
	m_vTronUnknown3C = pMsg->ReadLTVector();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpectatorObjectFX::OnServerMessage
//
//	PURPOSE:	Handle a message from the server (RETAIL: 0x101c0330)
//
// ----------------------------------------------------------------------- //

LTBOOL CSpectatorObjectFX::OnServerMessage(ILTMessage_Read *pMsg)
{
	if (!CSpecialFX::OnServerMessage(pMsg)) return LTFALSE;

	m_bTronUnknown38 = pMsg->Readbool();
	m_vTronUnknown3C = pMsg->ReadLTVector();

	return LTTRUE;
}
