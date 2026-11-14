// ----------------------------------------------------------------------- //
//
// MODULE  : MsgIDs.h
//
// PURPOSE : Message ids
//
// CREATED : 9/22/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __MSG_IDS_H__
#define __MSG_IDS_H__

#include "SFXMsgIds.h"              // Special fx message ids

// RETAIL: Tron's numbering. Tron forked NOLF2's ids in 2002 and added its
// own: the values below are retail Tron's, read from the dispatch switches
// of both retail images (CShell.dll: CGameClientShell::OnMessage
// 0x100e42c0, CInterfaceMgr::OnMessage 0x100c9770, CTronInterfaceMgr's
// 0x1009d9b0, CPlayerMgr's 0x10125360, CTronPlayerMgr's 0x100a4b80,
// ClientMultiplayerMgr's 0x100e8cd0, CMissionMgr's 0x100ebe10; Object.lto:
// CGameServerShell::OnMessage 0x100deab0, CTronGameServerShell's
// 0x1005e0b0, CServerMissionMgr's 0x10161df0, CServerSaveLoadMgr's
// 0x101ca250) and from the ids the message-writing call sites write.
// Names marked "ours" are Tron's ids we named from what their handlers do;
// MID_TRON_UNKNOWN_<id> are Tron's ids not yet identified.

// General Client <-> Server messages

#define MID_PLAYER_UPDATE				100
#define MID_PLAYER_SKILLS				101 // Both ways
#define MID_PLAYER_SUMMARY				102 
#define MID_PLAYER_INFOCHANGE			103 // Both ways
#define MID_TRON_UNKNOWN_104			104	// RETAIL: Tron's, server to client (CTronPlayerObj 0x10067ce0 from a command "off"/"stop"/"revert"/"restore" with three floats; CTronInterfaceMgr 0x1009de00 dispatches its sub-ids 0 to 3)
#define	MID_PLAYER_STATE_CHANGE			105
#define MID_PLAYER_MESSAGE				106
#define MID_PLAYER_CHEAT				107

#define MID_PLAYER_ORIENTATION			108	// Server to client
// Orientation sub-messages
	#define MID_ORIENTATION_ALL			0	// Pitch, Yaw, Roll
	#define MID_ORIENTATION_YAW			1	// Yaw only! (Pitch amd Roll preserved on client)

#define MID_PLAYER_INFO					109
	//player info sub-messages
	#define	MID_PI_JOIN					0	// this player is joining
	#define	MID_PI_EXIST				1	// this is info about an existing player sent to someone joining
	#define MID_PI_UPDATE				2	// this is an update of existing players

#define MID_PLAYER_REMOVED				110
#define MID_PLAYER_SCORED				111
#define MID_PLAYER_RESPAWN				113	// RETAIL: 113 in Tron, as Build 51's (retail CGameServerShell::OnMessage's case for HandlePlayerRespawn 0x100df2e0)
#define MID_PLAYER_MULTIPLAYER_INIT		114
#define MID_PLAYER_SINGLEPLAYER_INIT	115
#define MID_PLAYER_LOADCLIENT			116
#define MID_PLAYER_DAMAGE				117
#define MID_PLAYER_ACTIVATE				118
//Activation sub-messages
	#define MID_ACTIVATE_NORMAL			0
	#define MID_ACTIVATE_SEARCH			1
	#define MID_ACTIVATE_MOVE			2
	#define MID_ACTIVATE_REVIVE			3
	#define MID_ACTIVATE_WAKEUP			4

#define MID_PLAYER_CLIENTMSG			119	// Client to server
#define MID_PLAYER_OVERLAY				120 // Server to client
#define MID_PLAYER_TELEPORT				121 // Client to server
#define MID_PLAYER_SCORE				122	// RETAIL: 122 in Tron, Tron's CPlayerScore (CInterfaceMgr's case 0x100c98e3; SharedScoring's MID_TRON_SCORE)

#define MID_PLAYER_TRANSMISSION			123 // Server to client
#define MID_PLAYER_CHATMODE				124	// Client to server
#define MID_PLAYER_GHOSTMESSAGE			125
#define MID_PLAYER_CREDITS				126
#define MID_TRON_HUD_TEXT				127	// RETAIL: Tron's, server to client: two string ids shown with the "msgfadeout" sound (CTronInterfaceMgr 0x1009df00). The name is ours
#define MID_PLAYER_CONNECTED			128
#define MID_SET_PERFORMANCE_RATING		129	// RETAIL: Tron's, server to client: a rating and its value (CTronInterfaceMgr 0x1009e03b, CRatingMgr::SetPerformanceRating). The name is ours

#define MID_WEAPON_CHANGE				130	  // Both ways
#define MID_WEAPON_SOUND				131
#define MID_WEAPON_SOUND_LOOP			132
#define MID_WEAPON_FIRE					133
#define MID_WEAPON_RELOAD				134


#define MID_COMPILE						135	// Client to Server (ratings update)

	#define MWEAPFIRE_VECTOR              0
	#define MWEAPFIRE_PROJECTILE          1
	#define MWEAPFIRE_DISC                2

///////////////////////////////////////////
// Light cycle messages for TRON
#define MID_LIGHT_CYCLE_INFO			136	// Client <-> Server

	#define LCI_ADD_CYCLIST				1
	#define LCI_REMOVE_CYCLIST			2
	#define LCI_DEREZ_CYCLIST			3
	// RETAIL: Tron's further sub-ids, from the server's CServerLightCycleMgr
	// (Object.lto 0x1004f420..0x10053050); the names are ours.
	#define LCI_TRONUNKNOWN_6			6	// Client to Server: a string for the player
	#define LCI_TURBO					8
	#define LCI_START_LIGHT_CYCLE		9	// Client to Server
	#define LCI_END_LIGHT_CYCLE			10	// Client to Server
	#define LCI_SET_POWERUP				11	// Server to Client
	#define LCI_ACTIVATE_POWERUP		12
	#define LCI_TRONUNKNOWN_13			13	// Server to Client
	#define LCI_TRIM_TRAIL				14	// both ways: a trail trimmed (CShell 0x10014620)
	#define LCI_START_UPDATING			15	// Client to Server; Server to the cycle object
	#define LCI_UPDATING_STARTED		16	// Server to Client
	#define LCI_DIALOG					22	// Server to Client: a rider's line
	#define LCI_POSITION				24	// Client to Server
	#define LCI_COLLISION				25	// Server to Client
	#define LCI_TRONUNKNOWN_26			26	// Server to Client
	#define LCI_GAME_SPEED				27	// Server to Client
	#define LCI_TRAIL_LENGTH			28	// Server to Client
	#define LCI_POWERUP_GRABBED			30	// Server to Client
	#define LCI_POWERUP_TAKEN			31	// Server to Client
	#define LCI_CYCLISTS				32	// Server to Client: all the cyclists
	#define LCI_TRONUNKNOWN_33			33	// Server to Client
	#define LCI_GRAB_FAILED				34	// Server to Client
///////////////////////////////////////////

#define	MID_PERFORMANCE					149	// RETAIL: 149 in Tron (CUserProfile::SendPerformanceMsg, CShell.dll; CGameServerShell::HandlePerformanceMessage's case, Object.lto 0x100ded8a)


// Messages regarding projectiles
#define MID_PROJECTILE                          138
	#define MPROJ_UPDATE_POINT_TARGET             0 // Client to Server- give a projectile a new point target
	#define MPROJ_UPDATE_OBJECT_TARGET            1
	#define MPROJ_UPDATE_CONTROL_LINE             2 // Client to server- give a projectile a new control line
	#define MPROJ_RETURN                          3 // Client to Server- return the projectile to the owner, if applicable
	#define MPROJ_RETURNING                       4 // Server to Client- weapon is returning
	#define MPROJ_RETURNING_DISTANCE_THRESHOLD    5 // Server to Client- sent to an AI owner when returning disc is a certain distance from that owner
	#define MPROJ_RETURNED                        6 // Server to Client- weapon has returned, unlock it
	#define MPROJ_START_SENDING_MESSAGES          7
	#define MPROJ_STOP_SENDING_MESSAGES           8
	#define MPROJ_BLOCKED                         9 // Server to Client- player has successfully blocked the disc
	#define MPROJ_START_SWAT_BLOCK               10
	#define MPROJ_START_HOLD_BLOCK               11
	#define MPROJ_END_HOLD_BLOCK                 12
	#define MPROJ_START_ARM_BLOCK                13
	// types of Disc tracking
	#define MPROJ_DISC_TRACKING_HOMING          100
	#define MPROJ_DISC_TRACKING_STEADY          101
	#define MPROJ_DISC_TRACKING_POINT_GUIDED    102
	#define MPROJ_DISC_TRACKING_CONTROL_LINE    103

#define MID_WEAPON_CONTINUOUS_FIRE_START		139	// RETAIL: Tron's, client to server (CShell.dll 0x10018810, CClientWeaponContinuousFireBase's TU; CGameServerShell's handler 0x100def60 calls CPlayerObj's virtual 107). The name is ours
#define MID_WEAPON_CONTINUOUS_FIRE_STOP			140	// RETAIL: Tron's, client to server (CShell.dll 0x10018470; handler 0x100def90, CPlayerObj's virtual 108). The name is ours
#define MID_DISC_HOLD_DEFENSE					142	// RETAIL: Tron's, client to server (CClientWeaponDisc, CShell.dll 0x10019e77; CTronGameServerShell's case to CTronPlayerObj 0x10063d60, a vector). The name is ours

#define MID_CLIENT_LOADED						144	// RETAIL: 144 in Tron (retail CGameServerShell::OnMessage 0x100deab0 sends it to HandleClientLoaded)
#define MID_SWITCHINGWORLDSSTATE				145	// RETAIL: 145 in Tron (retail Object.lto 0x100dda93, 0x100e1a34)
enum SwitchingWorldsState
{
	eSwitchingWorldsStateNone,
	eSwitchingWorldsStatePlayerHookup,
	eSwitchingWorldsStateAutoSave,
	eSwitchingWorldsStateWaitForClient,
	eSwitchingWorldsStateFinished,
};

#define MID_CLIENT_READY_FOR_AUTOLOAD			141	// Client to Server- Lets server know the client is ready to begin an automatic load.

// Info on doomsday objects...
#define MID_DOOMSDAY_MESSAGE					198 // Server -> Client. PORT: NOLF2's; Tron has none (142 is Tron's MID_DISC_HOLD_DEFENSE): moved to an id Tron leaves free
	#define	MID_DOOMSDAY_PIECE_PICKEDUP			0
	#define MID_DOOMSDAY_PIECE_DROPPED			1
	#define MID_DOOMSDAY_PIECE_PLACED			2
	#define MID_DOOMSDAY_PIECE_STOLEN			3	
	#define MID_DOOMSDAY_DEVICE_COMPLETED		4
	#define MID_DOOMSDAY_PIECE_RESPAWNED		5

#define MID_PLAYER_TAUNT                        146
#define MID_MULTIPLAYER_UPDATE                  147
#define MID_TRON_DISC_FIRED                     148	// RETAIL: Tron's, client to server, no data: sent from the disc's fire animation (CShell.dll 0x1001a370); CTronGameServerShell's case 0x1005e168 sets CTronPlayerObj's flag (0x10068660). The name is ours
#define MID_TRON_EXIT_REQUEST					150	// RETAIL: Tron's, client to server: a bool (CShell.dll 0x1011df1c); CTronGameServerShell's case to CTronPlayerObj 0x10068560 ("ExitTrigger", "ENDLIGHTCYCLE", "exit"). The name is ours
#define MID_TRON_UNKNOWN_151					151	// RETAIL: Tron's, server to client (CTronPlayerObj::FilterDamage; CTronInterfaceMgr 0x1009dde4 hands it to the subroutine manager, 0x10093580)
#define MID_TRON_UNKNOWN_152					152	// RETAIL: Tron's, server to client: a bool (CTronPlayerObj::OnTrigger; CTronPlayerMgr 0x100a6ec0)

// PORT: NOLF2's, not in Tron (their ids are Tron's MID_TRON_DISC_FIRED,
// MID_PERFORMANCE, MID_TRON_EXIT_REQUEST): moved to ids Tron leaves free
#define MID_MULTIPLAYER_OPTIONS                 199
#define	MID_TEAM_INFO							204
	#define MTEAM_FULL							0
	#define MTEAM_SCORE							1
#define	MID_TEAM_ADD							205
#define MID_PLAYER_CANCELREVIVE					196	// PORT: NOLF2's (113 is Tron's MID_PLAYER_RESPAWN)
#define MID_PLAYER_TEAM							197	// PORT: NOLF2's (127 is Tron's MID_TRON_HUD_TEXT)

#define MID_MUSIC                               153

// RETAIL: 154 to 156 in Tron (CTronPlayerMgr::OnMessage's cases; BinaryBit
// and PatchRoutine, Object.lto 0x10032af5: the client's stop and complete
// come back as object messages)
#define MID_START_ENERGY_TRANSFER				154	// Server to Client
#define MID_STOP_ENERGY_TRANSFER				155	// Client to Server
#define MID_ENERGY_TRANSFER_COMPLETE			156 // Client to Server
#define MID_TRON_UNKNOWN_157					157	// RETAIL: Tron's, server to client: a float for the subroutine manager (CTronInterfaceMgr 0x1009de96)

#define MID_QUERY_TARGET_PROPERTIES				158 // Client <-> Server
// Target Property ID's:
	#define TARGET_PROP_ENERGY_REQUIRED			1
	#define TARGET_PROP_HEALTH					2
	#define TARGET_PROP_INFO_STRING				8	// RETAIL: InfoString's (retail Object.lto); the name is ours

#define MID_OBJECT_MESSAGE						159 // Client -> Server
	
#define MID_GAME_PAUSE                          160
#define MID_GAME_UNPAUSE                        161

#define MID_USE_ENERGY							162	// RETAIL: Tron's, client to server: energy used (CTronPlayerStats::UseEnergy, CSubroutineMgr::Update; CTronGameServerShell's case to CTronPlayerObj 0x10063cb0). The name is ours
#define MID_TRON_UNKNOWN_163					163	// RETAIL: Tron's, server to client (CTronPlayerObj::OnTrigger; CTronInterfaceMgr 0x1009ddae: string 3235 on the subroutines screen)
#define MID_TRON_ENERGY_GAINED					164	// RETAIL: Tron's, server to client: a byte for string 3271 (CoreDump; CTronInterfaceMgr 0x1009dec8). The name is ours
#define MID_SYSTEM_MEMORY_CONFIGURATION			165	// RETAIL: Tron's, server to client: a string (CTronPlayerObj::OnTrigger; CTronInterfaceMgr 0x1009e093, CSubroutineMgr::SetSystemMemoryConfiguration). The name is ours
#define MID_TRON_UNKNOWN_166					166	// RETAIL: Tron's, server to client: a uint32, a byte and a string for the subroutines screen (CTronPlayerObj 0x10066b90; CTronInterfaceMgr 0x1009dd4b)

#define MID_GEAR_PICKEDUP                       170
#define MID_INTEL_PICKEDUP                      171

#define MID_SUBROUTINE_OBTAINED					172
#define MID_ADDITIVE_OBTAINED					173
#define MID_PROCEDURAL_OBTAINED					174
#define MID_PRIMITIVE_OBTAINED					175
#define MID_TRON_PING							176	// RETAIL: Tron's, server to client: a string and add or remove (CTronInterfaceMgr 0x1009db90, CTronPlayerStats::AddPing). The name is ours
#define MID_OPTIMIZER_BEGIN						177	// RETAIL: Tron's, server to client (Optimizer, Object.lto 0x10031769; CTronInterfaceMgr 0x1009dd06 opens the subroutines screen). The name is ours
#define MID_OPTIMIZER_RESULT					178	// RETAIL: Tron's, client to server as an object message (Optimizer 0x100308bd). The name is ours
#define MID_TRON_UNKNOWN_180					180	// RETAIL: Tron's, server to client: a bool, a position and a rotation (CTronPlayerObj::OnTrigger; CTronInterfaceMgr 0x1009dc03)

#define MID_SAVE_GAME                           181

#define MID_LOAD_GAME							182
enum LoadGameType
{
	kLoadGameTypeSlot,
	kLoadGameTypeQuick,
	kLoadGameTypeReload,
	kLoadGameTypeContinue,
};

// First message sent from client to server to start the game.
#define MID_START_GAME							183

// Sent when the game is over.
#define MID_END_GAME							184

// Tell server to load a level from scratch.
#define MID_START_LEVEL							185

#define MID_EXIT_LEVEL							186

#define MID_SAVE_DATA							187
#define MID_TRON_UNKNOWN_188					188	// RETAIL: Tron's, server to client: a bool (CTronPlayerObj::OnTrigger; CTronInterfaceMgr 0x1009dcdc)

#define MID_SERVER_ERROR                        190
#define MID_LOAD_FAILED							191
#define MID_OBJECTIVES_DATA						192
#define MID_SYSTEM_MEMORY						193	// RETAIL: Tron's, server to client (CTronPlayerMgr::HandleMsgSystemMemory 0x100a6740). The name is ours
#define MID_TRON_UNKNOWN_194					194	// RETAIL: Tron's, server to client (Object.lto 0x101ccc00; CMissionMgr::OnMessage's case to its virtual 1)
#define MID_TRON_UNKNOWN_195					195	// RETAIL: Tron's, server to client (Object.lto 0x101ccc00; CMissionMgr::OnMessage's case to its virtual 2)
#define MID_TRON_SERVERMESSAGE					200	// RETAIL: Tron's, both ways, with sub-ids (SharedScoring.h; CTronPlayerMgr 0x100a6e50, CTronGameServerShell's case). The name is ours
#define MID_SCMD								201	// RETAIL: 201 in Tron (retail ScmdConsole_Impl::OnMessage, TRONSrv.exe 0x403d8c, CShell.dll 0x1017eadc; ScmdServer_Impl::OnMessage, Object.lto 0x101c5cf8)

#define MID_MULTIPLAYER_DATA                    202
#define MID_MULTIPLAYER_SERVERDIR				203 // server directory message

#define MID_CLIENT_PLAYER_UPDATE				212	// Server to client
#define MID_FRAG_SELF							213 // Client to server
#define MID_PINGTIMES							214 // Server to client
#define MID_SHAKE_SCREEN						215 // Client to server
#define MID_CHANGE_WORLDPROPERTIES				216	// Server to client

#define MID_CLIENTFX_CREATE                     230 // Server to client
#define MID_SFX_MESSAGE                         231 // Server to client

#define MID_CONSOLE_TRIGGER                     235 // Client to server
#define MID_CONSOLE_COMMAND                     236 // Client to server
#define MID_CONSOLE_CLIENTFX					237 // Client to server

#define MID_DECISION							239	// Both ways
#define MID_DIFFICULTY							240 // Client to server
#define MID_GAMETEXMGR							241	// Server to client
#define MID_DISPLAY_METER						243	// Server to client						
#define MID_TRON_VERSION						244	// RETAIL: Tron's, server to client: show the jet version (CTronPlayerObj::OnTrigger; CTronInterfaceMgr 0x1009df94, string 2557). The name is ours

#define MID_HANDSHAKE                           245 // Both ways, handshake negotiation
// Handshake sub-messages
    #define MID_HANDSHAKE_HELLO                 0   // Hello! (version verification)  Both ways
    #define MID_HANDSHAKE_PASSWORD              1   // Wassa password?  Server to client
    #define MID_HANDSHAKE_LETMEIN               2   // What, this password?  Client to server
    #define MID_HANDSHAKE_DONE                  3   // You'll do just fine.  Server to client
    #define MID_HANDSHAKE_WRONGPASS             4   // Wrong password.  Server to client
    // Handshake version state

#define MID_GADGETTARGET						246 // Both ways, handling of a gadget target
#define MID_CLEAR_PROGRESSIVE_DAMAGE			247 // Client to server
#define MID_SEARCH								248 // Both ways, handling of searching
#define MID_ADD_PUSHER							249 // Server to client

// AI message IDs 250 - 255
#define MID_STIMULUS                            250 // Client to server
#define MID_RENDER_STIMULUS                     251 // Client to server
#define MID_ADD_GOAL                            252 // Client to server
#define MID_REMOVE_GOAL                         253 // Client to server
#define MID_OBJECT_ALPHA                        254 // Client to server


// Object <-> Object messages...


// Interface Change ids shared between client and server...These are
// used with above MID_PLAYER_INFOCHANGE

#define IC_AMMO_ID					1
#define IC_HEALTH_ID				2
#define IC_ARMOR_ID					3
#define IC_DEFAULTWEAPON_ID			4
#define IC_WEAPON_PICKUP_ID			5
#define IC_AIRLEVEL_ID				6
#define IC_ROCKETLOCK_ID			7
#define IC_OUTOFAMMO_ID				8
#define IC_OBJECTIVE_ID				9
#define IC_MOD_PICKUP_ID			10
#define IC_FADE_SCREEN_ID			11
#define IC_RESET_INVENTORY_ID		12
#define IC_MISSION_TEXT_ID			13
#define IC_MISSION_FAILED_ID		14
#define IC_MAX_HEALTH_ID			15
#define IC_MAX_ARMOR_ID				16
#define IC_WEAPON_OBTAIN_ID			17
#define IC_OPTION_ID				18
#define IC_PARAMETER_ID				19
#define IC_KEY_ID					20
#define IC_SKILLS_ID				21
#define IC_REGION_CHANGE			22
#define IC_MAX_ENERGY_ID			23
#define IC_ENERGY_ID				24
#define IC_PSETS_ID					25
#define IC_PERFORMANCE_RATING_ID	26
#define IC_BUILD_POINTS_ID			27

//helper id's that tell what to do with objectives, parameters, keys, etc.
#define ITEM_ADD_ID			0
#define ITEM_REMOVE_ID      1
#define ITEM_CLEAR_ID       2
#define ITEM_COMPLETE_ID    3

//helper id's to differentiate between different types of search messages
#define SEARCH_START		0
#define SEARCH_FOUND		1


// Client to player messages...These are used with MID_PLAYER_CLIENTMSG
enum ClientPlayerMessage
{
	CP_MOTION_STATUS = 1,
	CP_DAMAGE,
	CP_WEAPON_STATUS,
	CP_FLASHLIGHT,
	CP_PLAYER_LEAN,
	CP_PHYSICSMODEL,
	CP_DAMAGE_VEHICLE_IMPACT,
	CP_DEFLECT,
};

// CP_MOTION_STATUS, Motion Status is used to notify the server if the motion of the player
// has changed (did he jump/land/fall)....
enum CPMotionStatusTypes
{
	MS_NONE,
	MS_JUMPED,
	MS_FALLING,
	MS_LANDED,
};

// CP_DAMAGE, Damage is used to inflict damage the player (e.g., fall damage)...

// CP_WEAPON_STATUS, Weapon Status is used to tell the server what animation the player-view
// weapon is playing...
enum CPWeaponStatusTyps
{
	WS_NONE,
	WS_RELOADING,
	WS_SELECT,
	WS_DESELECT,
};

// CP_FLASHLIGHT, Flashlight is used to simply send new info regarding the location of the 
// flashlight beam impact
enum CPFlashLightTypes
{
	FL_ON,
	FL_OFF,
};

// CP_PLAYER_LEAN, Lean is used to register a stimulus when the player is fully leaned out
// and remove the stimulu when they start to lean back in. 
enum CPPlayerLeanTypes
{
	PL_LEFT,
	PL_RIGHT,
	PL_CENTER,
};

// CP_PHYSICSMODEL, New model is defined by PlayerPhysicsModel...

// CP_DAMAGE_VEHICLE_IMPACT, Damage from vehicle impact

// Client update flags

#define CLIENTUPDATE_CAMERAOFFSET            (1<<0)
#define CLIENTUPDATE_CONTCODE                (1<<1)
#define CLIENTUPDATE_3RDPERSON               (1<<2)
#define CLIENTUPDATE_3RDPERVAL               (1<<3)
#define CLIENTUPDATE_ALLOWINPUT              (1<<4)
#define CLIENTUPDATE_PLAYERROT               (1<<5)
#define CLIENTUPDATE_ACCURATEPLAYERROT       (1<<6)
#define CLIENTUPDATE_FULLPLAYERROT			 (1<<7)


#endif // __MSG_IDS_H__
