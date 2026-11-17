// ----------------------------------------------------------------------- //
//
// MODULE  : CServerMark.cpp
//
// PURPOSE : CServerMark implementation
//
// CREATED : 1/15/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ServerMark.h"
#include "WeaponMgr.h"
#include "ilttransform.h"


LINKFROM_MODULE( ServerMark );

#pragma force_active on
BEGIN_CLASS(CServerMark)
END_CLASS_DEFAULT_FLAGS(CServerMark, BaseClass, NULL, NULL, CF_HIDDEN)
#pragma force_active off

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMark::CServerMark()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CServerMark::CServerMark() : BaseClass(OT_SPRITE)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMark::~CServerMark()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CServerMark::~CServerMark()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMark::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

// RETAIL: 0x1022ec80: Tron's mark is Build 51's: it only goes away with
// its parent.  NOLF2's timed fade out (and its MarkWM console variables and
// save data) is not in Tron.
uint32 CServerMark::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PARENTATTACHMENTREMOVED :
		{
            g_pLTServer->RemoveObject(m_hObject);
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}
