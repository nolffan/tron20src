/******
qr2.h
GameSpy Query & Reporting 2 SDK

Copyright 1999-2003 GameSpy Industries, Inc

******/
/*
 * RETAIL: Tron's 2003 SDK, written from what Object.lto links
 * (0x10041fa0..0x10043380) with the 2007 snapshot in cjacksrc as the
 * template. The 2003 SDK has no unicode interface, no public address,
 * client connected or IP filter callbacks, no IP verification and no split
 * query replies; struct qr2_implementation_s is 0x108 bytes (retail
 * qr2_init_socket, 0x10041fa0).
 */
#ifndef _QR2_H_
#define _QR2_H_

#include "../nonport.h"
#include "qr2regkeys.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{e_qrnoerror, //no error occured
e_qrwsockerror, //a standard socket call failed (exhausted resources?)
e_qrbinderror, //the SDK was unable to find an available port to bind on
e_qrdnserror, //a DNS lookup (for the master server) failed
e_qrconnerror,  //the server is behind a nat and does not support negotiation
e_qrnochallengeerror //no challenge was received from the master - either the master is down, or a firewall is blocking UDP
} qr2_error_t;

typedef enum {key_server, key_player, key_team, key_type_count} qr2_key_type;

typedef struct qr2_implementation_s *qr2_t;
typedef struct qr2_keybuffer_s *qr2_keybuffer_t;
typedef struct qr2_buffer_s *qr2_buffer_t;

typedef void (*qr2_serverkeycallback_t)(int keyid, qr2_buffer_t outbuf, void *userdata);
typedef void (*qr2_playerteamkeycallback_t)(int keyid, int index, qr2_buffer_t outbuf, void *userdata);
typedef void (*qr2_keylistcallback_t)(qr2_key_type keytype, qr2_keybuffer_t keybuffer, void *userdata);
typedef int  (*qr2_countcallback_t)(qr2_key_type keytype, void *userdata);
typedef void (*qr2_adderrorcallback_t)(qr2_error_t error, char *errmsg, void *userdata);
typedef void (*qr2_natnegcallback_t)(int cookie, void *userdata);
typedef void (*qr2_clientmessagecallback_t)(char *data, int len, void *userdata);

void qr2_register_natneg_callback(qr2_t qrec, qr2_natnegcallback_t nncallback);
void qr2_register_clientmessage_callback(qr2_t qrec, qr2_clientmessagecallback_t cmcallback);

qr2_error_t qr2_init(/*[out]*/qr2_t *qrec, const char *ip, int baseport, const char *gamename, const char *secret_key,
			int ispublic, int natnegotiate,
			qr2_serverkeycallback_t server_key_callback,
			qr2_playerteamkeycallback_t player_key_callback,
			qr2_playerteamkeycallback_t team_key_callback,
			qr2_keylistcallback_t key_list_callback,
			qr2_countcallback_t playerteam_count_callback,
			qr2_adderrorcallback_t adderror_callback,
			void *userdata);

qr2_error_t qr2_init_socket(/*[out]*/qr2_t *qrec, SOCKET s, int boundport, const char *gamename, const char *secret_key,
			int ispublic, int natnegotiate,
			qr2_serverkeycallback_t server_key_callback,
			qr2_playerteamkeycallback_t player_key_callback,
			qr2_playerteamkeycallback_t team_key_callback,
			qr2_keylistcallback_t key_list_callback,
			qr2_countcallback_t playerteam_count_callback,
			qr2_adderrorcallback_t adderror_callback,
			void *userdata);

void qr2_think(qr2_t qrec);
void qr2_parse_query(qr2_t qrec, char *query, int len, struct sockaddr *sender);
void qr2_send_statechanged(qr2_t qrec);
void qr2_shutdown(qr2_t qrec);
void qr2_keybuffer_add(qr2_keybuffer_t keybuffer, int keyid);
void qr2_buffer_add(qr2_buffer_t outbuf, const char *value);
void qr2_buffer_add_int(qr2_buffer_t outbuf, int value);

/* for CDKey SDK integration */
#define REQUEST_KEY_LEN 4
#define RECENT_CLIENT_MESSAGES_TO_TRACK 10
typedef void (*cdkey_process_t)(char *buf, int len, struct sockaddr *fromaddr);

struct qr2_implementation_s
{
	SOCKET hbsock;									// 0x00
	char gamename[64];								// 0x04
	char secret_key[64];							// 0x44
	char instance_key[REQUEST_KEY_LEN];				// 0x84
	qr2_serverkeycallback_t server_key_callback;	// 0x88
	qr2_playerteamkeycallback_t player_key_callback;
	qr2_playerteamkeycallback_t team_key_callback;
	qr2_keylistcallback_t key_list_callback;
	qr2_countcallback_t playerteam_count_callback;
	qr2_adderrorcallback_t adderror_callback;		// 0x9c
	qr2_natnegcallback_t nn_callback;				// 0xa0
	qr2_clientmessagecallback_t cm_callback;		// 0xa4
	gsi_time lastheartbeat;							// 0xa8
	gsi_time lastka;								// 0xac
	int listed_state;								// 0xb0
	int ispublic;									// 0xb4
	int qport;										// 0xb8
	int read_socket;								// 0xbc
	int nat_negotiate;								// 0xc0
	struct sockaddr_in hbaddr;						// 0xc4
	cdkey_process_t cdkeyprocess;					// 0xd4
	int client_message_keys[RECENT_CLIENT_MESSAGES_TO_TRACK];	// 0xd8
	int cur_message_key;							// 0x100
	void *udata;									// 0x104
};

#ifdef __cplusplus
}
#endif

#endif
