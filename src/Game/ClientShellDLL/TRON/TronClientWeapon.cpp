// ----------------------------------------------------------------------- //
//
// MODULE  : TronClientWeapon.cpp
//
// PURPOSE : Tron's common base of its client weapons (retail only, written
//           from CShell.dll 0x10097060..; see TronClientWeapon.h).
//
//           Only what the derived weapons need is here so far: the
//           constructor, destructor, OnMessage, Update, Load/Save and the
//           small helpers.  The larger new virtuals are stand-ins marked
//           "not yet written" (this TU is the 0x10090000 band's).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronClientWeapon.h"
#include "MsgIDs.h"
#include "SubroutineMgr.h"
#include "GameClientShell.h"
#include "ScreenSubroutines.h"	// RETAIL: its statics' initializers (0x10096ff0..)

// RETAIL: 0x10097090
CTronClientWeapon::CTronClientWeapon()
	: CClientWeapon(),
	  m_fTronUnknown204( 0.0f )
{
	TronUnknown97F40();
}

// RETAIL: 0x10097060
CTronClientWeapon::~CTronClientWeapon()
{
}

// RETAIL: 0x100970e0
bool CTronClientWeapon::OnMessage( uint8 messageID, ILTMessage_Read *pMsg )
{
	if ( MID_PROJECTILE == messageID )
	{
		if ( HandleMessageProjectile( messageID, pMsg ) )
		{
			return true;
		}
	}

	return false;
}

// RETAIL: 0x10097110 (three arguments there: see the header)
WeaponState CTronClientWeapon::Update( bool bFire, FireType eFireType )
{
	bool bStopped = false;

	if ( TronUnknownVirtual18C() )
	{
		bFire = false;
		eFireType = FT_NORMAL_FIRE;
		bStopped = true;
	}

	TronUnknownVirtual180();

	if ( bStopped )
	{
		return W_FIRING_NOAMMO;
	}

	return CClientWeapon::Update( bFire, eFireType );
}

// RETAIL: 0x100971e0
void CTronClientWeapon::Select()
{
	TronUnknown98B20();
	CClientWeapon::Select();
}

// RETAIL: 0x10097160
void CTronClientWeapon::Load( ILTMessage_Read *pMsg )
{
	if ( !pMsg )
	{
		return;
	}

	TronUnknown97F40();
	TronUnknownVirtual178( pMsg );
	m_fTronUnknown204 = pMsg->Readfloat();

	CClientWeapon::Load( pMsg );
}

// RETAIL: 0x100971a0.  Slot 68 (ours TronUnknownVirtual110, an int there)
// takes the message.
void CTronClientWeapon::Save( ILTMessage_Write *pMsg )
{
	if ( !pMsg )
	{
		return;
	}

	TronUnknownVirtual110( (int)pMsg );
	pMsg->Writefloat( m_fTronUnknown204 );

	CClientWeapon::Save( pMsg );
}

// RETAIL: 0x10097200 (651 bytes) not yet written
float CTronClientWeapon::GetEnergyUse()
{
	return 0.0f;
}

// RETAIL: 0x100976c0
bool CTronClientWeapon::TronUnknownVirtual170()
{
	TronUnknown98B20();
	return PlayDeselectAnimation();
}

// RETAIL: 0x100978f0 (296 bytes) not yet written
bool CTronClientWeapon::HandleMessageProjectile( uint8 messageID, ILTMessage_Read *pMsg )
{
	return false;
}

// RETAIL: 0x10097c30 (358 bytes) not yet written
void CTronClientWeapon::TronUnknownVirtual178( ILTMessage_Read *pMsg )
{
}

// RETAIL: 0x10098b30 (105 bytes) not yet written: it drains
// m_fTronUnknown204 * the frame time (client shell +0x7bac) of energy through
// Tron's float energy API of CTronPlayerStats (0x100a9c90 the energy left
// after the subroutines' reserve, 0x100a9d00 telling the server), which our
// CTronPlayerStats (uint32 energy) does not have yet
void CTronClientWeapon::TronUnknownVirtual180()
{
}

// RETAIL: 0x10098be0 not yet written: it returns
// g_pTronPlayerStats's 0x100ac2f0 (GetWeaponId(); that weapon's subroutine,
// through the weapon's +0xa4 and the subroutine manager's 0x100952e0),
// neither written yet
bool CTronClientWeapon::TronUnknownVirtual184()
{
	return false;
}

// RETAIL: 0x10098d10
bool CTronClientWeapon::TronUnknownVirtual18C()
{
	if ( TronUnknownVirtual184() && TronUnknownVirtual164() )
	{
		return false;
	}

	if ( W_BEGIN_FIRING == GetState() ||
	     W_FIRING_NOAMMO == GetState() ||
	     W_RELOADING == GetState() ||
	     W_SWAT_DEFENSE == GetState() ||
	     W_FIRED == GetState() )
	{
		return false;
	}

	return true;
}

// RETAIL: 0x10098c00: the player's subroutine's version and state, and
// the game's difficulty
SubroutineDataKey CTronClientWeapon::GetSubroutineDataKey( const char *szName )
{
	PlayerSubroutine *pSub = g_pSubroutineMgr->GetPlayerSubroutine( szName, LTNULL );

	int nVersion = kSubData_Alpha;
	switch ( pSub->eVersion )
	{
		case VERSION_ALPHA: nVersion = kSubData_Alpha; break;
		case VERSION_BETA:  nVersion = kSubData_Beta;  break;
		case VERSION_GOLD:  nVersion = kSubData_Gold;  break;
	}

	int nState = kSubData_Okay;
	switch ( pSub->eState )
	{
		case SUBSTATE_OKAY:    nState = kSubData_Okay;    break;
		case SUBSTATE_CORRUPT: nState = kSubData_Corrupt; break;
	}

	return SubroutineDataKey( nVersion, nState,
		g_pGameClientShell ? g_pGameClientShell->GetDifficulty() : kSubData_Easy );
}

// RETAIL: 0x10098c90 (119 bytes) not yet written: it is
// InterpolateRating<float>(Rating_Accuracy, ...) of three floats of Tron's
// AMMO at +0x18, +0x1c and +0x20 (energy use min, max, surge), which our
// AMMO (NOLF2's) does not have yet
float CTronClientWeapon::GetBaseEnergyUse()
{
	return 0.0f;
}

// RETAIL: 0x10098b20
void CTronClientWeapon::TronUnknown98B20()
{
	m_fTronUnknown204 = 0.0f;
}

// RETAIL: 0x10097f40 clears eleven pairs of bools at CClientWeapon+0x13c
// (not yet at retail's offset in ours)
void CTronClientWeapon::TronUnknown97F40()
{
	for ( int i = 0; i < 11; ++i )
	{
		m_abTronUnknown13C[i][0] = false;
		m_abTronUnknown13C[i][1] = false;
	}
}
