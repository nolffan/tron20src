// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIListItemCtrl.h
//
// PURPOSE : A text control that sends a command of its own when chosen
//
// ----------------------------------------------------------------------- //

#ifndef __LTGUILISTITEMCTRL_H__
#define __LTGUILISTITEMCTRL_H__

// RETAIL: Tron's (RTTI .?AVCLTGUIListItemCtrl@@, vtable 0x101e7338; the
// file and member names are ours): CShell.dll 0x10048ef0..0x1004922b,
// after the light cycle game data manager's TU. The host and light cycle
// screens list their saved sets with it (0x100048ba, 0x10009f0a).

#include "ltguimgr.h"

class CLTGUIListItemCtrl : public CLTGUITextCtrl
{
public:
	CLTGUIListItemCtrl();							// RETAIL: 0x10048ef0

	// as CLTGUITextCtrl's, with the command sent when the item is chosen.
	// RETAIL: 0x10049110
	LTBOOL	Create(std::wstring sText, uint32 nCommandID, uint32 nSelectCommandID, uint32 nHelpID,
				CUIFont *pFont, uint8 nFontSize, CLTGUICommandHandler *pCommandHandler,
				uint32 nParam1 = 0, uint32 nParam2 = 0);

	// sends the select command. RETAIL: 0x10049200 (slot 19)
	virtual LTBOOL	OnEnter();

	uint32	GetSelectCommandID() { return m_nSelectCommandID; }

protected:
	uint32	m_nSelectCommandID;						// RETAIL: +0x68
};

#endif // __LTGUILISTITEMCTRL_H__
