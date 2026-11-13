// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenEnum.h
//
// PURPOSE : Enums and string constants for screens.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#undef ADD_SCREEN

#if defined(INCLUDE_AS_ENUM)
	#define ADD_SCREEN(id,name) SCREEN_ID_##id,
#elif defined(INCLUDE_AS_STRING)
	#define ADD_SCREEN(id,name) #name,
#else
	#error	To use this include file, first define either INCLUDE_AS_ENUM or INCLUDE_AS_STRING, to include the screens as enums, or string constants.
#endif


// RETAIL: Tron's screens, in retail's order: CShell.dll's screen name table
// (char[32] each, 0x1021e314; 0x1021e454 is JOIN_SERVERLIST's) and
// CTronScreenMgr::AddScreen's jump table (0x100ad388), whose cases create
// the classes noted below (by their constructors' vtables). Build 69's list
// (May 2002) is NOLF2's; retail drops PLAYER_SKILLS, JOIN, JOIN_LAN,
// PLAYER_TEAM, HOST_DM/TDM/DD_OPTIONS, HOST_WEAPONS, TEAM, CROSSHAIR and
// END_DM_MISSION, puts JOIN_SERVERLIST and SERVERINFO where JOIN and JOIN_LAN
// were and appends its new screens after SUBROUTINES. The id names of the new
// screens are ours, made from their table strings. Ids AddScreen creates no
// screen for: HOST, HOST_OPTIONS, HOST_MISSION, HOST_LEVELS,
// END_COOP_MISSION, BASE_HOST, BASE_LIGHTCYCLE.

ADD_SCREEN(NONE,GenericScreen)						// 0x00
ADD_SCREEN(MAIN,ScreenMain)							// 0x01 CScreenMain

//main
ADD_SCREEN(SINGLE,ScreenSingle)						// 0x02 CScreenSingle
ADD_SCREEN(MULTI,ScreenMulti)						// 0x03 CScreenMulti
ADD_SCREEN(OPTIONS,ScreenOptions)					// 0x04 CScreenOptions
ADD_SCREEN(PROFILE,ScreenProfile)					// 0x05 CScreenProfile

//single
ADD_SCREEN(LOAD,ScreenLoad)							// 0x06 CScreenLoad
ADD_SCREEN(SAVE,ScreenSave)							// 0x07 CScreenSave

//multi
ADD_SCREEN(PLAYER,ScreenPlayer)						// 0x08 CScreenPlayer
ADD_SCREEN(HOST,ScreenHost)							// 0x09 none
ADD_SCREEN(JOIN_SERVERLIST,ScreenJoinServerList)	// 0x0a CScreenJoinServerList
ADD_SCREEN(SERVERINFO,ScreenServerInfo)				// 0x0b CScreenServerInfo

//options
ADD_SCREEN(DISPLAY,ScreenDisplay)					// 0x0c CScreenDisplay
ADD_SCREEN(AUDIO,ScreenAudio)						// 0x0d CScreenAudio
ADD_SCREEN(GAME,ScreenGame)							// 0x0e CScreenGame
ADD_SCREEN(PERFORMANCE,ScreenPerformance)			// 0x0f CScreenPerformance
ADD_SCREEN(CONTROLS,ScreenControls)					// 0x10 CScreenControls

//multi - host
ADD_SCREEN(HOST_OPTIONS,ScreenHostOptions)			// 0x11 none
ADD_SCREEN(HOST_MISSION,ScreenHostMission)			// 0x12 none
ADD_SCREEN(HOST_LEVELS,ScreenHostLevels)			// 0x13 none

//options - controls
ADD_SCREEN(CONFIGURE,ScreenConfigure)				// 0x14 CScreenConfigure
ADD_SCREEN(MOUSE,ScreenMouse)						// 0x15 CScreenMouse
ADD_SCREEN(KEYBOARD,ScreenKeyboard)					// 0x16 CScreenKeyboard
ADD_SCREEN(JOYSTICK,ScreenJoystick)					// 0x17 CScreenJoystick

//not really standard screens, but needed for layout stuff
ADD_SCREEN(FAILURE,ScreenFailure)					// 0x18 CScreenFailure
ADD_SCREEN(END_MISSION,ScreenEndMission)			// 0x19 CScreenEndMission
ADD_SCREEN(END_COOP_MISSION,ScreenEndCoopMission)	// 0x1a none
ADD_SCREEN(PRELOAD,ScreenPreload)					// 0x1b CScreenPreload
ADD_SCREEN(POSTLOAD,ScreenPostload)					// 0x1c CScreenPostload

// Tron's player management screen
ADD_SCREEN(SUBROUTINES,ScreenSubroutines)			// 0x1d CScreenSubroutines

// Tron's host screens (CBaseScreenHost is their base class)
ADD_SCREEN(BASE_HOST,BaseScreenHost)				// 0x1e none
ADD_SCREEN(HOST_CONNECTION,ScreenHostConnection)	// 0x1f CScreenHostConnection
ADD_SCREEN(HOST_GAME,ScreenHostGame)				// 0x20 CScreenHostGame
ADD_SCREEN(HOST_MAPS,ScreenHostMaps)				// 0x21 CScreenHostMaps

// Tron's light cycle screens (CBaseScreenLightCycle is their base class)
ADD_SCREEN(BASE_LIGHTCYCLE,ScreenBaseLightCycle)	// 0x22 none
ADD_SCREEN(LIGHTCYCLE_CIRCUIT,ScreenLightCycleCircuit)	// 0x23 CScreenLightCycleCircuit
ADD_SCREEN(LIGHTCYCLE_PLAYERCONFIG,ScreenLightCyclePlayerConfig)	// 0x24 CScreenLightCyclePlayerConfig
ADD_SCREEN(LIGHTCYCLE_CUSTOMMAP,ScreenLightCycleCustomMap)	// 0x25 CScreenLightCycleCustomMaps
ADD_SCREEN(LIGHTCYCLE_CUSTOMGAME,ScreenLightCycleCustomGame)	// 0x26 CScreenLightCycleGame
ADD_SCREEN(LIGHTCYCLE_VICTORY,ScreenLightCycleVictory)	// 0x27 CScreenLightCycleVictory

// Tron's team selection screen
ADD_SCREEN(TEAMSELECT,TeamSelect)					// 0x28 CScreenTeamSelect

//this must be the last id
ADD_SCREEN(UNASSIGNED,GenericScreen)				// 0x29
