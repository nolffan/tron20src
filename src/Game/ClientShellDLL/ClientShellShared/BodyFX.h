// ----------------------------------------------------------------------- //
//
// MODULE  : BodyFX.h
//
// PURPOSE : Body special fx class - Definition
//
// CREATED : 02.01.2000
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BODY_FX_H__
#define __BODY_FX_H__

#include "SpecialFX.h"
#include "ModelButeMgr.h"
#include "SharedFXStructs.h"
#include "Animator.h"
#include "ClientFXMgr.h"
#include "HitBox.h"

class CRagDoll;

// RETAIL: Tron's CBodyFX is 0x8c bytes (constructor CShell.dll 0x1012da60,
// CBankedList<CBodyFX> 0x10115660). It is between Build 51's and NOLF2's:
// it has NOLF2's rag doll pointer, one ClientFX link and the client hit box,
// but no backpack (the low violence and multiplayer body swap), no hide keys
// and no creation time.

class CBodyFX : public CSpecialFX
{
	public :

		// Ctors/Dtors/etc

		CBodyFX();
		~CBodyFX();

        LTBOOL Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);

        LTBOOL CreateObject(ILTClient* pClientDE);

		// Updates

        LTBOOL	Update();

		// Handlers

        LTBOOL OnServerMessage(ILTMessage_Read *pMsg);
		void OnModelKey(HLOCALOBJ hObj, ArgList *pArgs);

		uint8 GetClientId() const { return m_bs.nClientId; }
		
		virtual uint32 GetSFXID() { return SFX_BODY_ID; }

		HOBJECT	GetHitBox() const { return m_HitBox.GetObject(); }

		bool	CanBeCarried() const { return m_bs.bCanBeCarried; }
		bool	CanBeRevived() const { return m_bs.bCanBeRevived; }
		
		ModelSkeleton GetModelSkeleton() const { return g_pModelButeMgr->GetModelSkeleton(m_bs.eModelId); }

	protected :

		// Updates

		void UpdateFade();

		bool SetupRagDoll();

		bool CreateFX(char* pFXName);

		void CreateDamageFX(DamageType eType);

	protected :

		CRagDoll*			m_pRagDoll;				// +0x38

		BODYCREATESTRUCT	m_bs;					// +0x3c Our createstruct

		LTFLOAT				m_fFaderTime;			// +0x54
		LTFLOAT				m_fFaderTimer;			// +0x58

		// PORT: Build 51's name for the member at +0x5c. Retail's code does not
		// use it, and the constructor does not set it.
		ANIMTRACKERID		m_TwitchTracker;

		// keyframed ClientFX
		CLIENTFX_LINK		m_fx;					// +0x60

		CHitBox				m_HitBox;				// +0x64
};

#endif