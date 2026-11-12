// ----------------------------------------------------------------------- //
//
// MODULE  : SecRezzer.h
//
// PURPOSE : A spawner of security programs at its rez points
//
// RETAIL: Tron 2.0's, not in any source release (retail Object.lto: class
// def 0x102dcb78, size 0x830; vtable 0x10294a64), written from retail
// Object.lto 0x10043400..0x10048f70. A destructible model that, told ON,
// opens and rezzes its programs (a template) at the free rez points of
// its own, keeping up to MaxActive of them; a player with its permission
// sets disables it. The names but the class's are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SECREZZER_H__
#define __SECREZZER_H__

#include "Spawner.h"
#include "DestructibleModel.h"
#include "ActivateTypeMgr.h"
#include "AutoCommand.h"
#include "SharedFXStructs.h"
#include <vector>
#include <slist>

// Where a security program rezzes in (a SecRezzerPoint's)
struct SecRezzerPointInfo
{
	LTVector	vPos;
	LTRotation	rRot;
};

typedef std::vector<SecRezzerPointInfo> SecRezzerPointList;

// RETAIL: CDestructibleModel is 0x738 bytes in retail (its constructor,
// retail 0x1010ebd0), 0x728 in ours: this pads SecRezzer's members to
// retail's offsets. Remove it when CDestructibleModel is retail's (it
// then fails to compile: an array of size 0)
#define SECREZZER_DESTRUCTIBLEMODEL_PAD		(0x738 - sizeof(CDestructibleModel))

class SecRezzer : public Spawner
{
	public :

		SecRezzer();
		virtual ~SecRezzer();

		SecRezzerPointList& GetPoints() { return m_lstPoints; }

	protected :

		// RETAIL: the states Update runs (retail 0x10045550); an "enter"
		// virtual sets the next state, an "update" waits
		enum State
		{
			eStateEnterIdle = 0,
			eStateIdle,
			eStateEnterSelect,
			eStateSelect,
			eStateEnterRun,
			eStateRun,
			eStateLock,
			eStateEnterDeselect,
			eStateDeselect,
			eStateEnterDisable,
			eStateDisable,
			eStateDisabled,
		};

        virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual uint32	ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		// RETAIL: its own virtuals, slots 25..35 (vtable 0x10294a64)
		virtual void	Spawn();
		virtual void	EnterIdle();
		virtual void	EnterSelect();
		virtual void	UpdateSelect();
		virtual void	EnterRun();
		virtual void	UpdateRun();
		virtual void	EnterDeselect();
		virtual void	UpdateDeselect();
		virtual void	UpdateLock();
		virtual void	EnterDisable();
		virtual void	UpdateDisable();

	private :

		LTBOOL	ReadProp(ObjectCreateStruct *pStruct);
		LTBOOL	InitialUpdate();
		void	Update();
		int		CountActive();
		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		CDestructibleModel				m_damage;
		uint8							m_aDestructibleModelPad[SECREZZER_DESTRUCTIBLEMODEL_PAD];
		int								m_eState;
		int								m_nMaxActive;			// At most this many of our programs at once
		int								m_nNumberToSpawn;		// Rezzed each time we are told ON
		int								m_nToSpawn;				// Left to rez this time
		std::string						m_sSpawnName;			// The name of the programs (ON's argument)
		std::slist<LTObjRef>			m_lstSpawned;			// Our programs
		SecRezzerPointList				m_lstPoints;			// Our rez points
		SecRezzerPointList				m_lstFreePoints;		// Those no one stands at
		uint8							m_nPSets;				// The permission sets a player needs
		CLIENTFXUSERCREATESTRUCT		m_cs;					// Our special fx message: our model
		AutoCommand<MultiDispatch>		m_InvalidPSetCommand;	// Run when a player without the permission sets tries
		LTObjRef						m_hHook;				// Told LOCK/UNLOCK/DISABLE (HOOKSENDER)
		bool							m_bLocked;				// m_hHook was told LOCK
		CActivateTypeHandler			m_ActivateTypeHandler;
		uint32							m_nTronUnknown82c;
};

// RETAIL: its plugin (RTTI SecrezzerPlugin, vtable 0x10294a48): the
// ModelTemplate list
class SecrezzerPlugin : public CSpawnerPlugin
{
	public :

		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);

	protected :

		CModelButeMgrPlugin			m_ModelButeMgrPlugin;
};

#endif // __SECREZZER_H__
