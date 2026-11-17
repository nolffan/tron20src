// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeExitLevelByRezOut.cpp
//
// PURPOSE : Tron's exit level node where the AI rezzes out. Retail only,
//           written from the retail code (see the header).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AINodeExitLevelByRezOut.h"
#include "AIGoalButeMgr.h"
#include "CommandMgr.h"
#include "AIButeMgr.h"
#include "WeaponMgr.h"
#include "SortedStringList.h"

LINKFROM_MODULE( AINodeExitLevelByRezOut );

// RETAIL: the props (0x1026e090): SmartObject has a static list, which
// the plugin fills (AINodeExitLevelByRezOutPlugin, 0x1026e190).

BEGIN_CLASS(AINodeExitLevelByRezOut)
	ADD_STRINGPROP_FLAG(SmartObject,			"None",			0|PF_STATICLIST)
	ADD_STRINGPROP_FLAG(OnUseCmd,				"",				0)
END_CLASS_DEFAULT_FLAGS_PLUGIN(AINodeExitLevelByRezOut, AINode, NULL, NULL, 0, AINodeExitLevelByRezOutPlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeExitLevelByRezOutPlugin::PreHook_EditStringList
//
//	PURPOSE:	RETAIL: 0x1026e190: the smart objects that are not hidden,
//				sorted.
//
// ----------------------------------------------------------------------- //

extern CAIButeMgr s_AIButeMgr;
extern CAIGoalButeMgr s_AIGoalButeMgr;

LTRESULT AINodeExitLevelByRezOutPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( !g_pWeaponMgr )
	{
		CWeaponMgrPlugin WeaponMgrPlugin;
		WeaponMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
	}

	if ( !g_pAIButeMgr )
	{
		char szFile[256];
		sprintf(szFile, "%s\\Attributes\\AIButes.txt", szRezPath);
		s_AIButeMgr.SetInRezFile(LTFALSE);
		s_AIButeMgr.Init(szFile);
	}

	if ( !g_pAIGoalButeMgr )
	{
		char szFile[256];
		sprintf(szFile, "%s\\Attributes\\AIGoals.txt", szRezPath);
		s_AIGoalButeMgr.SetInRezFile(LTFALSE);
		s_AIGoalButeMgr.Init(szFile);
	}

	if ( !_strcmpi("SmartObject", szPropName) )
	{
		CSortedStringList lstNames( cMaxStrings, cMaxStringLength );

		uint32 cSmartObjects = s_AIGoalButeMgr.GetNumSmartObjectTemplates();
		for(uint32 iSmartObject=0; iSmartObject < cSmartObjects; ++iSmartObject)
		{
			if( !s_AIGoalButeMgr.GetSmartObjectTemplate(iSmartObject)->bHidden )
			{
				lstNames.AddString( s_AIGoalButeMgr.GetSmartObjectTemplate(iSmartObject)->szName );
			}
		}

		lstNames.PopulateStringList( aszStrings, pcStrings );

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

CMDMGR_BEGIN_REGISTER_CLASS(AINodeExitLevelByRezOut)
CMDMGR_END_REGISTER_CLASS(AINodeExitLevelByRezOut, AINode)

// ----------------------------------------------------------------------- //

void AINodeExitLevelByRezOut::ReadProp(ObjectCreateStruct* pocs)
{
	// RETAIL: 0x1026e720
	if ( g_pLTServer->GetPropGeneric( "SmartObject", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] && g_pAIGoalButeMgr->GetSmartObjectTemplate( g_gp.m_String ) )
		{
			m_nSmartObjectID = g_pAIGoalButeMgr->GetSmartObjectTemplate( g_gp.m_String )->nID;
		}
		else
		{
			m_nSmartObjectID = 0;
		}
	}

	// RETAIL: a SmartObject name is not taken as a command (as in
	// AINodeExitLevel::ReadProp)
	if ( g_pLTServer->GetPropGeneric( "OnUseCmd", &g_gp ) == LT_OK )
	{
		if ( !g_gp.m_String[0] || !g_pAIGoalButeMgr->GetSmartObjectTemplate( g_gp.m_String ) )
		{
			m_sOnUseCmd = g_gp.m_String;
		}
	}

	AINodeGuard::ReadProp(pocs);
}

// ----------------------------------------------------------------------- //

void AINodeExitLevelByRezOut::Load(ILTMessage_Read *pMsg)
{
	// RETAIL: 0x1026e870
	LOAD_DWORD( m_nSmartObjectID );

	char szString[1024];
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sOnUseCmd = szString;

	AINodeGuard::Load(pMsg);
}

// ----------------------------------------------------------------------- //

void AINodeExitLevelByRezOut::Save(ILTMessage_Write *pMsg)
{
	// RETAIL: 0x1026e940
	SAVE_DWORD( m_nSmartObjectID );
	SAVE_CHARSTRING( m_sOnUseCmd.c_str( ));

	AINodeGuard::Save(pMsg);
}

// ----------------------------------------------------------------------- //

const char* AINodeExitLevelByRezOut::GetSmartObjectCommand()
{
	// RETAIL: 0x1026e980
	if( m_nSmartObjectID < g_pAIGoalButeMgr->GetNumSmartObjectTemplates() )
	{
		AIGBM_SmartObjectTemplate* pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate( m_nSmartObjectID );

		SMART_OBJECT_CMD_MAP::iterator it = pSmartObject->mapCmds.find( GetType() );
		if( it != pSmartObject->mapCmds.end() )
		{
			return g_pLTServer->GetStringData( it->second );
		}
	}

	return "";
}

// ----------------------------------------------------------------------- //

void AINodeExitLevelByRezOut::ProcessOnUseCmd(HOBJECT hUser)
{
	// RETAIL: 0x1026ea10
	if( g_pCmdMgr->IsValidCmd( m_sOnUseCmd.c_str( )))
	{
		g_pCmdMgr->Process( m_sOnUseCmd.c_str( ), m_hObject, hUser );
	}
}
