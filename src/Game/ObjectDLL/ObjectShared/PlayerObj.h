// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerObj.h
//
// PURPOSE : Player object definition
//
// CREATED : 9/18/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CPLAYER_OBJ_H__
#define __CPLAYER_OBJ_H__

#include "Character.h"
#include "MsgIds.h"
#include "CheatDefs.h"
#include "TemplateList.h"
#include "CVarTrack.h"
#include "ClientServerShared.h"
#include "Skills.h"
#include "AnimatorPlayer.h"
#include "PlayerShared.h"
#include "ActivationData.h"
#include "SharedMission.h"
#include "SharedScoring.h"
#include "IDList.h"
#include "NetDefs.h"

LINKTO_MODULE( PlayerObj );


class CPlayerAttachments;
class CWeapon;
class PlayerVehicle;
class TeleportPoint;
class PlayerLure;
class GameStartPoint;
class CAI;
struct WeaponFireInfo;
enum  EnumAIStimulusID;


// RETAIL: one of Tron's player subroutines (0x43 bytes: retail CPlayerObj's
// constructor clears the first byte of each). The names are ours
#define PLAYER_MAX_SUBROUTINES		12
#define PLAYER_SUBROUTINE_NAME_LEN	64

struct PlayerSubroutineSlot
{
	PlayerSubroutineSlot() { szName[0] = '\0'; }

	char	szName[PLAYER_SUBROUTINE_NAME_LEN];
	uint8	nTronUnknown40;		// RETAIL: tested before 0x41 is used (0x1000f616)
	uint8	nTronUnknown41;
	uint8	nTronUnknown42;
};

struct ObjectiveMsgInfo
{
	ObjectiveMsgInfo()
	:	m_nType		( 0 ),
		m_nRequest	( 0 ),
		m_dwId		( 0 )
	{
	}

	uint8	m_nType;
	uint8	m_nRequest;
	uint32	m_dwId;
};


class CPlayerObj : public CCharacter
{
	public :

		CPlayerObj();
		virtual ~CPlayerObj();

		// Attachments

		virtual void CreateAttachments();
		
		virtual void AddToObjectList( ObjectList *pObjList, eObjListControl eControl = eObjListNODuplicates );

		void SendStartLevelCommand();
		void SetClient(HCLIENT hClient);
		HCLIENT GetClient() const		{ return m_hClient; }

		void SetClientSaveData(ILTMessage_Read *pMsg);
		void BuildKeepAlives(ObjectList* pList);

		// If the update had CLIENTUPDATE_ALLOWINPUT, then it returns FALSE because nothing
		// else will be in the packet.
        LTBOOL ClientUpdate(ILTMessage_Read *pMsg);

        LTBOOL MultiplayerInit(ILTMessage_Read *pMsg);
        LTBOOL MultiplayerUpdate(ILTMessage_Read *pMsg);

		// Check if player is allowed to respawn.
		bool CanRespawn( );

		// Check if player is allowed to be revived.
		bool CanRevive( );

		// RETAIL: virtual in Tron (CTronPlayerObj overrides it, retail
		// 0x10067210), with two more arguments: an object to spawn at and
		// another whose position it takes (retail 0x1007da50 reads each one's
		// m_hObject). Their names and types are ours
        virtual void Respawn(uint8 nServerLoadGameFlags=LOAD_NEW_LEVEL, bool bGetStartPt=true,
			GameStartPoint* pStartPoint=LTNULL, GameBase* pSpawnAt=LTNULL);
		void Revive();
        void CancelRevive( );
		void ResetAfterDeath();
		void ResetMultiplayerSkills();
        void SetSpectatorMode(LTBOOL bOn);
        void SetInvisibleMode(LTBOOL bOn);
		void ToggleGodMode();
		// RETAIL: Tron's zones ask it (EnergyZone, HealthZone). The name is ours
		LTBOOL InGodMode() const { return m_bGodMode; }

		// Makes player invulnerable for a period of time.
		void SetRespawnInvulnerability( );

		// Called when client is loaded.
		void SetClientLoaded( bool bClientLoaded );
		bool IsClientLoaded( ) { return m_bClientLoaded; }

		void FullAmmoCheat();
		virtual void FullWeaponCheat();	// RETAIL: virtual in Tron
		void FullModsCheat();
		void FullGearCheat();
		void RepairArmorCheat();
		void HealCheat();
		virtual void GimmeGunCheat( uint8 nId );	// RETAIL: virtual in Tron
		// RETAIL: an empty virtual of Tron's (CTronPlayerObj's sets or clears
		// all permission sets, retail 0x10067120: a cheat)
		virtual void TronUnknownVirtual105( LTBOOL /*bOn*/ ) {}
		void GimmeModCheat( uint8 nId );
		void GimmeGearCheat( uint8 nId );
		void GimmeAmmoCheat( uint8 nId );
		void SkillsCheat();

		// Call to setup the player for new mission.
		bool SetupForNewMission( );

		virtual CAttachments*	TransferAttachments( bool bRemove ); // Responsibility of call to delete attachments
		virtual void			TransferWeapons(Body* pBody, bool bRemove);
		virtual	void			RemoveAttachments( bool bDestroyAttachments );


		void RideVehicle(PlayerVehicle* pVehicle);

		// Tell the player to follow a lure.
		bool FollowLure( PlayerLure& playerLure );
		bool StopFollowingLure( );

		virtual void PushCharacter(const LTVector &vPos, LTFLOAT fRadius, LTFLOAT fStartDelay, LTFLOAT fDuration, LTFLOAT fStrength);

        LTBOOL HasDangerousWeapon();
        LTBOOL HasMeleeWeapon();

        void ChangeWeapon(uint8 nCommandId, LTBOOL bAuto=LTFALSE, int32 nAmmoId=-1);

		void SetControlFlags(uint32 nFlags) { m_nControlFlags = nFlags; }
		uint32 GetControlFlags() const { return m_nControlFlags; }

		void ChangeState(PlayerState eNewState);
		PlayerState GetState() const { return m_eState; }

		void HandlePreExit();
		void HandleExit( bool bNewMission );
		virtual void  HandleDamage(const DamageStruct& damage);
		// RETAIL: two empty virtuals of Tron's taking a client message (the
		// server shell hands them its player's messages, retail 0x100def81 and
		// 0x100defb1; CTronPlayerObj's set and clear its flag at 0x325c)
		virtual void TronUnknownVirtual107( ILTMessage_Read* /*pMsg*/ ) {}
		virtual void TronUnknownVirtual108( ILTMessage_Read* /*pMsg*/ ) {}
		void HandleActivateMessage(ILTMessage_Read *pMsg);
		void HandlePlayerPositionMessage(ILTMessage_Read *pMsg);
		void HandleWeaponFireMessage(ILTMessage_Read *pMsg);
		void HandleWeaponSoundMessage(ILTMessage_Read *pMsg);
		void HandleWeaponSoundLoopMessage(ILTMessage_Read *pMsg);
		void HandleClientMsg(ILTMessage_Read *pMsg);
		void HandleTeleportMsg(ILTMessage_Read *pMsg);
        bool HandleObjectiveMessage(const CParsedMsg &cMsg);
        bool HandleKeyMessage(const CParsedMsg &cMsg);
        bool HandleTransmissionMessage(const CParsedMsg &cMsg);
        bool HandleOverlayMessage(const CParsedMsg &cMsg);
        bool HandleCreditsMessage(const CParsedMsg &cMsg);
        bool HandleIntelMessage(const CParsedMsg &cMsg);
        bool HandleFadeScreenMessage(const CParsedMsg &cMsg, bool bFadeIn);
        bool HandleMissionTextMessage(const CParsedMsg &cMsg);
        bool HandleMissionFailedMessage(const CParsedMsg &cMsg);
        bool HandleMusicMessage(const CParsedMsg &cMsg);
		bool HandleRemoveAllBadAIMessage( );

		void SetNetClientData( NetClientData const& NetClientData );
		char const* GetNetUniqueName( );

		void	SetChatting(LTBOOL bChatting);

		virtual LTBOOL DoDialogueSubtitles() { return LTTRUE; }

		//used only for ending co-op levels
		LTBOOL  IsReadyToExit() {return m_bReadyToExit;}
		void	ReadyToExit(LTBOOL bReady) {m_bReadyToExit = bReady;}

		void	ForceDuck(LTBOOL bForce) {m_bForceDuck = bForce;}
		LTBOOL	IsForcedToDuck() const { return m_bForceDuck; }

		int   GetRespawnCount() { return(m_nRespawnCount); }
		void  SetRespawnCount(int nRespawns) { m_nRespawnCount = nRespawns; }
		void  IncRespawnCount() { m_nRespawnCount++; }
		LTBOOL  RespawnCalled() const { return m_bRespawnCalled; }

        void    DoWeaponChange(uint8 nWeaponId,uint8 nAmmoId);
		void	DoWeaponReload(uint8 nWeaponId);

		CSkills* GetPlayerSkills() { return &m_Skills; }
		void	 HandleSkillUpdate(ILTMessage_Read *pMsg);

		MissionStats* GetMissionStats() { return &m_Stats; }
		CPlayerScore* GetPlayerScore() { return &m_Score; }

		void TeleportClientToServerPos();

        virtual LTFLOAT ComputeDamageModifier(ModelNode eModelNode);

		void SendIDToClients();

		// Hiding

		LTBOOL IsHidden() const { return m_bHidden; }
		void ClearHiding();
		
		void SetVisibleToEnemyAI( CAI* pAI, bool bVis );

		// Carrying...

		void	SetCarriedObject(HOBJECT hBody);	// RETAIL: no transition argument in Tron
		HOBJECT	GetCarriedObject() {return m_hCarriedObject;}
		bool	IsCarrying() {return (GetCarriedObject() != LTNULL);}
		bool	IsCarryingHeavyObject();
		void	RemoveCarriedObject();
		bool	DropCarriedObject( );	// RETAIL: no transition argument in Tron
		// PORT: for NOLF2's callers that pass one
		bool	DropCarriedObject( bool /*bTransition*/ ) { return DropCarriedObject( ); }


		virtual void	HideCharacter(LTBOOL bHide);

		// Handshake status of the player
		LTBOOL HasDoneHandshake() { return m_bHasDoneHandshake; }
		void FinishHandshake() { m_bHasDoneHandshake = LTTRUE; }

		void ResetObjectRelationMgr(const char* const pszNewRelation=NULL);
		
		// damage filtering
		virtual bool FilterDamage( DamageStruct *pDamageStruct );

		// Get the complete list of all PlayerObj's.
		typedef std::vector< CPlayerObj* > PlayerObjList;
		static PlayerObjList const& GetPlayerObjList( ) { return m_lstPlayerObjs; }
		static uint32 GetNumberPlayersWithClients( );
		
		
		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );
		
		PlayerPhysicsModel	GetPlayerPhysicsModel() const { return m_ePPhysicsModel; }

		// PORT: Tron's CPlayerObj has no used start point member (its layout
		// ends at 0x31e8); these stand in for NOLF2's callers
		void	SetUsedStartPoint( bool /*bUsed*/ ) { }
		bool	UsedStartPoint( ) const { return false; }

		// Creates body after respawn.
		void	CreateRespawnBody( );

		// Setup our team model.
		bool	SetupTeamModel( bool bResetModel );

		// The teamid the player is on.
		uint8	GetTeamID( ) const;
		// PORT: no team change requests in Tron (no member for one)
		bool	RequestedTeamChange() {return false; }

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);
        virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

        void    WeaponCheat(uint8 nWeaponId);

		void	StartDeath();
        void    HandleDead(LTBOOL bRemoveObj);
		void	InitializeWeapons(CWeapons* pWeapons);
		char*	GetDamageSound(DamageType eType);
		char*	GetDeathSound();

		LTFLOAT	GetFootstepVolume();

		virtual void ProcessDamageMsg(ILTMessage_Read *pMsg);

        virtual LTVector GetHeadOffset() { return GetPlayerHeadOffset( ); }

        bool	ProcessCommandLure(const CParsedMsg &cMsg);

		
		virtual void PreCreateSpecialFX(CHARCREATESTRUCT& cs);

		// [kml] Since we have a derived class, these need to be virtual
        virtual void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        virtual void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
        virtual LTBOOL   Update();
		virtual void UpdateInterface( bool bForceUpdate ); // Send client interface info.

		virtual void GetExtraFireMessageInfo( uint8, ILTMessage_Read*, WeaponFireInfo* ) {}
		// RETAIL: Tron's: reads more of the fire message into what follows
		// GetExtraFireMessageInfo's (retail 0x10080140, called right after it:
		// eleven pairs of flags at +0x68, then the data each flags at +0x80
		// on; it does not use this). PORT: not yet written, empty here
		virtual void TronUnknownVirtual114( ILTMessage_Read* /*pMsg*/, WeaponFireInfo* /*pInfo*/ ) {}
		// RETAIL: an empty virtual of Tron's with one argument (retail 0x1026abb0)
		virtual void TronUnknownVirtual115( uint32 /*nUnknown*/ ) {}



		void	PostPropRead(ObjectCreateStruct *pStruct);
        virtual LTBOOL   InitialUpdate(int nInfo);	// RETAIL: virtual in Tron

		void	CreatePVWeapon();

		virtual void UpdateCommands();		// Update our animator based on client commands
		virtual void UpdateMovement();		// Update player movement
		virtual void UpdateVehicleMovement(); // Update movement while on a vehicle.
		virtual bool UpdateDamageAnimations(); // Update our animatior based on any damage we are taking

        virtual HMODELANIM GetDeathAni(LTBOOL bFront);

		virtual HMODELANIM GetCrouchDeathAni(LTBOOL bFront);

        void UpdateInContainerState(HOBJECT* objContainers, uint32 & nContainers);

		void UpdateClientPhysics();		// Update client physics state.
		void UpdateAirLevel();			// Update player's air
		void UpdateClientEffects();		// Update client side effects

		void UpdateHealth();	// Update player's health (regeneration). RETAIL: not virtual in Tron

		void UpdateHiding();			// Update player's hiding state
		void StartHiding();				// player is starting to hide
		void StartHidden();				// player is now hidden
		void EndHidden();				// player is no longer hidden, goes back to starting to hide.
		LTBOOL CanHide();				// Can the player hide right now?

		void BecomeVisible();			// Sets the visibility based stimuli to on
		void BecomeInvisible();			// Sets the visibility based stimuli to off

        void ResetPlayer( ); // Reset the player
		void ResetHealth(LTBOOL bMaxOnly = LTFALSE, float fHealthPercent = 1.0f, float fArmorPercent = 1.0f);
		void ResetInventory(LTBOOL bRemoveGear=LTTRUE);
		void ResetModel();

		void TweakCameraOffset();

        LTBOOL PlayingCinematic(LTBOOL bStopCinematic=LTFALSE);

		void  HandleGameRestore();

		void UpdateSpecialFX();

		void UpdateClientViewPos();
		void UpdateConsoleVars();

        void TeleFragObjects(LTVector & vPos);

		void HandleTeleport(const LTVector& vPos);
		void HandleTeleport(TeleportPoint* pTeleportPoint);
		void Teleport(const LTVector & vPos, const LTVector & vPitchYawRoll);

		void HandleLinkBroken(HOBJECT hObj);
		void BuildCameraList();

		void WriteVehicleMessage(ILTMessage_Write *pMsg);
		void WriteInterfaceMessage(ILTMessage_Write *pMsg);

		void ChangeSpeedsVar(float &var, float fVal)
		{
			if (var != fVal)
			{
				m_PStateChangeFlags |= PSTATE_SPEEDS;
			}
			var = fVal;
		}

		void	SetLeashLen(float fVal)		{ChangeSpeedsVar(m_fLeashLen, fVal);}
		void	SetLeashSpring(float fVal)	{ChangeSpeedsVar(m_fLeashSpring, fVal);}
		void	SetLeashSpringRate(float fVal)	{ChangeSpeedsVar(m_fLeashSpringRate, fVal);}
		void	SetRunVel(float fVal)		{ChangeSpeedsVar(m_fRunVel, fVal);}
		void	SetWalkVel(float fVal)		{ChangeSpeedsVar(m_fWalkVel, fVal);}
		void	SetJumpVel(float fVal)		{ChangeSpeedsVar(m_fJumpVel, fVal);}
		void	SetSwimVel(float fVal)		{ChangeSpeedsVar(m_fSwimVel, fVal);}
		void	SetMoveMul(float fVal)		{ChangeSpeedsVar(m_fMoveMultiplier, fVal);}
		void	SetJumpVelMul(float fVal)	{ChangeSpeedsVar(m_fJumpMultiplier, fVal);}
		void	SetLadderVel(float fVal)	{ChangeSpeedsVar(m_fLadderVel, fVal);}

		virtual void	SetPhysicsModel(PlayerPhysicsModel eModel=PPM_NORMAL, LTBOOL bUpdateClient=LTTRUE);
		virtual void	SetVehiclePhysicsModel(PlayerPhysicsModel eModel);
		virtual void	SetNormalPhysicsModel();
	public:
		// RETAIL: virtual in Tron (CTronPlayerObj's removes its light cycle,
		// retail 0x10067900)
		virtual void	RemoveVehicleModel();
	protected:

		void	AcquireDefaultWeapon();
		bool	AcquireLevelDefaultWeapons( int nMissionId, int nLevelId );
		bool	AcquireLevelDefaultMods( int nMissionId, int nLevelId );
		bool	AcquireLevelDefaultAmmo( int nMissionId, int nLevelId );
		bool	AcquireCurrentLevelDefaults();
		// RETAIL: virtual in Tron, with a flag (retail 0x1007e9d0, ret 8;
		// CTronPlayerObj's, 0x100686e0, hands the weapon's data to
		// TronUnknownVirtual128 when it is set). The flag's name is ours
		virtual void	AcquireWeapon(char* pWeaponName, bool bTronUnknown = false);
		void	AcquireMod(char* pModName);
		void	AcquireAmmo(char* pAmmoName);
		void	AcquireGear(char* pGearName);
		void	ChangeToWeapon(const char* pWeaponName);

		virtual void HandleVolumeEnter(AIInformationVolume* pNewVolume);
		virtual float GetVerticalThreshold() const;
		virtual float GetInfoVerticalThreshold() const;

		// RETAIL: three virtuals of Tron's about weapons, returning true here
		// (retail 0x100dd4f0); CTronPlayerObj's take a weapon by name
		// (0x10066f20), its data (AcquireWeapon and the weapon cheat hand it,
		// 0x100686e0, 0x10066fa0) and its id (0x10066e30, through
		// TronUnknownVirtual128)
		virtual bool	TronUnknownVirtual127( char const* /*pWeaponName*/ ) { return true; }
		virtual bool	TronUnknownVirtual128( WEAPON const* /*pWeapon*/ ) { return true; }
		virtual bool	TronUnknownVirtual129( uint8 /*nWeaponId*/ ) { return true; }
		// RETAIL: an empty virtual of Tron's with two arguments (Respawn calls
		// it, retail 0x1007def8; CTronPlayerObj's is 0x100672a0)
		virtual void	TronUnknownVirtual130( uint32 /*nUnknown1*/, uint32 /*nUnknown2*/ ) {}

        virtual LTBOOL DoActivate(CActivationData* pData);

	
		// Pickup an object.
		bool PickupObject( HOBJECT hObject );
		bool CanDropObject( );

		// Generates a unique name for all players from a given name.
		void GenerateUniqueName( char const* pszNameBase, char* pszUniqueName, int nUniqueNameSize );

		// Setup our dead body...
		virtual void	SetupBody(Body* pBody);
		// Get rid of our dead body
		void	RemoveBody();

		// Accessors the lean value.
		void SetLean( CPPlayerLeanTypes eLean, LTVector const& vPos );
		CPPlayerLeanTypes GetLean( ) { return m_ePlayerLean; }

		//handle request to switch teams
		void SwitchToTeam(uint8 nTeam );

	protected :  // Data members

        LTFLOAT      m_fOldHitPts;
		LTFLOAT		 m_fOldEnergy;
        LTFLOAT      m_fOldArmor;
        LTFLOAT      m_fOldAirLevel;
        LTFLOAT      m_fAirLevel;
        LTVector     m_vOldModelColor;
        LTFLOAT      m_fOldModelAlpha;

		// When we respawn, this is incremented so we ignore packets from an older teleport.

        uint8       m_ClientMoveCode;

		PlayerState m_eState;
		
		// Handles case where the player is invulnerable during respawn in MP.  Don't save.
		bool		m_bRespawnInvulnerability;
		CTimer		m_tmrRespawnInvulnerabilityTime;

		// Handle protection from special damage types while recovering from similar damage.
		// RETAIL: no grief protection in Tron (retail has no GriefTime, and its
		// members are 0x14 bytes fewer here: retail CPlayerObj's constructor)

		// Flag indicating we're waiting for the client to
		// finish loading.  Not saved.
		bool		m_bClientLoaded;

        LTBOOL      m_b3rdPersonView;
        uint32      m_nSavedFlags;
		LTBOOL		m_bRespawnCalled;

		// Animator

		CAnimatorPlayer	m_Animator;

		// RETAIL: a Tron flag at 0x2a80 (retail CPlayerObj's constructor sets
		// it, the server shell sets and clears it, and the player position
		// message asks it before it moves the player). The name is ours
		bool			m_bTronUnknown2a80;

		// Info about cheats...

        LTBOOL		m_bGodMode;
        LTBOOL      m_bAllowInput;
        LTBOOL      m_bChatting;
	    uint16		m_nClientChangeFlags;

		// RETAIL: no cinematic invulnerability flag in Tron (nothing uses
		// 0x2a92; UpdateClientViewPos goes by god mode)

		// Note: These aren't saved/loaded because changing levels
		// will mess up the state of these (and they are cheats anyway)
        LTBOOL		m_bSpectatorMode;	
		// RETAIL: a Tron flag before it (the server shell sets and clears
		// it, retail 0x100555f1; Update does not kill a player outside the
		// world with it). The name is ours
		bool		m_bTronUnknown2a98;
		bool		m_bInvisible;

		CLTMsgRef_Read m_pClientSaveData;  // Client data to save

		CString		m_sStartLevelCommand;

		PlayerPhysicsModel	m_ePPhysicsModel;
		LTObjRefNotifier	m_hVehicleModel;

		LTBOOL				m_bReadyToExit;
		LTBOOL				m_bForceDuck;

		CSkills				m_Skills;

		MissionStats		m_Stats;
		CPlayerScore		m_Score;


	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		CPlayerAttachments*		m_pPlayerAttachments;	// Our attachments aggregate

		LTObjRef	m_CurContainers[MAX_TRACKED_CONTAINERS];
        uint32      m_nCurContainers;

        bool		m_bFirstUpdate;
		bool		m_bNewMission;

		bool		m_bForceUpdateInterface;

        uint32      m_PStateChangeFlags;   // Which things need to get sent to client.

		int*		m_pnOldAmmo;

		HCLIENT		m_hClient;


		// Data members used in load/save...

        uint32      m_dwLastLoadFlags;


		// Multiplayer data
		std::string	m_sNetNameUnique;
		int			m_nRespawnCount;

		LTBOOL		m_bHasDoneHandshake; // Is the handshaking done yet?


		// ForceUpdate camera support...

		CTList<LPBASECLASS>	 m_Cameras;
        LTBOOL               m_bCameraListBuilt;


		// Motion status from client

        uint8       m_nMotionStatus;
        uint8       m_nWeaponStatus;
		CPPlayerLeanTypes m_ePlayerLean;


		// Hiding
		LTFLOAT		m_fHiddenTimer;
		LTFLOAT		m_fHideDuration;
		LTBOOL		m_bHiding;
		LTBOOL		m_bHidden;


		// Carrying

		LTObjRefNotifier	m_hCarriedObject;
		bool				m_bCanDropBody;
		bool				m_bCarryingAI;
		// RETAIL: a Tron flag after it (saved right after it, retail
		// 0x10081423; the commands update asks it). The name is ours
		bool				m_bTronUnknown2cd6;

		// Client/server movement interaction variables
		LTVector	m_vLastClientPos;	// Last position provided by the client
		LTBOOL		m_bUseLeash;		// Is leashing turned on?


		// Console variables used to tweak player movement...

		CVarTrack	m_LeashLenTrack;
		CVarTrack	m_LeashSpringTrack;
		CVarTrack	m_LeashSpringRateTrack;
		CVarTrack	m_ShowNodesTrack;
		CVarTrack	m_MoveVelMulTrack;
		CVarTrack	m_JumpVelMulTrack;
		CVarTrack	m_SwimVelTrack;
		CVarTrack	m_LadderVelTrack;

		float		m_fLeashLen;
		float		m_fLeashSpring;
		float		m_fLeashSpringRate;

        LTBOOL      m_bShowNodes;

		char		m_szMultiplayerSkin[128];
		char		m_szMultiplayerHead[128];

		// Lure Object used for keyframing player's position.
		LTObjRef	m_hPlayerLure;
		
		EnumAIStimulusID m_eLeanVisibleStimID;

		LTVector	m_vLastPos;	// our position last update
                
		static PlayerObjList	m_lstPlayerObjs;

		LTVector	m_vClientCameraOffset;
		
		LTObjRef	m_hDeadBody;

		float		m_fLastVehicleFootstepTime;	

		// RETAIL: Tron keeps the time the camera offset came as a float, put
		// here (retail CPlayerObj's constructor sets 0x2e98 after m_hDeadBody;
		// ClientUpdate stores GetTime() there, Save writes it as a time after
		// the offset). The name is ours (NOLF2's is an int of milliseconds)
		float		m_fClientCameraOffsetTimeReceived;

		// RETAIL: twelve of Tron's subroutines, 0x43 bytes each, at 0x2e9c
		// (retail PostPropRead and Load fill the names from the server's
		// subroutine list, Save and Load write each as a string and three
		// bytes; 0x10086240 finds one by name). The names are ours
		PlayerSubroutineSlot	m_aSubroutines[PLAYER_MAX_SUBROUTINES];

		// Is an enemy AI looking at us?
		bool		m_bVisibleToEnemyAI;

		bool		m_bSendStartLevelCommand;

		// Client-side control flags
		uint32		m_nControlFlags;

		// RETAIL: three Tron flags (retail CPlayerObj's constructor clears
		// them after the model; retail 0x10080f8c and 0x1008110b read them
		// from a client message). The names are ours
		bool		m_bTronUnknown31c8;
		bool		m_bTronUnknown31c9;
		bool		m_bTronUnknown31ca;

		// RETAIL: Tron has no last good drop position nor activation data
		// member here: retail ClientUpdate stores the time of the client's
		// update at 0x31cc, Respawn keeps the start point it used at 0x31d0
		// and the server shell counts in 0x31d4. The names are ours
		float		m_fLastClientUpdateTime;
		GameStartPoint*	m_pLastStartPoint;	// RETAIL: Respawn sets it (0x1007dc96)
		uint32		m_nTronUnknown31d4;

		LTRotation	m_rFullPlayerRot;

		// RETAIL: NOLF2's saved velocity and object flags, save version, used
		// start point, revive and team change members are not in Tron's
		// CPlayerObj (it ends with the rotation, 0x31e8 bytes)
};


#endif  // __CPLAYER_OBJ_H__
