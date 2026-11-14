// ----------------------------------------------------------------------- //
//
// MODULE  : ClientSoundMgr.h
//
// PURPOSE : ClientSoundMgr definition - Controls sound on the client
//
// CREATED : 7/10/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_SOUND_MGR_H__
#define __CLIENT_SOUND_MGR_H__

#include "SoundMgr.h"

class CClientSoundMgr;
extern CClientSoundMgr* g_pClientSoundMgr;

#define CSNDMGR_DEFAULT_FILE		"Attributes\\ClientSnd.txt"

class CClientSoundMgr : public CGameSoundMgr
{
	public :

		CClientSoundMgr();
		~CClientSoundMgr();

        virtual LTBOOL	Init(const char* szAttributeFile=CSNDMGR_DEFAULT_FILE);
		virtual void	Term();

        // RETAIL: Tron's PlaySoundLocal takes a 7th argument (CShell 0x100fa810,
        // ret 0x1c): the volume scale of CGameSoundMgr::PlaySoundFromObject;
        // retail's callers push 1.0f.  The name is ours.
        HLTSOUND	PlaySoundLocal(const char *pName, SoundPriority ePriority=SOUNDPRIORITY_MISC_LOW,
            uint32 dwFlags=0, uint8 nVolume=SMGR_DEFAULT_VOLUME, float fPitchShift=1.0f, 
			uint8 nSoundClass = 0, float fVolumeScale = 1.0f );
 
		HLTSOUND	PlayInterfaceSound(const char *pName, uint32 dwFlags=0);

		virtual	LTVector GetObjectPos(HOBJECT hObj)
		{
			LTVector vPos(0, 0, 0);
			if (hObj)
			{
				g_pLTClient->GetObjectPos(hObj, &vPos);
			}

			return vPos;
		}

		virtual	HLTSOUND PlaySound(PlaySoundInfo & playSoundInfo);

		// RETAIL: Tron's (retail 0x100fab80; the name is ours)
		void	SetSoundFilter(uint8 nFilterId);

};

#endif // __CLIENT_SOUND_MGR_H__