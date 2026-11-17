// ----------------------------------------------------------------------- //
//
// MODULE  : LensFlareFX.h
//
// PURPOSE : LensFlare special fx class - Definition
//
// CREATED : 5/9/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LENS_FLARE_FX_H__
#define __LENS_FLARE_FX_H__

#include "SpecialFX.h"

struct LENSFLARECREATESTRUCT : public SFXCREATESTRUCT
{
	LENSFLARECREATESTRUCT();

	// RETAIL: Tron's takes the object too, for the socket (0x101a58f0)
    LTBOOL InitFromMessage(HOBJECT hObj, LENSFLARECREATESTRUCT & lens, ILTMessage_Read *pMsg);

    LTBOOL   bInSkyBox;
    LTBOOL   bCreateSprite;
    LTBOOL   bSpriteOnly;
    LTBOOL   bUseObjectAngle;
    LTBOOL   bSpriteAdditive;
    LTFLOAT  fSpriteOffset;
    LTFLOAT  fMinAngle;
    LTFLOAT  fMinSpriteAlpha;
    LTFLOAT  fMaxSpriteAlpha;
    LTFLOAT  fMinSpriteScale;
    LTFLOAT  fMaxSpriteScale;
	HSTRING	hstrSpriteFile;
    LTBOOL   bBlindingFlare;
    LTFLOAT  fBlindObjectAngle;
    LTFLOAT  fBlindCameraAngle;
    LTFLOAT  fMinBlindScale;
    LTFLOAT  fMaxBlindScale;
	// RETAIL: Tron's: the socket of the object the flare is at (+0x4c; its
	// constructor sets 0, its users test for INVALID_MODEL_SOCKET:
	// 0x1010ae84, 0x101a6bb5). The name is ours
	HMODELSOCKET	hSocket;
};

inline LENSFLARECREATESTRUCT::LENSFLARECREATESTRUCT()
{
    bInSkyBox           = LTFALSE;
    bCreateSprite       = LTFALSE;
    bSpriteOnly         = LTFALSE;
    bUseObjectAngle     = LTFALSE;
    bBlindingFlare      = LTFALSE;
    bSpriteAdditive     = LTTRUE;
	fSpriteOffset		= 0.0f;
	fMinAngle			= 0.0f;
	fMinSpriteAlpha		= 0.0f;
	fMaxSpriteAlpha		= 0.0f;
	fMinSpriteScale		= 0.0f;
	fMaxSpriteScale		= 0.0f;
	fBlindObjectAngle	= 0.0f;
	fBlindCameraAngle	= 0.0f;
	fMinBlindScale		= 0.0f;
	fMaxBlindScale		= 0.0f;
    hstrSpriteFile      = LTNULL;
	hSocket				= 0;
}

class CLensFlareFX : public CSpecialFX
{
	public :

		CLensFlareFX() : CSpecialFX()
		{
            m_hFlare = LTNULL;
		}

		~CLensFlareFX()
		{
			if (m_hFlare)
			{
				m_pClientDE->RemoveObject(m_hFlare);
			}

			if (m_cs.hstrSpriteFile)
			{
				m_pClientDE->FreeString(m_cs.hstrSpriteFile);
			}
		}

        virtual LTBOOL Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_LENSFLARE_ID; }

	protected :

		// RETAIL: 0x101a6ba0 (the name is ours): the flare's position, at
		// its socket when it has one
		void	GetFlarePos(LTVector* pvPos);

		LENSFLARECREATESTRUCT		m_cs;
		HOBJECT						m_hFlare;
};

#endif // __LENS_FLARE_FX_H__