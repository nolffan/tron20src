// ----------------------------------------------------------------------- //
//
// MODULE  : StartupCommand.cpp
//
// PURPOSE : The StartupCommand implementation
//
// CREATED : 11/27/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "StartupCommand.h"

LINKFROM_MODULE( StartupCommand );

BEGIN_CLASS( StartupCommand )

	// RETAIL: Tron's has only the command (ClassDef 0x1032f150: one
	// property), as build 51's; NOLF2 1.x added RoundCondition and SafeExecute
	ADD_STRINGPROP_FLAG( Command, "", PF_NOTIFYCHANGE )

END_CLASS_DEFAULT_FLAGS_PLUGIN( StartupCommand, GameBase, NULL, NULL, 0, CStartupCommandPlugin )

CMDMGR_BEGIN_REGISTER_CLASS( StartupCommand )
CMDMGR_END_REGISTER_CLASS( StartupCommand, GameBase )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CStartupCommandPlugin::PreHook_PropChanged
//
//  PURPOSE:	Make sure the Command is valid
//
// ----------------------------------------------------------------------- //

LTRESULT CStartupCommandPlugin::PreHook_PropChanged( const char *szObjName,
													 const char *szPropName,
												     const int nPropType,
												     const GenericProp &gpPropValue,
												     ILTPreInterface *pInterface,
													 const	char *szModifiers )
{
	if( LT_OK == m_CmdMgrPlugin.PreHook_PropChanged( szObjName,
														 szPropName,
														 nPropType, 
														 gpPropValue,
														 pInterface,
														 szModifiers ))
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	StartupCommand::StartupCommand
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

StartupCommand::StartupCommand( ) :	GameBase( OT_NORMAL )
{
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	StartupCommand::~StartupCommand
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

StartupCommand::~StartupCommand( )
{
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	StartupCommand::EngineMessageFn
//
//  PURPOSE:	Handle message from the engine...
//
// ----------------------------------------------------------------------- //

uint32 StartupCommand::EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData )
{
	switch( messageID )
	{
		case MID_PRECREATE :
		{
			// Let the GameBase handle the message first

			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			ObjectCreateStruct	*pOCS = (ObjectCreateStruct*)pData;

			if( pOCS )
			{
				if( PRECREATE_WORLDFILE == fData )
				{
					ReadProps( pOCS );
				}
			}

			// Important!! - We already sent the message to the GameBase so DONT do it again.

			return dwRet;
		}
		break;

		case MID_OBJECTCREATED :
		{
			// Wait untill the first update to send the command.  This way any object that relies
			// on MID_ALLOBJECTSCREATED to finish their setup will have done so. 

			SetNextUpdate( UPDATE_NEXT_FRAME );
		}
		break;

		case MID_UPDATE :
		{
			// RETAIL: 0x1023a646, Tron's sends the command on the first update
			// (no round condition, no "safe execute" as NOLF2 1.x has)

			if( g_pCmdMgr->IsValidCmd( m_sStartupCmd.c_str( )))
			{
				g_pCmdMgr->Process( m_sStartupCmd.c_str( ), m_hObject, m_hObject );
			}

			g_pLTServer->RemoveObject( m_hObject );
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn( messageID, pData, fData );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	StartupCommand::ReadProps
//
//  PURPOSE:	Read in the property values...
//
// ----------------------------------------------------------------------- //

void StartupCommand::ReadProps( ObjectCreateStruct *pOcs )
{
	ASSERT( pOcs != LTNULL );

	GenericProp gProp;

	if( g_pLTServer->GetPropGeneric( "Command", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
		{
			m_sStartupCmd = gProp.m_String;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StartupCommand::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void StartupCommand::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	SAVE_CHARSTRING( m_sStartupCmd.c_str( ));	// RETAIL: 0x1023a7f0
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StartupCommand::Load
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void StartupCommand::Load(ILTMessage_Read *pMsg, uint32 dwSaveFlags)
{
	char szString[1024];
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sStartupCmd = szString;
}
