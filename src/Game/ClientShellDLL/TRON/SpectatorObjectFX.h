// ----------------------------------------------------------------------- //
//
// MODULE  : SpectatorObjectFX.h
//
// PURPOSE : RETAIL: Tron 2.0's, retail-only (RTTI .?AVCSpectatorObjectFX@@,
//           vtable 0x101eb1c0; no source anywhere): the client side of an
//           object a light cycle spectator can watch (SFX_SPECTATOROBJECT_ID;
//           CSFXMgr::GetSpectatorFX 0x101086a0 and GetNextSpectatorFX
//           0x101086f0 find them). 0x48 bytes. The member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SPECTATOR_OBJECT_FX_H__
#define __SPECTATOR_OBJECT_FX_H__

#include "SpecialFX.h"

class CSpectatorObjectFX : public CSpecialFX
{
	public :

		// RETAIL: inline (CBankedList<CSpectatorObjectFX>::New, 0x10113f70);
		// the destructor is the compiler's
		CSpectatorObjectFX()
		{
			m_bTronUnknown38 = false;
		}

		virtual LTBOOL	Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);	// RETAIL: 0x101c02c0
		virtual LTBOOL	OnServerMessage(ILTMessage_Read *pMsg);			// RETAIL: 0x101c0330

		// RETAIL: 0x101140e0
		virtual uint32	GetSFXID() { return SFX_SPECTATOROBJECT_ID; }

		// RETAIL: GetNextSpectatorFX (0x101086f0) reads it directly
		bool	GetTronUnknown38() const { return m_bTronUnknown38; }

	protected :

		bool		m_bTronUnknown38;	// +0x38, sent by the server
		LTVector	m_vTronUnknown3C;	// +0x3c, sent by the server

		friend class CSFXMgr;
};

#endif // __SPECTATOR_OBJECT_FX_H__
