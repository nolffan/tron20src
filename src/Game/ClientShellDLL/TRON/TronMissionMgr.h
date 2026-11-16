// ----------------------------------------------------------------------- //
//
// MODULE  : TronMissionMgr.h
//
// PURPOSE : Tron's mission manager: the light cycle circuits' end
//
// RETAIL: written from the retail CShell.dll (vtable 0x101e8e70; the
// functions 0x100a32e0..0x100a4113). It is CTronGameClientShell's member
// (+0x317d4, 0x3c bytes) and the shell's GetMissionMgr (0x10099d30).
// The class's name is ours; its members' names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __TRONMISSIONMGR_H__
#define __TRONMISSIONMGR_H__

#include "MissionMgr.h"
#include <string>

class CTronMissionMgr : public CMissionMgr
{
	public:

		// RETAIL: inline in CTronGameClientShell's constructor (0x10099a2a)
		CTronMissionMgr() {}

		// RETAIL: 0x100a3fd0: the text of the circuit just completed (the
		// unlocks), which the circuit screen shows on focus
		std::wstring&	GetCircuitMessage();

		// RETAIL: 0x100a3fe0
		void			ClearCircuitMessage();

	protected:

		// RETAIL: slot 1 (0x100a32e0): the server's message 194, a light
		// cycle circuit won: the profile's unlocks
		virtual bool	TronUnknownVirtual4( ILTMessage_Read& msg );

		// RETAIL: slot 2 (0x100a4080): the server's message 195, the next
		// level of the circuit by index (-1: the circuit's end)
		virtual bool	TronUnknownVirtual8( ILTMessage_Read& msg );

	private:

		std::wstring	m_sCircuitMessage;	// RETAIL: +0x30
};

#endif // __TRONMISSIONMGR_H__
