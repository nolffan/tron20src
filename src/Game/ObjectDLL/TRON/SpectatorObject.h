// ----------------------------------------------------------------------- //
//
// MODULE  : SpectatorObject.h
//
// PURPOSE : A camera position for the multiplayer spectator view
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x10150050..0x10150520, its class def at 0x102fcfc8). The
// names of the class, its property and its special FX id are retail's;
// the rest are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SPECTATOROBJECT_H__
#define __SPECTATOROBJECT_H__

#include "GameBase.h"

class SpectatorObject : public GameBase
{
	public :

		SpectatorObject();
		virtual ~SpectatorObject();

		typedef std::vector< SpectatorObject* > SpectatorObjectList;
		static SpectatorObjectList const& GetSpectatorObjectList() { return m_lstSpectatorObjects; }

		// RETAIL: CGameServerShell (0x100df723) looks for the primary one
		bool		IsPrimary() const { return m_bPrimary; }

	protected :

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

	private :

		LTBOOL	ReadProp(ObjectCreateStruct *pStruct);
		void	CreateSpecialFX();

		LTVector	m_vPitchYawRoll;	// +0x38
		bool		m_bPrimary;			// +0x44

		static SpectatorObjectList m_lstSpectatorObjects;
};

#endif // __SPECTATOROBJECT_H__
