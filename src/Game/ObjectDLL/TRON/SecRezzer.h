// ----------------------------------------------------------------------- //
//
// MODULE  : SecRezzer.h
//
// PURPOSE : A spawner of security programs at its rez points
//
// RETAIL: Tron 2.0's, not in any source release (retail Object.lto: class
// def 0x102dcb78, size 0x830; vtable 0x10294a64). Only what its rez points
// need is declared yet; the class is still to be written. The names but
// the class's are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SECREZZER_H__
#define __SECREZZER_H__

#include "Spawner.h"
#include <vector>

// Where a security program rezzes in (a SecRezzerPoint's)
struct SecRezzerPointInfo
{
	LTVector	vPos;
	LTRotation	rRot;
};

typedef std::vector<SecRezzerPointInfo> SecRezzerPointList;

class SecRezzer : public Spawner
{
	public :

		SecRezzerPointList& GetPoints();
};

// RETAIL: the rez points are a vector at +0x7c8 of SecRezzer (retail
// 0x100492a7); a view of it stands in until the class is written
struct SecRezzerView { uint8 aPad[0x7c8]; SecRezzerPointList lstPoints; };
inline SecRezzerPointList& SecRezzer::GetPoints() { return ((SecRezzerView*)this)->lstPoints; }

#endif // __SECREZZER_H__
