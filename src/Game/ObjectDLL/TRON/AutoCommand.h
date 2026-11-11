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
// BinaryBit and others. Its string is not an std::string: it has copies of
// the string code of its own (retail 0x10012480, 0x10012830), and compares
// with _memicmp (retail 0x10012430): a caseless string.
//
// ----------------------------------------------------------------------- //

#ifndef __AUTOCOMMAND_H__
#define __AUTOCOMMAND_H__

#include <string>
#include "CommandMgr.h"

// A char_traits that compares without case (the name is ours)
struct CaselessCharTraits : public std::char_traits<char>
{
	static int compare(const char* s1, const char* s2, size_t n)
	{
		return _memicmp(s1, s2, n);
	}

	static size_t length(const char* s)
	{
		if (!s) return 0;
		return strlen(s);
	}
};

typedef std::basic_string<char, CaselessCharTraits> CaselessString;

// The command runs whenever told to
class MultiDispatch
{
};

// RETAIL: one member (AutoCommand<SingleDispatch>'s string is at +8);
// not yet identified
class SingleDispatch
{
	protected :

		uint32	m_nTronUnknown0;
};

template <class Dispatch>
class AutoCommand : public Dispatch
{
	public :

		virtual ~AutoCommand() {}

		bool IsEmpty() const { return m_sCommand == ""; }
		const char* GetCommand() const { return m_sCommand.c_str(); }

		void Clear() { m_sCommand.clear(); }

		// Read the command from the property of that name, dropping it
		// when it is not a valid command
		void ReadProp(const ObjectCreateStruct* pStruct, const char* szPropName)
		{
			const char* pszCommand = pStruct->m_cProperties.GetPropString(szPropName, LTNULL);
			if (pszCommand)
			{
				m_sCommand = pszCommand;

				if (!IsEmpty() && !g_pCmdMgr->IsValidCmd(m_sCommand.c_str()))
				{
					g_pLTServer->CPrint("Invalid Command Message: %s", m_sCommand.c_str());
					Clear();
				}
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

#endif // __AUTOCOMMAND_H__
