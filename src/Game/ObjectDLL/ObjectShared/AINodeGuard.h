// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeGuard.h
//
// PURPOSE : AINodeGuard class definition
//
// CREATED : 11/13/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_GUARD_H_
#define _AI_NODE_GUARD_H_

#include "AINode.h"

LINKTO_MODULE( AINodeGuard );


typedef std::vector< HSTRING > HSTRING_LIST;
typedef std::vector< LTObjRef > HOBJECT_LIST;


class AINodeGuard : public AINode
{
	typedef AINode super;

	public :

		// Ctors/Dtors/etc

		AINodeGuard();
		virtual ~AINodeGuard();

		// Engine 

		virtual void ReadProp(ObjectCreateStruct *pocs);

		// Init

		virtual void Init();

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);
		
		// Type

		EnumAINodeType GetType() { return kNode_Guard; }

	protected :

		HSTRING_LIST	m_lstGuardedNodeNames;
		HOBJECT_LIST	m_lstGuardedNodes;
};

//-----------------------------------------------------------------

class AINodeTalk : public AINodeGuard
{
	public :
		// Type
		EnumAINodeType GetType() { return kNode_Talk; }
};

//-----------------------------------------------------------------

// RETAIL: Tron's exit level node has an OnUseCmd (DefaultAINodeExitLevel-
// Constructor 0x1026d4f0: a std::string at +0xd4, sizeof 0xe0) and reads,
// loads and saves it before AINode's own (0x1026de40, 0x1026df30, 0x1026dff0:
// it skips AINodeGuard's). CAIGoalExitLevel runs the command (0x1026e020).

class AINodeExitLevel : public AINodeGuard
{
	public :		
		AINodeExitLevel() { m_fRadiusSqr = 0.f; }

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pocs);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Command (our name)

		void ProcessOnUseCmd(HOBJECT hUser);

		// Type
		EnumAINodeType GetType() { return kNode_ExitLevel; }

	protected :

		std::string		m_sOnUseCmd;
};

//-----------------------------------------------------------------

#endif // _AI_NODE_GUARD_H_
