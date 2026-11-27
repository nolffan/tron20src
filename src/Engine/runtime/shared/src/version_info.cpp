
#include "bdefs.h"
#include "version_info.h"


void LTVersionInfo::GetString(char *pStr, uint32 nStrBytes)
{
	char tempStr[512];

#ifdef TRON20SRC_MODERN
	// MODERN.md EN-83: Lithtech.exe has no version resource (RCDATA
	// 12346), so the version is not known: say so, and not
	// "4294967295.4294967295" (docs/RETAIL_BUGS.md)
	if ((m_MajorVersion == INVALID_LT_VERSION) || (m_MinorVersion == INVALID_LT_VERSION))
		LTSNPrintF(tempStr, sizeof(tempStr), "unknown");
	else
		LTSNPrintF(tempStr, sizeof(tempStr), "%lu.%lu", m_MajorVersion, m_MinorVersion);
#else
	LTSNPrintF(tempStr, sizeof(tempStr), "%lu.%lu", m_MajorVersion, m_MinorVersion);
#endif
	LTStrCpy(pStr, tempStr, nStrBytes);
}





