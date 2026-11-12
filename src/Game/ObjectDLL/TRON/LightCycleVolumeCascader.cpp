// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleVolumeCascader.cpp
//
// PURPOSE : Turns ON the LightCycleVolumes around it in a growing circle
//
// RETAIL: Tron 2.0's, written from retail Object.lto
// (LightCycleVolumeCascader.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LightCycleVolumeCascader.h"
#include "ParsedMsg.h"
#include "ServerUtilities.h"
#include <algorithm>

BEGIN_CLASS(LightCycleVolumeCascader)
	ADD_REALPROP(CascadeRate, 0.0f)
	ADD_REALPROP(MaxRadius, 0.0f)
END_CLASS_DEFAULT_FLAGS(LightCycleVolumeCascader, GameBase, NULL, NULL, 0)

CMDMGR_BEGIN_REGISTER_CLASS( LightCycleVolumeCascader )
CMDMGR_END_REGISTER_CLASS( LightCycleVolumeCascader, GameBase )

// RETAIL: what OnSave writes the list with (its call through the pointer
// is to 0x1002e5c0); ours
struct WriteObjectFn
{
	ILTMessage_Write*	m_pMsg;
	void				(ILTMessage_Write::*m_pfnWrite)(HOBJECT);

	void operator()(HOBJECT hObj) { (m_pMsg->*m_pfnWrite)(hObj); }
};

// RETAIL: what OnLoad fills the list with (std::generate's at 0x1002e430
// calls it through the pointer); ours
struct ReadObjectFn
{
	ILTMessage_Read*	m_pMsg;
	HOBJECT				(ILTMessage_Read::*m_pfnRead)();

	HOBJECT operator()() { return (m_pMsg->*m_pfnRead)(); }
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleVolumeCascader::LightCycleVolumeCascader / ~
//
// ----------------------------------------------------------------------- //

LightCycleVolumeCascader::LightCycleVolumeCascader()
:	GameBase(OT_NORMAL),
	m_bCascading(false),
	m_bDone(false),
	m_fCascadeRate(0.0f),
	m_fMaxRadius(0.0f),
	m_fRadius(0.0f)
{
}

LightCycleVolumeCascader::~LightCycleVolumeCascader()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleVolumeCascader::OnPrecreate
//
// ----------------------------------------------------------------------- //

uint32 LightCycleVolumeCascader::OnPrecreate(ObjectCreateStruct* pOCS, float precreateType)
{
	if (precreateType == PRECREATE_WORLDFILE || precreateType == PRECREATE_STRINGPROP)
	{
		if (!pOCS) return 0;

		m_fCascadeRate = pOCS->m_cProperties.GetPropReal("CascadeRate", 0.0f);
		m_fMaxRadius = pOCS->m_cProperties.GetPropReal("MaxRadius", 0.0f);
	}

	return GameBase::OnPrecreate(pOCS, precreateType);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleVolumeCascader::OnAllObjectsCreated
//
//	PURPOSE:	Collect the LightCycleVolumes within the max radius
//
// ----------------------------------------------------------------------- //

uint32 LightCycleVolumeCascader::OnAllObjectsCreated()
{
	if (!m_bCascading && !m_bDone)
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		HCLASS hVolumeClass = g_pLTServer->GetClass("LightCycleVolume");

		ObjectList* pList = g_pLTServer->FindObjectsTouchingSphere(&vPos, m_fMaxRadius);
		for (ObjectLink* pLink = pList->m_pFirstLink; pLink; pLink = pLink->m_pNext)
		{
			if (g_pLTServer->GetObjectClass(pLink->m_hObject) == hVolumeClass)
			{
				m_lstVolumes.push_back(LTObjRef(pLink->m_hObject));
			}
		}

		g_pLTServer->RelinquishList(pList);
	}

	return GameBase::OnAllObjectsCreated();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleVolumeCascader::OnUpdate
//
//	PURPOSE:	Grow the circle, turning ON the volumes it reaches
//
// ----------------------------------------------------------------------- //

uint32 LightCycleVolumeCascader::OnUpdate()
{
	if (!m_bCascading)
	{
		SetNextUpdate(0.0f);
	}
	else if (m_fRadius < m_fMaxRadius && !m_lstVolumes.empty())
	{
		m_fRadius = Clamp(m_fCascadeRate + m_fRadius, 0.0f, m_fMaxRadius);

		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		LTFLOAT fRadiusSqr = m_fRadius * m_fRadius;

		VolumeList::iterator iter = m_lstVolumes.begin();
		while (iter != m_lstVolumes.end())
		{
			LTObjRef hVolume = *iter;

			LTVector vVolumePos;
			g_pLTServer->GetObjectPos(hVolume, &vVolumePos);

			if ((vVolumePos - vPos).MagSqr() < fRadiusSqr)
			{
				SendTriggerMsgToObject(this, hVolume, LTFALSE, "ON");
				iter++;
				m_lstVolumes.remove(hVolume);
			}
			else
			{
				iter++;
			}
		}

		SetNextUpdate(0.1f);
	}
	else
	{
		m_bDone = true;
		m_bCascading = false;
		SetNextUpdate(0.0f);
	}

	return GameBase::OnUpdate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleVolumeCascader::OnSave / OnLoad
//
// ----------------------------------------------------------------------- //

uint32 LightCycleVolumeCascader::OnSave(ILTMessage_Write *pMsg, float dwSaveFlags)
{
	SAVE_bool(m_bCascading);
	SAVE_bool(m_bDone);
	SAVE_FLOAT(m_fCascadeRate);
	SAVE_FLOAT(m_fMaxRadius);
	SAVE_FLOAT(m_fRadius);

	pMsg->Writeint16(m_lstVolumes.size());
	WriteObjectFn fnWrite = { pMsg, &ILTMessage_Write::WriteObject };
	for (VolumeList::iterator iter = m_lstVolumes.begin(); iter != m_lstVolumes.end(); iter++)
	{
		fnWrite(*iter);
	}

	return GameBase::OnSave(pMsg, dwSaveFlags);
}

uint32 LightCycleVolumeCascader::OnLoad(ILTMessage_Read *pMsg, float dwLoadFlags)
{
	LOAD_bool(m_bCascading);
	LOAD_bool(m_bDone);
	LOAD_FLOAT(m_fCascadeRate);
	LOAD_FLOAT(m_fMaxRadius);
	LOAD_FLOAT(m_fRadius);

	m_lstVolumes.resize(pMsg->Readint16());
	ReadObjectFn fnRead = { pMsg, &ILTMessage_Read::ReadObject };
	std::generate(m_lstVolumes.begin(), m_lstVolumes.end(), fnRead);

	return GameBase::OnLoad(pMsg, dwLoadFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleVolumeCascader::OnTrigger
//
//	PURPOSE:	ON starts the cascade
//
// ----------------------------------------------------------------------- //

bool LightCycleVolumeCascader::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_On("ON");

	if (cMsg.GetArg(0) == s_cTok_On && !m_bCascading)
	{
		SetNextUpdate(0.1f);
		m_bCascading = true;
	}

	return GameBase::OnTrigger(hSender, cMsg);
}
