// ----------------------------------------------------------------------- //
//
// MODULE  : AttachedExplosion.cpp
//
// PURPOSE : An explosion that follows the object it is attached to
//
// RETAIL: Tron 2.0's, written from retail Object.lto (AttachedExplosion.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AttachedExplosion.h"
#include "ParsedMsg.h"
#include "Attachments.h"

BEGIN_CLASS(AttachedExplosion)
END_CLASS_DEFAULT_FLAGS(AttachedExplosion, Explosion, NULL, NULL, 0)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachedExplosion::AttachedExplosion()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AttachedExplosion::AttachedExplosion()
:	Explosion(),
	m_hTarget( LTNULL )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachedExplosion::Update()
//
//	PURPOSE:	Follow the attachment, then update the explosion
//
// ----------------------------------------------------------------------- //

void AttachedExplosion::Update()
{
	GetAttachedPos(&m_vPos);
	Explosion::Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachedExplosion::OnTrigger()
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool AttachedExplosion::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Attach(KEY_ATTACH);

	if (cMsg.GetArg(0) == s_cTok_Attach)
	{
		HATTACHMENT hAttachment;
		if (g_pLTServer->FindAttachment(hSender, m_hObject, &hAttachment) == LT_OK)
		{
			m_hTarget = hSender;
			m_hFiredFrom = hSender;
		}
	}

	return Explosion::OnTrigger(hSender, cMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachedExplosion::GetAttachedPos()
//
//	PURPOSE:	Get the position of the attachment (the object's own
//				position when there is no target)
//
// ----------------------------------------------------------------------- //

LTRESULT AttachedExplosion::GetAttachedPos(LTVector *pvPos)
{
	if (!pvPos) return LT_ERROR;

	if (!m_hTarget)
	{
		g_pLTServer->GetObjectPos(m_hObject, pvPos);
		return LT_OK;
	}

	HATTACHMENT hAttachment;
	LTRESULT res = g_pLTServer->FindAttachment(m_hTarget, m_hObject, &hAttachment);
	if (res != LT_OK) return res;

	LTransform transform;
	res = g_pCommonLT->GetAttachmentTransform(hAttachment, transform, LTTRUE);
	if (res != LT_OK) return res;

	*pvPos = transform.m_Pos;
	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachedExplosion::Save()/Load()
//
//	PURPOSE:	Save and load the object
//
// ----------------------------------------------------------------------- //

void AttachedExplosion::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	Explosion::Save(pMsg, dwSaveFlags);

	SAVE_HOBJECT(m_hTarget);
}

void AttachedExplosion::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	Explosion::Load(pMsg, dwLoadFlags);

	LOAD_HOBJECT(m_hTarget);
}
