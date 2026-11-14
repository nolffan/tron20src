// ----------------------------------------------------------------------- //
//
// MODULE  : TronCDKey.cpp
//
// PURPOSE : The CD key, as the installer left it in the registry
//
// RETAIL: Tron 2.0's, written from retail CShell.dll 0x1000b640,
// 0x1000b7d0 (see TronCDKey.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronCDKey.h"
#include "regmgr.h"
#include <ctype.h>

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronCDKey::CTronCDKey
//
// ----------------------------------------------------------------------- //

CTronCDKey::CTronCDKey()
{
	CRegMgr regMgr;
	if (regMgr.Init("Monolith Productions", "TRON 2.0", "1.0", NULL, HKEY_LOCAL_MACHINE, NULL))
	{
		char szKey[30];
		uint32 nSize = sizeof(szKey);
		regMgr.Get("CDKey", szKey, nSize, "");

		int nLen = strlen(szKey);
		for (int i = 0; i < nLen; i++)
		{
			szKey[i] = toupper(szKey[i]);
		}

		m_sCDKey = szKey;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronCDKey::GetCDKey
//
// ----------------------------------------------------------------------- //

std::string CTronCDKey::GetCDKey() const
{
	return m_sCDKey;
}
