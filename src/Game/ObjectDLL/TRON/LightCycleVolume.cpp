// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleVolume.cpp
//
// PURPOSE : A model (a wall, a ramp) on a light cycle grid that raises
//           itself when it is turned ON
//
// RETAIL: Tron 2.0's, written from retail Object.lto (LightCycleVolume.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LightCycleVolume.h"
#include "ParsedMsg.h"
#include "ServerUtilities.h"
#include "CommonUtilities.h"
#include <string>

// RETAIL: the animations, file statics at 0x103591d0, 0x103591c0,
// 0x103591b0 and 0x103591a0 (their initializers at 0x1002c6f0..0x1002cad0)
static std::string s_sUpNS("UpNS");
static std::string s_sUpEW("UpEW");
static std::string s_sRaiseNS("RaiseNS");
static std::string s_sRaiseEW("RaiseEW");

BEGIN_CLASS(LightCycleVolume)
	ADD_STRINGPROP_FLAG(ModelTemplate, "Default", PF_STATICLIST)
	ADD_VECTORPROP_VAL_FLAG(Dims, 4.0f, 32.0f, 128.0f, PF_HIDDEN | PF_DIMS | PF_LOCALDIMS)
END_CLASS_DEFAULT_FLAGS_PLUGIN(LightCycleVolume, GameBase, NULL, NULL, 0, LightCycleVolumeObjectPlugin)

CMDMGR_BEGIN_REGISTER_CLASS( LightCycleVolume )
CMDMGR_END_REGISTER_CLASS( LightCycleVolume, GameBase )

bool LightCycleVolumeObjectPlugin::sm_bInitted = false;

LightCycleVolumeObjectPlugin::LightCycleVolumeObjectPlugin()
{
	m_pModelButeMgrPlugin = new CModelButeMgrPlugin;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleVolumeObjectPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the ModelTemplate list with the models
//
// ----------------------------------------------------------------------- //

LTRESULT LightCycleVolumeObjectPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
	uint32* pcStrings,
	const uint32 cMaxStrings,
	const uint32 cMaxStringLength)
{
	if (!sm_bInitted)
	{
		m_pModelButeMgrPlugin->PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		sm_bInitted = true;
	}

	if (_stricmp("ModelTemplate", szPropName) == 0)
	{
		uint32 cModels = g_pModelButeMgr->GetNumModels();
		for (uint32 i = 0; i < cModels; i++)
		{
			strncpy(aszStrings[(*pcStrings)++], g_pModelButeMgr->GetModelName((ModelId)i), cMaxStringLength);
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleVolume::LightCycleVolume / ~LightCycleVolume
//
// ----------------------------------------------------------------------- //

LightCycleVolume::LightCycleVolume()
:	GameBase(OT_NORMAL),
	m_bRaised(false),
	m_hAnim(INVALID_MODEL_ANIM)
{
}

LightCycleVolume::~LightCycleVolume()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleVolume::OnPrecreate
//
//	PURPOSE:	Set up the model from its template; the model always
//				stands upright, its rotation only tells which way it faces
//
// ----------------------------------------------------------------------- //

uint32 LightCycleVolume::OnPrecreate(ObjectCreateStruct* pOCS, float precreateType)
{
	if (precreateType == PRECREATE_WORLDFILE || precreateType == PRECREATE_STRINGPROP)
	{
		if (!pOCS) return 0;

		GenericProp genProp;
		genProp.m_Type = LT_PT_ROTATION;
		genProp.m_Rotation = pOCS->m_cProperties.GetPropRotation("Rotation", LTRotation(0.0f, 0.0f, 0.0f));

		LTVector vForward = genProp.m_Rotation.Forward();
		if (fabs(vForward.z) < fabs(vForward.x))
		{
			m_bEastWest = 1;
		}
		else
		{
			m_bEastWest = 0;
		}

		genProp.m_Rotation = LTRotation();
		genProp.m_Type = LT_PT_ROTATION;
		pOCS->m_cProperties.AddProp("Rotation", genProp);

		m_eModelId = g_pModelButeMgr->GetModelId(pOCS->m_cProperties.GetPropString("ModelTemplate", LTNULL));

		const char* pFilename = g_pModelButeMgr->GetModelFilename(m_eModelId);
		if (pFilename && pFilename[0])
		{
			SAFE_STRCPY(pOCS->m_Filename, pFilename);
		}

		g_pModelButeMgr->CopySkinFilenames(m_eModelId, 0, pOCS->m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
		g_pModelButeMgr->CopyRenderStyleFilenames(m_eModelId, pOCS);

		pOCS->m_ObjectType = OT_MODEL;
		pOCS->m_Flags &= ~(FLAG_VISIBLE | FLAG_RAYHIT | FLAG_SOLID | FLAG_GRAVITY);
		pOCS->m_Flags |= FLAG_TOUCH_NOTIFY | FLAG_MODELKEYS;
	}

	return GameBase::OnPrecreate(pOCS, precreateType);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleVolume::OnObjectCreated
//
// ----------------------------------------------------------------------- //

uint32 LightCycleVolume::OnObjectCreated(float createType)
{
	SetNextUpdate(0.0f);
	MoveObjectToFloor(m_hObject);

	return GameBase::OnObjectCreated(createType);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleVolume::OnUpdate
//
//	PURPOSE:	Once raised, hold the Up animation
//
// ----------------------------------------------------------------------- //

uint32 LightCycleVolume::OnUpdate()
{
	if (m_bRaised)
	{
		SetNextUpdate(0.1f);

		if (g_pLTServer->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
		{
			SetAnimation(1);
			g_pModelLT->SetLooping(m_hObject, MAIN_TRACKER, true);
		}
	}

	return GameBase::OnUpdate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleVolume::OnSave / OnLoad
//
// ----------------------------------------------------------------------- //

uint32 LightCycleVolume::OnSave(ILTMessage_Write *pMsg, float dwSaveFlags)
{
	SAVE_BOOL(m_bRaised);
	SAVE_BYTE(m_eModelId);
	SAVE_BYTE(m_bEastWest);
	SAVE_DWORD(m_hAnim);

	return GameBase::OnSave(pMsg, dwSaveFlags);
}

uint32 LightCycleVolume::OnLoad(ILTMessage_Read *pMsg, float dwLoadFlags)
{
	LOAD_BOOL(m_bRaised);
	LOAD_BYTE_CAST(m_eModelId, ModelId);
	LOAD_BYTE(m_bEastWest);
	LOAD_DWORD(m_hAnim);

	return GameBase::OnLoad(pMsg, dwLoadFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleVolume::OnTrigger
//
//	PURPOSE:	ON raises the volume
//
// ----------------------------------------------------------------------- //

bool LightCycleVolume::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_On("ON");

	if (cMsg.GetArg(0) == s_cTok_On && !m_bRaised)
	{
		SetNextUpdate(0.1f);
		SetAnimation(0);
		m_bRaised = true;
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE | FLAG_SOLID, FLAG_VISIBLE | FLAG_SOLID);
	}

	return GameBase::OnTrigger(hSender, cMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleVolume::EngineMessageFn
//
//	PURPOSE:	Play the ClientFX the animations' keys ask for
//
// ----------------------------------------------------------------------- //

uint32 LightCycleVolume::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	if (messageID == MID_MODELSTRINGKEY)
	{
		ArgList* pArgList = (ArgList*)pData;
		if (pArgList->argc)
		{
			int i = 0;
			while (i < pArgList->argc)
			{
				if (!pArgList->argv[i++]) continue;

				if (_stricmp(pArgList->argv[0], "ClientFX") == 0 && pArgList->argc == 2)
				{
					PlayClientFX(pArgList->argv[1], m_hObject);
				}
			}
		}
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleVolume::SetAnimation
//
//	PURPOSE:	Play the Raise (0) or Up (1) animation for the way the
//				volume faces, with the animation's dims
//
// ----------------------------------------------------------------------- //

void LightCycleVolume::SetAnimation(uint32 nState)
{
	std::string sAnim;

	switch (nState)
	{
		case 0:
			if (m_bEastWest)
			{
				sAnim = s_sRaiseEW;
			}
			else
			{
				sAnim = s_sRaiseNS;
			}
			break;

		case 1:
			if (m_bEastWest)
			{
				sAnim = s_sUpEW;
			}
			else
			{
				sAnim = s_sUpNS;
			}
			break;
	}

	HMODELANIM hAnim;
	g_pModelLT->GetAnimIndex(m_hObject, sAnim.c_str(), hAnim);

	if (m_hAnim == hAnim)
	{
		g_pModelLT->ResetAnim(m_hObject, MAIN_TRACKER);
	}
	else
	{
		g_pModelLT->SetCurAnim(m_hObject, MAIN_TRACKER, hAnim);
	}

	g_pModelLT->SetLooping(m_hObject, MAIN_TRACKER, false);

	LTVector vDims;
	g_pCommonLT->GetModelAnimUserDims(m_hObject, &vDims, hAnim);
	g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);

	m_hAnim = hAnim;
}
