// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleMissile.h
//
// PURPOSE : The missile power-up of the light cycle game
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x10029f40..0x1002aa50: its class def at 0x102da3f8, hidden,
// no properties; vtable 0x10293e0c, 0x7c bytes). The class name is
// retail's; the rest are ours. CServerLightCycleMgr::ActivatePowerup
// creates it and sets its owner, position, direction and cyclist.
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTCYCLEMISSILE_H__
#define __LIGHTCYCLEMISSILE_H__

#include "GameBase.h"

struct LIGHT_CYCLIST;

class LightCycleMissile : public GameBase
{
	public :

		LightCycleMissile();							// RETAIL: 0x1002a350
		virtual ~LightCycleMissile();					// RETAIL: 0x1002a440

	protected :

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);	// RETAIL: 0x1002a500

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);	// RETAIL: 0x1002a4f0
		LTBOOL	InitialUpdate();							// RETAIL: 0x1002a5a0
		void	Update();									// RETAIL: 0x1002a5c0
		void	CreateFX();									// RETAIL: 0x1002a970

		// What the missile flies into. RETAIL: 0x1002a9e0
		static bool	FilterFn(HOBJECT hObj, void *pUserData);

		LTObjRef		m_hFX;							// RETAIL: +0x38 (its looping client fx)
		bool			m_bCreateFX;					// RETAIL: +0x48 (not when loaded)

	public :

		// Set by CServerLightCycleMgr::ActivatePowerup (retail 0x10051cb3)
		LTObjRef		m_hOwner;						// RETAIL: +0x4c (the cycle that fired it)
		LTVector		m_vStartPos;					// RETAIL: +0x5c
		LTVector		m_vDir;							// RETAIL: +0x68
		LIGHT_CYCLIST*	m_pOwnerCyclist;				// RETAIL: +0x74

	private :

		LTFLOAT			m_fLastHitTime;					// RETAIL: +0x78 (of a trail)
};

#endif // __LIGHTCYCLEMISSILE_H__
