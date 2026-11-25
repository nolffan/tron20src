/****************************************************************************
;
;	MODULE:		REGFILE (.H)
;
;	PURPOSE:	MODERN.md APPS-3: the modern build's place for the values
;				that retail kept in the registry. Ours (no retail
;				counterpart).
;
;	COMMENT:	The values are in "tron20.ini", in the directory of the
;				running program (Tron.exe, TronLauncher.exe, Lithtech.exe
;				with the game DLLs, TRONSrv.exe: all in the game
;				directory, so all use the same file). Each registry key
;				is a section, its path without the root key and without
;				a first "SOFTWARE\" part:
;
;					[Monolith Productions\TRON 2.0\1.0]
;					Disable Sound=0
;					ProfileName="Player"
;
;				A string is written in quotes (so that its spaces stay),
;				a DWORD in decimal, binary data with
;				WritePrivateProfileStruct (hex digits and a checksum).
;				Section and value names are not case-sensitive, as the
;				registry's are not. The registry is not read or written.
;
;				Every function reads or writes the file at once, so the
;				launcher, the game and the server see the values of the
;				others. C and C++ can include this file (GameSpy's
;				nonport.c does). Each function is static: each module
;				that includes the file has its own copy.
;
****************************************************************************/

#ifndef REGFILE_H
#define REGFILE_H

#ifdef TRON20SRC_MODERN

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REGFILE_NAME		"tron20.ini"
#define REGFILE_MAXVALUE	4096

// a value that no value of the file can be: GetPrivateProfileString gives it
// back when the value is not in the file
static const char g_szRegFileMissing[] = "\x01\x02<missing>\x02\x01";

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RegFile_Path
//
//	PURPOSE:	The full path of the file, beside the running program
//
// ----------------------------------------------------------------------- //

static BOOL RegFile_Path(char* szPath, DWORD dwSize)
{
	char szExe[MAX_PATH];
	DWORD nLen = GetModuleFileNameA(NULL, szExe, sizeof(szExe));
	char* pSlash;
	if(nLen == 0 || nLen >= sizeof(szExe))
	{
		return FALSE;
	}
	pSlash = strrchr(szExe, '\\');
	if(pSlash)
	{
		pSlash[1] = '\0';
	}
	else
	{
		strcpy(szExe, ".\\");
	}
	if(strlen(szExe) + strlen(REGFILE_NAME) + 1 > dwSize)
	{
		return FALSE;
	}
	strcpy(szPath, szExe);
	strcat(szPath, REGFILE_NAME);
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RegFile_Section
//
//	PURPOSE:	The section name of a registry key path: without a first
//				"SOFTWARE\" part (CRegMgr adds "Software", the launcher
//				gives "SOFTWARE"), so that every caller of the same key
//				gets the same section
//
// ----------------------------------------------------------------------- //

static BOOL RegFile_Section(char* szSection, DWORD dwSize, const char* szKey)
{
	if(!szKey)
	{
		return FALSE;
	}
	if(_strnicmp(szKey, "SOFTWARE\\", 9) == 0)
	{
		szKey += 9;
	}
	if(szKey[0] == '\0' || strlen(szKey) + 1 > dwSize)
	{
		return FALSE;
	}
	strcpy(szSection, szKey);
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RegFile_GetRaw
//
//	PURPOSE:	The text of a value (quotes removed), FALSE when the file
//				does not have it
//
// ----------------------------------------------------------------------- //

static BOOL RegFile_GetRaw(const char* szSection, const char* szField, char* szBuf, DWORD dwSize)
{
	char szPath[MAX_PATH];
	DWORD nLen;
	if(!szSection || !szField || !szBuf || dwSize < 2 || !RegFile_Path(szPath, sizeof(szPath)))
	{
		return FALSE;
	}
	nLen = GetPrivateProfileStringA(szSection, szField, g_szRegFileMissing, szBuf, dwSize, szPath);
	if(strcmp(szBuf, g_szRegFileMissing) == 0)
	{
		return FALSE;
	}
	// a value as long as the buffer was cut
	if(nLen >= dwSize - 1)
	{
		return FALSE;
	}
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RegFile_SetRaw
//
//	PURPOSE:	Writes the text of a value (NULL deletes the value; a NULL
//				field deletes the section)
//
// ----------------------------------------------------------------------- //

static BOOL RegFile_SetRaw(const char* szSection, const char* szField, const char* szValue)
{
	char szPath[MAX_PATH];
	if(!szSection || !RegFile_Path(szPath, sizeof(szPath)))
	{
		return FALSE;
	}
	return WritePrivateProfileStringA(szSection, szField, szValue, szPath) ? TRUE : FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RegFile_GetString
//
//	PURPOSE:	A string value. dwSize: in, the size of the buffer; out,
//				the size of the string with its terminator (as
//				RegQueryValueEx)
//
// ----------------------------------------------------------------------- //

static BOOL RegFile_GetString(const char* szSection, const char* szField, char* szValue, DWORD* pdwSize)
{
	char szBuf[REGFILE_MAXVALUE];
	DWORD nLen;
	if(!szValue || !pdwSize || *pdwSize == 0)
	{
		return FALSE;
	}
	if(!RegFile_GetRaw(szSection, szField, szBuf, sizeof(szBuf)))
	{
		return FALSE;
	}
	nLen = strlen(szBuf) + 1;
	if(nLen > *pdwSize)
	{
		*pdwSize = nLen;
		return FALSE;
	}
	memcpy(szValue, szBuf, nLen);
	*pdwSize = nLen;
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RegFile_SetString
//
//	PURPOSE:	Writes a string value, in quotes
//
// ----------------------------------------------------------------------- //

static BOOL RegFile_SetString(const char* szSection, const char* szField, const char* szValue)
{
	char szBuf[REGFILE_MAXVALUE];
	if(!szField || !szValue || strlen(szValue) + 3 > sizeof(szBuf))
	{
		return FALSE;
	}
	szBuf[0] = '"';
	strcpy(szBuf + 1, szValue);
	strcat(szBuf, "\"");
	return RegFile_SetRaw(szSection, szField, szBuf);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RegFile_GetDWord
//
//	PURPOSE:	A DWORD value: a number in the file (decimal, or 0x hex)
//
// ----------------------------------------------------------------------- //

static BOOL RegFile_GetDWord(const char* szSection, const char* szField, DWORD* pdwValue)
{
	char szBuf[64];
	char* pEnd = NULL;
	DWORD dwValue;
	if(!pdwValue || !RegFile_GetRaw(szSection, szField, szBuf, sizeof(szBuf)) || szBuf[0] == '\0')
	{
		return FALSE;
	}
	dwValue = strtoul(szBuf, &pEnd, 0);
	if(!pEnd || *pEnd != '\0')
	{
		return FALSE;
	}
	*pdwValue = dwValue;
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RegFile_SetDWord
//
//	PURPOSE:	Writes a DWORD value, in decimal
//
// ----------------------------------------------------------------------- //

static BOOL RegFile_SetDWord(const char* szSection, const char* szField, DWORD dwValue)
{
	char szBuf[16];
	if(!szField)
	{
		return FALSE;
	}
	sprintf(szBuf, "%lu", (unsigned long)dwValue);
	return RegFile_SetRaw(szSection, szField, szBuf);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RegFile_GetBinary
//
//	PURPOSE:	Binary data. dwSize: in, the size of the buffer; out, the
//				size of the data. WritePrivateProfileStruct's form keeps
//				no size, so the size comes from the text (two digits a
//				byte and two for the checksum)
//
// ----------------------------------------------------------------------- //

static BOOL RegFile_GetBinary(const char* szSection, const char* szField, void* pValue, DWORD* pdwSize)
{
	char szBuf[REGFILE_MAXVALUE];
	char szPath[MAX_PATH];
	DWORD nLen;
	if(!pValue || !pdwSize || *pdwSize == 0 || !RegFile_GetRaw(szSection, szField, szBuf, sizeof(szBuf)))
	{
		return FALSE;
	}
	nLen = strlen(szBuf);
	if(nLen < 2 || (nLen & 1))
	{
		return FALSE;
	}
	nLen = nLen / 2 - 1;
	if(nLen > *pdwSize || !RegFile_Path(szPath, sizeof(szPath)))
	{
		return FALSE;
	}
	if(!GetPrivateProfileStructA(szSection, szField, pValue, nLen, szPath))
	{
		return FALSE;
	}
	*pdwSize = nLen;
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RegFile_SetBinary
//
//	PURPOSE:	Writes binary data
//
// ----------------------------------------------------------------------- //

static BOOL RegFile_SetBinary(const char* szSection, const char* szField, const void* pValue, int nLen)
{
	char szPath[MAX_PATH];
	if(!szSection || !szField || !pValue || nLen < 0 || !RegFile_Path(szPath, sizeof(szPath)))
	{
		return FALSE;
	}
	return WritePrivateProfileStructA(szSection, szField, (LPVOID)pValue, (UINT)nLen, szPath) ? TRUE : FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RegFile_HasSection
//
//	PURPOSE:	Whether the file has a section (a key "exists")
//
// ----------------------------------------------------------------------- //

static BOOL RegFile_HasSection(const char* szSection)
{
	char szBuf[8];
	char szPath[MAX_PATH];
	if(!szSection || !RegFile_Path(szPath, sizeof(szPath)))
	{
		return FALSE;
	}
	// the names of the section's values; a section without values counts
	// as missing (WritePrivateProfileString makes no empty section)
	return GetPrivateProfileStringA(szSection, NULL, "", szBuf, sizeof(szBuf), szPath) > 0;
}

#endif // TRON20SRC_MODERN

#endif // REGFILE_H
