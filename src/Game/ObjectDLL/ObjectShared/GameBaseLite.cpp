// ----------------------------------------------------------------------- //
//
// MODULE  : GameBaseLite.cpp
//
// PURPOSE : "Lite" game base object class definition  (For game objects without an HOBJECT)
//
// CREATED : 7/12/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GameBaseLite.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "VersionMgr.h"

LINKFROM_MODULE( GameBaseLite );

// RETAIL: Tron's ClassDef (0x102fd3f8) has no properties: the GameType
// group came to NOLF2 later
BEGIN_CLASS(GameBaseLite)
END_CLASS_DEFAULT_FLAGS(GameBaseLite, BaseClass, NULL, NULL, CF_HIDDEN | CF_CLASSONLY)

GameBaseLite::GameBaseLite(bool bStartActive) :
	BaseClass(),
	m_bActive(bStartActive),
	m_hClass(0),
	m_nSerializeID(0),
	m_bRegistered(false)
{
	// Clear out the invalid data from BaseClass
	m_hObject = 0;
	m_nType = OT_NORMAL;

	// RETAIL: 0x10153390 (Tron's, as GameBase's)
	m_nSaveVersion = g_pVersionMgr->GetSaveVersion();
	m_hstrSave = LTNULL;
}

GameBaseLite::~GameBaseLite()
{
	ASSERT(!g_pGameServerShell->GetLiteObjectMgr()->HasObject(this));

	FREE_HSTRING(m_hstrSave);	// RETAIL: 0x10153470
}

uint32 GameBaseLite::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
    if (!g_pLTServer) return 0;

	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();
	switch(messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)pMsg->Readuint32();
			TriggerMsg(hSender, szMsg);
		}
		break;

		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, pMsg);
}

// RETAIL: 0x101535a0/0x101535e0: Tron saves GameBase's way (no version
// manager calls, no 2001 save-file branch)
void GameBaseLite::Save(ILTMessage_Write *pMsg)
{
	SAVE_bool(m_bActive);
	SAVE_DWORD(m_nSaveVersion);
	SAVE_HSTRING(m_hstrSave);
}

void GameBaseLite::Load(ILTMessage_Read *pMsg)
{
	LOAD_bool(m_bActive);
	LOAD_DWORD(m_nSaveVersion);
	LOAD_HSTRING(m_hstrSave);
}

void GameBaseLite::InitialUpdate(float fUpdateType)
{
	// If someone's telling us to update, we're obviously registered.  (Needed for proper load behavior.)
	m_bRegistered = true;
}

void GameBaseLite::Activate()
{
	if (m_bActive)
		return;

	m_bActive = true;

	// Tell the ObjectLiteMgr to activate us
	g_pGameServerShell->GetLiteObjectMgr()->ActivateObject(this);
}

void GameBaseLite::Deactivate()
{
	if (!m_bActive)
		return;

	m_bActive = false;
	
	// Tell the ObjectLiteMgr to de-activate us
	g_pGameServerShell->GetLiteObjectMgr()->DeactivateObject(this);
}

void GameBaseLite::SetName(const char *pName)
{
	// RETAIL: 0x10153670 asks the virtual IsRegistered()
	if (IsRegistered())
	{
		m_bRegistered = false;
		g_pGameServerShell->GetLiteObjectMgr()->RenameObject(this, pName);
		m_bRegistered = true;
		return;
	}
	m_sName = pName;
}

bool GameBaseLite::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Remove("REMOVE");

	// RETAIL: 0x10153720: Tron only answers REMOVE while registered, and
	// does not remove the object here (no CLiteObjectMgr call)
	if (IsRegistered() && (cMsg.GetArg(0) == s_cTok_Remove))
	{
#ifdef TRON20SRC_MODERN
		// RETAIL BUG: (docs/RETAIL_BUGS.md) REMOVE is accepted but the
		// object stays; remove it as NOLF2 does
		g_pGameServerShell->GetLiteObjectMgr()->RemoveObject(this);
#endif
		return true;
	}

	return false;
}

void GameBaseLite::TriggerMsg(HOBJECT hSender, const char* pMsg)
{
	if (!pMsg) return;

	// RETAIL: 0x101537c0
	if (!IsRegistered()) return;

	ConParse parse;
	parse.Init(pMsg);

	while (g_pCommonLT->Parse(&parse) == LT_OK)
	{
		// Don't parse empty messages
		if (!parse.m_nArgs || !parse.m_Args[0])
			continue;

		CParsedMsg cCurMsg(parse.m_nArgs, parse.m_Args);
		OnTrigger(hSender, cCurMsg);
	}
}

uint32 GameBaseLite::OnPrecreate(ObjectCreateStruct* pOCS, float precreateType)
{
	if (!BaseClass::OnPrecreate(pOCS, precreateType))
		return 0;

	// Save the class
	SetClass(pOCS->m_hClass);

	// Save the name
	SetName(pOCS->m_Name);
	ASSERT(*GetName() && "Creating lite object without a name!");

	// Read our properties
	if (!ReadProp(pOCS))
		return 0;

	// Add ourselves to the ObjectLiteMgr
	g_pGameServerShell->GetLiteObjectMgr()->AddObject(this);
	m_bRegistered = true;

	return 1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBaseLite::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

// RETAIL: Tron's is empty (slot 30 is the shared "return true" at
// 0x100dd4f0); NOLF2's game type filter is not in it
bool GameBaseLite::ReadProp( ObjectCreateStruct* pStruct )
{
	return true;
}
