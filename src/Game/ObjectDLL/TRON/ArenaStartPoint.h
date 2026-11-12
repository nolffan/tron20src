// ----------------------------------------------------------------------- //
//
// MODULE  : ArenaStartPoint.h
//
// PURPOSE : ArenaStartPoint - where a player (or spectator) of a disc
//           arena starts.
//
// RETAIL: Tron 2.0 only, written from retail Object.lto (RTTI
//         ArenaStartPoint, vtable 0x102958ac; TU 0x10086de0..0x100872b0, its
//         class def at 0x102ea978). The names of members and the plugin's
//         are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __ARENASTARTPOINT_H__
#define __ARENASTARTPOINT_H__

#include "GameBase.h"

LINKTO_MODULE( ArenaStartPoint );

class ArenaStartPoint : public GameBase
{
	public :

		ArenaStartPoint();
		virtual ~ArenaStartPoint();

		// RETAIL: the arena types ("Type", retail .rdata 0x102ea968)
		enum ArenaStartType
		{
			kType_Spectator,
			kType_Team1,
			kType_Team2,
			kNumTypes,
		};

	protected :

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

	private :

		LTBOOL	ReadProp(ObjectCreateStruct *pData);

		ArenaStartType	m_eType;		// RETAIL: 0x38
		int				m_nArena;		// RETAIL: 0x3c, 0..3 ("Arena" 1..4)
		HSTRING			m_hstrName;		// RETAIL: 0x40
		LTVector		m_vRotation;	// RETAIL: 0x44, Euler angles
};

class CArenaStartPointPlugin : public IObjectPlugin
{
	public :

		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);
};

#endif // __ARENASTARTPOINT_H__
