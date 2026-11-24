// ----------------------------------------------------------------------- //
//
// MODULE  : PropTypeMgr.h
//
// PURPOSE : Definition of prop type mgr
//
// CREATED : 4/27/2000
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROP_TYPE_MGR_H__
#define __PROP_TYPE_MGR_H__

#include "GameButeMgr.h"
#include "TemplateList.h"
#include "ButeListReader.h"
#include "TRON/CaselessString.h"

struct ObjectCreateStruct;
class CPropTypeMgr;
extern CPropTypeMgr* g_pPropTypeMgr;

#define PTMGR_DEFAULT_FILE		"Attributes\\PropTypes.txt"
#define PTMGR_INVALID_ID		-1

#define PROPTYPE_MAX_NAME_LENGTH		32
#define PROPTYPE_MAX_FILE_PATH			64


struct PROP_DISTURB
{
	 PROP_DISTURB();
	~PROP_DISTURB();

	LTFLOAT	fStimRadius;

	LTFLOAT	fTouchSoundRadius;
	CaselessString	sTouchSound;	// RETAIL: caseless (PropType::ReadProp 0x100f55b0 copies it to the caseless PropDisturbStruct::sTouchSoundName)
	uint32	nTouchAlarmLevel;

	uint32	nDestroyAlarmLevel;
	std::string	sDestroyFilename;
	CButeListReader blrDestroySkinReader;
	CButeListReader blrDestroyRenderStyleReader;
	
	uint32	nHitAlarmLevel;
	std::string	sHitSound;
	LTFLOAT	fHitSoundRadius;

	uint32  nPropTypeId;
};


struct PROPTYPE
{
	 PROPTYPE();
	~PROPTYPE();

    LTBOOL      Init(CButeMgr & buteMgr, char* aTagName, uint32 nId);

	void		SetupModel(ObjectCreateStruct* pStruct);

    uint32      nId;

	// RETAIL: caseless, as the two below: their assignments check for NULL
	// (Object.lto 0x100fdfd7, 0x100fe060, 0x100fe19c)
	CaselessInlineString	sType;
	CaselessInlineString	sFilename;
	CaselessInlineString	sDebrisType;
	// RETAIL: Tron's (0x28, Object.lto 0x100fe0fd): the ClientFX played when
	// the prop is destroyed (CDestructibleModel::m_sDestroyFX). Caseless,
	// as is sActivateType (0x34)
	CaselessInlineString	sDeathClientFX;
	CaselessInlineString	sActivateType;

	CButeListReader blrPropSkinReader;
	CButeListReader blrPropRenderStyleReader;

    LTVector    vScale;
    LTVector    vObjectColor;
    LTFLOAT     fAlpha;
    LTBOOL      bVisible;
    LTBOOL      bSolid;
    LTBOOL      bShadow;
	LTBOOL		bGravity;
    LTBOOL      bMoveToFloor;
    LTBOOL      bAdditive;
    LTBOOL      bMultiply;
    LTBOOL      bRayHit;
	LTBOOL		bActivateable;
	LTBOOL		bSearchable;
	LTBOOL		bTouchable;

	int			nHitPoints;

	PROP_DISTURB*	pDisturb;
};

typedef CTList<PROPTYPE*> PropTypeList;


class CPropTypeMgr : public CGameButeMgr
{
	public :

		CPropTypeMgr();
		~CPropTypeMgr();

        void            Reload() { Term(); m_buteMgr.Term(); Init(); }

        LTBOOL           Init(const char* szAttributeFile=PTMGR_DEFAULT_FILE);
		void			Term();

        PROPTYPE*       GetPropType(uint32 nId);
		PROPTYPE*		GetPropType(char* pType);

		int				GetNumPropTypes() const { return m_PropTypeList.GetLength(); }

	private :

		PropTypeList	m_PropTypeList;
};


#ifndef __PSX2
////////////////////////////////////////////////////////////////////////////
//
// CPropTypeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CPropTypeMgr
//
////////////////////////////////////////////////////////////////////////////
#include "iobjectplugin.h"

class CPropTypeMgrPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT	PreHook_EditStringList(
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

        LTBOOL PopulateStringList(char** aszStrings, uint32* pcStrings,
            const uint32 cMaxStrings, const uint32 cMaxStringLength);

	protected :

		static CPropTypeMgr		sm_PropTypeMgr;
};
#endif

#endif // __PROP_TYPE_MGR_H__