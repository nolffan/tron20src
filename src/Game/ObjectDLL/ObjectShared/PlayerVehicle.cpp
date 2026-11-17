// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerVehicle.cpp
//
// PURPOSE : Implementation of the PlayerVehicle
//
// CREATED : 8/31/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerVehicle.h"
#include "PlayerButes.h"
#include "ServerButeMgr.h"
#include "CommonUtilities.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "PlayerObj.h"
#include "KeyMgr.h"



LINKFROM_MODULE( PlayerVehicle );



// ----------------------------------------------------------------------- //
//
//	CLASS:		PlayerVehicle
//
//	PURPOSE:	An PlayerVehicle object
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(PlayerVehicle)
	ADD_STRINGPROP_FLAG(Filename, "Props\\Models\\Snowmobile.ltb", PF_HIDDEN | PF_DIMS | PF_LOCALDIMS | PF_FILENAME | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, "Props\\Skins\\Snowmobile.dtx", PF_FILENAME | PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_SOLID_FLAG(0, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_SHADOW_FLAG(1, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(MoveToFloor, LTTRUE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(VehicleType, "", PF_STATICLIST)
	ADD_REALPROP(RespawnTime, -1.0f)
	ADD_BOOLPROP_FLAG(Locked, LTFALSE, 0)
	ADD_STRINGPROP_FLAG(LockedCommand, "", PF_NOTIFYCHANGE)
END_CLASS_DEFAULT_FLAGS_PLUGIN(PlayerVehicle, Prop, NULL, NULL, 0, CPlayerVehiclePlugin)


CMDMGR_BEGIN_REGISTER_CLASS( PlayerVehicle )

	CMDMGR_ADD_MSG( ACTIVATE, 1, NULL, "ACTIVATE" )
	CMDMGR_ADD_MSG( LOCK, 1, NULL, "LOCK" )
	CMDMGR_ADD_MSG( UNLOCK, 1, NULL, "UNLOCK" )

CMDMGR_END_REGISTER_CLASS( PlayerVehicle, Prop )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::PlayerVehicle()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

PlayerVehicle::PlayerVehicle() : Prop ()
{
	m_fRespawnTime = -1.0f;

	m_vOriginalPos.Init();
	m_vOriginalDims.Init(10, 10, 10);
    m_rOriginalRot.Init();

	m_bLocked = false;
	m_hstrLockedCommand	= LTNULL;

	// RETAIL: Build 51's debris (0x1017e299)
	m_pDebrisOverride = "Metal small";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::~PlayerVehicle()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

PlayerVehicle::~PlayerVehicle()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PlayerVehicle::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
            uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
				PostPropRead((ObjectCreateStruct*)pData);
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
            uint32 dwRet =  Prop::EngineMessageFn(messageID, pData, fData);

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			return dwRet;
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData);
		}
		break;

		default : break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) return;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("VehicleType", &genProp) == LT_OK)
	{
		m_ePPhysicsModel = GetPlayerPhysicsModelFromPropertyName(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("RespawnTime", &genProp) == LT_OK)
	{
		m_fRespawnTime = genProp.m_Float;
	}

	if( g_pLTServer->GetPropGeneric( "Locked", &genProp ) == LT_OK )
	{
		m_bLocked = genProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "LockedCommand", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] )
		{
			m_hstrLockedCommand = g_pLTServer->CreateString( genProp.m_String );
		}
	}

	m_PlayerVehicleStruct.ePhysicsModel = m_ePPhysicsModel;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	// Make us activateable...

	// Need rayhit to activate.
	// Need clientnonsolid because we like redundancies.
	// Need touchnotify so that we get put into containers.
	m_dwFlags |= (FLAG_RAYHIT | FLAG_CLIENTNONSOLID | FLAG_TOUCH_NOTIFY);
	m_dwFlags &= ~( FLAG_SOLID | FLAG_GRAVITY );


    char* pModelAttribute	= LTNULL;
    char* pSkin1Attribute	= LTNULL;
    char* pSkin2Attribute	= LTNULL;
	char* pSkin3Attribute	= LTNULL;
	char* pRenderStyle1Bute	= LTNULL;
	char* pRenderStyle2Bute	= LTNULL;
	char* pRenderStyle3Bute = LTNULL;

	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
		{
			pModelAttribute		= PLAYER_BUTE_SNOWMOBILEMODEL;
			pSkin1Attribute		= PLAYER_BUTE_SNOWMOBILESKIN;
			pSkin2Attribute		= PLAYER_BUTE_SNOWMOBILESKIN2;
			pSkin3Attribute		= PLAYER_BUTE_SNOWMOBILESKIN3;
			pRenderStyle1Bute	= PLAYER_BUTE_SNOWMOBILERS;
			pRenderStyle2Bute	= PLAYER_BUTE_SNOWMOBILERS2;
			pRenderStyle3Bute	= PLAYER_BUTE_SNOWMOBILERS3;
		}
		break;

		default :
		break;
	}

	// Set the correct filename and skin...

	if (pModelAttribute)
	{
		g_pServerButeMgr->GetPlayerAttributeString(pModelAttribute,
			pStruct->m_Filename, ARRAY_LEN(pStruct->m_Filename));
	}

	if (pSkin1Attribute)
	{
		g_pServerButeMgr->GetPlayerAttributeString(pSkin1Attribute,
			pStruct->m_SkinNames[0], ARRAY_LEN(pStruct->m_SkinNames[0]));
	}

	if (pSkin2Attribute)
	{
		g_pServerButeMgr->GetPlayerAttributeString(pSkin2Attribute,
			pStruct->m_SkinNames[1], ARRAY_LEN(pStruct->m_SkinNames[1]));
	}

	if (pSkin3Attribute)
	{
		g_pServerButeMgr->GetPlayerAttributeString(pSkin3Attribute,
			pStruct->m_SkinNames[2], ARRAY_LEN(pStruct->m_SkinNames[2]));
	}

	if (pRenderStyle1Bute)
	{
		g_pServerButeMgr->GetPlayerAttributeString(pRenderStyle1Bute,
			pStruct->m_RenderStyleNames[0], ARRAY_LEN(pStruct->m_RenderStyleNames[0]));
	}

	if (pRenderStyle2Bute)
	{
		g_pServerButeMgr->GetPlayerAttributeString(pRenderStyle2Bute,
			pStruct->m_RenderStyleNames[1], ARRAY_LEN(pStruct->m_RenderStyleNames[1]));
	}

	if (pRenderStyle3Bute)
	{
		g_pServerButeMgr->GetPlayerAttributeString(pRenderStyle3Bute,
			pStruct->m_RenderStyleNames[2], ARRAY_LEN(pStruct->m_RenderStyleNames[2]));
	}

	// RETAIL: no translucency attribute in Tron (0x1017e644)
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool PlayerVehicle::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Activate( "ACTIVATE" );
	static CParsedMsg::CToken s_cTok_Lock( "LOCK" );
	static CParsedMsg::CToken s_cTok_UnLock( "UNLOCK" );

	if (cMsg.GetArg(0) == s_cTok_Activate)
	{
		// RETAIL: Prop does not see the activate in Tron (0x1017e736)
		DoActivate(hSender);
	}
	else if( cMsg.GetArg(0) == s_cTok_Lock )
	{
		m_bLocked = true;
	}
	else if( cMsg.GetArg(0) == s_cTok_UnLock )
	{
		m_bLocked = false;
	}
	else
	{
		return Prop::OnTrigger(hSender, cMsg);
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::InitialUpdate()
{
    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);

	// Can't damage player vehicles...

    m_damage.SetCanDamage(LTFALSE);
	m_damage.SetMass(1000.0f);

	g_pLTServer->GetObjectPos(m_hObject, &m_vOriginalPos);
	g_pLTServer->GetObjectRotation(m_hObject, &m_rOriginalRot);
	g_pPhysicsLT->GetObjectDims(m_hObject, &m_vOriginalDims);

	CreateSFXMsg();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::Respawn()
//
//	PURPOSE:	Start the respawn process...
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::Respawn()
{
    LTFLOAT fRespawnTime = m_fRespawnTime;

	if (fRespawnTime >= 0.0f)
	{
		g_pLTServer->SetObjectPos(m_hObject, &m_vOriginalPos);
		g_pLTServer->SetObjectRotation(m_hObject, &m_rOriginalRot);

        LTVector vZero(0, 0, 0);
		g_pPhysicsLT->SetVelocity(m_hObject, &vZero);
	}
	else
	{
		fRespawnTime = 0.0f;
	}


	// Try to set our dims (make sure we fit where we were placed)...

	LTVector vDims = m_vOriginalDims;
    if (g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, SETDIMS_PUSHOBJECTS) == LT_ERROR)
	{
        g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);
	}

 	LTVector vVec(0, 0, 0);
	g_pPhysicsLT->SetVelocity(m_hObject, &vVec);

	// RETAIL: Tron sets all of its flags, without the visible flag (0x1017e975)
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_dwFlags & ~FLAG_VISIBLE, FLAGMASK_ALL );

	m_RespawnTimer.Start(fRespawnTime);
    SetNextUpdate(UPDATE_NEXT_FRAME);

	MoveObjectToFloor(m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::Update()
//
//	PURPOSE:	Process updates if necessary...
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::Update()
{
	if (m_RespawnTimer.Stopped())
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
        SetNextUpdate(UPDATE_NEVER);
	}
	else
	{
        SetNextUpdate(UPDATE_NEXT_FRAME);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::DoActivate()
//
//	PURPOSE:	Handle Activate...
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::DoActivate(HOBJECT hSender)
{
	// Make sure we're visible (i.e., spawned in)...

    uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	if ( !(dwFlags & FLAG_VISIBLE) || !hSender || !IsPlayer(hSender)) return;

	
	if( m_bLocked )
	{
		// Check to see if the player has the correct keys to operate a vehicle...

		if( !g_pKeyMgr->CanCharacterControlObject( hSender, m_hObject ))
		{
			// Send a command if we have one...

			if( m_hstrLockedCommand )
			{
				const char	*pCmd = g_pLTServer->GetStringData( m_hstrLockedCommand );

				if( pCmd && g_pCmdMgr->IsValidCmd( pCmd ))
				{
					g_pCmdMgr->Process( pCmd, m_hObject, hSender );
				}
			}

			// No keys? No vehicle! 
		
			return;
		}
	}

	// Tell the player to hop aboard...

    CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hSender);
	if (pPlayer)
	{
		pPlayer->RideVehicle(this);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	SAVE_VECTOR(m_vOriginalPos);
	SAVE_VECTOR(m_vOriginalDims);
	SAVE_ROTATION(m_rOriginalRot);
 	SAVE_FLOAT(m_fRespawnTime);
	SAVE_BYTE((uint8)m_ePPhysicsModel);
	SAVE_bool( m_bLocked );
	SAVE_HSTRING( m_hstrLockedCommand );

	m_RespawnTimer.Save(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	LOAD_VECTOR(m_vOriginalPos);
	LOAD_VECTOR(m_vOriginalDims);
	LOAD_ROTATION(m_rOriginalRot);
 	LOAD_FLOAT(m_fRespawnTime);
	LOAD_BYTE_CAST(m_ePPhysicsModel, PlayerPhysicsModel);
	LOAD_bool( m_bLocked );
	LOAD_HSTRING( m_hstrLockedCommand );

	m_RespawnTimer.Load(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::CreateSFXMsg
//
//	PURPOSE:	Create our special fx message
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::CreateSFXMsg()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_PLAYERVEHICLE_ID);
    m_PlayerVehicleStruct.Write(cMsg);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}


LTRESULT CPlayerVehiclePlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{

	// Handle vehicle type...

	if (_strcmpi("VehicleType", szPropName) == 0)
	{
		for (int i=PPM_FIRST; i < PPM_NUM_MODELS; i++)
		{
			_ASSERT(cMaxStrings > (*pcStrings) + 1);
			if (cMaxStrings > (*pcStrings) + 1)
			{
				strcpy(aszStrings[(*pcStrings)++], GetPropertyNameFromPlayerPhysicsModel((PlayerPhysicsModel)i));
			}
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerVehiclePlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CPlayerVehiclePlugin::PreHook_PropChanged( const char *szObjName,
													const char *szPropName, 
													const int  nPropType, 
													const GenericProp &gpPropValue,
													ILTPreInterface *pInterface,
													const char *szModifiers )
{
	// Check if the props are our commands and then just send it to the CommandMgr..

	if( !_stricmp( "LockedCommand", szPropName ))
	{
		if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
													szPropName, 
													nPropType, 
													gpPropValue,
													pInterface,
													szModifiers ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}


