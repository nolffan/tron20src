// ----------------------------------------------------------------------- //
//
// MODULE  : DestructibleModel.h
//
// PURPOSE : Destructible model aggregate class
//
// CREATED : 4/23/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DESTRUCTABLE_MODEL_H__
#define __DESTRUCTABLE_MODEL_H__

#include "Destructible.h"
#include "DebrisFuncs.h"
#include "SurfaceMgr.h"
#include "TRON/CaselessString.h"

#define DEFAULT_DWMA_MASS			2000.0f

class CFXButeMgrPlugin;
enum EnumAIStimulusID;


// Use ADD_DESTRUCTIBLE_WORLD_MODEL_AGGREGATE() in your class definition
// to enable the following properties in the editor.  For example:
//
//BEGIN_CLASS(CMyCoolObj)
//	ADD_DESTRUCTIBLE_WORLD_MODELAGGREGATE()
//	ADD_STRINGPROP(Filename, "")
//  ...
//

#define ADD_DESTRUCTIBLE_MODEL_AGGREGATE(group, flags) \
	ADD_DESTRUCTIBLE_AGGREGATE((group), (flags)) \
	ADD_REALPROP_FLAG(Mass, DEFAULT_DWMA_MASS, (group) | (flags)) \
	PROP_DEFINEGROUP(ExplosionProperties, ((group)<<1) | (flags)) \
		ADD_BOOLPROP_FLAG(CreateExplosion, 0, ((group)<<1) | (flags)) \
		ADD_LONGINTPROP_FLAG(WeaponId, 0, ((group)<<1) | (flags)) \
		ADD_BOOLPROP_FLAG(FireAlongForward, 0, ((group)<<1) | (flags)) \
		ADD_REALPROP_FLAG(DamageFactor, 1.0f, ((group)<<1) | (flags)) \
		ADD_STRINGPROP_FLAG(Spawn, "", ((group)<<1) | (flags)) \
	ADD_DEBRISTYPE_PROPERTY((flags)) \
	ADD_STRINGPROP_FLAG(SurfaceOverride, "", (flags) | PF_STATICLIST)

#ifndef __PSX2
class CDestructibleModelPlugin : public CDestructiblePlugin
{

  public:
	CDestructibleModelPlugin();
	virtual ~CDestructibleModelPlugin();
    virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

  private:

	CFXButeMgrPlugin*	m_pFXButeMgrPlugin;
	CDebrisPlugin		m_DebrisPlugin;
	CSurfaceMgrPlugin	m_SurfaceMgrPlugin;

	static LTBOOL		sm_bInitted;
};
#endif

class CDestructibleModel : public CDestructible
{
	public :

		CDestructibleModel();
		virtual ~CDestructibleModel();

		// RETAIL: Tron's (slot 0x14, retail 0x10110310): the damage filter
		// (DestructibleModelFilterHook) calls it; for two damage types (0x21,
		// 0x23) with a duration it scales the damage by the frame time and
		// clears the duration. Always returns true. Not yet named
		virtual bool	TronUnknownVirtual14( DamageStruct* pDamage );

        void        DoExplosion(char* pTargetName=LTNULL);
        void        SetDestroyedStimulus(LTFLOAT fStimRadius, uint32 nDestroyAlarmLevel);

        void		Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void		Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		// RETAIL: Tron's: a ClientFX played at the object when it is destroyed
		// (0x6e8, Object.lto 0x1010f910; PropType::Setup sets it from the prop
		// type, 0x100f58b5). The name is ours
		CaselessInlineString	m_sDestroyFX;

        uint8       m_nDebrisId;

        LTBOOL      m_bCreatedDebris;

        LTBOOL      m_bCreateExplosion;
        uint8       m_nExplosionWeaponId;
        LTBOOL      m_bFireAlongForward;
        LTFLOAT     m_fDamageFactor;

        LTBOOL      m_bRemoveOnDeath;

		HSTRING		m_hstrSpawn;
		HSTRING		m_hstrSurfaceOverride;

		uint32		m_dwOriginalFlags;
		LTBOOL		m_bSaveNeverDestroy;
		LTBOOL		m_bSaveCanDamage;

		// RETAIL: Tron's (0x724): set while "HIDDEN 1" is in effect, so that a
		// second HIDDEN 1 does not overwrite the saved damage flags
		// (Object.lto 0x1010f3e6). The name is ours
		bool		m_bHidden;

	protected:

		EnumAIStimulusID	m_eStimID;
		LTFLOAT				m_fStimRadius;
		uint32				m_nDestroyAlarmLevel;

	protected:

        uint32  EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT lData);
        uint32  ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);

        LTBOOL   ReadProp(ObjectCreateStruct *);
		void	CreateWorldModelDebris();
		void	CreateDebris();
		void	SetSurfaceType();
		void	RegisterDestroyedStimulus();
		// RETAIL: Tron's (Object.lto 0x1010f910), the name is ours
		void	PlayDestroyFX();

	private :

		void SpawnItem();
};

#endif // __DESTRUCTABLE_MODEL_H__