// ----------------------------------------------------------------------- //
//
// MODULE  : HUDPopup.cpp
//
// PURPOSE : Implementation of CHUDPopup to display popups
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "HUDMgr.h"
	#include "InterfaceResMgr.h"
	#include "PopupMgr.h"
	#include "HUDPopup.h"



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::CHUDPopup
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CHUDPopup::CHUDPopup()
:	CHUDItem		( )
{
	m_UpdateFlags	= kHUDFrame;
	m_bVisible		= LTFALSE;	
	m_bColorOverride = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::Init
//
//  PURPOSE:	Initialize the popup...
//
// ----------------------------------------------------------------------- //

LTBOOL CHUDPopup::Init()
{
	// RETAIL: 0x101877c0: Tron's text starts empty
	m_Text.Create(std::wstring(),0,0,g_pInterfaceResMgr->GetFont(0),8,LTNULL);
	m_Frame.Create(g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\frame.dtx"),200,320,LTTRUE);

	return LTTRUE;

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::Term
//
//  PURPOSE:	Destroy thyself...
//
// ----------------------------------------------------------------------- //

void CHUDPopup::Term()
{
	m_Frame.Destroy();
	m_Text.Destroy();
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::Render
//
//  PURPOSE:	Draw the popup...
//
// ----------------------------------------------------------------------- //

void CHUDPopup::Render()
{
	// RETAIL: 0x10187850: through the virtual IsVisible first
	if (!IsVisible() || !m_bVisible) return;
	m_Frame.Render();
	m_Text.Render();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::Update
//
//  PURPOSE:	Update the popup...
//
// ----------------------------------------------------------------------- //

void CHUDPopup::Update()
{
	// Sanity checks...
	if( !IsVisible() ) return;

	if( m_fScale != g_pInterfaceResMgr->GetXRatio() )
		SetScale( g_pInterfaceResMgr->GetXRatio() );

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::SetScale
//
//  PURPOSE:	Set the items scale...
//
// ----------------------------------------------------------------------- //

void CHUDPopup::SetScale(float fScale)
{
	m_Frame.SetScale( fScale );
	m_Text.SetScale( fScale );

	m_fScale = fScale;
}

