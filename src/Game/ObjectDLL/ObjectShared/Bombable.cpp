// ----------------------------------------------------------------------- //
//
// MODULE  : Bombable.cpp
//
// PURPOSE : The Bombable implementation
//
// CREATED : 10/25/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "MsgIds.h"
	#include "PlayerObj.h"
	#include "Spawner.h"
	#include "GadgetTargetTypes.h"
	#include "Bombable.h"
	#include "ParsedMsg.h"
	#include "FXButeMgr.h"
	#include "ServerSpecialFX.h"
	#include "ServerSoundMgr.h"

//
// Defines...
//

	#define TIME_BOMB_AMMO_NAME		"Time Bomb"

LINKFROM_MODULE( Bombable );

BEGIN_CLASS( Bombable )
	
	ADD_STRINGPROP_FLAG( GadgetTargetName, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS )
	ADD_LONGINTPROP_FLAG( CodeID, 0000, PF_HIDDEN ) 
	ADD_REALPROP_FLAG( MinTime, 0.0f, PF_HIDDEN )
	ADD_REALPROP_FLAG( MaxTime, 0.0f, PF_HIDDEN )
	ADD_REALPROP_FLAG( DefuseTime, 15.0f, 0 )
	ADD_REALPROP_FLAG( DetonateTime, -1.0f, 0 )
	ADD_STRINGPROP_FLAG( Command, "", PF_HIDDEN )
	ADD_STRINGPROP_FLAG( PlantedCommand, "", PF_NOTIFYCHANGE )
	ADD_STRINGPROP_FLAG( DetonatedCommand, "", PF_NOTIFYCHANGE )
	ADD_STRINGPROP_FLAG( DefusedCommand, "", PF_NOTIFYCHANGE )
	ADD_BOOLPROP_FLAG( StartPlanted, LTFALSE, 0 )
	ADD_BOOLPROP_FLAG( CanDefuse, LTFALSE, 0 )

END_CLASS_DEFAULT_FLAGS_PLUGIN( Bombable, GadgetTarget, NULL, NULL, 0, CBombablePlugin )


CMDMGR_BEGIN_REGISTER_CLASS( Bombable )

	CMDMGR_ADD_MSG( DETONATE, 1, NULL, "DETONATE" )
	CMDMGR_ADD_MSG( PLANTBOMB, 1, NULL, "PLANTBOMB" )
	// RETAIL: Tron's (0x10268400): VISIBLE, SOLID and HIDDEN without arguments
	CMDMGR_ADD_MSG( VISIBLE, 1, NULL, "VISIBLE" )
	CMDMGR_ADD_MSG( SOLID, 1, NULL, "SOLID" )
	CMDMGR_ADD_MSG( HIDDEN, 1, NULL, "HIDDEN" )
	
CMDMGR_END_REGISTER_CLASS( Bombable, GadgetTarget )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CBombablePlugin::PreHook_EditStringList
//
//  PURPOSE:	Populate the Type list
//
// ----------------------------------------------------------------------- //

LTRESULT CBombablePlugin::PreHook_EditStringList(
													const char* szRezPath,
													const char* szPropName,
													char** aszStrings,
													uint32* pcStrings,
													const uint32 cMaxStrings,
													const uint32 cMaxStringLength )
{

	if( LT_OK == CPropPlugin::PreHook_EditStringList(szRezPath, szPropName,
		aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}
	else if( !_stricmp("GadgetTargetName", szPropName) )
	{
		// Fill the list with our Gadget Target names...

		m_GadgetTargetMgrPlugin.SetFilterOutTypes( GT_TYPE_TO_FLAG( eKeyPad ) |
												   GT_TYPE_TO_FLAG( eCardReader ) |
												   GT_TYPE_TO_FLAG( eCodedText ) |
												   GT_TYPE_TO_FLAG( eComboLock ) |
												   GT_TYPE_TO_FLAG( ePadLock ) |
												   GT_TYPE_TO_FLAG( eDoorKnob ) |
												   GT_TYPE_TO_FLAG( eTelephone ) |
												   GT_TYPE_TO_FLAG( ePhotographable ) |
												   GT_TYPE_TO_FLAG( eInvisibleInk ));

		if( m_GadgetTargetMgrPlugin.PreHook_EditStringList( szRezPath, szPropName, aszStrings,
			pcStrings, cMaxStrings, cMaxStringLength ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::TimeBomb
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Bombable::Bombable( )
:	GadgetTarget		( ),
	m_fDefuseTime		( 0.0f ),
	m_fDetonateTime		( -1.0f ),
	m_hstrDetonateCmd	( LTNULL ),
	m_hstrDefusedCmd	( LTNULL ),
	m_bStartPlanted		( false ),
	m_bCanDefuse		( false )
{
	// RETAIL: 0x102685f0, Tron has no defusing sounds, bomb pickup or
	// DefuseOnce (build 51's members and NOLF2's StartPlanted/CanDefuse)
	m_hTimeBombModel.SetReceiver( *this );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::~TimeBomb
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Bombable::~Bombable( )
{
	FREE_HSTRING( m_hstrDetonateCmd );
	FREE_HSTRING( m_hstrDefusedCmd );

	if( m_hTimeBombModel )
	{
		// Remove the model...

		HATTACHMENT hAttachment;
        if (g_pLTServer->FindAttachment( m_hObject, m_hTimeBombModel, &hAttachment ) == LT_OK )
		{
            g_pLTServer->RemoveAttachment( hAttachment );
		}

        g_pLTServer->RemoveObject( m_hTimeBombModel );
        m_hTimeBombModel = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::EngineMessageFn
//
//  PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Bombable::EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData )
{
	switch( messageID )
	{
		case MID_UPDATE :
		{
			Update();				
		}
		break;

		case MID_ALLOBJECTSCREATED :
		{
			if( m_bStartPlanted )
				SetupDisabledState( LTFALSE );
		}
		break;

		default : break;
	}

	return GadgetTarget::EngineMessageFn( messageID, pData, fData );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::ReadProp
//
//  PURPOSE:	Handle reading in the property values
//
// ----------------------------------------------------------------------- //

LTBOOL Bombable::ReadProp( ObjectCreateStruct *pStruct )
{
	if( !pStruct ) return LTFALSE;

	// Send to base class first...

	GadgetTarget::ReadProp( pStruct );

	GenericProp gProp;

	if( g_pLTServer->GetPropGeneric( "DefuseTime", &gProp ) == LT_OK )
	{
		m_fDefuseTime = gProp.m_Float;
	}

	if( g_pLTServer->GetPropGeneric( "DetonateTime", &gProp ) == LT_OK )
	{
		m_fDetonateTime = gProp.m_Float;
	}

	// Our planted command is actually our regular disabled command...

	if( g_pLTServer->GetPropGeneric( "PlantedCommand", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
			m_hstrDisabledCmd = g_pLTServer->CreateString( gProp.m_String );
	}

	if( g_pLTServer->GetPropGeneric( "DetonatedCommand", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
			m_hstrDetonateCmd = g_pLTServer->CreateString( gProp.m_String );
	}

	if( g_pLTServer->GetPropGeneric( "DefusedCommand", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
			m_hstrDefusedCmd = g_pLTServer->CreateString( gProp.m_String );
	}

	if( g_pLTServer->GetPropGeneric( "StartPlanted", &gProp ) == LT_OK )
	{
		m_bStartPlanted = gProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "CanDefuse", &gProp ) == LT_OK )
	{
		m_bCanDefuse = gProp.m_Bool;
	}


	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::Update
//
//  PURPOSE:	Update the timebomb...
//
// ----------------------------------------------------------------------- //

void Bombable::Update()
{
	if( m_Timer.On() && m_Timer.Stopped() )
		SetupDetonatedState( );

	SetNextUpdate( UPDATE_NEXT_FRAME );	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::SetupDetonatedState
//
//  PURPOSE:	Blow the sucker up...
//
// ----------------------------------------------------------------------- //

void Bombable::SetupDetonatedState( )
{
	// Send the detonate command...
	
	if( m_hstrDetonateCmd )
	{
        const char* szCmd = g_pLTServer->GetStringData( m_hstrDetonateCmd );
		if( szCmd )
		{
			g_pCmdMgr->Process( szCmd, m_hObject, m_hObject );
		}
	}

	// Remove the Time Bomb model...

	HATTACHMENT hAttachment;
	if (g_pLTServer->FindAttachment( m_hObject, m_hTimeBombModel, &hAttachment ) == LT_OK )
	{
		g_pLTServer->RemoveAttachment( hAttachment );
	}

	g_pLTServer->RemoveObject( m_hTimeBombModel );
	m_hTimeBombModel = LTNULL;

	m_Timer.Stop();
	SetNextUpdate( UPDATE_NEVER );

	// Remove ourselves..

	g_pLTServer->RemoveObject( m_hObject );

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::SetupDisabledState
//
//  PURPOSE:	The bomb is planted so show it and start the countdown...
//
// ----------------------------------------------------------------------- //

void Bombable::SetupDisabledState( LTBOOL bDestroyed )
{
	if( m_bDisabled )
		return;

	GadgetTarget::SetupDisabledState( bDestroyed );

	// Create the time bomb model, and attach it to the bomable area...

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );

	LTRotation rRot;
	g_pLTServer->GetObjectRotation( m_hObject, &rRot );

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Pos = vPos;
	theStruct.m_Rotation = rRot;

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( TIME_BOMB_AMMO_NAME );
	if( !pAmmo || !pAmmo->pProjectileFX )
		return;

	SAFE_STRCPY(theStruct.m_Filename, pAmmo->pProjectileFX->szModel );
	SAFE_STRCPY(theStruct.m_SkinName, pAmmo->pProjectileFX->szSkin );

	theStruct.m_Scale = pAmmo->pProjectileFX->vModelScale;
	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE;
	theStruct.m_ObjectType  = OT_MODEL;

    HCLASS hClass = g_pLTServer->GetClass("GameBase");
    LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pModel) return;

	m_hTimeBombModel = pModel->m_hObject;

	// Attach the model to the the camera...

    LTVector vOffset(0, 0, 0);
    LTRotation rOffset;

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, m_hTimeBombModel, "TimeBomb",
											     &vOffset, &rOffset, &hAttachment);
    if (dRes != LT_OK)
	{
        g_pLTServer->SetObjectPos( m_hTimeBombModel, &vPos );

		// The time bomb model is rotated 180 degrees...
		rRot.Rotate( rRot.Up(), MATH_PI );
		
		g_pLTServer->SetObjectRotation( m_hTimeBombModel, &rRot );
	}

	// Set the Models's animation...

    HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_hTimeBombModel, "Idle");
	if( hAni != INVALID_MODEL_ANIM )
	{
        g_pLTServer->SetModelLooping( m_hTimeBombModel, LTTRUE );
        g_pLTServer->SetModelAnimation( m_hTimeBombModel, hAni );
	}
	
	
	// Set us up so we can now be activated to defuse the TimeBomb...

	if( m_bCanDefuse )
	{
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, m_dwGadgetUsrFlgs | USRFLG_CAN_ACTIVATE );
	}

	// Start the countdown...

	if( m_fDetonateTime > 0.0f )
	{
		m_Timer.Start( m_fDetonateTime );
		SetNextUpdate( UPDATE_NEXT_FRAME );
	}

	// Play the projectiles ClientFX on the time bomb model...

	if( pAmmo->pProjectileFX->szFXName[0] )
		SetObjectClientFXMsg( m_hTimeBombModel, pAmmo->pProjectileFX->szFXName, FXFLAG_LOOP );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::HandleActivateMsg
//
//  PURPOSE:	Start defusing the bomb...
//
// ----------------------------------------------------------------------- //

void Bombable::HandleActivateMsg( HOBJECT hSender )
{
	// RETAIL: 0x102690c0, build 51's (no team check, no defusing sound)

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_User, dwFlags );

	if( (dwFlags & USRFLG_CAN_ACTIVATE) && m_bDisabled )
	{
		// Send a message to the client letting it know how long it should take
		// and whether or not it should show the bar...

		if( !IsPlayer( hSender ) ) return;

		CPlayerObj *pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( hSender );
		if( !pPlayer ) return;

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_GADGETTARGET );
		cMsg.WriteObject( m_hObject );
		cMsg.Writeuint8( 0 /*eINVALID*/ );
		cMsg.Writefloat( m_fDefuseTime );
		cMsg.Writefloat( m_fDefuseTime );
		cMsg.Writeuint32( 0 );
		g_pLTServer->SendToClient( cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::HandleStoppedMsg
//
//  PURPOSE:	Handle stopping the disabling
//
// ----------------------------------------------------------------------- //

void Bombable::HandleStoppedMsg( const CParsedMsg &cMsg )
{
	// RETAIL: 0x10269250, build 51's

	if( !m_bDisabled )
	{
		GadgetTarget::HandleStoppedMsg( cMsg );
		return;
	}

	// We already planted the bomb...
	// The second stopped message is for stopping the defusing of the bomb...

	LTFLOAT	fTime = (LTFLOAT)atof( cMsg.GetArg(1) );

	if( fTime > 0.0f )
	{
		// Make sure we can try and defuse it again...

		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE );
	}
	else
	{
		// Phew! It was defused
		
		SetupDefusedState( );		
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::SetupDefusedState
//
//  PURPOSE:	The bomb was defused... Set us back up to plant the bomb again.
//
// ----------------------------------------------------------------------- //

void Bombable::SetupDefusedState( )
{
	// RETAIL: 0x102692c0, build 51's with the TimeBomb weapon pickup

	// The Bomb is no longer planted...

	m_bDisabled = LTFALSE;

	// Send the defused command...
	
	if( m_hstrDefusedCmd )
	{
        const char* szCmd = g_pLTServer->GetStringData( m_hstrDefusedCmd );
		if( szCmd )
		{
			g_pCmdMgr->Process( szCmd, m_hObject, m_hObject );
		}
	}

	// Stop the countdown...

	m_Timer.Stop();
	SetNextUpdate( UPDATE_NEVER );

	// Create the TimeBomb ammo powerup...

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( TIME_BOMB_AMMO_NAME );
	if( !pAmmo || !m_hTimeBombModel || !pAmmo->pProjectileFX )
		return;

	char szSpawn[512];
	sprintf(szSpawn, "WeaponItem WeaponType TimeBomb;AmmoCount 1;MoveToFloor 0" );

	LTVector vScale = pAmmo->pProjectileFX->vModelScale;

	LTRotation	rRot;
	g_pLTServer->GetObjectRotation( m_hTimeBombModel, &rRot );

	LTVector	vPos;
	g_pLTServer->GetObjectPos( m_hTimeBombModel, &vPos );

	BaseClass* pClass = SpawnObject(szSpawn, vPos, rRot);

	if (pClass)
	{
		SetObjectFilenames( pClass->m_hObject, pAmmo->pProjectileFX->szModel, pAmmo->pProjectileFX->szSkin );

		g_pLTServer->ScaleObject(pClass->m_hObject, &vScale);

		LTVector vDims;
		g_pPhysicsLT->GetObjectDims(pClass->m_hObject, &vDims);
		vDims.x *= vScale.x;
		vDims.y *= vScale.y;
		vDims.z *= vScale.z;

		g_pPhysicsLT->SetObjectDims(pClass->m_hObject, &vDims, 0);
	}


	// Remove the Time Bomb model...

	HATTACHMENT hAttachment;
	if (g_pLTServer->FindAttachment( m_hObject, m_hTimeBombModel, &hAttachment ) == LT_OK )
	{
		g_pLTServer->RemoveAttachment( hAttachment );
	}

	g_pLTServer->RemoveObject( m_hTimeBombModel );
	m_hTimeBombModel = LTNULL;

	// Clients can try to plant another bomb...

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, m_dwGadgetUsrFlgs, m_dwGadgetUsrFlgs );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::OnSave
//
//  PURPOSE:	Save it
//
// ----------------------------------------------------------------------- //

LTBOOL Bombable::OnSave( ILTMessage_Write *pMsg )
{
	if( !pMsg ) return LTFALSE;

	m_Timer.Save( pMsg );

	SAVE_FLOAT( m_fDefuseTime );
	SAVE_FLOAT( m_fDetonateTime );
	SAVE_HSTRING( m_hstrDetonateCmd );
	SAVE_HSTRING( m_hstrDefusedCmd );
	SAVE_HOBJECT( m_hTimeBombModel );
	SAVE_bool( m_bStartPlanted );
	SAVE_bool( m_bCanDefuse );

	return GadgetTarget::OnSave( pMsg );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::OnLoad
//
//  PURPOSE:	Load it
//
// ----------------------------------------------------------------------- //

LTBOOL Bombable::OnLoad( ILTMessage_Read *pMsg )
{
	if( !pMsg ) return LTFALSE;

	m_Timer.Load( pMsg );

	LOAD_FLOAT( m_fDefuseTime );
	LOAD_FLOAT( m_fDetonateTime );
	LOAD_HSTRING( m_hstrDetonateCmd );
	LOAD_HSTRING( m_hstrDefusedCmd );
	LOAD_HOBJECT( m_hTimeBombModel );
	LOAD_bool( m_bStartPlanted );
	LOAD_bool( m_bCanDefuse );

	return GadgetTarget::OnLoad( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Bombable::OnLinkBroken
//
//	PURPOSE:	Handle attached object getting removed.
//
// ----------------------------------------------------------------------- //

void Bombable::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if( pRef == &m_hTimeBombModel )
	{
		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->FindAttachment( m_hObject, hObj, &hAttachment) )
		{
			g_pLTServer->RemoveAttachment(hAttachment);
		}
	}
	GadgetTarget::OnLinkBroken( pRef, hObj );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::OnTrigger
//
//  PURPOSE:	Handle a trigger message...
//
// ----------------------------------------------------------------------- //

bool Bombable::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	// RETAIL: 0x102697e0, Tron's: VISIBLE, SOLID and HIDDEN are relayed to
	// the planted bomb model and then handled as usual
	static CParsedMsg::CToken s_cTok_Detonate( "DETONATE" );
	static CParsedMsg::CToken s_cTok_PlantBomb( "PLANTBOMB" );
	static CParsedMsg::CToken s_cTok_Visible( "VISIBLE" );
	static CParsedMsg::CToken s_cTok_Solid( "SOLID" );
	static CParsedMsg::CToken s_cTok_Hidden( "HIDDEN" );
	
	if( cMsg.GetArg(0) == s_cTok_Detonate )
	{
		SetupDetonatedState();
		return true;
	}
	else if( cMsg.GetArg(0) == s_cTok_PlantBomb )
	{
		SetupDisabledState( LTFALSE );
		return true;
	}
	else if( (cMsg.GetArg(0) == s_cTok_Visible) ||
			 (cMsg.GetArg(0) == s_cTok_Solid) ||
			 (cMsg.GetArg(0) == s_cTok_Hidden) )
	{
		// Relay the message to the bomb model if one is planted...

		if( m_hTimeBombModel )
		{
			char szMsg[128] = {0};
			cMsg.ReCreateMsg( szMsg, ARRAY_LEN( szMsg ), 0 );
			SendTriggerMsgToObject( this, m_hTimeBombModel, LTTRUE, szMsg );

			return GadgetTarget::OnTrigger( hSender, cMsg );
		}

		return true;
	}
	
	return GadgetTarget::OnTrigger( hSender, cMsg );
}
