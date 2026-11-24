
//----------------------------------------------------------------------------
//              
//	MODULE:		PowerArmor.cpp
//              
//	PURPOSE:	- implementation
//              
//	CREATED:	13.03.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#include "PowerArmor.h"		
#include "ltserverobj.h"
#include "ObjectMsgs.h"
#include "Attachments.h"
#include "ModelButeMgr.h"
#include "CharacterHitBox.h"
#include "Projectile.h"
#include "GameServerShell.h"

// Forward declarations

// Globals

// Statics

LINKFROM_MODULE( PowerArmor );

BEGIN_CLASS(PowerArmor)
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP(1), PF_HIDDEN)
END_CLASS_DEFAULT_FLAGS(PowerArmor, Prop, NULL, NULL, CF_HIDDEN)

// RETAIL: the damage types the armor stops whole at the two easier
// difficulties (Object.lto .data 0x102fa9e4). The name is ours
static DamageType s_aeArmorProofDamageTypes[] =
{
	DT_DISC,
	DT_SEQUENCER,
	DT_CLUSTER,
	DT_ENEMYDISC,
	DT_CLUSTERAUTOBURST,
};

// RETAIL: the armor's hit points (Object.lto 0x102987f8). The name is ours
const LTFLOAT c_fPowerArmorHitPoints = 20.0f;

// RETAIL: Prop's attachment fade cvars (Object.lto 0x10378a3c, 0x10378a4c)
extern CVarTrack s_vtAttachmentFadeDelay;
extern CVarTrack s_vtAttachmentFadeDuration;


//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::PowerArmor()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
PowerArmor::PowerArmor()
{
	m_eState = kPA_Off;
	m_eLock = kPA_Unlocked;
	m_bAttached = true;
	m_fMaxHitPoints = c_fPowerArmorHitPoints;
	m_fHitPoints = c_fPowerArmorHitPoints;
	m_bCanDamage = true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::~PowerArmor()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
PowerArmor::~PowerArmor()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::EngineMessageFn()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ uint32 PowerArmor::EngineMessageFn(uint32 messageID,void *pData,LTFLOAT fData)
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
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			return Prop::EngineMessageFn(messageID, pData, fData);
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

		case MID_INITIALUPDATE:
		{
			m_damage.SetCanDamage(LTFALSE);
			m_damage.m_bRemoveOnDeath = LTFALSE;
		}
		break;
	}

	uint32 iResult = Prop::EngineMessageFn(messageID, pData, fData);

	// UGLY!  There are 2 reasons for this code here:
	// 
	// 1:  The Destructable aggregate sets the touchnotify flag in the 
	// InitialUpdate.  As this is called in the aggregates INITIAL_UPDATE,
	// we need to kill this flag or the armor and wearer will collide
	// constantly and performance will die.
	//
	// 2. Props flag setting only ADDS flags, it doesn't remove them.  This
	// will insure that the flags are truely and totally gone.
	if ( messageID == MID_INITIALUPDATE )
	{
		if (fData != INITIALUPDATE_SAVEGAME)
		{
			g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, 0, FLAG_TOUCH_NOTIFY );

			// RETAIL: Tron's armor starts off: invisible and shrunk
			// (Object.lto 0x1013b379)
			if ( m_eState == kPA_Off )
			{
				float r, g, b, a;
				g_pLTServer->GetObjectColor( m_hObject, &r, &g, &b, &a );
				g_pLTServer->SetObjectColor( m_hObject, r, g, b, 0.0f );

				LTVector vScale( 0.0f, 0.0f, 0.0f );
				g_pLTServer->ScaleObject( m_hObject, &vScale );
			}
		}
	}

	return iResult;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::ObjectMessageFn()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ uint32 PowerArmor::ObjectMessageFn(HOBJECT hSender,ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();
	switch(messageID)
	{
		case MID_TRIGGER:
		{
			uint32 flags;
			g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, flags );

			const char* szMsg = (const char*)pMsg->Readuint32();

			// RETAIL: Tron's commands (Object.lto 0x1013b485)
			if ( !_stricmp(szMsg, KEY_TURNON) )
			{
				if ( m_eState != kPA_TurningOn && m_eState != kPA_On && m_eLock != kPA_LockedOff )
				{
					m_eState = kPA_TurningOn;
					SetNextUpdate(UPDATE_NEXT_FRAME);
				}
			}
			else if ( !_stricmp(szMsg, KEY_TURNOFF) )
			{
				if ( m_eState != kPA_TurningOff && m_eState != kPA_Off && m_eLock != kPA_LockedOn )
				{
					m_eState = kPA_TurningOff;
					SetNextUpdate(UPDATE_NEXT_FRAME);
				}
			}
			else if ( !_stricmp(szMsg, KEY_LOCKON) )
			{
				m_eLock = kPA_LockedOn;
				if ( m_eState == kPA_Off || m_eState == kPA_TurningOff )
				{
					m_eState = kPA_TurningOn;
					SetNextUpdate(UPDATE_NEXT_FRAME);
				}
			}
			else if ( !_stricmp(szMsg, KEY_LOCKOFF) )
			{
				m_eLock = kPA_LockedOff;
				if ( m_eState == kPA_On || m_eState == kPA_TurningOn )
				{
					m_eState = kPA_TurningOff;
					SetNextUpdate(UPDATE_NEXT_FRAME);
				}
			}
			// RETAIL: compares with an empty string, not KEY_UNLOCK
			// (Object.lto 0x1013b5b4; docs/RETAIL_BUGS.md)
#ifdef TRON20SRC_MODERN
			else if ( !_stricmp(szMsg, KEY_UNLOCK) )
#else
			else if ( !_stricmp(szMsg, "") )
#endif
			{
				m_eLock = kPA_Unlocked;
			}
			else if ( !_stricmp(szMsg, KEY_REMOVE) )
			{
				m_bAttached = false;
				g_pLTServer->RemoveObject( m_hObject );
			}
			else if ( !_stricmp(szMsg, KEY_CANDAMAGE) )
			{
				m_bCanDamage = true;
			}
			else if ( !_stricmp(szMsg, KEY_CANTDAMAGE) )
			{
				m_bCanDamage = false;
			}
		}
		break;

		case MID_DAMAGE:
		{
			// RETAIL: Tron's armor takes its damage from projectiles only
			// (HandleProjectileImpact)
			return LTTRUE;
		}
		break;

		default:
			break;
	}

	return Prop::ObjectMessageFn(hSender, pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::Update()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void PowerArmor::Update(void)
{
	uint32 flags;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, flags );

	switch (m_eState)
	{
	case kPA_On:
		HandleOn();
		break;
	
	case kPA_Off:
		HandleOff();
		break;
	
	case kPA_TurningOn:
		HandleTurningOn();
		break;

	case kPA_TurningOff:
		HandleTurningOff();
		break;
	};
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::HandleOff()
//              
//	PURPOSE:	RETAIL: Tron's armor fades out (Object.lto 0x1013b6e0)
//              
//----------------------------------------------------------------------------
void PowerArmor::HandleOff()
{
	UpdateFade();

	if ( !m_bFading )
	{
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, 0, FLAG_VISIBLE );
		SetNextUpdate(UPDATE_NEVER);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::HandleOn()
//              
//	PURPOSE:	RETAIL: Tron's armor fades in (Object.lto 0x1013b720)
//              
//----------------------------------------------------------------------------
void PowerArmor::HandleOn()
{
	UpdateFade();

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );

	if ( !m_bFading )
	{
		SetNextUpdate(UPDATE_NEVER);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::HandleTurningOn()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void PowerArmor::HandleTurningOn()
{
	m_eState = kPA_On;

	LTFLOAT fDuration = s_vtAttachmentFadeDuration.GetFloat();
	LTFLOAT fDelay = s_vtAttachmentFadeDelay.GetFloat();
	StartFade( fDuration, fDelay, 1.0f, false );

	m_fStartAlpha = 0.0f;
	m_fEndAlpha = 1.0f;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::HandleTurningOff()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void PowerArmor::HandleTurningOff()
{
	m_eState = kPA_Off;

	LTFLOAT fDuration = s_vtAttachmentFadeDuration.GetFloat();
	LTFLOAT fDelay = s_vtAttachmentFadeDelay.GetFloat();
	StartFade( fDuration, fDelay, 0.0f, false );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::Save()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void PowerArmor::Save(ILTMessage_Write *pMsg)
{
	SAVE_DWORD(m_eState);
	SAVE_DWORD(m_eLock);
	SAVE_bool(m_bAttached);
	SAVE_CHARSTRING(m_sOnDestroyFX.c_str());
	SAVE_FLOAT(m_fHitPoints);
	SAVE_FLOAT(m_fMaxHitPoints);
	SAVE_bool(m_bCanDamage);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::Load()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void PowerArmor::Load(ILTMessage_Read *pMsg)
{
	LOAD_DWORD_CAST(m_eState, ePA_State);
	LOAD_DWORD_CAST(m_eLock, ePA_Lock);
	LOAD_bool(m_bAttached);

	char szString[80];
	LOAD_CHARSTRING(szString, ARRAY_LEN(szString));
	m_sOnDestroyFX = szString;

	LOAD_FLOAT(m_fHitPoints);
	LOAD_FLOAT(m_fMaxHitPoints);
	LOAD_bool(m_bCanDamage);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::ReadProp()
//              
//	PURPOSE:	RETAIL: Tron's (Object.lto 0x1013ba20)
//              
//----------------------------------------------------------------------------
void PowerArmor::ReadProp(ObjectCreateStruct *pStruct)
{
	if ( !pStruct ) return;

	GenericProp genProp;

	if ( g_pLTServer->GetPropGeneric( "OnDestroyFX", &genProp ) == LT_OK )
	{
		m_sOnDestroyFX = genProp.m_String;
	}

	if ( g_pLTServer->GetPropGeneric( "HitPoints", &genProp ) == LT_OK )
	{
		m_fMaxHitPoints = m_fHitPoints = genProp.m_Float;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	PowerArmor::HandleProjectileImpact()
//              
//	PURPOSE:	RETAIL: Tron's (Object.lto 0x1013bb40)
//              
//----------------------------------------------------------------------------
bool PowerArmor::HandleProjectileImpact( CProjectile* pProjectile, CAttachmentPosition* pAttachPos )
{
	if ( !pProjectile || !pAttachPos ) return false;

	if ( m_eState != kPA_On ) return false;

	GameDifficulty eDifficulty = g_pGameServerShell->GetDifficulty();
	if ( eDifficulty == GD_EASY || eDifficulty == GD_NORMAL )
	{
		for ( int i = 0; i < ARRAY_LEN(s_aeArmorProofDamageTypes); i++ )
		{
			if ( pProjectile->m_eInstDamageType == s_aeArmorProofDamageTypes[i] )
			{
				if ( m_bCanDamage )
				{
					PlayClientFX( (char*)m_sOnDestroyFX.c_str(), m_hObject );
					CAttachment::Destroy( pAttachPos->GetAttachment() );
					pAttachPos->SetAttachment( LTNULL );
				}

				return true;
			}
		}
	}

	LTFLOAT fDamage = pProjectile->m_fInstDamage;
	pProjectile->ScaleTronDamage( &fDamage );
	pProjectile->ScaleTronDamage101( &fDamage );
	pProjectile->ScaleTronDamage105( &fDamage );

	// RETAIL: a second damage the projectile has when its 0x7c flag is set:
	// its modifier 9 damage times its duration (0x84 times 0x80)
	LTFLOAT fExtraDamage = 0.0f;
	if ( pProjectile->m_bTronUnknown7C )
	{
		fExtraDamage = pProjectile->m_fTronUnknown84 * pProjectile->m_fTronUnknown80;
	}

	LTFLOAT fHitPoints = GetHitPoints() - ( fDamage + fExtraDamage );
	if ( fHitPoints <= 0.0f )
	{
		fHitPoints = 0.0f;
	}

	fDamage = fDamage - GetHitPoints();

	if ( m_bCanDamage )
	{
		m_fHitPoints = fHitPoints;
	}

	if ( m_fHitPoints <= 0.0f )
	{
		PlayClientFX( (char*)m_sOnDestroyFX.c_str(), m_hObject );
		CAttachment::Destroy( pAttachPos->GetAttachment() );
		pAttachPos->SetAttachment( LTNULL );
	}

	if ( fDamage < 0.0f )
	{
		fDamage = 0.0f;
	}

	pProjectile->m_fInstDamage = fDamage;

	return true;
}

BEGIN_CLASS(OccludingArmor)
END_CLASS_DEFAULT_FLAGS(OccludingArmor, Prop, NULL, NULL, CF_HIDDEN)

// RETAIL: 0x102faa28, its default constructor 0x1013ca10
BEGIN_CLASS(ShellArmor)
END_CLASS_DEFAULT_FLAGS(ShellArmor, OccludingArmor, NULL, NULL, CF_HIDDEN)



//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::OccludingArmor()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
OccludingArmor::OccludingArmor()
{
	// Construct
	m_hHitBox = NULL;
	m_hParentObject = NULL;
	m_hCurrentAnimation = INVALID_MODEL_ANIM;
	m_eModelId = eModelIdInvalid;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::~OccludingArmor()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ OccludingArmor::~OccludingArmor()
{
	// Destruct

	if (m_hHitBox)
	{
        g_pLTServer->RemoveObject(m_hHitBox);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::EngineMessageFn()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ uint32 OccludingArmor::EngineMessageFn(uint32 messageID,void *pData, LTFLOAT lData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (lData == PRECREATE_WORLDFILE || lData == PRECREATE_STRINGPROP)
			{
				ReadShieldProps(static_cast<ObjectCreateStruct*>(pData));
			}
		}
		break;

		case MID_OBJECTCREATED:
		{
			if (lData != OBJECTCREATED_SAVEGAME  )
			{
				CreateHitBox();
			}

			// The Armor is NOT deleted when is life is expired.  We want to
			// handle that manually on our level.  Attachments which delete
			// themselves don't clean up references properly.
			m_damage.m_bRemoveOnDeath = LTFALSE;

			// RETAIL: Tron's (0x1013bfb7): update from the next frame on and
			// never deactivate
			SetNextUpdate(0.001f);
			m_bCanDeactivate = false;
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save(static_cast<ILTMessage_Write*>(pData));
		}
		break;

		case MID_LOADOBJECT:
		{
            Load(static_cast<ILTMessage_Read*>(pData));
		}
		break;

		case MID_MODELSTRINGKEY:
		{
			HandleModelString( static_cast<ArgList*>(pData) );
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;
	}

	return ( Prop::EngineMessageFn(messageID, pData, lData) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::ObjectMessageFn()
//              
//	PURPOSE:	handles the damage itself -- does 
//              
//----------------------------------------------------------------------------
/*virtual*/ uint32 OccludingArmor::ObjectMessageFn(HOBJECT hSender,ILTMessage_Read *pMsg)
{
	static const char cKeyDerezz[] = "REMOVE";

	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch ( messageID )
	{
		case MID_TRIGGER:
		{
			// If this is an attach message, then the sender is the owner.
			const char* szMsg = reinterpret_cast<const char*>(pMsg->Readuint32());
			if ( 0 == strcmp( KEY_ATTACH, szMsg ) )
			{
				m_hParentObject = hSender;
			}

			if ( 0 == strcmp( cKeyDerezz, szMsg ) )
			{
				HandleDeath();
			}
		}
		break;

		case MID_DAMAGE:
		{
			// Intentionally NEVER pass MID_Damage to the base classes.  Prop
			// currently automaticly deletes the instance if passed down 
			// through the HandleDestroy() function.
			if ( CanDamage() )
			{
				GameBase::ObjectMessageFn(hSender, pMsg);

				if ( m_damage.IsDead() )
				{
					HandleDeath();
				}
			}
			return ( 0 );
		}
		break;

		// If we are told to play an animation, then just start playing it.  
		// This object is going to function pretty much entirely on KeyStrings
		case MID_ATTACHMENTANIM:
		{
			char szAnimationName[128];
			pMsg->ReadHStringAsString( szAnimationName, sizeof(szAnimationName));	

			SetAnimation(szAnimationName);
		}
		break;
	}

	return ( Prop::ObjectMessageFn(hSender, pMsg) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::Save()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void OccludingArmor::Save(ILTMessage_Write *pMsg)
{
	// 
	SAVE_HOBJECT(m_hHitBox);
	SAVE_HOBJECT(m_hParentObject);

	SAVE_DWORD(m_hCurrentAnimation);
	SAVE_BYTE(m_eShieldState);
	SAVE_BYTE(m_eModelId);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::Load()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void OccludingArmor::Load(ILTMessage_Read *pMsg)
{
	// 
	LOAD_HOBJECT(m_hHitBox);
	LOAD_HOBJECT(m_hParentObject);

	LOAD_DWORD(m_hCurrentAnimation);
	LOAD_BYTE_CAST(m_eShieldState, eShieldState);
	LOAD_BYTE_CAST(m_eModelId, ModelId);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::ReadShieldProps()
//              
//	PURPOSE:	Read the additional fields the OccludingArmor cares about.
//              
//----------------------------------------------------------------------------
void OccludingArmor::ReadShieldProps(ObjectCreateStruct* pData)
{
	GenericProp genProp;
    if (g_pLTServer->GetPropGeneric("ModelId", &genProp) == LT_OK)
	{
		AIASSERT( genProp.m_String[0], m_hObject, "OccludingArmor requires a ModelId to set a skeleton"  );
		if(genProp.m_String[0])
		{
			m_eModelId = g_pModelButeMgr->GetModelId(genProp.m_String);
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::GetAttachments()
//              
//	PURPOSE:	OccludingArmor does not have attachments, so return NULL when
//				queried for them.
//              
//----------------------------------------------------------------------------
/*virtual*/ CAttachments* OccludingArmor::GetAttachments()
{
	return ( NULL );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::GetModelSkeleton()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ ModelSkeleton OccludingArmor::GetModelSkeleton() const
{
	return ( g_pModelButeMgr->GetModelSkeleton(m_eModelId) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::SetModelNodeLastHit()
//              
//	PURPOSE:	Ignore the the LastNodeHit currently(?)
//              
//----------------------------------------------------------------------------
/*virtual*/ void OccludingArmor::SetModelNodeLastHit(ModelNode eModelNode)
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::UsingHitDetection()
//              
//	PURPOSE:	This object ALWAYS uses HitDetection.	
//              
//----------------------------------------------------------------------------
/*virtual*/ LTBOOL OccludingArmor::UsingHitDetection() const
{
	return ( LTTRUE );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::GetParentTransform()
//              
//	PURPOSE:	If now parent was specified, return a cleared LTransform.  If 
//				a parent was specified, then return the 'correct' transform
//              
//----------------------------------------------------------------------------
/*virtual*/ HATTACHMENT OccludingArmor::GetAttachment( ) const
{
	HATTACHMENT hAttachment;
	LTRESULT  ltResult = g_pLTServer->FindAttachment( m_hParentObject, m_hObject, &hAttachment );
	UBER_ASSERT( ltResult==LT_OK, "Failed to find attachment");
	return ( hAttachment );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::ComputeDamageModifier()
//              
//	PURPOSE:	Returns the modifier for the node hit.
//              
//----------------------------------------------------------------------------
/*virtual*/ LTFLOAT OccludingArmor::ComputeDamageModifier(ModelNode eModelNode)
{
	// 
	return ( 1.0f ); 
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::GetNodeRadius()
//              
//	PURPOSE:	Hack the node radii depending on the state of the AI.  When
//				the armor is up, the root node needs to have a huge radius, 
//				and the other nodes need to have none(?)
//              
//----------------------------------------------------------------------------
/*virtual*/ float OccludingArmor::GetNodeRadius(ModelSkeleton eModelSkeleton,ModelNode eModelNode)
{
	return ( g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, eModelNode) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::TronUnknownVirtual24()
//              
//	PURPOSE:	RETAIL: 0x1013c3e0: add the object the armor is attached to
//				(as a hit box user) to a list, as CThorne does with its ball
//              
//----------------------------------------------------------------------------
/*virtual*/ void OccludingArmor::TronUnknownVirtual24(int nArg)
{
	std::list<IHitBoxUser*>* pList = (std::list<IHitBoxUser*>*)nArg;

	IHitBoxUser* pUser = dynamic_cast<IHitBoxUser*>(g_pLTServer->HandleToObject(m_hParentObject));
	if (pUser)
	{
		pList->push_back(pUser);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::Update()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void OccludingArmor::Update()
{
	// RETAIL: Tron follows the parent object (0x1013c460)
	SetNextUpdate(0.001f);

	CCharacterHitBox* pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(m_hHitBox));
	if( !pHitBox || !m_hParentObject )
	{
		return;
	}

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hParentObject, &vPos);
	g_pLTServer->SetObjectPos(m_hObject, &vPos);

	LTVector vDims;
	g_pPhysicsLT->GetObjectDims(m_hParentObject, &vDims);

	// RETAIL: the armor and its hit box both get the parent's dims
	// grown by 16 (0x1013c4f6)
	vDims = LTVector( vDims.x + 16.0f, vDims.y + 16.0f, vDims.z + 16.0f );
	g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);
	g_pPhysicsLT->SetObjectDims(m_hHitBox, &vDims, 0);

	uint32 dwFlags;
	g_pModelLT->GetPlaybackState(m_hObject, MAIN_TRACKER, dwFlags );

	pHitBox->Update();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::CreateHitBox()
//              
//	PURPOSE:	Created a new hitbox for this object
//              
//----------------------------------------------------------------------------
void OccludingArmor::CreateHitBox()
{
	UBER_ASSERT(m_hHitBox==NULL, "Expected HitBox to be NULL" );

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	HCLASS hClass = g_pLTServer->GetClass("CCharacterHitBox");
	UBER_ASSERT(hClass!=NULL, "GetClass CCharacterHitBox failed" );
	if (!hClass)
	{
		return;
	}

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);
	theStruct.m_Pos = vPos;
	g_pLTServer->GetObjectRotation(m_hObject, &theStruct.m_Rotation);

	// Allocate an object...

	CCharacterHitBox* pHitBox = (CCharacterHitBox *)g_pLTServer->CreateObject(hClass, &theStruct);
	UBER_ASSERT(pHitBox!=NULL, "CCharacterHitBox allocation failed" );
	if (!pHitBox)
	{
		return;
	}

	m_hHitBox = pHitBox->m_hObject;
	pHitBox->Init(m_hObject, this);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ShellArmor::HandleDeath()
//              
//	PURPOSE:	Handles all of the trigger message dispatching which goes off
//				when the OccludingArmor dies.
//              
//----------------------------------------------------------------------------
void ShellArmor::HandleDeath()
{
	// Do a pretty effect, and become invisible and unshootable
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, 0, FLAG_TOUCH_NOTIFY | FLAG_SOLID | FLAG_RAYHIT | FLAG_VISIBLE);
	m_damage.SetCanDamage(LTFALSE);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::HandleModelString()
//              
//	PURPOSE:	Handles strings passed in through the animation.
//              
//----------------------------------------------------------------------------
void OccludingArmor::HandleModelString(ArgList* pArgList)
{
	AIASSERT( pArgList, m_hObject, ""  );
	AIASSERT( pArgList->argc > 0, m_hObject, ""  );
	AIASSERT( pArgList->argv[0], m_hObject, ""  );

	const char* const szKey = pArgList->argv[0];

	if ( 0 == strcmp( szKey, "STATE" ) )
	{
		AIASSERT( pArgList->argv[1], m_hObject, ""  );
		const char* const szState = pArgList->argv[1];
		
		if ( 0 == strcmp( szState, "ATTACK" ) )
		{
			SetState( eShieldAttack );
		}
		else if ( 0 == strcmp( szState, "BLOCK" ) )
		{
			SetState( eShieldBlock );
		}
		else if ( 0 == strcmp( szState, "NORMAL" ) )
		{
			SetState( eShieldNormal );
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::SetState()
//              
//	PURPOSE:	Unified location where the armors state can be changed for 
//				making debugging a bit easier.
//              
//----------------------------------------------------------------------------
void OccludingArmor::SetState( eShieldState eNewState )
{
	m_eShieldState = eNewState;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::SetAnimation()
//              
//	PURPOSE:	Sets an animation (or resets the animation) based on the name 
//				of an animation string passed in.  Keeps a reference to the 
//				selected to play and clears the state.  If the passed in 
//				animation is also the current animation, then reset it.
//
//----------------------------------------------------------------------------
void OccludingArmor::SetAnimation(const char* const szAnimationName)
{
	LTRESULT ltResult;

	HMODELANIM hAnim;
	ltResult = g_pModelLT->GetAnimIndex( m_hObject, const_cast<char*>(szAnimationName), hAnim );
	AIASSERT1( ltResult==LT_OK, m_hObject, "Failed to find animation %s", szAnimationName );

	if ( m_hCurrentAnimation == hAnim )
	{
		g_pModelLT->ResetAnim( m_hObject, MAIN_TRACKER );
		AIASSERT( ltResult==LT_OK, m_hObject, "Failed to Reset the animation" );
	}
	else
	{
		g_pModelLT->SetCurAnim( m_hObject, MAIN_TRACKER, hAnim );
		AIASSERT( ltResult==LT_OK, m_hObject, "Failed to set the animation" );
	}

	g_pModelLT->SetLooping( m_hObject, MAIN_TRACKER, false );

	// Remember the animation currently playing.
	m_hCurrentAnimation = hAnim;

	// Clear the state, just in case we missed any clean up keys.
	SetState( eShieldNormal );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	OccludingArmor::CanDamage()
//              
//	PURPOSE:	Returns true if the armor can currently be damaged, false if 
//				it cannot.
//              
//----------------------------------------------------------------------------
LTBOOL OccludingArmor::CanDamage() const
{
	if ( m_eShieldState != eShieldAttack )
	{
		// If we are not in the attack state, then we are undamageable.
		return ( LTFALSE );
	}

	if ( m_damage.IsDead() )
	{
		// If we are dead, we can't be more damaged
		return ( LTFALSE );
	}

	return ( LTTRUE );
}