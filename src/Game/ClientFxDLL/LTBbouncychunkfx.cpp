//------------------------------------------------------------------
//
//   MODULE  : BOUNCYCHUNKFX.CPP
//
//   PURPOSE : Implements class CLTBBouncyChunkFX
//
//   CREATED : On 12/3/98 At 6:34:44 PM
//
//------------------------------------------------------------------

// RETAIL: Tron's version of this file is not in any source release. This
// TU (0x1000f670..0x10011a30 in ClientFx.fxd) is written from the retail
// code; NOLF2's file gave the names and the parts that did not change.

// Includes....

#include "stdafx.h"
#include "LTBBouncyChunkFX.h"
#include "ClientFX.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLTBBouncyChunkProps::CBouncyChunkProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CLTBBouncyChunkProps::CLTBBouncyChunkProps() :
	m_bPlayImpactSound(false),
	m_vGravity(0.0f, 0.0f, 0.0f),
	m_fGravityAmount(0.0f),
	m_fChunkSpeed(0.0f),
	m_fChunkSpread(0.0f),
	m_vChunkDir(0.0f, 0.0f, 0.0f),
	m_vRandDirMin(0.0f, 0.0f, 0.0f),
	m_vRandDirMax(0.0f, 0.0f, 0.0f)
{
	m_sModelName[0] = '\0';
	m_sImpactSound[0] = '\0';

	m_sSkinName[0][0] = '\0';
	m_sSkinName[1][0] = '\0';
	m_sSkinName[2][0] = '\0';
	m_sSkinName[3][0] = '\0';
	m_sSkinName[4][0] = '\0';
	m_sRenderStyle[0][0] = '\0';
	m_sRenderStyle[1][0] = '\0';
	m_sRenderStyle[2][0] = '\0';
	m_sRenderStyle[3][0] = '\0';

	m_fRandomSpeedBiasMin = m_fRandomSpeedBiasMax = 0.0f;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLTBBouncyChunkProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FxED
//
// ----------------------------------------------------------------------- //

bool CLTBBouncyChunkProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
{
	if(!CBaseFXProps::ParseProperties(pProps, nNumProps))
		return false;

	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pProps[nCurrProp];

		if (!stricmp(fxProp.m_sName, "Model"))
		{
			char sTmp[128];
			strcpy(sTmp, fxProp.m_data.m_sVal);

			// Get the path name

			char *sExt  = strtok(sTmp, "|");
			char *sPath = strtok(NULL, "|");
			if (sPath) strcpy(m_sModelName, sPath);
		}
		else if (!stricmp(fxProp.m_sName, "Skin"))
		{
			char sTmp[128];
			strcpy(sTmp, fxProp.m_data.m_sVal);

			// Get the path name

			char *sExt  = strtok(sTmp, "|");
			char *sPath = strtok(NULL, "|");
			if (sPath) strcpy(m_sSkinName[0], sPath);
		}
		else if( !stricmp( fxProp.m_sName, "Skin0" ))
		{
			fxProp.GetPath( m_sSkinName[0] );
		}
		else if( !stricmp( fxProp.m_sName, "Skin1" ))
		{
			fxProp.GetPath( m_sSkinName[1] );
		}
		else if( !stricmp( fxProp.m_sName, "Skin2" ))
		{
			fxProp.GetPath( m_sSkinName[2] );
		}
		else if( !stricmp( fxProp.m_sName, "Skin3" ))
		{
			fxProp.GetPath( m_sSkinName[3] );
		}
		else if( !stricmp( fxProp.m_sName, "Skin4" ))
		{
			fxProp.GetPath( m_sSkinName[4] );
		}
		else if( !stricmp( fxProp.m_sName, "RenderStyle0" ))
		{
			fxProp.GetPath( m_sRenderStyle[0] );
		}
		else if( !stricmp( fxProp.m_sName, "RenderStyle1" ))
		{
			fxProp.GetPath( m_sRenderStyle[1] );
		}
		else if( !stricmp( fxProp.m_sName, "RenderStyle2" ))
		{
			fxProp.GetPath( m_sRenderStyle[2] );
		}
		else if( !stricmp( fxProp.m_sName, "RenderStyle3" ))
		{
			fxProp.GetPath( m_sRenderStyle[3] );
		}
		else if (!stricmp(fxProp.m_sName, "ChunkSound"))
		{
			char sTmp[128];
			strcpy(sTmp, fxProp.m_data.m_sVal);

			// Get the path name

			char *sExt  = strtok(sTmp, "|");
			char *sPath = strtok(NULL, "|");
			if (sPath)
			{
				strcpy(m_sImpactSound, sPath);
				m_bPlayImpactSound = true;
			}
		}
		else if (!stricmp(fxProp.m_sName, "Amount"))
		{
			m_fGravityAmount = fxProp.m_data.m_fVal;
		}
		else if (!stricmp(fxProp.m_sName, "Gravity"))
		{
			m_vGravity.x = fxProp.m_data.m_fVec[0];
			m_vGravity.y = fxProp.m_data.m_fVec[1];
			m_vGravity.z = fxProp.m_data.m_fVec[2];
			m_vGravity.Norm();
		}
		else if (!stricmp(fxProp.m_sName, "ChunkSpeed"))
		{
			m_fChunkSpeed = fxProp.m_data.m_fVal;
		}
		else if (!stricmp(fxProp.m_sName, "ChunkDir"))
		{
			m_vChunkDir.x = fxProp.m_data.m_fVec[0];
			m_vChunkDir.y = fxProp.m_data.m_fVec[1];
			m_vChunkDir.z = fxProp.m_data.m_fVec[2];
			m_vChunkDir.Norm();
		}
		else if (!stricmp(fxProp.m_sName, "RandDirMin"))
		{
			m_vRandDirMin.x = fxProp.m_data.m_fVec[0];
			m_vRandDirMin.y = fxProp.m_data.m_fVec[1];
			m_vRandDirMin.z = fxProp.m_data.m_fVec[2];
		}
		else if (!stricmp(fxProp.m_sName, "RandDirMax"))
		{
			m_vRandDirMax.x = fxProp.m_data.m_fVec[0];
			m_vRandDirMax.y = fxProp.m_data.m_fVec[1];
			m_vRandDirMax.z = fxProp.m_data.m_fVec[2];
		}
		else if (!stricmp(fxProp.m_sName, "RandomSpeedBiasMin"))
		{
			m_fRandomSpeedBiasMin = fxProp.m_data.m_fVal;
		}
		else if (!stricmp(fxProp.m_sName, "RandomSpeedBiasMax"))
		{
			m_fRandomSpeedBiasMax = fxProp.m_data.m_fVal;
		}
		else if (!stricmp(fxProp.m_sName, "ChunkSpread"))
		{
			m_fChunkSpread = fxProp.m_data.m_fVal;
		}
	}

	m_vGravity *= m_fGravityAmount;

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : CLTBBouncyChunkFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CLTBBouncyChunkFX::CLTBBouncyChunkFX()
:	CBaseFX				( CBaseFX::eLTBBouncyChunkFX ),
	m_hBouncyChunk		( LTNULL ),
	m_hImpactSound		( LTNULL )
{

}

//------------------------------------------------------------------
//
//   FUNCTION : ~CLTBBouncyChunkFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CLTBBouncyChunkFX::~CLTBBouncyChunkFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : GetSpawnTransform()
//
//   PURPOSE  : RETAIL: Tron's (0x1000ff20). Puts vPos and rRot where
//				the follow type of the effect puts the chunk. The name
//				is ours.
//
//------------------------------------------------------------------

void CLTBBouncyChunkFX::GetSpawnTransform(LTVector &vPos, LTRotation &rRot)
{
	ILTModel *pModelLT = m_pLTClient->GetModelLT();

	LTVector vOffset = GetProps()->m_vOffset + m_vTronUnknown;
	LTVector vAlignedOffset = vOffset;

	switch (GetProps()->m_nFollowType)
	{
		case UP_FIXED :
		{
			vPos = m_vCreatePos + vOffset;
		}
		break;

		case UP_FOLLOW :
		{
			if (m_hParent)
			{
#ifdef TRON20SRC_MODERN
				// MODERN.md GAME-119: at the parent, plus the offset
				m_pLTClient->GetObjectPos(m_hParent, &vPos);
#else
				// RETAIL: retail reads the parent's position into m_vPos,
				// not into vPos.
				m_pLTClient->GetObjectPos(m_hParent, &m_vPos);
#endif
				vPos += vOffset;
			}
		}
		break;

		case UP_PLAYERVIEW :
		{
			// Let game code set our position.
		}
		break;

		case UP_NODEATTACH :
		{
			LTransform lTrans;
			HMODELNODE hNode;

			if( LT_OK == pModelLT->GetNode( m_hParent, GetProps()->m_szAttach, hNode ) )
			{
				if( LT_OK == pModelLT->GetNodeTransform( m_hParent, hNode, lTrans, LTTRUE ) )
				{
					vPos	= lTrans.m_Pos + vAlignedOffset;
					rRot	= lTrans.m_Rot;
				}
			}
		}
		break;

		case UP_SOCKETATTACH :
		{
			LTransform		lTrans;
			HMODELSOCKET	hSocket;

			if( LT_OK == pModelLT->GetSocket( m_hParent, GetProps()->m_szAttach, hSocket ) )
			{
				if( LT_OK == pModelLT->GetSocketTransform( m_hParent, hSocket, lTrans, LTTRUE ) )
				{
					vPos	= lTrans.m_Pos + vAlignedOffset;
					rRot	= lTrans.m_Rot;
				}
			}
			else
			{
				if( GetAttachmentSocketTransform( m_hParent, GetProps()->m_szAttach, lTrans.m_Pos, lTrans.m_Rot ))
				{
					vPos	= lTrans.m_Pos + vAlignedOffset;
					rRot	= lTrans.m_Rot;
				}
			}
		}
		break;

		case UP_PVNODEATTACH :
		{
			LTransform lTrans;
			HMODELNODE hNode;

			if( LT_OK == pModelLT->GetNode( m_hParent, GetProps()->m_szAttach, hNode ) )
			{
				if( LT_OK == pModelLT->GetNodeTransform( m_hParent, hNode, lTrans, LTFALSE ) )
				{
					//grab the position of the object to compensate for offset
					LTVector vObjectPos;
					m_pLTClient->GetObjectPos(m_hParent, &vObjectPos);

					vPos	= lTrans.m_Pos + vAlignedOffset + vObjectPos;
					rRot	= lTrans.m_Rot;
				}
			}
		}
		break;

		case UP_PVSOCKETATTACH:
		{
			LTransform		lTrans;
			HMODELSOCKET	hSocket;

			if( LT_OK == pModelLT->GetSocket( m_hParent, GetProps()->m_szAttach, hSocket ) )
			{
				if( LT_OK == pModelLT->GetSocketTransform( m_hParent, hSocket, lTrans, LTFALSE ) )
				{
					//grab the position of the object to compensate for offset
					LTVector vObjectPos;
					m_pLTClient->GetObjectPos(m_hParent, &vObjectPos);

					vPos	= lTrans.m_Pos + vAlignedOffset + vObjectPos;
					rRot	= lTrans.m_Rot;
				}
			}
		}
		break;
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CLTBBouncyChunkFX
//
//------------------------------------------------------------------

bool CLTBBouncyChunkFX::Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	// Perform base class initialisation

	if (!CBaseFX::Init(pClientDE, pBaseData, pProps))
		return false;

	// RETAIL: the chunk is thrown by the first Update (0x10010980)

	m_bFirstUpdate = true;

	LTVector vPos = m_vCreatePos;
	LTRotation rRot = m_rCreateRot;

	GetSpawnTransform(vPos, rRot);

	float scale;
	CalcScale(m_tmElapsed, GetProps()->m_tmLifespan, &scale);

	LTVector vScale(scale, scale, scale);

	ObjectCreateStruct ocs;
	INIT_OBJECTCREATESTRUCT(ocs);

	ocs.m_ObjectType		= OT_MODEL;
	ocs.m_Flags				= FLAG_NOLIGHT | FLAG_VISIBLE;
	ocs.m_Pos				= vPos;
	ocs.m_Rotation			= rRot;
	ocs.m_Scale				= vScale;
	strcpy(ocs.m_Filename, GetProps()->m_sModelName);

	SAFE_STRCPY( ocs.m_SkinNames[0], GetProps()->m_sSkinName[0] );
	SAFE_STRCPY( ocs.m_SkinNames[1], GetProps()->m_sSkinName[1] );
	SAFE_STRCPY( ocs.m_SkinNames[2], GetProps()->m_sSkinName[2] );
	SAFE_STRCPY( ocs.m_SkinNames[3], GetProps()->m_sSkinName[3] );
	SAFE_STRCPY( ocs.m_SkinNames[4], GetProps()->m_sSkinName[4] );

	SAFE_STRCPY( ocs.m_RenderStyleNames[0], GetProps()->m_sRenderStyle[0] );
	SAFE_STRCPY( ocs.m_RenderStyleNames[1], GetProps()->m_sRenderStyle[1] );
	SAFE_STRCPY( ocs.m_RenderStyleNames[2], GetProps()->m_sRenderStyle[2] );
	SAFE_STRCPY( ocs.m_RenderStyleNames[3], GetProps()->m_sRenderStyle[3] );

	m_hBouncyChunk = m_pLTClient->CreateObject(&ocs);

	// Hide the chunk until the first update throws it

	m_pLTClient->Common()->SetObjectFlags(m_hBouncyChunk, OFT_Flags, 0, FLAG_VISIBLE);

	// Create the base object

	CreateDummyObject();

	// Success !!

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CLTBBouncyChunkFX
//
//------------------------------------------------------------------

void CLTBBouncyChunkFX::Term()
{
	if (m_hBouncyChunk) m_pLTClient->RemoveObject(m_hBouncyChunk);
	m_hBouncyChunk = NULL;

	if (m_hObject) m_pLTClient->RemoveObject(m_hObject);
	m_hObject = NULL;

	if (m_hImpactSound)
	{
		m_pLTClient->SoundMgr()->KillSound(m_hImpactSound);
		m_hImpactSound = NULL;
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates class CLTBBouncyChunkFX
//
//------------------------------------------------------------------

bool CLTBBouncyChunkFX::Update(float tmFrameTime)
{
	// Base class update first

	if (!CBaseFX::Update(tmFrameTime))
		return false;

	if (m_bFirstUpdate)
	{
		// RETAIL: Tron's. Throw the chunk in a random direction, turned
		// by the spawn rotation.

		LTVector vPos = m_vCreatePos;
		LTRotation rRot = m_rCreateRot;

		GetSpawnTransform(vPos, rRot);

		float fSpeed = GetRandom(GetProps()->m_fRandomSpeedBiasMin, GetProps()->m_fRandomSpeedBiasMax) + GetProps()->m_fChunkSpeed;

		LTVector vDir(GetRandom(GetProps()->m_vRandDirMin.x, GetProps()->m_vRandDirMax.x),
					  GetRandom(GetProps()->m_vRandDirMin.y, GetProps()->m_vRandDirMax.y),
					  GetRandom(GetProps()->m_vRandDirMin.z, GetProps()->m_vRandDirMax.z));

		if (vDir.z + vDir.y + vDir.x == 0.0f)
			vDir.z = 1.0f;

		vDir /= vDir.Mag();

		// Turn the direction by the rotation (q * v * ~q)

		float tx =  rRot.m_Quat[1] * vDir.z + rRot.m_Quat[3] * vDir.x - rRot.m_Quat[2] * vDir.y;
		float ty =  rRot.m_Quat[3] * vDir.y - rRot.m_Quat[0] * vDir.z + rRot.m_Quat[2] * vDir.x;
		float tz =  rRot.m_Quat[3] * vDir.z + rRot.m_Quat[0] * vDir.y - rRot.m_Quat[1] * vDir.x;
		float tw = -rRot.m_Quat[0] * vDir.x - rRot.m_Quat[1] * vDir.y - rRot.m_Quat[2] * vDir.z;

		float cx = -rRot.m_Quat[0];
		float cy = -rRot.m_Quat[1];
		float cz = -rRot.m_Quat[2];

		m_vVel.x = tw * cx + tx * rRot.m_Quat[3] + ty * cz - tz * cy;
		m_vVel.y = ty * rRot.m_Quat[3] - tx * cz + tw * cy + tz * cx;
		m_vVel.z = tw * cz + tz * rRot.m_Quat[3] + tx * cy - ty * cx;

		m_vVel = m_vVel * fSpeed;

		g_pLTClient->SetObjectPos(m_hBouncyChunk, &vPos, LTTRUE);
		g_pLTClient->SetObjectRotation(m_hBouncyChunk, &rRot);

		m_pLTClient->Common()->SetObjectFlags(m_hBouncyChunk, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);

		m_bFirstUpdate = false;
	}

	if ((m_hImpactSound) && (m_pLTClient->IsDone(m_hImpactSound)))
	{
		m_pLTClient->SoundMgr()->KillSound(m_hImpactSound);
		m_hImpactSound = NULL;
	}

	// Set the object scale

	LTVector vScale(m_scale, m_scale, m_scale);

	m_pLTClient->SetObjectScale(m_hBouncyChunk, &vScale);

	LTVector vCur;
	m_pLTClient->GetObjectPos(m_hBouncyChunk, &vCur);

	// Compute the new position of the chunk

	LTVector vNew = vCur;
	vNew += m_vVel * tmFrameTime;

	m_vVel += GetProps()->m_vGravity * tmFrameTime;

	// Move the object and collide against the world

	ClientIntersectQuery ciq;
	ClientIntersectInfo  cii;

	ciq.m_From  = vCur;
	ciq.m_To    = vNew;
	ciq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;

	if (m_pLTClient->IntersectSegment(&ciq, &cii))
	{
		vNew = cii.m_Point + cii.m_Plane.m_Normal;
		vCur = vNew;

		// Compute the reflected velocity

		LTVector N = cii.m_Plane.m_Normal;
		LTVector L = m_vVel;
		L.x = -L.x;
		L.y = -L.y;
		L.z = -L.z;

		LTVector vReflected = N * 2.0f;
		vReflected *= (N.Dot(L));
		vReflected -= L;

		vReflected.Norm();
		vReflected *= (m_vVel.Mag() * 0.7f);

		m_vVel = vReflected;

		const char *sImpactSound = GetProps()->m_sImpactSound;
		if (sImpactSound[0] != '.')
		{
			// Play the bounce sound

			PlaySoundInfo psi;
			memset(&psi, 0, sizeof(PlaySoundInfo));

			psi.m_dwFlags = PLAYSOUND_GETHANDLE |
							PLAYSOUND_CTRL_VOL |
							PLAYSOUND_CLIENT |
							PLAYSOUND_TIME |
							PLAYSOUND_3D |
							PLAYSOUND_REVERB;

			psi.m_nVolume = 50;

			strcpy(psi.m_szSoundName, GetProps()->m_sImpactSound);
			psi.m_nPriority		= 0;
			psi.m_vPosition		= m_vPos;
			psi.m_fInnerRadius	= 100;
			psi.m_fOuterRadius	= 300;

			if (!m_hImpactSound)
			{
				if (m_pLTClient->SoundMgr()->PlaySound(&psi, m_hImpactSound) == LT_OK)
				{
					m_hImpactSound = psi.m_hSound;
				}
			}
		}
	}

	m_pLTClient->SetObjectPos(m_hBouncyChunk, &vNew);
	m_pLTClient->SetObjectColor(m_hBouncyChunk, m_red, m_green, m_blue, m_alpha);
	m_pLTClient->SetObjectPos(m_hObject, &vNew);

	// Success !!

	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : fxGetBouncyChunkFXProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetLTBBouncyChunkProps(CFastList<FX_PROP> *pList)
{
	FX_PROP fxProp;

	float fVec[3];
	fVec[0] = 0.0f;
	fVec[1] = 1.0f;
	fVec[2] = 0.0f;

	// Add the base props

	AddBaseProps(pList);

	// Add all the props to the list

	fxProp.Path("Model", "ltb|...");
	pList->AddTail(fxProp);

	fxProp.Path("Skin", "dtx|...");
	pList->AddTail(fxProp);

	fxProp.Path( "Skin0", "dtx|..." );
	pList->AddTail(fxProp);

	fxProp.Path( "Skin1", "dtx|..." );
	pList->AddTail( fxProp );

	fxProp.Path( "Skin2", "dtx|..." );
	pList->AddTail( fxProp );

	fxProp.Path( "Skin3", "dtx|..." );
	pList->AddTail( fxProp );

	fxProp.Path( "Skin4", "dtx|..." );
	pList->AddTail( fxProp );

	fxProp.Vector("ChunkDir", fVec);
	pList->AddTail(fxProp);

	fxProp.Float("ChunkSpeed", 40.0f);
	pList->AddTail(fxProp);

	fxProp.Float("ChunkSpread", 40.0f);
	pList->AddTail(fxProp);

	fxProp.Path( "RenderStyle0", "ltb|..." );
	pList->AddTail( fxProp );

	fxProp.Path( "RenderStyle1", "ltb|..." );
	pList->AddTail( fxProp );

	fxProp.Path( "RenderStyle2", "ltb|..." );
	pList->AddTail( fxProp );

	fxProp.Path( "RenderStyle3", "ltb|..." );
	pList->AddTail( fxProp );

	fVec[0] = 0.0f;
	fVec[1] = -1.0f;
	fVec[2] = 0.0f;

	fxProp.Vector("Gravity", fVec);
	pList->AddTail(fxProp);

	fxProp.Float("Amount", 10.0f);
	pList->AddTail(fxProp);

	fxProp.Path("ChunkSound", "wav|...");
	pList->AddTail(fxProp);

	fVec[0] = 0.0f;
	fVec[1] = 0.0f;
	fVec[2] = 0.0f;

	fxProp.Vector("RandDirMin", fVec);
	pList->AddTail(fxProp);

	fxProp.Vector("RandDirMax", fVec);
	pList->AddTail(fxProp);

	fxProp.Float("RandomSpeedBiasMin", 0.0f);
	pList->AddTail(fxProp);

	fxProp.Float("RandomSpeedBiasMax", 0.0f);
	pList->AddTail(fxProp);
}
