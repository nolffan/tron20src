// ----------------------------------------------------------------------- //
//
// MODULE  : SpectatorObjectFX.h
//
// PURPOSE : Spectator object special fx - Definition
//
// RETAIL: Tron 2.0's (RTTI CSpectatorObjectFX, CShell.dll vtable
// 0x101eb1c0; Init 0x101c02c0, OnServerMessage 0x101c0330; GetSFXID
// 0x101140e0). SFX id 62. CSFXMgr::GetSpectatorFX (0x101086a0) finds
// them. The member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SPECTATOR_OBJECT_FX_H__
#define __SPECTATOR_OBJECT_FX_H__

#include "SpecialFX.h"

class CSpectatorObjectFX : public CSpecialFX
{
	public :

		// RETAIL: inline (CSFXMgr's bank, 0x101140cb): the vector stays
		// unset
		CSpectatorObjectFX()
		{
			m_bTronUnknown38 = false;
		}

		virtual LTBOOL	Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
		virtual LTBOOL	OnServerMessage(ILTMessage_Read *pMsg);

		virtual uint32	GetSFXID() { return SFX_SPECTATOROBJECT_ID; }

		bool			GetTronUnknown38() const { return m_bTronUnknown38; }
		const LTVector&	GetTronUnknown3C() const { return m_vTronUnknown3C; }

	protected :

		bool		m_bTronUnknown38;	// +0x38
		LTVector	m_vTronUnknown3C;	// +0x3c
};

#endif // __SPECTATOR_OBJECT_FX_H__
