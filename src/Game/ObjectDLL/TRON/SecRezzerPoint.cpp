// ----------------------------------------------------------------------- //
//
// MODULE  : SecRezzerPoint.cpp
//
// PURPOSE : A point a SecRezzer rezzes its programs in at
//
// RETAIL: Tron 2.0's, written from retail Object.lto (SecRezzerPoint.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SecRezzerPoint.h"
#include "SecRezzer.h"
#include "ModelButeMgr.h"

BEGIN_CLASS(SecRezzerPoint)
	ADD_STRINGPROP_FLAG(SecRezzer, "", PF_OBJECTLINK)
END_CLASS_DEFAULT(SecRezzerPoint, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzerPoint::SecRezzerPoint()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

SecRezzerPoint::SecRezzerPoint() : BaseClass()
{
	m_szSecRezzer[0] = '\0';
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzerPoint::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 SecRezzerPoint::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate();
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzerPoint::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

LTBOOL SecRezzerPoint::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

	// RETAIL: needs the model bute manager, which it does not use
	if (!g_pModelButeMgr) return LTFALSE;

	if (g_pLTServer->GetPropGeneric("SecRezzer", &genProp) == LT_OK && genProp.m_String[0])
	{
		LTStrCpy(m_szSecRezzer, genProp.m_String, ARRAY_LEN(m_szSecRezzer));
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzerPoint::InitialUpdate
//
//	PURPOSE:	Remember where we are
//
// ----------------------------------------------------------------------- //

LTBOOL SecRezzerPoint::InitialUpdate()
{
	g_pLTServer->GetObjectPos(m_hObject, &m_vPos);
	g_pLTServer->GetObjectRotation(m_hObject, &m_rRot);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecRezzerPoint::OnAllObjectsCreated
//
//	PURPOSE:	Give our SecRezzer our position, and go away
//
// ----------------------------------------------------------------------- //

uint32 SecRezzerPoint::OnAllObjectsCreated()
{
	if (m_szSecRezzer[0])
	{
		ObjArray<HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
		g_pLTServer->FindNamedObjects(m_szSecRezzer, objArray);

		if (objArray.NumObjects())
		{
			SecRezzer* pRezzer = dynamic_cast<SecRezzer*>(g_pLTServer->HandleToObject(objArray.GetObject(0)));
			if (pRezzer)
			{
				SecRezzerPointInfo point;
				point.vPos = m_vPos;
				point.rRot = m_rRot;
				pRezzer->GetPoints().push_back(point);
			}
		}
	}

	g_pLTServer->RemoveObject(m_hObject);

	return 0;
}
