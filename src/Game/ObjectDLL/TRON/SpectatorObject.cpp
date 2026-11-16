// ----------------------------------------------------------------------- //
//
// MODULE  : SpectatorObject.cpp
//
// PURPOSE : A camera position for the multiplayer spectator view
//
// RETAIL: Tron 2.0's, written from retail Object.lto (SpectatorObject.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SpectatorObject.h"
#include "MsgIDs.h"
#include "SFXMsgIds.h"

BEGIN_CLASS(SpectatorObject)
	ADD_BOOLPROP(Primary, LTFALSE)
END_CLASS_DEFAULT(SpectatorObject, GameBase, NULL, NULL)

SpectatorObject::SpectatorObjectList SpectatorObject::m_lstSpectatorObjects;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpectatorObject::SpectatorObject
//
//	PURPOSE:	Initialize (RETAIL: 0x101501b0)
//
// ----------------------------------------------------------------------- //

SpectatorObject::SpectatorObject() : GameBase(OT_NORMAL)
{
	m_vPitchYawRoll.Init();
	m_bPrimary = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpectatorObject::~SpectatorObject
//
//	PURPOSE:	Take the object off the list (RETAIL: 0x10150200)
//
// ----------------------------------------------------------------------- //

SpectatorObject::~SpectatorObject()
{
	SpectatorObjectList::iterator iter = m_lstSpectatorObjects.begin();
	while( iter != m_lstSpectatorObjects.end() )
	{
		if( *iter == this )
		{
			m_lstSpectatorObjects.erase( iter );
			break;
		}

		++iter;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpectatorObject::EngineMessageFn
//
//	PURPOSE:	Handle engine messages (RETAIL: 0x10150270)
//
// ----------------------------------------------------------------------- //

uint32 SpectatorObject::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
    uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct *pStruct = (ObjectCreateStruct*)pData;

			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE)
				{
					ReadProp(pStruct);
				}
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAGMASK_ALL);

			CreateSpecialFX();

			m_lstSpectatorObjects.push_back( this );
		}
		break;

		default : break;
	}

	return dwRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpectatorObject::CreateSpecialFX
//
//	PURPOSE:	Tell the clients about the object (RETAIL: 0x10150330)
//
// ----------------------------------------------------------------------- //

void SpectatorObject::CreateSpecialFX()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_SPECTATOROBJECT_ID);
	cMsg.Writebool(m_bPrimary);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());

	cMsg.Reset();
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_SPECTATOROBJECT_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writebool(m_bPrimary);
	cMsg.WriteLTVector(m_vPitchYawRoll);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpectatorObject::ReadProp
//
//	PURPOSE:	Read the properties (RETAIL: 0x101504a0)
//
// ----------------------------------------------------------------------- //

LTBOOL SpectatorObject::ReadProp(ObjectCreateStruct *pStruct)
{
    g_pLTServer->GetPropRotationEuler("Rotation", &m_vPitchYawRoll);

	GenericProp genProp;

	if( g_pLTServer->GetPropGeneric( "Primary", &genProp ) == LT_OK )
	{
		m_bPrimary = genProp.m_Bool;
	}

	return LTTRUE;
}
