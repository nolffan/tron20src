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
		virtual uint32 TronUnknownVirtual68(void* pArg);

        LTFLOAT			m_fDamageRadius;
        LTFLOAT         m_fMaxDamage;
		DamageType		m_eDamageType;
		LTObjRef		m_hFiredFrom;
        LTVector        m_vPos;
        LTBOOL          m_bRemoveWhenDone;
        uint8           m_nImpactFXId;

        LTFLOAT         m_fProgDamage;
        LTFLOAT         m_fProgDamageLifetime;
        LTFLOAT         m_fProgDamageDuration;
        LTFLOAT         m_fProgDamageRadius;
		DamageType		m_eProgDamageType;

		CTimer			m_ProgDamageTimer;

	private :

		void ReadProp();
		void AreaDamageObjectsInSphere();
		void AreaDamageObject(HOBJECT hObj);
		void ProgDamageObjectsInSphere();
		void ProgDamageObject(HOBJECT hObj);

		// RETAIL: virtual in Tron, in this order, after slot 0x68 (its vtable;
		// Mine's are overrides)
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