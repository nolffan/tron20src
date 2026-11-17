// ----------------------------------------------------------------------- //
//
// MODULE  : RadarObjectFX.h
//
// PURPOSE : RadarObject special fx class - Definition
//
// CREATED : 6/6/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "RadarObjectFX.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRadarObjectFX::CRadarObjectFX
//
//	PURPOSE:	ctor
//
// ----------------------------------------------------------------------- //

CRadarObjectFX::CRadarObjectFX( )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRadarObjectFX::~CRadarObjectFX
//
//	PURPOSE:	dtor
//
// ----------------------------------------------------------------------- //

CRadarObjectFX::~CRadarObjectFX( )
{
	// RETAIL: Tron's (0x101bfd80) has no radar to remove the object from
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RadarObjectFX::Init
//
//	PURPOSE:	Init the RadarObject fx
//
// ----------------------------------------------------------------------- //

LTBOOL CRadarObjectFX::Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg )
{
	if( !hServObj ) return LTFALSE;
	if( !CSpecialFX::Init( hServObj, pMsg )) return LTFALSE;
	
	if( !g_pGameClientShell->ShouldUseRadar() )
		return LTFALSE;

	m_cs.hServerObj = hServObj;
	m_cs.Read( pMsg );

	// RETAIL: Tron's (0x101bfdb0) only reads the create struct: it has no
	// radar to add the object to

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRadarObjectFX::OnServerMessage
//
//	PURPOSE:	Handle any messages from our server object...
//
// ----------------------------------------------------------------------- //

LTBOOL CRadarObjectFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg )) return LTFALSE;

	m_cs.Read( pMsg );

	// RETAIL: Tron's (0x101bfe10) only reads the create struct: it has no
	// radar to update

	return LTTRUE;
}

