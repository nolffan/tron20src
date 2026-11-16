// ----------------------------------------------------------------------- //
//
// MODULE  : VersionMgr.cpp
//
// PURPOSE : Implementation of versioning manager
//
// CREATED : 11/16/2000
//
// (c) 2000-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "VersionMgr.h"
#include "regmgr.h"
#include "ClientServerShared.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr::CVersionMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CVersionMgr::CVersionMgr()
{
	// Determine what language version the game is using...

	m_szLanguage[0]	= '\0';
	m_bLowViolence	= false;

	m_regMgr.Init("Monolith Productions", GAME_NAME, "1.0");

	// Get the language registry key...

	if (m_regMgr.IsValid())
	{
		unsigned int nBufSize = sizeof(m_szLanguage);
		m_regMgr.Get("Language", m_szLanguage, nBufSize, "English");

// RETAIL: Tron still has the low-violence check (0x10171d50); NOLF2
		// moved it to CRes and added NetRegionCode after Tron
		if (m_szLanguage[0])
		{
			if (_stricmp(m_szLanguage, "German") == 0)
			{
				m_bLowViolence = true;
			}
		}
	}
}

