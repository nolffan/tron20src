// ----------------------------------------------------------------------- //
//
// MODULE  : IONode.h
//
// PURPOSE : An I/O node: a player who has its ping and the permission
//			 sets it asks for engages it (it opens, runs its command and
//			 closes when the player walks away)
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x100222c0..0x10024c40; its class def at 0x102d97e0, vtable
// 0x10293c20). The names of the class, its properties, messages and model
// strings are retail's; the members, functions and states are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __IONODE_H__
#define __IONODE_H__

#include "GameBase.h"
#include "ActivateTypeMgr.h"
#include "AutoCommand.h"

class CPlayerObj;

// RETAIL: a user flag an I/O node sets on itself (InitialUpdate); not yet
// identified on the client. The name is ours
#define USRFLG_IONODE			(1<<1)

class IONode : public GameBase
{
	public :

		IONode();
		virtual ~IONode();

	protected :

		// RETAIL: one per virtual of slots 26..35, which Update calls in
		// turn (an "enter" sets the next state; an "update" waits)
		enum State
		{
			eStateEnterLocked = 0,
			eStateLocked,
			eStateEnterUnlocked,
			eStateUnlocked,
			eStateEnterOpen,
			eStateOpen,
			eStateEnterOpening,
			eStateOpening,
			eStateEnterClosing,
			eStateClosing,
		};

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual uint32	ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		// RETAIL: its own virtuals, slots 25..35 (vtable 0x10293c20)
		virtual void	Trigger();
		virtual void	EnterLocked();
		virtual void	UpdateLocked();
		virtual void	EnterUnlocked();
		virtual void	UpdateUnlocked();
		virtual void	EnterOpen();
		virtual void	UpdateOpen();
		virtual void	EnterOpening();
		virtual void	UpdateOpening();
		virtual void	EnterClosing();
		virtual void	UpdateClosing();

	private :

		LTBOOL	InitialUpdate();
		void	Update();
		LTBOOL	ReadProp(ObjectCreateStruct *pStruct);
		LTBOOL	HasPing(CPlayerObj* pPlayer);
		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		void	HandleModelString(ArgList* pArgList);
		void	PlayLoopSound();

		int							m_eState;
		std::string					m_sPingName;			// The ping a player needs
		HSTRING						m_hstrTriggerCommand;	// Run when engaged
		AutoCommand<MultiDispatch>	m_InvalidPSetCommand;	// Run when a player without the permission sets tries
		uint8						m_nPSets;				// The permission sets a player needs
		LTObjRef					m_hActivator;			// Who engaged us
		LTVector					m_vActivatorPos;		// Where they were when we opened
		bool						m_bFirstUpdate;
		bool						m_bMoveToFloor;			// RETAIL: never set (retail bug: MoveToFloor is not read)
		bool						m_bPinged;				// The player has our ping
		int32						m_nStringOverrideID;	// An info string for the client's target code
		CActivateTypeHandler		m_ActivateTypeHandler;
		HSTRING						m_hstrLoopSound;		// A looping sound from the model's keys
		HLTSOUND					m_hLoopSound;
};

#endif // __IONODE_H__
