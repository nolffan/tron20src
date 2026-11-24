// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeGuard.cpp
//
// PURPOSE : AINodeGuard implementation
//
// CREATED : 11/13/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AINodeGuard.h"
#include "AINodeMgr.h"
#include "AIUtils.h"
#include "AITypes.h"
#include "AIAssert.h"
#include "AIGoalButeMgr.h"

LINKFROM_MODULE( AINodeGuard );

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeGuard)

	ADD_REALPROP_FLAG(ReturnRadius,	256.0f,		0|PF_RADIUS)

	PROP_DEFINEGROUP(GuardedNodes, PF_GROUP(1)) \

		ADD_STRINGPROP_FLAG(Node1, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Node2, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Node3, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Node4, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Node5, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Node6, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Node7, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Node8, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Node9, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Node10, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Node11, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Node12, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Node13, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Node14, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Node15, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Node16, "", PF_GROUP(1)|PF_OBJECTLINK)

END_CLASS_DEFAULT(AINodeGuard, AINode, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeGuard)
CMDMGR_END_REGISTER_CLASS(AINodeGuard, AINode)

BEGIN_CLASS(AINodeTalk)
	ADD_REALPROP_FLAG(ReturnRadius,	256.0f,		0|PF_RADIUS)
END_CLASS_DEFAULT(AINodeTalk, AINode, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeTalk)
CMDMGR_END_REGISTER_CLASS(AINodeTalk, AINode)

BEGIN_CLASS(AINodeExitLevel)
	ADD_STRINGPROP_FLAG(OnUseCmd,				"",				0)	// RETAIL: Tron's (0x1026d460)
END_CLASS_DEFAULT(AINodeExitLevel, AINode, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeExitLevel)
CMDMGR_END_REGISTER_CLASS(AINodeExitLevel, AINode)

// ----------------------------------------------------------------------- //

AINodeGuard::AINodeGuard()
{
}

AINodeGuard::~AINodeGuard()
{
	HSTRING hstrNodeName;
	HSTRING_LIST::iterator it;
	for ( it = m_lstGuardedNodeNames.begin(); it != m_lstGuardedNodeNames.end(); ++it )
	{
		hstrNodeName = *it;
		FREE_HSTRING( hstrNodeName );
	}
}

void AINodeGuard::ReadProp(ObjectCreateStruct* pocs)
{
	super::ReadProp(pocs);

    if ( g_pLTServer->GetPropGeneric( "ReturnRadius", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_fRadiusSqr = g_gp.m_Float*g_gp.m_Float;
		}
	}

	// Read list of guarded nodes.

	uint32 iGuardedNode = 0;
	char szBuffer[128];
	sprintf(szBuffer, "Node%d", iGuardedNode+1);

	while( g_pLTServer->GetPropGeneric( szBuffer, &g_gp) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_lstGuardedNodeNames.push_back( g_pLTServer->CreateString( g_gp.m_String ) );
		}
	
		++iGuardedNode;
		sprintf(szBuffer, "Node%d", iGuardedNode+1);
	}
}

void AINodeGuard::Init()
{
	super::Init();

	// Get the guarded nodes.
	// Guarded nodes will be owned by this guard node.  Nodes that are
	// owned may only be used by the AI assigned to this guard node.

	HSTRING hstrNodeName;
	HSTRING_LIST::iterator it;
	for ( it = m_lstGuardedNodeNames.begin(); it != m_lstGuardedNodeNames.end(); ++it )
	{
		hstrNodeName = *it;
		AINode* pNode = g_pAINodeMgr->GetNode( hstrNodeName );

		if ( pNode )
		{
			m_lstGuardedNodes.push_back( pNode->m_hObject );
			pNode->SetNodeOwner( m_hObject );
		}
		else
		{
			AIASSERT1( 0, m_hObject, "AINodeGuard::Init: Cannot find guarded node \"%s\"", ::ToString( hstrNodeName ));
		}

		FREE_HSTRING( hstrNodeName );
	}
	
	m_lstGuardedNodeNames.clear();
}

void AINodeGuard::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	// Save guarded node names.

	HSTRING hstrNodeName;
	HSTRING_LIST::iterator hstr_it;
	SAVE_DWORD( m_lstGuardedNodeNames.size() );
	for ( hstr_it = m_lstGuardedNodeNames.begin(); hstr_it != m_lstGuardedNodeNames.end(); ++hstr_it )
	{
		hstrNodeName = *hstr_it;
		SAVE_HSTRING( hstrNodeName );
	}

	// Save guarded nodes.

	HOBJECT hNode;
	HOBJECT_LIST::iterator h_it;
	SAVE_DWORD( m_lstGuardedNodes.size() );
	for ( h_it = m_lstGuardedNodes.begin(); h_it != m_lstGuardedNodes.end(); ++h_it )
	{
		hNode = *h_it;
		SAVE_HOBJECT( hNode );
	}
}

void AINodeGuard::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	// Load guarded node names.

	HSTRING hstrNodeName;
	uint32 cNodeNames;
	LOAD_DWORD( cNodeNames );
	m_lstGuardedNodeNames.reserve( cNodeNames );
	for( uint32 iNodeName=0; iNodeName < cNodeNames; ++iNodeName )
	{
		LOAD_HSTRING( hstrNodeName );
		m_lstGuardedNodeNames.push_back( hstrNodeName );
	}

	// Load guarded nodes.

	HOBJECT hNode;
	uint32 cNodes;
	LOAD_DWORD( cNodes );
	m_lstGuardedNodes.reserve( cNodes );
	for( uint32 iNode=0; iNode < cNodes; ++iNode )
	{
		LOAD_HOBJECT( hNode );
		m_lstGuardedNodes.push_back( hNode );
	}
}

// ----------------------------------------------------------------------- //

void AINodeExitLevel::ReadProp(ObjectCreateStruct* pocs)
{
	// RETAIL: 0x1026de40; a SmartObject name is not taken as a command
	if ( g_pLTServer->GetPropGeneric( "OnUseCmd", &g_gp ) == LT_OK )
	{
		if ( !g_gp.m_String[0] || !g_pAIGoalButeMgr->GetSmartObjectTemplate( g_gp.m_String ) )
		{
			m_sOnUseCmd = g_gp.m_String;
		}
	}

	AINode::ReadProp(pocs);
}

void AINodeExitLevel::Load(ILTMessage_Read *pMsg)
{
	// RETAIL: 0x1026df30
	char szString[1024];
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sOnUseCmd = szString;

	AINode::Load(pMsg);
}

void AINodeExitLevel::Save(ILTMessage_Write *pMsg)
{
	// RETAIL: 0x1026dff0
	SAVE_CHARSTRING( m_sOnUseCmd.c_str( ));

	AINode::Save(pMsg);
}

void AINodeExitLevel::ProcessOnUseCmd(HOBJECT hUser)
{
	// RETAIL: 0x1026e020
	if( g_pCmdMgr->IsValidCmd( m_sOnUseCmd.c_str( )))
	{
		HOBJECT hSender = m_hObject;
		g_pCmdMgr->Process( m_sOnUseCmd.c_str( ), hSender, hUser );
	}
}
