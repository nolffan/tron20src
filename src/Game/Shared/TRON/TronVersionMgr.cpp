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

// RETAIL: the game's guid, which NOLF2's TO2VersionMgr.cpp defines for the
// shared code (CGameClientShell::OnEngineInitialized gives it to the
// engine): retail CShell.dll copies it from 0x1021ea20, and the same 16
// bytes are in Object.lto and TRONSrv.exe, once each, first in this file's
// data.
// {FC272B66-8372-49EF-A642-28CAD2B9EAC9}
LTGUID GAMEGUID =
{ 0xfc272b66, 0x8372, 0x49ef, { 0xa6, 0x42, 0x28, 0xca, 0xd2, 0xb9, 0xea, 0xc9 } };

// RETAIL: v1.030's version, net version and build, after the guid in every
// image (CShell.dll 0x1021ea30, the getters 0x100afb50..0x100afb70; the
// developer build's TRONSrv.exe has "v1.010", "TRON 10", "Build 72").
// Build 69's file is May 2002's: "TRON Build 20 (05/09/02) DO NOT
// DISTRIBUTE", "Build 20 (05/09/02)".
static const char* s_szVersion		= "v1.030";
static const char* s_szNetVersion	= "TRON 12";
static const char* s_szBuild		= "Build 74";
static const uint32 s_nSaveVersion  = 0;

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
