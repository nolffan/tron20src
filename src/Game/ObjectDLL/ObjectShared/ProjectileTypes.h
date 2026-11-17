// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileTypes.h
//
// PURPOSE : ProjectileTypes class - definition
//
// CREATED : 10/3/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILE_TYPES_H__
#define __PROJECTILE_TYPES_H__

#include "Projectile.h"
#include "Timer.h"


enum ContainerCode;
enum EnumAIStimulusID;

LINKTO_MODULE( ProjectileTypes );

extern CTList<class CGrenade*> g_lstGrenades;

class CGrenade : public CProjectile
{
	public :

		CGrenade();
		~CGrenade();

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual void HandleImpact(HOBJECT hObj);
		virtual void UpdateGrenade();
        virtual void ResetRotationVel(LTVector* pvNewVel=LTNULL, SURFACE* pSurf=LTNULL);
		virtual void RotateToRest();
		virtual void Detonate(HOBJECT hObj, CollisionInfo* pCollInfo = LTNULL);

		// RETAIL: not virtual in Tron (CGrenade's vtable 0x102981b8 has 51
		// slots, ShouldPlayBounceSound/GetBounceSound the last two)
        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		virtual LTBOOL ShouldPlayBounceSound(SURFACE* pSurface);
		virtual const char* GetBounceSound(SURFACE* pSurface);

        LTBOOL   m_bSpinGrenade;     // Should the grenade spin
        LTBOOL   m_bUpdating;        // Are we updating the grenade

		ContainerCode m_eContainerCode;
		SurfaceType	  m_eLastHitSurface;

        LTFLOAT  m_fPitch;
        LTFLOAT  m_fYaw;
        LTFLOAT  m_fRoll;
        LTFLOAT  m_fPitchVel;
        LTFLOAT  m_fYawVel;
        LTFLOAT  m_fRollVel;
		LTBOOL	 m_bRotatedToRest;
		uint32	 m_cBounces;
		LTBOOL	 m_bAddToGrenadeList;

		// Don't need to save this...

        HLTSOUND    m_hBounceSnd;   // Handle to current bounce sound
};


// RETAIL: Tron has none of NOLF2's gadget projectile classes that were
// here (see ProjectileTypes.cpp).

#endif //  __PROJECTILE_TYPES_H__
