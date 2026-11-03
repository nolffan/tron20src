// ----------------------------------------------------------------------- //
//
// MODULE  : VersionMgr.h
//
// PURPOSE : Definition of versioning manager
//
// CREATED : 11/16/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VERSION_MGR_H__
#define __VERSION_MGR_H__

#include "ltbasetypes.h"
#include "regmgr.h"

extern LTGUID GAMEGUID;

extern char const GAME_NAME[];
extern int const GAME_HANDSHAKE_VER_MAJOR;
extern int const GAME_HANDSHAKE_VER_MINOR;
extern int const GAME_HANDSHAKE_VER;



class CVersionMgr
{
	public :

		CVersionMgr();
		virtual ~CVersionMgr() {}

		// RETAIL: Tron's CVersionMgr has five virtuals (its destructor, GetVersion,
		// GetBuild, GetSaveVersion, GetNetVersion: retail's vtables): NOLF2's
		// UpdateCT, GetNetGameName, GetNetRegion and GetBuildGuid came after it

		virtual const char* GetVersion() = 0;
		virtual const char* GetBuild() = 0;
		virtual const uint32 GetSaveVersion() = 0;
		virtual const char* GetNetVersion() = 0;
		const char* GetNetRegion() { return m_szNetRegion; }	// RETAIL: not virtual (above)


		const char* GetLanguage()	const { return m_szLanguage; }

		//low violence needs to be implemented on clientside only
#ifdef _CLIENTBUILD
		inline bool	IsLowViolence() const { return m_bLowViolence; }
#endif

		CRegMgr*	GetRegMgr()		{ return &m_regMgr; }

		typedef uint32 TSaveVersion;

		// The SaveVersions should be set to the build number of the released version.  The CurrentBuild
		// should always be set to the curent development build number.  The latest save version should also 
		// equal the CurrentBuild.

		static const TSaveVersion	kSaveVersion__1_1;
		static const TSaveVersion	kSaveVersion__1_2;
		static const TSaveVersion	kSaveVersion__1_3;
	private:
		static const TSaveVersion	kSaveVersion__CurrentBuild;
	public:

		void			SetCurrentSaveVersion( TSaveVersion nCurrentSaveVersion ) { m_nCurrentSaveVersion = nCurrentSaveVersion; }
		TSaveVersion	GetCurrentSaveVersion( ) const;

		void			SetLGFlags( uint8 nLGFlags ) { m_nLastLGFlags = nLGFlags; }

	protected :
		CRegMgr	m_regMgr;
		char	m_szLanguage[64];
		char	m_szNetRegion[64];
		bool	m_bLowViolence;

		// This is the save version of the currently loading save game.
		// This number is only valid during a ILTServer::RestoreObjects call.
		TSaveVersion	m_nCurrentSaveVersion;

		uint8			m_nLastLGFlags;
};

extern CVersionMgr* g_pVersionMgr;

#endif __VERSION_MGR_H__