// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDamageDir.h
//
// PURPOSE : HUDItem to display directional damage info
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// RETAIL: NOLF2's TO2 file, which Tron's CShell.dll has too (its RTTI and
// code, retail 0x10024fd0..0x100257a0)
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_DAMAGE_DIR_H
#define __HUD_DAMAGE_DIR_H

#include "HUDItem.h"



//******************************************************************************************
//** HUD directional damage display
//******************************************************************************************
class CHUDDamageDir : public CHUDItem
{
public:
	CHUDDamageDir();

	// RETAIL: as most of Tron's HUD items (retail 0x10023440)
	virtual bool	IsUsedInGameType(GameType eGameType) { return eGameTypeSingle == eGameType || (eGameType > eGameTypeSingleLightCycleCustom && eGameType <= eGameTypeDiscTournament); }

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

	void		UpdateScale();

private:
	uint16		m_nSize;

	LTBOOL		m_bDraw;
	float		m_fScale;

	enum eConstants
	{
		kNumDamageSectors = 12,
	};


	LT_POLYGT4	m_Poly[kNumDamageSectors];	// RETAIL: Tron's (its constructor: 0x60 a quad)

	HTEXTURE	m_hArrow;


};

#endif