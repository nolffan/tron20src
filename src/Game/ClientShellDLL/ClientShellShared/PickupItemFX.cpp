// ----------------------------------------------------------------------- //
//
// MODULE  : PickupItemFX.cpp
//
// PURPOSE : PickupItem - Implementation
//
// CREATED : 8/20/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PickupItemFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "GameClientShell.h"
#include "SFXMsgIds.h"

extern CGameClientShell* g_pGameClientShell;

#define PICKUPITEM_ROTVEL	0.3333f * MATH_CIRCLE

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

// RETAIL: 0x101b78c0 (Build 51's)
LTBOOL CPickupItemFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	PICKUPITEMCREATESTRUCT* pPICS = (PICKUPITEMCREATESTRUCT*)psfxCreateStruct;

	m_bRotate = pPICS->bRotate;
	m_bBounce = pPICS->bBounce;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::CreateObject
//
//	PURPOSE:	Create object associated the fx
//
// ----------------------------------------------------------------------- //

// RETAIL: 0x101b78f0
LTBOOL CPickupItemFX::CreateObject(ILTClient *pClientDE)
{
    LTBOOL bRet = CSpecialFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::Update
//
//	PURPOSE:	Update the pickupitem
//
// ----------------------------------------------------------------------- //

// RETAIL: 0x101b7920 (Build 51's: no client fx to show or hide)
LTBOOL CPickupItemFX::Update()
{
    if (!m_pClientDE || m_bWantRemove || !m_hServerObject) return LTFALSE;

    LTFLOAT fDeltaTime = g_pGameClientShell->GetFrameTime();

	if (m_bRotate)
	{
        LTRotation rRot;
		g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
		rRot.Rotate(rRot.Up(), PICKUPITEM_ROTVEL * fDeltaTime);
		g_pLTClient->SetObjectRotation(m_hServerObject, &rRot);
	}

	if (m_bBounce)
	{

	}


    return LTTRUE;
}
