// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeExitLevelByRezOut.h
//
// PURPOSE : Tron's exit level node where the AI rezzes out with a smart
//           object. Retail only (RTTI .?AVAINodeExitLevelByRezOut@@,
//           vtable 0x102a1030, ClassDef 0x10336280, sizeof 0xe4; Object.lto
//           0x1026e080..0x1026ea6c). CAIGoalExitLevelByRezOut uses it.
//           The file name and the names of members and functions that the
//           properties do not give are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __AINODE_EXIT_LEVEL_BY_REZ_OUT_H__
#define __AINODE_EXIT_LEVEL_BY_REZ_OUT_H__

#include "AINodeGuard.h"

LINKTO_MODULE( AINodeExitLevelByRezOut );

class AINodeExitLevelByRezOut : public AINodeGuard
{
	public :

		// RETAIL: DefaultConstructor 0x1026e500
		AINodeExitLevelByRezOut() { m_fRadiusSqr = 0.f; }

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pocs);		// RETAIL: 0x1026e720

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);				// RETAIL: 0x1026e870
		virtual void Save(ILTMessage_Write *pMsg);				// RETAIL: 0x1026e940

		// The smart object's command for this node type, or "".

		const char* GetSmartObjectCommand();					// RETAIL: 0x1026e980

		// Command

		void ProcessOnUseCmd(HOBJECT hUser);					// RETAIL: 0x1026ea10

		// Type

		// RETAIL: 0x1026e590
		EnumAINodeType GetType() { return kNode_ExitLevelByRezOut; }

	protected :

		uint32			m_nSmartObjectID;	// +0xd4, "SmartObject"
		std::string		m_sOnUseCmd;		// +0xd8, "OnUseCmd"
};

#endif // __AINODE_EXIT_LEVEL_BY_REZ_OUT_H__
