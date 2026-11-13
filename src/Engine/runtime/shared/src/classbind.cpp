#include "bdefs.h"

#include "classbind.h"
#include "bindmgr.h"
#include "iservershell.h"
#include "ltproperty.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//the ILTServer game interface
#include "iltserver.h"
static ILTServer *ilt_server;
define_holder(ILTServer, ilt_server);





typedef ClassDef** (*ObjectDLLSetupFn)(int *nDefs, void *pServer, int *version);


static void cb_VerifyClassDefProperties(ClassBindModule *pModule)
{
    int i, j;
    PropDef *pProp;

    for (i=0; i < pModule->m_nClassDefs; i++)
    {
        for (j=0; j < pModule->m_pClassDefs[i]->m_nProps; j++)
        {
            pProp = &pModule->m_pClassDefs[i]->m_Props[j];

            if (pProp->m_PropType >= NUM_PROPERTYTYPES || pProp->m_PropType < 0)
                pProp->m_PropType = NUM_PROPERTYTYPES;
        }
    }
}

#ifdef __XBOX
extern ClassDef** ObjectDLLSetup(int *nDefs, ILTServer *pServer, int *version);
#endif

// [dlj] Linux doesn't need this because we use dynamic linking 
//#ifndef _WIN32
//extern ClassDef** ObjectDLLSetup(int *nDefs, ILTServer *pServer, int *version);
//#endif // !_WIN32

// TRON: Tron 2.0's is the older interface (server.dll 0x10001839): the
// module is allocated here and returned through ppModule, and the binding
// takes no bTempFile.  RETAIL BUG: the module is allocated before the
// binding and leaked when it fails, and freed with free() (it was allocated
// with new, LTMemAlloc) on a version mismatch (docs/RETAIL_BUGS.md)
int cb_LoadModule(const char *pModuleName, ClassBindModule *&pModuleOut, int *version)
{
    ClassBindModule *pModule;
    CBindModuleType *hModule;
    ObjectDLLSetupFn theFunction;
    int status;

    // this is alloced here and then returned
    LT_MEM_TRACK_ALLOC(pModule = new ClassBindModule, LT_MEM_TYPE_MISC);

    status = bm_BindModule(pModuleName, hModule);
    if (status == BIND_CANTFINDMODULE)
        return CB_CANTFINDMODULE;

    // Get the function.
    theFunction = (ObjectDLLSetupFn)bm_GetFunctionPointer(hModule, "ObjectDLLSetup");
    if (!theFunction)
    {
        bm_UnbindModule(hModule);
        return CB_NOTCLASSMODULE;
    }

    // Ok.. setup the classbindmodule.
    pModule->m_hModule = hModule;
    pModule->m_pClassDefs = theFunction(&pModule->m_nClassDefs, ilt_server, version);

    if (*version != SERVEROBJ_VERSION)
    {
        free(pModule);
        bm_UnbindModule(hModule);
        return CB_VERSIONMISMATCH;
    }

    // Verify all properties!
    cb_VerifyClassDefProperties(pModule);

    pModuleOut = pModule;
    return CB_NOERROR;
}

// TRON: RETAIL: server.dll 0x10001918: frees the module cb_LoadModule made
void cb_UnloadModule(ClassBindModule *pModule)
{
    bm_UnbindModule(pModule->m_hModule);
    delete pModule;
}


int cb_GetNumClassDefs(ClassBindModule *hModule)
{
    return hModule->m_nClassDefs;
}


ClassDef** cb_GetClassDefs(ClassBindModule *hModule)
{
    return hModule->m_pClassDefs;
}


ClassDef* cb_FindClass(ClassBindModule *pModule, const char *pClassName)
{
    int i;

    for (i=0; i < pModule->m_nClassDefs; i++)
        if (strcmp(pModule->m_pClassDefs[i]->m_ClassName, pClassName) == 0)
            return pModule->m_pClassDefs[i];

    return LTNULL;
}


PropDef* cb_FindVarFull(ClassBindModule *hModule, ClassDef *pClass, const char *pVarName)
{
    int i;

    while (pClass)
    {
        for (i=0; i < pClass->m_nProps; i++)
            if (strcmp(pClass->m_Props[i].m_PropName, pVarName) == 0)
                return &pClass->m_Props[i];
    
        pClass = pClass->m_ParentClass;
    }

    return LTNULL;
}


// IsClassFlagSet
//
// Finds if class flags are set in this or any parent class...
ClassDef *cb_IsClassFlagSet(ClassBindModule *hModule, ClassDef *pClass, const uint32 dwClassFlag)
{
    while (pClass)
    {
        if (pClass->m_ClassFlags & dwClassFlag)
            return pClass;
    
        pClass = pClass->m_ParentClass;
    }

    return LTNULL;
}


