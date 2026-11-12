// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponSuffusion.h
//
// PURPOSE : Tron's client-side suffusion polymer weapon (retail only: RTTI
//           .?AVCClientWeaponSuffusion@@, vtable 0x101e6988; CShell.dll
//           0x1001bc80..0x1001c72b).  Member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef  _ClientWeaponSuffusion_h_INCLUDED_
#define  _ClientWeaponSuffusion_h_INCLUDED_

#include "TronClientWeapon.h"
#include "Timer.h"

class CClientWeaponSuffusion : public CTronClientWeapon
{
public:
	CClientWeaponSuffusion();
	virtual ~CClientWeaponSuffusion();

	virtual void  Load( ILTMessage_Read *pMsg );
	virtual void  Save( ILTMessage_Write *pMsg );

protected:
	// RETAIL: slot 56 (0x1001c3d0): is it a fire animation (the extra one
	// counts unless only the normal ones are asked for)
	virtual bool  TronUnknownVirtualE0( int nArg1, int nArg2 );

	virtual void  SendFireMessage( bool bFire,
	                               LTFLOAT fPerturb,
	                               LTVector const &vFirePos,
	                               LTVector const &vDir,
	                               LTVector const &vImpactPos,
	                               HOBJECT hObject );

	// RETAIL: 0x1001bd20
	virtual uint8 GetFireMessageType() const { return kTronFire_Suffusion; }

	// RETAIL: 0x1001c410
	bool          IsExtraFireAni( uint32 dwAni );

	HMODELANIM    m_nExtraFireAni;		// RETAIL: +0x208
	float         m_fTronUnknown20C;	// RETAIL: +0x20c (saved, sent with the fire message)
	CTimer       *m_pTimer;				// RETAIL: +0x210
	float         m_fTronUnknown214;	// RETAIL: +0x214 (InitAnimations sets it)
};

#endif //_ClientWeaponSuffusion_h_INCLUDED_
