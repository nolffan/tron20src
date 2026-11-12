// ----------------------------------------------------------------------- //
//
// MODULE  : Thorne.h
//
// PURPOSE : Thorne, the boss: a model that summons, creates, charges and
//           throws an energy ball at its target
//
// RETAIL: Tron 2.0's, written from retail Object.lto (no source exists).
//         Names of members and methods are ours unless a string says so.
//
// ----------------------------------------------------------------------- //

#ifndef __THORNE_H__
#define __THORNE_H__

#include "GameBase.h"
#include "IHitBoxUser.h"
#include "Weapons.h"
#include "PowerArmor.h"
#include "SFXMsgIds.h"
#include "SharedFXStructs.h"
#include "ModelButeMgr.h"
#include <list>
#include <string>
#include "CaselessString.h"

// RETAIL: the list Thorne's IHitBoxUser slot 9 adds its ball to (retail
// 0x10057ba0, and the list's node allocation 0x1005a0e0)
typedef std::list<IHitBoxUser*> HitBoxUserList;

// ----------------------------------------------------------------------- //
//
//	CLASS:		ThorneBall
//
//	PURPOSE:	The ball Thorne throws.
//
//	RETAIL: ThorneBall : OccludingArmor (Prop, IHitBoxUser), 0x870 bytes, its
//	class registered in Thorne's TU (ClassDef 0x102dda50), its vtables
//	(0x10294ca8...) with OccludingArmor's code (retail 0x1013be80...).
//	PORT: OccludingArmor is not yet written; ShellArmor, its sibling of the
//	same shape, stands in.
//
// ----------------------------------------------------------------------- //

class ThorneBall : public ShellArmor
{
	public :

		// Ours: let Thorne set the ball's state
		void	SetBallState(int nState) { SetState((eShieldState)nState); }
};

// ----------------------------------------------------------------------- //
//
//	CLASS:		CThorne
//
// ----------------------------------------------------------------------- //

class CThorne : public GameBase, public IHitBoxUser
{
	public :

		CThorne();
		~CThorne();

		// IHitBoxUser (RETAIL: vtable 0x10294d20)
		virtual CAttachments*	GetAttachments() { return LTNULL; }
		virtual ModelSkeleton	GetModelSkeleton() const { return m_eModelSkeleton; }
		virtual void			SetModelNodeLastHit(ModelNode eModelNode) {}
		virtual LTBOOL			UsingHitDetection() const { return LTTRUE; }
		virtual HATTACHMENT		GetAttachment() const { return LTNULL; }
		virtual LTFLOAT			ComputeDamageModifier(ModelNode eModelNode) { return 1.0f; }
		virtual float			GetNodeRadius(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		virtual void			UpdateClientHitBox() {}
		// RETAIL: slot 8 returns Thorne's hit box object (retail 0x10095c70)
		virtual CAnimator*		GetAnimator() { return (CAnimator*)(HOBJECT)m_hHitBox; }
		// RETAIL: slot 9 adds the ball to a HitBoxUserList (retail 0x10057ba0)
		virtual void			TronUnknownVirtual24(int nArg);
		virtual HOBJECT			TronUnknownVirtual28() { return m_hObject; }

	protected :

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);
		void	ReadProp(ObjectCreateStruct *pStruct);
		void	Update();
		void	HandleModelString(HOBJECT hSender, const CParsedMsg &cMsg);
		void	InitialUpdate(LTFLOAT fInfo);
		void	AllObjectsCreated();
		void	CreateHitBox();
		void	CreateBall();
		LTVector GetFirePos();
		void	SetState(int eState);
		void	ResetAttachFX();
		void	UpdateState();
		void	Fire();
		void	PlayStateAnim();

		// Time since the charge began, or until it ends; a set m_fChargePauseTime
		// stands for now
		LTFLOAT	GetChargeTime() { return m_fChargePauseTime > 0.0f ? m_fChargePauseTime : g_pLTServer->GetTime(); }

		int				m_eState;				// ThorneAnim (BossButeMgr.h)
		LTFLOAT			m_fChargeStartTime;
		LTFLOAT			m_fChargeDuration;
		LTFLOAT			m_fChargePauseTime;
		bool			m_bCharging;
		ModelSkeleton	m_eModelSkeleton;
		LTObjRef		m_hHitBox;
		CaselessString	m_sSummonCmd;
		bool			m_bAnimDone;
		CaselessString	m_sTargetObject;
		LTObjRef		m_hTargetObject;
		LTObjRef		m_hBall;
		bool			m_bAllowSummon;
		CWeapons		m_Weapons;
		CaselessString	m_sWeapon;				// RETAIL: saved, never set
		uint8			m_nWeaponId;
		CaselessString	m_sAmmo;				// RETAIL: saved, never set
		uint8			m_nAmmoId;
		CLIENTFXUSERCREATESTRUCT m_cs;		// RETAIL: holds the model id too
};

class ThorneObjectPlugin : public IObjectPlugin
{
	public :

		ThorneObjectPlugin();

		virtual ~ThorneObjectPlugin()
		{
			if (m_pModelButeMgrPlugin)
			{
				debug_delete(m_pModelButeMgrPlugin);
				m_pModelButeMgrPlugin = LTNULL;
			}
		}

		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);

	private :

		CModelButeMgrPlugin*	m_pModelButeMgrPlugin;
};

#endif // __THORNE_H__
