/****************************************************************************
;
;	MODULE:			PatchRoutine.h
;
;	PURPOSE:		Patch routine class for TRON
;
;	HISTORY:		02/12/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2001, Monolith Productions, Inc.
;
;	RETAIL:			Tron 2.0's, rewritten from retail Object.lto (class def
;					0x102dab78, size 0x78; vtable 0x102942e4): Build 69's
;					drained itself on the server; retail's client drains it
;					(messages 154..156). Names but Build 69's are ours.
;
****************************************************************************/

#ifndef _PATCHROUTINE_H_
#define _PATCHROUTINE_H_

#include "GameBase.h"

class CTronPlayerObj;

class PatchRoutine : public GameBase
{
	public :

		PatchRoutine();
		~PatchRoutine();

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);
		virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		void Reset() { m_nCurrentHealth = m_nTotalHealth; m_nCurrentEnergy = m_nTotalEnergy; }

	private :

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		void HandleActivate(HOBJECT hSender);
		void HandleStopActivate(bool bTellClient);
		void InitialUpdate();
		void Remove();
		void CreateTransferFX();
		void KillTransferFX();
        LTBOOL ReadProp(ObjectCreateStruct *pData);

		int m_nTotalHealth;
		int m_nTotalEnergy;
		int	m_nCurrentHealth;
		int m_nCurrentEnergy;
		LTFLOAT m_fDistAboveFloor;		// Moved to this far above the floor
		bool m_bActive;
		CTronPlayerObj* m_pPlayer;
		LTObjRef m_hIdleFX;
		LTObjRef m_hTransferFX;
		bool m_bFromWorld;				// Created from the world (not spawned)
		bool m_bInfinite;				// Never runs out (a total of -1)
};

#endif // _PATCHROUTINE_H_
