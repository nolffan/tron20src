// ----------------------------------------------------------------------- //
//
// MODULE  : ClientInfoMgr.cpp
//
// PURPOSE : Manages and displays player info for multiplayer games
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientInfoMgr.h"
#include "iltclient.h"
#include "GameClientShell.h"
#include <stdio.h>

#define VERT_SPACING 3

extern CGameClientShell* g_pGameClientShell;

namespace
{
	char szUnknownPlayer[32] = "";
	const int kNumTeams = 2;
}


CClientInfoMgr::CClientInfoMgr()
{
    m_pClients = LTNULL;
}

CClientInfoMgr::~CClientInfoMgr()
{
    CLIENT_INFO* ptr = LTNULL;
	while (m_pClients)
	{
		ptr = m_pClients->pNext;
		debug_delete(m_pClients);
		m_pClients = ptr;
	}

}

void CClientInfoMgr::Init()
{

	// RETAIL: 0x100fc9d0 loads string 611 (0x263); Tron's CRes number of
	// IDS_UNKNOWN_PLAYER differs from NOLF2's
	LoadString(611,szUnknownPlayer,sizeof(szUnknownPlayer) );

	m_nLocalID = 0;
}

// RETAIL: 0x100fc9f0. Tron's keeps no admin flag or team here and does
// not tell the radar.
void CClientInfoMgr::UpdateClient ( char const* pszName, uint32 nID )
{
	// if we have this client in the list, update their info
	CLIENT_INFO* pClient = GetClientByID(nID, false);
	if (pClient)
	{
		pClient->sName = pszName;

		UpdateClientSort(pClient);
	}
	else
	{
		//hmmm... we should have found them...
		g_pLTClient->CPrint("CClientInfoMgr::UpdateClient () : Unknown Client %d (%s)", nID, pszName);
	}
}

// RETAIL: 0x100fcac0. Tron's replaces a duplicate client instead of
// updating it; NOLF2's PlayerConnected is not in Tron.
void CClientInfoMgr::AddClient ( char const* pszName, uint32 nID)
{
	g_pHUDMgr->QueueUpdate(kHUDTronUnknown08000000);
	g_pHUDMgr->QueueUpdate(kHUDTronUnknown01000000);

	m_nLocalID = 0;
	g_pLTClient->GetLocalClientID (&m_nLocalID);

	// if we already have this client in the list, take the old one away
	CLIENT_INFO* pDup = GetClientByID(nID, false);
	if (pDup)
	{
		g_pLTClient->CPrint("CClientInfoMgr::AddClient () : Duplicate Client %d (%s)", nID, pszName);
		RemoveClient(nID);
	}

	// create the new object

	CLIENT_INFO* pNew = debug_new(CLIENT_INFO);
	if (!pNew) return;

	pNew->nID = nID;
	pNew->sName = pszName;

	pNew->sScore.Init(nID);

	// if we don't have a list yet, set the list pointer to the new object
	if (!m_pClients)
	{
		m_pClients = pNew;
		return;
	}

	// we do have a list - insert the object
	CLIENT_INFO* ptr = m_pClients;

	//insert at head?
	if (ptr->sScore.GetScore() < pNew->sScore.GetScore())
	{
		pNew->pNext = m_pClients;
		m_pClients = pNew;
		return;
	}

	CLIENT_INFO* pNext = ptr->pNext;
	while (pNext && pNext->sScore.GetScore() >= pNew->sScore.GetScore())
	{
		ptr = pNext;
		pNext = ptr->pNext;
	}
	if (pNext)
	{
		pNext->pPrev = pNew;
	}
	ptr->pNext = pNew;
	pNew->pNext = pNext;
	pNew->pPrev = ptr;

}

// RETAIL: 0x100fcc20 (no radar update in Tron)
void CClientInfoMgr::RemoveClient (uint32 nID)
{
	if (!m_pClients) return;

	m_nLocalID = 0;
	g_pLTClient->GetLocalClientID (&m_nLocalID);

	// find the client

	CLIENT_INFO* ptr = GetClientByID(nID, false);
	if (!ptr) return;

	// remove the client from the list

	if (ptr->pNext) 
		ptr->pNext->pPrev = ptr->pPrev;
	if (ptr->pPrev) 
		ptr->pPrev->pNext = ptr->pNext;
	if (m_pClients == ptr) 
		m_pClients = ptr->pNext;

	debug_delete(ptr);

	g_pHUDMgr->QueueUpdate(kHUDTronUnknown08000000);
	g_pHUDMgr->QueueUpdate(kHUDTronUnknown01000000);
}

// RETAIL: 0x100fccb0
void CClientInfoMgr::RemoveAllClients()
{

    CLIENT_INFO* ptr = LTNULL;
	while (m_pClients)
	{
		ptr = m_pClients->pNext;
		debug_delete(m_pClients);
		m_pClients = ptr;
	}

	g_pHUDMgr->QueueUpdate(kHUDTronUnknown08000000);
	g_pHUDMgr->QueueUpdate(kHUDTronUnknown01000000);
}

void CClientInfoMgr::UpdateClientSort(CLIENT_INFO* pCur)
{
	if (!pCur) return;

	// RETAIL: 0x100fcd00
	g_pHUDMgr->QueueUpdate(kHUDTronUnknown08000000);
	g_pHUDMgr->QueueUpdate(kHUDTronUnknown01000000);

	// put this client in the correct position in the list (most frags to least frags)
	CLIENT_INFO* pTmp = pCur;

	LTBOOL bBack = LTFALSE;
	//try going forward
	while (pTmp->pPrev && pCur->sScore.GetScore() > pTmp->pPrev->sScore.GetScore())	
		pTmp = pTmp->pPrev;
	if (pTmp == pCur)
	{
		//try going backward
		while (pTmp->pNext && pCur->sScore.GetScore() < pTmp->pNext->sScore.GetScore())	
			pTmp = pTmp->pNext;
		if (pTmp == pCur) return;
		bBack = LTTRUE;

	}

	// we found a new position - remove current from the list
	if (pCur->pPrev)
		pCur->pPrev->pNext = pCur->pNext;
	else
		m_pClients = pCur->pNext;
	if (pCur->pNext) 
		pCur->pNext->pPrev = pCur->pPrev;

	// put us back in in the correct position
	if (bBack)
	{
		pCur->pPrev = pTmp;
		pCur->pNext = pTmp->pNext;
		pTmp->pNext = pCur;
		if (pCur->pNext)
			pCur->pNext->pPrev = pCur;
	}
	else
	{
		pCur->pPrev = pTmp->pPrev;
		pCur->pNext = pTmp;
		if (pTmp->pPrev) 
			pTmp->pPrev->pNext = pCur;
		else
			m_pClients = pCur;
		pTmp->pPrev = pCur;
	}


}


uint32 CClientInfoMgr::GetNumClients()
{
	if (!m_pClients) return 0;

	CLIENT_INFO* ptr = m_pClients;

    uint32 nCount = 0;
	while (ptr)
	{
		nCount++;
		ptr = ptr->pNext;
	}

	return nCount;
}

char const* CClientInfoMgr::GetPlayerName (uint32 nID)
{
	// RETAIL: 0x100fcde0 gives the unknown player's name when there are
	// no clients
    if (m_pClients)
	{
		CLIENT_INFO* ptr = GetClientByID(nID);
		if (ptr)
			return ptr->sName.c_str( );
	}

	return szUnknownPlayer;
}


CLIENT_INFO* CClientInfoMgr::GetLocalClient()
{
	return GetClientByID(m_nLocalID);
}

CLIENT_INFO* CClientInfoMgr::GetClientByID(uint32 nID, bool bUpdateOnFailure)
{
	CLIENT_INFO* ptr = m_pClients;
	while (ptr)
	{
		if (ptr->nID == nID)
			return ptr;

		ptr = ptr->pNext;
	}
	if (bUpdateOnFailure)
		g_pInterfaceMgr->UpdateClientList();
    return LTNULL;
}
