// ----------------------------------------------------------------------- //
//
// MODULE  : Explosion.h
//
// PURPOSE : Explosion - Definition
//
// CREATED : 11/25/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __EXPLOSION_H__
#define __EXPLOSION_H__

#include "GameBase.h"
#include "DamageTypes.h"
#include "FXButeMgr.h"
#include "Timer.h"
#include "../TRON/ContinuousFireHelper.h"

LINKTO_MODULE( Explosion );

class Explosion : public GameBase
{
	public :

 		Explosion();
		// RETAIL: Tron's is declared: a derived class's (OptimizerExplosion,
		// retail 0x10030300) sets its vtable
		virtual ~Explosion() {}
        void Setup(HOBJECT hFiredFrom, uint8 nAmmoType);

		HOBJECT GetFiredFrom() const { return m_hFiredFrom; }

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

        virtual LTVector GetBoundingBoxColor();

        virtual void Start(HOBJECT hFiredFrom=LTNULL);

		// RETAIL: Tron's slot 0x68 (retail 0x100ef230: 1 when given nothing,
		// else the explosion's position is read into m_vPos). Not yet named
		virtual LTRESULT TronUnknownVirtual68(LTVector* pvPos);

        LTFLOAT			m_fDamageRadius;
        LTFLOAT         m_fMaxDamage;
		DamageType		m_eDamageType;
		LTObjRef		m_hFiredFrom;
        LTVector        m_vPos;
        // RETAIL: a bool in Tron (+0x60; Explosion::Explosion, Object.lto 0x100eeec6)
        bool            m_bRemoveWhenDone;
        uint8           m_nImpactFXId;

        LTFLOAT         m_fProgDamage;
        LTFLOAT         m_fProgDamageLifetime;
        LTFLOAT         m_fProgDamageDuration;
        LTFLOAT         m_fProgDamageRadius;
		DamageType		m_eProgDamageType;

		CTimer			m_ProgDamageTimer;

		// RETAIL: Tron's members (constructor 0x100eeec6; Save 0x100f0870).
		// The names are ours.
		bool			m_bAreaDamageIgnoreOwner;	// 0x88 AMMO::bAreaDamageIgnoreOwner
		bool			m_bDamageForever;		// 0x89 "DamageForever"
		bool			m_bStartOn;				// 0x8a "StartOn"
		WEAPON const*	m_pWeaponData;			// 0x8c
		AMMO const*		m_pAmmoData;			// 0x90
		ContinuousFireData	m_FireData;			// 0x94 (sent with MID 0x3f3)

		// RETAIL: helpers of the fire data (0x100ef210..0x100ef450)
		void ClearFireData();								// 0x100ef210
		void ReadFireData(ILTMessage_Read *pMsg);			// 0x100ef260
		void ScaleDamageField8(LTFLOAT *pfDamage);			// 0x100ef3b0
		void ScaleDamageField10(LTFLOAT *pfDamage);		// 0x100ef3d0
		void ScaleDamage(LTFLOAT *pfDamage);				// 0x100ef400
		void DoFireDataDamage(HOBJECT hDamager, HOBJECT hObj, LTVector const& vDir);	// 0x100ef450
		void SetupFromAmmo(AMMO const *pAmmo);				// 0x100eef50

		virtual uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);	// slot 2

	private :

		void ReadProp();
		void AreaDamageObjectsInSphere();
		void AreaDamageObject(HOBJECT hObj);
		void ProgDamageObjectsInSphere();
		void ProgDamageObject(HOBJECT hObj);

	protected :

		// RETAIL: virtual in Tron, in this order, after slot 0x68 (its vtable;
		// Mine's and AttachedExplosion's are overrides)
        virtual void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        virtual void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		virtual void Update();
};

#ifndef __PSX2
class CExplosionPlugin : public IObjectPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
            uint32* pcStrings,
            const uint32 cMaxStrings,
            const uint32 cMaxStringLength);

	private:

		CFXButeMgrPlugin	m_FXButeMgrPlugin;
};
#endif

#endif // __EXPLOSION_H__