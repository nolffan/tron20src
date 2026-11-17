// ----------------------------------------------------------------------- //
//
// MODULE  : HUDMgr.h
//
// PURPOSE : Definition of CHUDMgr class
//
// CREATED : 07/17/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDMGR_H
#define __HUDMGR_H

#include "HUDItem.h"
#include "HUDMessageQueue.h"
#include "HUDChatInput.h"
#include "HUDTransmission.h"
#include "HUDPaused.h"
#include "HUDMissionText.h"
#include "HUDSubtitles.h"
#include "HUDDamage.h"
#include "HUDDecision.h"
#include "HUDPopup.h"
#include "HUDDisplayMeter.h"
// RETAIL: Tron's CHUDScores is CTronHUDMgr's (TRON/HUDScores.h)
class CHUDScores;

enum eHUDUpdateFlag
{
	kHUDNone		= 0x00000000,
	kHUDFrame		= 0x00000001,
	kHUDHealth		= 0x00000002,
	kHUDArmor		= 0x00000004,
	kHUDDamage		= 0x00000008,
	kHUDAmmo		= 0x00000010,
	kHUDAir			= 0x00000020,	//TO2
	kHUDHiding		= 0x00000040,	//TO2
	kHUDWeapons		= 0x00000080,	//TRON
	kHUDPermissions	= 0x00000100,	//TRON
	kHUDVersion		= 0x00000200,	//TRON
	kHUDEnergy		= 0x00000400,	//TRON
	kHUDCarry		= 0x00000800,   //TO2
	kHUDChooser		= 0x00001000,	//TO2
	kHUDEnergyTrans = 0x00002000,	//TRON
	kHUDObjectives	= 0x00004000,	//both
	kHUDProcedurals = 0x00008000,	//TRON
	kHUDDistance	= 0x00010000,
	kHUDProgressBar	= 0x00020000,	//TO2
	kHUDDisplayMeter= 0x00400000,	// RETAIL: Tron's value (CHUDDisplayMeter ctor, 0x10187250)
	kHUDRespawn     = 0x00080000,
	kHUDScores		= 0x00100000,
	kHUDDoomsday	= 0x00200000,
	// RETAIL: Tron's; the client info manager (0x100fcd00) queues both
	// when the score list changes. The names are ours.
	kHUDTronUnknown01000000	= 0x01000000,
	kHUDTronUnknown08000000	= 0x08000000,
	kHUDAll			= 0xFFFFFFFF,
};


//******************************************************************************************
//** HUD Manager
//******************************************************************************************
class CHUDMgr
{
public:

	CHUDMgr();
	~CHUDMgr();

    virtual	LTBOOL		Init();
    virtual	void		Term();

	// RETAIL: Tron's (retail CHUDMgr's vtable, slots 2 and 3): the items are
	// added for the game type being played (CHUDItem::IsUsedInGameType) and
	// initialized; and terminated and taken away again. The names are ours
	virtual bool		AddItems();
	virtual bool		RemoveItems();

    virtual	void        Render();
    virtual	void        Update();

	virtual	void		QueueUpdate(uint32 nUpdateFlag);
	virtual uint32		QueryUpdateFlags() {return m_UpdateFlags;}

	virtual	void		ScreenDimsChanged();

	virtual	void		NextLayout();
	virtual	void		PrevLayout();
	virtual	void		UpdateLayout();

	virtual	void		Show(LTBOOL bShow)	{m_bVisible = bShow;}
	virtual	LTBOOL		IsShown()			{return m_bVisible;}

	virtual void			SetRenderLevel(eHUDRenderLevel eLevel) {m_eLevel = eLevel;}
	virtual eHUDRenderLevel	GetRenderLevel() {return m_eLevel;}

	// RETAIL: Tron's last slot (16), empty here; CTronHUDMgr's adds more of its
	// items for the game type. Not yet identified
	virtual void			TronUnknownVirtual40() {}


protected:

	uint8		m_nCurrentLayout;
	LTBOOL		m_bVisible;

	uint32			m_UpdateFlags;
	eHUDRenderLevel m_eLevel;

	// RETAIL: Tron's layout (constructor 0x100f34f0, destructor 0x100f3910):
	// the three message queues are made on the heap, and there is no radar.
	CHUDDamage			m_Damage;				// +0x14
	CHUDChatMsgQueue*	m_pChatMsgs;			// +0x3c
	CHUDPickupMsgQueue*	m_pPickupMsgs;			// +0x40
	CHUDTransmission	m_Transmission;			// +0x44
	CHUDChatInput		m_ChatInput;			// +0x188
	CHUDMissionText		m_MissionText;			// +0x9b8
	CHUDSubtitles		m_Subtitles;			// +0xb84
	CHUDDecision		m_Decision;				// +0xc14
	CHUDPopup			m_Popup;				// +0x2b80
	CHUDRewardMsgQueue*	m_pRewardMsgs;			// +0x312c
	CHUDPaused			m_Paused;				// +0x3130
	CHUDDisplayMeter	m_DisplayMeter;			// +0x3278

	//items
	typedef std::vector<CHUDItem *> ItemArray;
	ItemArray m_itemArray;			// Pointer to each screen

	// RETAIL: inlined in Tron's AddItems, once an item
	void		AddItem(CHUDItem* pItem, GameType eGameType)
	{
		if (pItem->IsUsedInGameType(eGameType))
			m_itemArray.push_back(pItem);
	}

};

extern CHUDMgr*				g_pHUDMgr;
extern CHUDChatMsgQueue*	g_pChatMsgs;
extern CHUDPickupMsgQueue*	g_pPickupMsgs;
extern CHUDChatInput*		g_pChatInput;
extern CHUDTransmission*	g_pTransmission;
extern CHUDMissionText*		g_pMissionText;
extern CHUDSubtitles*		g_pSubtitles;
extern CHUDDecision*		g_pDecision;
extern CHUDPopup*			g_pPopup;
extern CHUDRewardMsgQueue*	g_pRewardMsgs;
extern CHUDPaused*			g_pPaused;
extern CHUDDisplayMeter*	g_pDisplayMeter;
extern CHUDScores*			g_pScores;

#endif
