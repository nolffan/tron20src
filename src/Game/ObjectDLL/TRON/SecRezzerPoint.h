// ----------------------------------------------------------------------- //
//
// MODULE  : SecRezzerPoint.h
//
// PURPOSE : A point a SecRezzer rezzes its programs in at
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x10048f70..0x10049390; its class def at 0x102dcec0, vtable
// 0x10294af8). The names of the class and its property are retail's; the
// rest are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SECREZZERPOINT_H__
#define __SECREZZERPOINT_H__

#include "ltengineobjects.h"

class SecRezzerPoint : public BaseClass
{
	public :

		SecRezzerPoint();
		virtual ~SecRezzerPoint() {}

	protected :

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual uint32	OnAllObjectsCreated();

	private :

		LTBOOL	ReadProp(ObjectCreateStruct *pStruct);
		LTBOOL	InitialUpdate();

		char		m_szSecRezzer[128];		// The SecRezzer we are a point of
		LTVector	m_vPos;
		LTRotation	m_rRot;
};

#endif // __SECREZZERPOINT_H__
