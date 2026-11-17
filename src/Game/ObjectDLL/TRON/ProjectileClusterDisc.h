// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileClusterDisc.h
//
// PURPOSE : TRON Disc cluster class - definition
//
// CREATED : 04/15/01
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
//
// RETAIL: Tron's CClusterDisc (vtable 0x10294520, 0xa38 bytes; constructor
// 0x10035700) does not emit shards: it explodes when it goes back to its
// owner, after a time that the class data's InitialExplosionSuppressTime
// gives.  The names of the members are ours.
//
// ----------------------------------------------------------------------- //

#ifndef  _ProjectileClusterDisc_INCLUDED_
#define  _ProjectileClusterDisc_INCLUDED_

#include "ProjectileDisc.h"


class CClusterDisc : public CDisc
{
public:
	// destructor
	~CClusterDisc();

	// constructor
	CClusterDisc();

protected:
	// setup the disc (called externally)
	virtual LTBOOL      Setup( CWeapon const*, WeaponFireInfo const& );

	virtual uint32      EngineMessageFn( uint32, void*, LTFLOAT );

	// code to handle state changing information on ReturnMode
	virtual void        EnterReturnMode( HOBJECT hObj );

	// RETAIL: 0x100358c0
	void				Burst();

	// RETAIL: 0x100358e0: the explosion of the weapon's cluster modifier
	void				CreateExplosion();

	// RETAIL: 0x10035dc0: the client FX of the weapon's cluster modifier
	void				PlayBurstFX();

private:
	void				Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
	void				Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

protected:
	LTFLOAT				m_fFireTime;				// 0xa28 (a time)
	LTFLOAT				m_fExplosionSuppressTime;	// 0xa2c
	LTFLOAT				m_fTronUnknownA30;			// 0xa30 (a time)
	bool				m_bTronUnknownA34;			// 0xa34
};

#endif // _ProjectileClusterDisc_INCLUDED_
