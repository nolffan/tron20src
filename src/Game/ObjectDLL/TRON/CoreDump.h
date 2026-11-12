/****************************************************************************
;
;	MODULE:			CoreDump.h
;
;	PURPOSE:		Core Dump object class for TRON
;
;	HISTORY:		03/22/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2001, Monolith Productions, Inc.
;
****************************************************************************/

// RETAIL: retail's core dump is not Build 69's (that drained energy and ran
// its inventory's commands): it holds up to three archive items a player
// can take through the decision window, plus energy and health, under
// ribbon and tetra fx. Written from retail Object.lto (0x10017dd0..
// 0x1001a080; vtable 0x1029382c, 0x13c bytes). Member and function names
// not in Build 69 are ours.

#ifndef _COREDUMP_H_
#define _COREDUMP_H_

#include "GameBase.h"
#include "ArchiveItem.h"
#include "Timer.h"
#include "ActivateTypeMgr.h"

#define CD_MAX_ITEMS	3

class CTronPlayerObj;

class CoreDump : public GameBase
{
	public :

		CoreDump();
		~CoreDump();

		// RETAIL: inline (CTronGameServerShell::DropInventoryObject,
		// retail 0x1005f20d)
		void Init(float fEnergy, float fHealth)
		{
			m_bActive = false;
			m_fHealth = m_fTotalHealth = fHealth;
			m_fEnergy = m_fTotalEnergy = fEnergy;
		}

		void SetItems(ArchiveItem* aItems);

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);
		virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		int GetNumItems();
		void CreateTetras();
		HOBJECT CreateFX(const char* pszFXName, bool bSmoothShutdown);
		void HandleActivate(HOBJECT hSender);
		void HandleStopActivate();
		LTBOOL Update();
		void Term();
		void ShowContents();
		void HandleChoice(uint8 nChoice);

		// Energy and health the player gets, decaying
		float m_fTotalEnergy;
		float m_fEnergy;
		float m_fTotalHealth;
		float m_fHealth;

		uint8 m_bShown;
		bool m_bActive;
		CTronPlayerObj* m_pPlayer;

		ArchiveItem m_aItems[CD_MAX_ITEMS];
		int m_aeShownCondition[CD_MAX_ITEMS];	// What a virus scan may show of each condition
		uint32 m_nNumItems;

		CTimer m_RibbonTimer;
		CTimer m_LifeTimer;

		LTObjRef m_hRibbons;
		LTObjRef m_hTetras;

		CActivateTypeHandler m_ActivateTypeHandler;
};

#endif // _COREDUMP_H_
