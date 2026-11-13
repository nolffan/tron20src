// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIListItemCtrl.cpp
//
// PURPOSE : A text control that sends a command of its own when chosen
//
// ----------------------------------------------------------------------- //

// RETAIL: Tron's, written from retail CShell.dll (0x10048ef0..0x1004922b;
// see LTGUIListItemCtrl.h)

#include "stdafx.h"
#include "LTGUIListItemCtrl.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTGUIListItemCtrl::CLTGUIListItemCtrl
//
//	PURPOSE:	Constructor (RETAIL: 0x10048ef0)
//
// ----------------------------------------------------------------------- //

CLTGUIListItemCtrl::CLTGUIListItemCtrl()
{
	m_nSelectCommandID = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTGUIListItemCtrl::Create
//
//	PURPOSE:	Create the control (RETAIL: 0x10049110)
//
// ----------------------------------------------------------------------- //

LTBOOL CLTGUIListItemCtrl::Create(std::wstring sText, uint32 nCommandID, uint32 nSelectCommandID, uint32 nHelpID,
								  CUIFont *pFont, uint8 nFontSize, CLTGUICommandHandler *pCommandHandler,
								  uint32 nParam1, uint32 nParam2)
{
	m_nSelectCommandID = nSelectCommandID;
	return CLTGUITextCtrl::Create(sText, nCommandID, nHelpID, pFont, nFontSize, pCommandHandler, nParam1, nParam2);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTGUIListItemCtrl::OnEnter
//
//	PURPOSE:	Send the select command (RETAIL: 0x10049200)
//
// ----------------------------------------------------------------------- //

LTBOOL CLTGUIListItemCtrl::OnEnter()
{
	if (m_pCommandHandler && m_pCommandHandler->SendCommand(m_nSelectCommandID, m_nParam1, m_nParam2))
		return LTTRUE;
	return LTFALSE;
}
