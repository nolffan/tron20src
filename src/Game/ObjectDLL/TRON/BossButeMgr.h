// ----------------------------------------------------------------------- //
//
// MODULE  : BossButeMgr.h
//
// PURPOSE : The boss's (Thorne's) attributes
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x10015170..0x10015810, vtable 0x10293674). The class name
// and its bute tags are retail's; the module, members and functions are
// ours.
//
// ----------------------------------------------------------------------- //

#ifndef __BOSSBUTEMGR_H__
#define __BOSSBUTEMGR_H__

#include "GameButeMgr.h"
#include "CaselessString.h"
#include <map>

class CBossButeMgr;
extern CBossButeMgr* g_pBossButeMgr;

// Thorne's animations, in the order of their bute labels (Disrupt<n>...)
enum ThorneAnim
{
	kThorneAnim_Disrupt = 0,
	kThorneAnim_Summon,
	kThorneAnim_Angry,
	kThorneAnim_CreateBall,
	kThorneAnim_ChargeBall,
	kThorneAnim_ThrowBall,

	kNumThorneAnims
};

// RETAIL: one aggregate member of CBossButeMgr (its constructor and
// destructor work through its address, retail 0x100151d1, 0x10015369)
struct ThorneAttributes
{
	// RETAIL: keyed by ThorneAnim (ReadThorne's insert 0x1001577a, Thorne's
	// lookup 0x10059dfb)
	typedef std::multimap<int, std::string> AnimMap;

	LTFLOAT			fAngerChance;
	LTFLOAT			fBaseBallHealth;
	CARange			rngChargeTime;
	AnimMap			mapAnims;			// Each animation to its ThorneAnim
	LTVector		vFireOffset;
	CaselessString	sFireSocketName;
	CaselessString	sHittableNode;
};

class CBossButeMgr : public CGameButeMgr
{
	public :

		CBossButeMgr();
		~CBossButeMgr() {}

		LTBOOL	Init(const char* szAttributeFile);
		void	Term() {}

		// RETAIL: Thorne reads these directly (Thorne.cpp)
		const ThorneAttributes&	GetThorne() const { return m_Thorne; }

	protected :

		void	ReadThorne();

		ThorneAttributes	m_Thorne;
};

#endif // __BOSSBUTEMGR_H__
