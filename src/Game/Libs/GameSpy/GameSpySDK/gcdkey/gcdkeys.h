/******
gcdkeys.h
GameSpy CDKey SDK Server Header

Copyright 1999-2003 GameSpy Industries, Inc

******/
/*
 * RETAIL: Tron's 2003 SDK, written from what Object.lto links
 * (0x100200b0..0x1002118a) with the 2007 snapshot in cjacksrc as the
 * template.
 */
#ifndef _GOACDKEYS_H_
#define _GOACDKEYS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define QR2CDKEY_INTEGRATION

typedef void (*AuthCallBackFn)(int gameid, int localid, int authenticated, char *errmsg, void *instance);

extern char gcd_hostname[64];

int gcd_init(int gameid);

#ifdef QR2CDKEY_INTEGRATION
#include "../qr2/qr2.h"
int gcd_init_qr2(qr2_t qrec, int gameid);
#endif

void gcd_shutdown(void);
void gcd_authenticate_user(int gameid, int localid, unsigned int userip, const char *challenge,
						   const char *response, AuthCallBackFn authfn, void *instance);
void gcd_disconnect_user(int gameid, int localid);
void gcd_disconnect_all(int gameid);
void gcd_think(void);
char *gcd_getkeyhash(int gameid, int localid);

#ifdef __cplusplus
}
#endif

#endif
