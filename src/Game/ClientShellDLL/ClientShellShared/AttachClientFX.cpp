// ----------------------------------------------------------------------- //
//
// MODULE  : AttachClientFX.cpp
//
// PURPOSE : The client fx a model's bute record lists, played on an object
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x101ad060..
// 0x101ad740; AttachClientFX.h). The names are ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AttachClientFX.h"
#include "ParsedMsg.h"

#define KEY_SHOW_ATTACHFX		"SHOW_ATTACHFX"		// SHOW_ATTACHFX [n]
#define KEY_HIDE_ATTACHFX		"HIDE_ATTACHFX"		// HIDE_ATTACHFX [n]
#define KEY_FX					"FX"
#define KEY_RESET_ATTACHFX		"RESET_ATTACHFX"	// RESET_ATTACHFX [n]

// RETAIL: 0x101ad060
CAttachClientFX::CAttachClientFX()
{
	m_pAttachFX = LTNULL;
	m_hObject = LTNULL;
	m_eModelId = eModelIdInvalid;
}

// RETAIL: 0x101ad080
void CAttachClientFX::Init(HOBJECT hObj, ModelId eModelId)
{
	if (m_hObject || !hObj) return;

	m_hObject = hObj;
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Client, CF_NOTIFYMODELKEYS, CF_NOTIFYMODELKEYS);

	SetModel(eModelId);
}

// RETAIL: 0x101ad0c0
void CAttachClientFX::SetModel(ModelId eModelId)
{
	if (eModelId == m_eModelId) return;

	ShutdownAll();
	m_eModelId = eModelId;
	CreateAll();
}

// RETAIL: 0x101ad0f0
CAttachClientFX::~CAttachClientFX()
{
	ShutdownAll();

	CLIENTFX_LINK_NODE* pNode = m_pAttachFX;
	m_pAttachFX = LTNULL;
	debug_delete(pNode);
}

// RETAIL: 0x101ad170
void CAttachClientFX::ShowAll()
{
	CLIENTFX_LINK_NODE* pNode = m_pAttachFX;
	while (pNode)
	{
		if (!pNode->m_Link.IsValid()) return;
		pNode->m_Link.GetInstance()->Show();
		pNode = pNode->m_pNext;
	}
}

// RETAIL: 0x101ad190
void CAttachClientFX::OnModelKey(HLOCALOBJ hObj, ArgList *pArgs)
{
	if (!hObj || !pArgs || !pArgs->argv || !pArgs->argc) return;

	CParsedMsg cMsg(pArgs->argc, pArgs->argv);
	HandleMsg(cMsg);
}

// RETAIL: 0x101ad1e0
LTBOOL CAttachClientFX::OnServerMessage(ILTMessage_Read *pMsg)
{
	switch (pMsg->Readuint8())
	{
		case 2:
		{
			char szMsg[80];
			pMsg->ReadString(szMsg, sizeof(szMsg));

			ConParse cParse(szMsg);
			while (g_pCommonLT->Parse(&cParse) == LT_OK)
			{
				if (cParse.m_nArgs > 0)
				{
					CParsedMsg cMsg(cParse.m_nArgs, cParse.m_Args);
					HandleMsg(cMsg);
				}
			}
		}
		break;

		default:
			return LTFALSE;
	}

	return LTTRUE;
}

// RETAIL: 0x101ad2a0
CLIENTFX_LINK* CAttachClientFX::GetLink(const char* szIndex)
{
	if (!szIndex) return LTNULL;
	return GetLink(atoi(szIndex));
}

// RETAIL: 0x101ad2e0
void CAttachClientFX::HandleMsg(const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_ShowAttachFX(KEY_SHOW_ATTACHFX);
	static CParsedMsg::CToken s_cTok_HideAttachFX(KEY_HIDE_ATTACHFX);
	static CParsedMsg::CToken s_cTok_FX(KEY_FX);
	static CParsedMsg::CToken s_cTok_ResetAttachFX(KEY_RESET_ATTACHFX);

	if (cMsg.GetArg(0) == s_cTok_ShowAttachFX)
	{
		if (cMsg.GetArgCount() > 1)
		{
			CLIENTFX_LINK* pLink = GetLink(cMsg.GetArg(1));
			if (pLink && pLink->IsValid())
			{
				pLink->GetInstance()->Show();
			}
		}
		else
		{
			ShowAll();
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_HideAttachFX)
	{
		if (cMsg.GetArgCount() > 1)
		{
			CLIENTFX_LINK* pLink = GetLink(cMsg.GetArg(1));
			if (pLink && pLink->IsValid())
			{
				pLink->GetInstance()->Hide();
			}
		}
		else
		{
			HideAll();
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_ResetAttachFX)
	{
		if (cMsg.GetArgCount() > 1)
		{
			int nIndex = atoi(cMsg.GetArg(1));
			CreateFX(nIndex, GetLink(nIndex));
		}
		else
		{
			CreateAll();
		}
	}
}

// RETAIL: 0x101ad580
void CAttachClientFX::HideAll()
{
	CLIENTFX_LINK_NODE* pNode = m_pAttachFX;
	while (pNode)
	{
		if (!pNode->m_Link.IsValid()) return;
		pNode->m_Link.GetInstance()->Hide();
		pNode = pNode->m_pNext;
	}
}

// RETAIL: 0x101ad5a0
void CAttachClientFX::CreateAll()
{
	ShutdownAll();
	if (m_eModelId == eModelIdInvalid) return;

	int nNumFX = g_pModelButeMgr->GetNumClientFX(m_eModelId);
	for (int i = 0; i < nNumFX; i++)
	{
		CLIENTFX_LINK_NODE* pNode = debug_new(CLIENTFX_LINK_NODE);
		CreateFX(i, &pNode->m_Link);

		if (m_pAttachFX)
			m_pAttachFX->AddToEnd(pNode);
		else
			m_pAttachFX = pNode;
	}
}

// RETAIL: 0x101ad600
void CAttachClientFX::CreateFX(int nIndex, CLIENTFX_LINK* pLink)
{
	if (!pLink) return;

	if (pLink->IsValid())
	{
		g_pClientFXMgr->ShutdownClientFX(pLink);
	}

	const char* pFX = g_pModelButeMgr->GetClientFX(m_eModelId, nIndex);
	if (!pFX || !pFX[0]) return;

	CLIENTFX_CREATESTRUCT fxInit(pFX, FXFLAG_LOOP, m_hObject);
	g_pClientFXMgr->CreateClientFX(pLink, fxInit);

	// they start hidden
	if (pLink->IsValid())
	{
		pLink->GetInstance()->Hide();
	}
}

// RETAIL: 0x101ad6f0
void CAttachClientFX::ShutdownAll()
{
	CLIENTFX_LINK_NODE* pNode = m_pAttachFX;
	while (pNode)
	{
		if (pNode->m_Link.IsValid())
		{
			g_pClientFXMgr->ShutdownClientFX(&pNode->m_Link);
		}
		pNode = pNode->m_pNext;
	}

	pNode = m_pAttachFX;
	m_pAttachFX = LTNULL;
	debug_delete(pNode);
}
