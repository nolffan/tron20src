// ----------------------------------------------------------------------- //
//
// MODULE  : ArchiveItem.h
//
// PURPOSE : An item an archive bin or a core dump holds: a subroutine,
//			 procedural, primitive, permission set, email, custom command
//			 or help file the player can take
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto. The item is 0x2c bytes (ArchiveBin, CoreDump and Body keep
// arrays of them; its inline constructor and destructor are emitted at
// 0x1000d6c0, 0x1000d7c0). The type, build and condition names are
// retail's string tables (ArchiveBin's copy at 0x10293408, CoreDump's at
// 0x102937fc, CServerSubroutineMgr's at 0x10295910). The module, struct,
// member and function names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __ARCHIVEITEM_H__
#define __ARCHIVEITEM_H__

#include <string>

// What an item is (ArchiveBin's TypeN property)
enum ArchiveItemType
{
	kAIT_None = -1,
	kAIT_Subroutine = 0,
	kAIT_Procedural,
	kAIT_Primitive,
	kAIT_PermissionSet,
	kAIT_Email,
	kAIT_Custom,
	kAIT_HelpFile,

	kAIT_NumTypes
};

// A subroutine's build (ArchiveBin's SubroutineBuildN property)
enum ArchiveItemBuild
{
	kAIB_Alpha = 0,
	kAIB_Beta,
	kAIB_Gold,

	kAIB_NumBuilds
};

// A subroutine's condition (ArchiveBin's SubroutineConditionN property)
enum ArchiveItemCondition
{
	kAIC_Normal = 0,
	kAIC_Infected,
	kAIC_Foreign,

	kAIC_NumConditions
};

// RETAIL: static: each user has its copy of the tables it uses
static const char* const s_aszArchiveItemTypes[kAIT_NumTypes] =
{
	"Subroutine",
	"Procedural",
	"Primitive",
	"Permission Set",
	"Email",
	"Custom",
	"HelpFile",
};

static const char* const s_aszArchiveItemBuilds[kAIB_NumBuilds] =
{
	"Alpha",
	"Beta",
	"Gold",
};

static const char* const s_aszArchiveItemConditions[kAIC_NumConditions] =
{
	"Normal",
	"Infected",
	"Foreign",
};

struct ArchiveItem
{
	ArchiveItem()
		: m_eType		( kAIT_None ),
		  m_eBuild		( -1 ),
		  m_eCondition	( -1 ),
		  m_nPSet		( 0 ),
		  m_fCost		( 0.0f )
	{
	}

	// Empty the slot
	void Clear()
	{
		m_eType = kAIT_None;
		m_sName.clear();
		m_eBuild = -1;
		m_eCondition = -1;
		m_sCommand.clear();
		m_nPSet = 0;
		m_fCost = 0.0f;
	}

	int			m_eType;		// ArchiveItemType
	std::string	m_sName;		// What the item is, or the custom command's name
	int			m_eBuild;		// ArchiveItemBuild (subroutines)
	int			m_eCondition;	// ArchiveItemCondition (subroutines)
	std::string	m_sCommand;		// The custom command
	uint8		m_nPSet;		// The permission set's number
	float		m_fCost;
};

#endif // __ARCHIVEITEM_H__
