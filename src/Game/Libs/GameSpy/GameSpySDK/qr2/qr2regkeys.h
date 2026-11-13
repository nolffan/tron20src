/******
qr2regkeys.h
GameSpy Query & Reporting 2 SDK: the reserved keys

Copyright 1999-2003 GameSpy Industries, Inc

******/
/*
 * RETAIL: Tron's 2003 SDK, written from what Object.lto links (the key list
 * at 0x102dc630, qr2_register_key at 0x10043380) with the 2007 snapshot in
 * cjacksrc as the template. The 2003 list ends at modname (30; no
 * nn_groupid, country or region keys).
 */
#ifndef _QR2REGKEYS_H_
#define _QR2REGKEYS_H_

#include "../nonport.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_REGISTERED_KEYS 254
#define NUM_RESERVED_KEYS 50

#define HOSTNAME_KEY		1
#define GAMENAME_KEY		2
#define GAMEVER_KEY			3
#define HOSTPORT_KEY		4
#define MAPNAME_KEY			5
#define GAMETYPE_KEY		6
#define GAMEVARIANT_KEY		7
#define NUMPLAYERS_KEY		8
#define NUMTEAMS_KEY		9
#define MAXPLAYERS_KEY		10
#define GAMEMODE_KEY		11
#define TEAMPLAY_KEY		12
#define FRAGLIMIT_KEY		13
#define TEAMFRAGLIMIT_KEY	14
#define TIMEELAPSED_KEY		15
#define TIMELIMIT_KEY		16
#define ROUNDTIME_KEY		17
#define ROUNDELAPSED_KEY	18
#define PASSWORD_KEY		19
#define GROUPID_KEY			20
#define PLAYER__KEY			21
#define SCORE__KEY			22
#define SKILL__KEY			23
#define PING__KEY			24
#define TEAM__KEY			25
#define DEATHS__KEY			26
#define PID__KEY			27
#define TEAM_T_KEY			28
#define SCORE_T_KEY			29
#define MODNAME_KEY			30

extern const char *qr2_registered_key_list[];

void qr2_register_key(int keyid, const char *key);

#ifdef __cplusplus
}
#endif

#endif
