// ----------------------------------------------------------------------- //
//
// MODULE  : GadgetTargetFX.cpp
//
// PURPOSE : GadgetTarget - Implementation
//
// CREATED : 8/20/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GadgetTargetFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "GameClientShell.h"
#include "SFXMsgIds.h"

extern CGameClientShell* g_pGameClientShell;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGadgetTargetFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

// RETAIL: 0x101b7d20
LTBOOL CGadgetTargetFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	GADGETTARGETCREATESTRUCT* pGTCS = (GADGETTARGETCREATESTRUCT*)psfxCreateStruct;

	m_eType = pGTCS->eType;

    return LTTRUE;
}
