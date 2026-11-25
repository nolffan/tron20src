// ----------------------------------------------------------------------- //
//
// MODULE  : HUDMgr.cpp
//
// PURPOSE : Implementation of CHUDMgr class
//
// CREATED : 07/17/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "GameClientShell.h"


CHUDMgr*			g_pHUDMgr = LTNULL;
CHUDChatMsgQueue*	g_pChatMsgs = LTNULL;
CHUDPickupMsgQueue*	g_pPickupMsgs = LTNULL;
CHUDChatInput*		g_pChatInput = LTNULL;
CHUDTransmission*	g_pTransmission = LTNULL;
CHUDMissionText*	g_pMissionText = LTNULL;
CHUDSubtitles*		g_pSubtitles = LTNULL;
CHUDDecision*		g_pDecision = LTNULL;
CHUDPopup*			g_pPopup = LTNULL;
// RETAIL: Tron's HUD has no radar (retail CHUDMgr::Init 0x100f3b10, no
// CHUDRadar RTTI in CShell.dll): NOLF2's g_pRadar and its callers are gone
CHUDRewardMsgQueue*	g_pRewardMsgs = LTNULL;
CHUDPaused*			g_pPaused = LTNULL;
CHUDDisplayMeter*	g_pDisplayMeter = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDMgr::CHUDMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CHUDMgr::CHUDMgr()
{
	m_itemArray.reserve(30);
	m_eLevel = kHUDRenderNone;

	// RETAIL: Tron's (0x100f361d)
	m_pChatMsgs = debug_new(CHUDChatMsgQueue);
	m_pPickupMsgs = debug_new(CHUDPickupMsgQueue);
	m_pRewardMsgs = debug_new(CHUDRewardMsgQueue);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDMgr::~CHUDMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CHUDMgr::~CHUDMgr()
{
	Term();

	// RETAIL: Tron's (0x100f3944)
	if (m_pChatMsgs)
	{
		debug_delete(m_pChatMsgs);
		m_pChatMsgs = LTNULL;
	}
	if (m_pPickupMsgs)
	{
		debug_delete(m_pPickupMsgs);
		m_pPickupMsgs = LTNULL;
	}
	if (m_pRewardMsgs)
	{
		debug_delete(m_pRewardMsgs);
		m_pRewardMsgs = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CHUDMgr::Init()
{
	// RETAIL: Tron's (0x100f3b10): no radar, and the items are added by
	// AddItems when a game starts (CInterfaceMgr)
	g_pPopup = &m_Popup;
	g_pDecision = &m_Decision;
	g_pSubtitles = &m_Subtitles;
	g_pChatMsgs = m_pChatMsgs;
	g_pChatInput = &m_ChatInput;
	g_pMissionText = &m_MissionText;
	g_pTransmission = &m_Transmission;
	g_pPickupMsgs = m_pPickupMsgs;
	g_pRewardMsgs = m_pRewardMsgs;
	g_pPaused = &m_Paused;
	g_pDisplayMeter = &m_DisplayMeter;

	m_nCurrentLayout = GetConsoleInt("HUDLayout",0);

	g_pHUDMgr = this;

	m_bVisible = LTTRUE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDMgr::AddItems()
//
//	PURPOSE:	Add the items of the game type being played, and init them
//				(RETAIL: Tron's; retail 0x100f3c20)
//
// ----------------------------------------------------------------------- //

bool CHUDMgr::AddItems()
{
	GameType eGameType = g_pGameClientShell->GetGameType();

	if (m_pRewardMsgs->IsUsedInGameType(eGameType))
		m_itemArray.push_back(m_pRewardMsgs);
	if (m_Popup.IsUsedInGameType(eGameType))
		m_itemArray.push_back(&m_Popup);
	if (m_Decision.IsUsedInGameType(eGameType))
		m_itemArray.push_back(&m_Decision);
	if (m_Subtitles.IsUsedInGameType(eGameType))
		m_itemArray.push_back(&m_Subtitles);
	if (m_pChatMsgs->IsUsedInGameType(eGameType))
		m_itemArray.push_back(m_pChatMsgs);
	if (m_ChatInput.IsUsedInGameType(eGameType))
		m_itemArray.push_back(&m_ChatInput);
	if (m_MissionText.IsUsedInGameType(eGameType))
		m_itemArray.push_back(&m_MissionText);
	if (m_Transmission.IsUsedInGameType(eGameType))
		m_itemArray.push_back(&m_Transmission);
	if (m_pPickupMsgs->IsUsedInGameType(eGameType))
		m_itemArray.push_back(m_pPickupMsgs);
	if (m_Damage.IsUsedInGameType(eGameType))
		m_itemArray.push_back(&m_Damage);
	if (m_DisplayMeter.IsUsedInGameType(eGameType))
		m_itemArray.push_back(&m_DisplayMeter);

	// RETAIL: Tron's (0x100f3f53): the derived HUD adds its own items
	TronUnknownVirtual40();

	m_Damage.Init();
	m_pChatMsgs->Init();
	m_pPickupMsgs->Init();
	m_Transmission.Init();
	m_ChatInput.Init();
	m_MissionText.Init();
	m_Subtitles.Init();
	m_Decision.Init();
	m_Popup.Init();
	m_pRewardMsgs->Init();
	m_Paused.Init();
	m_DisplayMeter.Init();

	m_UpdateFlags = kHUDAll;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDMgr::RemoveItems()
//
//	PURPOSE:	Term the items and take them away (RETAIL: Tron's; retail
//				0x100f4000)
//
// ----------------------------------------------------------------------- //

bool CHUDMgr::RemoveItems()
{
	ItemArray::iterator iter = m_itemArray.begin();

	while (iter != m_itemArray.end())
	{
		(*iter)->Term();
		iter++;
	}

	m_itemArray.clear();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDMgr::Term()
//
//	PURPOSE:	Terminate the HUD
//
// ----------------------------------------------------------------------- //

void CHUDMgr::Term()
{
	ItemArray::iterator iter = m_itemArray.begin();

	while (iter != m_itemArray.end())
	{
		(*iter)->Term();
		iter++;
	}

	m_itemArray.clear();

	g_pHUDMgr = LTNULL;
}


void CHUDMgr::Update()
{

	int nLayout = GetConsoleInt("HUDLayout",0);
	if (nLayout != m_nCurrentLayout)
	{
		m_nCurrentLayout = nLayout;
		UpdateLayout();
	}
	m_UpdateFlags |= kHUDFrame;

	ItemArray::iterator iter = m_itemArray.begin();

	while (iter != m_itemArray.end())
	{
		if (m_UpdateFlags & (*iter)->GetUpdateFlags())
			(*iter)->Update();
		iter++;
	}

	m_UpdateFlags = kHUDNone;

}

void CHUDMgr::Render()
{
	if (!m_bVisible) return;

	ItemArray::iterator iter = m_itemArray.begin();

	while (iter != m_itemArray.end())
	{
		//we need to make sure that this hud item is up to date though
		if (m_UpdateFlags & (*iter)->GetUpdateFlags())
			(*iter)->Update();

		if ((*iter)->GetRenderLevel() <= m_eLevel)
			(*iter)->Render();
		iter++;
	}
	
}


void CHUDMgr::QueueUpdate(uint32 nUpdateFlag)
{
	m_UpdateFlags |= nUpdateFlag;
}

void CHUDMgr::ScreenDimsChanged()
{
#ifdef TRON20SRC_MODERN
	// MODERN.md CL-30: the items lay themselves out again for the new
	// screen. Most test only the scale, which two screens of the same
	// height share (1920x1080 and 2560x1080).
	UpdateLayout();
#endif
	m_UpdateFlags = kHUDAll;
}


void CHUDMgr::NextLayout()
{
	m_nCurrentLayout++;
	if (m_nCurrentLayout >= g_pLayoutMgr->GetNumHUDLayouts())
		m_nCurrentLayout = 0;
    WriteConsoleInt("HUDLayout",m_nCurrentLayout);
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	pProfile->m_nLayout = m_nCurrentLayout;
	pProfile->Save();
	UpdateLayout();
}

void CHUDMgr::PrevLayout()
{
	if (m_nCurrentLayout == 0)
		m_nCurrentLayout =  g_pLayoutMgr->GetNumHUDLayouts() - 1;
	else
		m_nCurrentLayout--;
    WriteConsoleInt("HUDLayout",m_nCurrentLayout);
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	pProfile->m_nLayout = m_nCurrentLayout;
	pProfile->Save();
	UpdateLayout();
}

void CHUDMgr::UpdateLayout()
{
	ItemArray::iterator iter = m_itemArray.begin();

	while (iter != m_itemArray.end())
	{
		(*iter)->UpdateLayout();
		iter++;
	}


	m_UpdateFlags = kHUDAll;

}




