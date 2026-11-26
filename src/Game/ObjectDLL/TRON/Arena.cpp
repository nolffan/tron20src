// ----------------------------------------------------------------------- //
//
// MODULE  : Arena.cpp
//
// PURPOSE : A disc arena of a multiplayer level
//
// RETAIL: Tron 2.0's, written from retail Object.lto (Arena.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Arena.h"

BEGIN_CLASS(Arena)
	ADD_STRINGPROP_FLAG(Arena, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(OnPlayerSpawn, "", PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(OnPlayerDeath, "", PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(OnMatchStart, "", PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(OnMatchEnd, "", PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(OnPointStart, "", PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(OnPointEnd, "", PF_NOTIFYCHANGE)
END_CLASS_DEFAULT_FLAGS_PLUGIN(Arena, GameBase, NULL, NULL, CF_ALWAYSLOAD, CArenaPlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Arena::Arena()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Arena::Arena() : GameBase(OT_NORMAL)
{
	m_nArena				= 0;
	m_hstrName				= LTNULL;
	m_hstrOnPlayerSpawnCmd	= LTNULL;
	m_hstrOnPlayerDeathCmd	= LTNULL;
	m_hstrOnMatchStartCmd	= LTNULL;
	m_hstrOnMatchEndCmd		= LTNULL;
	m_hstrOnPointStartCmd	= LTNULL;
	m_hstrOnPointEndCmd		= LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Arena::~Arena()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Arena::~Arena()
{
	// RETAIL: the strings are freed, not cleared
	if (m_hstrName) g_pLTServer->FreeString(m_hstrName);
	if (m_hstrOnPlayerSpawnCmd) g_pLTServer->FreeString(m_hstrOnPlayerSpawnCmd);
	if (m_hstrOnPlayerDeathCmd) g_pLTServer->FreeString(m_hstrOnPlayerDeathCmd);
	if (m_hstrOnMatchStartCmd) g_pLTServer->FreeString(m_hstrOnMatchStartCmd);
	if (m_hstrOnMatchEndCmd) g_pLTServer->FreeString(m_hstrOnMatchEndCmd);
	if (m_hstrOnPointStartCmd) g_pLTServer->FreeString(m_hstrOnPointStartCmd);
	if (m_hstrOnPointEndCmd) g_pLTServer->FreeString(m_hstrOnPointEndCmd);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Arena::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Arena::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE)
				{
					ReadProp(pStruct);
				}

				// RETAIL: every arena object is named "Arena"
				SAFE_STRCPY(pStruct->m_Name, "Arena");
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				SetNextUpdate(UPDATE_NEVER);
			}
		}
		break;

		default : break;
	}

	return dwRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Arena::ReadProp()
//
//	PURPOSE:	Read the properties
//
// ----------------------------------------------------------------------- //

LTBOOL Arena::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

	if (g_pLTServer->GetPropGeneric("Name", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrName = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	if (g_pLTServer->GetPropGeneric("Arena", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			int nArena = atoi(genProp.m_String);
			if (nArena >= 1 && nArena <= 4)
			{
				m_nArena = nArena - 1;
			}
			else
			{
				m_nArena = 0;
			}
		}
	}

	if (g_pLTServer->GetPropGeneric("OnPlayerSpawn", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrOnPlayerSpawnCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	if (g_pLTServer->GetPropGeneric("OnPlayerDeath", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrOnPlayerDeathCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	if (g_pLTServer->GetPropGeneric("OnMatchStart", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrOnMatchStartCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	if (g_pLTServer->GetPropGeneric("OnMatchEnd", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrOnMatchEndCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	if (g_pLTServer->GetPropGeneric("OnPointStart", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrOnPointStartCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	if (g_pLTServer->GetPropGeneric("OnPointEnd", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrOnPointEndCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArenaPlugin::PreHook_EditStringList()
//
//	PURPOSE:	The arena numbers
//
// ----------------------------------------------------------------------- //

LTRESULT CArenaPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
	uint32* pcStrings,
	const uint32 cMaxStrings,
	const uint32 cMaxStringLength)
{
	if (_stricmp("Arena", szPropName) == 0)
	{
		for (int i = 0; i < 4; i++)
		{
			if (cMaxStrings > (*pcStrings) + 1)
			{
				sprintf(aszStrings[(*pcStrings)++], "%d", i + 1);
			}
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArenaPlugin::PreHook_PropChanged()
//
//	PURPOSE:	Check the event commands
//
// ----------------------------------------------------------------------- //

LTRESULT CArenaPlugin::PreHook_PropChanged(
	const	char		*szObjName,
	const	char		*szPropName,
	const	int			nPropType,
	const	GenericProp	&gpPropValue,
			ILTPreInterface	*pInterface,
	const	char		*szModifiers )
{
	// RETAIL: the tests miss their "!": every property but none of these
	// goes to the command manager (docs/RETAIL_BUGS.md)
#ifdef TRON20SRC_MODERN
	// GAME-78: only the six event properties
	if (!_stricmp("OnPlayerSpawn", szPropName) ||
		!_stricmp("OnPlayerDeath", szPropName) ||
		!_stricmp("OnMatchStart", szPropName) ||
		!_stricmp("OnMatchEnd", szPropName) ||
		!_stricmp("OnPointStart", szPropName) ||
		!_stricmp("OnPointEnd", szPropName))
#else
	if (_stricmp("OnPlayerSpawn", szPropName) ||
		_stricmp("OnPlayerDeath", szPropName) ||
		_stricmp("OnMatchStart", szPropName) ||
		_stricmp("OnMatchEnd", szPropName) ||
		_stricmp("OnPointStart", szPropName) ||
		_stricmp("OnPointEnd", szPropName))
#endif
	{
		if (m_CommandMgrPlugin.PreHook_PropChanged(szObjName, szPropName, nPropType,
			gpPropValue, pInterface, szModifiers) == LT_OK)
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}
