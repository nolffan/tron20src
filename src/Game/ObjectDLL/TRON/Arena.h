// ----------------------------------------------------------------------- //
//
// MODULE  : Arena.h
//
// PURPOSE : A disc arena of a multiplayer level: its number and the
//           commands of its match and point events
//
// RETAIL: Tron 2.0's, written from retail Object.lto (RTTI .?AVArena@@,
//         vtable 0x10297fcc, ClassDef 0x102f71d0, 0x58 bytes; code
//         0x10110370..0x10110c0e). Member names are ours. What sends the
//         event commands is not in this file.
//
// ----------------------------------------------------------------------- //

#ifndef __ARENA_H__
#define __ARENA_H__

#include "GameBase.h"
#include "CommandMgr.h"

class Arena : public GameBase
{
	public :

		Arena();				// RETAIL: 0x101106f0
		virtual ~Arena();		// RETAIL: 0x10110750

	protected :

		virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);	// 0x10110840

	private :

		LTBOOL	ReadProp(ObjectCreateStruct *pStruct);		// RETAIL: 0x101108d0

		int		m_nArena;					// 0x38 "Arena" (1..4) less one
		HSTRING	m_hstrName;					// 0x3c
		HSTRING	m_hstrOnPlayerSpawnCmd;		// 0x40
		HSTRING	m_hstrOnPlayerDeathCmd;		// 0x44
		HSTRING	m_hstrOnMatchStartCmd;		// 0x48
		HSTRING	m_hstrOnMatchEndCmd;		// 0x4c
		HSTRING	m_hstrOnPointStartCmd;		// 0x50
		HSTRING	m_hstrOnPointEndCmd;		// 0x54
};

class CArenaPlugin : public IObjectPlugin
{
	public :

		// RETAIL: 0x10110b00: the arena numbers 1..4
		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);

		// RETAIL: 0x10110b70: the event commands
		virtual LTRESULT PreHook_PropChanged(
			const	char		*szObjName,
			const	char		*szPropName,
			const	int			nPropType,
			const	GenericProp	&gpPropValue,
					ILTPreInterface	*pInterface,
			const	char		*szModifiers );

	private :

		CCommandMgrPlugin	m_CommandMgrPlugin;
};

#endif // __ARENA_H__
