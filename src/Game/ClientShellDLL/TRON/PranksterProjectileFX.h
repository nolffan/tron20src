// ----------------------------------------------------------------------- //
//
// MODULE  : PranksterProjectileFX.h
//
// PURPOSE : The Prankster's projectile special fx - Definition
//
// RETAIL: Tron 2.0's (RTTI CPranksterProjectileFX, CShell.dll vtable
// 0x101ec978; functions 0x101bfe30..0x101bffa7). SFX id 59. The member
// and setter names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __PRANKSTER_PROJECTILE_FX_H__
#define __PRANKSTER_PROJECTILE_FX_H__

#include "ClientFXMgr.h"
#include "ProjectileFX.h"

class CPranksterProjectileFX : public CProjectileFX
{
	public :

		CPranksterProjectileFX();
		~CPranksterProjectileFX();

		virtual void	Term();
		virtual LTBOOL	CreateObject(ILTClient* pClientDE);

		virtual uint32	GetSFXID() { return SFX_PRANKSTERPROJECTILE_ID; }

		void	SetTronUnknown7C(LTBOOL bValue);

	protected :

		LTBOOL	m_bTronUnknown7C;	// +0x7c: set once the object exists
};

#endif // __PRANKSTER_PROJECTILE_FX_H__
