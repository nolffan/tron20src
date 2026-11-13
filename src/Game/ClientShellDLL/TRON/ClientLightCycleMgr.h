// ----------------------------------------------------------------------- //
//
// MODULE  : ClientLightCycleMgr.h
//
// PURPOSE : The client's light cycle manager: draws the trails, predicts
//           the cycles and talks to the server about them.
//
// RETAIL: Tron 2.0's, retail-only (RTTI .?AVCClientLightCycleMgr@@, vtable
// 0x101e56d0). Written from retail CShell.dll 0x100106c0..0x100160a0; it is
// a member of CTronGameClientShell (retail +0x32e00, 0x2844 bytes). The
// class name is Monolith's; the other names are ours unless said.
//
// RETAIL: layout. The offsets below are retail's (and ours).
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENTLIGHTCYCLEMGR_H__
#define __CLIENTLIGHTCYCLEMGR_H__

#include "LightCycleMgr.h"
#include "iltdrawprim.h"

// The trail quads are kept in blocks of this many (RETAIL: 0x400)
#define LCM_TRAIL_POLYS_PER_BUFFER	1024
// The most blocks (RETAIL: the array at +0x133c, 0xfa0 bytes)
#define LCM_MAX_TRAIL_BUFFERS		1000
// The trail start quads (RETAIL: the array at +0x22dc, 16 of 0x54 bytes)
#define LCM_MAX_START_POLYS			16

class CClientLightCycleMgr : public CLightCycleMgr
{
	public:

		CClientLightCycleMgr();					// RETAIL: 0x100107b0
		virtual ~CClientLightCycleMgr();		// RETAIL: 0x10010a90

		virtual bool Init();					// RETAIL: 0x10010bc0
		virtual void Term();					// RETAIL: 0x10010f50

		virtual bool CyclistCrash(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLIST* pOther, bool bNotify);	// RETAIL: 0x10010fb0
		virtual void UpdateCyclists();			// RETAIL: 0x10011410

		virtual bool TrimTrail(LIGHT_CYCLIST* pCyclist, int nTrail, int nPoint, bool bNotify);	// RETAIL: 0x10014620
		virtual void UpdateCyclistDims(LIGHT_CYCLIST* pCyclist, LTVector & vDims, bool bTronUnknown);	// RETAIL: 0x100143d0
		virtual void RemoveAllCyclists();		// RETAIL: 0x100145f0
		virtual bool SetCyclistPowerup(LIGHT_CYCLIST* pCyclist, int nPowerup);	// RETAIL: 0x100148c0
		virtual void DeactivateShield(LIGHT_CYCLIST* pCyclist, bool bNotify);	// RETAIL: 0x10014770

		virtual std::string GetCyclistName(HOBJECT hObj);	// RETAIL: 0x10012dd0
		virtual void CyclistMovementChanged(LIGHT_CYCLIST* pCyclist, bool bTurned, float fAccel, float fWallAccel, uint32 nFlags);	// RETAIL: 0x10013c90
		virtual void TrailCut(LightCycleCollisionInfo* pInfo);	// RETAIL: 0x100147f0

		// Draws all the trails (the canvas' function). RETAIL: 0x10011ac0
		void DrawTrails();

	protected:

		// Shows a line in the chat messages. RETAIL: 0x10012db0
		void ShowMessage(const char* szMsg);

		// Fills the trail quads of the cyclist's trails. RETAIL: 0x10013210
		void BuildCyclistPolys(LIGHT_CYCLIST* pCyclist);
		// RETAIL: 0x100132f0
		void BuildTrailPolys(LIGHT_CYCLE_TRAIL* pTrail, uint8 nRed, uint8 nGreen, uint8 nBlue,
			float fHeight, float fBase, bool bCurrentTrail, bool bAlive);
		// Adds a wall quad from pvStart to pvEnd. RETAIL: 0x10013970
		void AddTrailPoly(LIGHT_CYCLE_TRAIL_POINT* pvStart, LIGHT_CYCLE_TRAIL_POINT* pvEnd, uint8 nRed, uint8 nGreen, uint8 nBlue,
			float fHeight, float fBase, float fDerez, float fDerezStartTime, bool bAlive);
		// Adds the quad at the head of a trail. RETAIL: 0x10013b80
		void AddStartPoly(LIGHT_CYCLE_TRAIL_POINT* pvStart, LIGHT_CYCLE_TRAIL_POINT* pvEnd, uint8 nRed, uint8 nGreen, uint8 nBlue,
			float fHeight, float fBase, float fDerez);
		// Adds a block of trail quads. RETAIL: 0x100156f0
		bool AllocTrailBuffer();

		LT_POLYFT4*	m_apTrailPolys[LCM_MAX_TRAIL_BUFFERS];	// RETAIL: +0x133c
		LT_POLYFT4	m_aStartPolys[LCM_MAX_START_POLYS];		// RETAIL: +0x22dc
		int			m_nTrailPolys;			// RETAIL: +0x281c used in the current block
		int			m_nTrailBuffers;		// RETAIL: +0x2820 blocks allocated
		int			m_nTrailBuffer;			// RETAIL: +0x2824 the current block
		int			m_nStartPolys;			// RETAIL: +0x2828
		HTEXTURE	m_hTailTex;				// RETAIL: +0x282c props\skins\Tail.dtx
		HTEXTURE	m_hTailStartTex;		// RETAIL: +0x2830 props\skins\TailStart.dtx
		HLTSOUND	m_hDialogSnd;			// RETAIL: +0x2834 (0x10013100, the message handler)
		uint32		m_nTronUnknown2838;		// RETAIL: +0x2838 (not touched in this TU)
		float		m_fLastPredictTime;		// RETAIL: +0x283c (UpdateCyclists)
		float		m_fPredictInterval;		// RETAIL: +0x2840 (1/15 s)
};

#endif // __CLIENTLIGHTCYCLEMGR_H__
