// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleFX.h
//
// PURPOSE : A light cycle's special fx - Definition
//
// RETAIL: Tron 2.0's (RTTI CLightCycleFX, CShell.dll vtable 0x101eb140;
// functions 0x1012cf00..0x1012da52; SFX id 58). It keeps the cycle's model
// on its rider, plays the derez and respawn fx and the engine sound. The
// names of the members and of the non-virtual functions are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTCYCLE_FX_H__
#define __LIGHTCYCLE_FX_H__

#include "SpecialFX.h"
#include "SharedFXStructs.h"
#include "ClientFXMgr.h"

class CLightCycleFX : public CSpecialFX
{
	public :

		// RETAIL: inline (CSFXMgr's bank, 0x10113225)
		CLightCycleFX()
			: CSpecialFX(),
			m_bTronUnknown68(false),
			m_bTronUnknown69(true),
			m_bTronUnknown6A(false),
			m_bTronUnknown6B(true),
			m_hTronUnknown70(LTNULL)
		{
		}

		~CLightCycleFX();

		virtual void	Term();
		virtual LTBOOL	Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
		virtual LTBOOL	Update();
		virtual LTBOOL	OnServerMessage(ILTMessage_Read *pMsg);

		virtual uint32	GetSFXID() { return SFX_LIGHTCYCLE_ID; }

		// RETAIL: 0x1012d650, 0x1012d720, 0x1012d7f0, 0x1012d8c0 (called by
		// CClientLightCycleMgr), 0x1012d990 (called by CTronPlayerMgr::Update)
		void	CreateTronUnknownFX4C(const char* pszFX);
		void	CreateTronUnknownFX50(const char* pszFX);
		void	CreateTronUnknownFX54(const char* pszFX);
		void	CreateTronUnknownFX58(const char* pszFX, const LTVector &vOffset);
		void	ShowTronUnknownFX48(HOBJECT hObj, bool bShow);

		// RETAIL: inline (CSFXMgr::GetLightCycleFX, 0x10108685); the name is
		// ours
		HOBJECT	GetRider() const { return m_hTronUnknown70; }

	protected :

		bool	FindCyclist();		// RETAIL: 0x1012da00

		LIGHTCYCLECREATESTRUCT	m_cs;						// +0x38
		HLTSOUND				m_hTronUnknownSnd44;		// +0x44: the engine sound
		CLIENTFX_LINK			m_TronUnknownFX48;			// +0x48: the cycle's own fx
		CLIENTFX_LINK			m_TronUnknownFX4C;
		CLIENTFX_LINK			m_TronUnknownFX50;
		CLIENTFX_LINK			m_TronUnknownFX54;
		CLIENTFX_LINK			m_TronUnknownFX58;			// positioned at m_vTronUnknownFXOffset
		LTVector				m_vTronUnknownFXOffset;		// +0x5c
		bool					m_bTronUnknown68;			// +0x68: waiting for the derez fx
		bool					m_bTronUnknown69;			// +0x69: start the derez fx
		bool					m_bTronUnknown6A;			// +0x6a: hidden (Term)
		bool					m_bTronUnknown6B;			// +0x6b: start the cycle's fx
		bool					m_bTronUnknown6C;			// +0x6c: don't follow the rider
		HOBJECT					m_hTronUnknown70;			// +0x70: the rider
};

#endif // __LIGHTCYCLE_FX_H__
