// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponDisc.cpp
//
// PURPOSE : Tron specific client-side version of the disc
//
// CREATED : 4/12/02 (was WeaponModel.h)
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ClientWeaponDisc.h"

#include "MsgIDs.h"
#include "PlayerMgr.h"
#include "CMoveMgr.h"
#include "PlayerStats.h"
#include "RatingMgr.h"
#include "FXButeMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initializers (0x100189d0..0x10018a00)

// RETAIL: Tron's projectile message 14, what the release of the fire key
// sends (0x1001a181; Object.lto's disc reads it, ProjectileDisc.cpp has the
// same name). PORT: MsgIDs.h's MPROJ_RETURN (3) is NOLF2's. The name is ours
#define MPROJ_DISC_TRONUNKNOWN14		14


//
// Externs
//
extern bool g_bInfiniteAmmo;


namespace
{
	// also defined in ClientWeapon.cpp;
	HMODELANIM const INVALID_ANI = ( static_cast< HMODELANIM >( -1 ) );
	int const INFINITE_AMMO_AMOUNT = 1000;

	// defense animation names
	char *ns_szSwatDefenseAnimationName = "BlockSwat";
	char *ns_szPreHoldDefenseAnimationName = "PreBlock";
	char *ns_szHoldDefenseAnimationName = "Block";
	char *ns_szImpactHoldDefenseAnimationName = "BlockImpact";
	char *ns_szPostHoldDefenseAnimationName = "PostBlock";

	float const CLIENTWEAPONDISC_PROBE_LENGTH = 100000.0f;

	// RETAIL: Tron's console variable (0x10236e30; its constructor
	// 0x10018a40): the time between two control line updates of the disc
	// in a multiplayer game (CClientWeaponDisc::Update). The names are ours
	VarTrack g_vtDiscControlRate;
	bool g_bDiscVarsInit = false;
};

// RETAIL: 0x10018a60 (the name is ours): the disc's constructor calls it
// with g_pLTClient, but it uses the global
static void InitDiscVars( ILTClient * )
{
	if ( !g_bDiscVarsInit )
	{
		if ( !g_vtDiscControlRate.IsInitted() )
		{
			g_vtDiscControlRate.Init( g_pLTClient, "DiscControlRate", NULL, 0.1f );
		}

		g_bDiscVarsInit = true;
	}
}



// ---------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::CClientWeaponDisc()
//
//	PURPOSE:	Constructor
//
// ---------------------------------------------------------------------- //

// RETAIL: 0x10018b60 (near: it also clears CClientWeapon+0x12c, inits a
// "DiscControlRate" console variable, 0x10018a60, and starts
// m_fTimeDiscActive at 0)
CClientWeaponDisc::CClientWeaponDisc() :
	  CTronClientWeapon()
	, m_nSwatDefenseAni( INVALID_ANI )
	, m_nPreHoldDefenseAni( INVALID_ANI )
	, m_nHoldDefenseAni( INVALID_ANI )
	, m_nImpactHoldDefenseAni( INVALID_ANI )
	, m_nPostHoldDefenseAni( INVALID_ANI )
	, m_bPlayImpactHoldAnimation( false )
	, m_bIsDiscActive( false )
	, m_bFireMessageSent( false )
	, m_bDiscNeedsUpdates( false )
	, m_bIgnoreFireKeyframe( false )
	, m_fTimeDiscActive( 0.0f )
{
	// base class redefines
	m_bAutoSwitchEnabled = false;

	InitDiscVars( g_pLTClient );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::~CClientWeaponDisc()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CClientWeaponDisc::~CClientWeaponDisc()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::OnMessage()
//
//	PURPOSE:	Process messages from the server
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::OnMessage( uint8 messageID, ILTMessage_Read *pMsg )
{
	switch ( messageID )
	{
		case MID_PROJECTILE:
		{
			// projectile message, send to the hanler routine
			bool bMessageHandled = HandleMessageProjectile( messageID, pMsg );
			if ( bMessageHandled )
			{
				return true;
			}
		}
		break;

		default:
		{
			// only projectile messages should be going to the weapon
			ASSERT( 0 );
		}
		break;
	}  // switch( messageID )

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::UpdateModelState()
//
//	PURPOSE:	Update the model state for the disc
//
// ----------------------------------------------------------------------- //

WeaponState CClientWeaponDisc::UpdateModelState( bool bFire, bool bAltFire )
{
	WeaponState eRet = W_IDLE;

	// Determine what we should be doing...
	// (mostly updates animations, also
	// updates a couple supporting variables)
	// RETAIL: 0x10018c00: the fire key with the modifier swats, the alt
	// fire key holds
	if ( bFire )
	{
		if ( g_pLTClient->IsCommandOn( COMMAND_ID_DEFEND_MODIFIER ) )
		{
			UpdateSwatDefense();
		}
		else
		{
			UpdateFiring();
		}
	}
	else if ( bAltFire )
	{
		UpdateHoldDefense();
	}
	else
	{
		UpdateNonFiring();
	}

	// this is reset here just in case it was set while the hold
	// animation was not playing
	m_bPlayImpactHoldAnimation = false;

	if ( m_bFire )
	{
		// gadgets...always the special case  :-(
		bool bGadgetSpecialCase = m_pAmmo->eType != GADGET;

		// doesn't actually fire, just updates the
		// ammo and clears the m_bFire flag
		eRet = UpdateAmmoFromFire( bGadgetSpecialCase );

		// clear the fire flag
		m_bFire = false;
	}
	else if ( m_bIgnoreFireKeyframe )
	{
		// RETAIL: the bool at +0x225 asks for slot 102's message (which
		// retail bool is which of ours is a guess, see the header)
		TronUnknownVirtual198();
		m_bIgnoreFireKeyframe = false;
	}

	// See if we just finished deselecting the weapon...
	if ( m_bWeaponDeselected )
	{
		m_bWeaponDeselected = false;

		if ( m_cbDeselect )
		{
			// call the deselect callback
			m_cbDeselect( m_nWeaponId, m_pcbData );

			// clear out the callback data
			m_cbDeselect = 0;
			m_pcbData = 0;
		}

		// deactivate the current weapon, its not used anymore
		Deactivate();
	}

	return eRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::UpdateFiring()
//
//	PURPOSE:	Update the animation state of the model
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::UpdateFiring()
{
	m_bCanSetLastFire = true;

	if ( GetState() == W_RELOADING )
	{
		if ( !PlayReloadAnimation() )
		{
			SetState( W_FIRING );
		}
	}
	if ( GetState() == W_IDLE )
	{
		SetState( W_FIRING );
	}
	if ( GetState() == W_SELECT )
	{
		if ( !PlaySelectAnimation() )
		{
			SetState( W_FIRING );
		}
	}
	if ( GetState() == W_DESELECT )
	{
		if ( !PlayDeselectAnimation() )
		{
			SetState( W_FIRING );
		}
	}
	if ( GetState() == W_SWAT_DEFENSE )
	{
		if ( !PlaySwatDefenseAnimation( false ) )
		{
			SetState( W_FIRING );
		}
	}
	if ( GetState() == W_HOLD_DEFENSE )
	{
		if ( !PlayHoldDefenseAnimation( false ) )
		{
			SetState( W_FIRING );
		}
	}
	if ( ( GetState() == W_FIRING ) || ( GetState() == W_FIRING_NOAMMO ) )
	{
		if ( PlayFireAnimation( true ) )
		{
			m_bCanSetLastFire = false;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::UpdateSwatDefense()
//
//	PURPOSE:	Update the animation state of the model
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::UpdateSwatDefense()
{
	if ( GetState() == W_RELOADING )
	{
		if ( !PlayReloadAnimation() )
		{
			SetState( W_SWAT_DEFENSE );
		}
	}
	if ( GetState() == W_IDLE )
	{
		SetState( W_SWAT_DEFENSE );
	}
	if ( GetState() == W_SELECT )
	{
		if ( !PlaySelectAnimation() )
		{
			SetState( W_SWAT_DEFENSE );
		}
	}
	if ( GetState() == W_DESELECT )
	{
		if ( !PlayDeselectAnimation() )
		{
			// TODO: the transition from this state should
			// be to W_INACTIVE, which involves making sure
			// making that change here won't negatively
			// affect anything downline
			SetState( W_SWAT_DEFENSE );
		}
	}
	if ( GetState() == W_HOLD_DEFENSE )
	{
		if ( !PlayHoldDefenseAnimation( false ) )
		{
			SetState( W_SWAT_DEFENSE );
		}
	}
	if ( ( GetState() == W_FIRING ) || ( GetState() == W_FIRING_NOAMMO ) )
	{
		if ( !PlayFireAnimation( false ) )
		{
			SetState( W_SWAT_DEFENSE );
		}
	}
	if ( GetState() == W_SWAT_DEFENSE )
	{
		PlaySwatDefenseAnimation( true );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::UpdateHoldDefense()
//
//	PURPOSE:	Update the animation state of the model
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::UpdateHoldDefense()
{
	if ( GetState() == W_RELOADING )
	{
		if ( !PlayReloadAnimation() )
		{
			SetState( W_HOLD_DEFENSE );
		}
	}
	if ( GetState() == W_IDLE )
	{
		SetState( W_HOLD_DEFENSE );
	}
	if ( GetState() == W_SELECT )
	{
		if ( !PlaySelectAnimation() )
		{
			SetState( W_HOLD_DEFENSE );
		}
	}
	if ( GetState() == W_DESELECT )
	{
		if ( !PlayDeselectAnimation() )
		{
			// TODO: the transition from this state should
			// be to W_INACTIVE, which involves making sure
			// making that change here won't negatively
			// affect anything downline
			SetState( W_HOLD_DEFENSE );
		}
	}
	if ( ( GetState() == W_FIRING ) || ( GetState() == W_FIRING_NOAMMO ) )
	{
		if ( !PlayFireAnimation( false ) )
		{
			SetState( W_HOLD_DEFENSE );
		}
	}
	if ( GetState() == W_SWAT_DEFENSE )
	{
		PlaySwatDefenseAnimation( true );
	}
	if ( GetState() == W_HOLD_DEFENSE )
	{
		PlayHoldDefenseAnimation( true );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::UpdateNonFiring
//
//	PURPOSE:	Update the non-firing animation state of the model.
//				RETAIL: 0x10019000
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::UpdateNonFiring()
{
	m_bCanSetLastFire = true;

	if ( GetState() == W_FIRING )
	{
		if ( !PlayFireAnimation( false ) )
		{
			SetState( W_IDLE );
		}
		else
		{
			m_bCanSetLastFire = false;
		}
	}
	if ( GetState() == W_FIRING_NOAMMO )
	{
		SetState( W_IDLE );
	}
	if ( GetState() == W_RELOADING )
	{
		ShowDisc();
		if ( !PlayReloadAnimation() )
		{
			SetState( W_IDLE );
		}
	}
	if ( GetState() == W_SELECT )
	{
		ShowDisc();
		if ( !PlaySelectAnimation() )
		{
			SetState( W_IDLE );
		}
	}
	if ( GetState() == W_DESELECT )
	{
		ShowDisc();
		if ( !PlayDeselectAnimation() )
		{
			m_bWeaponDeselected = true;
			SetState( W_IDLE );
		}
	}
	if ( GetState() == W_SWAT_DEFENSE )
	{
		ShowDisc();
		if ( !PlaySwatDefenseAnimation( false ) )
		{
			SetState( W_IDLE );
		}
	}
	if ( GetState() == W_HOLD_DEFENSE )
	{
		ShowDisc();
		if ( !PlayHoldDefenseAnimation( false ) )
		{
			SetState( W_IDLE );
		}
	}
	if ( GetState() == W_IDLE )
	{
		ShowDisc();
		PlayIdleAnimation();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::PlaySwatDefenseAnimation()
//
//	PURPOSE:	Set the model to the swat defense animation.
//				RETAIL: 0x10019920 (slot 100): plays it again unless it
//				still plays, and tells the server the disc is out
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::PlaySwatDefenseAnimation( bool bResetAni )
{
	uint32 dwAni = g_pLTClient->GetModelAnimation( m_hObject );
	uint32 dwState = g_pLTClient->GetModelPlaybackState( m_hObject );

	if ( IsSwatDefenseAni( dwAni ) )
	{
		if ( !( dwState & MS_PLAYDONE ) )
		{
			return true;
		}

		if ( bResetAni )
		{
			// button held down, play again
			PlayAnimation( GetSwatDefenseAni(), false, 1.0f, false );
			SendDiscFired();
			return true;
		}

		return false;
	}

	PlayAnimation( GetSwatDefenseAni(), false, 1.0f, false );
	SendDiscFired();
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::InitAnimations()
//
//	PURPOSE:	Initialize the disc specific weapons
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::InitAnimations( bool bAllowSelectOverride )
{
	// Swat defense animations
	m_nSwatDefenseAni = g_pLTClient->GetAnimIndex( m_hObject, ns_szSwatDefenseAnimationName );

	// Hold defense animations
	m_nPreHoldDefenseAni = g_pLTClient->GetAnimIndex( m_hObject, ns_szPreHoldDefenseAnimationName );
	m_nHoldDefenseAni = g_pLTClient->GetAnimIndex( m_hObject, ns_szHoldDefenseAnimationName );
	m_nImpactHoldDefenseAni = g_pLTClient->GetAnimIndex( m_hObject, ns_szImpactHoldDefenseAnimationName );
	m_nPostHoldDefenseAni = g_pLTClient->GetAnimIndex( m_hObject, ns_szPostHoldDefenseAnimationName );

	// init the base class animations
	CTronClientWeapon::InitAnimations( bAllowSelectOverride );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::IsSwatDefenseAni()
//
//	PURPOSE:	Returns true if the passed animation is a swat defense animation
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::IsSwatDefenseAni( uint32 dwAni )
{
	ASSERT( INVALID_ANI != m_nPreHoldDefenseAni );
	if ( m_nSwatDefenseAni == dwAni )
	{
		return true;
	}
	else
	{
		return false;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::GetSwatDefenseAni()
//
//	PURPOSE:	Returns the swat defense animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeaponDisc::GetSwatDefenseAni() const
{
	ASSERT( INVALID_ANI != m_nSwatDefenseAni );
	return m_nSwatDefenseAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::IsHoldDefenseAni()
//
//	PURPOSE:	Returns true if the passed animation is any type of hold defense
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::IsHoldDefenseAni( uint32 dwAni )
{
	ASSERT( INVALID_ANI != m_nPreHoldDefenseAni );
	ASSERT( INVALID_ANI != m_nHoldDefenseAni );
	ASSERT( INVALID_ANI != m_nImpactHoldDefenseAni );
	ASSERT( INVALID_ANI != m_nPostHoldDefenseAni );

	if ( ( dwAni == m_nPreHoldDefenseAni ) ||
	     ( dwAni == m_nHoldDefenseAni ) ||
	     ( dwAni == m_nImpactHoldDefenseAni ) ||
	     ( dwAni == m_nPostHoldDefenseAni ) )
	{
		return true;
	}
	else
	{
		return false;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::GetPreHoldDefenseAni()
//
//	PURPOSE:	Returns the pre hold defense animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeaponDisc::GetPreHoldDefenseAni() const
{
	ASSERT( INVALID_ANI != m_nPreHoldDefenseAni );
	return m_nPreHoldDefenseAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::GetHoldDefenseAni()
//
//	PURPOSE:	Returns the hold defense animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeaponDisc::GetHoldDefenseAni() const
{
	ASSERT( INVALID_ANI != m_nHoldDefenseAni );
	return m_nHoldDefenseAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::GetImpactHoldDefenseAni()
//
//	PURPOSE:	Return the impact defense animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeaponDisc::GetImpactHoldDefenseAni() const
{
	ASSERT( INVALID_ANI != m_nImpactHoldDefenseAni );
	return m_nImpactHoldDefenseAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::GetPostHoldDefenseAni()
//
//	PURPOSE:	Return the post hold defense animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeaponDisc::GetPostHoldDefenseAni() const
{
	ASSERT( INVALID_ANI != m_nPostHoldDefenseAni );
	return m_nPostHoldDefenseAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::HandleMessageProjectile()
//
//	PURPOSE:	Handle projectile messages from the server
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::HandleMessageProjectile( uint8 messageID,
                                                 ILTMessage_Read *pMsg )
{
	// get the projectile message subtype
	uint8 nProjectileMessageType = pMsg->Readuint8( );

	switch ( nProjectileMessageType )
	{
		case MPROJ_RETURNING:
		{
			//
			// disc is returning
			//

			// stop sending updates to the disc
			m_bDiscNeedsUpdates = false;

			// we handled message, return true
			return true;
		}
		break;

		case MPROJ_RETURNING_DISTANCE_THRESHOLD:
		{
			// don't handle this currently
		}
		break;

		case MPROJ_RETURNED:
		{
			//
			// we now have the disc!
			//

			// get the extra information to be sure

			// get the weapon id
			uint8 nWeaponId = pMsg->Readuint8();

			// get the ammo id
			uint8 nAmmoId = pMsg->Readuint8();

			// get the number of ammos (not needed, cleaning out the message)
			uint8 nAmmoAmount = pMsg->Readuint8();

			// Since the weapon cannot switch until we get
			// the disc back, make sure the returned type
			// is the same weapon and ammo type
			ASSERT( m_nWeaponId == nWeaponId );
			ASSERT( m_nAmmoId == nAmmoId );

			// disc is no longer flying
			m_bIsDiscActive = false;

			// stop sending updates to the disc
			m_bDiscNeedsUpdates = false;

			// reset the active timer
			m_fTimeDiscActive = -1.0f;

			// give the disc back
			IncrementAmmo();

			// we handled message, return true
			return true;
		}
		break;

		case MPROJ_BLOCKED:
		{
			// RETAIL: Tron's (0x10019290): the impact hold animation
			// plays next, and the FX of the block type
			m_bPlayImpactHoldAnimation = true;

			uint8 nDefendType = pMsg->Readuint8( );
			pMsg->SeekTo( 0 );

			CreateDefendFX( nDefendType );
		}
		break;

		default:
		{
			// The projectile message type is malformed...
			// perhaps the message was created improperly,
			// or perhaps the client weapon is not
			// designed to handle these messages.
			ASSERT( 0 );
		}
		break;
	};  // switch( nProjectileMessageType )

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::OnModelKey()
//
//	PURPOSE:	Update the animation state of the model
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::OnModelKey( HLOCALOBJ hObj, ArgList* pArgs )
{
	ASSERT( 0 != hObj );

	// get the keyframe intent
	char const *pKey = pArgs->argv[ 0 ];

	if ( 0 == stricmp( pKey, WEAPON_KEY_FIRE ) )
	{
		if ( !m_bIgnoreFireKeyframe )
		{
			// Only allow fire keys if it is a fire animation...
			uint32 dwAni = g_pLTClient->GetModelAnimation( m_hObject );
			if ( IsFireAni( dwAni ) )
			{
				// we've received a fire key, setup for throwing the disc
				m_bFire = true;

				// clear the fire message sent flag
				m_bFireMessageSent = false;

				// keep track of the total time disc is active
				m_fTimeDiscActive = 0.0f;
			}
		}

		// reset the ignore fire keyframe flag
		m_bIgnoreFireKeyframe = false;

		return true;
	}

	// if this point is reached the message was not
	// handled by the Disc, try another object
	return CTronClientWeapon::OnModelKey( hObj, pArgs );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::Update()
//
//	PURPOSE:	Override the main weapon update function.
//
// ----------------------------------------------------------------------- //

/// DEBUG CODE
bool g_bDisplayClientWeaponTarget = false;
/// DEBUG CODE

WeaponState CClientWeaponDisc::Update( bool bFire, bool bAltFire,
                                       FireType eFireType /*=FT_NORMAL_FIRE*/ )
{
	// RETAIL: no debug target point (NOLF2's g_bDisplayClientWeaponTarget)

	// 
	// See if we are disabled...If so don't allow any weapon stuff...
	if ( m_bDisabled )
	{
		return W_IDLE;
	}

	// RETAIL: 0x100194a0: while the fire key is held, and in a multiplayer
	// game no more often than DiscControlRate seconds
	if ( m_bFireMessageSent && bFire )
	{
		bool bSend = true;
		if ( IsMultiplayerGame() )
		{
			float fRate = g_vtDiscControlRate.GetFloat( 0.1f );
			if ( g_pLTClient->GetTime() < fRate + m_fTimeDiscActive )
			{
				bSend = false;
			}
		}

		if ( bSend )
		{
			CAutoMessage cMsg;

			m_fTimeDiscActive = g_pLTClient->GetTime();

			// RETAIL: Tron's projectile messages are object messages to our
			// player (CTronPlayerObj::HandleMessageProjectile)
			cMsg.Writeuint8( MID_OBJECT_MESSAGE );
			cMsg.WriteObject( g_pMoveMgr->GetServerObject() );
			cMsg.Writeuint32( MID_PROJECTILE );

			// write the projectile message subtype
			cMsg.Writeuint8( MPROJ_UPDATE_CONTROL_LINE );

			// pass a control vector, which is a vector pointing
			// straight out in front of the player
			LTVector vControlDirection;
			CalculateControlDirection( &vControlDirection );

			// get the control position (in this case, the camera position)
			LTVector vControlPosition;
			g_pLTClient->GetObjectPos( g_pPlayerMgr->GetCamera(), &vControlPosition );

			// write the control vector
			cMsg.WriteLTVector( vControlDirection );

			// write the control position
			cMsg.WriteLTVector( vControlPosition );

			// send the message
			g_pLTClient->SendToServer( cMsg.Read(), IsMultiplayerGame() ? 0 : MESSAGE_GUARANTEED );
		}
	}

	return CTronClientWeapon::Update( bFire, bAltFire, eFireType );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::PlayFireAnimation()
//
//	PURPOSE:	Set model to firing animation.  If the model has a PreFire animation
//				we will play that first and then play the Fire animation.  If the model
//				has a PostFire animation we will play that as soon as the Fire ani is done. 
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::PlayFireAnimation( bool bResetAni )
{
	// bResetAni is equivalant to saying "the fire key is down"

	// Can only set the last fire type if a fire animation isn't playing
	// (i.e., we'll assume this function will return false)...
	uint32 dwAni    = g_pLTClient->GetModelAnimation( m_hObject );
	uint32 dwState  = g_pLTClient->GetModelPlaybackState( m_hObject );

	bool bIsFireAni = IsFireAni( dwAni );

	uint32 dwPreFireAni = GetPreFireAni();
	uint32 dwPostFireAni = GetPostFireAni();

	if ( !bIsFireAni )
	{
		// If we have a pre-fire animation, play it before the fire animation...
		if( ( INVALID_ANI != dwPreFireAni ) && !bIsFireAni )
		{
			PlayAnimation( dwPreFireAni );
			return true;
		}
	}
	else
	{
		if ( ( dwPreFireAni == dwAni ) )
		{
			// Since the disc firing sequence and the related variables
			// to esure we have a boomerang style weapon are pretty
			// sensitive to variation, force the entire prefire animation
			// to finish completely, the queue either a fire or a post
			// fire animation depending on what the user wants to do.

			if ( dwState & MS_PLAYDONE )
			{
				//
				// There are 2 ways to fire a disc.  Either click-to-fire
				// click-to-return, or hold-to-fire-release-to-return.
				// The logic here supports both versions.  If the user
				// is pressing the fire button when the fire key is
				// received, we are considering that hold-to-fire.  If
				// not, it is considered click-to-fire.
				//

				// if bResetAni is true, the user wants to fire
				if ( bResetAni || m_bIsDiscActive )
				{
					// we are done playing the prefire animation, play the
					// fire animation as looping
					uint32 dwFireAni = GetFireAni( m_eLastFireType );
					if ( INVALID_ANI == dwFireAni )
					{
						// no valid fire animation
						ASSERT( !"no valid fire animation" );
						return false;
					}

					PlayAnimation( dwFireAni, true, 1.0f, true );
				}
				else
				{
					//
					// the user dosen't want to fire
					//

					if ( !m_bIsDiscActive )
					{
						if ( INVALID_ANI == dwPostFireAni )
						{
							// no valid fire animation
							return false;
						}

						PlayAnimation( dwPostFireAni );
					}
				}
			}
			// else, don't let anything happen until the prefire is done
		}
		else if ( ( dwPostFireAni == dwAni ) && ( dwState & MS_PLAYDONE ) )
		{
			// postfire animation is finished,
			// handle the ending
			DoSpecialEndFire();

			if ( bResetAni )
			{
				// the user wants to start throwing again...NOW!
				// Without this here, there is no way for the weapon
				// to start firing again until the user lets the
				// mouse button up.

				// RETAIL: Tron tells the server (0x100198b7). PORT: before
				// it retail calls CTronClientWeapon's 0x10097f20, not in
				// our build
				PlayAnimation( dwPreFireAni, true, 1.0f, false );
				SendDiscFired();
				return true;
			}

			// no special animation playing
			return false;
		}
		else if ( ( dwPostFireAni != dwAni ) && 
		          ( !m_bIsDiscActive ) )
		{
			// play the postfire animation immediately
			if ( INVALID_ANI == dwPostFireAni )
			{
				// no valid postfire animation
				ASSERT( !"no valid postfire animation" );
				return false;
			}

			PlayAnimation( dwPostFireAni );
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::PlayHoldDefenseAnimation()
//
//	PURPOSE:	Set the model to the hold defense animation
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::PlayHoldDefenseAnimation( bool bResetAni )
{
	// RETAIL: 0x10019990 (slot 101). Tron's tests the four hold animations
	// first; bResetAni is not used.

	// get thde current animation and the current animation state
	uint32 dwAni    = g_pLTClient->GetModelAnimation( m_hObject );
	uint32 dwState  = g_pLTClient->GetModelPlaybackState( m_hObject );

	bool bPreHold	= ( GetPreHoldDefenseAni() == dwAni );
	bool bHold		= ( GetHoldDefenseAni() == dwAni );
	bool bImpact	= ( GetImpactHoldDefenseAni() == dwAni );
	bool bPostHold	= ( GetPostHoldDefenseAni() == dwAni );

	if ( ( bPreHold || bHold ) && m_bPlayImpactHoldAnimation )
	{
		// play the impact hold animation
		PlayAnimation( GetImpactHoldDefenseAni(), true, 1.0f, false );
		return true;
	}

	if ( bPreHold )
	{
		if ( dwState & MS_PLAYDONE )
		{
			PlayAnimation( GetHoldDefenseAni(), false, 1.0f, false );

			if ( !IsMultiplayerGame() )
			{
				// let the server know
				SendDefendMessage( MPROJ_START_HOLD_BLOCK, GetPreHoldDefenseAni() );

				// turn on accurate rotations on the server
				g_pPlayerMgr->StartServerAccurateRotation();

				// start sending the camera offset to the server
				g_pPlayerMgr->StartSendingCameraOffsetToServer();
			}
		}
	}
	else if ( bHold )
	{
		if ( dwState & MS_PLAYDONE )
		{
			// play the hold "release" aniamtion
			PlayAnimation( GetPostHoldDefenseAni(), true, 1.0f, false );

			// let the server know
			SendDefendMessage( MPROJ_END_HOLD_BLOCK, GetPostHoldDefenseAni() );

			// turn off accurate rotations on the server
			g_pPlayerMgr->EndServerAccurateRotation();

			// stop sending the camera offset to the server
			g_pPlayerMgr->EndSendingCameraOffsetToServer();
		}
	}
	else if ( bImpact )
	{
		if ( dwState & MS_PLAYDONE )
		{
			// play the hold "release" aniamtion
			PlayAnimation( GetPostHoldDefenseAni(), true, 1.0f, false );

			// let the server know
			SendDefendMessage( MPROJ_END_HOLD_BLOCK, GetPostHoldDefenseAni() );

			// turn off accurate rotations on the server
			g_pPlayerMgr->EndServerAccurateRotation();

			// stop sending the camera offset to the server
			g_pPlayerMgr->EndSendingCameraOffsetToServer();
		}
	}
	else if ( bPostHold )
	{
		if ( dwState & MS_PLAYDONE )
		{
			// all done!
			return false;
		}
	}
	else
	{
		// no hold animation playing, start the hold
		PlayAnimation( GetPreHoldDefenseAni(), false, 1.0f, false );

		if ( IsMultiplayerGame() )
		{
			// let the server know
			SendDefendMessage( MPROJ_START_HOLD_BLOCK, GetPreHoldDefenseAni() );

			// turn on accurate rotations on the server
			g_pPlayerMgr->StartServerAccurateRotation();

			// start sending the camera offset to the server
			g_pPlayerMgr->StartSendingCameraOffsetToServer();
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::AddExtraFireMessageInfo
//
//	PURPOSE:	Add disc firing information to the fire message
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::AddExtraFireMessageInfo( bool bFire, ILTMessage_Write *pMsg )
{
	ASSERT( pMsg );

	// get the projectile structure
	PROJECTILEFX const *pProjectileFX = m_pAmmo->pProjectileFX;
	ASSERT( 0 != pProjectileFX );

	// get the projectile class data
	DISCCLASSDATA *pDiscClassData =
		dynamic_cast< DISCCLASSDATA* >( pProjectileFX->pClassData );
	ASSERT( 0 != pDiscClassData );

	if ( bFire )
	{
		//
		// The player uses the control line method of guiding the disc.
		//

		// find the point we're aiming from
		LTVector vTargetStart;
		// get the camera
		HOBJECT hCamera = g_pPlayerMgr->GetCamera();
		ASSERT( 0 != hCamera );
		// get the camera's position, its the fire position
		g_pLTClient->GetObjectPos( hCamera, &vTargetStart );

		// find the point we're aiming at
	//	LTVector vTargetPoint;
	//	bResult = CalculatePointGuidedTarget( &vTargetPoint );
	//	ASSERT( true == bResult );

		// determine the angle offset (angle from center to release the disc)
		LTFLOAT fDiscReleaseAngle;
		fDiscReleaseAngle = InterpolateRating<LTFLOAT>(
				Rating_Accuracy,
				pDiscClassData->fInitialAngleMin,
				pDiscClassData->fInitialAngleMax,
				pDiscClassData->fInitialAngleSurge
			);

		// pass a control vector, which is a vector pointing
		// straight out in front of the player
		LTVector vControlDirection;
		CalculateControlDirection( &vControlDirection );

		// write the angle offset
		pMsg->Writefloat( fDiscReleaseAngle );

		// write the desired kind of disc tracking
		pMsg->Writeuint8(
				static_cast< uint8 >( MPROJ_DISC_TRACKING_CONTROL_LINE )
			);

		// write the control vector
		pMsg->WriteLTVector( vControlDirection );

		// write the control position (in this case, the camera)
		pMsg->WriteLTVector( vTargetStart );
	}
	else
	{
		//
		// The player is shooting the disc that flys straight with no control.
		//

		// write the angle offset
		pMsg->Writefloat( 0.0f );

		// write the desired kind of disc tracking
		pMsg->Writeuint8(
				static_cast< uint8 >( MPROJ_DISC_TRACKING_STEADY )
			);
	}

	// write if this is a cluster disc or not (blah)
	pMsg->Writeuint8( static_cast< uint8 >( pDiscClassData->bClusterDiscInfo ) );

	// the fire message will be sent shortly after this, so
	// set the flag indicating we've send the fire message
	m_bFireMessageSent = true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::SendDefendMessage()
//
//	PURPOSE:	Send a message telling the server the swat defense is starting
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::SendDefendMessage( uint8 cBlockMsgType, uint32 nAni ) const
{
	LTRESULT ltResult;
	CAutoMessage cMsg;

	// RETAIL: an object message to our player (retail 0x10097da0)
	cMsg.Writeuint8( MID_OBJECT_MESSAGE );
	cMsg.WriteObject( g_pMoveMgr->GetServerObject() );
	cMsg.Writeuint32( MID_PROJECTILE );

	// write the projectile subtype
	cMsg.Writeuint8( cBlockMsgType );

	// write the weapon type for server validation
	// RETAIL: the weapon id (0x10097e43 reads +0x28), not the ammo id
	cMsg.Writeuint8( m_nWeaponId );

	// write the current timestamp (in milliseconds)
	cMsg.Writeuint32(
		static_cast< int >( g_pLTClient->GetTime() * 1000.0f )
	);

	// get the swat defenese animation length
	uint32 nLength;
	ltResult =
		g_pModelLT->GetAnimLength(
			m_hObject,
			nAni,
			nLength
		);
	ASSERT( LT_OK == ltResult );

	// write the length
	cMsg.Writeuint32( nLength );

	// send the message
	ltResult =
		g_pLTClient->SendToServer(
			cMsg.Read(),
			MESSAGE_GUARANTEED
		);
	ASSERT( LT_OK == ltResult );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::CalculatePointGuidedTarget()
//
//	PURPOSE:	Shoot a line through the camera and return first valid point
//				it hits
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::CalculatePointGuidedTarget( LTVector *pvTargetPoint )
{
	//
	// safety check
	//

	// params
	ASSERT( 0 != pvTargetPoint );

	// globals
	ASSERT( 0 != g_pLTClient );
	ASSERT( 0 != g_pPlayerMgr );


	//
	// Cast a ray from the camera to determine where
	// the point guided weapon's target is.
	//

	// get the camera
	HOBJECT hCamera;
	hCamera = g_pPlayerMgr->GetCamera();

	// determine the start point of the segment
	LTVector vStartPos;
	g_pLTClient->GetObjectPos( hCamera, &vStartPos );

	// determine the orientation of the segment
	LTRotation rRot;
	g_pLTClient->GetObjectRotation( hCamera, &rRot );

	// determine the direction of the segment
	LTVector vForward;
	vForward = rRot.Forward();

	// scale the direction to get the distance we're going to check
	LTVector vDistance;
	VEC_MULSCALAR( vDistance, vForward, CLIENTWEAPONDISC_PROBE_LENGTH );

	// determine the end point of the segment
	LTVector vEndPos;
	VEC_ADD( vEndPos, vStartPos, vDistance );

	// setup the intersect segment
	IntersectInfo iInfo;
	IntersectQuery qInfo;
	qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

	// DON'T collide with these objects
	HOBJECT hFilterList[] = { 
			g_pLTClient->GetClientObject()  // the player
			, g_pPlayerMgr->GetMoveMgr()->GetObject()  // the player
			, LTNULL
		};

	// callback function when we hit something
	// ObjListFilterFn: just filter out objects specified in above list,
	// otherwise register a hit
	qInfo.m_FilterFn  = ObjListFilterFn;  
	qInfo.m_pUserData = hFilterList;

	// end points
	qInfo.m_From = vStartPos;
	qInfo.m_To = vEndPos;

	// do the intersect segment
	LTBOOL bResult;
	bResult = g_pLTClient->IntersectSegment( &qInfo, &iInfo );

	// if we hit something, set the point
	if ( bResult )
	{
		*pvTargetPoint = iInfo.m_Point;
	}

	return ( LTTRUE == bResult );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::CalculateControlDirection()
//
//	PURPOSE:	Calculate a vector to send to the disc to influence its control
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::CalculateControlDirection( LTVector *pvControlDirection )
{
	//
	// safety check
	//

	// params
	ASSERT( 0 != pvControlDirection );

	// globals
	ASSERT( 0 != g_pLTClient );
	ASSERT( 0 != g_pPlayerMgr );


	//
	// The control direction is the direction the player is facing
	//

	// get the camera
	HOBJECT hCamera;
	hCamera = g_pPlayerMgr->GetCamera();
	ASSERT( 0 != hCamera );

	// determine the orientation of the segment
	LTRotation rRot;
	g_pLTClient->GetObjectRotation( hCamera, &rRot );

	// determine the direction of the segment
	LTVector vForward;
	*pvControlDirection = rRot.Forward();

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::Fire()
//
//	PURPOSE:	Fire the weapon
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::Fire( bool bFire )
{
	//
	// update disc specific variables
	//

	// keep track locally that the disc has been thrown
	m_bIsDiscActive = true;

	// keep sending updates to the disc until it says stop
	// or we get it back
	m_bDiscNeedsUpdates = true;

	// now call the the parent's version
	CTronClientWeapon::Fire( bFire );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::TronUnknownVirtual198()
//
//	PURPOSE:	RETAIL: 0x10019e00 (slot 102; the name is ours): tell the
//				server where the disc is and where it points
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::TronUnknownVirtual198()
{
	LTVector vU, vR, vF, vFirePos;
	if ( !GetFireVectors( &vU, &vR, &vF, &vFirePos ) )
	{
		return;
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_DISC_HOLD_DEFENSE );
	cMsg.WriteLTVector( vFirePos );
	cMsg.WriteLTVector( vF );
	cMsg.Writefloat( g_pLTClient->GetTime() );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::HandleFireKeyDownTransition()
//
//	PURPOSE:	What do do when the fire key goes down
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::HandleFireKeyDownTransition()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::HandleFireKeyUpTransition()
//
//	PURPOSE:	What to do when the fire key goes up
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::HandleFireKeyUpTransition()
{
	//
	// There are 2 ways to fire a disc.  Either click-to-fire
	// click-to-return, or hold-to-fire-release-to-return.
	// The logic here supports both versions.  If the user
	// is pressing the fire button when the fire key is
	// received, we are considering that hold-to-fire.  If
	// not, it is considered click-to-fire.
	//

	// RETAIL: 0x10019f30 tests and clears the bools at +0x21e and +0x21d
	// (which retail bool is which of ours is a guess, see the header), and
	// leaves the message to SendDiscReturn
	if ( m_bFireMessageSent )
	{
		m_bIsDiscActive = false;
		m_bFireMessageSent = false;

		// tell the disc the fire key is up
		SendDiscRelease();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::TronUnknownVirtualF8()
//
//	PURPOSE:	RETAIL: 0x10019f50 (slot 62): the disc returns
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::TronUnknownVirtualF8()
{
	SendDiscReturn();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::HandleFireKeyDown()
//
//	PURPOSE:	What to do while the fire key is continually pressed
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::HandleFireKeyDown()
{
	// 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::HandleFireKeyUp()
//
//	PURPOSE:	What to do when the fire key goes up
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::HandleFireKeyUp()
{
	// 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::DoSpecialEndFire()
//
//	PURPOSE:	What to do when the fire key goes up
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::DoSpecialEndFire()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::IncrementAmmo
//
//	PURPOSE:	Increment the weapon's ammo count
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::IncrementAmmo( int nAmount /*=1*/ )
{
	int nAmmo;
	bool bInfiniteAmmo = ( g_bInfiniteAmmo || ( !!( m_pWeapon->bInfiniteAmmo ) ) );
	if ( bInfiniteAmmo )
	{
		nAmmo = INFINITE_AMMO_AMOUNT;
	}
	else
	{
		nAmmo = g_pPlayerStats->GetAmmoCount( m_nAmmoId );
	}

	// add the specified amount of ammo
	nAmmo = nAmmo + nAmount;

	// Update our stats.  This will ensure that our stats are always
	// accurate (even in multiplayer).  This call will also ensure
	// it doesn't go over the max amount.
	g_pPlayerStats->UpdateAmmo( m_nWeaponId,
	                            m_nAmmoId,
	                            nAmmo,
	                            LTFALSE,
	                            LTFALSE );

	// check we have more ammo than we did a moment ago
	int nNewAmmoAmount = g_pPlayerStats->GetAmmoCount( m_nAmmoId );

	// if we have more ammo than before and the clip is not full
	// RETAIL: Tron's test is <= (0x10019fc6)
	if ( ( nAmmo <= nNewAmmoAmount ) &&
	     ( m_nAmmoInClip < m_pWeapon->nShotsPerClip ) )
	{
		// add ammo to the clip
		m_nAmmoInClip += nAmount;

		// make sure we don't go beyond the maximum
		if ( m_nAmmoInClip > m_pWeapon->nShotsPerClip )
		{
			m_nAmmoInClip = m_pWeapon->nShotsPerClip;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::SendDiscReturn()
//
//	PURPOSE:	RETAIL: 0x10019ff0 (the name is ours): tell the disc to
//				return, through an object message to our player
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::SendDiscReturn()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_OBJECT_MESSAGE );
	cMsg.WriteObject( g_pMoveMgr->GetServerObject() );
	cMsg.Writeuint32( MID_PROJECTILE );
	cMsg.Writeuint8( MPROJ_RETURN );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::SendDiscRelease()
//
//	PURPOSE:	RETAIL: 0x1001a0f0 (the name is ours): tell the disc that
//				the fire key is up, through an object message to our player
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::SendDiscRelease()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_OBJECT_MESSAGE );
	cMsg.WriteObject( g_pMoveMgr->GetServerObject() );
	cMsg.Writeuint32( MID_PROJECTILE );
	cMsg.Writeuint8( MPROJ_DISC_TRONUNKNOWN14 );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::Load() / Save()
//
//	PURPOSE:	RETAIL: 0x10019690 / 0x10019710 (retail only)
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::Load( ILTMessage_Read *pMsg )
{
	m_bPlayImpactHoldAnimation = pMsg->Readbool();
	m_bIsDiscActive = pMsg->Readbool();
	m_bFireMessageSent = pMsg->Readbool();
	m_bDiscNeedsUpdates = pMsg->Readbool();
	m_bIgnoreFireKeyframe = pMsg->Readbool();
	CTronClientWeapon::Load( pMsg );
}

void CClientWeaponDisc::Save( ILTMessage_Write *pMsg )
{
	pMsg->Writebool( m_bPlayImpactHoldAnimation );
	pMsg->Writebool( m_bIsDiscActive );
	pMsg->Writebool( m_bFireMessageSent );
	pMsg->Writebool( m_bDiscNeedsUpdates );
	pMsg->Writebool( m_bIgnoreFireKeyframe );
	CTronClientWeapon::Save( pMsg );
}

// RETAIL: 0x1001a1f0
bool CClientWeaponDisc::TronUnknownVirtual19C()
{
	return 0 < g_pPlayerStats->GetAmmoCount( m_nAmmoId );
}

// RETAIL: 0x1001a210
bool CClientWeaponDisc::TronUnknownVirtual1A0()
{
	return g_pPlayerStats->GetAmmoCount( m_nAmmoId ) != (uint32)m_pAmmo->GetMaxAmount( LTNULL );
}

// RETAIL: 0x1001a240.  PORT: retail's DISCCLASSDATA has its swat defend FX
// names 8 bytes further on (+0xa4, +0xd4) than ours.
void CClientWeaponDisc::CreateDefendFX( int nDefendType )
{
	if ( !m_pAmmo->pProjectileFX )
	{
		return;
	}

	DISCCLASSDATA *pDiscData = (DISCCLASSDATA*)m_pAmmo->pProjectileFX->pClassData;
	if ( !pDiscData )
	{
		return;
	}

	char *szFXName;
	if ( 3 == nDefendType )
	{
		szFXName = pDiscData->szSwatCriticalDefendPVFXName;
	}
	else if ( 2 == nDefendType )
	{
		szFXName = pDiscData->szSwatDefendPVFXName;
	}
	else
	{
		return;
	}

	if ( !szFXName || !szFXName[0] )
	{
		return;
	}

	CLIENTFX_CREATESTRUCT fxInit( szFXName, FXFLAG_REALLYCLOSE, m_hObject );
	g_pClientFXMgr->CreateClientFX( LTNULL, fxInit, LTTRUE );
}

// RETAIL: 0x1001a320
void CClientWeaponDisc::ShowDisc()
{
	HMODELPIECE hPiece = 0;
	ILTModel *pModelLT = g_pLTClient->GetModelLT();
	if ( LT_OK == pModelLT->GetPiece( m_hObject, "Disk", hPiece ) )
	{
		pModelLT->SetPieceHideStatus( m_hObject, hPiece, LTFALSE );
	}
}

// RETAIL: 0x1001a370
void CClientWeaponDisc::SendDiscFired()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_TRON_DISC_FIRED );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
}
