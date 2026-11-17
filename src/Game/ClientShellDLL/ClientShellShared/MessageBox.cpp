// ----------------------------------------------------------------------- //
//
// MODULE  : MessageBox.cpp
//
// PURPOSE : Handle the display of a simple message box
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MessageBox.h"
#include "ScreenCommands.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"

extern CGameClientShell* g_pGameClientShell;

// KLS - Added new control type to allow for sounds when control is selected...
class CLTGUIAutoSoundTextCtrl : public CLTGUITextCtrl
{
  protected:

	// Override virtual function that is called when the state of the control
	// is changed.
	virtual void OnSelChange()
	{
		g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
	}
};

namespace
{
	// RETAIL: Tron's are wide (0x10270508, 0x102704f8, 0x102704e8,
	// 0x102704d8), filled by Init
	std::wstring szOK;
	std::wstring szCancel;
	std::wstring szYes;
	std::wstring szNo;
	const uint8 kIndent = 8;
	const uint16 kBaseWidth = 320;
	const uint16 kBaseHeight = 200;
	const uint16 kBigWidth = 480;
	const uint16 kMinWidth = 160;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMessageBox::CMessageBox()
{
    m_pCallback = LTNULL;
    m_bVisible  = LTFALSE;
    m_bGameWasPaused = LTFALSE;

	m_pText = LTNULL;
	m_pEdit = LTNULL;
	m_pOK = LTNULL;
	m_pCancel = LTNULL;

	m_pData = LTNULL;

}

CMessageBox::~CMessageBox()
{
	Term();
}

// RETAIL: 0x100ddf20. Tron's reads the dialog's fonts and colors from the
// layout's DialogBox tag.
LTBOOL CMessageBox::Init()
{
	uint8 nFont = (uint8)g_pLayoutMgr->GetInt("DialogBox", "DialogFontNum");
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);
	uint8 nFontSize = (uint8)g_pLayoutMgr->GetInt("DialogBox", "DialogFontSize");

	char szTmp[128];
	LoadString(IDS_OK, szTmp, sizeof(szTmp));
	LTGUIWiden(szOK, szTmp);
	LoadString(IDS_CANCEL, szTmp, sizeof(szTmp));
	LTGUIWiden(szCancel, szTmp);
	LoadString(IDS_YES, szTmp, sizeof(szTmp));
	LTGUIWiden(szYes, szTmp);
	LoadString(IDS_NO, szTmp, sizeof(szTmp));
	LTGUIWiden(szNo, szTmp);

	m_pOK = debug_new(CLTGUIAutoSoundTextCtrl);
    if (!m_pOK->Create(szOK, CMD_OK, LTNULL, pFont, nFontSize, this))
	{
		debug_delete(m_pOK);
		m_pOK = LTNULL;
        return LTFALSE;
	}

	LTVector vSel = g_pLayoutMgr->GetVector("DialogBox", "OKSelected");
	LTVector vNon = g_pLayoutMgr->GetVector("DialogBox", "OKNormal");
	LTVector vDis = g_pLayoutMgr->GetVector("DialogBox", "OKDisabled");
	uint32 selColor = SET_ARGB(0xFF, vSel.x, vSel.y, vSel.z);
	uint32 nonColor = SET_ARGB(0xFF, vNon.x, vNon.y, vNon.z);
	uint32 disColor = SET_ARGB(0xFF, vDis.x, vDis.y, vDis.z);
	m_pOK->SetColors(selColor, nonColor, disColor);

	m_pCancel = debug_new(CLTGUIAutoSoundTextCtrl);
    if (!m_pCancel->Create(szNo, CMD_CANCEL, LTNULL, pFont, nFontSize, this))
	{
		debug_delete(m_pCancel);
		m_pCancel = LTNULL;
        return LTFALSE;
	}

	vSel = g_pLayoutMgr->GetVector("DialogBox", "CancelSelected");
	vNon = g_pLayoutMgr->GetVector("DialogBox", "CancelNormal");
	vDis = g_pLayoutMgr->GetVector("DialogBox", "CancelDisabled");
	selColor = SET_ARGB(0xFF, vSel.x, vSel.y, vSel.z);
	nonColor = SET_ARGB(0xFF, vNon.x, vNon.y, vNon.z);
	disColor = SET_ARGB(0xFF, vDis.x, vDis.y, vDis.z);
	m_pCancel->SetColors(selColor, nonColor, disColor);

	m_pText = debug_new(CLTGUITextCtrl);
    if (!m_pText->Create(std::wstring(), LTNULL, LTNULL, pFont, nFontSize, this))
	{
		debug_delete(m_pText);
		m_pText = LTNULL;
        return LTFALSE;
	}

	vSel = g_pLayoutMgr->GetVector("DialogBox", "TextSelected");
	vNon = g_pLayoutMgr->GetVector("DialogBox", "TextNormal");
	vDis = g_pLayoutMgr->GetVector("DialogBox", "TextDisabled");
	selColor = SET_ARGB(0xFF, vSel.x, vSel.y, vSel.z);
	nonColor = SET_ARGB(0xFF, vNon.x, vNon.y, vNon.z);
	disColor = SET_ARGB(0xFF, vDis.x, vDis.y, vDis.z);
	m_pText->SetColors(selColor, nonColor, disColor);
	m_pText->SetFixedWidth(kBaseWidth-2*kIndent);
	m_pText->Enable(LTFALSE);

	m_pEdit = debug_new(CLTGUIEditCtrl);
    if (!m_pEdit->Create(g_pLTClient, CMD_OK, pFont, nFontSize, 60, this))
	{
		debug_delete(m_pEdit);
		m_pEdit = LTNULL;
        return LTFALSE;
	}

	vSel = g_pLayoutMgr->GetVector("DialogBox", "EditSelected");
	vNon = g_pLayoutMgr->GetVector("DialogBox", "EditNormal");
	vDis = g_pLayoutMgr->GetVector("DialogBox", "EditDisabled");
	m_pEdit->SetColors(SET_ARGB(0xFF, vSel.x, vSel.y, vSel.z),
					   SET_ARGB(0xFF, vNon.x, vNon.y, vNon.z),
					   SET_ARGB(0xFF, vDis.x, vDis.y, vDis.z));
	m_pEdit->EnableCaret(LTTRUE);
	m_pEdit->SetFixedWidth(kBaseWidth-2*kIndent,LTTRUE);

	char szBack[128] = "";
	g_pLayoutMgr->GetString("DialogBox", "DialogFrame", szBack, sizeof(szBack));

	m_Dlg.Create(g_pInterfaceResMgr->GetTexture(szBack),kBaseWidth,kBaseHeight);
	LTIntPt tmp(0,0);
	m_Dlg.AddControl(m_pText, tmp);
	m_Dlg.AddControl(m_pEdit, tmp);
	m_Dlg.AddControl(m_pOK, tmp);
	m_Dlg.AddControl(m_pCancel, tmp);

    return LTTRUE;
}

void CMessageBox::Term()
{
	m_Dlg.Destroy();
}

void CMessageBox::Show(int nStringID, MBCreate* pCreate, uint8 nFontSize, LTBOOL bDefaultReturn)
{
	if (m_bVisible || !pCreate)
		return;

	Show(LoadTempWString(nStringID),pCreate,nFontSize,bDefaultReturn);	// TRON: wide (RETAIL: 0x100de8d0)
}

// TRON: the text is wide, by value (RETAIL: 0x100de920); the font is the one
// for the size, set on the text and the edit control; a box without buttons
// (LTMB_NONE) has no room for them
void CMessageBox::Show(std::wstring sString, MBCreate* pCreate, uint8 nFontSize, LTBOOL bDefaultReturn)
{
	if (m_bVisible || !pCreate)
		return;

	m_eType = pCreate->eType;

	m_bGameWasPaused = g_pGameClientShell->IsGamePaused();
	if (!m_bGameWasPaused)
        g_pGameClientShell->PauseGame(LTTRUE,LTTRUE);

	uint8 nSize;
	if (nFontSize)
		nSize = nFontSize;
	else
		nSize = g_pLayoutMgr->GetDialogFontSize();

	//build everything at normal scale to make calculations easier
	m_Dlg.SetScale(1.0f);

	m_pText->SetFixedWidth(kBaseWidth-2*kIndent);
	m_pText->SetString(sString);
	CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(nSize);
	m_pText->SetFont(pFont,nSize);

	if (m_eType == LTMB_EDIT)
	{
		std::wstring sText;
		LTGUIWiden(sText, pCreate->pString);
		m_pEdit->SetText(sText);
		m_pEdit->SetInputMode(pCreate->eInput);
		m_pEdit->SetMaxLength(pCreate->nMaxChars);

		m_pEdit->Show(LTTRUE);
		m_pEdit->SetFont(pFont,nSize);
	}
	else
		m_pEdit->Show(LTFALSE);



	LTFLOAT fScale = g_pInterfaceResMgr->GetXRatio();
	// need to do this to get accurate sizes, since the width and height of the string 
	// do not scale precicely
	m_pText->SetScale(fScale);
	int32 fw,fh;
	m_pText->GetString()->GetDims(&fw,&fh);
	int nWidth = (int)(fw/fScale);
	int nHeight = (int)(fh/fScale);

	if (nHeight > kBaseWidth)
	{
		m_pText->SetFixedWidth(kBigWidth-2*kIndent);
		m_pText->GetString()->GetDims(&fw,&fh);
		nWidth = (int)(fw/fScale);
		nHeight = (int)(fh/fScale);
	}

	uint16 nDlgWidth = nWidth + 2*kIndent;
	if (nDlgWidth < kMinWidth)
		nDlgWidth = kMinWidth;
	uint16 nDlgHeight;
	if (m_eType == LTMB_NONE)
		nDlgHeight = nHeight + 3*kIndent;
	else
		nDlgHeight = nHeight + 3*kIndent + m_pOK->GetHeight();
	if (m_eType == LTMB_EDIT)
	{
		// need to do this to get accurate sizes, since the width and height of the string 
		// do not scale precicely
		m_pEdit->SetScale(fScale);
		nDlgHeight += (kIndent + m_pEdit->GetHeight());
		if (nDlgWidth < kBaseWidth)
			nDlgWidth = kBaseWidth;
		m_pEdit->SetFixedWidth(nDlgWidth-2*kIndent,LTTRUE);
	}


	LTIntPt offset;

	m_Dlg.SetSize(nDlgWidth,nDlgHeight);

	offset.x = (nDlgWidth - nWidth) / 2;
	offset.y = kIndent;
	m_Dlg.SetControlOffset(m_pText,offset);


	switch (m_eType)
	{
	case LTMB_YESNO:
		{
			m_pOK->SetString(szYes);
			offset.x = ((nDlgWidth / 2) - m_pOK->GetWidth()) - 2*kIndent;
			offset.y = (nDlgHeight - kIndent) - m_pOK->GetHeight();
			m_Dlg.SetControlOffset(m_pOK,offset);
			m_pOK->Show(LTTRUE);

			m_pCancel->SetString(szNo);
			offset.x = (nDlgWidth / 2) + 2*kIndent;
			offset.y = (nDlgHeight - kIndent) - m_pOK->GetHeight();
			m_Dlg.SetControlOffset(m_pCancel,offset);
			m_pCancel->Show(LTTRUE);
		}	break;
	case LTMB_OK:
		{
			m_pOK->SetString(szOK);
			offset.x = (nDlgWidth - m_pOK->GetWidth()) / 2;
			offset.y = (nDlgHeight - kIndent) - m_pOK->GetHeight();
			m_Dlg.SetControlOffset(m_pOK,offset);
			m_pOK->Show(LTTRUE);
			m_pCancel->Show(LTFALSE);
		} break;
	case LTMB_EDIT:
		{
			offset.x = kIndent;
			offset.y = 2* kIndent + m_pText->GetHeight();
			m_Dlg.SetControlOffset(m_pEdit,offset);

			m_pOK->SetString(szOK);
			offset.x = ((nDlgWidth / 2) - m_pOK->GetWidth()) - 2*kIndent;
			offset.y = (nDlgHeight - kIndent) - m_pOK->GetHeight();
			m_Dlg.SetControlOffset(m_pOK,offset);
			m_pOK->Show(LTTRUE);

			m_pCancel->SetString(szCancel);
			offset.x = (nDlgWidth / 2) + 2*kIndent;
			offset.y = (nDlgHeight - kIndent) - m_pOK->GetHeight();
			m_Dlg.SetControlOffset(m_pCancel,offset);
			m_pCancel->Show(LTTRUE);
		} break;
	case LTMB_NONE:
		{
			m_pOK->Show(LTFALSE);
			m_pCancel->Show(LTFALSE);
		} break;
	};


	m_pCallback = pCreate->pFn;
	m_pData = pCreate->pData;

	if (bDefaultReturn)
	{
		m_Dlg.SetSelection(m_Dlg.GetIndex(m_pOK));
	}
	else
	{
		m_Dlg.SetSelection(m_Dlg.GetIndex(m_pCancel));
	}

	offset.x =  (640 - nDlgWidth) / 2;
	offset.y = (480 - nDlgHeight) / 2;
	m_Dlg.SetBasePos(offset);
	m_Dlg.SetScale(g_pInterfaceResMgr->GetXRatio());

	m_Dlg.Show(LTTRUE);
    m_bVisible = LTTRUE;

    // Set the cursor to the appropriate state
	g_pInterfaceMgr->UpdateCursorState();
}


void CMessageBox::Close(LTBOOL bReturn)
{
	if (!m_bVisible)
		return;
    m_bVisible = LTFALSE;
	m_Dlg.Show(LTFALSE);

	// TRON: an edit box gives its callback the wide text (RETAIL: 0x100df2e0)
	std::wstring sText;
	if (m_eType == LTMB_EDIT)
	{
		sText = m_pEdit->GetText();
		m_pData = (void*)sText.c_str();
	}


	if (!m_bGameWasPaused)
        g_pGameClientShell->PauseGame(LTFALSE);

	if (m_pCallback)
	{
		m_pCallback(bReturn, m_pData);
	}

	// Play sound...
	g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);

	// Set the cursor to the appropriate state
	g_pInterfaceMgr->UpdateCursorState();
}

void CMessageBox::Draw()
{
	if (m_bVisible)
	{
		m_Dlg.Render();
	}
}

LTBOOL CMessageBox::HandleChar(unsigned char c)
{
	if (!m_bVisible) return LTFALSE;
	if (m_eType == LTMB_EDIT)
		return m_pEdit->HandleChar(c);

	return LTFALSE;
}

LTBOOL CMessageBox::HandleKeyDown(int key, int rep)
{
	if (!m_bVisible) return LTFALSE;
	if (m_eType == LTMB_EDIT)
	{
		//special handling for right and left arrows so that they may cycle through controls
		// when the edit box is not selected
		if (m_Dlg.GetSelectedControl() != m_pEdit)
		{
			if (key == VK_LEFT)
				return m_Dlg.OnUp();
			if (key == VK_RIGHT)
				return m_Dlg.OnDown();
		}

		if (m_pEdit->HandleKeyDown(key,rep))
			return LTTRUE;
	}

    LTBOOL handled = LTFALSE;
	switch (key)
	{
	case VK_LEFT:
	case VK_UP:
		{
			handled = m_Dlg.OnUp();
			break;
		}
	case VK_RIGHT:
	case VK_DOWN:
		{
			handled = m_Dlg.OnDown();
			break;
		}
	case VK_RETURN:
		{
			handled = m_Dlg.OnEnter();
			break;
		}
	case VK_ESCAPE:
		{
			// TRON: a box without buttons does not close (RETAIL: 0x100df594)
			if (m_eType == LTMB_NONE)
				return LTFALSE;
			Close(LTFALSE);
			handled = LTTRUE;
			break;
		}
	default:
		{
			CLTGUICtrl* pCtrl = m_Dlg.GetSelectedControl();
			if (pCtrl)
				handled = pCtrl->HandleKeyDown(key,rep);
			else
				handled = LTFALSE;
			break;
		}
	}

	// Handled the key
	return handled;
}

LTBOOL CMessageBox::OnLButtonDown(int x, int y)
{
	if (m_bVisible)
	{
		return m_Dlg.OnLButtonDown( x, y);
	}
    return LTFALSE;
}

LTBOOL CMessageBox::OnLButtonUp(int x, int y)
{
	if (m_bVisible)
	{
		return m_Dlg.OnLButtonUp( x, y);
	}
    return LTFALSE;
}


LTBOOL CMessageBox::OnMouseMove(int x, int y)
{
	if (m_bVisible)
	{
		return m_Dlg.OnMouseMove(x,y);
	}
    return LTFALSE;
}

uint32 CMessageBox::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_OK:
        Close(LTTRUE);
        return LTTRUE;
		break;
	case CMD_CANCEL:
        Close(LTFALSE);
        return LTTRUE;
		break;
	default:
        return LTFALSE;
		break;
	}
}
