/****************************************************************************
;
;	MODULE:			TronPlayerObj.cpp
;
;	PURPOSE:		Tron-specific player object
;
;	HISTORY:		1/28/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/

#include "stdafx.h"
#include "TronPlayerObj.h"
#include "ContinuousFireHelper.h"
#include "InventoryButeMgr.h"
#include "WeaponFireInfo.h"
#include "Attachments.h"
#include "ParsedMsg.h"
#include "Weapon.h"
#include "FXButeMgr.h"
#include "LightCycleMgr.h"
#include "ServerLightCycleMgr.h"
#include "LightCycleMatchMgr.h"
#include "TronGameServerShell.h"
#include "LightCycle.h"
#include "LightCycleButeMgr.h"
#include "LightCycleGameData.h"
#include "ServerMissionMgr.h"
#include "ArenaStartPoint.h"
#include "MultiplayerLevelInfoMgr.h"
#include "PingButeMgr.h"
#include "AIUtils.h"

BEGIN_CLASS(CTronPlayerObj)
END_CLASS_DEFAULT_FLAGS(CTronPlayerObj, CPlayerObj, NULL, NULL, CF_HIDDEN)

#define TRIGGER_ACQUIRE_SUBROUTINE				"ACQUIRESUB"
#define TRIGGER_ACQUIRE_PROCEDURAL				"ACQUIREPROC"
#define TRIGGER_ACQUIRE_PRIMITIVE				"ACQUIREPRIM"
#define TRIGGER_ADD_BUILD_POINTS				"ADDBUILDPOINTS"
#define TRIGGER_START_LIGHTCYCLE				"STARTLIGHTCYCLE"
#define TRIGGER_END_LIGHTCYCLE					"ENDLIGHTCYCLE"

// RETAIL: Tron's messages (retail 0x100623a0)
CMDMGR_BEGIN_REGISTER_CLASS( CTronPlayerObj )
	CMDMGR_ADD_MSG( ACQUIRESUB,		-1,		NULL,			"ACQUIRESUB <Subroutine Name> [state] [condition]")
	CMDMGR_ADD_MSG( ACQUIREPROC,	2,		NULL,			"ACQUIREPROC <Procedural Name>")
	CMDMGR_ADD_MSG( ACQUIREPRIM,	2,		NULL,			"ACQUIREPRIM <Primitive Name>")
	CMDMGR_ADD_MSG( ACQUIREPSET,	2,		NULL,			"ACQUIREPSET <permission set number | all>")
	CMDMGR_ADD_MSG( ACQUIRENOTE,	2,		NULL,			"ACQUIRENOTE <Note Name>")
	CMDMGR_ADD_MSG( ACQUIREPOWERUP,	2,		NULL,			"ACQUIREPOWERUP")
	CMDMGR_ADD_MSG_ARG_RANGE( ADDPING,	2,	3,	NULL,		"ADDPING <Ping Name> <1/0 (Play sound or not, defaults to 1>")
	CMDMGR_ADD_MSG( REMOVEPING,		2,		NULL,			"REMOVEPING <Ping Name>")
	CMDMGR_ADD_MSG( IOCAM,			-1,		NULL,			"IOCAM <on/off> [Location] [Target]")
	CMDMGR_ADD_MSG( TURN,			2,		NULL,			"TURN <left/right>")
	CMDMGR_ADD_MSG( SINCONSOLEVAR,	-1,		NULL,			"SINCONSOLEVAR <cvar> {off | <min> <max> <period>}")
	CMDMGR_ADD_MSG( RAMPCONSOLEVAR,	-1,		NULL,			"RAMPCONSOLEVAR <cvar> [start] <end> <period>")
	CMDMGR_ADD_MSG( ADDBUILDPOINTS,	2,		NULL,			"ADDBUILDPOINTS <number to add>")
	CMDMGR_ADD_MSG( LOCKWEAPON,		1,		NULL,			"LOCKWEAPON")
	CMDMGR_ADD_MSG( UNLOCKWEAPON,	1,		NULL,			"UNLOCKWEAPON")
CMDMGR_END_REGISTER_CLASS( CTronPlayerObj, CPlayerObj )

// Subroutine states
char *g_SubStates[] =
{
	"ALPHA",
	"BETA",
	"GOLD"
};
int g_nNumSubStates = (sizeof(g_SubStates) / sizeof(char *));

// Subroutine conditions (subject to change)
char *g_SubConditions[] =
{
	"NORMAL",
	"INFECTED",
	"NONCOMPATIBLE"
};
int g_nNumSubConditions = (sizeof(g_SubConditions) / sizeof(char *));


namespace
{
	int8 const TRONPLAYEROBJ_NO_DEFEND = -1;
	float const TRONPLAYEROBJ_CLIENT_CAMERA_TIME_OLD_THRESHOLD = 1.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::CTronPlayerObj()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

CTronPlayerObj::CTronPlayerObj()
	: CPlayerObj()
	, m_byOldPSets( 0 )
	, m_nBuildPoints( 0 )
	, m_nOldBuildPoints( 100 )
	, m_fTronUnknown31f0( 0.0f )
	, m_fTronUnknown31f4( 0.0f )
	, m_fTronUnknown31f8( 0.0f )
	, m_fTronUnknown31fc( 0.0f )
	, m_nDefendClientTimeStarted( 0 )
	, m_nDefendServerTimeStarted( 0 )
	, m_nDefendDuration( 0 )
	, m_cDefendType( TRONPLAYEROBJ_NO_DEFEND )
	, m_cDefendAmmoId( WMGR_INVALID_ID )
	, m_bTronUnknown325c( false )
	, m_pSwatWeapon( LTNULL )
	, m_pSwatWeapons( LTNULL )
	, m_nLastWeaponId( 1 )
	, m_nLastAmmoId( 1 )
{
	// RETAIL: retail 0x100626d0
	m_iPermissionSet = 0;

	m_hLightCycle = LTNULL;
	strcpy( m_szLightCycleModel, "LightCycleBlue" );

	m_fOldMaxHitPoints = -1.0f;
	m_fOldMaxEnergy = -1.0f;

	memset( m_iPerformanceRatings, 0, sizeof( m_iPerformanceRatings ));
	memset( m_iSurgePerformanceRatings, 0, sizeof( m_iSurgePerformanceRatings ));
	memset( m_szTronUnknown3267, 0, sizeof( m_szTronUnknown3267 ));
	memset( m_szTronUnknown32a7, 0, sizeof( m_szTronUnknown32a7 ));

	if( g_pGameServerShell && !IsMultiplayerGame( ))
	{
		m_bMakeBody = false;
	}

	m_afChanceToBeCorruptedByInfectedAmmo[0] = g_pServerButeMgr->GetPlayerAttributeFloat( "InventoryChanceToBeCorruptedByInfectedAmmoEasy" );
	m_afChanceToBeCorruptedByInfectedAmmo[1] = g_pServerButeMgr->GetPlayerAttributeFloat( "InventoryChanceToBeCorruptedByInfectedAmmoNormal" );
	m_afChanceToBeCorruptedByInfectedAmmo[2] = g_pServerButeMgr->GetPlayerAttributeFloat( "InventoryChanceToBeCorruptedByInfectedAmmoHard" );
	m_afChanceToBeCorruptedByInfectedAmmo[3] = g_pServerButeMgr->GetPlayerAttributeFloat( "InventoryChanceToBeCorruptedByInfectedAmmoVeryHard" );

	m_bTronUnknown32f8 = false;
	m_bTronUnknown32f9 = false;
	m_bTronUnknown32fa = false;
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::~CTronPlayerObj()
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

CTronPlayerObj::~CTronPlayerObj()
{
	// RETAIL: retail 0x10062a30
	if( m_pSwatWeapons )
	{
		debug_delete( m_pSwatWeapons );
	}

	// RETAIL: retail's light cycle manager is called through its vtable
	// (slot 4, with the player's object); ours is Build 69's
	if( g_pLightCycleMgr )
	{
		g_pLightCycleMgr->RemoveCyclist( m_hObject );
	}

	RemoveVehicleModel( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::ObjectMessageFn()
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

// RETAIL: Tron's object messages that the player reads (0x10062b60; the
// names are ours)
#define MID_UPDATE_UTILITY_SUBROUTINE		137
#define MID_RESET_HIT_POINTS_AND_ENERGY		141
#define MID_TRONUNKNOWN_143					143

uint32 CTronPlayerObj::ObjectMessageFn( HOBJECT hSender,
                                        ILTMessage_Read *pMsg )
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch ( messageID )
	{
		case MID_PROJECTILE:
		{
			bool bMessageHandled;
			bMessageHandled =
					HandleMessageProjectile( hSender, pMsg );
			if ( bMessageHandled )
			{
				return 0;
			}
		}
		break;

		case MID_UPDATE_UTILITY_SUBROUTINE:
		{
			// a utility subroutine's state and condition
			char szName[64];
			pMsg->ReadString( szName, ARRAY_LEN( szName ));

			PlayerSubroutineSlot* pSlot = FindSubroutine( szName );
			if( pSlot )
			{
				pSlot->nTronUnknown40 = ( pMsg->Readbool() );
				pSlot->nTronUnknown41 = ( pMsg->Readbool() );
				pSlot->nTronUnknown42 = pMsg->Readuint8();
				return 0;
			}

			g_pLTServer->CPrint( "ERROR - Could not update utility subroutine (%s) because it wasn't defined when the player was initialized!\n", szName );
			return 0;
		}
		break;

		case MID_RESET_HIT_POINTS_AND_ENERGY:
		{
			m_damage.SetHitPoints( m_damage.GetMaxHitPoints() );
			m_damage.SetEnergy( m_damage.GetMaxEnergy() );
			return 0;
		}
		break;

		case MID_TRONUNKNOWN_143:
		{
			// RETAIL: 0x10062c5a
			HandleFootstep( true );
			return 0;
		}
		break;
	}

	return CPlayerObj::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::HandleMessageProjectile()
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

bool CTronPlayerObj::HandleMessageProjectile( HOBJECT hSender,
                                              ILTMessage_Read *pMsg )
{
	// RETAIL: 0x10062ca0.  Tron's handles the hold block only (its swat
	// and arm blocks go on to the weapon) and tells the client of a
	// blocked disc

	// get the projectile message's subtype
	uint8 nProjectileMessageSubType = pMsg->Readuint8();

	switch ( nProjectileMessageSubType )
	{
		//case MPROJ_RETURNED:
		// the place where the ammo gets incremented is in CWeapons

		case MPROJ_START_HOLD_BLOCK:
		{
			// get the rest of the info

			// get the weapon id
			uint8 cAmmoId = pMsg->Readuint8();

			// get the timestamp
			uint32 nDefendTimestamp = pMsg->Readuint32();

			// get the animation length
			uint32 nDefendDuration = pMsg->Readuint32();

			// make sure the current weapon is the weapon
			// being blocked with
			// RETAIL: Tron compares the weapon id (CWeapon +0x40, 0x10062d63)
			CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon();
			if ( !pWeapon || ( pWeapon->GetId() != cAmmoId ) )
			{
				// Tut tut, no cheating!
				return true;
			}

			if ( TRONPLAYEROBJ_NO_DEFEND != m_cDefendType )
			{
				// we are already handling a block, check
				// if the new one is valid
				if ( ( m_nDefendClientTimeStarted + m_nDefendDuration ) >
					 static_cast< int >( nDefendTimestamp ) )
				{
					// Tut, tut, no cheating!
					return true;
				}
			}

			// we're ok with the new block, setup the info
			m_cDefendType = nProjectileMessageSubType;
			m_nDefendClientTimeStarted = nDefendTimestamp;
			m_nDefendServerTimeStarted =
				static_cast< int >(
					g_pLTServer->GetTime() * 1000.0f
				);
			m_nDefendDuration = nDefendDuration;
			m_cDefendAmmoId = cAmmoId;

			return true;
		}
		break;

		case MPROJ_END_HOLD_BLOCK:
		{
			// get the rest of the info

			// get the weapon id
			uint8 cAmmoId = pMsg->Readuint8();

			// get the timestamp
			uint32 nDefendTimestamp = pMsg->Readuint32();

			// get the animation length
			uint32 nDefendDuration = pMsg->Readuint32();

			// make sure the current weapon is the weapon
			// being blocked with
			// RETAIL: Tron compares the weapon id (CWeapon +0x40, 0x10062d63)
			CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon();
			if ( !pWeapon || ( pWeapon->GetId() != cAmmoId ) )
			{
				// Tut tut, no cheating!
				return true;
			}

			if ( MPROJ_START_HOLD_BLOCK != m_cDefendType )
			{
				// Woa!  We can't end the hold block until
				// after it starts!  Something is out of
				// order, or there's some cheating going on!
				return true;
			}

			// we're ok with the new block, setup the info
			m_cDefendType = nProjectileMessageSubType;
			m_nDefendClientTimeStarted = nDefendTimestamp;
			m_nDefendServerTimeStarted =
				static_cast< int >(
					g_pLTServer->GetTime() * 1000.0f
				);
			m_nDefendDuration = nDefendDuration;
			m_cDefendAmmoId = cAmmoId;

			return true;
		}
		break;

		case MPROJ_BLOCKED:
		{
			// tell the client that it blocked the disc
			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_PROJECTILE );
			cMsg.Writeuint8( MPROJ_BLOCKED );
			cMsg.Writeuint8( pMsg->Readuint8() );
			g_pLTServer->SendToClient( cMsg.Read(), m_hClient, MESSAGE_GUARANTEED );

			return true;
		}
		break;

		default:
		{
			// the message will be forwarded through the chain, eventually
			// to the CWeapon attachment where we want it to go

			// reset the message, we don't want to alter it
			pMsg->SeekTo(0);

			// return false, this did NOT handle the message
			return false;
		}
		break;
	};
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::StartContinuousFire
//
//	PURPOSE:	Start the fire of a continuous weapon (PRod, EnergyClaw)
//
// ----------------------------------------------------------------------- //

// RETAIL: Object.lto 0x10062fa0, CTronPlayerObj vtable slot 107 (CPlayerObj's
// is empty).  The names of the function and of the message IDs are ours.
void CTronPlayerObj::StartContinuousFire(ILTMessage_Read *pMsg)
{
	m_bTronUnknown325c = true;

	switch ((uint8)pMsg->PeekBits(8))
	{
		case kContinuousFire_PRod:
			m_pPRodFireHelper->Start(pMsg);
			break;

		case kContinuousFire_EnergyClaw:
			m_pEnergyClawFireHelper->Start(pMsg);
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::StopContinuousFire
//
//	PURPOSE:	Stop the fire of a continuous weapon
//
// ----------------------------------------------------------------------- //

// RETAIL: Object.lto 0x10062ff0, vtable slot 108.
void CTronPlayerObj::StopContinuousFire(ILTMessage_Read *pMsg)
{
	m_bTronUnknown325c = false;

	switch ((uint8)pMsg->ReadBits(8))
	{
		case kContinuousFire_PRod:
			m_pPRodFireHelper->Stop();
			break;

		case kContinuousFire_EnergyClaw:
			m_pEnergyClawFireHelper->Stop();
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::Save
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //
void CTronPlayerObj::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if(!pMsg)
		return;

	// RETAIL: retail 0x10063030
	SAVE_BYTE(m_byOldPSets);
	SAVE_FLOAT(m_fTronUnknown31f8);
	SAVE_FLOAT(m_fTronUnknown31fc);
	SAVE_WORD(m_nBuildPoints);
	SAVE_WORD(m_nOldBuildPoints);
	SAVE_FLOAT(m_fOldMaxHitPoints);
	SAVE_FLOAT(m_fOldMaxEnergy);
	SAVE_FLOAT(m_fTronUnknown31f0);
	SAVE_FLOAT(m_fTronUnknown31f4);
	SAVE_HOBJECT(m_hLightCycle);
	SAVE_CHARSTRING(m_szLightCycleModel);
	SAVE_bool(m_bTronUnknown325c);

	for(int i=0;i<NUM_RATINGS;i++)
	{
		SAVE_BYTE(m_iPerformanceRatings[i]);
		SAVE_BYTE(m_iSurgePerformanceRatings[i]);
	}

	SAVE_BYTE(m_lstPings.size());
	for( PingList::iterator iter = m_lstPings.begin(); iter != m_lstPings.end(); iter++ )
	{
		SAVE_CHARSTRING( (*iter).c_str() );
	}

	CPlayerObj::Save(pMsg, dwSaveFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //
void CTronPlayerObj::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	// RETAIL: retail 0x100631c0
	LOAD_BYTE(m_byOldPSets);
	LOAD_FLOAT(m_fTronUnknown31f8);
	LOAD_FLOAT(m_fTronUnknown31fc);
	LOAD_WORD(m_nBuildPoints);
	LOAD_WORD(m_nOldBuildPoints);
	LOAD_FLOAT(m_fOldMaxHitPoints);
	LOAD_FLOAT(m_fOldMaxEnergy);
	LOAD_FLOAT(m_fTronUnknown31f0);
	LOAD_FLOAT(m_fTronUnknown31f4);
	LOAD_HOBJECT(m_hLightCycle);
	LOAD_CHARSTRING(m_szLightCycleModel, ARRAY_LEN(m_szLightCycleModel));
	LOAD_bool(m_bTronUnknown325c);

	for(int i=0;i<NUM_RATINGS;i++)
	{
		LOAD_BYTE(m_iPerformanceRatings[i]);
		LOAD_BYTE(m_iSurgePerformanceRatings[i]);
	}

	m_lstPings.clear();
	uint8 nPings;
	LOAD_BYTE(nPings);
	for( int nPing = 0; nPing < nPings; nPing++ )
	{
		char szPing[128];
		LOAD_CHARSTRING( szPing, ARRAY_LEN( szPing ));
		AddPing( szPing, false );
	}

	m_bTronUnknown32f8 = false;
	m_bTronUnknown32f9 = false;
	m_bTronUnknown32fa = false;

	CPlayerObj::Load(pMsg, dwLoadFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::AddPing
//
//	PURPOSE:	Add a ping (RETAIL: retail 0x100675a0; the name is ours)
//
// ----------------------------------------------------------------------- //
void CTronPlayerObj::AddPing(char const* pszPing, bool bPlaySound)
{
	// RETAIL: retail 0x100675a0: a ping of pingbutes.txt; the client is
	// told the ping's character
	if( bPlaySound )
	{
		PlaySound( "AddPing", m_fSoundRadius, LTTRUE );
	}

	char szTag[30];
	g_pPingButeMgr->GetPingTag( pszPing, szTag, ARRAY_LEN( szTag ));
	if( !szTag[0] )
	{
		Warn( "%s not found in pingbutes.txt", pszPing );
		return;
	}

	m_lstPings.push_front( std::string( pszPing ));

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_TRON_PING );
	char szCharacter[128];
	g_pPingButeMgr->GetCharacter( szTag, szCharacter, ARRAY_LEN( szCharacter ));
	cMsg.WriteString( szCharacter );
	cMsg.Writeuint8( 1 );

	if( GetClient( ))
	{
		g_pLTServer->SendToClient( cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::RemovePing
//
//	PURPOSE:	RETAIL: retail 0x10067770 (the name is ours): "REMOVEPING"
//
// ----------------------------------------------------------------------- //

void CTronPlayerObj::RemovePing(char const* pszPing)
{
	char szTag[30];
	g_pPingButeMgr->GetPingTag( pszPing, szTag, ARRAY_LEN( szTag ));

	// RETAIL: 0x10067770 warns where the loop ends (its epilogue follows
	// the found ping's code; ours follows the warning)
	PingList::iterator iter;
	for( iter = m_lstPings.begin( ); iter != m_lstPings.end( ); iter++ )
	{
		if( !_stricmp( iter->c_str( ), pszPing ))
		{
			m_lstPings.erase( iter );

			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_TRON_PING );
			char szCharacter[128];
			g_pPingButeMgr->GetCharacter( szTag, szCharacter, ARRAY_LEN( szCharacter ));
			cMsg.WriteString( szCharacter );
			cMsg.Writeuint8( 0 );

			if( GetClient( ))
			{
				g_pLTServer->SendToClient( cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED );
			}

			return;
		}
	}

	Warn( "%s not found in pingbutes.txt", pszPing );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::HandleSinConsoleVarMsg/HandleRampConsoleVarMsg
//
//	PURPOSE:	RETAIL: retail 0x10067ce0, 0x10067f60 (the names are ours):
//				"SINCONSOLEVAR" and "RAMPCONSOLEVAR" tell the client to
//				move a console variable (MID_TRON_UNKNOWN_104: 0 ramp, 1
//				sine, 2 stop, 3 revert)
//
// ----------------------------------------------------------------------- //

bool CTronPlayerObj::HandleSinConsoleVarMsg(const CParsedMsg &cMsg)
{
	if( cMsg.GetArgCount( ) != 3 && cMsg.GetArgCount( ) != 5 )
		return false;

	uint8 nType = 1;
	float fMin = 0.0f;
	float fMax = 0.0f;
	float fPeriod = 1.0f;
	char szVar[64];

	strncpy( szVar, cMsg.GetArg( 1 ), ARRAY_LEN( szVar ) - 1 );
	szVar[ARRAY_LEN( szVar ) - 1] = 0;

	if( cMsg.GetArgCount( ) == 3 )
	{
		if( !_strcmpi( cMsg.GetArg( 2 ), "off" ) || !_strcmpi( cMsg.GetArg( 2 ), "stop" ))
		{
			nType = 2;
		}
		else if( !_strcmpi( cMsg.GetArg( 2 ), "revert" ) || !_strcmpi( cMsg.GetArg( 2 ), "restore" ))
		{
			nType = 3;
		}
		else
		{
			return false;
		}
	}
	else if( cMsg.GetArgCount( ) == 5 )
	{
		fMin = (float)atof( cMsg.GetArg( 2 ));
		fMax = (float)atof( cMsg.GetArg( 3 ));
		fPeriod = (float)atof( cMsg.GetArg( 4 ));
	}

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8( MID_TRON_UNKNOWN_104 );
	cClientMsg.Writeuint8( nType );
	cClientMsg.WriteString( szVar );
	cClientMsg.Writefloat( fMin );
	cClientMsg.Writefloat( fMax );
	cClientMsg.Writefloat( fPeriod );
	g_pLTServer->SendToClient( cClientMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED );

	return true;
}

bool CTronPlayerObj::HandleRampConsoleVarMsg(const CParsedMsg &cMsg)
{
	if( cMsg.GetArgCount( ) < 4 || cMsg.GetArgCount( ) > 5 )
		return false;

	char szVar[64];
	strncpy( szVar, cMsg.GetArg( 1 ), ARRAY_LEN( szVar ) - 1 );
	szVar[ARRAY_LEN( szVar ) - 1] = 0;

	float fStart = 0.0f;
	float fEnd;
	float fPeriod;
	bool bStart = false;

	if( cMsg.GetArgCount( ) == 5 )
	{
		bStart = true;
		fStart = (float)atof( cMsg.GetArg( 2 ));
		fEnd = (float)atof( cMsg.GetArg( 3 ));
		fPeriod = (float)atof( cMsg.GetArg( 4 ));
	}
	else
	{
		fEnd = (float)atof( cMsg.GetArg( 2 ));
		fPeriod = (float)atof( cMsg.GetArg( 3 ));
	}

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8( MID_TRON_UNKNOWN_104 );
	cClientMsg.Writeuint8( 0 );
	cClientMsg.WriteString( szVar );
	cClientMsg.Writeuint8( bStart ? 1 : 0 );
	cClientMsg.Writefloat( fStart );
	cClientMsg.Writefloat( fEnd );
	cClientMsg.Writefloat( fPeriod );
	g_pLTServer->SendToClient( cClientMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::HandleAnnouncementMsg/HandleRatingMsg
//
//	PURPOSE:	RETAIL: retail 0x10068250, 0x100683a0 (the names are ours):
//				"ANNOUNCEMENT <string id> [string id]" and "RATING <rating>
//				<0..100>"
//
// ----------------------------------------------------------------------- //

bool CTronPlayerObj::HandleAnnouncementMsg(const CParsedMsg &cMsg)
{
	if( cMsg.GetArgCount( ) < 2 )
		return false;

	uint32 nStringId = atol( cMsg.GetArg( 1 ));
	uint32 nStringId2 = 0;
	if( cMsg.GetArgCount( ) > 2 )
	{
		nStringId2 = atol( cMsg.GetArg( 2 ));
	}

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8( MID_TRON_HUD_TEXT );
	cClientMsg.Writeuint32( nStringId );
	cClientMsg.Writeuint32( nStringId2 );
	g_pLTServer->SendToClient( cClientMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED );

	return true;
}

bool CTronPlayerObj::HandleRatingMsg(const CParsedMsg &cMsg)
{
	if( cMsg.GetArgCount( ) < 3 )
		return false;

	const char* aszRatings[NUM_RATINGS] =
	{
		"health", "energy", "efficiency", "transfer", "processor"
	};

	const char* pszRating = cMsg.GetArg( 1 );

	int nRating = -1;
	for( int i = 0; i < NUM_RATINGS; i++ )
	{
		if( !_stricmp( pszRating, aszRatings[i] ))
		{
			nRating = i;
			break;
		}
	}

	if( nRating == -1 )
		return true;

	int nValue = atoi( cMsg.GetArg( 2 ));
	if( nValue < 0 || nValue > 100 )
		return true;

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8( MID_SET_PERFORMANCE_RATING );
	cClientMsg.Writeuint8( nRating );
	cClientMsg.Writeuint8( nValue );
	g_pLTServer->SendToClient( cClientMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CTronPlayerObj::GetExtraFireMessageInfo()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CTronPlayerObj::GetExtraFireMessageInfo( uint8 nWeaponType,
											  ILTMessage_Read *pMsg,
                                              WeaponFireInfo *pInfo )
{
	// RETAIL: 0x10063390.  Tron reads extra data for the disc, the ball, the
	// suffusion, the energy claw and the drunken dims; the other types
	// (vector, projectile, ...) have none.  The cluster disc's client also
	// writes its spread (CClientWeaponClusterDisc::AddExtraFireMessageInfo),
	// but this reads none of it.
	ASSERT( 0 != pInfo );

	switch ( nWeaponType )
	{
		case MWEAPFIRE_DISC:
		{
			// get the angle offset (from center) to fire disc
			pInfo->fDiscReleaseAngle = pMsg->Readfloat();

			// get the disc tracking type
			pInfo->nDiscTrackingType = pMsg->Readuint8();

			// RETAIL: Tron's: the player's auto aim setting; in a disc game
			// the server's option for the game type decides
			pInfo->bDiscAutoAim = pMsg->Readbool();
			if ( pInfo->bDiscAutoAim && IsDiscGameType() )
			{
				if ( eGameTypeDiscArena == g_pGameServerShell->GetServerGameOptions().m_eGameType )
				{
					pInfo->bDiscAutoAim = g_pGameServerShell->GetServerGameOptions().m_bArenaAllowAutoAim;
				}
				else
				{
					pInfo->bDiscAutoAim = g_pGameServerShell->GetServerGameOptions().m_bTournAllowAutoAim;
				}
			}

			switch ( pInfo->nDiscTrackingType )
			{
				case MPROJ_DISC_TRACKING_HOMING:
				{
					// get the first object point
					pInfo->hObjectTarget = pMsg->ReadObject();
				}
				break;

				case MPROJ_DISC_TRACKING_POINT_GUIDED:
				{
					// get the first target point
					pInfo->vPointTarget = pMsg->ReadLTVector();
				}
				break;

				case MPROJ_DISC_TRACKING_CONTROL_LINE:
				{
					// get the first control vector
					pInfo->vControlDirection = pMsg->ReadLTVector();

					// get the first target point
					pInfo->vControlPosition = pMsg->ReadLTVector();
				}
				break;

				default:
				{
					// An invalid DiscTracking type was encountered.
					// Check the construction of the message itself
					// (probably on the client).
					ASSERT( 0 );
				}
				break;
			};

		}
		break;

		case MWEAPFIRE_BALL:
		{
			// RETAIL: 0x100634f0: the ball's velocity
			pInfo->bOverrideVelocity = LTTRUE;
			pInfo->fOverrideVelocity = pMsg->Readfloat();
		}
		break;

		case MWEAPFIRE_SUFFUSION:
		{
			// RETAIL: 0x10063517
			pInfo->fDamageMultiplier = pMsg->Readfloat() + 1.0f;
			pInfo->nTronUnknown64 = pMsg->Readuint8();
		}
		break;

		case MWEAPFIRE_ENERGYCLAW:
		{
			// RETAIL: 0x10063555: the claw's range
			pInfo->bOverrideRange = LTTRUE;
			pInfo->fOverrideRange = pMsg->Readfloat();
		}
		break;

		case MWEAPFIRE_DRUNKENDIMS:
		{
			// RETAIL: 0x1006357a
			pInfo->nTronUnknown64 = pMsg->Readuint8();
		}
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::UpdateInterface
//
//	PURPOSE:	Tell the client of about any changes
//
// ----------------------------------------------------------------------- //
void CTronPlayerObj::UpdateInterface( bool bForceUpdate )
{
	// RETAIL: 0x100635c0
	if ( !m_pPlayerAttachments ) return;
	if (!m_hClient || !g_pWeaponMgr) return;

	// See if the psets have changed...
	if(m_iPermissionSet != m_byOldPSets || bForceUpdate)
	{
		m_byOldPSets = m_iPermissionSet;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_PSETS_ID);
		cMsg.Writeuint8(m_iPermissionSet);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(0.0f);
		g_pLTServer->SendToClient(cMsg.Read(), m_hClient, MESSAGE_GUARANTEED);
	}

	// See if the build points have changed
	if(m_nBuildPoints != m_nOldBuildPoints || bForceUpdate)
	{
		m_nOldBuildPoints = m_nBuildPoints;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_BUILD_POINTS_ID);
		cMsg.Writeuint8(0);
		cMsg.Writeuint8(0);
		cMsg.Writefloat((LTFLOAT)m_nBuildPoints);
		g_pLTServer->SendToClient(cMsg.Read(), m_hClient, MESSAGE_GUARANTEED);
	}

	// RETAIL: the light cycle follows the player
	if (m_hLightCycle)
	{
		LTVector vPos;
		LTRotation rRot;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		g_pLTServer->GetObjectRotation(m_hObject, &rRot);
		g_pLTServer->SetObjectPos(m_hLightCycle, &vPos);
		g_pLTServer->SetObjectRotation(m_hLightCycle, &rRot);
	}

	// See if the maximum health has changed
	if (m_damage.GetMaxHitPoints() != m_fOldMaxHitPoints || bForceUpdate)
	{
		m_fOldMaxHitPoints = m_damage.GetMaxHitPoints();

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_MAX_HEALTH_ID);
		cMsg.Writeuint8(0);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(m_fOldMaxHitPoints);
		g_pLTServer->SendToClient(cMsg.Read(), m_hClient, MESSAGE_GUARANTEED);
	}

	// See if the maximum energy has changed
	if (m_damage.GetMaxEnergy() != m_fOldMaxEnergy || bForceUpdate)
	{
		m_fOldMaxEnergy = m_damage.GetMaxEnergy();

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_MAX_ENERGY_ID);
		cMsg.Writeuint8(0);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(m_fOldMaxEnergy);
		g_pLTServer->SendToClient(cMsg.Read(), m_hClient, MESSAGE_GUARANTEED);
	}

	// Call the base class
	CPlayerObj::UpdateInterface(bForceUpdate);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::Update
//
//	PURPOSE:	Main update
//
// ----------------------------------------------------------------------- //
LTBOOL CTronPlayerObj::Update()
{
	// RETAIL: retail 0x10063a80
	// Drain (or charge) our energy...
	if( fabs( m_fTronUnknown31f0 ) > FLT_EPSILON )
	{
		float fEnergy = m_damage.GetEnergy( );
		fEnergy -= g_pLTServer->GetFrameTime( ) * m_fTronUnknown31f0;

		if( fEnergy < m_fTronUnknown31f4 )
		{
			fEnergy = m_fTronUnknown31f4;
		}
		else if( m_damage.GetMaxEnergy( ) < fEnergy )
		{
			fEnergy = m_damage.GetMaxEnergy( );
		}

		m_damage.SetEnergy( fEnergy );
	}

	if( m_bTronUnknown32fa )
	{
		EndLightCycle( );
	}

	if( m_bTronUnknown32f8 )
	{
		StartLightCycle( false, true );
	}
	else if( m_bTronUnknown32f9 )
	{
		StartLightCycle( true, true );
	}

	// Call down to the base class
	return(CPlayerObj::Update());
}

// RETAIL: a rating's share of its surge level makes a maximum between 100
// and 200 (300 once surging). The name is ours
static inline float GetRatedMaximum( uint8 nRating, uint8 nSurgeRating )
{
	float fRatio = (float)nRating / nSurgeRating;
	return ((fRatio >= 1.0f) ? 200.0f : fRatio * 100.0f) + 100.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::HandleRatings
//
//	PURPOSE:	The client's armor, ratings and energy floor (RETAIL:
//				retail 0x10063b50, called by the server shell's message
//				handler 0x1005ed20; the names are ours)
//
// ----------------------------------------------------------------------- //
void CTronPlayerObj::HandleRatings(ILTMessage_Read *pMsg)
{
	m_fTronUnknown31f8 = pMsg->Readfloat();
	m_fTronUnknown31fc = pMsg->Readfloat();

	for( int i = 0; i < NUM_RATINGS; i++ )
	{
		m_iPerformanceRatings[i] = pMsg->Readuint8();
		m_iSurgePerformanceRatings[i] = pMsg->Readuint8();
	}

	m_fTronUnknown31f4 = pMsg->Readfloat();

	// RETAIL: the health and energy ratings scale the maximums between 100
	// and 200 (300 once surging); the current values keep their share
	float fValue = m_damage.GetHitPoints();
	float fOldMax = m_damage.GetMaxHitPoints();
	float fNewMax = GetRatedMaximum( m_iPerformanceRatings[Rating_Health], m_iSurgePerformanceRatings[Rating_Health] );
	m_damage.SetMaxHitPoints( fNewMax );
	m_damage.SetHitPoints( fNewMax / fOldMax * fValue );

	fValue = m_damage.GetEnergy();
	fOldMax = m_damage.GetMaxEnergy();
	fNewMax = GetRatedMaximum( m_iPerformanceRatings[Rating_Energy], m_iSurgePerformanceRatings[Rating_Energy] );
	m_damage.SetMaxEnergy( fNewMax );
	m_damage.SetEnergy( fNewMax / fOldMax * fValue );

	UpdateInterface( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::HandleEnergy
//
//	PURPOSE:	Spend energy at once, or set the rate Update drains it at
//				(RETAIL: retail 0x10063cb0, called by the server shell's
//				message handler 0x1005edc0; the names are ours)
//
// ----------------------------------------------------------------------- //
void CTronPlayerObj::HandleEnergy(ILTMessage_Read *pMsg)
{
	if( m_bGodMode )
		return;

	bool bInstant = !!pMsg->Readuint8();
	float fAmount = pMsg->Readfloat();

	if( bInstant )
	{
		float fEnergy = m_damage.GetEnergy( ) - fAmount;

		if( fEnergy < m_fTronUnknown31f4 )
		{
			fEnergy = m_fTronUnknown31f4;
		}
		else if( m_damage.GetMaxEnergy( ) < fEnergy )
		{
			fEnergy = m_damage.GetMaxEnergy( );
		}

		m_damage.SetEnergy( fEnergy );
	}
	else
	{
		m_fTronUnknown31f0 = fAmount;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::HandleSwat
//
//	PURPOSE:	Fire the swat weapon from where the client says (RETAIL:
//				retail 0x10063d60, called by the server shell's message
//				handler 0x1005edf0; the names are ours)
//
// ----------------------------------------------------------------------- //
void CTronPlayerObj::HandleSwat(ILTMessage_Read *pMsg)
{
	if( !m_pSwatWeapon )
		return;

	WeaponFireInfo info;
	info.hFiredFrom = m_hObject;

	LTVector vPos = pMsg->ReadLTVector();
	info.vFirePos = vPos;
	info.vFlashPos = vPos;

	LTVector vDir = pMsg->ReadLTVector();
	info.vPath = vDir.Unit();

	m_pSwatWeapon->UpdateWeapon( info, LTTRUE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::HandleExitRequest
//
//	PURPOSE:	The client asks to leave the level (RETAIL: retail
//				0x10068560, called by the server shell's message handler
//				0x1005ee20; the names are ours)
//
// ----------------------------------------------------------------------- //
void CTronPlayerObj::HandleExitRequest(ILTMessage_Read *pMsg)
{
	if( !pMsg )
		return;

	if( pMsg->Readbool() )
	{
		if( m_szTronUnknown3267[0] )
		{
			SendTriggerMsgToObjects( this, m_szTronUnknown3267, "TRIGGER" );
		}

		return;
	}

	// RETAIL: two single player levels have their own exits
	if( GetGameType() == eGameTypeSingle )
	{
		switch( g_pServerMissionMgr->GetCurrentMission() )
		{
			case 4:
			{
				if( g_pServerMissionMgr->GetCurrentLevel() == 2 )
				{
					SendTriggerMsgToObjects( this, "Player", "option remove 5115" );
					SendTriggerMsgToObjects( this, "Player", "option remove 5251" );
					SendTriggerMsgToObjects( this, "Player", "ENDLIGHTCYCLE" );
					SendTriggerMsgToObjects( this, "exit", "trigger" );
					return;
				}
			}
			break;

			case 5:
			{
				if( g_pServerMissionMgr->GetCurrentLevel() == 2 )
				{
					SendTriggerMsgToObjects( this, "ExitTrigger", "UNLOCK" );
					SendTriggerMsgToObjects( this, "ExitTrigger", "TRIGGER" );
					return;
				}
			}
			break;
		}
	}

	if( m_szTronUnknown32a7[0] )
	{
		SendTriggerMsgToObjects( this, m_szTronUnknown32a7, "TRIGGER" );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::HandleCompile
//
//	PURPOSE:	What to do when we get a compile message from the client
//
// ----------------------------------------------------------------------- //
void CTronPlayerObj::HandleCompile(ILTMessage_Read *pMsg)
{
	// RETAIL: Tron's only flags it, for the commands update (retail
	// 0x10068660)
	if( pMsg )
	{
		m_bTronUnknown2cd6 = true;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::EnergyCheat
//
//	PURPOSE:	Fill the player's energy (RETAIL: retail 0x10068680, the
//				server shell's "mpenergy" cheat 0x1005ef90; the name is ours)
//
// ----------------------------------------------------------------------- //
void CTronPlayerObj::EnergyCheat()
{
	m_damage.SetEnergy( m_damage.GetMaxEnergy() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::Ideal*Cheat
//
//	PURPOSE:	The current level's ideal inventory (RETAIL: retail
//				0x100686a0 to 0x100686d0, the server shell's cheats
//				0x1005efa0 to 0x1005efd0; the names are ours)
//
// ----------------------------------------------------------------------- //
void CTronPlayerObj::IdealWeaponsCheat()
{
	AcquireCurrentLevelIdealWeapons();
}

void CTronPlayerObj::IdealSubroutinesCheat()
{
	AcquireCurrentLevelIdealSubroutines();
}

void CTronPlayerObj::IdealProceduralsCheat()
{
	AcquireCurrentLevelIdealProcedurals();
}

void CTronPlayerObj::IdealPointsCheat()
{
	AcquireCurrentLevelIdealBuildPoints();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::IsDefending()
//
//	PURPOSE:	Return if the player is attempting to defend.
//
// ----------------------------------------------------------------------- //
bool CTronPlayerObj::IsDefending() const
{
	// RETAIL: only the hold block in Tron (retail 0x10063eb0)
	switch ( m_cDefendType )
	{
		case MPROJ_START_HOLD_BLOCK:
		{
			return true;
		}
		break;

		case MPROJ_END_HOLD_BLOCK:
		{
			int32 nCurrentTime =
				static_cast< int >(
					g_pLTServer->GetTime() * 1000.0f
				);

			if ( ( m_nDefendServerTimeStarted + m_nDefendDuration ) > nCurrentTime )
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		break;

		default:
		{
			// not blocking
			return false;
		}
		break;
	};
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::GetDefensePercentage()
//
//	PURPOSE:	How much of the attack damage has our defenese prevented
//
//	NOTES:		There are actually 2 defense percentages.  One is based
//				on timing, i.e. player's reation speed, animation speed,
//				etc.  The other is based on the player's orientation to
//				the incoming projectile.  If the vector parameter is
//				specified, BOTH percentages will be computed and the
//				combined result will be returned.  If the vector is NOT
//				specified, only the timing will be computed.
//
// ----------------------------------------------------------------------- //
float CTronPlayerObj::GetDefensePercentage( LTVector const *pIncomingProjectilePosition /*=0*/) const
{
	// RETAIL: 0x10063f00: Tron takes the values from the defending weapon's
	// DEFENSECLASSDATA, and only the hold block defends (b69's swat and
	// arm blocks have no values).  The vector is the direction of the
	// damage.
	if ( TRONPLAYEROBJ_NO_DEFEND == m_cDefendType )
	{
		// not blocking
		return 0.0f;
	}

	// RETAIL: the id kept by HandleDefend is a weapon id
	WEAPON const *pWeaponData = g_pWeaponMgr->GetWeapon( m_cDefendAmmoId );
	if ( !pWeaponData )
	{
		return 0.0f;
	}

	DEFENSECLASSDATA const *pDefenseData = g_pWeaponMgr->GetDefenseClass( pWeaponData->nDefenseClassId );
	if ( !pDefenseData )
	{
		return 0.0f;
	}

	//
	// Do this in 2 passes.  The first pass willl determine
	// the contribution to the defense percentage due to the
	// timing of the player/animations.  The second pass
	// will add the contribution of the player's orientation.
	//
	float fDefenseTimingPercentage;
#ifdef TRON20SRC_MODERN
	float fDefenseOrientationPercentage = 0.0f;
#else
	float fDefenseOrientationPercentage;
#endif // TRON20SRC_MODERN


	//
	// Determine Timing Percentage
	//

	switch ( m_cDefendType )
	{
		case MPROJ_START_HOLD_BLOCK:
		{
			// get the current server time
			float fCurrentServerTime = g_pLTServer->GetTime() * 1000.0f;

			// determine the starting defend percentage
			float fStartDefendPercentage = pDefenseData->fStartDefendPercentage;

			// determine the max defend percentage					
			float fMaxDefendPercentage = pDefenseData->fMaxDefendPercentage;

			// determine at exactly how much time we've been in the block
			// NOTE: this is relative time, not absolute
			float fBlockTime = 
				fCurrentServerTime - m_nDefendServerTimeStarted;

			if ( ( -MATH_EPSILON <= fBlockTime ) && 
			     ( fBlockTime <= m_nDefendDuration ) )
			{
				// somewhere on the uprise
				fDefenseTimingPercentage =
					fStartDefendPercentage + 
						(
							(
								( 
									fMaxDefendPercentage -
									fStartDefendPercentage
								) /
								static_cast< float >( m_nDefendDuration )
							) * 
							fBlockTime
						);
			}
			else if ( m_nDefendDuration < fBlockTime )
			{
				// within the max range
				fDefenseTimingPercentage = fMaxDefendPercentage;
			}
			else
			{
				// math problem, we should be here if we are outside
				// the bounds of the defense
				ASSERT( 0 );
				fDefenseTimingPercentage = 0.0f;
			}
		}
		break;

		case MPROJ_END_HOLD_BLOCK:
		{
			// get the current server time
			float fCurrentServerTime = g_pLTServer->GetTime() * 1000.0f;

			// make sure we're within the range of the block
			if ( ( static_cast< int >( fCurrentServerTime ) -
			       m_nDefendServerTimeStarted ) > m_nDefendDuration )
			{
				// nope, the block is over
				return 0.0f;
			}

			// detecmine the max defend percentage					
			float fMaxDefendPercentage = pDefenseData->fMaxDefendPercentage;

			// determine the ending defend percentage
			float fEndDefendPercentage = pDefenseData->fEndDefendPercentage;

			// determine at exactly how much time we've been in the block
			// NOTE: this is relative time, not absolute
			float fBlockTime = 
				fCurrentServerTime - m_nDefendServerTimeStarted;

			// somewhere on the downfall
			fDefenseTimingPercentage =
				fMaxDefendPercentage - 
					(
						(
							( 
								fMaxDefendPercentage -
								fEndDefendPercentage
							) /
							( m_nDefendDuration )
						) * 
						( fBlockTime )
					);
		}
		break;

		default:
		{
			// There is some type of block defined that we
			// are not handling, and we SHOULD be handling it.
			ASSERT( 0 );

			return 0.0f;
		}
		break;
	};


	// check if the oriention percentage should be computed
	if ( 0 == pIncomingProjectilePosition )
	{
		// No vector specifed, there is no way
		// to compute orientation defense.
		return fDefenseTimingPercentage;
	}

	//
	// Determine Orientation percentage
	//

	float fOrientationMinDefendPercentage;
	float fOrientationMaxDefendPercentage;
	float fOrientationDeadZone;
	float fOrientationMaxZone;

	switch ( m_cDefendType )
	{
		case MPROJ_START_HOLD_BLOCK:
		case MPROJ_END_HOLD_BLOCK:
		{
			fOrientationMinDefendPercentage  = pDefenseData->fOrientationMinDefense;
			fOrientationMaxDefendPercentage  = pDefenseData->fOrientationMaxDefense;
			fOrientationDeadZone             = MATH_PI - pDefenseData->fOrientationDeadZone;
			fOrientationMaxZone              = pDefenseData->fOrientationMaxZone;
		}
		break;

		default:
		{
			// There is some type of block defined that we
			// are not handling, and we SHOULD be handling it.
			ASSERT( 0 );

			return 0.0f;
		}
		break;
	};

	LTRESULT ltResult;
	LTVector vDefendPos;
	LTVector vDefendPosToProjectile;

	// get the player's position (RETAIL: not used)
	ltResult = g_pLTServer->GetObjectPos( m_hObject, &vDefendPos );
	ASSERT( LT_OK == ltResult );

	// REMOVE THIS CODE FOR FINAL RELEASE, BUT LEAVE FOR TESTING MULTIPLAYER
	// print a warning if the time is new enough
	if ( TRONPLAYEROBJ_CLIENT_CAMERA_TIME_OLD_THRESHOLD < ( g_pLTServer->GetTime() - GetClientCameraOffsetTimeReceived() ) )
	{
		g_pLTServer->CPrint( "Client Camera Offset time is old, possible lag\n" );
		g_pLTServer->CPrint( "   condition that will affect defensive accuracy.\n" );
		g_pLTServer->CPrint( "   Currnt value is %5.3f units old.\n", ( g_pLTServer->GetTime() - GetClientCameraOffsetTimeReceived() ) );

	}

	// find a unit vector from us to the projectile (the projectile comes
	// from the opposite of the damage direction)
	vDefendPosToProjectile = -( *pIncomingProjectilePosition );
	vDefendPosToProjectile.y = 0;
	vDefendPosToProjectile.Normalize();

	// determine a forward vector reprensenting the direction the
	// player is facing
	LTRotation vPlayerViewOrientation;
	ltResult = g_pLTServer->GetObjectRotation( m_hObject, &vPlayerViewOrientation );
	ASSERT( LT_OK == ltResult );

	LTVector vPlayerViewForward = vPlayerViewOrientation.Forward();
	vPlayerViewForward.y = 0;
	vPlayerViewForward.Normalize();

	// find the angle between the two vectors
	float fDefenseAngle = ltacosf( vPlayerViewForward.Dot( vDefendPosToProjectile ) );

	// RETAIL: an angle less than MATH_EPSILON leaves the orientation
	// percentage not set (see docs/RETAIL_BUGS.md)
	if ( ( MATH_EPSILON <= fDefenseAngle) && ( fDefenseAngle <= fOrientationMaxZone ) )
	{
		// it's within the max zone
		fDefenseOrientationPercentage = fOrientationMaxDefendPercentage;
	}
	else if ( ( fOrientationMaxZone < fDefenseAngle) && ( fDefenseAngle <= fOrientationDeadZone ) )
	{
		// it's within the dropoff range
		fDefenseOrientationPercentage = 
			fOrientationMaxDefendPercentage +
			( fDefenseAngle - fOrientationMaxZone ) *
				( fOrientationMinDefendPercentage - fOrientationMaxDefendPercentage ) /
				( fOrientationDeadZone - fOrientationMaxZone );
	}
	else if ( fOrientationDeadZone <= fDefenseAngle)
	{
		// it's within the dead zone
		fDefenseOrientationPercentage = 0.0f;
	}

	//
	// Final Defense Result
	//

	return fDefenseOrientationPercentage + fDefenseTimingPercentage;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::FilterDamage()
//
//	PURPOSE:	Change the damage struct before damage is dealt to the player
//
// ----------------------------------------------------------------------- //

bool CTronPlayerObj::FilterDamage( DamageStruct *pDamageStruct )
{
	// RETAIL: retail 0x10064300: no damage in god mode or on a light cycle;
	// infected ammo may corrupt the player, a block takes only the disc
	// weapons' damage, and the player's own crushing is not scaled
	if( m_bGodMode )
		return false;

	if( m_hLightCycle )
		return false;

	if( IsInfectingDamage( pDamageStruct->eType ))
	{
		float fChance = m_afChanceToBeCorruptedByInfectedAmmo[g_pGameServerShell->GetDifficulty()];
		if( GetRandom( 0.0f, 1.0f ) < fChance )
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_TRON_UNKNOWN_151 );
			g_pLTServer->SendToClient( cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED );
		}
	}

	if( IsDefending( ) && IsDefendableDamage( pDamageStruct->eType ))
	{
		// player is defending, attenuate the damage to take
		pDamageStruct->fDamage *= ( 1.0f - GetDefensePercentage( &pDamageStruct->vDir ));
		if( MATH_EPSILON > pDamageStruct->fDamage )
			return false;
	}

	if( pDamageStruct->eType != DT_CRUSH || pDamageStruct->hDamager != GetUserObject( ))
	{
		pDamageStruct->fDamage -= pDamageStruct->fDamage * m_fTronUnknown31f8;
	}

	// pass the filter up the hierarchy
	return CPlayerObj::FilterDamage( pDamageStruct );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::IsDefendableDamage/IsInfectingDamage
//
//	PURPOSE:	RETAIL: retail 0x100644c0, 0x100644f0 (the names are ours):
//				the damage types a block stops, and those of the infected
//				ammo
//
// ----------------------------------------------------------------------- //

static const DamageType s_aeDefendableDamage[] =
{
	DT_DISC, DT_SEQUENCER, DT_CLUSTER, DT_ENEMYDISC, DT_CLUSTERAUTOBURST
};

static const DamageType s_aeInfectingDamage[] =
{
	DT_DRUNKENDIMS, DT_BALL, DT_CORRUPTICDIMS, DT_BALLLAUNCHER
};

bool CTronPlayerObj::IsDefendableDamage( DamageType eType )
{
	for( int i = 0; i < ARRAY_LEN( s_aeDefendableDamage ); i++ )
	{
		if( eType == s_aeDefendableDamage[i] )
			return true;
	}

	return false;
}

bool CTronPlayerObj::IsInfectingDamage( DamageType eType )
{
	for( int i = 0; i < ARRAY_LEN( s_aeInfectingDamage ); i++ )
	{
		if( eType == s_aeInfectingDamage[i] )
			return true;
	}

	return false;
}


// RETAIL: retail CTronPlayerObj does not override HandleDamage (its slot
// 106 is CPlayerObj's); Build 69's, which told the client of a block, is
// gone


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::OnTrigger
//
//	PURPOSE:	Process a trigger message
//
// ----------------------------------------------------------------------- //
bool CTronPlayerObj::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	// RETAIL: 0x10064520.  The commands after ACQUIRENOTE are Tron's; the
	// names of the messages and of the members they set are ours.
	static CParsedMsg::CToken s_cTok_Acquire_Subroutine(TRIGGER_ACQUIRE_SUBROUTINE);
	static CParsedMsg::CToken s_cTok_Acquire_Procedural(TRIGGER_ACQUIRE_PROCEDURAL);
	static CParsedMsg::CToken s_cTok_Acquire_Primitive(TRIGGER_ACQUIRE_PRIMITIVE);
	static CParsedMsg::CToken s_cTok_Acquire_PSet("ACQUIREPSET");
	static CParsedMsg::CToken s_cTok_Acquire_Note("ACQUIRENOTE");
	static CParsedMsg::CToken s_cTok_Set_Build_Points("SETBUILDPOINTS");
	static CParsedMsg::CToken s_cTok_Add_Build_Points(TRIGGER_ADD_BUILD_POINTS);
	static CParsedMsg::CToken s_cTok_Add_Ping("ADDPING");
	static CParsedMsg::CToken s_cTok_Remove_Ping("REMOVEPING");
	static CParsedMsg::CToken s_cTok_IOCam("IOCAM");
	static CParsedMsg::CToken s_cTok_CamFX("CAMFX");
	static CParsedMsg::CToken s_cTok_Turn("TURN");
	static CParsedMsg::CToken s_cTok_Start_LightCycle(TRIGGER_START_LIGHTCYCLE);
	static CParsedMsg::CToken s_cTok_Start_SuperCycle("STARTSUPERCYCLE");
	static CParsedMsg::CToken s_cTok_End_LightCycle(TRIGGER_END_LIGHTCYCLE);
	static CParsedMsg::CToken s_cTok_Sin_Console_Var("SINCONSOLEVAR");
	static CParsedMsg::CToken s_cTok_Ramp_Console_Var("RAMPCONSOLEVAR");
	static CParsedMsg::CToken s_cTok_Acquire_Powerup("ACQUIREPOWERUP");
	static CParsedMsg::CToken s_cTok_Announcement("ANNOUNCEMENT");
	static CParsedMsg::CToken s_cTok_Announcement_Version("ANNOUNCEMENTFINALVERSIONNUMBER");
	static CParsedMsg::CToken s_cTok_Rating("RATING");
	static CParsedMsg::CToken s_cTok_Race_Trigger("RACETRIGGER");
	static CParsedMsg::CToken s_cTok_Race_Trigger_No("RACETRIGGERNO");
	static CParsedMsg::CToken s_cTok_Lock_Disc("LOCKDISC");
	static CParsedMsg::CToken s_cTok_Lock_Weapon("LOCKWEAPON");
	static CParsedMsg::CToken s_cTok_Unlock_Weapon("UNLOCKWEAPON");
	static CParsedMsg::CToken s_cTok_Set_System_Config("SETSYSTEMCONFIG");
	static CParsedMsg::CToken s_cTok_1P_Cam("1PCAM");
	static CParsedMsg::CToken s_cTok_3P_Cam("3PCAM");
	static CParsedMsg::CToken s_cTok_Max_Zoom("MAXZOOM");
	static CParsedMsg::CToken s_cTok_Set_Sub_Help("SETSUBHELP");
	static CParsedMsg::CToken s_cTok_Manage_Subs("MANAGESUBS");
	static CParsedMsg::CToken s_cTok_Enable_LC_Dialog("ENABLELCDIALOG");
	static CParsedMsg::CToken s_cTok_Kill_All_AI_LC("KILLALLAILC");
	static CParsedMsg::CToken s_cTok_Wave_Complete("WAVECOMPLETE");
	static CParsedMsg::CToken s_cTok_Spawn_AI_Cycles("SPAWNAICYCLES");
	static CParsedMsg::CToken s_cTok_Test_Hit_By_Corruption("TestHitByCorruption");

	if(cMsg.GetArg(0) == s_cTok_Acquire_Subroutine)
	{
		char const* pSub = cMsg.GetArg(1);
		char const* pState = NULL;
		char const* pCondition = NULL;

		// Check for a 3rd and/or 4th parameter
		if(cMsg.GetArgCount() > 2)
		{
			// Check for a 3rd param
			if(CheckSubState(cMsg.GetArg(2)))
			{
				pState = cMsg.GetArg(2);
			}
			else if(CheckSubCondition(cMsg.GetArg(2)))
			{
				pCondition = cMsg.GetArg(2);
			}
			else
			{
				g_pLTServer->CPrint("ERROR - Could not give subroutine to the player with bad 2nd param = (%s). Bad level designer. BAD!!!\n",cMsg.GetArg(2).c_str());
				ASSERT(FALSE);
				return false;
			}
			
			// Check for a 4th param
			if(cMsg.GetArgCount() > 3)
			{
				if(CheckSubState(cMsg.GetArg(3)))
				{
					pState = cMsg.GetArg(3);
				}
				else if(CheckSubCondition(cMsg.GetArg(3)))
				{
					pCondition = cMsg.GetArg(3);
				}
				else
				{
					g_pLTServer->CPrint("ERROR - Could not give subroutine to the player with bad 3rd param = (%s). Bad level designer. BAD!!!\n",cMsg.GetArg(3).c_str());
					ASSERT(FALSE);
					return false;
				}
			}
		}

		return AcquireSubroutine(pSub,pState,pCondition);
	}
	else if(cMsg.GetArg(0) == s_cTok_Acquire_Procedural)
	{
		return AcquireProcedural(cMsg.GetArg(1));
	}
	else if(cMsg.GetArg(0) == s_cTok_Acquire_Primitive)
	{
		return AcquirePrimitive(cMsg.GetArg(1));
	}
	else if(cMsg.GetArg(0) == s_cTok_Acquire_PSet)
	{
		return AcquirePermissionSet(cMsg.GetArg(1));
	}
	else if(cMsg.GetArg(0) == s_cTok_Acquire_Note)
	{
		return AcquireNote(cMsg.GetArg(1));
	}
	else if(cMsg.GetArg(0) == s_cTok_Set_Build_Points)
	{
		if(cMsg.GetArgCount() > 0)
		{
			m_nBuildPoints = atoi(cMsg.GetArg(1));
		}

		return true;
	}
	else if(cMsg.GetArg(0) == s_cTok_Add_Build_Points)
	{
		if(cMsg.GetArgCount() > 0)
		{
			int nBuildPoints = atoi(cMsg.GetArg(1));
			m_nBuildPoints += nBuildPoints;
#ifdef _DEBUG
			g_pLTServer->CPrint("%d Build Points added to player. Total is now %d\n",nBuildPoints,m_nBuildPoints);
#endif
		}

		return true;
	}
	else if(cMsg.GetArg(0) == s_cTok_Add_Ping)
	{
		// RETAIL: retail 0x10065113
		if(cMsg.GetArgCount() > 1)
		{
			bool bPlaySound = true;
			if(cMsg.GetArgCount() >= 2 && IsFalseChar(*cMsg.GetArg(2)))
			{
				bPlaySound = false;
			}

			AddPing(cMsg.GetArg(1), bPlaySound);
		}
	}
	else if(cMsg.GetArg(0) == s_cTok_Remove_Ping)
	{
		if(cMsg.GetArgCount() > 1)
		{
			RemovePing(cMsg.GetArg(1));
		}
	}
	else if(cMsg.GetArg(0) == s_cTok_IOCam)
	{
		// "IOCAM <on/off> [Location] [Target]": a camera at the location's
		// object, looking at the target's
		if(cMsg.GetArgCount() > 2)
		{
			const char* pszLocation = cMsg.GetArg(2);
			ObjArray<HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
			g_pLTServer->FindNamedObjects(pszLocation, objArray);
			HOBJECT hLocation = objArray.NumObjects() ? objArray.GetObject(0) : LTNULL;
			if(hLocation)
			{
				CAutoMessage cIOCamMsg;
				cIOCamMsg.Writeuint8(MID_TRON_UNKNOWN_180);
				cIOCamMsg.Writeuint8(1);

				LTVector vPos;
				g_pLTServer->GetObjectPos(hLocation, &vPos);
				LTRotation rRot;
				g_pLTServer->GetObjectRotation(hLocation, &rRot);

				cIOCamMsg.WriteCompLTVector(vPos);
				cIOCamMsg.WriteCompLTRotation(rRot);

				g_pLTServer->FindNamedObjects(cMsg.GetArg(3), objArray);
				cIOCamMsg.WriteObject(objArray.NumObjects() ? objArray.GetObject(0) : LTNULL);

				g_pLTServer->SendToClient(cIOCamMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
			}
		}
		else
		{
			CAutoMessage cIOCamMsg;
			cIOCamMsg.Writeuint8(MID_TRON_UNKNOWN_180);
			cIOCamMsg.Writeuint8(0);
			g_pLTServer->SendToClient(cIOCamMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
		}

		return true;
	}
	else if(cMsg.GetArg(0) == s_cTok_CamFX)
	{
		bool bOn = false;
		if(cMsg.GetArgCount() > 1 && !_strcmpi(cMsg.GetArg(1), "on"))
		{
			bOn = true;
		}

		CAutoMessage cCamFXMsg;
		cCamFXMsg.Writeuint8(MID_TRON_UNKNOWN_188);
		cCamFXMsg.Writeuint8(bOn);
		g_pLTServer->SendToClient(cCamFXMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
	}
	else if(cMsg.GetArg(0) == s_cTok_Turn)
	{
		// "TURN <left/right>": the turn arrow of the light cycle
		if(cMsg.GetArgCount() > 1)
		{
			CAutoMessage cTurnMsg;
			cTurnMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
			if(!_stricmp(cMsg.GetArg(1), "left"))
			{
				cTurnMsg.Writeuint8(LCI_TRONUNKNOWN_4);
			}
			else
			{
				cTurnMsg.Writeuint8(LCI_TRONUNKNOWN_5);
			}
			g_pLTServer->SendToClient(cTurnMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
		}
	}
	else if(cMsg.GetArg(0) == s_cTok_Start_LightCycle)
	{
		// RETAIL: Tron starts and ends the cycle in Update
		if(!IsMultiplayerGame())
		{
			m_bTronUnknown32f8 = true;
		}
	}
	else if(cMsg.GetArg(0) == s_cTok_Start_SuperCycle)
	{
		if(!IsMultiplayerGame())
		{
			m_bTronUnknown32f9 = true;
		}
	}
	else if(cMsg.GetArg(0) == s_cTok_End_LightCycle)
	{
		if(!IsMultiplayerGame())
		{
			m_bTronUnknown32fa = true;
		}
	}
	else if(cMsg.GetArg(0) == s_cTok_Sin_Console_Var)
	{
		return HandleSinConsoleVarMsg(cMsg);
	}
	else if(cMsg.GetArg(0) == s_cTok_Ramp_Console_Var)
	{
		return HandleRampConsoleVarMsg(cMsg);
	}
	else if(cMsg.GetArg(0) == s_cTok_Acquire_Powerup)
	{
		LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->FindCyclist(m_hObject);
		if(pCyclist)
		{
			for(int i = 0; i < kNumLCPowerupTypes; i++)
			{
				if(!_stricmp(cMsg.GetArg(1), g_aszLCPowerupNames[i]))
				{
					g_pLightCycleMgr->SetCyclistPowerup(pCyclist, i);
				}
			}
		}

		return true;
	}
	else if(cMsg.GetArg(0) == s_cTok_Announcement)
	{
		return HandleAnnouncementMsg(cMsg);
	}
	else if(cMsg.GetArg(0) == s_cTok_Announcement_Version)
	{
		CAutoMessage cVersionMsg;
		cVersionMsg.Writeuint8(MID_TRON_VERSION);
		g_pLTServer->SendToClient(cVersionMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
		return true;
	}
	else if(cMsg.GetArg(0) == s_cTok_Rating)
	{
		return HandleRatingMsg(cMsg);
	}
	else if(cMsg.GetArg(0) == s_cTok_Race_Trigger)
	{
		if(cMsg.GetArgCount() > 1)
		{
			strcpy(m_szTronUnknown3267, cMsg.GetArg(1));
		}
	}
	else if(cMsg.GetArg(0) == s_cTok_Race_Trigger_No)
	{
		bool bSet = false;
		if(cMsg.GetArgCount() > 1)
		{
			bSet = true;
			strcpy(m_szTronUnknown32a7, cMsg.GetArg(1));
		}

		CAutoMessage cRaceMsg;
		cRaceMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
		cRaceMsg.Writeuint8(LCI_TRONUNKNOWN_21);
		cRaceMsg.Writebool(bSet);
		g_pLTServer->SendToClient(cRaceMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
	}
	else if(cMsg.GetArg(0) == s_cTok_Lock_Disc)
	{
		CAutoMessage cLockMsg;
		cLockMsg.Writeuint8(MID_TRON_UNKNOWN_152);
		cLockMsg.Writebool(true);
		g_pLTServer->SendToClient(cLockMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
		return true;
	}
	else if(cMsg.GetArg(0) == s_cTok_Lock_Weapon)
	{
		CAutoMessage cLockMsg;
		cLockMsg.Writeuint8(MID_TRON_UNKNOWN_152);
		cLockMsg.Writebool(true);
		g_pLTServer->SendToClient(cLockMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
		return true;
	}
	else if(cMsg.GetArg(0) == s_cTok_Unlock_Weapon)
	{
		CAutoMessage cLockMsg;
		cLockMsg.Writeuint8(MID_TRON_UNKNOWN_152);
		cLockMsg.Writebool(false);
		g_pLTServer->SendToClient(cLockMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
		return true;
	}
	else if(cMsg.GetArg(0) == s_cTok_Set_System_Config)
	{
		CAutoMessage cConfigMsg;
		cConfigMsg.Writeuint8(MID_SYSTEM_MEMORY_CONFIGURATION);
		cConfigMsg.WriteString(cMsg.GetArg(1));
		g_pLTServer->SendToClient(cConfigMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
		return true;
	}
	else if(cMsg.GetArg(0) == s_cTok_Set_Sub_Help)
	{
		return AcquireSubHelp(cMsg.GetArg(1));
	}
	else if(cMsg.GetArg(0) == s_cTok_1P_Cam)
	{
		CAutoMessage cCamMsg;
		cCamMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
		cCamMsg.Writeuint8(LCI_TRONUNKNOWN_18);
		g_pLTServer->SendToClient(cCamMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
	}
	else if(cMsg.GetArg(0) == s_cTok_3P_Cam)
	{
		CAutoMessage cCamMsg;
		cCamMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
		cCamMsg.Writeuint8(LCI_TRONUNKNOWN_19);
		g_pLTServer->SendToClient(cCamMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
	}
	else if(cMsg.GetArg(0) == s_cTok_Max_Zoom)
	{
		CAutoMessage cZoomMsg;
		cZoomMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
		cZoomMsg.Writeuint8(LCI_TRONUNKNOWN_20);
		float fZoom = (float)atof(cMsg.GetArg(1));
		cZoomMsg.Writefloat(fZoom);
		g_pLTServer->SendToClient(cZoomMsg.Read(), GetClient(), MESSAGE_GUARANTEED);

		if(GetGameType() == eGameTypeMultiplayerLightcycles)
		{
			g_pLightCycleMatchMgr->SetTronUnknownA4(fZoom);
		}
	}
	else if(cMsg.GetArg(0) == s_cTok_Manage_Subs)
	{
		CAutoMessage cSubsMsg;
		cSubsMsg.Writeuint8(MID_TRON_UNKNOWN_163);
		g_pLTServer->SendToClient(cSubsMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
	}
	else if(cMsg.GetArg(0) == s_cTok_Enable_LC_Dialog)
	{
		bool bEnable = true;
		if(cMsg.GetArgCount() >= 2 && !IsTrueChar(*cMsg.GetArg(1)))
		{
			bEnable = false;
		}

		((CServerLightCycleMgr*)g_pLightCycleMgr)->SetTronUnknown1365(bEnable);
	}
	else if(cMsg.GetArg(0) == s_cTok_Kill_All_AI_LC)
	{
		((CServerLightCycleMgr*)g_pLightCycleMgr)->DerezAllAICyclists();
	}
	else if(cMsg.GetArg(0) == s_cTok_Wave_Complete)
	{
		((CTronGameServerShell*)g_pGameServerShell)->NextCircuitWave();
	}
	else if(cMsg.GetArg(0) == s_cTok_Spawn_AI_Cycles)
	{
		((CTronGameServerShell*)g_pGameServerShell)->SpawnAICycles();
	}
	else if(cMsg.GetArg(0) == s_cTok_Test_Hit_By_Corruption)
	{
		CAutoMessage cHitMsg;
		cHitMsg.Writeuint8(MID_TRON_UNKNOWN_151);
		g_pLTServer->SendToClient(cHitMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
	}
	
	// Base-class it
	return(CPlayerObj::OnTrigger(hSender, cMsg));
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::AcquireSubroutine
//
//	PURPOSE:	A message came telling us to acquire this
//
// --------------------------------------------------------------------------- //
bool CTronPlayerObj::AcquireSubroutine(char const* pName, char const* pState, char const* pCondition)
{
	// Send a message to the client
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SUBROUTINE_OBTAINED);
	cMsg.WriteString(pName);
	
	if(pState)
	{
		// Sanity check
		if(CheckSubState(pState))
		{
			cMsg.WriteString(pState);
		}
		else
		{
			// We have a bad subroutine state!
			// Bad level deisgner... BAD!!!
			g_pLTServer->CPrint("ERROR - Could not give subroutine to the player with alpha/beta/gold state = (%s). Bad level designer. BAD!!!\n",pState);
			ASSERT(FALSE);
			return false;
		}
	}
	else
	{
		cMsg.WriteString("ALPHA");
	}
	
	if(pCondition)
	{
		// Sanity check
		if(CheckSubCondition(pCondition))
		{
			cMsg.WriteString(pCondition);
		}
		else
		{
			// We have a bad subroutine condition!
			// Bad level deisgner... BAD!!!
			g_pLTServer->CPrint("ERROR - Could not give subroutine to the player with condition = (%s). Bad level designer. BAD!!!\n",pCondition);
			ASSERT(FALSE);
			return false;
		}
	}
	else
	{
		cMsg.WriteString("NORMAL");
	}
	
	g_pLTServer->SendToClient(cMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::AcquireProcedural
//
//	PURPOSE:	A message came telling us to acquire this
//
// --------------------------------------------------------------------------- //
bool CTronPlayerObj::AcquireProcedural(char const* pName)
{
	// Send a message to the client
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PROCEDURAL_OBTAINED);
	cMsg.WriteString(pName);
	g_pLTServer->SendToClient(cMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::AcquirePrimitive
//
//	PURPOSE:	A message came telling us to acquire this
//
// --------------------------------------------------------------------------- //
bool CTronPlayerObj::AcquirePrimitive(char const* pName)
{
	// Send a message to the client
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PRIMITIVE_OBTAINED); // PORT: the id was SendToClient's argument
	cMsg.WriteString(pName);
	g_pLTServer->SendToClient(cMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::AcquireNote
//
//	PURPOSE:	Tell the client about an inventory item ("ACQUIRENOTE")
//
// --------------------------------------------------------------------------- //

// RETAIL: Object.lto 0x10066a10.  The name is ours.
bool CTronPlayerObj::AcquireNote(char const* pName)
{
	int nIndex = g_pInventoryButeMgr->GetItemIndex(pName);
	if (nIndex != -1)
	{
		int nTextId = g_pInventoryButeMgr->GetItemTextID(nIndex);
		uint8 nPopupId = (uint8)g_pInventoryButeMgr->GetItemType(nIndex);
		LTBOOL bIsIntel = nPopupId;
		char* pImage = g_pInventoryButeMgr->GetItemImage(nIndex);
		LTBOOL bAdd = LTTRUE;
		if (nPopupId >= 2)
			bAdd = LTFALSE;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_INTEL_PICKEDUP);
		cMsg.Writeuint32(nTextId);
		cMsg.Writeuint8(nPopupId);
		cMsg.Writeuint8(bIsIntel);
		cMsg.Writeuint8(1);
		cMsg.Writeuint8(bAdd);
		cMsg.Writebool(false);
		cMsg.WriteString(pImage);
		g_pLTServer->SendToClient(cMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
	}

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::AcquireSubHelp
//
//	PURPOSE:	Tell the client about an item for the subroutines screen
//
// --------------------------------------------------------------------------- //

// RETAIL: Object.lto 0x10066b90.  The name is ours.
bool CTronPlayerObj::AcquireSubHelp(char const* pName)
{
	int nIndex = g_pInventoryButeMgr->GetItemIndex(pName);
	if (nIndex != -1)
	{
		int nTextId = g_pInventoryButeMgr->GetItemTextID(nIndex);
		uint8 nPopupId = (uint8)g_pInventoryButeMgr->GetItemType(nIndex);
		LTBOOL bIsIntel = nPopupId;
		char* pImage = g_pInventoryButeMgr->GetItemImage(nIndex);

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_INTEL_PICKEDUP);
		cMsg.Writeuint32(nTextId);
		cMsg.Writeuint8(nPopupId);
		cMsg.Writeuint8(bIsIntel);
		cMsg.Writeuint8(1);
		cMsg.Writeuint8(0);
		cMsg.Writebool(true);
		cMsg.WriteString(pImage);
		g_pLTServer->SendToClient(cMsg.Read(), GetClient(), MESSAGE_GUARANTEED);

		cMsg.Reset();
		cMsg.Writeuint8(MID_TRON_UNKNOWN_166);
		cMsg.Writeuint32(nTextId);
		cMsg.Writeuint8(bIsIntel);
		cMsg.WriteString(pImage);
		g_pLTServer->SendToClient(cMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
	}

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::AcquirePermissionSet
//
//	PURPOSE:	Add a permission set ("ACQUIREPSET <1-8|all>", 0 clears)
//
// --------------------------------------------------------------------------- //

// RETAIL: Object.lto 0x10066da0.  The name is ours.
bool CTronPlayerObj::AcquirePermissionSet(char const* pSet)
{
	if (!stricmp(pSet, "all"))
	{
		for (int i = 1; i <= 8; i++)
		{
			m_iPermissionSet |= (1 << (i - 1));
		}
		return true;
	}

	uint8 nSet = (uint8)atoi(pSet);
	if (nSet <= 8)
	{
		if (nSet)
		{
			m_iPermissionSet |= (1 << (nSet - 1));
		}
		else
		{
			m_iPermissionSet = 0;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::CheckSubState
//
//	PURPOSE:	Verifies whether or not a subroutine state is valid
//
// ----------------------------------------------------------------------- //
bool CTronPlayerObj::CheckSubState(char const* pState)
{
	for(int i=0;i<g_nNumSubStates;i++)
	{
		if(stricmp(pState, g_SubStates[i]) == 0)
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::CheckSubCondition
//
//	PURPOSE:	Verifies whether or not a subroutine condition is valid
//
// ----------------------------------------------------------------------- //
bool CTronPlayerObj::CheckSubCondition(char const* pCondition)
{
	for(int i=0;i<g_nNumSubConditions;i++)
	{
		if(stricmp(pCondition, g_SubConditions[i]) == 0)
		{
			return true;
		}
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::ResetDefend
//
//	PURPOSE:	Reset the variables related to defense
//
// ----------------------------------------------------------------------- //

void CTronPlayerObj::ResetDefend()
{
	m_nDefendClientTimeStarted = 0;
	m_nDefendServerTimeStarted = 0;
	m_nDefendDuration = 0;
	m_cDefendType = TRONPLAYEROBJ_NO_DEFEND;
	m_cDefendAmmoId = 0;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::StartLightCycle
//
//	PURPOSE:	Player gets on a light cycle
//
// ----------------------------------------------------------------------- //
bool CTronPlayerObj::StartLightCycle( bool bSuper, bool bTronUnknown )
{
	// RETAIL: Tron's takes which cycle and a flag (retail 0x10067420): it
	// clears the start flags, picks the cycle's model (the light cycle
	// game's shade, or "LightCycleBlue" / "LightCycleBlueSuper"), goes to
	// the light cycle physics model and adds the cyclist to the light
	// cycle manager
	m_bTronUnknown32f8 = false;
	m_bTronUnknown32f9 = false;

	GameType eGameType = g_pGameServerShell->GetGameType();
	if( ( eGameType == eGameTypeSingleLightCycleCircuit ) || ( eGameType == eGameTypeSingleLightCycleCustom ) )
	{
		// RETAIL: the game options keep the light cycle game's data (its
		// style and shade at 0xa0 and 0xa1)
		CLightCycleGameData* pCycleData = ( CLightCycleGameData* )g_pGameServerShell->GetServerGameOptions().GetLightCycleGameData();
		if( !pCycleData )
			return false;

		LIGHTCYCLECOLOR* pColor = g_pLightCycleButeMgr->GetShadeCycleColor( pCycleData->m_bTronUnknownA0, pCycleData->m_bTronUnknownA1 );
		if( !pColor )
			return false;

		SAFE_STRCPY( m_szLightCycleModel, pColor->szName );
	}
	else if( ( eGameType != eGameTypeMultiplayerLightcycles ) && bTronUnknown )
	{
		if( bSuper )
		{
			strcpy( m_szLightCycleModel, "LightCycleBlueSuper" );
		}
		else
		{
			strcpy( m_szLightCycleModel, "LightCycleBlue" );
		}
	}

	SetPhysicsModel( PPM_LIGHTCYCLE );

	LTVector vDims;
	g_pPhysicsLT->GetObjectDims( m_hLightCycle, &vDims );

	return ( g_pLightCycleMgr->AddCyclist( m_hObject, m_szLightCycleModel, false, &vDims, 0, true ) != LTNULL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::EndLightCycle
//
//	PURPOSE:	Player gets off a light cycle
//
// ----------------------------------------------------------------------- //
bool CTronPlayerObj::EndLightCycle()
{
	// RETAIL: retail 0x10067570
	m_bTronUnknown32fa = false;

	// Go to normal physics model.
	SetPhysicsModel(PPM_NORMAL);

	// RETAIL: retail's light cycle manager is called through its vtable
	// (slot 4); ours is Build 69's
	g_pLightCycleMgr->RemoveCyclist( m_hObject );
	return true;
}

void CTronPlayerObj::SetVehiclePhysicsModel(PlayerPhysicsModel eModel)
{
	// RETAIL: 0x10067940: the light cycle is an object of its own (class
	// LightCycle) that the player rides
	if(eModel == PPM_LIGHTCYCLE)
	{
		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT( theStruct );

		g_pLTServer->GetObjectPos( m_hObject, &theStruct.m_Pos );
		g_pLTServer->GetObjectRotation( m_hObject, &theStruct.m_Rotation );

		HCLASS hClass = g_pLTServer->GetClass( "LightCycle" );

		char szProps[512];
		sprintf( szProps, "CycleColor %s", m_szLightCycleModel );

		LightCycle* pCycle = ( LightCycle* )g_pLTServer->CreateObjectProps( hClass, &theStruct, szProps );
		if( pCycle )
		{
			m_hLightCycle = pCycle->m_hObject;
			pCycle->Init( this );
		}
		else
		{
			g_pLTServer->CPrint( "ERROR - Could not create the light cycle object!\n" );
		}
	}
	else
	{
		CPlayerObj::SetVehiclePhysicsModel(eModel);
	}
}

// RETAIL: retail CTronPlayerObj does not override SetNormalPhysicsModel
// (its slot 124 is CPlayerObj's); Build 69's is gone


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::Respawn
//
//	PURPOSE:	Respawn the player (RETAIL: retail 0x10067210)
//
// ----------------------------------------------------------------------- //

void CTronPlayerObj::Respawn(uint8 nServerLoadGameFlags, bool bGetStartPt,
							 GameStartPoint* pStartPoint, GameBase* pSpawnAt)
{
	// RETAIL: 0x10067213 tests for a disc game type
	if( IsDiscGameType( ))
	{
		// Remember the weapon we had...

		CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon();
		if( pWeapon && pWeapon->GetId() != WMGR_INVALID_ID &&
			pWeapon->GetAmmoId() != WMGR_INVALID_ID && pWeapon->GetId() > 0 )
		{
			m_nLastWeaponId = pWeapon->GetId();
			m_nLastAmmoId = pWeapon->GetAmmoId();
		}

		ResetDefend();
	}

	CPlayerObj::Respawn( nServerLoadGameFlags, bGetStartPt, pStartPoint, pSpawnAt );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::SetLightCycleColor
//
//	PURPOSE:	Set the color of our light cycle
//
// ----------------------------------------------------------------------- //

// RETAIL: Object.lto 0x10067b90.  The name is ours.
bool CTronPlayerObj::SetLightCycleColor(char const* pszColor)
{
	if (!stricmp(pszColor, m_szLightCycleModel))
		return true;

	LIGHTCYCLECOLOR* pColor = g_pLightCycleButeMgr->GetCycleColor(pszColor);
	if (!pColor)
		return false;

	if (!g_pLightCycleButeMgr->GetCycleType(pColor->nCycleType))
		return false;

	strcpy(m_szLightCycleModel, pszColor);
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::SetDeathAnimation
//
//	PURPOSE:	On a light cycle, die without a death animation or body
//
// ----------------------------------------------------------------------- //

// RETAIL: Object.lto 0x10067c10 (CCharacter's slot 64).
void CTronPlayerObj::SetDeathAnimation()
{
	if (m_ePPhysicsModel == PPM_LIGHTCYCLE)
	{
		if (m_bStartedDeath) return;

		StartDeath();

		m_eDeathType = CD_NORMAL;
		m_eDeathDamageType = m_damage.GetDeathType();
		m_bMakeBody = false;

		HandleDead(LTFALSE);
	}
	else
	{
		CPlayerObj::SetDeathAnimation();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::HandleDead
//
//	PURPOSE:	Remember our weapon and tell the server shell
//
// ----------------------------------------------------------------------- //

// RETAIL: Object.lto 0x10067c60.
void CTronPlayerObj::HandleDead(LTBOOL /*bRemoveObj*/)
{
	CPlayerObj::HandleDead(LTFALSE);

	if (IsMultiplayerGame())
	{
		// Remember the weapon we had...

		CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon();
		if( pWeapon && pWeapon->GetId() != WMGR_INVALID_ID &&
			pWeapon->GetAmmoId() != WMGR_INVALID_ID && pWeapon->GetId() > 0 )
		{
			m_nLastWeaponId = pWeapon->GetId();
			m_nLastAmmoId = pWeapon->GetAmmoId();
		}

		g_pGameServerShell->TronUnknownVirtual6C((uint32)this, 0);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::RemoveVehicleModel
//
//	PURPOSE:	Remove our light cycle (RETAIL: retail 0x10067900)
//
// ----------------------------------------------------------------------- //

void CTronPlayerObj::RemoveVehicleModel()
{
	if( m_hLightCycle )
	{
		g_pLTServer->RemoveObject( m_hLightCycle );
		m_hLightCycle = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::GimmeGunCheat
//
//	PURPOSE:	Give a weapon (RETAIL: retail 0x100670e0)
//
// ----------------------------------------------------------------------- //

void CTronPlayerObj::GimmeGunCheat( uint8 nId )
{
	if( m_damage.IsDead() || !m_pPlayerAttachments )
		return;

	TronUnknownVirtual129( nId );
	CPlayerObj::GimmeGunCheat( nId );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::TronUnknownVirtual105
//
//	PURPOSE:	Give or take all permission sets (RETAIL: retail 0x10067120)
//
// ----------------------------------------------------------------------- //

void CTronPlayerObj::TronUnknownVirtual105( LTBOOL bOn )
{
	if( bOn )
	{
		m_iPermissionSet = 0xff;
	}
	else
	{
		m_iPermissionSet = 0;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::AcquireWeapon
//
//	PURPOSE:	Acquire a weapon (RETAIL: retail 0x100686e0)
//
// ----------------------------------------------------------------------- //

void CTronPlayerObj::AcquireWeapon( char* pWeaponName, bool bTronUnknown )
{
	if( !pWeaponName )
		return;

	WEAPON const* pWeapon = g_pWeaponMgr->GetWeapon( pWeaponName );
	if( bTronUnknown )
	{
		TronUnknownVirtual128( pWeapon );
	}

	CPlayerObj::AcquireWeapon( pWeaponName, bTronUnknown );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::CreateSwatWeapon
//
//	PURPOSE:	Make the weapon the player blocks with (RETAIL: retail
//				0x10068180; the name is ours)
//
// ----------------------------------------------------------------------- //

void CTronPlayerObj::CreateSwatWeapon()
{
	char szWeapon[30] = {0};
	g_pServerButeMgr->GetPlayerAttributeString( "SwatWeapon", szWeapon, ARRAY_LEN( szWeapon ));

	WEAPON const* pWeapon = g_pWeaponMgr->GetWeapon( szWeapon );
	if( !pWeapon )
		return;

	m_pSwatWeapons = debug_new( CWeapons );
	m_pSwatWeapons->Init( m_hObject );
	m_pSwatWeapons->ObtainWeapon( pWeapon->nId );
	m_pSwatWeapons->ChangeWeapon( pWeapon->nId );

	m_pSwatWeapon = m_pSwatWeapons->GetCurWeapon();
	m_pSwatWeapons->SetAmmo( m_pSwatWeapon->GetAmmoId() );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::InitialUpdate
//
//	PURPOSE:	First update (RETAIL: retail 0x10063a50)
//
// ----------------------------------------------------------------------- //

LTBOOL CTronPlayerObj::InitialUpdate( int nInfo )
{
	CreateSwatWeapon();

	if( IsMultiplayerGame( ))
	{
		m_bTronUnknown4C = false;
		m_bMakeBody = true;
	}

	return CPlayerObj::InitialUpdate( nInfo );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::TronUnknownVirtual128
//
//	PURPOSE:	Acquire what a weapon needs (RETAIL: retail 0x10066ec0)
//
// ----------------------------------------------------------------------- //

bool CTronPlayerObj::TronUnknownVirtual128( WEAPON const* pWeapon )
{
	if( !pWeapon )
		return false;

	AcquirePrimitive( pWeapon->szPrimitiveType );

	if( pWeapon->szSubroutineNeeded[0] )
	{
		if( !FindSubroutine( pWeapon->szSubroutineNeeded ))
		{
			AcquireSubroutine( pWeapon->szSubroutineNeeded, "alpha", "normal" );
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::TronUnknownVirtual127
//
//	PURPOSE:	Acquire what a subroutine record's weapon needs, in the
//				record's state and condition (RETAIL: retail 0x10066f20)
//
// ----------------------------------------------------------------------- //

// RETAIL: the record's state and condition names (retail 0x102de3a8,
// 0x102de3d8); ours
static char const s_aszSubroutineStates[][15] = { "alpha", "beta", "gold" };
static char const s_aszSubroutineConditions[][15] = { "normal", "infected" };

bool CTronPlayerObj::TronUnknownVirtual127( char const* pszRecord )
{
	if( !pszRecord )
		return false;

	WEAPON const* pWeapon = g_pWeaponMgr->GetWeapon( pszRecord );
	if( !pWeapon )
		return false;

	AcquirePrimitive( pWeapon->szPrimitiveType );

	if( pWeapon->szSubroutineNeeded[0] )
	{
		TronSubroutineRecord const* pRecord = (TronSubroutineRecord const*)pszRecord;
		AcquireSubroutine( pWeapon->szSubroutineNeeded,
			s_aszSubroutineStates[pRecord->nState],
			s_aszSubroutineConditions[pRecord->nCondition] );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::FullWeaponCheat
//
//	PURPOSE:	RETAIL: retail 0x10066fa0: what each player weapon needs,
//				all the mods and the level's ideal weapons
//
// ----------------------------------------------------------------------- //

void CTronPlayerObj::FullWeaponCheat()
{
	if( m_damage.IsDead( ))
		return;

	for( int i = g_pWeaponMgr->GetFirstWeaponCommandId( ); i <= g_pWeaponMgr->GetLastWeaponCommandId( ); i++ )
	{
		WEAPON const* pWeapon = g_pWeaponMgr->GetWeapon( g_pWeaponMgr->GetWeaponId( i ));
		if( !pWeapon )
			continue;

		if( g_pWeaponMgr->IsPlayerWeapon( pWeapon->nId ))
		{
			TronUnknownVirtual128( pWeapon );
		}
	}

	FullModsCheat( );
	AcquireCurrentLevelIdealWeapons( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::TronUnknownVirtual130
//
//	PURPOSE:	RETAIL: retail 0x100672a0: a disc game respawn. A spectator
//				gets no weapons unless the game type's "DiscsInSpectator"
//				is set; the others get the level's default primitives and
//				weapons and change back to the weapon they had
//
// ----------------------------------------------------------------------- //

void CTronPlayerObj::TronUnknownVirtual130( GameStartPoint* pStartPoint, bool bTronUnknown )
{
	RemoveProjectiles( );

	bool bSpectator = ( ((ArenaStartPoint*)pStartPoint)->GetType( ) == ArenaStartPoint::kType_Spectator );

	bool bDiscsInSpectator;
	ServerGameOptions& opts = g_pGameServerShell->GetServerGameOptions( );
	if( opts.m_eGameType == eGameTypeDiscArena )
		bDiscsInSpectator = opts.m_bArenaDiscsInSpectator;
	else
		bDiscsInSpectator = opts.m_bTournDiscsInSpectator;

	if( !bTronUnknown )
	{
		m_Stats.Init( );
		m_Score.SetClientID( g_pLTServer->GetClientID( GetClient( )));
		ResetPlayer( );
	}

	ResetHealth( LTFALSE, 1.0f, 1.0f );
	ResetInventory( LTTRUE, LTTRUE );

	if( bSpectator && !bDiscsInSpectator )
		return;

	MPLevelStringList& lstPrims = g_pMultiplayerLevelInfoMgr->m_lstDefaultPrims;
	MPLevelStringList::iterator iter;
	for( iter = lstPrims.begin( ); iter != lstPrims.end( ); iter++ )
	{
		AcquirePrimitive( iter->c_str( ));
	}

	MPLevelStringList& lstWeaps = g_pMultiplayerLevelInfoMgr->m_lstDefaultWeaps;
	for( iter = lstWeaps.begin( ); iter != lstWeaps.end( ); iter++ )
	{
		AcquireWeapon( (char*)iter->c_str( ));
	}

	if( m_nLastWeaponId != WMGR_INVALID_ID && m_nLastAmmoId != WMGR_INVALID_ID )
	{
		ChangeWeapon( g_pWeaponMgr->GetCommandId( m_nLastWeaponId ), LTTRUE, m_nLastAmmoId );
	}

	m_nLastWeaponId = WMGR_INVALID_ID;
	m_nLastAmmoId = WMGR_INVALID_ID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::TronUnknownVirtual129
//
//	PURPOSE:	Acquire what a player weapon needs (RETAIL: retail
//				0x10066e30)
//
// ----------------------------------------------------------------------- //

bool CTronPlayerObj::TronUnknownVirtual129( uint8 nWeaponId )
{
	if( !g_pWeaponMgr->IsPlayerWeapon( nWeaponId ))
		return false;

	WEAPON const* pWeapon = g_pWeaponMgr->GetWeapon( nWeaponId );
	if( !pWeapon )
		return false;

	return TronUnknownVirtual128( pWeapon );
}
