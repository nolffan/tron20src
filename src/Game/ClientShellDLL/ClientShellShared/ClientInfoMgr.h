#ifndef __CLIENTINFOMGR_H
#define __CLIENTINFOMGR_H

#include "ltbasedefs.h"
#include "ClientUtilities.h"
#include "SharedMission.h"
#include "SharedScoring.h"

class ILTClient;


struct CLIENT_INFO
{
    CLIENT_INFO()   { nPing = 0; nID = 0; pPrev = LTNULL; pNext = LTNULL; }

	uint16			nPing;
    uint32          nID;
	std::string		sName;
	MissionStats	sStats;
	CPlayerScore	sScore;
	// RETAIL: Tron's CLIENT_INFO is 0x74 bytes (0x100fce50): it has no
	// bIsAdmin and no nTeamID. The team of a client is in the team
	// manager (CTronTeamMgr::GetTeamOfClient).

	CLIENT_INFO*	pPrev;
	CLIENT_INFO*	pNext;
};

class CClientInfoMgr
{
public:

	CClientInfoMgr();
	~CClientInfoMgr();

    void    Init ();

    // RETAIL: Tron's take only the name and the id (0x100fcac0, 0x100fc9f0)
    void    AddClient ( char const* pszName, uint32 nID);
    void    UpdateClient ( char const* pszName, uint32 nID);
    void    RemoveClient (uint32 nID);
	void	RemoveAllClients();

	CLIENT_INFO* GetLocalClient();
	CLIENT_INFO* GetFirstClient() {return m_pClients;}
    CLIENT_INFO* GetClientByID(uint32 nID, bool bUpdateOnFailure = true);

    uint32  GetNumClients();
    char const* GetPlayerName (uint32 nID);

	void	UpdateClientSort(CLIENT_INFO* pCur);


protected:


	CLIENT_INFO*		m_pClients;

	uint32				m_nLocalID;

};

#endif