/******
nonport.c
GameSpy Common Code

Copyright 1999-2003 GameSpy Industries, Inc

******/
/*
 * RETAIL: Tron's copy, linked into CShell.dll (0x1004e070..0x1004e453).
 * Object.lto links the same file (0x1002f920..0x1002fcd4: current_time,
 * SocketShutDown, goastrdup, CanReceiveOnSocket, getlocalhost and the
 * unique ID code), for its GameSpy reporting.
 * cjacksrc's reconstruction of the 2003 SDK's file (written there from the
 * 2007 snapshot's common/gsPlatformUtil.c and gsPlatformSocket.c), with the
 * unique ID code in the order Tron's retail has it: GOAGetUniqueID first,
 * then the static helpers it calls, after their forward declarations. The
 * functions nothing in CShell.dll calls are not in the retail image.
 */

#include "nonport.h"
#ifdef TRON20SRC_MODERN
#include "../../../libs/RegMgr/regfile.h"
#endif

#pragma comment(lib, "wsock32.lib")

#define REGKEY "Software\\GameSpy\\GameSpy 3D\\Registration"

gsi_time current_time()  //returns current time in msec
{
	return (GetTickCount());
}

void msleep(gsi_time msec)
{
	Sleep(msec);
}

void SocketStartUp()
{
	WSADATA data;
	WSAStartup(MAKEWORD(1,1), &data);
}

void SocketShutDown()
{
	WSACleanup();
}

char * goastrdup(const char *src)
{
	char *res;
	if(src == NULL)      //PANTS|02.11.00|check for NULL before strlen
		return NULL;
	res = (char *)gsimalloc(strlen(src) + 1);
	if(res != NULL)      //PANTS|02.02.00|check for NULL before strcpy
		strcpy(res, src);
	return res;
}

int SetSockBlocking(SOCKET sock, int isblocking)
{
	int rcode;
	unsigned long argp;

	if(isblocking)
		argp = 0;
	else
		argp = 1;
	rcode = ioctlsocket(sock, FIONBIO, &argp);

	if(rcode == 0)
		return 1;
	return 0;
}

int DisableNagle(SOCKET sock)
{
	int rcode;
	int noDelay = 1;

	rcode = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&noDelay, sizeof(int));

	return gsiSocketIsNotError(rcode);
}

int SetReceiveBufferSize(SOCKET sock, int size)
{
	int rcode;
	rcode = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const char *)&size, sizeof(int));
	return gsiSocketIsNotError(rcode);
}

int SetSendBufferSize(SOCKET sock, int size)
{
	int rcode;
	rcode = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const char *)&size, sizeof(int));
	return gsiSocketIsNotError(rcode);
}

int GetReceiveBufferSize(SOCKET sock)
{
	int rcode;
	int size;
	int len;

	len = sizeof(size);

	rcode = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&size, &len);

	if(gsiSocketIsError(rcode))
		return rcode;

	return size;
}

int GetSendBufferSize(SOCKET sock)
{
	int rcode;
	int size;
	int len;

	len = sizeof(size);

	rcode = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&size, &len);

	if(gsiSocketIsError(rcode))
		return rcode;

	return size;
}

int CanReceiveOnSocket(SOCKET sock)
{
	fd_set fd;
	struct timeval timeout;
	int rcode;

	FD_ZERO(&fd);
	FD_SET(sock, &fd);

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	rcode = select(FD_SETSIZE, &fd, NULL, NULL, &timeout);
	if(gsiSocketIsError(rcode) || (rcode == 0))
		return 0;

	return 1;
}

int CanSendOnSocket(SOCKET sock)
{
	fd_set fd;
	struct timeval timeout;
	int rcode;

	FD_ZERO(&fd);
	FD_SET(sock, &fd);

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	rcode = select(FD_SETSIZE, NULL, &fd, NULL, &timeout);
	if(gsiSocketIsError(rcode) || (rcode == 0))
		return 0;

	return 1;
}

HOSTENT * getlocalhost(void)
{
	char hostname[256] = "";

	// get the local host's name
	gethostname(hostname, sizeof(hostname));

	// return the host for that name
	return gethostbyname(hostname);
}

int IsPrivateIP(IN_ADDR * addr)
{
	int b1;
	int b2;
	unsigned int ip;

	// get the first 2 bytes
	ip = ntohl(addr->s_addr);
	b1 = (int)(ip >> 24);
	b2 = (int)((ip >> 16) & 0xFF);

	// 10.X.X.X
	if(b1 == 10)
		return 1;

	// 172.16-31.X.X
	if((b1 == 172) && ((b2 >= 16) && (b2 <= 31)))
		return 1;

	// 192.168.X.X
	if((b1 == 192) && (b2 == 168))
		return 1;

	return 0;
}

/* Unique ID: a random key, kept in the registry */

static long randSeed = 1;

static void GenerateID(char *keyval);
static void Util_RandSeed(unsigned long seed);
static int Util_RandInt(int low, int high);
static long longrand(void);
static long nextlongrand(long seed);

static const char * GOAGetUniqueID_Internal(void);

GetUniqueIDFunction GOAGetUniqueID = GOAGetUniqueID_Internal;

static const char * GOAGetUniqueID_Internal(void)
{
	static char keyval[MAX_PATH] = "";
#ifdef TRON20SRC_MODERN
	// MODERN.md APPS-3: the ID is kept in the file beside the program
	// (regfile.h), in the section of its retail key (HKEY_CURRENT_USER\
	// Software\GameSpy\GameSpy 3D\Registration, value Crypt), not in the
	// registry
	DWORD len = MAX_PATH;
	if (!RegFile_GetString(REGKEY + 9, "Crypt", keyval, &len) || strlen(keyval) != 19) //need to generate a new key
	{
		GenerateID(keyval);
		RegFile_SetString(REGKEY + 9, "Crypt", keyval);
	}
#else
	unsigned int ret;
	int docreate;
	HKEY thekey;
	DWORD thetype = REG_SZ;
	DWORD len = MAX_PATH;
	DWORD disp;

	if (RegOpenKeyExA(HKEY_CURRENT_USER, REGKEY, 0, KEY_ALL_ACCESS, &thekey) != ERROR_SUCCESS)
		docreate = 1;
	else
		docreate = 0;
	ret = RegQueryValueExA(thekey, "Crypt", 0, &thetype, (LPBYTE)keyval, &len);

	if (ret != ERROR_SUCCESS || strlen(keyval) != 19) //need to generate a new key
	{
		GenerateID(keyval);
		if (docreate)
		{
			ret = RegCreateKeyExA(HKEY_CURRENT_USER, REGKEY, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &thekey, &disp);
		}
		RegSetValueExA(thekey, "Crypt", 0, REG_SZ, (const LPBYTE)keyval, strlen(keyval)+1);
	}
	RegCloseKey(thekey);
#endif // TRON20SRC_MODERN

	// Strip out the -'s.
	memmove(keyval + 4, keyval + 5, 4);
	memmove(keyval + 8, keyval + 10, 4);
	memmove(keyval + 12, keyval + 15, 4);
	keyval[16] = '\0';

	return keyval;
}

static void GenerateID(char *keyval)
{
	int i;
	const char crypttab[63] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	LARGE_INTEGER l1;
	UINT seed;

	if (QueryPerformanceCounter(&l1))
		seed = (l1.LowPart ^ l1.HighPart);
	else
		seed = 0;

	/* RETAIL: CShell 0x1004e2e0, Object 0x1002fb60 and TRONSrv 0x00403630
	   call time, clock and then GetTickCount. cl calls them in that sequence
	   only from this sequence of the operands. */
	Util_RandSeed(time(NULL) ^ GetTickCount() ^ clock() ^ seed);
	for (i = 0; i < 19; i++)
		if (i == 4 || i == 9 || i == 14)
			keyval[i] = '-';
		else
			keyval[i] = crypttab[Util_RandInt(0, 62)];
	keyval[19] = 0;
}

static void Util_RandSeed(unsigned long seed)
{
	randSeed = (seed != 0) ? (long)(seed & 2147483647) : 1;
}

static int Util_RandInt(int low, int high)
{
	int range = high-low;
	int num;

	num = longrand() % range;

	return(num + low);
}

static long longrand(void)
{
	randSeed = nextlongrand(randSeed);
	return randSeed;
}

static long nextlongrand(long seed)
{
	unsigned long lo, hi;

	lo = 16807 * (long)(seed & 0xFFFF);
	hi = 16807 * (long)((unsigned long)seed >> 16);
	lo += (hi & 0x7FFF) << 16;
	if (lo > 2147483647)
	{
		lo &= 2147483647;
		++lo;
	}
	lo += hi >> 15;
	if (lo > 2147483647)
	{
		lo &= 2147483647;
		++lo;
	}
	return (long)lo;
}

#ifdef TRON20SRC_MODERN
// MODERN (GS-1, GS-2): the domain of the master server and the server
// list (nonport.h). Each binary that links this file (CShell.dll,
// Object.lto, TRONSrv.exe) finds it the same way, so a client, the
// server it hosts and a dedicated server in the same directory agree.

static char s_szDomainName[128];

// The value after "+MasterDomain" (or "-MasterDomain") in the command line.
static int gsiDomainFromCommandLine(char *out, int len)
{
	const char *p = GetCommandLineA();
	char token[128];
	int bNext = 0;

	while (p && *p)
	{
		int n = 0;
		while (*p == ' ' || *p == '\t')
			p++;
		if (!*p)
			break;
		if (*p == '"')
		{
			p++;
			while (*p && *p != '"')
			{
				if (n < (int)sizeof(token) - 1)
					token[n++] = *p;
				p++;
			}
			if (*p == '"')
				p++;
		}
		else
		{
			while (*p && *p != ' ' && *p != '\t')
			{
				if (n < (int)sizeof(token) - 1)
					token[n++] = *p;
				p++;
			}
		}
		token[n] = '\0';
		if (bNext)
		{
			strncpy(out, token, (size_t)len - 1);
			out[len - 1] = '\0';
			return 1;
		}
		if ((token[0] == '+' || token[0] == '-') && _stricmp(token + 1, "MasterDomain") == 0)
			bNext = 1;
	}
	return 0;
}

// One word of a line of autoexec.cfg: quoted or not.
static const char * gsiConfigWord(const char *p, char *out, int len)
{
	int n = 0;
	while (*p == ' ' || *p == '\t')
		p++;
	if (*p == '"')
	{
		p++;
		while (*p && *p != '"' && *p != '\r' && *p != '\n')
		{
			if (n < len - 1)
				out[n++] = *p;
			p++;
		}
		if (*p == '"')
			p++;
	}
	else
	{
		while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n')
		{
			if (n < len - 1)
				out[n++] = *p;
			p++;
		}
	}
	out[n] = '\0';
	return p;
}

// The engine keeps the console variables in autoexec.cfg, in the current
// directory (the game directory), one a line: "MasterDomain" "<value>".
static int gsiDomainFromConfig(char *out, int len)
{
	char line[512];
	char name[64];
	int bFound = 0;
	FILE *fp = fopen("autoexec.cfg", "r");
	if (!fp)
		return 0;
	while (fgets(line, sizeof(line), fp))
	{
		const char *p = gsiConfigWord(line, name, sizeof(name));
		if (_stricmp(name, "MasterDomain") != 0)
			continue;
		gsiConfigWord(p, out, len);
		bFound = 1;
	}
	fclose(fp);
	return bFound;
}

const char * gsiGetDomainName(void)
{
	char szValue[128];
	DWORD nLen;

	szValue[0] = '\0';
	nLen = GetEnvironmentVariableA("TRON20SRC_MASTERDOMAIN", szValue, sizeof(szValue));
	if (nLen == 0 || nLen >= sizeof(szValue))
	{
		szValue[0] = '\0';
		if (!gsiDomainFromCommandLine(szValue, sizeof(szValue)))
			gsiDomainFromConfig(szValue, sizeof(szValue));
	}

	// A host name has no spaces and no slashes; anything else is the default.
	if (!szValue[0] || strpbrk(szValue, " \t/\\:"))
		strcpy(szValue, GSI_DOMAIN_NAME);

	strcpy(s_szDomainName, szValue);
	return s_szDomainName;
}

void gsiMakeHostName(char *hostname, int len, const char *service)
{
	const char *pszDomain = gsiGetDomainName();
	if (inet_addr(pszDomain) != INADDR_NONE)
		_snprintf(hostname, (size_t)len, "%s", pszDomain);
	else
		_snprintf(hostname, (size_t)len, "%s.%s", service, pszDomain);
	hostname[len - 1] = '\0';
}
#endif
