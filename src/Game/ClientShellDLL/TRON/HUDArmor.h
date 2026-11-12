// ----------------------------------------------------------------------- //
//
// MODULE  : HUDArmor.h
//
// PURPOSE : HUDItem to display Jet's Armor
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_ARMOR_H
#define __HUD_ARMOR_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD Armor display
//**
//** RETAIL: as retail CShell.dll has it (0x10021e00..0x10022350, vtable
//** 0x101e6be0): the base and six pieces at the layout's armor offset from
//** the bottom right, each piece drawn in its own texture or, when its
//** subroutine is corrupt, its corrupt one. Member names past build 69's
//** are ours.
//******************************************************************************************
class CHUDArmor : public CHUDItem
{
public:
	// RETAIL: eGameTypeSingle, Cooperative, Deathmatch and the disc games
	// (retail's commonest slot 3, 0x10023440: a switch)
	virtual bool	IsUsedInGameType(GameType eGameType)
	{
		switch (eGameType)
		{
			case eGameTypeSingle:
			case eGameTypeCooperative:
			case eGameTypeDeathmatch:
			case eGameTypeDiscArena:
			case eGameTypeDiscTournament:
				return true;
		}
		return false;
	}

	CHUDArmor();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();
    void        UpdateLayout();

private:
	float		m_fScale;				// +0x10
	float		m_fLayoutScale;			// +0x14
	bool		m_bHavePiece[8];		// +0x18
	bool		m_bCorrupt[8];			// +0x20
	LT_POLYFT4	m_Base;					// +0x28
	LT_POLYFT4	m_ArmorPiece[8];		// +0x7c
	HTEXTURE	m_hBaseTex;				// +0x31c
	HTEXTURE	m_hArmorPieceTex[8];	// +0x320
	HTEXTURE	m_hCorruptPieceTex[8];	// +0x340
};

#endif