// ----------------------------------------------------------------------- //
//
// MODULE  : Thorne.cpp
//
// PURPOSE : Thorne, the boss, and the ball he throws
//
// RETAIL: Tron 2.0's, written from retail Object.lto (Thorne.h); its TU runs
//         0x100575e0..0x1005a2e0. Names of members and methods are ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Thorne.h"
#include "BossButeMgr.h"
#include "CharacterHitBox.h"
#include "CommandMgr.h"
#include "ParsedMsg.h"
#include "ServerSoundMgr.h"
#include "WeaponFireInfo.h"
#include "Weapon.h"

// RETAIL: the TU's constants (retail .rdata 0x10294c2c...)
static const char* const s_szAttach		= "ATTACH";
static const char s_szModelTemplate[]	= "ModelTemplate";
static const char s_szSummonCmd[]		= "SummonCmd";
static const char s_szTargetObject[]	= "TargetObject";
static const char s_szAmmo[]			= "Ammo";
static const char s_szWeapon[]			= "Weapon";

// RETAIL: Tron's SFX id of Thorne's special fx (60), and the character fx
// message its attach fx strings ride on (2); not in our SFXMsgIds.h
#define SFX_THORNE_ID				60
#define THORNEFX_ATTACHFX_MSG		2

// ----------------------------------------------------------------------- //
//
//	ThorneBall: RETAIL: ClassDef 0x102dda50 (hidden, no properties)
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(ThorneBall)
END_CLASS_DEFAULT_FLAGS(ThorneBall, ShellArmor, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	CThorne: RETAIL: ClassDef 0x102dda80, properties 0x10057690
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(CThorne)
	ADD_STRINGPROP_FLAG(ModelTemplate, "Default", PF_STATICLIST)
	ADD_STRINGPROP(SummonCmd, "")
	ADD_OBJECTPROP(TargetObject, "")
	ADD_STRINGPROP(Ammo, "")
	ADD_STRINGPROP(Weapon, "")
END_CLASS_DEFAULT_FLAGS_PLUGIN(CThorne, GameBase, NULL, NULL, 0, ThorneObjectPlugin)

CMDMGR_BEGIN_REGISTER_CLASS(CThorne)
CMDMGR_END_REGISTER_CLASS(CThorne, GameBase)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::CThorne
//
//	PURPOSE:	RETAIL: 0x100579b0
//
// ----------------------------------------------------------------------- //

CThorne::CThorne()
	: GameBase(OT_NORMAL)
	, m_eState(kThorneAnim_CreateBall)
	, m_fChargeStartTime(0.0f)
	, m_fChargeDuration(0.0f)
	, m_fChargePauseTime(0.0f)
	, m_bCharging(false)
	, m_eModelSkeleton(eModelSkeletonInvalid)
	, m_hHitBox(LTNULL)
	, m_bAnimDone(false)
	, m_hTargetObject(LTNULL)
	, m_hBall(LTNULL)
	, m_bAllowSummon(false)
	, m_nWeaponId(0)
	, m_nAmmoId(0)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::TronUnknownVirtual24
//
//	PURPOSE:	RETAIL: 0x10057ba0: add the ball (as a hit box user) to a list
//
// ----------------------------------------------------------------------- //

void CThorne::TronUnknownVirtual24(int nArg)
{
	HitBoxUserList* pList = (HitBoxUserList*)nArg;

	IHitBoxUser* pUser = dynamic_cast<IHitBoxUser*>(g_pLTServer->HandleToObject(m_hBall));
	if (pUser)
	{
		pList->push_back(pUser);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::~CThorne
//
//	PURPOSE:	RETAIL: 0x10057c30
//
// ----------------------------------------------------------------------- //

CThorne::~CThorne()
{
	if (m_hHitBox)
	{
		g_pLTServer->RemoveObject(m_hHitBox);
		m_hHitBox = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::EngineMessageFn
//
//	PURPOSE:	RETAIL: 0x10057d90
//
// ----------------------------------------------------------------------- //

uint32 CThorne::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch (messageID)
	{
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

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate(fData);
		}
		break;

		case MID_ALLOBJECTSCREATED:
		{
			AllObjectsCreated();
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_MODELSTRINGKEY:
		{
			ArgList* pArgList = (ArgList*)pData;

			CaselessString sKey;
			for (int i = 0; i < pArgList->argc; i++)
			{
				sKey += pArgList->argv[i];
				sKey += " ";
			}

			ConParse parse;
			parse.Init(sKey.c_str());

			while (g_pCommonLT->Parse(&parse) == LT_OK)
			{
				if (parse.m_nArgs && parse.m_Args[0])
				{
					CParsedMsg cParsedMsg(parse.m_nArgs, parse.m_Args);
					HandleModelString(m_hObject, cParsedMsg);
				}
			}
		}
		break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::Save
//
//	PURPOSE:	RETAIL: 0x10058040
//
// ----------------------------------------------------------------------- //

void CThorne::Save(ILTMessage_Write *pMsg)
{
	SAVE_BYTE(m_eState);
	SAVE_TIME(m_fChargeStartTime);
	SAVE_TIME(m_fChargePauseTime);
	SAVE_FLOAT(m_fChargeDuration);
	SAVE_bool(m_bCharging);
	SAVE_HOBJECT(m_hHitBox);
	SAVE_CHARSTRING(m_sSummonCmd.c_str());
	SAVE_BYTE(m_bAnimDone);
	SAVE_HOBJECT(m_hBall);
	SAVE_bool(m_bAllowSummon);
	SAVE_CHARSTRING(m_sTargetObject.c_str());
	m_Weapons.Save(pMsg, 0);
	SAVE_CHARSTRING(m_sWeapon.c_str());
	SAVE_CHARSTRING(m_sAmmo.c_str());
	SAVE_BYTE(m_nWeaponId);
	SAVE_BYTE(m_nAmmoId);
	SAVE_BYTE(m_eModelSkeleton);
	m_cs.Write(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::Load
//
//	PURPOSE:	RETAIL: 0x100581a0
//
// ----------------------------------------------------------------------- //

void CThorne::Load(ILTMessage_Read *pMsg)
{
	char szBuf[256];

	m_eState = pMsg->Readuint8();
	LOAD_TIME(m_fChargeStartTime);
	LOAD_TIME(m_fChargePauseTime);
	LOAD_FLOAT(m_fChargeDuration);
	LOAD_bool(m_bCharging);
	LOAD_HOBJECT(m_hHitBox);
	LOAD_CHARSTRING(szBuf, sizeof(szBuf));
	m_sSummonCmd = szBuf;
	LOAD_BYTE(m_bAnimDone);
	LOAD_HOBJECT(m_hBall);
	LOAD_bool(m_bAllowSummon);
	LOAD_CHARSTRING(szBuf, sizeof(szBuf));
	m_sTargetObject = szBuf;
	m_Weapons.Load(pMsg, 0);
	LOAD_CHARSTRING(szBuf, sizeof(szBuf));
	m_sWeapon = szBuf;
	LOAD_CHARSTRING(szBuf, sizeof(szBuf));
	m_sAmmo = szBuf;
	LOAD_BYTE(m_nWeaponId);
	LOAD_BYTE(m_nAmmoId);
	m_eModelSkeleton = (ModelSkeleton)pMsg->Readuint8();
	m_cs.Read(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::ReadProp
//
//	PURPOSE:	RETAIL: 0x100583f0
//
// ----------------------------------------------------------------------- //

void CThorne::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	const GenericPropList *pProps = &pStruct->m_cProperties;

	m_cs.eModelId = g_pModelButeMgr->GetModelId(pProps->GetPropString(s_szModelTemplate, LTNULL));
	m_eModelSkeleton = g_pModelButeMgr->GetModelSkeleton(m_cs.eModelId);

	m_sSummonCmd = pProps->GetPropString(s_szSummonCmd, "");

	// RETAIL: the test is inverted: a valid command is reported invalid
	// (docs/RETAIL_BUGS.md)
	if (g_pCmdMgr->IsValidCmd(m_sSummonCmd.c_str()))
	{
		ObjectCPrint(pStruct->m_Name, "Invalid summon command %s", m_sSummonCmd.c_str());
	}

	m_sTargetObject = pProps->GetPropString(s_szTargetObject, "");

	const AMMO* pAmmo = g_pWeaponMgr->GetAmmo(pProps->GetPropString(s_szAmmo, LTNULL));
	m_nAmmoId = pAmmo ? pAmmo->nId : 0;

	const WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(pProps->GetPropString(s_szWeapon, LTNULL));
	m_nWeaponId = pWeapon ? pWeapon->nId : 0;

	const char* pFilename = g_pModelButeMgr->GetModelFilename(m_cs.eModelId);
	if (pFilename && pFilename[0])
	{
		SAFE_STRCPY(pStruct->m_Filename, pFilename);
	}

	g_pModelButeMgr->CopySkinFilenames(m_cs.eModelId, 0, pStruct->m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
	g_pModelButeMgr->CopyRenderStyleFilenames(m_cs.eModelId, pStruct);

	pStruct->m_ObjectType = OT_MODEL;
	pStruct->m_Flags &= ~(FLAG_RAYHIT | FLAG_SOLID | FLAG_TOUCH_NOTIFY | FLAG_GRAVITY);
	pStruct->m_Flags |= FLAG_VISIBLE | FLAG_MODELKEYS;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::Update
//
//	PURPOSE:	RETAIL: 0x100586d0
//
// ----------------------------------------------------------------------- //

void CThorne::Update()
{
	if (!g_pLTServer->HandleToObject(m_hBall)) return;

	if (!m_hTargetObject)
	{
		HOBJECT hTarget;
		if (LT_OK == FindNamedObject(m_sTargetObject.c_str(), hTarget))
		{
			m_hTargetObject = hTarget;
		}
	}

	g_pLTServer->SetNextUpdate(m_hObject, 0.01f);

	CCharacterHitBox* pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(m_hHitBox));
	if (pHitBox)
	{
		pHitBox->Update();
	}

	if (!m_bAnimDone)
	{
		m_bAnimDone = (g_pLTServer->GetModelPlaybackState(m_hObject) & MS_PLAYDONE);
	}

	// RETAIL: the fire socket's transform, unused
	HMODELSOCKET hSocket;
	g_pModelLT->GetSocket(m_hObject, g_pBossButeMgr->GetThorne().sFireSocketName.c_str(), hSocket);
	LTransform tf;
	g_pModelLT->GetSocketTransform(m_hObject, hSocket, tf, LTTRUE);

	UpdateState();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::HandleModelString
//
//	PURPOSE:	RETAIL: 0x10058810: a model key's command
//
// ----------------------------------------------------------------------- //

void CThorne::HandleModelString(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Summon("SUMMON");
	static CParsedMsg::CToken s_cTok_Fire("FIRE");
	static CParsedMsg::CToken s_cTok_PlaySound("PLAYSOUND");
	static CParsedMsg::CToken s_cTok_ButeSound("BUTE_SOUND_KEY");
	static CParsedMsg::CToken s_cTok_CreateBall("CREATEBALL");

	if (cMsg.GetArg(0) == s_cTok_Fire)
	{
		Fire();
	}
	else if (cMsg.GetArg(0) == s_cTok_Summon)
	{
		g_pCmdMgr->Process(m_sSummonCmd.c_str(), g_pLTServer->HandleToObject(m_hObject), g_pLTServer->HandleToObject(LTNULL));
	}
	else if (cMsg.GetArg(0) == s_cTok_PlaySound || cMsg.GetArg(0) == s_cTok_ButeSound)
	{
		if (cMsg.GetArgCount() >= 2)
		{
			// PORT: Tron's PlaySoundFromObject takes one more argument (1.0f)
			g_pServerSoundMgr->PlaySoundFromObject(m_hObject, cMsg.GetArg(1), -1.0f, SOUNDPRIORITY_MISC_LOW, 0, 100, 1.0f, -1.0f, 0);
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_CreateBall)
	{
		CAutoMessage cClientMsg;
		cClientMsg.Writeuint8(MID_SFX_MESSAGE);
		cClientMsg.Writeuint8(SFX_THORNE_ID);
		cClientMsg.WriteObject(m_hObject);
		cClientMsg.Writeuint8(THORNEFX_ATTACHFX_MSG);
		cClientMsg.WriteString("SHOW_ATTACHFX 1");
		g_pLTServer->SendToClient(cClientMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::OnTrigger
//
//	PURPOSE:	RETAIL: 0x10058c00
//
// ----------------------------------------------------------------------- //

bool CThorne::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_AllowSummon("ALLOWSUMMON");

	if (cMsg.GetArg(0) == s_cTok_AllowSummon)
	{
		m_bAllowSummon = true;
	}

	return GameBase::OnTrigger(hSender, cMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::InitialUpdate
//
//	PURPOSE:	RETAIL: 0x10058ca0
//
// ----------------------------------------------------------------------- //

void CThorne::InitialUpdate(LTFLOAT fInfo)
{
	if (fInfo == INITIALUPDATE_WORLDFILE || fInfo == INITIALUPDATE_STRINGPROP)
	{
		CreateHitBox();
		CreateBall();
		SetState(kThorneAnim_CreateBall);

		m_Weapons.Init(m_hObject, LTNULL);
		m_Weapons.ObtainWeapon(m_nWeaponId, 0xfe, -1, LTFALSE);

		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(SFX_THORNE_ID);
			m_cs.Write(cMsg);
			g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
		}

		// RETAIL: built, never sent
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_SFX_MESSAGE);
			cMsg.Writeuint8(SFX_THORNE_ID);
			cMsg.WriteObject(m_hObject);
			m_cs.Write(cMsg);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::AllObjectsCreated
//
//	PURPOSE:	RETAIL: 0x10058e50
//
// ----------------------------------------------------------------------- //

void CThorne::AllObjectsCreated()
{
	g_pLTServer->SetNextUpdate(m_hObject, 0.01f);

	if (!m_hTargetObject)
	{
		HOBJECT hTarget;
		if (LT_OK == FindNamedObject(m_sTargetObject.c_str(), hTarget))
		{
			m_hTargetObject = hTarget;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::CreateHitBox
//
//	PURPOSE:	RETAIL: 0x10058f20
//
// ----------------------------------------------------------------------- //

void CThorne::CreateHitBox()
{
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	HCLASS hClass = g_pLTServer->GetClass("CCharacterHitBox");
	if (!hClass) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Pos = vPos;
	g_pLTServer->GetObjectRotation(m_hObject, &theStruct.m_Rotation);

	CCharacterHitBox* pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->CreateObject(hClass, &theStruct));
	if (pHitBox)
	{
		m_hHitBox = pHitBox->m_hObject;
		pHitBox->Init(m_hObject, this);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::CreateBall
//
//	PURPOSE:	RETAIL: 0x10059140: the ball, attached at the fire socket
//
// ----------------------------------------------------------------------- //

void CThorne::CreateBall()
{
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD;
	theStruct.m_Flags2 |= FLAG2_DYNAMICDIRLIGHT;
	theStruct.m_NextUpdate = 1.0f;

	int nAttachmentID = g_pAttachButeMgr->GetAttachmentIDByName("ThorneBall");
	int nAttachmentType = g_pAttachButeMgr->GetAttachmentType(nAttachmentID);

	char szClass[128];
	szClass[0] = '\0';

	g_pAttachButeMgr->GetAttachmentClass(nAttachmentID,szClass,sizeof(szClass));
	if ( !szClass[0] ) return;

	g_pAttachButeMgr->GetAttachmentModel(nAttachmentID,theStruct.m_Filename,sizeof(theStruct.m_Filename));
	g_pAttachButeMgr->CopyAttachmentSkins(nAttachmentID, theStruct.m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
	g_pAttachButeMgr->CopyAttachmentRenderStyles(nAttachmentID, theStruct.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN+1);

	HCLASS hClass = g_pLTServer->GetClass(szClass);
	if ( hClass == NULL )
	{
		char szError[512];
		sprintf(szError, "CreateObjectAttachment: No object with class: %s found", szClass );
	}

	if(g_pAttachButeMgr->GetAttachmentTranslcuent(nAttachmentID))
		theStruct.m_Flags2 |= FLAG2_FORCETRANSLUCENT;

	char szModel[128] = "";
	g_pAttachButeMgr->GetAttachmentProperties(nAttachmentID,szModel,sizeof(szModel));
	LPBASECLASS pModel = g_pLTServer->CreateObjectProps(hClass, &theStruct, szModel);
	if ( !pModel ) return;

	HATTACHMENT hAttachment;
	g_pLTServer->CreateAttachment(m_hObject, pModel->m_hObject, (char*)g_pBossButeMgr->GetThorne().sFireSocketName.c_str(), &LTVector(0,0,0), &LTRotation(), &hAttachment);

	ThorneBall* pBall = dynamic_cast<ThorneBall*>(pModel);
	if (!pBall) return;

	SendTriggerMsgToObject(g_pLTServer->HandleToObject(m_hObject), pModel->m_hObject, LTTRUE, s_szAttach);

	pBall->GetDestructible()->SetMaxHitPoints(g_pBossButeMgr->GetThorne().fBaseBallHealth);
	pBall->GetDestructible()->SetTronUnknown6D0(true);
	g_pCommonLT->SetObjectFlags(pBall->m_hObject, OFT_Flags, 0, FLAG_VISIBLE);
	pBall->SetBallState(0);
	pBall->GetDestructible()->Reset(pBall->GetDestructible()->GetMaxHitPoints(), pBall->GetDestructible()->GetMaxArmorPoints(), pBall->GetDestructible()->GetMaxEnergy());
	pBall->GetDestructible()->SetCanDamage(LTTRUE);

	m_hBall = pModel->m_hObject;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::GetFirePos
//
//	PURPOSE:	RETAIL: 0x100595b0: the fire socket's position
//
// ----------------------------------------------------------------------- //

LTVector CThorne::GetFirePos()
{
	HMODELSOCKET hSocket;
	g_pModelLT->GetSocket(m_hObject, g_pBossButeMgr->GetThorne().sFireSocketName.c_str(), hSocket);
	LTransform tf;
	g_pModelLT->GetSocketTransform(m_hObject, hSocket, tf, LTTRUE);
	return tf.m_Pos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::SetState
//
//	PURPOSE:	RETAIL: 0x10059640
//
// ----------------------------------------------------------------------- //

void CThorne::SetState(int eState)
{
	m_eState = eState;

	ResetAttachFX();
	PlayStateAnim();

	switch (m_eState)
	{
		case kThorneAnim_Disrupt:
		{
			PlayClientFX("DEATHFX_thornboss_dissipate", LTNULL, &GetFirePos(), LTNULL, 0);
		}
		break;

		case kThorneAnim_CreateBall:
		{
			ThorneBall* pBall = (ThorneBall*)g_pLTServer->HandleToObject(m_hBall);
			g_pCommonLT->SetObjectFlags(pBall->m_hObject, OFT_Flags, 0, FLAG_VISIBLE);
			pBall->SetBallState(0);
			pBall->GetDestructible()->Reset(pBall->GetDestructible()->GetMaxHitPoints(), pBall->GetDestructible()->GetMaxArmorPoints(), pBall->GetDestructible()->GetMaxEnergy());
			pBall->GetDestructible()->SetCanDamage(LTTRUE);

			pBall = (ThorneBall*)g_pLTServer->HandleToObject(m_hBall);
			g_pCommonLT->SetObjectFlags(pBall->m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
			pBall->SetBallState(1);
		}
		break;

		case kThorneAnim_ChargeBall:
		{
			CARange rngChargeTime = g_pBossButeMgr->GetThorne().rngChargeTime;
			m_fChargeDuration = GetRandom((LTFLOAT)rngChargeTime.GetMin(), (LTFLOAT)rngChargeTime.GetMax());
			m_fChargeStartTime = g_pLTServer->GetTime();
			m_bCharging = true;

			ThorneBall* pBall = (ThorneBall*)g_pLTServer->HandleToObject(m_hBall);
			g_pCommonLT->SetObjectFlags(pBall->m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
			pBall->SetBallState(1);

			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_SFX_MESSAGE);
			cMsg.Writeuint8(SFX_THORNE_ID);
			cMsg.WriteObject(m_hObject);
			cMsg.Writeuint8(THORNEFX_ATTACHFX_MSG);
			cMsg.WriteString("SHOW_ATTACHFX 2");
			g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
		}
		break;

		case kThorneAnim_ThrowBall:
		{
			LTRotation rRot(0.0f, 0.0f, 0.0f, 1.0f);
			PlayClientFX("DEATHFX_thornboss_fire", LTNULL, &GetFirePos(), &rRot, 0);
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::ResetAttachFX
//
//	PURPOSE:	RETAIL: 0x10059900
//
// ----------------------------------------------------------------------- //

void CThorne::ResetAttachFX()
{
	m_bAnimDone = false;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_THORNE_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(THORNEFX_ATTACHFX_MSG);
	cMsg.WriteString("RESET_ATTACHFX");
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	ThorneBall* pBall = (ThorneBall*)g_pLTServer->HandleToObject(m_hBall);
	g_pCommonLT->SetObjectFlags(pBall->m_hObject, OFT_Flags, 0, FLAG_VISIBLE);
	pBall->SetBallState(0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::UpdateState
//
//	PURPOSE:	RETAIL: 0x10059a30
//
// ----------------------------------------------------------------------- //

void CThorne::UpdateState()
{
	switch (m_eState)
	{
		case kThorneAnim_Disrupt:
		{
			if (m_bAnimDone)
			{
				if (m_bAllowSummon)
				{
					SetState(kThorneAnim_Summon);
					m_bAllowSummon = false;
				}
				else
				{
					SetState(kThorneAnim_Angry);
				}
			}
		}
		break;

		case kThorneAnim_Summon:
		case kThorneAnim_Angry:
		{
			if (m_bAnimDone)
			{
				SetState(kThorneAnim_CreateBall);
			}
		}
		break;

		case kThorneAnim_CreateBall:
		{
			ThorneBall* pBall = (ThorneBall*)g_pLTServer->HandleToObject(m_hBall);
			if (pBall->GetDestructible()->IsDead() == LTTRUE)
			{
				SetState(kThorneAnim_Disrupt);
			}
			else if (m_bAnimDone)
			{
				SetState(kThorneAnim_ChargeBall);
			}
		}
		break;

		case kThorneAnim_ChargeBall:
		{
			ThorneBall* pBall = (ThorneBall*)g_pLTServer->HandleToObject(m_hBall);
			if (pBall->GetDestructible()->IsDead() == LTTRUE)
			{
				SetState(kThorneAnim_Disrupt);
			}
			else if (GetChargeElapsed() >= m_fChargeDuration || GetChargeLeft() <= 0.0f)
			{
				SetState(kThorneAnim_ThrowBall);
			}
			else if (m_bAnimDone)
			{
				PlayStateAnim();
			}
		}
		break;

		case kThorneAnim_ThrowBall:
		{
			if (m_bAnimDone)
			{
				SetState(kThorneAnim_CreateBall);
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::Fire
//
//	PURPOSE:	RETAIL: 0x10059bd0: fire the weapon at the target
//
// ----------------------------------------------------------------------- //

void CThorne::Fire()
{
	m_Weapons.ChangeWeapon(m_nWeaponId);
	if (!m_Weapons.IsValidWeapon(m_Weapons.GetCurWeaponId())) return;

	CWeapon* pWeapon = m_Weapons.GetCurWeapon();
	if (!pWeapon) return;

	m_Weapons.SetAmmo(m_nAmmoId);
	pWeapon->ReloadClip(LTFALSE, 1000, m_nAmmoId);

	LTVector vTargetPos;
	g_pLTServer->GetObjectPos(m_hTargetObject, &vTargetPos);
	vTargetPos += g_pBossButeMgr->GetThorne().vFireOffset;

	LTVector vFirePos = GetFirePos();

	WeaponFireInfo weaponFireInfo;
	weaponFireInfo.hFiredFrom	= m_hObject;
	weaponFireInfo.vPath		= vTargetPos - vFirePos;
	weaponFireInfo.vFirePos		= vFirePos;
	weaponFireInfo.vFlashPos	= vFirePos;
	weaponFireInfo.hTestObj		= m_hTargetObject;
	weaponFireInfo.nSeed		= 100;
	weaponFireInfo.nFireTimestamp = (uint32)g_pLTServer->GetTime();

	pWeapon->Fire(weaponFireInfo);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::PlayStateAnim
//
//	PURPOSE:	RETAIL: 0x10059de0: one of the state's animations, at random
//
// ----------------------------------------------------------------------- //

void CThorne::PlayStateAnim()
{
	const ThorneAttributes::AnimMap& mapAnims = g_pBossButeMgr->GetThorne().mapAnims;

	ThorneAttributes::AnimMap::const_iterator it = mapAnims.lower_bound(m_eState);
	if (it == mapAnims.end()) return;

	std::pair<ThorneAttributes::AnimMap::const_iterator, ThorneAttributes::AnimMap::const_iterator> range = mapAnims.equal_range(m_eState);
	int nAnims = 0;
	std::distance(range.first, range.second, nAnims);

	int nAnim = GetRandom(0, nAnims - 1);
	for (int i = nAnim; i > 0; i--)
	{
		++it;
	}

	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, (char*)it->second.c_str());

	HMODELANIM hCurAnim;
	g_pModelLT->GetCurAnim(m_hObject, MAIN_TRACKER, hCurAnim);
	if (hCurAnim == hAnim)
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
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ThorneObjectPlugin::ThorneObjectPlugin
//
//	PURPOSE:	RETAIL: 0x10059f30
//
// ----------------------------------------------------------------------- //

ThorneObjectPlugin::ThorneObjectPlugin()
{
	m_pModelButeMgrPlugin = debug_new(CModelButeMgrPlugin);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThorne::GetNodeRadius
//
//	PURPOSE:	RETAIL: 0x101590f0 (folded with CCharacter's)
//
// ----------------------------------------------------------------------- //

float CThorne::GetNodeRadius(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	return g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, eModelNode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ThorneObjectPlugin::PreHook_EditStringList
//
//	PURPOSE:	RETAIL: 0x1005a010
//
// ----------------------------------------------------------------------- //

LTRESULT ThorneObjectPlugin::PreHook_EditStringList(const char* szRezPath,
	const char* szPropName, char** aszStrings, uint32* pcStrings,
	const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	static bool s_bInitted = false;
	if (!s_bInitted)
	{
		m_pModelButeMgrPlugin->PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		s_bInitted = true;
	}

	if (!_stricmp("ModelTemplate", szPropName))
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
