// ----------------------------------------------------------------------- //
//
// MODULE  : HUDMessageQueue.h
//
// PURPOSE : Definition of CHUDMessageQueue to display messages
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_MSG_Q_H
#define __HUD_MSG_Q_H

#include "HUDItem.h"
#include "HUDMessage.h"

//******************************************************************************************
//** HUD Message Queue
//******************************************************************************************
class CHUDMessageQueue : public CHUDItem
{
public:
	// RETAIL: every game type
	// RETAIL: CHUDSubtitles' form (0x100fd0c0, which the HUD items fold into)
	virtual bool	IsUsedInGameType(GameType eGameType) { if (eGameType < eGameTypeSingle || eGameType > eGameTypeMultiplayerLightcycles) return false; return true; }

	CHUDMessageQueue();
	

    virtual LTBOOL      Init();
	virtual void		Term();

    virtual void        Render();
    virtual void        Update();

	virtual	void		AddMessage(MsgCreate &fmt, bool bHistoryOnly = false);

	virtual uint8		GetNumActiveMsgs() {return (uint8)m_ActiveMsgs.size(); }

	virtual void		ShowHistory(LTBOOL bShow);
	virtual void		ClearHistory();

	virtual void SetHistoryOffset(uint16 nOffset);

	//display an earlier page of history
	virtual void IncHistoryOffset();

	//display an later page of history
	virtual void DecHistoryOffset();

	virtual void		CanDraw( bool bDraw ) { m_bDraw = bDraw; }

protected:
	static const uint16 kMaxHistory;
	

	LTBOOL		m_bTopJustify;
	LTBOOL		m_bShowHistory;

	// RETAIL: Tron's (0x18): ShowHistory does nothing unless it is set
	// (0x1015f540); only the chat queue sets it (its constructor,
	// 0x1015f760). The name is ours.
	LTBOOL		m_bCanShowHistory;

	typedef std::vector<CHUDMessage*> MessageArray;
	MessageArray	m_ActiveMsgs;
	MessageArray	m_HistoryMsgs;

	typedef std::vector<MsgCreate *> MCArray;
	MCArray	m_History;

	uint16		m_nHistoryOffset;


    LTIntPt		m_BasePos;
	MsgCreate	m_MsgFormat;

	uint8		m_nMaxActiveMsgs;
	uint8		m_nMaxHistoryMsgs;

	bool		m_bDraw;
};

enum eChatMsgType
{
	kMsgDefault = 0,
	kMsgChat,
	kMsgCheatConfirm,
	kMsgTransmission,
	kMsgScmd,
	kMsgTeam,
	kMsgRedTeam,
	kMsgBlueTeam,
	kNumChatMsgTypes,

	// RETAIL: Tron's colors by type (CHUDChatMsgQueue::UpdateLayout
	// 0x1015f8d0): 5 is the multiplayer announcements' (the match managers'
	// team messages pass 5, e.g. 0x1000f45e), 6 team chat's, 7 a second
	// transmission color; 4 is not read. These names are ours.
	kMsgMultiAnnounce = kMsgTeam,
	kMsgTeamChat = kMsgRedTeam,
	kMsgTransmission2 = kMsgBlueTeam
};

class CHUDChatMsgQueue : public CHUDMessageQueue
{
public:

	// RETAIL: Tron's constructor (0x1015f760) lets the chat queue show
	// its history.
	CHUDChatMsgQueue();

	virtual void        UpdateLayout();

	virtual	void		AddMessage(const char *pszString, eChatMsgType type = kMsgDefault);
	virtual	void		AddMessage(int nMessageID, eChatMsgType type = kMsgDefault);

	// RETAIL: Tron's last slot (retail 0x1015f8b0): the layout of the
	// multiplayer queue or the single player one. The name is ours
	virtual void		SetMultiplayerLayout(bool bMultiplayer)
	{
		UpdateLayout(bMultiplayer ? "MultiChatMessageQueue" : "ChatMessageQueue");
	}

	// RETAIL: Tron's, NOLF2's UpdateLayout for a given layout tag (retail
	// 0x1015f8d0)
	void				UpdateLayout(char *pTag);


protected:

	uint32		m_nMsgColors[kNumChatMsgTypes];

};

class CHUDPickupMsgQueue : public CHUDMessageQueue
{
public:

	virtual void        UpdateLayout();
    virtual void        Render();


	virtual	void		AddMessage(const char *pszString,const char *pszImage);
	virtual	void		AddMessage(int nMessageID,const char *pszImage);

};

class CHUDRewardMsgQueue : public CHUDChatMsgQueue
{
public:

	virtual void        UpdateLayout();
};

// RETAIL: Tron's announcement queues (RTTI CHUDAnnouncementMsgQueue and
// CHUDMultiAnnouncementMsgQueue, vtables 0x101e8ef8/0x101e8e98, 23 slots):
// chat queues (0xa4 bytes, made with the chat queue's constructor by the
// HUD manager's at 0x1009b003) with their own layouts and centered,
// shadowed messages.
class CHUDAnnouncementMsgQueue : public CHUDChatMsgQueue
{
public:

	virtual void        UpdateLayout();

	virtual	void		AddMessage(const char *pszString, eChatMsgType type = kMsgDefault);
	virtual	void		AddMessage(int nMessageID, eChatMsgType type = kMsgDefault)
	{
		CHUDChatMsgQueue::AddMessage(nMessageID, type);
	}

protected:

	void				UpdateLayout(char *pTag);
};

class CHUDMultiAnnouncementMsgQueue : public CHUDAnnouncementMsgQueue
{
public:

	virtual void        UpdateLayout();
};

#endif