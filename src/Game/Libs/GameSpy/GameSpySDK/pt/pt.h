/*
GameSpy PT SDK
Dan "Mr. Pants" Schoenblum
dan@gamespy.com

Copyright 2000 GameSpy Industries, Inc

RETAIL: Tron 2.0 links only the patch check of the 2002 PT SDK
(CShell.dll 0x1004f770..0x1004fa4c); this header declares what it has.
*/

#ifndef _PT_H_
#define _PT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********
** TYPES **
**********/
typedef int PTBool;
#define PTFalse 0
#define PTTrue  1

// This callback gets called when a patch is being checked for
// with ptCheckForPatchAndTrackUsage.
///////////////////////////////////////////////////////////////
typedef void (* ptPatchCallback)
(
	PTBool available,         // If PTTrue, a patch is available.
	PTBool mandatory,         // If PTTrue, this patch is flagged as being mandatory.
	const char * versionName, // The display name for the new version.
	int fileID,               // The FilePlanet fileID for the patch, or 0.
	const char * downloadURL, // A URL to download the patch from, or NULL.
	void * param              // User-data passed originally passed to the function.
);

/**************
** FUNCTIONS **
**************/

// Check for a patch for the current version and particular
// distribution of the product, and track the usage of it.
// RETAIL: 0x1004f9b0
PTBool ptCheckForPatchAndTrackUsage
(
	int userID,                   // The GP userID of the player who is using the product.  Can be 0.
	int productID,                // The ID of this product.
	const char * versionUniqueID, // A string uniquely identifying this version.
	int distributionID,           // The distribution ID for this version.  Can be 0.
	ptPatchCallback callback,     // The callback to call with the patch info.
	PTBool blocking,              // If PTTrue, don't return until after the callback is called.
	void * param                  // User-data to pass to the callback.
);

#ifdef __cplusplus
}
#endif

#endif
