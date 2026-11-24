// ----------------------------------------------------------------------- //
//
// MODULE  : LTPoly.h
//
// PURPOSE : Wrapper classes to initialize ltdrawprim structs
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LTPOLY_H__
#define __LTPOLY_H__

//
// Includes...
//

	#include "iltdrawprim.h"

// RETAIL: Tron's LT_POLYGT4 clears itself (iltdrawprim.h), so this wrapper
// does not clear it again: CHUDBar's constructor (CShell.dll 0x101cb7e0)
// clears each poly once.
struct LTPoly_GT4 : public LT_POLYGT4
{
};

#endif // __LTPOLY_H__