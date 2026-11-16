// ----------------------------------------------------------------------- //
//
// MODULE  : DecisionObject.h
//
// PURPOSE : DecisionObject - Definition
//
// CREATED : 12/03/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DECISIONOBJECT_H__
#define __DECISIONOBJECT_H__

#include "ltengineobjects.h"
#include "clientservershared.h"
#include "GameBase.h"

LINKTO_MODULE( DecisionObject );

// RETAIL: Tron's choice has a string of its own besides the string ID
// (0x10 bytes; destructor 0x100c0af0, DecisionObject::Save 0x100c1880).
struct ChoiceData
{
    ChoiceData()
	{
		nStringID	= 0;
		hstrString	= LTNULL;
        hstrCmd     = LTNULL;
	}

    virtual ~ChoiceData()
	{
		FREE_HSTRING(hstrCmd);
		FREE_HSTRING(hstrString);
	}

	inline void Save(ILTMessage_Write *pMsg)
	{
		SAVE_DWORD(nStringID);
		SAVE_HSTRING(hstrCmd);
		SAVE_HSTRING(hstrString);
	}

	inline void Load(ILTMessage_Read *pMsg)
	{
		LOAD_DWORD(nStringID);
		LOAD_HSTRING(hstrCmd);
		LOAD_HSTRING(hstrString);
	}

	// Data...

	uint32	nStringID;
	HSTRING	hstrString;		// RETAIL: Tron's ("Choice%dString")
	HSTRING	hstrCmd;
};


// RETAIL: Tron's DecisionObject is 0xac bytes (ClassDef 0x102f2140;
// constructor 0x100c0a30, Save 0x100c1880).
class DecisionObject : public GameBase
{
	public :

		DecisionObject();
		~DecisionObject();

        void    HandleChoose(uint8 nChoice);

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		uint32	ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);	// RETAIL: Tron's (0x100c0d80)

		bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

		ChoiceData	m_ChoiceData[MAX_DECISION_CHOICES];	// 0x38
		HSTRING		m_hstrAbortCmd;				// 0x98
		HSTRING		m_hstrPlayerFiredAbortCmd;	// 0x9c RETAIL: Tron's
		bool		m_bVisible;					// 0xa0
		bool		m_bRemoveAfterChoice;		// 0xa1
		float		m_fRadius;					// 0xa4
		bool		m_bLock;					// 0xa8
		bool		m_bStartOn;					// 0xa9 RETAIL: Tron's
		bool		m_bEscapable;				// 0xaa RETAIL: Tron's
		uint8		m_nWindowStyle;				// 0xab RETAIL: Tron's

		void	ReadProp(ObjectCreateStruct *pData);
		void	InitialUpdate();
		void	Update();

		// RETAIL: Tron's shows the window to the object named by its
		// argument (the decision object itself when there is none)
        void    HandleShow(const char* pszTarget);
        void    HandleAbort(bool bNotifyClient);
		void	Show(bool bShow, bool bForceShow = false, HOBJECT hTarget = LTNULL);

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);
};

class CDecisionObjectPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT PreHook_PropChanged( 
			const	char		*szObjName,
			const	char		*szPropName,
			const	int			nPropType,
			const	GenericProp	&gpPropValue,
					ILTPreInterface	*pInterface,
			const	char		*szModifiers);

	protected:

		CCommandMgrPlugin	m_CommandMgrPlugin;

};

#endif // __DECISIONOBJECT_H__