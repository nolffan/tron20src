// ----------------------------------------------------------------------- //
//
// MODULE  : IObjectRelationMgrUser.h
//
// PURPOSE : What an object owning a CObjectRelationMgr tells about itself
//
// RETAIL: Tron's: a base of CCharacter (its RTTI: at +0x40), with five
// pure virtuals (retail Object.lto's vtable 0x10295454). The interface's
// name is retail's; its functions' names are ours, from what CCharacter's
// do (0x100628e0, 0x10092110, 0x100920f0, 0x10093920, 0x10062950).
//
// ----------------------------------------------------------------------- //

#ifndef __IOBJECTRELATIONMGRUSER_H__
#define __IOBJECTRELATIONMGRUSER_H__

class CRelationUser;
class CDataUser;

class IObjectRelationMgrUser
{
	public :

		virtual LTBOOL			IsDead() const = 0;
		virtual CRelationUser*	GetRelationUser() = 0;
		virtual CDataUser*		GetDataUser() = 0;
		virtual HOBJECT			GetRegionObject() = 0;
		virtual HOBJECT			GetUserObject() const = 0;
};

#endif // __IOBJECTRELATIONMGRUSER_H__
