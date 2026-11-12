// ----------------------------------------------------------------------- //
//
// MODULE  : AutoCommand.h
//
// PURPOSE : A command an object reads from its properties and runs
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto. The class names are retail's (RTTI: AutoCommand<MultiDispatch>,
// vtable 0x102935b4; AutoCommand<SingleDispatch>, 0x102935d0); the module,
// members and functions are ours. Its users are IONode, ArchiveBin,
// BinaryBit and others. Its string is a CaselessString (CaselessString.h).
//
// ----------------------------------------------------------------------- //

#ifndef __AUTOCOMMAND_H__
#define __AUTOCOMMAND_H__

#include "CaselessString.h"
#include "CommandMgr.h"


// The command runs whenever told to
class MultiDispatch
{
};

// The command runs the first time only (RETAIL: Optimizer's, retail
// 0x100308f3; its flag is saved after the command, retail 0x1003216c)
class SingleDispatch
{
	public :

		SingleDispatch() : m_bDispatched(false) {}

	protected :

		bool	m_bDispatched;
};

template <class Dispatch>
class AutoCommand : public Dispatch
{
	public :

		virtual ~AutoCommand() {}

		bool IsEmpty() const { return m_sCommand == ""; }
		const char* GetCommand() const { return m_sCommand.c_str(); }

		void Clear() { m_sCommand.clear(); }

		// Drop the command when it is not a valid command (RETAIL: out of
		// line where not inlined, retail 0x100324c0, 0x10032550)
		bool Validate()
		{
			if (!IsEmpty() && !g_pCmdMgr->IsValidCmd(m_sCommand.c_str()))
			{
				g_pLTServer->CPrint("Invalid Command Message: %s", m_sCommand.c_str());
				Clear();
				return false;
			}

			return true;
		}

		// Read the command from the property of that name, dropping it
		// when it is not a valid command
		void ReadProp(const ObjectCreateStruct* pStruct, const char* szPropName)
		{
			const char* pszCommand = pStruct->m_cProperties.GetPropString(szPropName, LTNULL);
			if (pszCommand)
			{
				m_sCommand = pszCommand;
				Validate();
			}
			else
			{
				g_pLTServer->CPrint("CommandMessage: Failed to find %s in %s", szPropName, pStruct->m_Name);
				Clear();
			}
		}

		void Execute(HOBJECT hSender, HOBJECT hTarget)
		{
			if (!IsEmpty())
			{
				g_pCmdMgr->Process(m_sCommand.c_str(), hSender, hTarget);
			}
		}

		void Save(ILTMessage_Write* pMsg)
		{
			pMsg->WriteString(m_sCommand.c_str());
		}

		void Load(ILTMessage_Read* pMsg)
		{
			char szCommand[1024];
			pMsg->ReadString(szCommand, sizeof(szCommand));
			m_sCommand = szCommand;
		}

	protected :

		CaselessString	m_sCommand;
};

template <>
inline void AutoCommand<SingleDispatch>::Execute(HOBJECT hSender, HOBJECT hTarget)
{
	if (!m_bDispatched && !IsEmpty())
	{
		g_pCmdMgr->Process(m_sCommand.c_str(), hSender, hTarget);
		m_bDispatched = true;
	}
}

template <>
inline void AutoCommand<SingleDispatch>::Save(ILTMessage_Write* pMsg)
{
	pMsg->WriteString(m_sCommand.c_str());
	pMsg->Writebool(m_bDispatched);
}

template <>
inline void AutoCommand<SingleDispatch>::Load(ILTMessage_Read* pMsg)
{
	char szCommand[1024];
	pMsg->ReadString(szCommand, sizeof(szCommand));
	m_sCommand = szCommand;
	m_bDispatched = pMsg->Readbool();
}

#endif // __AUTOCOMMAND_H__
