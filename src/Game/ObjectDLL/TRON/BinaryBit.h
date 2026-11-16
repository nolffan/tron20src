/****************************************************************************
;
;	MODULE:			BinaryBit.h
;
;	PURPOSE:		Binary Bit class for TRON
;
;	HISTORY:		2/14/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/

#ifndef _BINARYBIT_H_
#define _BINARYBIT_H_

#include "GameBase.h"
#include "CommandMgr.h"
#include "ActivateTypeMgr.h"
#include "AutoCommand.h"

// RETAIL: the user flag InitialUpdate sets (0x10014610); the same bit as
// USRFLG_IONODE (IONode.h). The name is ours
#define USRFLG_BINARYBIT		(1<<1)

enum BinaryBitState
{
	BBState_PoweredDown = 0,
	BBState_GoingToSocket,
	BBState_On,
	BBState_Off
};

class BinaryBit : public GameBase
{
	public :

		BinaryBit();
		virtual ~BinaryBit();

	protected :

        virtual	uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual	uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

	private :

        void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		
        LTBOOL	ReadProp(ObjectCreateStruct *pStruct);
		LTBOOL	InitialUpdate();
		LTBOOL	HandleModelString(ArgList* pArgList);

		LTBOOL	ChangeState(BinaryBitState eNewState);
		void	TurnOn();
		void	TurnOff();
		void	HandleGoingToSocket();
		void	HandleArriveAtSocket();
		void	HandleActivateMessage(HOBJECT hSender);
		void	DoPowerUp();

		// Member variablez, yo
		// RETAIL: the members at retail's offsets (ReadProp 0x10013960, Save
		// 0x10013e00): Tron's BinaryBit has the activate type, the invalid
		// permission set command, a string override and one more flag that
		// Build 69's has not. Their code is not yet written
		CActivateTypeHandler	m_ActivateTypeHandler;		// +0x38 "ActivateType"
		LTBOOL			m_bLocked;					// Is the binary bit locked
		LTBOOL			m_bStartOnInSocket;			// Are we on or off when we first reach the socket?
		BinaryBitState	m_eState;					// What's the state
		int				m_nOriginalEnergyRequired;	// How much is originally required to power us
		int				m_nCurrentEnergyRequired;	// How much is currently required
		LTBOOL			m_bPlayingTransitionAnim;	// Are we playing a short on/off animation?

		// Commands
		HSTRING			m_hstrPowerUpCommand;
		HSTRING			m_hstrTurnOnCommand;
		HSTRING			m_hstrTurnOffCommand;		// +0x70

		AutoCommand<MultiDispatch>	m_InvalidPSetCommand;	// +0x74 "InvalidPSetCommand"

		uint8			m_byPSets;					// +0x84

		HOBJECT			m_hFX;						// +0x88

		int32			m_nStringOverrideID;		// +0x8c "StringOverrideID"
		bool			m_bTronUnknown90;			// +0x90 saved last but one
};

class CBinaryBitPlugin : public IObjectPlugin
{
	public :

		virtual LTRESULT PreHook_PropChanged(
				const char *szObjName,
				const char *szPropName,
				const int nPropType,
				const GenericProp &gpPropValue,
				ILTPreInterface *pInterface,
				const char *szModifiers ); // PORT: NOLF2's interface

	protected :

		CCommandMgrPlugin m_CommandMgrPlugin;
};

#endif // _BINARYBIT_H_