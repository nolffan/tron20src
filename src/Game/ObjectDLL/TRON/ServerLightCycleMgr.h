// ----------------------------------------------------------------------- //
//
// MODULE  : ServerLightCycleMgr.h
//
// PURPOSE : The server's light cycle manager
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto. RTTI .?AVCServerLightCycleMgr@@, vtable 0x10294b70; it derives
// from CLightCycleMgr (LightCycleMgr.h, vtable 0x10294bcc). Its TU is
// 0x1004f420..0x10053b50 (with the STL templates it emits). The class name
// is retail's; the file name, the members and the names of the functions
// not in the base are ours. Retail offsets are given (ours are the same).
//
// ----------------------------------------------------------------------- //

#ifndef __SERVER_LIGHT_CYCLE_MGR_H__
#define __SERVER_LIGHT_CYCLE_MGR_H__

#include "LightCycleMgr.h"
#include <list>
#include <queue>

class LightCyclePowerup;

// A queued rider dialog line (0x18 bytes; the names are ours)
struct LCDialogQueueEntry
{
	LCDIALOGSOUNDS*	pSounds;		// RETAIL: +0x00
	float			fPriority;		// RETAIL: +0x04
	LTVector		vPos;			// RETAIL: +0x08
	LIGHT_CYCLIST*	pCyclist;		// RETAIL: +0x14

	// RETAIL: the priority_queue's ordering (0x10053160, 0x100539f0)
	bool operator<(const LCDialogQueueEntry& other) const { return fPriority < other.fPriority; }
};

// A client's report of its cycle (0x14 bytes, LCI_POSITION; the names are ours)
struct LCPositionUpdate
{
	HOBJECT	hObject;				// RETAIL: +0x00
	float	fX;						// RETAIL: +0x04
	float	fZ;						// RETAIL: +0x08
	float	fForwardX;				// RETAIL: +0x0c
	float	fForwardZ;				// RETAIL: +0x10
};

typedef std::priority_queue<LCDialogQueueEntry>	LCDialogQueue;
typedef std::list<LCPositionUpdate*>			LCPositionUpdateList;
typedef std::vector<LightCyclePowerup*>			LCPowerupList;

class CServerLightCycleMgr : public CLightCycleMgr
{
	public:

		CServerLightCycleMgr();						// RETAIL: 0x1004fb50
		virtual ~CServerLightCycleMgr() { Term(); }	// RETAIL: 0x1004fe90

		virtual bool Init();						// RETAIL: 0x100500c0
		virtual void Term();						// RETAIL: 0x10050100

		virtual LIGHT_CYCLIST* AddCyclist(HOBJECT hObj, const char* szCycleColor, bool bUpdate,
			const LTVector* pvDims, uint32 nNameId, bool bTronUnknown);	// RETAIL: 0x10050aa0
		virtual bool RemoveCyclist(HOBJECT hObj);	// RETAIL: 0x10050d30
		virtual bool DerezCyclist(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLIST* pKiller, bool bTronUnknown);	// RETAIL: 0x10050e90
		virtual void HandleHeadOnCollision(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLIST* pOther);	// RETAIL: 0x10053050
		virtual void Update();						// RETAIL: 0x100501d0
		virtual bool HandleCollision(LIGHT_CYCLIST* pCyclist, LightCycleCollisionInfo* pInfo, bool bTronUnknown1, bool bTronUnknown2);	// RETAIL: 0x100520d0
		virtual bool TronUnknownVirtual9(LIGHT_CYCLIST* pCyclist, uint32 nTronUnknown1, uint32 nTronUnknown2, bool bTronUnknown);	// RETAIL: 0x10051fc0
		virtual bool SetPowerup(LIGHT_CYCLIST* pCyclist, int nPowerup);	// RETAIL: 0x10051840
		virtual bool ActivatePowerup(LIGHT_CYCLIST* pCyclist, const LTVector& vPos);	// RETAIL: 0x10051980
		virtual void TronUnknownVirtual17(LIGHT_CYCLIST* pCyclist, uint32 nTronUnknown);	// RETAIL: 0x10051fe0
		virtual void TronUnknownVirtual18();		// RETAIL: 0x100524c0
		virtual void OnCyclistTurbo(LIGHT_CYCLIST* pCyclist, bool bTronUnknown, float fTronUnknown1, float fTronUnknown2, uint32 nTronUnknown);	// RETAIL: 0x10051620

		// A client's light cycle message (MID_LIGHT_CYCLE_INFO). RETAIL: 0x10051150
		bool OnMessage(ILTMessage_Read* pMsg, HCLIENT hSender);

		// Derezzes all the AI cyclists. RETAIL: 0x10052660
		void DerezAllAICyclists();

		// The difficulty's speed and trail length, told to hClient (or all).
		// RETAIL: 0x100526a0, 0x100526d0
		void SetGameSpeed(int nIndex, HCLIENT hClient);
		void SetTrailLength(int nIndex, HCLIENT hClient);

		// Tells a client about all the cyclists. RETAIL: 0x100529f0
		void SendCyclistsToClient(HCLIENT hClient);

		// RETAIL: 0x10052900
		void SendTronUnknown21(HCLIENT hClient, int8 nTronUnknown);

		// The power-ups lying around. RETAIL: 0x10052380 (Remove)
		void AddPowerup(LightCyclePowerup* pPowerup) { m_collPowerups.push_back(pPowerup); }
		bool RemovePowerup(HOBJECT hPowerup);

		// RETAIL: 0x10052360 (writes g_pServerLightCycleMgr's flag)
		static void EnableDialog(bool bEnable);

	protected:

		// The non-virtual workers of the virtuals above
		bool DerezCyclist(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLIST* pKiller, bool bTronUnknown, bool bNotifyMission);	// RETAIL: 0x10050eb0
		bool TronUnknown9(LIGHT_CYCLIST* pCyclist, uint32 nTronUnknown1, uint32 nTronUnknown2, HCLIENT hExclude);	// RETAIL: 0x10051e50
		void SendCyclistTurbo(LIGHT_CYCLIST* pCyclist, bool bTronUnknown, float fTronUnknown1, float fTronUnknown2,
			float fTronUnknown3, float fTronUnknown4, uint32 nTronUnknown, HCLIENT hExclude);	// RETAIL: 0x10051690

		// Rider dialog. RETAIL: 0x1004f660, 0x1004f8c0, 0x10052330
		void PlayDialog(LIGHT_CYCLIST* pCyclist, int eType);
		void PlayDialogEntry(const LCDialogQueueEntry& entry);
		void ClearDialogQueue();
		float GetDialogTime() { return (m_fDialogPauseTime > 0.0f) ? m_fDialogPauseTime : g_pLTServer->GetTime(); }

		// The cyclist of the first player. RETAIL: 0x10051810
		LIGHT_CYCLIST* GetPlayerCyclist();

		// The cyclist nearest pCyclist. RETAIL: 0x10052250
		LIGHT_CYCLIST* FindNearestCyclist(LIGHT_CYCLIST* pCyclist, bool bAIOnly, bool bWithPowerup);

		// The power-up nearest an object. RETAIL: 0x100523f0
		LightCyclePowerup* FindNearestPowerup(HOBJECT hObj, bool bAny);

		// Messages. RETAIL: 0x10052700, 0x10052800, 0x10052b40, 0x10052f20
		void SendGameSpeed(HCLIENT hClient);
		void SendTrailLength(HCLIENT hClient);
		void WriteCyclist(LIGHT_CYCLIST* pCyclist, ILTMessage_Write* pMsg);
		void WriteTrail(LIGHT_CYCLE_TRAIL* pTrail, ILTMessage_Write* pMsg);

		// The position reports' pool. RETAIL: 0x10052fe0
		LCPositionUpdate* NewPositionUpdate();

		LCDialogQueue			m_DialogQueue;				// RETAIL: +0x133c
		float					m_fMinTimeBetweenDialog;	// RETAIL: +0x134c
		float					m_fMaxTimeBetweenDialog;	// RETAIL: +0x1350
		float					m_fLastDialogTime;			// RETAIL: +0x1354
		float					m_fDialogDelay;				// RETAIL: +0x1358
		float					m_fDialogPauseTime;			// RETAIL: +0x135c
		bool					m_bDialogPlayed;			// RETAIL: +0x1360
		// RETAIL: +0x1361..0x1363 are never read or written in the TU
		// (0x1004f420..0x10053b50); m_bDialogEnabled is at +0x1364 (0x1005236d).
		uint8					m_xTronUnknown1361[3];
		bool					m_bDialogEnabled;			// RETAIL: +0x1364
		bool					m_bTronUnknown1365;			// RETAIL: +0x1365
		LCPowerupList			m_collPowerups;				// RETAIL: +0x1368
		LCPositionUpdateList	m_lstPositionUpdates;		// RETAIL: +0x1374
		LCPositionUpdateList	m_lstFreePositionUpdates;	// RETAIL: +0x1378
};

extern CServerLightCycleMgr* g_pServerLightCycleMgr;	// RETAIL: 0x1035a550

#endif // __SERVER_LIGHT_CYCLE_MGR_H__
