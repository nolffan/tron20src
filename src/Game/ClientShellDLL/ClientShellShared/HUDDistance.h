// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDistance.h
//
// PURPOSE : HUDItem to display player distance to an area
//
// CREATED : 5/08/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_DISTANCE_H__
#define __HUD_DISTANCE_H__

//
// Includes...
//

	#include "HUDItem.h"
	#include "HUDBar.h"


class CHUDDistance : public CHUDItem
{
	public: // Methods...
	// RETAIL: single player only
	// RETAIL: CHUDBuildNoteCount's form (0x100226c0, which the HUD items fold into)
	virtual bool	IsUsedInGameType(GameType eGameType) { return (eGameTypeSingle == eGameType) ? true : false; }


		CHUDDistance();
		~CHUDDistance();

		LTBOOL      Init();
		void        Term();		// RETAIL: Tron's (0x101613b0)
		void        Render();
		void        Update();
		void        UpdateLayout();

	private: // Members...

		LTIntPt		m_BasePos;
		float		m_fBlinkSpeed;
		float		m_fAlpha;
		float		m_fIconSize;

		bool		m_bDraw;
		bool		m_bBlink;
		float		m_fFadeIncrement;
		int8		m_nFadeDir;
		bool		m_bFadeOut;
		float		m_fFadeOutSpeed;
		float		m_fMaxAlpha;

		// RETAIL: Tron's (+0x38): true after Term, until the next Update
		// queues a fresh update (0x101614f0). The name is ours
		bool		m_bReset;

		float		m_fDistPercent;

		// RETAIL: an LT_POLYGT4 (cleared once, 0x1016135c)
		LT_POLYGT4	m_Poly;
		HTEXTURE	m_hIcon;			//  icon
};

#endif // __HUD_DISTANCE_H__