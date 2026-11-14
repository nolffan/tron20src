/*
GameSpy PT SDK
Dan "Mr. Pants" Schoenblum
dan@gamespy.com

Copyright 2000 GameSpy Industries, Inc

RETAIL: the 2002 SDK's patch check as Tron 2.0's CShell.dll links it
(0x1004f770..0x1004fa4c), written from the retail code with the 2007
SDK (cjacksrc) as the guide: its buffers are smaller (256 bytes, the
download URL 101) and its URL also carries the user and unique IDs.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pt.h"
#include "../ghttp/ghttp.h"
#include "../nonport.h"

/************
** DEFINES **
************/
#define PTA_MAX_STRING_SIZE 256
#define PTA_MAX_URL_SIZE    101

/**********
** TYPES **
**********/
typedef struct ptaPatchData
{
	ptPatchCallback callback;
	void * param;
} ptaPatchData;

/************
** GLOBALS **
************/
static char URL[PTA_MAX_STRING_SIZE];	// RETAIL: 0x1026e12c

/**************
** FUNCTIONS **
**************/
static const char * ptaGetKeyValue(const char * buffer, const char * key);
static void ptaCallPatchCallback(ptaPatchData * data, PTBool available, PTBool mandatory,
	const char * versionName, int fileID, const char * downloadURL);
static GHTTPBool ptaPatchFailed(ptaPatchData * data);

// RETAIL: 0x1004f770
static GHTTPBool ptaPatchCompletedCallback
(
	GHTTPRequest request,
	GHTTPResult result,
	char * buffer,
	GHTTPByteCount bufferLen,
	void * param
)
{
	ptaPatchData * data = (ptaPatchData *)param;
	const char * value;
	PTBool mandatory;
	int fileID;
	char versionName[PTA_MAX_STRING_SIZE];
	char downloadURL[PTA_MAX_URL_SIZE];

	// Check for success.
	/////////////////////
	if(result != GHTTPSuccess)
		return ptaPatchFailed(data);

	// Check for a patch.
	/////////////////////
	value = ptaGetKeyValue(buffer, "\\newver\\");
	if(!value || !atoi(value))
		return ptaPatchFailed(data);

	// Check the mandatory flag.
	////////////////////////////
	value = ptaGetKeyValue(buffer, "\\lockout\\");
	mandatory = (value && atoi(value));

	// Get the file id.
	///////////////////
	value = ptaGetKeyValue(buffer, "\\fpfileid\\");
	if(value)
		fileID = atoi(value);
	else
		fileID = 0;

	// Get the name.
	////////////////
	value = ptaGetKeyValue(buffer, "\\newvername\\");
	if(value)
	{
		strncpy(versionName, value, sizeof(versionName));
		versionName[sizeof(versionName) - 1] = '\0';
	}
	else
		versionName[0] = '\0';

	// Get the URL.
	///////////////
	value = ptaGetKeyValue(buffer, "\\dlurl\\");
	if(value)
	{
		strncpy(downloadURL, value, sizeof(downloadURL));
		downloadURL[sizeof(downloadURL) - 1] = '\0';
	}
	else
		downloadURL[0] = '\0';

	// Call the callback.
	/////////////////////
	ptaCallPatchCallback(data, PTTrue, mandatory, versionName, fileID, downloadURL);

	return GHTTPTrue;
}

// RETAIL: 0x1004f8d0
static const char * ptaGetKeyValue
(
	const char * buffer,
	const char * key
)
{
	static char value[PTA_MAX_STRING_SIZE];	// RETAIL: 0x1026e02c
	const char * str;
	unsigned int len;

	str = strstr(buffer, key);
	if(!str)
		return NULL;
	str += strlen(key);
	len = strcspn(str, "\\");
	if(len >= (sizeof(value) - 1))
		len = (sizeof(value) - 1);
	memcpy(value, str, len);
	value[len] = '\0';

	return value;
}

// RETAIL: 0x1004f940
static void ptaCallPatchCallback
(
	ptaPatchData * data,
	PTBool available,
	PTBool mandatory,
	const char * versionName,
	int fileID,
	const char * downloadURL
)
{
	if(data->callback)
		data->callback(available, mandatory, versionName, fileID, downloadURL, data->param);

	free(data);
}

// already returns ghttptrue
// RETAIL: 0x1004f980
static GHTTPBool ptaPatchFailed
(
	ptaPatchData * data
)
{
	ptaCallPatchCallback(data, PTFalse, PTFalse, "", 0, "");

	return GHTTPTrue;
}

// RETAIL: 0x1004f9b0
PTBool ptCheckForPatchAndTrackUsage
(
	int userID,
	int productID,
	const char * versionUniqueID,
	int distributionID,
	ptPatchCallback callback,
	PTBool blocking,
	void * param
)
{
	ptaPatchData * data;

	// Check the arguments.
	///////////////////////
	if(!versionUniqueID)
		return PTFalse;
	if(!callback)
		return PTFalse;

	// Store some data.
	///////////////////
	data = (ptaPatchData *)malloc(sizeof(ptaPatchData));
	if(!data)
		return PTFalse;
	memset(data, 0, sizeof(ptaPatchData));
	data->callback = callback;
	data->param = param;

	// Build the URL.
	/////////////////
	sprintf(URL, "http://motd.gamespy.com/motd/vercheck.asp?userid=%d&productid=%d&versionuniqueid=%s&distid=%d&uniqueid=%s",
		userID, productID, versionUniqueID, distributionID, GOAGetUniqueID());

	// Send the request.
	////////////////////
	if((ghttpGetFileA(URL, (GHTTPBool)blocking, ptaPatchCompletedCallback, data) == -1) && !blocking)
		return PTFalse;

	return PTTrue;
}
