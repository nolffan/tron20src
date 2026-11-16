// ----------------------------------------------------------------------- //
//
// MODULE  : ServerMissionMgr.h
//
// PURPOSE : Definition of class to handle managment of missions and worlds.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SERVERMISSIONMGR_H__
#define __SERVERMISSIONMGR_H__

#include "ClientServerShared.h"
#include "PlayerTracker.h"

typedef std::vector< int > Campaign;

class CServerMissionMgr : public IPlayerTrackerReceiver
{
	public:

		CServerMissionMgr();
		virtual ~CServerMissionMgr();

		// Called when level switch is needed.
		// RETAIL: virtual in Tron (slot 2, retail 0x10161f70)
		virtual bool	ExitLevelSwitch( );

		// Called to force switch to next mission
		bool			NextMission( );
		
		// Called to force switch to specific mission within a campaign.
		// RETAIL: Tron has no NextRound or out-of-line SwitchToCampaignIndex
		// (no function between NextMission 0x10162030 and ExitLevelToLevel
		// 0x101620d0); NOLF2's body is kept inline for the server commands.
		bool			SwitchToCampaignIndex( int nCampaignIndex )
		{
			m_nCurCampaignIndex = nCampaignIndex - 1;
			return NextMission( );
		}

		// Called when level switch is needed.
		bool			ExitLevelToLevel( char const* pszNewLevel );

		// Called when level transition is needed.
		bool			ExitLevelTransition( int nNewLevel );

		// Handles messages.
		bool			OnMessage( HCLIENT hSender, ILTMessage_Read& msg );

		// track mission failure states
		void			SetMissionFailed(bool bFail);
		bool			MissionFailed() { return m_bMissionFailed; }

		// Handle when we have to abort a playertracker.
		virtual void	OnPlayerTrackerAbort( );

		LTBOOL			IsCustomLevel()   const {return m_bCustomLevel;}
		int				GetCurrentMission() const {return m_nCurrentMission;}
		int				GetCurrentLevel()	const {return m_nCurrentLevel;}
		int				GetCurrentCampaignIndex( ) const { return m_nCurCampaignIndex; }

		char const*		GetLevelFromMission( int nMission, int nLevel );
		bool			SetMissionBasedOnLevel( char const* pszFilename );
		int				FindNextCampaignIndex( int nStartingCampaignIndex, int nMissionIndex );

		// RETAIL: Tron keeps the server settings as loose members (+0x1d..
		// +0x20, Save 0x10161ba0) and has no ServerMissionSettings struct.
		bool			UseSkills( ) const { return m_bUseSkills; }
		bool			FriendlyFire( ) const { return m_bFriendlyFire; }
		uint8			GetMPDifficulty( ) const { return m_nMPDifficulty; }
		float			GetPlayerDiffFactor( ) const { return m_fPlayerDiffFactor; }

		Campaign&		GetCampaign( ) { return m_Campaign; }

	// Game type overrides.
	public:

		// Initializes the object.  Overrides should call up.
		virtual bool	Init( );

		// Terminates the object.  Overrides should call up.
		virtual void	Term();

		// Save/load state of missionmgr.  Overrides should call up.
		virtual bool	Save( ILTMessage_Write& msg, uint32 dwSaveFlags );
		virtual bool	Load( ILTMessage_Read& msg, uint32 dwSaveFlags );

		// Called every frame.  Overrides should call up.
		virtual void	Update( );

		// Called to check if gametype allows respawning.
		// RETAIL: CanRespawn and LevelStarted are not virtual in Tron (its
		// vtable; build 51 has neither)
		bool	CanRespawn( CPlayerObj const& player ) const { return true; }

		// Called when the clients enter the world
		// RETAIL: Tron has no out-of-line LevelStarted (no start time member)
		void	LevelStarted() {}

	// Game type overrides.
	protected:

		// Does loading of next level.
		virtual bool	FinishExitLevel( );

		// Init game.
		virtual bool	StartGame( ILTMessage_Read& msg );

		// Called when the game has finished the last mission.
		virtual bool	EndGame( );

	protected:

		// Handle start of game.
		bool			HandleStartGame( HCLIENT hSender, ILTMessage_Read& msg );

		// Handle start level
		bool			HandleStartLevel( HCLIENT hSender, ILTMessage_Read& msg );

		// Handle client's exitlevel.
		bool			HandleExitLevel( HCLIENT hSender, ILTMessage_Read& msg );


		// Call to send client exit level message with current level and mission.
		// RETAIL: virtual in Tron (its last slot, retail 0x10162350)
		virtual bool	SendExitLevelMessage( );

		// Sets up the campaign to run.  The campaign is the list of missions in
		// the order they should be played.
		bool			SetupCampaign( char const* pszCampaignName );

		// Called by exitlevelXXX routines to go to a new level.
		bool			ExitLevel( char const* pszNewLevel, bool bTransitionToLevel );

	// RETAIL: Tron's layout (ctor 0x10161890, Save 0x10161ba0, Load
	// 0x10161c70): size 0x48. NOLF2's round counter, start time, display
	// timer and settings struct are absent.
	protected:

		// +0x4: saved and loaded first; MID_START_GAME sends it first (the
		// client sends the profile's byte at +0x73).  Name is ours.
		bool			m_bTronUnknown4;

		// Campaign file specified at MID_STARTGAME.
		CString			m_sCampaignFile;		// +0x8

		// List of missions to play.
		Campaign		m_Campaign;				// +0xc
		int				m_nCurCampaignIndex;	// +0x18

		// Loop when we reach the last level of the last mission.
		bool			m_bLoopCampaign;		// +0x1c

		bool			m_bUseSkills;			// +0x1d
		bool			m_bFriendlyFire;		// +0x1e
		uint8			m_nMPDifficulty;		// +0x1f
		float			m_fPlayerDiffFactor;	// +0x20

		// has the player (or players) failed the mission
		bool			m_bMissionFailed;		// +0x24

	// Unsaved data.
	protected:

		// Used when we are tracking players we sent messages to.
		PlayerTracker	m_PlayerTracker;		// +0x28

		// Controls whether we switch levels or transition levels.
		bool			m_bTransitionLevels;	// +0x34

		// Set when we're switching missions.
		bool			m_bNewMission;			// +0x35

		// Set while we're waiting for exitlevel back from clients.
		bool			m_bExitingLevel;		// +0x36

		// Set in OnPlayerTrackerAbort.  Update polls for it.  Tells us to continue with save.
		bool			m_bPlayerTrackerAborted;	// +0x37

		bool			m_bCustomLevel;			// +0x38
		int				m_nCurrentMission;		// +0x3c
		int				m_nCurrentLevel;		// +0x40
		CString			m_sCurrentWorldName;	// +0x44

};

extern CServerMissionMgr* g_pServerMissionMgr;

#endif // __SERVERMISSIONMGR_H__