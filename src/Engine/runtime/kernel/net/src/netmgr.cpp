

#include "bdefs.h"
#include "netmgr.h"
#include "systimer.h"
#include "sysdebugging.h"
#include "s_client.h"

#include "localdriver.h"
#include "sysudpdriver.h"

#include <algorithm>


extern int32 g_CV_ShowConnStats;
extern int32 g_TransportDebug;
extern int32 g_bLocalDebug;
extern float g_CV_LatencySim;
extern float g_CV_DropRate;
extern int32 g_CV_ParseNet_Incoming;
extern int32 g_CV_ParseNet_Outgoing;
extern int32 g_CV_ParseNet;

// Just used for the timestamp in debug output (this is a global so when debugging
// local games, the time frame is the same between client and server).
Counter g_NMTimeCounter(CSTART_MILLI);




// ------------------------------------------------------------------------ //
// Helpers
// ------------------------------------------------------------------------ //

// TRON: the packet parser is in the final build too (server.dll 0x10021a76,
// ParsePacket; ParseNet's console variables are its). Tron's is not the
// 2003/2006 Jupiter one (cjacksrc's, NOLF2's, Build 69's), which takes each
// object update apart: it names a message by its game message ID
// (0x10021945) and an update's change flags by their whole names, and has
// the network protocol's older packet IDs (0xF..0x16, 0x32, 0x36).

// RETAIL: server.dll 0x10021945, the game's message names (NOLF's MID_s,
// TRON's few of its own); the name is ours
static const char* GetMsgName(uint32 nMsgID)
{
	switch (nMsgID)
	{
		case 100 : return "PLAYER_UPDATE";
		case 101 : return "PLAYER_SKILLS";
		case 102 : return "PLAYER_SUMMARY";
		case 103 : return "PLAYER_INFOCHANGE";
		case 104 : return "PLAYER_SCORE";
		case 105 : return "PLAYER_STATE_CHANGE";
		case 106 : return "PLAYER_MESSAGE";
		case 107 : return "PLAYER_CHEAT";
		case 108 : return "PLAYER_ORIENTATION";
		case 109 : return "PLAYER_INFO";
		case 110 : return "PLAYER_REMOVED";
		case 111 : return "PLAYER_SCORED";
		case 112 : return "PLAYER_RESPAWN";
		case 113 : return "PLAYER_CANCELREVIVE";
		case 114 : return "PLAYER_MULTIPLAYER_INIT";
		case 115 : return "PLAYER_SINGLEPLAYER_INIT";
		case 116 : return "PLAYER_LOADCLIENT";
		case 117 : return "PLAYER_DAMAGE";
		case 118 : return "PLAYER_ACTIVATE";
		case 130 : return "WEAPON_CHANGE";
		case 212 : return "CLIENT_PLAYER_UPDATE";
		case 214 : return "PINGTIMES";
		case 231 : return "SFX_MESSAGE";
		default :
		{
			static char aBuffer[256];
			LTSNPrintF(aBuffer, sizeof(aBuffer), "%d", nMsgID);
			return aBuffer;
		}
	}
}

void ParsePacket(const CPacket_Read &cPacket, bool bFirst, char *aMsgBuff, uint32 nBufferSize, uint8 nTravelDir = NETMGR_TRAVELDIR_UNKNOWN)
{
	// Parse the incoming packet...
	if(!aMsgBuff) 
		return;

	CPacket_Read cParsePacket(cPacket);


	// We need a buffer to stuff this into..
	char aTemp[256];

	if(bFirst)
	{
		// RETAIL: clears the pointer's size, not the buffer's (as Jupiter's)
		memset(aMsgBuff, 0, sizeof(aMsgBuff));
	}

	switch(nTravelDir)
	{
		case NETMGR_TRAVELDIR_SERVER2CLIENT:
			LTStrCat(aMsgBuff, (bFirst) ? "<S2C>" : "<s2c>", nBufferSize);
			break;

		case NETMGR_TRAVELDIR_CLIENT2SERVER:
			LTStrCat(aMsgBuff, (bFirst) ? "<C2S>" : "<c2s>", nBufferSize);
			break;

		case NETMGR_TRAVELDIR_UNKNOWN:
		default:
			LTStrCat(aMsgBuff, (bFirst) ? "<U2U>" : "<u2u>", nBufferSize);
			break;
	}

	uint32 nPacketID = cParsePacket.Readuint8();

	// NetMgr layer
	switch(nPacketID)
	{
		case 4 : //SMSG_NETPROTOCOLVERSION
			LTStrCat(aMsgBuff, "(NPV) ", nBufferSize);
			break;

		case 5 : //SMSG_UNLOADWORLD
			LTStrCat(aMsgBuff, "(ULW) ", nBufferSize);
			break;

		case 6 : //SMSG_LOADWORLD
			LTStrCat(aMsgBuff, "(LW )  ", nBufferSize);
			break;

		case 7 : //SMSG_CLIENTOBJECTID
			LTStrCat(aMsgBuff, "(COI) ", nBufferSize);
			break;

		case 8 : //SMSG_UPDATE
		{
			LTSNPrintF(aTemp, sizeof(aTemp), "(UPD) |%d| ", cParsePacket.TellEnd());
			LTStrCat(aMsgBuff, aTemp, nBufferSize);

			while (!cParsePacket.EOP())
			{
				// Each object's update is preceded by its length
				uint32 nUpdateLen = cParsePacket.Readuint32();
				uint32 nUpdateEnd = cParsePacket.Tell() + nUpdateLen;

				uint32 nFlags = (uint32)cParsePacket.Readuint8();
				if(nFlags & (1<<7)) //CF_OTHER
					nFlags |= (uint32)cParsePacket.Readuint8() << 8;

				uint16 nObjectID = cParsePacket.Readuint16();
				LTSNPrintF(aTemp, sizeof(aTemp), "\n    %04x ", nObjectID);
				LTStrCat(aMsgBuff, aTemp, nBufferSize);

				if (nFlags & (1<<0))
					LTStrCat(aMsgBuff, "NEWOBJECT ", nBufferSize);
				if (nFlags & (1<<1))
					LTStrCat(aMsgBuff, "POSITION ", nBufferSize);
				if (nFlags & (1<<2))
					LTStrCat(aMsgBuff, "ROTATION ", nBufferSize);
				if (nFlags & (1<<3))
					LTStrCat(aMsgBuff, "FLAGS ", nBufferSize);
				if (nFlags & (1<<4))
					LTStrCat(aMsgBuff, "SCALE ", nBufferSize);
				if (nFlags & (1<<5))
					LTStrCat(aMsgBuff, "MODELINFO ", nBufferSize);
				if (nFlags & (1<<6))
					LTStrCat(aMsgBuff, "RENDERINFO ", nBufferSize);
				if (nFlags & (1<<8))
					LTStrCat(aMsgBuff, "ATTACHMENTS ", nBufferSize);
				if (nFlags & (1<<9))
					LTStrCat(aMsgBuff, "TELEPORT ", nBufferSize);
				if (nFlags & (1<<10))
					LTStrCat(aMsgBuff, "SNAPROTATION ", nBufferSize);
				if (nFlags & (1<<11))
					LTStrCat(aMsgBuff, "FILENAMES ", nBufferSize);
				if (nFlags & (1<<13))
					LTStrCat(aMsgBuff, "FORCEMODELINFO ", nBufferSize);
				if (nFlags & (1<<14))
					LTStrCat(aMsgBuff, "PREDICTIONCAP ", nBufferSize);
				if (nFlags & (1<<15))
					LTStrCat(aMsgBuff, "DIMS ", nBufferSize);

				cParsePacket.SeekTo(nUpdateEnd);
			}
			break;
		}

		case 0xA : //SMSG_UNGUARANTEEDUPDATE
		{
			LTSNPrintF(aTemp, sizeof(aTemp), "(UUP) |%d| ", cParsePacket.TellEnd());
			LTStrCat(aMsgBuff, aTemp, nBufferSize);
			break;
		}

		case 0xC : //SMSG_YOURID
			LTStrCat(aMsgBuff, "(YID) ", nBufferSize);
			break;

		case 0xB : //CMSG_MESSAGE
		case 0xD : //SMSG_MESSAGE
		{
			LTStrCat(aMsgBuff, "(MSG) ", nBufferSize);
			if (cParsePacket.TellEnd() < 8)
			{
				LTStrCat(aMsgBuff, "BAD ", nBufferSize);
				break;
			}
			uint8 nMsgID = cParsePacket.Readuint8();
			LTSNPrintF(aTemp, sizeof(aTemp), "%s |%d| ", GetMsgName(nMsgID), cParsePacket.TellEnd());
			LTStrCat(aMsgBuff, aTemp, nBufferSize);
			break;
		}

		case 0xE : //SMSG_PACKETGROUP
			LTStrCat(aMsgBuff, "(GRP) ", nBufferSize);
			break;

		case 0xF : //SMSG_CONSOLEVAR
			LTStrCat(aMsgBuff, "(CV ) ", nBufferSize);
			break;

		case 0x10 : //SMSG_SKYDEF
			LTStrCat(aMsgBuff, "(SKY) ", nBufferSize);
			break;

		case 0x11 : //SMSG_INSTANTSPECIALEFFECT
			LTSNPrintF(aTemp, sizeof(aTemp), "(ISE) |%d|", cParsePacket.TellEnd());
			LTStrCat(aMsgBuff, aTemp, nBufferSize);
			break;

		case 0x13 : //SMSG_PRELOADLIST
			LTStrCat(aMsgBuff, "(PLL) ", nBufferSize);
			break;

		case 0x14 : //SMSG_THREADLOAD
			LTStrCat(aMsgBuff, "(THL) ", nBufferSize);
			break;

		case 0x15 : //SMSG_UNLOAD
			LTStrCat(aMsgBuff, "(UL ) ", nBufferSize);
			break;

		case 0x16 : //SMSG_GLOBALLIGHT
			LTStrCat(aMsgBuff, "(GL ) ", nBufferSize);
			break;

		case 0x32 : //STC_FILEDESC
		{
			LTStrCat(aMsgBuff, "(FD ) ", nBufferSize);
			while (!cParsePacket.EOP())
			{
				cParsePacket.Seek(48);
				cParsePacket.ReadString(aTemp, sizeof(aTemp));
				LTStrCat(aMsgBuff, "\n    ", nBufferSize);
				LTStrCat(aMsgBuff, aTemp, nBufferSize);
			}
			break;
		}

		case 0x36 : //CTS_FILESTATUS
			LTStrCat(aMsgBuff, "(FS ) ", nBufferSize);
			break;

		default :
			LTSNPrintF(aTemp, sizeof(aTemp), "(UNK) |%d, %d|", nPacketID, cParsePacket.TellEnd());
			LTStrCat(aMsgBuff, aTemp, nBufferSize);
			break;
	}
}

void ParseMsg(const CPacket_Read &cPacket, uint32 nFlags, uint32 nTravelDir, CBaseConn *pConn)
{
	if (nFlags == 0)
		return;

	// Don't parse local messages unless they really want us to
	if (((pConn->m_ConnFlags & CONNFLAG_LOCAL) != 0) && ((nFlags & 0x10) == 0))
		return;

	char aMsgBuff[8192];

	ParsePacket(cPacket, true, aMsgBuff, sizeof(aMsgBuff), nTravelDir);

	// & 1 == show unclassified packets..
	bool bPrint = true;

	// Filter out unguaranteed updates
	if (bPrint && ((nFlags & 0x02) == 0))
	{
		if (strstr(aMsgBuff, "(UUP)"))
			bPrint = false;
	}

	// Filter out guaranteed updates
	if (bPrint && ((nFlags & 0x04) == 0))
	{
		if (strstr(aMsgBuff, "(UPD)"))
			bPrint = false;
	}

	// Filter out netmgr stuff
	if (bPrint && ((nFlags & 0x08) == 0))
	{
		if (strstr(aMsgBuff, "(NM )"))
			bPrint = false;
	}

	if(bPrint)
		dsi_ConsolePrint(aMsgBuff);
}

// ------------------------------------------------------------------------ //
// CBaseConn
// ------------------------------------------------------------------------ //

CBaseConn::CBaseConn()
{
	m_ConnFlags = 0;
}


CBaseConn::~CBaseConn()
{
	// Remove all queued packets.
	GDeleteAndRemoveElementsOB(m_Latent, m_pDriver->m_pNetMgr->m_LatentPacketBank);

	if(g_TransportDebug > 0)
	{
		dsi_ConsolePrint("Conn %p closing", this);
	}
}


// ------------------------------------------------------------------------ //
// CNetMgr
// ------------------------------------------------------------------------ //

CNetMgr::CNetMgr()
{
	m_pMainDriver = LTNULL;
	m_FrameTime = 0.0f;
	memset(&m_guidApp, 0, sizeof(m_guidApp));
	m_Flags = 0;
}


CNetMgr::~CNetMgr()
{
	Term();
}


bool CNetMgr::Init(char *pPlayerName)
{
	m_PlayerName[0] = 0;

	m_LatentPacketBank.SetCacheSize(256);
	m_nDroppedPackets = 0;
	m_pCurPrefix = "";

	LTStrCpy(m_PlayerName, pPlayerName, sizeof(m_PlayerName));
	m_fLastTime = time_GetTime( );

	SetAppGuid(LTNULL);
	SetMainDriver(LTNULL);

	return true;
}


void CNetMgr::Term()
{
	TermDrivers();

	ASSERT(m_aDelayedConnections.empty());

	m_PlayerName[0] = 0;
	m_pHandler = LTNULL;
}


LTRESULT CNetMgr::InitDrivers()
{
	TermDrivers();
	AddDriver("internet");
	return LT_OK;
}


void CNetMgr::TermDrivers()
{
	while (m_Drivers.GetSize() > 0)
	{
		uint32 prevSize = m_Drivers.GetSize();
		RemoveDriver(m_Drivers[0]);
		if (prevSize == m_Drivers.GetSize())
			break;
	}
}


LTRESULT CNetMgr::GetServiceList(NetService *&pListHead)
{
	pListHead = LTNULL;
	for (uint32 i = 0; i < m_Drivers; ++i)
	{
		m_Drivers[i]->GetServiceList(pListHead);
	}

	return LT_OK;
}


LTRESULT CNetMgr::FreeServiceList(NetService *pListHead)
{
	NetService *pCur, *pNext;

	for (pCur = pListHead; pCur; pCur = pNext)
	{
		pNext = pCur->m_pNext;
		delete pCur;
	}
	return LT_OK;
}


LTRESULT CNetMgr::SelectService(HNETSERVICE hService)
{
	BaseService *pService;

	ASSERT(hService);

	pService = (BaseService*)hService;
	if (m_Drivers.FindElement(pService->m_pDriver) == BAD_INDEX)
	{
		return LT_NOTINITIALIZED;
	}
	else
	{
		if (pService->m_pDriver->SelectService(pService) == LT_OK)
		{
			m_pMainDriver = pService->m_pDriver;
			return LT_OK;
		}
		else
		{
			return LT_ERROR;
		}
	}
}


LTRESULT CNetMgr::GetSessionList(NetSession* &pListHead, const char *pInfo)
{
	if (!m_pMainDriver)
		return LT_NOTINITIALIZED;

	return m_pMainDriver->GetSessionList(pListHead, pInfo);
}


LTRESULT CNetMgr::FreeSessionList(NetSession *pListHead)
{
	NetSession *pCur, *pNext;
	
	for (pCur = pListHead; pCur; pCur = pNext)
	{
		pNext = pCur->m_pNext;
		delete pCur;
	}
	
	return LT_OK;	
}


LTRESULT CNetMgr::GetSessionName(char *pName, uint32 bufLen)
{
	if (!m_pMainDriver)
	{
		pName[0] = 0;
		return LT_NOTINITIALIZED;
	}
	return m_pMainDriver->GetSessionName(pName, bufLen);
}


LTRESULT CNetMgr::SetSessionName(const char *pName)
{
	if (!m_pMainDriver)
	{
		return LT_NOTINITIALIZED;
	}
	return m_pMainDriver->SetSessionName(pName);
}


LTRESULT CNetMgr::GetLocalIpAddress(char *pAddress, uint32 bufLen, uint16 &hostPort)
{
	if (!m_pMainDriver)
	{
		pAddress[0] = 0;
		return LT_NOTINITIALIZED;
	}
	// TRON: NOLF2's: the driver's bool as it is (server.dll 0x1002229b)
	return m_pMainDriver->GetLocalIpAddress(pAddress, bufLen, hostPort);
}

LTRESULT CNetMgr::GetMaxConnections(uint32 &nMaxConnections)
{
	if (!m_pMainDriver)
	{
		return LT_NOTINITIALIZED;
	}
	return m_pMainDriver->GetMaxConnections(nMaxConnections);
}


void CNetMgr::SetNetHandler(CNetHandler *pHandler)
{
	m_pHandler = pHandler;

	// Flush the connection queue
	while (!m_aDelayedConnections.empty())
	{
		CBaseConn *pConn = m_aDelayedConnections.front();
		m_aDelayedConnections.pop_front();
		
		NewConnectionNotify(pConn);
	}
}

void CNetMgr::Update(char *pPrefix, LTFLOAT curTime, bool bAllowTimeout)
{
	uint32 i;

	m_pCurPrefix = pPrefix;

	float timeDelta = curTime - m_fLastTime;
	timeDelta = LTCLAMP(timeDelta, 0.001f, 0.3f);
	m_fLastTime = curTime;

	m_FrameTime = timeDelta;
	
	for (i = 0; i < m_Connections; ++i)
	{
		CBaseConn *pConn = m_Connections[i];

		// Countdown the latent packets and ship them off.
		for (GPOS pos = pConn->m_Latent.GetHeadPosition(); pos;)
		{
			LatentPacket *pLatent = pConn->m_Latent.GetNext(pos);
			
			pLatent->m_SendTimeCounter += timeDelta;
			if(pLatent->m_SendTimeCounter >= g_CV_LatencySim)
			{
				ReallySendPacket(pLatent->m_cPacket, pConn, pLatent->m_nPacketFlags);
				
				pConn->m_Latent.RemoveAt(pLatent);
				m_LatentPacketBank.Free(pLatent);
			}
		}

		// Update timers and do guaranteed delivery stuff...
		pConn->m_SendPPS.Update(timeDelta);
		pConn->m_SendBPS.Update(timeDelta);
		pConn->m_RecvPPS.Update(timeDelta);
		pConn->m_RecvBPS.Update(timeDelta);
	}	


	// Update the global send rate tracker
	m_SendBPS.Update(timeDelta);

	// Output stats on stuff.
	if (g_CV_ShowConnStats)
	{
		for (i = 0; i < m_Connections; ++i)
		{
			CBaseConn *pConn = m_Connections[i];

			NetDebugOut2(pConn, 0, "Conn %d Info - Ping: %.2f", i, pConn->GetPing());
			
			NetDebugOut2(pConn, 0, "Conn %d Recv - PPS: %.1f, BPS: %.1f",
				i, pConn->m_RecvPPS.GetRate(), pConn->m_RecvBPS.GetRate());

			NetDebugOut2(pConn, 0, "Conn %d Send - PPS: %.1f, BPS: %.1f",
				i, pConn->m_SendPPS.GetRate(), pConn->m_SendBPS.GetRate());

			NetDebugOut2(pConn, 0, "Conn %d Raw - Recv: %7d, Send: %7d, Loss: %6.2f",
				i, pConn->GetIncomingBandwidthUsage(), pConn->GetOutgoingBandwidthUsage(), pConn->GetPacketLoss());

		}
	}
	
	
	// Update the drivers.
	for (i = 0; i < m_Drivers; ++i)
		m_Drivers[i]->Update();
}


CBaseDriver* CNetMgr::AddDriver(const char *pInfo)
{
	CBaseDriver *pDriver = LTNULL;

	if (strcmp(pInfo, "local") == 0)
	{
		LT_MEM_TRACK_ALLOC(pDriver = new CLocalDriver, LT_MEM_TYPE_NETWORKING);
	}
	else if (strcmp(pInfo, "internet") == 0)
	{
		LT_MEM_TRACK_ALLOC(pDriver = new CUDPDriver,LT_MEM_TYPE_NETWORKING);
	}

	if (pDriver)
	{
		pDriver->m_pNetMgr = this;

		if (!pDriver->Init())
		{
			delete pDriver;
			return LTNULL;
		}

		LT_MEM_TRACK_ALLOC(m_Drivers.Append(pDriver), LT_MEM_TYPE_NETWORKING);

		pDriver->UpdateGUID(m_guidApp);
	}

	return pDriver;
}


void CNetMgr::RemoveDriver(CBaseDriver *pDriver)
{
	uint32 index = m_Drivers.FindElement(pDriver);

	ASSERT(index != BAD_INDEX);

	if (pDriver == m_pMainDriver)
		m_pMainDriver = LTNULL;

	delete pDriver;
	m_Drivers.Remove(index);
}


void CNetMgr::Disconnect(CBaseConn *id, EDisconnectReason reason)
{
	if (!id)
	{
		return;
	}

	NetDebugOut2(id, 1, "CNetMgr::Disconnect called, (reason %d).", reason);
	id->m_pDriver->Disconnect(id, reason);
}


bool CNetMgr::SendPacket(const CPacket_Read &cPacket, CBaseConn *idSendTo, uint32 packetFlags)
{
	if(!idSendTo)
		return false;

	ParseMsg(cPacket, g_CV_ParseNet_Outgoing | g_CV_ParseNet, NETMGR_TRAVELDIR_UNKNOWN, idSendTo);

	return LagOrSend(CPacket_Read(cPacket), idSendTo, packetFlags);
}


void CNetMgr::StartGettingPackets()
{
	m_Flags |= NETMGR_GETTINGPACKETS;
}


void CNetMgr::EndGettingPackets()
{
	m_Flags &= ~NETMGR_GETTINGPACKETS;
}

bool CNetMgr::GetPacket(uint8 nTravelDir, CPacket_Read *pPacket, CBaseConn **pSender)
{
	// Check the drivers.
	for (uint32 i=0; i < m_Drivers; i++)
	{
		CBaseDriver *pDriver = m_Drivers[i];

		CPacket_Read cCurPacket;
		CBaseConn *pCurSender;

		while (pDriver->GetPacket(&cCurPacket, &pCurSender))
		{
			IncRecvCounter(pCurSender, cCurPacket.Size());

			ParseMsg(cCurPacket, g_CV_ParseNet_Incoming | g_CV_ParseNet, nTravelDir, pCurSender);

			if (HandleReceivedPacket(cCurPacket, pCurSender, true))
			{
				*pPacket = cCurPacket;
				*pSender = pCurSender;
				return true;
			}
		}
	}

	return false;
}

bool CNetMgr::LagOrSend(const CPacket_Read &cPacket, CBaseConn *idSendTo, uint32 nPacketFlags)
{
	if (g_CV_LatencySim > 0.001f)
	{
		// Add it as a latent packet on this connection
		LatentPacket *pLatent = m_LatentPacketBank.Allocate();
		pLatent->m_cPacket = cPacket;
		pLatent->m_nPacketFlags = nPacketFlags;
		pLatent->m_SendTimeCounter = 0.0f;
		idSendTo->m_Latent.AddTail(pLatent);

		return true;
	}

	bool bRet = ReallySendPacket(cPacket, idSendTo, nPacketFlags);
	if (bRet)
	{
		IncSendCounter(idSendTo, cPacket.Size());
	}

	return bRet;
}

void CNetMgr::IncRecvCounter(CBaseConn *id, uint32 packetLen)
{
	if(!id)
		return;

	id->m_RecvPPS.Add(1.0f);
	id->m_RecvBPS.Add((float)packetLen);
}


void CNetMgr::IncSendCounter(CBaseConn *id, uint32 packetLen)
{
	if(!id)
		return;

	id->m_SendPPS.Add(1.0f);
	id->m_SendBPS.Add((float)packetLen);
	// Don't count the bandwidth of our local connection in the global count
	if ((id->m_ConnFlags & CONNFLAG_LOCAL) == 0)
		m_SendBPS.Add((float)packetLen);
}


bool CNetMgr::NewConnectionNotify(CBaseConn *id)
{
	if (g_TransportDebug)
		DebugOut("NewConnectionNotify\n");
	
	if (m_pHandler)
	{
		// Init the connection.
		LT_MEM_TRACK_ALLOC(m_Connections.Append(id), LT_MEM_TYPE_NETWORKING);

		if (m_pHandler->NewConnectionNotify(id, id->m_ConnFlags & CONNFLAG_LOCAL))
		{
			return true;
		}
		else
		{
			m_Connections.Remove(m_Connections.LastI());
		}
	}
	else
		m_aDelayedConnections.push_back(id);

	return false;
}


void CNetMgr::DisconnectNotify(CBaseConn *id, EDisconnectReason eDisconnectReason )
{
	uint32 index = m_Connections.FindElement(id);

	if (g_TransportDebug)
		DebugOut("DisconnectNotify\n");

	// Remove the connection.
	//ASSERT( index != BAD_INDEX );

	if (index != BAD_INDEX)
	{
		if (m_pHandler)
			m_pHandler->DisconnectNotify( id, eDisconnectReason );

		m_Connections.Remove(index);
	}
	else
	{
		// See if it's in the delayed queue
		TDelayedConnectionQueue::iterator iDelayedConn = std::find(m_aDelayedConnections.begin(), m_aDelayedConnections.end(), id);
		if (iDelayedConn != m_aDelayedConnections.end())
			m_aDelayedConnections.erase(iDelayedConn);
	}
}

bool CNetMgr::ReallySendPacket(const CPacket_Read &cPacket, CBaseConn *idSendTo, uint32 nPacketFlags)
{
	bool bIsGuaranteed = (((nPacketFlags & MESSAGE_GUARANTEED) != 0) && ((idSendTo->m_ConnFlags & CONNFLAG_LOCAL) == 0));

	CPacket_Read cSendPacket(cPacket);

	ASSERT(idSendTo);

	if (g_TransportDebug >= 5)
	{
		CPacket_Read cTempPacket(cSendPacket);
		cTempPacket.SeekTo(0);
		uint32 nPacketID = cTempPacket.Readuint8();
		NetDebugOut2(idSendTo, 5, "Sent packet to connection %p (packet ID %d, length %d).", 
			idSendTo, nPacketID, cSendPacket.Size());
	}

	// Send it...
	return idSendTo->m_pDriver->SendPacket(cSendPacket, idSendTo, bIsGuaranteed);
}


bool CNetMgr::HandleReceivedPacket(CPacket_Read &cPacket, CBaseConn *pSender, bool bMaybeDrop)
{
	// Reset the packet read index, just in case
	cPacket.SeekTo(0);

	if (g_CV_DropRate > 0.0001f && bMaybeDrop)
	{
		if (pSender->m_ConnFlags & CONNFLAG_LOCAL)
		{
			dsi_ConsolePrint("Warning: using DropRate on local connection");
			dsi_ConsolePrint("         MUST use ForceRemote in addition");
		}

		// Ok, maybe drop the packet.
		float test;
		test = ((float)rand() / RAND_MAX) * 100.0f;
		if (test < g_CV_DropRate)
		{
			++m_nDroppedPackets;

			NetDebugOut2(pSender, 1, "Dropping packet");
			return false;
		}
	}

	NetDebugOut2(pSender, 4, "Got packet from %p (packet ID %d, length %d).", 
		pSender, cPacket.Peekuint8(), cPacket.Size());

	return true;
}


void CNetMgr::SetAppGuid(LTGUID* pAppGuid)
{
	if (pAppGuid)
	{
		memcpy(&m_guidApp, pAppGuid, sizeof(m_guidApp));
	}
	else
	{
		memset(&m_guidApp, 0, sizeof(m_guidApp));
	}

	// Update the drivers
	for (uint32 i = 0; i < m_Drivers.GetSize(); i++)
	{
		m_Drivers[i]->UpdateGUID(m_guidApp);
	}
}

CBaseDriver* CNetMgr::GetDriver(const char* sDriver)
{
	for (uint32 i = 0; i < m_Drivers.GetSize(); i++)
	{
		CBaseDriver* pDriver = m_Drivers[i];
		if (pDriver)
		{
			if (stricmp(pDriver->m_Name, sDriver) == 0)
			{
				return pDriver;
			}
		}
	}

	return LTNULL;
}


// Helper for debugging.
void CNetMgr::NetDebugOut(int debugLevel, char *pMsg, ...)
{
	if (debugLevel > g_TransportDebug)
		return;

	static const uint32 knBufferSize = 512;
	char msg[knBufferSize];
	va_list marker;

	va_start(marker, pMsg);
	LTVSNPrintF(msg, knBufferSize, pMsg, marker);
	va_end(marker);

	dsi_ConsolePrint("(%d) %s%s", g_NMTimeCounter.CountMS(), m_pCurPrefix, msg);
}


void CNetMgr::NetDebugOut2(CBaseConn *pConn, int debugLevel, char *pMsg, ...)
{
	if (debugLevel > g_TransportDebug)
		return;

	if (!g_bLocalDebug && ((pConn->m_ConnFlags & CONNFLAG_LOCAL) != 0))
		return;

	static const uint32 knBufferSize = 512;
	char msg[knBufferSize];
	va_list marker;

	va_start(marker, pMsg);
	LTVSNPrintF(msg, knBufferSize, pMsg, marker);
	va_end(marker);

	dsi_ConsolePrint("(%d) %s%s", g_NMTimeCounter.CountMS(), m_pCurPrefix, msg);
}






