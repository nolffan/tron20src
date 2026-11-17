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

// RETAIL: Tron's CServerMark has no fading (no "MarkWMShow",
// "MarkWMFadeTime" or "MarkWMSolidTime" strings and no save in
// Object.lto): it only goes away with its parent (0x1022ec80).

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

uint32 CServerMark::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	if( messageID == MID_PARENTATTACHMENTREMOVED )
	{
		g_pLTServer->RemoveObject(m_hObject);
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}
