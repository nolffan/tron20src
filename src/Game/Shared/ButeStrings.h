// ----------------------------------------------------------------------- //
//
// MODULE  : ButeStrings.h
//
// PURPOSE : Tron's helpers that read attribute strings into std::strings
//
// ----------------------------------------------------------------------- //

#ifndef __BUTE_STRINGS_H__
#define __BUTE_STRINGS_H__

#include <string>
#include <vector>

#include "ButeMgr.h"

// RETAIL: Tron's helpers that read attribute strings (the names are ours).
// They are file-static functions defined in this header: every TU that
// uses them has its own copy (GetButeStringList: CShell.dll 0x10147830 in
// TeamMgr's TU, 0x1000b8e0 in CircuitButeMgr's; each is called only from
// its own TU), and /OPT:ICF folds the copies of GetButeString, which has no
// unwinding (0x10147740, called from both). The first reads one attribute
// (an empty string when it is missing), the second the numbered attributes
// <pszAttrBase>0, <pszAttrBase>1, ... until one is missing.

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetButeString
//
//	PURPOSE:	Read an attribute string (RETAIL: CShell.dll 0x10147740)
//
// ----------------------------------------------------------------------- //

static void GetButeString(CButeMgr& buteMgr, const char* pszTag, const char* pszAttr, std::string& sValue)
{
	const char* pszValue = buteMgr.GetString(pszTag, pszAttr);
	if (buteMgr.Success() && pszValue)
	{
		sValue = pszValue;
	}
	else
	{
		// RETAIL: the default is assigned through a null check (0x101477d6)
		const char* pszDefault = "";
		sValue.assign(pszDefault, pszDefault + (pszDefault ? strlen(pszDefault) : 0));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetButeStringList
//
//	PURPOSE:	Read the numbered attribute strings (RETAIL: CShell.dll
//				0x10147830)
//
// ----------------------------------------------------------------------- //

static void GetButeStringList(CButeMgr& buteMgr, const char* pszTag, const char* pszAttrBase, std::vector<std::string>& lstValues)
{
	lstValues.clear();

	char szAttr[80];
	int nIndex = 0;
	while (true)
	{
		std::string sValue;
		sprintf(szAttr, "%s%d", pszAttrBase, nIndex);
		nIndex++;
		GetButeString(buteMgr, pszTag, szAttr, sValue);
		if (!buteMgr.Success())
			break;

		lstValues.push_back(sValue);
	}
}


#endif // __BUTE_STRINGS_H__
