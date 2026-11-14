// ----------------------------------------------------------------------- //
//
// MODULE  : PropType.h
//
// PURPOSE : Model PropType - Definition
//
// CREATED : 4/26/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROP_TYPE_H__
#define __PROP_TYPE_H__

#include "Prop.h"
#include "PropTypeMgr.h"

LINKTO_MODULE( PropType );


class PropType : public Prop
{
	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

	private :

		// RETAIL: Tron's (Object.lto 0x100f55b0); Tron's PropType has no
		// members, no Save/Load and no runtime SetPropType
		void ReadProp(ObjectCreateStruct *pData);
};

// PORT: NOLF2's runtime prop types for NOLF2's DoomsDay objects (not in
// Tron; PropType.cpp). Ours
class NOLF2PropType : public PropType
{
	public :

		const char* GetPropTypeName( ) { return m_sPropType.c_str(); }

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

		bool	SetPropType( char const* pszPropType );

		bool	Setup( );

	protected:

		std::string m_sPropType;

	private :

		bool ReadProp(ObjectCreateStruct *pData);
        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
};

#ifndef __PSX2
class CPropTypePlugin : public CPropPlugin
{
  public:

    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);

	virtual LTRESULT PreHook_Dims(
			const char* szRezPath,
			const char* szPropValue,
			char* szModelFilenameBuf,
			int	  nModelFilenameBufLen,
			LTVector & vDims);

  protected :

	  	  CPropTypeMgrPlugin m_PropTypeMgrPlugin;
};
#endif

#endif // __PROP_H__