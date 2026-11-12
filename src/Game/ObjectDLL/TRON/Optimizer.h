// ----------------------------------------------------------------------- //
//
// MODULE  : Optimizer.h
//
// PURPOSE : An optimizer: a model a player with its permission sets
//			 activates; it plays its optimize animation, pausing its
//			 keyframer, until told (MID_OPTIMIZER_DONE) whether to die or
//			 to come back to life
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x1002fd40..0x100325d6; its class def at 0x102da9a8, vtable
// 0x1029425c, size 0xb0). The names of the classes, the properties, the
// messages, animations and strings are retail's; the members, functions,
// states and message id names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __OPTIMIZER_H__
#define __OPTIMIZER_H__

#include "GameBase.h"
#include "Explosion.h"
#include "ActivateTypeMgr.h"
#include "AutoCommand.h"
#include "SharedFXStructs.h"

class Optimizer : public GameBase
{
	public :

		Optimizer();
		virtual ~Optimizer();

	protected :

		// RETAIL: one per virtual of slots 25..30, which Update calls in
		// turn (an "enter" sets the next state; an "update" waits)
		enum State
		{
			eStateEnterLive = 0,
			eStateLive,
			eStateEnterOptimize,
			eStateOptimize,
			eStateEnterDeath,
			eStateDeath,
		};

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual uint32	ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		// RETAIL: its own virtuals, slots 25..30 (vtable 0x1029425c)
		virtual void	EnterLive();
		virtual void	UpdateLive();
		virtual void	EnterOptimize();
		virtual void	UpdateOptimize();	// RETAIL: empty (folded, 0x1010c0c0)
		virtual void	EnterDeath();
		virtual void	UpdateDeath();

	private :

		LTBOOL		InitialUpdate();
		bool		FindKeyFramer();
		void		Update();
		LTBOOL		ReadProp(ObjectCreateStruct *pStruct);
		HMODELANIM	GetAnim(const char* szAnim);
		void		Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void		Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		void		HandleModelString(ArgList* pArgList);
		void		PlayLoopSound();
		bool		IsVisible();

		int								m_eState;
		std::string						m_sKeyFramer;			// The keyframer we pause while optimizing
		HOBJECT							m_hKeyFramer;
		uint8							m_nPSets;				// The permission sets a player needs
		HSTRING							m_hstrLoopSound;		// A looping sound from the model's keys
		HLTSOUND						m_hLoopSound;
		AutoCommand<MultiDispatch>		m_InvalidPSetCommand;	// Run when a player without the permission sets tries
		AutoCommand<MultiDispatch>		m_ActivateCommand;		// Run when a player activates us
		AutoCommand<SingleDispatch>		m_OptimizeCommand;		// Run (once) when we are told to die
		CLIENTFXUSERCREATESTRUCT		m_cs;					// Our special fx message's data
		CActivateTypeHandler			m_ActivateTypeHandler;
};

// RETAIL: an Explosion with nothing of its own (its vtable 0x10294154 is
// Explosion's but for the destructor; class def 0x102da9d8, registered in
// this module)
class OptimizerExplosion : public Explosion
{
};

#endif // __OPTIMIZER_H__
