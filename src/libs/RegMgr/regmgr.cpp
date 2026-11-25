
#include "windows.h"
#include "regmgr.h"

#ifdef TRON20SRC_MODERN

// MODERN.md APPS-3: the modern build keeps the values in a file beside the
// program (regfile.h), not in the registry. The same class, the same
// calls: the game DLLs (CVersionMgr: Language, ProfileName; TronCDKey: the
// CD key) do not change. The root key is not used: HKEY_LOCAL_MACHINE and
// HKEY_CURRENT_USER go to the same file.

#include "regfile.h"

BOOL CRegMgr::Init(const char* sCompany, const char* sApp, const char* sVersion, const char* sSubKey, HANDLE hRootKey, char* sRoot2)
{
  m_bInitialized = FALSE;
  if (sCompany == NULL || sApp == NULL || sVersion == NULL) return FALSE;
  if (strlen(sApp) >= sizeof(m_sApp)) return FALSE;
  if (sSubKey != NULL && strlen(sSubKey) >= sizeof(m_sSubKey)) return FALSE;
  if (strlen(sCompany) + strlen(sApp) + strlen(sVersion) + 3 > sizeof(m_sSection)) return FALSE;
  strcpy(m_sApp,sApp);
  if (sSubKey != NULL) strcpy(m_sSubKey,sSubKey);
  else m_sSubKey[0] = '\0';
  m_hRootKey = (HKEY)hRootKey;
  m_hSoftwareKey = NULL;
  m_hCompanyKey = NULL;
  m_hAppKey = NULL;
  m_hVersionKey = NULL;
  m_hSubKey = NULL;
  // "Software" (or sRoot2) is not part of the section (RegFile_Section)
  sprintf(m_sSection, "%s\\%s\\%s", sCompany, sApp, sVersion);
  m_bInitialized = TRUE;
  if (!SetSubKey(sSubKey)) {
    m_bInitialized = FALSE;
    return FALSE;
  }
  return TRUE;
}

void CRegMgr::Term() {
  m_bInitialized = FALSE;
}

BOOL CRegMgr::SetSubKey(const char* sSubKey) {
  if (!m_bInitialized) return FALSE;
  // the section of the version key, then the sub key under it
  char* pSub = strchr(m_sSection, '\\');
  if (pSub) pSub = strchr(pSub + 1, '\\');
  if (pSub) pSub = strchr(pSub + 1, '\\');
  if (pSub) *pSub = '\0';
  if (sSubKey == NULL) return TRUE;
  if (strlen(m_sSection) + strlen(sSubKey) + 2 > sizeof(m_sSection)) return FALSE;
  strcat(m_sSection, "\\");
  strcat(m_sSection, sSubKey);
  return TRUE;
}

BOOL CRegMgr::Set(const char* sKey, const char* sValue) {
  if (!m_bInitialized) return FALSE;
  if (sKey == NULL) return FALSE;
  if (sValue == NULL) return FALSE;
  return RegFile_SetString(m_sSection, sKey, sValue);
}

BOOL CRegMgr::Set(const char* sKey, void* pValue, int nLen) {
  if (!m_bInitialized) return FALSE;
  if (sKey == NULL) return FALSE;
  if (pValue == NULL) return FALSE;
  return RegFile_SetBinary(m_sSection, sKey, pValue, nLen);
}

BOOL CRegMgr::Set(const char* sKey, DWORD nValue) {
  if (!m_bInitialized) return FALSE;
  if (sKey == NULL) return FALSE;
  return RegFile_SetDWord(m_sSection, sKey, nValue);
}

char* CRegMgr::Get(const char* sKey, char* sBuf, UINT32& nBufSize, const char* sDef) {
  if (m_bInitialized && sKey != NULL && sBuf != NULL && nBufSize > 0) {
    DWORD dwSize = nBufSize;
    if (RegFile_GetString(m_sSection, sKey, sBuf, &dwSize)) {
      nBufSize = dwSize;
      return sBuf;
    }
  }
  if (sDef != NULL && sBuf != NULL) {
    strcpy(sBuf,sDef);
    nBufSize = strlen(sBuf);
    return sBuf;
  }
  nBufSize = 0;
  return NULL;
}

void* CRegMgr::Get(const char* sKey, void* pBuf, UINT32& nBufSize, void* pDef, UINT32 nDefSize) {
  if (m_bInitialized && sKey != NULL && pBuf != NULL && nBufSize > 0) {
    DWORD dwSize = nBufSize;
    if (RegFile_GetBinary(m_sSection, sKey, pBuf, &dwSize)) {
      nBufSize = dwSize;
      return pBuf;
    }
  }
  if ((pDef != NULL) && (nDefSize > 0) && pBuf != NULL) {
    memcpy(pBuf,pDef,nDefSize);
    nBufSize = nDefSize;
    return pBuf;
  }
  nBufSize = 0;
  return NULL;
}

DWORD CRegMgr::Get(const char* sKey, DWORD nDef) {
  if (!m_bInitialized) return nDef;
  if (sKey == NULL) return nDef;
  DWORD nVal;
  if (RegFile_GetDWord(m_sSection, sKey, &nVal)) return nVal;
  return nDef;
}

BOOL CRegMgr::Delete(const char* sKey) {
  if (!m_bInitialized) return FALSE;
  if (sKey == NULL) return FALSE;
  return RegFile_SetRaw(m_sSection, sKey, NULL);
}

BOOL CRegMgr::CreateKey(HKEY hKey, const char* sSubKey, HKEY& hNewKey) {
  // no registry keys: a section is made when its first value is written
  hNewKey = NULL;
  return TRUE;
}

BOOL CRegMgr::DeleteApp() {
  // the version key's section (and the sub key's): the values of this
  // object; the file has no tree of keys to delete under the app
  if (!m_bInitialized) return FALSE;
  return RegFile_SetRaw(m_sSection, NULL, NULL);
}

BOOL CRegMgr::DeleteSubKey() {
  if (!m_bInitialized) return FALSE;
  return RegFile_SetRaw(m_sSection, NULL, NULL);
}

BOOL CRegMgr::DeleteUnderSubKey(const char* sSubKey) {
  if (!m_bInitialized) return FALSE;
  if (sSubKey == NULL) return FALSE;
  char sSection[sizeof(m_sSection) + 256];
  if (strlen(m_sSection) + strlen(sSubKey) + 2 > sizeof(sSection)) return FALSE;
  sprintf(sSection, "%s\\%s", m_sSection, sSubKey);
  return RegFile_SetRaw(sSection, NULL, NULL);
}

#else // TRON20SRC_MODERN

BOOL CRegMgr::Init(const char* sCompany, const char* sApp, const char* sVersion, const char* sSubKey, HANDLE hRootKey, char* sRoot2)
{
  strcpy(m_sApp,sApp);
  if (sSubKey != NULL) strcpy(m_sSubKey,sSubKey);
  else m_sSubKey[0] = '\0';
  m_hRootKey = (HKEY)hRootKey;
  char sSoftware[] = "Software";
  char* pSoftware;
  if (sRoot2 == NULL) pSoftware = sSoftware;
  else pSoftware = sRoot2;
  if (CreateKey(m_hRootKey, pSoftware, m_hSoftwareKey)) {
    if (CreateKey(m_hSoftwareKey, sCompany, m_hCompanyKey)) {
      if (CreateKey(m_hCompanyKey, sApp, m_hAppKey)) {
        if (CreateKey(m_hAppKey, sVersion, m_hVersionKey)) {
          m_bInitialized = TRUE;
          if (SetSubKey(sSubKey)) {
            return TRUE;
          }
          else {
            m_bInitialized = FALSE;
            return FALSE;
          }
        }
        else return FALSE;
      }
      else return FALSE;
    }
    else return FALSE;
  }
  else return FALSE;
} 

void CRegMgr::Term() {
  if (!m_bInitialized) return;
  m_bInitialized = FALSE;
  RegCloseKey(m_hSoftwareKey);
  RegCloseKey(m_hCompanyKey);
  RegCloseKey(m_hAppKey);
  if (m_hVersionKey != m_hSubKey) RegCloseKey(m_hVersionKey);
  RegCloseKey(m_hSubKey);
};

BOOL CRegMgr::SetSubKey(const char* sSubKey) {
  if (!m_bInitialized) return FALSE;
  if (sSubKey == NULL) {
    m_hSubKey = m_hVersionKey;
    return TRUE;
  }
  else {
    if (CreateKey(m_hVersionKey, sSubKey, m_hSubKey)) return TRUE;
    else return FALSE;
  }
};                                                        

BOOL CRegMgr::Set(const char* sKey, const char* sValue) {
  if (!m_bInitialized) return FALSE;
  if (sKey == NULL) return FALSE;
  if (sValue == NULL) return NULL;
  if (RegSetValueEx(m_hSubKey,sKey,0,REG_SZ,(const unsigned char*)sValue,strlen(sValue)+1) == ERROR_SUCCESS) return TRUE;
  else return FALSE;
};

BOOL CRegMgr::Set(const char* sKey, void* pValue, int nLen) {
  if (!m_bInitialized) return FALSE;
  if (sKey == NULL) return FALSE;
  if (pValue == NULL) return FALSE;
  if (RegSetValueEx(m_hSubKey,sKey,0,REG_BINARY,(const unsigned char*)pValue,nLen) == ERROR_SUCCESS) return TRUE;
  else return FALSE;
};

BOOL CRegMgr::Set(const char* sKey, DWORD nValue) {
  if (!m_bInitialized) return FALSE;
  if (sKey == NULL) return FALSE;
  if (RegSetValueEx(m_hSubKey,sKey,0,REG_DWORD,(const unsigned char*)&nValue,sizeof(nValue)) == ERROR_SUCCESS) return TRUE;
  else return FALSE;
};

char* CRegMgr::Get(const char* sKey, char* sBuf, UINT32& nBufSize, const char* sDef) {
  if (!m_bInitialized) goto Default;
  if (sKey == NULL) goto Default;
  if (sBuf == NULL) goto Default;
  if (nBufSize <= 0)  goto Default;
  DWORD nType;
  if (RegQueryValueEx(m_hSubKey,sKey,0,&nType,(unsigned char*)sBuf,( DWORD * )&nBufSize) == ERROR_SUCCESS) {
    if (nType != REG_SZ) goto Default;
    return sBuf;
  }
  else goto Default;
Default:
  if (sDef != NULL) {
    strcpy(sBuf,sDef);
    nBufSize = strlen(sBuf);
    return sBuf;
  }
  else {
    nBufSize = 0;
	return NULL;
  }
};

void* CRegMgr::Get(const char* sKey, void* pBuf, UINT32& nBufSize, void* pDef, UINT32 nDefSize) {
  if (!m_bInitialized) goto Default;
  if (sKey == NULL) goto Default;
  if (pBuf == NULL) goto Default;
  if (nBufSize <= 0) goto Default;
  DWORD nType;
  if (RegQueryValueEx(m_hSubKey,sKey,0,&nType,(unsigned char*)pBuf,( DWORD * )&nBufSize) == ERROR_SUCCESS) {
    if (nType != REG_BINARY) goto Default;
    return pBuf;
  }
  else goto Default;
Default:
  if ((pDef != NULL) && (nDefSize > 0)) {
    memcpy(pBuf,pDef,nDefSize);
	nBufSize = nDefSize;
	return pBuf;
  }
  else {
    nBufSize = 0;
	return NULL;
  }
};

DWORD CRegMgr::Get(const char* sKey, DWORD nDef) {
  if (!m_bInitialized) return nDef;
  if (sKey == NULL) return nDef;
  DWORD nType;
  DWORD nVal;
  DWORD nSize = sizeof(nVal);
  if (RegQueryValueEx(m_hSubKey,sKey,0,&nType,(unsigned char*)&nVal,&nSize) == ERROR_SUCCESS) {
    if (nType != REG_DWORD) return nDef;
    return nVal;
  }
  else return nDef;
};

BOOL CRegMgr::Delete(const char* sKey) {
  if (!m_bInitialized) return FALSE;
  if (sKey == NULL) return FALSE;
  if (RegDeleteValue(m_hSubKey,sKey) == ERROR_SUCCESS) return TRUE;
  else return FALSE;
};

BOOL CRegMgr::CreateKey(HKEY hKey, const char* sSubKey, HKEY& hNewKey) {
  DWORD nDisposition;
//  if (RegCreateKeyEx(hKey, sSubKey, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey,
  if (RegCreateKeyEx(hKey, sSubKey, 0, "", REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hNewKey,
      &nDisposition) == ERROR_SUCCESS) return TRUE;
  else return FALSE;
};

BOOL CRegMgr::DeleteApp() {
  if (RegDeleteKey(m_hCompanyKey,m_sApp) == ERROR_SUCCESS) return TRUE;
  else return FALSE;
};

BOOL CRegMgr::DeleteSubKey() {
  if (RegDeleteKey(m_hCompanyKey,m_sSubKey) == ERROR_SUCCESS) return TRUE;
  else return FALSE;
};

BOOL CRegMgr::DeleteUnderSubKey(const char* sSubKey) {
  if (RegDeleteKey(m_hSubKey,sSubKey) == ERROR_SUCCESS) return TRUE;
  else return FALSE;
};

#endif // TRON20SRC_MODERN
