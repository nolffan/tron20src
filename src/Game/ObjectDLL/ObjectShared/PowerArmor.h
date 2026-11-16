//----------------------------------------------------------------------------
//              
//	MODULE:		PowerArmor.h
//              
//	PURPOSE:	PowerArmor declaration
//              
//	CREATED:	13.03.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __POWERARMOR_H__
#define __POWERARMOR_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#include "Prop.h"
#include "IHitBoxUser.h"

// Forward declarations

// Globals

// Statics

LINKTO_MODULE( PowerArmor );

const char KEY_TURNON[]			= "ARMOR_ON";
const char KEY_TURNOFF[]		= "ARMOR_OFF";
// RETAIL: Tron's (Object.lto .rdata 0x10298790..0x102987e4)
const char KEY_LOCKON[]			= "ARMOR_LOCK_ON";
const char KEY_LOCKOFF[]		= "ARMOR_LOCK_OFF";
const char KEY_UNLOCK[]			= "ARMOR_UNLOCK";
const char KEY_REMOVE[]			= "ARMOR_REMOVE";
const char KEY_CANDAMAGE[]		= "ARMOR_CAN_DAMAGE";
const char KEY_CANTDAMAGE[]		= "ARMOR_CANT_DAMAGE";

class CProjectile;
class CAttachmentPosition;

//----------------------------------------------------------------------------
//              
//	CLASS:		PowerArmor
//              
//	PURPOSE:	Armor which can be turned on and off by the owner at will.
//				Does NOT actually take damage -- instead, the AI can route
//				some of the damage to the PowerArmor for reduction.
//              
//----------------------------------------------------------------------------
class PowerArmor : public Prop
{
	public:
		// Public members

		// Ctors/Dtors/etc
		PowerArmor();
		virtual ~PowerArmor();

		// RETAIL: Tron's (Object.lto 0x1013bb40): a projectile hits the
		// armor at pAttachPos; the armor takes its damage (or, at the two
		// easier difficulties, blocks some damage types whole) and is
		// destroyed when its hit points run out. Returns true when the armor
		// handled the hit. The name is ours
		bool HandleProjectileImpact( CProjectile* pProjectile, CAttachmentPosition* pAttachPos );

		// RETAIL: Tron's (inline: CCharacter 0x100942b4, CAI::InitAttachments
		// 0x1012f9a8)
		LTFLOAT GetHitPoints() const { return m_fHitPoints; }
		LTFLOAT GetMaxHitPoints() const { return m_fMaxHitPoints; }

	protected:
		// Protected members

		// Engine Interaction:
		virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual uint32  ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);
		virtual void Load(ILTMessage_Read *pMsg);

		void Update(void);

		void HandleOn();
		void HandleOff();
		void HandleTurningOn();
		void HandleTurningOff();

		// RETAIL: Tron's (Object.lto 0x1013ba20)
		void ReadProp(ObjectCreateStruct *pStruct);

	private:
		// Private members

		// Copy Constructor and Asignment Operator private to prevent 
		// automatic generation and inappropriate, unintentional use
		PowerArmor(const PowerArmor& rhs) {}
		PowerArmor& operator=(const PowerArmor& rhs ) {}

		// Save:
		enum ePA_State
		{
			kPA_On,
			kPA_Off,
			kPA_TurningOn,
			kPA_TurningOff,
		};

		// RETAIL: Tron's lock (ARMOR_LOCK_ON/OFF; the names are ours)
		enum ePA_Lock
		{
			kPA_LockedOn,
			kPA_LockedOff,
			kPA_Unlocked,
		};

		ePA_State	m_eState;		// What is the armor doing?
		// RETAIL: Tron has no m_bDisabled; its members (0x844..0x860) are
		// below, the names ours
		ePA_Lock	m_eLock;
		std::string	m_sOnDestroyFX;	// "OnDestroyFX"
		bool		m_bAttached;	// cleared by ARMOR_REMOVE
		LTFLOAT		m_fHitPoints;
		LTFLOAT		m_fMaxHitPoints;
		bool		m_bCanDamage;	// ARMOR_CAN_DAMAGE / ARMOR_CANT_DAMAGE
};


//----------------------------------------------------------------------------
//              
//	CLASS:		OccludingArmor
//
//	RETAIL: Tron's base of ShellArmor and ThorneBall (retail vtables
//	0x102988bc..., its class def 0x102fa9f8). The code NOLF2 has as
//	ShellArmor is OccludingArmor's in Tron; ShellArmor only overrides
//	HandleDeath.
//              
//	PURPOSE:	Special Armor which DOES take damage.  Using it results in a 
//				big performance hit, because it collides with the owner 
//				constantly.  OccludingArmor has a hitbox, just like a normal
//				character and done in depth hit detection on its own, unlike
//				the standard PowerArmor which relies on the user notifiying it
//				when it should turn off.
//              
//----------------------------------------------------------------------------
class OccludingArmor : public Prop, public IHitBoxUser
{
	public:
		OccludingArmor();
		virtual ~OccludingArmor();

		// Engine Interaction:
		virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual uint32  ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);
		virtual void Load(ILTMessage_Read *pMsg);

		// IHitBoxUser Interaction:
		virtual CAttachments* GetAttachments();
		virtual ModelSkeleton GetModelSkeleton() const;
		virtual void SetModelNodeLastHit(ModelNode eModelNode );
		virtual LTBOOL UsingHitDetection() const;
		virtual HATTACHMENT GetAttachment() const;
		virtual LTFLOAT ComputeDamageModifier(ModelNode eModelNode);
		virtual float GetNodeRadius( ModelSkeleton eModelSkeleton, ModelNode eModelNode );
		virtual void UpdateClientHitBox() {};
		// RETAIL: IHitBoxUser's last two: Tron's OccludingArmor has its own slot 0x24
		// (retail 0x1013c3e0, not yet written) and returns its object (0x1013c450)
		virtual HOBJECT TronUnknownVirtual28() { return m_hObject; }

	protected:
		enum eShieldState
		{
			eShieldNormal,
			eShieldAttack,
			eShieldBlock,
		};

		void	ReadShieldProps(ObjectCreateStruct *pData);

		void	Update();
		void	CreateHitBox();
		// RETAIL: virtual in Tron (OccludingArmor's slot 28 is empty)
		virtual void	HandleDeath() {}
		void	HandleModelString(ArgList* pArgList);

		void	SetState(eShieldState eNewState);
		void	SetAnimation(const char* const szAnimationName);

		LTBOOL	CanDamage() const;

	private:
		// Private members
		
		// Handle to the armors' hitbox object
		LTObjRef		m_hHitBox;			

		// Handle to the object the armor is attached to
		LTObjRef		m_hParentObject;

		// Handle to the currently playing animation
		HMODELANIM		m_hCurrentAnimation;

		// Animation specified 'state' for this shield.
		eShieldState	m_eShieldState;

		ModelId			m_eModelId;

		// Copy Constructor and Asignment Operator private to prevent 
		// automatic generation and inappropriate, unintentional use
		OccludingArmor(const PowerArmor& rhs) {}
		OccludingArmor& operator=(const PowerArmor& rhs ) {}
};

//----------------------------------------------------------------------------
//              
//	CLASS:		ShellArmor
//              
//	PURPOSE:	Special Armor which DOES take damage (see OccludingArmor).
//				RETAIL: Tron's (vtables 0x1029896c..., class def 0x102faa28)
//              
//----------------------------------------------------------------------------
class ShellArmor : public OccludingArmor
{
	public:
		ShellArmor() {}

	protected:
		virtual void	HandleDeath();
};

#endif // __POWERARMOR_H__

