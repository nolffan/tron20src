// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerVehicleFX.cpp
//
// PURPOSE : Beam special FX - Implementation
//
// CREATED : 6/8/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerVehicleFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "SFXMsgIds.h"
#include "GameClientShell.h"


extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerVehicleFX::Init
//
//	PURPOSE:	Init the beam fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerVehicleFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
    if (!CSpecialFX::Init(hServObj, pMsg)) return LTFALSE;
    if (!pMsg) return LTFALSE;

	PVCREATESTRUCT cs;

	cs.hServerObj = hServObj;
    cs.Read(pMsg);

	return Init(&cs);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerVehicleFX::Init
//
//	PURPOSE:	Init the player vehicle fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerVehicleFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	PVCREATESTRUCT* pFX = (PVCREATESTRUCT*)psfxCreateStruct;
	m_cs = *pFX;

	// RETAIL: 0x101b94a0 (one body with CClientFXAndActivateTypeFX::Init):
	// Tron has no radar for NOLF2's coop snowmobiles.

     return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerVehicleFX::Update
//
//	PURPOSE:	Update the tracer
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerVehicleFX::Update()
{
	if( !CSpecialFX::Update() || m_bWantRemove ) 
		return LTFALSE;

	return LTTRUE;
}