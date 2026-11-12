// ----------------------------------------------------------------------- //
//
// MODULE  : Optimizer.h
//
// PURPOSE : An optimizer: a model a player engages (with the permission
//			 sets it asks for) to optimize his subroutines
//
// RETAIL: Tron 2.0's, not in any source release (retail Object.lto: class
// def 0x102da9a8, size 0xb0; vtable 0x1029425c). The class names are
// retail's; the members' and functions' are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __OPTIMIZER_H__
#define __OPTIMIZER_H__

#include "GameBase.h"
#include "Explosion.h"
#include "ActivateTypeMgr.h"
#include "AutoCommand.h"
#include "SharedFXStructs.h"
#include "ModelButeMgr.h"

// RETAIL: an explosion of its own class (class def 0x102da9d8, size 0x15c);
// nothing of Explosion's is overridden
class OptimizerExplosion : public Explosion
{
};

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
			eStateEnterOptimizing,
			eStateOptimizing,
			eStateEnterDeath,
			eStateDeath,
		};

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual uint32	ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		// RETAIL: its own virtuals, slots 25..30 (vtable 0x1029425c)
		virtual void	EnterLive();
		virtual void	UpdateLive();
		virtual void	EnterOptimizing();
		virtual void	UpdateOptimizing() {}
		virtual void	EnterDeath();
		virtual void	UpdateDeath();

	private :

		LTBOOL			InitialUpdate();
		void			Update();
		bool			AllObjectsCreated();
		LTBOOL			ReadProp(ObjectCreateStruct *pStruct);
		HMODELANIM		GetAnim(const char* szAnim);
		void			Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void			Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		void			HandleModelString(ArgList* pArgList);
		void			PlayLoopSound();
		bool			IsVisible();

		int								m_eState;
		std::string						m_sKeyFramer;			// The keyframer we pause while optimizing
		HOBJECT							m_hKeyFramer;
		uint8							m_nPSets;				// The permission sets a player needs
		HSTRING							m_hstrLoopSound;		// A looping sound from the model's keys
		HLTSOUND						m_hLoopSound;
		AutoCommand<MultiDispatch>		m_InvalidPSetCommand;	// Run when a player without the permission sets tries
		AutoCommand<MultiDispatch>		m_ActivateCommand;		// Run when a player engages us
		AutoCommand<SingleDispatch>		m_OptimizeCommand;		// Run the first time a player optimizes
		CLIENTFXUSERCREATESTRUCT		m_cs;					// Our special fx message: our model
		CActivateTypeHandler			m_ActivateTypeHandler;
};

#endif // __OPTIMIZER_H__
