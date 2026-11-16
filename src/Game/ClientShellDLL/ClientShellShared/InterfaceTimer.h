// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceTimer.h
//
// PURPOSE : Definition of InterfaceTimer class
//
// CREATED : 10/18/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __INTERFACE_TIMER_H__
#define __INTERFACE_TIMER_H__

#include "ltbasedefs.h"
#include "iltfontmanager.h"
#include "TeamMgr.h"

extern ILTFontManager*	g_pFontManager;

class CInterfaceTimer
{
	public:

		// RETAIL: Tron's constructor is inline (build 51's; CInterfaceMgr's
		// constructor, 0x100c6906..0x100c6918, sets the members)
		CInterfaceTimer()
		{
			m_fTime		= 0.0f;
			m_bPause	= false;
			m_pTimeStr	= LTNULL;
			m_nBaseSize = 0;
		}
		~CInterfaceTimer();

		void		Draw();

		// RETAIL: reconstructed from the retail 1.3 CShell.dll (0x1005c484,
		// 0x10063f4e): the timer has no m_fTimeLeft, and SetTime only
		// stores; the source release also kept the time left while paused.
		void        SetTime(LTFLOAT fTime, bool bPause) { m_fTime = fTime; m_bPause = bPause; }
		float		GetTime() const { return m_fTime; }

	private:

		// RETAIL: Tron's timer is 0x18 bytes (build 51's members): it has no
		// team or color (CInterfaceMgr's m_Credits is at +0xe91c)

		// Server game time when timer runs out.
		float					m_fTime;
		bool					m_bPause;

		CUIFormattedPolyString	*m_pTimeStr;

		LTIntPt			m_BasePos;
		uint8			m_nBaseSize;
};

#endif  // __INTERFACE_TIMER_H__