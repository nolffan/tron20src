// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerSoundFX.h
//
// PURPOSE : Player Sound fx - Definition
//
// CREATED : 10/28/98 (was WeaponSoundFX)
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_SOUND_FX_H__
#define __PLAYER_SOUND_FX_H__

#include "SpecialFX.h"

struct PLAYERSOUNDCREATESTRUCT : public SFXCREATESTRUCT
{
    PLAYERSOUNDCREATESTRUCT();

    LTVector    vPos;
    uint8       nClientId;
    uint8       nType;
    uint8       nWeaponId;
	// RETAIL: Tron's (CShell 0x101050a0, the names are ours): the fire
	// sounds' volume scale and a bool for PlayWeaponSound
	LTFLOAT		fVolumeScale;	// +0x18
	bool		bTronUnknown;	// +0x1c
};

inline PLAYERSOUNDCREATESTRUCT::PLAYERSOUNDCREATESTRUCT()
{
	vPos.Init();
	nClientId	= 0;
	nType		= 0;
	nWeaponId	= 0;
	fVolumeScale = 0.0f;
	bTronUnknown = false;
}

class CPlayerSoundFX : public CSpecialFX
{
	public :

		CPlayerSoundFX() : CSpecialFX()
		{
			m_vPos.Init();
			m_nClientId	= 0;
			m_nType		= 0;
			m_nWeaponId	= 0;
			m_fVolumeScale	= 0.0f;
			m_bTronUnknown	= false;
		}

		~CPlayerSoundFX()
		{
		}

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update() { return LTFALSE; }

		virtual uint32 GetSFXID() { return SFX_PLAYERSOUND_ID; }

	private :

        LTVector	m_vPos;
        uint8		m_nClientId;
        uint8		m_nType;
        uint8		m_nWeaponId;
		LTFLOAT		m_fVolumeScale;	// RETAIL: +0x48 (the names are ours)
		bool		m_bTronUnknown;	// RETAIL: +0x4c
};

#endif // __PLAYER_SOUND_FX_H__