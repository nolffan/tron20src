// ----------------------------------------------------------------------- //
//
// MODULE  : TronClientMultiplayerMgr.cpp
//
// PURPOSE : Tron's client multiplayer manager
//
// ----------------------------------------------------------------------- //

// RETAIL: written from retail CShell.dll (0x10096ac0..0x10096fef). Tron's
// handshake answers the server's password message itself: it proves the
// client's files (each CRC once, then kept) and answers the server's CD
// key challenge (GameSpy's gcdkey). Contract J.A.C.K.'s handshake
// (cjacksrc) is the model of the key "encryption".

#include "stdafx.h"
#include "TronClientMultiplayerMgr.h"
#include "MsgIDs.h"
#include "CRC32.h"
#include "WeaponMgr.h"
#include "ModelButeMgr.h"
#include "SurfaceMgr.h"
#include "DamageFXMgr.h"
#include "FXButeMgr.h"
#include "SubroutineMgr.h"
#include "LightCycleButeMgr.h"
#include "ProfileMgr.h"
#include "GameClientShell.h"
#include "TronGameClientShell.h"
#include "TronCDKey.h"

#include "..\..\Libs\GameSpy\GameSpySDK\gcdkey\gcdkeyc.h"

// Global key used for cross-encryption  NOTE: MUST BE SAME AS ON SERVER!!
#define KEY_MESSER_UPPER 0xDEADFACE

inline uint32 MessUp32BitValue(uint32 nValue, uint32 nRot)
{
	// "Encrypt" the value (Yes, it's a very simple encryption...)
	uint32 nXORShift = nRot & 0xF;
	return ((KEY_MESSER_UPPER << nXORShift) + (KEY_MESSER_UPPER >> (32 - nXORShift))) ^ nValue;
}

// RETAIL: 0x10096ac0: the password message here (the CD key challenge, 8
// bytes, comes first); a bad CD key disconnects (sub-message 5); the
// others are ClientMultiplayerMgr's
bool TronClientMultiplayerMgr::HandleMsgHandshake( ILTMessage_Read & msg )
{
	int nHandshakeSub = (int)msg.Peekuint8();
	if (nHandshakeSub == MID_HANDSHAKE_PASSWORD)
	{
		msg.Readuint8();

		// The server's CD key challenge
		char szChallenge[9];
		msg.ReadData(szChallenge, 64);
		szChallenge[8] = 0;

#ifdef TRON20SRC_MODERN
		// MODERN (GS-4): no CD key. A modern server checks none, and the
		// answer (an MD5 of the key) would identify the player to every
		// server. The answer is made from an empty key: the message keeps
		// its form.
		char szResponse[73];
		gcd_compute_response("", szChallenge, szResponse);
#else
		CTronCDKey* pCDKey = ((CTronGameClientShell*)g_pGameClientShell)->GetCDKey();
		std::string sCDKey = pCDKey->GetCDKey();
		char szResponse[73];
		gcd_compute_response((char*)sCDKey.c_str(), szChallenge, szResponse);
#endif

		// Read in their key
		m_nServerKey = msg.Readuint32();
		bool   bNeedPassword = msg.Readbool();

		// Game password.
		uint32 nHashedPassword = 0;
		if (bNeedPassword)
		{
			nHashedPassword = m_nClientPass;
		}

		// Get the player name
		char sName[255];
		// (Make sure to clear out the first 4 characters, since that's all we care about)
		sName[0] = sName[1] = sName[2] = sName[3] = 0;
		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

		SAFE_STRCPY(sName,pProfile->m_sPlayerGuid.c_str( ));

		// Decrypt it
		m_nServerKey ^= MessUp32BitValue((uint32)this, (uint32)(sName[0]));

		// Encrypt "the string" with it
		uint32 nPassword = *((uint32*)sName);
		uint32 nXORMask = MessUp32BitValue(m_nServerKey, (uint32)(sName[0]));

		nPassword ^= nXORMask;

		// Get the weapons file CRC
		uint32 nWeaponCRC = g_pWeaponMgr->GetFileCRC();
		// Mask that up too
		nWeaponCRC ^= nXORMask;

		// Get the client shell file CRC
		char aClientShellName[MAX_PATH + 1];
		// Just in case getting the file name fails
		aClientShellName[0] = 0; 
		// Get the client shell handle from the engine
		HMODULE hClientShell;
		g_pLTClient->GetEngineHook("cshell_hinstance", (void**)&hClientShell);
		DWORD nResult = GetModuleFileName(hClientShell, aClientShellName, sizeof(aClientShellName));
		static uint32 nClientCRC = CRC32::CalcFileCRC(aClientShellName);
		uint32 nClientMaskedCRC = nClientCRC ^ nXORMask;

		// CRC the attribute files
		static uint32 nModelButesCRC = CRC32::CalcRezFileCRC( g_pModelButeMgr->GetAttributeFile( ));
		static uint32 nSurfaceCRC = CRC32::CalcRezFileCRC( g_pSurfaceMgr->GetAttributeFile( ));
		static uint32 nDamageFxCRC = CRC32::CalcRezFileCRC( g_pDamageFXMgr->GetAttributeFile( ));
		static uint32 nFXButesCRC = CRC32::CalcRezFileCRC( g_pFXButeMgr->GetAttributeFile( ));
		static uint32 nSubroutinesCRC = CRC32::CalcRezFileCRC( g_pSubroutineMgr->GetAttributeFile( ));
		static uint32 nLightCycleCRC = CRC32::CalcRezFileCRC( g_pLightCycleButeMgr->GetAttributeFile( ));

		uint32 nModelButesMaskedCRC = nModelButesCRC ^ nXORMask;
		uint32 nSurfaceMaskedCRC = nSurfaceCRC ^ nXORMask;
		uint32 nDamageFxMaskedCRC = nDamageFxCRC ^ nXORMask;
		uint32 nFXButesMaskedCRC = nFXButesCRC ^ nXORMask;
		uint32 nSubroutinesMaskedCRC = nSubroutinesCRC ^ nXORMask;
		uint32 nLightCycleMaskedCRC = nLightCycleCRC ^ nXORMask;

		// Send it back their direction
		CAutoMessage cResponse;
		cResponse.Writeuint8(MID_HANDSHAKE);
		cResponse.Writeuint8(MID_HANDSHAKE_LETMEIN);
		cResponse.Writeuint32(nPassword);
		cResponse.Writeuint32(nWeaponCRC);
		cResponse.Writeuint32(nClientMaskedCRC);
		cResponse.Writeuint32(nModelButesMaskedCRC);
		cResponse.Writeuint32(nSurfaceMaskedCRC);
		cResponse.Writeuint32(nDamageFxMaskedCRC);
		cResponse.Writeuint32(nFXButesMaskedCRC);
		cResponse.Writeuint32(nSubroutinesMaskedCRC);
		cResponse.Writeuint32(nLightCycleMaskedCRC);
		cResponse.Writeuint32(nHashedPassword);
		cResponse.WriteData(szChallenge, 64);
		cResponse.WriteData(szResponse, 576);
		g_pLTClient->SendToServer(cResponse.Read(), MESSAGE_GUARANTEED);

		return true;
	}
	else if (nHandshakeSub == 5)
	{
		// RETAIL: the server rejected the CD key (the sub-message and
		// the disconnect code have no names in the tree)
		m_bForceDisconnect = true;
		SetDisconnectCode(3, NULL);
		return true;
	}

	return ClientMultiplayerMgr::HandleMsgHandshake(msg);
}
