// ----------------------------------------------------------------------- //
//
// MODULE  : TronVersionMgr.cpp
//
// PURPOSE : Implementation of versioning manager
//
// CREATED : 11/16/2000
//
// (c) 2000-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronVersionMgr.h"

// RETAIL: v1.030's version and build, as its CShell.dll and Object.lto
// have them (Build 69's file is May 2002's: "TRON Build 20 (05/09/02) DO
// NOT DISTRIBUTE", "Build 20 (05/09/02)").
static const char* s_szVersion		= "v1.030";
static const char* s_szBuild		= "Build 74";
static const uint32 s_nSaveVersion  = 0;
// PORT: May 2002's; retail has no "1.0.0.0": to be read from its code
static const char* s_szNetVersion	= "1.0.0.0";
// PORT: NOLF2's CVersionMgr has these two, pure virtual. The name is Tron's
// GameSpy game name, which retail CShell.dll and Object.lto both carry; the
// build guid is to be read from the retail code.
static const char* s_szNetGameName	= "tron20";
static const LTGUID s_BuildGuid		= { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

// PORT: what NOLF2's TO2VersionMgr.cpp defines for the shared code. The
// name is for folder paths and registry entries: retail CShell.dll and
// Object.lto have "TRON 2.0". The handshake version is to be read from the
// retail code. The save versions are the gates NOLF2's 1.1-1.3 updates put
// in the shared code's save and load: with all at Tron's save version, 0,
// every save takes the newest path.
char const GAME_NAME[] = "TRON 2.0";
int const GAME_HANDSHAKE_VER = 0;
const CVersionMgr::TSaveVersion CVersionMgr::kSaveVersion__1_1 = 0;
const CVersionMgr::TSaveVersion CVersionMgr::kSaveVersion__1_2 = 0;
const CVersionMgr::TSaveVersion CVersionMgr::kSaveVersion__1_3 = 0;
const CVersionMgr::TSaveVersion CVersionMgr::kSaveVersion__CurrentBuild = s_nSaveVersion;

CVersionMgr* g_pVersionMgr = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronVersionMgr::CTronVersionMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTronVersionMgr::CTronVersionMgr() : CVersionMgr()
{
	g_pVersionMgr = this;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronVersionMgr::GetVersion()
//
//	PURPOSE:	Get the current version string
//
// ----------------------------------------------------------------------- //

const char* CTronVersionMgr::GetVersion()
{
	return s_szVersion;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronVersionMgr::GetSaveVersion()
//
//	PURPOSE:	Get the current save version 
//
// ----------------------------------------------------------------------- //

const uint32 CTronVersionMgr::GetSaveVersion()
{
	return s_nSaveVersion;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronVersionMgr::GetBuild()
//
//	PURPOSE:	Get the current build string
//
// ----------------------------------------------------------------------- //

const char* CTronVersionMgr::GetBuild()
{
	return s_szBuild;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronVersionMgr:GetNetVersion()
//
//	PURPOSE:	Get the current net version 
//
// ----------------------------------------------------------------------- //

const char* CTronVersionMgr::GetNetVersion()
{
	return s_szNetVersion;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronVersionMgr::GetNetGameName()
//
//	PURPOSE:	Get the net game name.  (PORT: NOLF2's interface)
//
// ----------------------------------------------------------------------- //

const char* CTronVersionMgr::GetNetGameName()
{
	return s_szNetGameName;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronVersionMgr::GetBuildGuid()
//
//	PURPOSE:	Gets a guid based on a build.  (PORT: NOLF2's interface)
//
// ----------------------------------------------------------------------- //

const LTGUID* CTronVersionMgr::GetBuildGuid()
{
	return &s_BuildGuid;
}
