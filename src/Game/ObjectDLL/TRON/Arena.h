// ----------------------------------------------------------------------- //
//
// MODULE  : Arena.h
//
// PURPOSE : Arena - a disc arena of the level (its index and commands)
//
// RETAIL: Tron 2.0 only (RTTI Arena, vtable 0x10297fcc; class def at
//         0x102f71d0, 0x58 bytes; its TU is retail 0x10110660..0x10110b70).
//         PORT: only the data members that the disc game types' match
//         manager reads (ServerDiscMatchMgr.cpp) are declared here; the
//         class's functions are not yet written. The names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __ARENA_H__
#define __ARENA_H__

#include "GameBase.h"

class Arena : public GameBase
{
	public :

		int			m_nIndex;			// RETAIL: 0x38
		uint32		m_nTronUnknown3C;	// RETAIL: 0x3c
		HSTRING		m_hstrRoundStartCmd;	// RETAIL: 0x40
		HSTRING		m_hstrKillCmd;		// RETAIL: 0x44
		HSTRING		m_hstrMatchStartCmd;	// RETAIL: 0x48
		HSTRING		m_hstrMatchEndCmd;	// RETAIL: 0x4c
		HSTRING		m_hstrFightCmd;		// RETAIL: 0x50
		HSTRING		m_hstrRoundEndCmd;	// RETAIL: 0x54
};

#endif // __ARENA_H__
