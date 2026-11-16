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

		//used to determine the least recently used startpoint
		float		GetLastUse()		const { return m_fLastUse;}
		void		SetLastUse(float fLastUse) { m_fLastUse = fLastUse;}

		uint8		GetTeamID()			const { return m_nTeamID; }

		bool		IsStartPoint()		const { return m_bStartPoint; }
		bool		IsSpawnPoint()		const { return m_bSpawnPoint; }

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

		// PORT: NOLF2's, which Tron's GameStartPoint has not (retail's ends
		// at +0x60): kept after Tron's members for the NOLF2 code still
		// standing in
		float		m_fLastUse;
		
		uint8		m_nTeamID;		// When in a team game this specifies which team can spawn at this start point.
		
		bool		m_bStartPoint;	// Can this start point be used as an initial start point?
		bool		m_bSpawnPoint;	// Can this start point be used as a respawn point?
};


class GameStartPointLesser
{
public:
	
	bool operator()(const GameStartPoint* x, const GameStartPoint* y) const
	{
		return (x->GetLastUse() < y->GetLastUse() );
	}
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