// ----------------------------------------------------------------------- //
//
// MODULE  : SearchLightFX.h
//
// PURPOSE : SearchLight special fx class - Definition
//
// CREATED : 6/8/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SEARCH_LIGHT_FX_H__
#define __SEARCH_LIGHT_FX_H__

#include "SpecialFX.h"
#include "LensFlareFX.h"

struct SEARCHLIGHTCREATESTRUCT : public SFXCREATESTRUCT
{
	SEARCHLIGHTCREATESTRUCT();

    LTFLOAT  fBeamLength;
    LTFLOAT  fBeamRadius;
    LTFLOAT  fBeamAlpha;
    LTFLOAT  fBeamRotTime;
    LTFLOAT  fLightRadius;
    LTVector vLightColor;
    LTBOOL   bBeamAdditive;
	// RETAIL: Tron's: the socket of the object the beam is at (+0x2c; the
	// constructor sets 0, the users test for INVALID_MODEL_SOCKET:
	// 0x101ae36d, 0x101af172). The name is ours
	HMODELSOCKET	hSocket;

	LENSFLARECREATESTRUCT lens;
};

inline SEARCHLIGHTCREATESTRUCT::SEARCHLIGHTCREATESTRUCT()
{
	fBeamLength		= 0.0f;
	fBeamRadius		= 0.0f;
	fBeamAlpha		= 0.0f;
	fBeamRotTime	= 0.0f;
	fLightRadius	= 0.0f;
    bBeamAdditive   = LTTRUE;
	vLightColor.Init();
	hSocket			= 0;
}


class CSearchLightFX : public CSpecialFX
{
	public :

		CSearchLightFX() : CSpecialFX()
		{
            m_hBeam     = LTNULL;
            m_hLight    = LTNULL;

			m_fBeamRotation = 0.0f;
		}

		~CSearchLightFX()
		{
			if (m_hBeam)
			{
				m_pClientDE->RemoveObject(m_hBeam);
			}

			if (m_hLight)
			{
				m_pClientDE->RemoveObject(m_hLight);
			}
		}

        virtual LTBOOL Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_SEARCHLIGHT_ID; }

	protected :

		// RETAIL: 0x101af160 (the name is ours): the beam's position, at its
		// socket when it has one
		void	GetBeamPos(LTVector* pvPos);

		SEARCHLIGHTCREATESTRUCT		m_cs;
		HOBJECT						m_hBeam;
		HOBJECT						m_hLight;

        LTFLOAT                      m_fBeamRotation;
		CLensFlareFX				m_LensFlare;
};

#endif // __SEARCH_LIGHT_FX_H__