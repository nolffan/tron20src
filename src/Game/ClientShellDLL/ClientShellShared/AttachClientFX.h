// ----------------------------------------------------------------------- //
//
// MODULE  : AttachClientFX.h
//
// PURPOSE : The client fx a model's bute record lists, played on an
//           object and shown, hidden or restarted by its model keys and
//           server messages
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x101ad060..
// 0x101ad740): NOLF2's CCharacterFX attach fx code made a class of its
// own, which CCharacterFX, CClientFxUserFX and others hold. The names are
// ours.
//
// ----------------------------------------------------------------------- //

#ifndef __ATTACHCLIENTFX_H__
#define __ATTACHCLIENTFX_H__

#include "ClientFXMgr.h"
#include "ModelButeMgr.h"

class CParsedMsg;

class CAttachClientFX
{
public:
	CAttachClientFX();
	~CAttachClientFX();

	// plays the model's fx on hObj (once: a second object is ignored)
	void	Init(HOBJECT hObj, ModelId eModelId);
	void	SetModel(ModelId eModelId);

	void	ShowAll();
	void	HideAll();

	// "SHOW_ATTACHFX [n]", "HIDE_ATTACHFX [n]", "RESET_ATTACHFX [n]"
	void	OnModelKey(HLOCALOBJ hObj, ArgList *pArgs);
	LTBOOL	OnServerMessage(ILTMessage_Read *pMsg);

private:
	// the link of fx n; -1 is the spare link
	CLIENTFX_LINK*	GetLink(const char* szIndex);
	CLIENTFX_LINK*	GetLink(int nIndex)
	{
		if (nIndex == -1) return &m_Link;
		if (!m_pAttachFX) return LTNULL;
		return m_pAttachFX->GetElement(nIndex);
	}

	void	HandleMsg(const CParsedMsg &cMsg);

	void	CreateAll();
	void	CreateFX(int nIndex, CLIENTFX_LINK* pLink);
	void	ShutdownAll();

	CLIENTFX_LINK		m_Link;			// +0x0
	CLIENTFX_LINK_NODE*	m_pAttachFX;	// +0x4
	HOBJECT				m_hObject;		// +0x8
	ModelId				m_eModelId;		// +0xc: eModelIdInvalid when none
};

#endif // __ATTACHCLIENTFX_H__
