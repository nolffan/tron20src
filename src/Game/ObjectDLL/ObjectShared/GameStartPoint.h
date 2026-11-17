// ----------------------------------------------------------------------- //
//
// MODULE  : GameStartPoint.h
//
// PURPOSE : GameStartPoint - Definition
//
// CREATED : 9/30/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_START_POINT_H__
#define __GAME_START_POINT_H__

#include "GameBase.h"
#include "NetDefs.h"
#include "ModelButeMgr.h"
#include "SharedMovement.h"
#include "CommandMgr.h"

LINKTO_MODULE( GameStartPoint );


class GameStartPoint : public GameBase
{
	public :

		GameStartPoint();
		~GameStartPoint();

		HSTRING		GetName()			const { return m_hstrName; }
        LTVector    GetPitchYawRoll()	const { return m_vPitchYawRoll; }

		ModelId		GetPlayerModelId()	const { return m_eModelId;}

		HSTRING		GetCommand()		const { return m_hstrCommand; }

        PlayerPhysicsModel  GetPlayerPhysicsModel() const { return m_ePPhysicsModel; }

		bool		IsLocked()			const { return m_bLocked; }
		bool		SendCommandOnRespawn() const { return m_bSendCommandOnRespawn; }

		typedef std::vector< GameStartPoint* > StartPointList;
		static StartPointList const& GetStartPointList() { return m_lstStartPoints; }

		// RETAIL: Tron's (FindStartPoint, Object.lto 0x100e0f18)
		GameType	GetGameType()		const { return m_eGameType; }

		// RETAIL: Tron's: a player respawned at it (retail CPlayerObj::Respawn
		// sets it, 0x1007dc92; the server shell's ResetStartPoints, 0x100e46f0,
		// and the light cycle game clear it). The names are ours
		bool		IsInUse()			const { return m_bInUse; }
		void		SetInUse(bool bInUse)	{ m_bInUse = bInUse; }

	protected :

        uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		bool	OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg );
		
	private :

		// RETAIL: Tron's members at retail's offsets (its constructor,
		// 0x1014f990, and destructor, 0x1014fa00; Build 51's order)
		GameType	m_eGameType;			// +0x38 Single/Multiplayer modes
		ModelId		m_eModelId;				// +0x3c Template model
        HSTRING     m_hstrName;             // +0x40 Name of start point
        LTVector    m_vPitchYawRoll;        // +0x44 Pitch, yaw, and roll of start point
		bool		m_bLocked;				// +0x50 If locked the player should not use this start point.
		bool		m_bSendCommandOnRespawn;// +0x51 Should the command be sent every respawn?

		PlayerPhysicsModel	m_ePPhysicsModel;	// +0x54 Starting player physics model

		HSTRING		m_hstrCommand;	// +0x58 Command to process

		// RETAIL: Tron's: a player respawned at it (+0x5c)
		bool		m_bInUse;

        static uint32 m_dwCounter;  // Counts gamestartpoints created for the level.

        LTBOOL ReadProp(ObjectCreateStruct *pStruct);

		static StartPointList m_lstStartPoints;
};


class CGameStartPointPlugin : public IObjectPlugin
{
  public:

    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);

	virtual LTRESULT PreHook_PropChanged( 
		const	char		*szObjName,
		const	char		*szPropName,
		const	int			nPropType,
		const	GenericProp	&gpPropValue,
				ILTPreInterface	*pInterface,
		const	char		*szModifiers );

  protected :

        CModelButeMgrPlugin m_ModelButeMgrPlugin;
		CCommandMgrPlugin	m_CommandMgrPlugin;
};


#endif // __GAME_START_POINT_H__