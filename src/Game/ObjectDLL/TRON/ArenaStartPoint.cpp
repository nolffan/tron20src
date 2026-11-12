// ----------------------------------------------------------------------- //
//
// MODULE  : ArenaStartPoint.cpp
//
// PURPOSE : ArenaStartPoint implementation
//
// RETAIL: Tron 2.0 only, written from retail Object.lto (TU 0x10086de0..
//         0x100872b0).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ArenaStartPoint.h"

LINKFROM_MODULE( ArenaStartPoint );

// RETAIL: retail .rdata 0x102ea968
static const char* s_aszArenaStartTypes[ArenaStartPoint::kNumTypes] =
{
	"Spectator",
	"Team 1",
	"Team 2",
};

#define ARENASTARTPOINT_MAX_ARENAS	4

BEGIN_CLASS(ArenaStartPoint)
	ADD_STRINGPROP_FLAG(Type, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(Arena, "", PF_STATICLIST)
END_CLASS_DEFAULT_FLAGS_PLUGIN(ArenaStartPoint, GameBase, NULL, NULL, CF_ALWAYSLOAD, CArenaStartPointPlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArenaStartPoint::ArenaStartPoint / ~ArenaStartPoint
//
//	PURPOSE:	(RETAIL: 0x10086f50 / 0x10086fa0)
//
// ----------------------------------------------------------------------- //

ArenaStartPoint::ArenaStartPoint() : GameBase()
{
	m_eType = kType_Spectator;
	m_nArena = 0;
	m_hstrName = LTNULL;
	m_vRotation.Init();
}

ArenaStartPoint::~ArenaStartPoint()
{
	if( m_hstrName )
	{
		g_pLTServer->FreeString( m_hstrName );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArenaStartPoint::EngineMessageFn
//
//	PURPOSE:	Handle engine messages (RETAIL: 0x10087010)
//
// ----------------------------------------------------------------------- //

uint32 ArenaStartPoint::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

	switch( messageID )
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if( pStruct )
			{
				if( fData == PRECREATE_WORLDFILE )
				{
					ReadProp( pStruct );
				}

				SAFE_STRCPY( pStruct->m_Name, "ArenaStartPoint" );
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if( fData != INITIALUPDATE_SAVEGAME )
			{
				SetNextUpdate( UPDATE_NEVER );
			}
		}
		break;
	}

	return dwRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArenaStartPoint::ReadProp
//
//	PURPOSE:	Read the object's properties (RETAIL: 0x100870a0)
//
// ----------------------------------------------------------------------- //

LTBOOL ArenaStartPoint::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;

	if( g_pLTServer->GetPropGeneric( "Name", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] )
		{
			m_hstrName = g_pLTServer->CreateString( genProp.m_String );
		}
	}

	g_pLTServer->GetPropRotationEuler( "Rotation", &m_vRotation );

	if( g_pLTServer->GetPropGeneric( "Type", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] )
		{
			for( int iType = 0; iType < kNumTypes; ++iType )
			{
				if( !_stricmp( genProp.m_String, s_aszArenaStartTypes[iType] ) )
				{
					m_eType = (ArenaStartType)iType;
					break;
				}
			}
		}
	}

	if( g_pLTServer->GetPropGeneric( "Arena", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] )
		{
			int nArena = atoi( genProp.m_String );
			if( nArena >= 1 && nArena <= ARENASTARTPOINT_MAX_ARENAS )
			{
				m_nArena = nArena - 1;
			}
			else
			{
				m_nArena = 0;
			}
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArenaStartPointPlugin::PreHook_EditStringList
//
//	PURPOSE:	The arenas and the start types (RETAIL: 0x100871f0)
//
// ----------------------------------------------------------------------- //

LTRESULT CArenaStartPointPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
	uint32* pcStrings,
	const uint32 cMaxStrings,
	const uint32 cMaxStringLength)
{
	if( !_stricmp( "Arena", szPropName ) )
	{
		for( int iArena = 0; iArena < ARENASTARTPOINT_MAX_ARENAS; ++iArena )
		{
			if( cMaxStrings > (*pcStrings) + 1 )
			{
				sprintf( aszStrings[(*pcStrings)++], "%d", iArena + 1 );
			}
		}

		return LT_OK;
	}

	if( !_stricmp( "Type", szPropName ) )
	{
		for( int iType = 0; iType < ArenaStartPoint::kNumTypes; ++iType )
		{
			if( cMaxStrings > (*pcStrings) + 1 )
			{
				sprintf( aszStrings[(*pcStrings)++], "%s", s_aszArenaStartTypes[iType] );
			}
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
