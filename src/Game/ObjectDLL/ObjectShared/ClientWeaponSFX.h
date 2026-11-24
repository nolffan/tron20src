// ----------------------------------------------------------------------- //
//
// MODULE  : CClientWeaponSFX.h
//
// PURPOSE : CClientWeaponSFX - Definition
//
// CREATED : 2/22/98
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_WEAPON_SFX_H__
#define __CLIENT_WEAPON_SFX_H__

#include "iltserver.h"
#include <memory.h>  // for memset
#include "ImpactType.h"

struct CLIENTWEAPONFX
{
    CLIENTWEAPONFX();

    HOBJECT      hFiredFrom;
    HOBJECT      hObj;
    uint8        nWeaponId;
    uint8        nAmmoId;
    uint8        nSurfaceType;
    uint16       wIgnoreFX;
    uint8        nShooterId;
	uint8        m_xTronUnknownF;		// RETAIL: Tron's (+0xf, sent after nShooterId: 0x101d6918)
    LTVector     vFirePos;
    LTVector     vPos;
    LTVector     vSurfaceNormal;
    IMPACT_TYPE  eImpactType;
	LTFLOAT      m_xTronUnknown38;		// RETAIL: Tron's (+0x38, a float: 0x101d697b)
	uint8        m_xTronUnknown3C;		// RETAIL: Tron's (+0x3c, sent as a float: 0x101d6992)
};

inline CLIENTWEAPONFX::CLIENTWEAPONFX()
{
	memset(this, 0, sizeof(CLIENTWEAPONFX));
	nShooterId = -1;
	// RETAIL: Tron also sets these (Lock 0x10255149, Projectile 0x10120ee6)
	m_xTronUnknownF = -1;
	m_xTronUnknown38 = 1.0f;
}


void CreateClientWeaponFX(CLIENTWEAPONFX & theStruct);


#endif // __CLIENT_WEAPON_SFX_H__