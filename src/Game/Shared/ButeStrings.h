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

class CButeMgr;

// RETAIL: Tron's (CShell.dll 0x10147740, 0x10147830; defined in TeamMgr.cpp,
// whose TU they open). The first reads one attribute (an empty string when
// it is missing), the second the numbered attributes <pszAttrBase>0,
// <pszAttrBase>1, ... until one is missing. The names are ours
void GetButeString(CButeMgr& buteMgr, const char* pszTag, const char* pszAttr, std::string& sValue);
void GetButeStringList(CButeMgr& buteMgr, const char* pszTag, const char* pszAttrBase, std::vector<std::string>& lstValues);

#endif // __BUTE_STRINGS_H__
