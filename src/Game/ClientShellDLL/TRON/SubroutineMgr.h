// ----------------------------------------------------------------------- //
//
// MODULE  : KeyMgr.h
//
// PURPOSE : Attribute file manager for key item info
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

// TODO add code that other modules can use to determine what subroutines are
// active, maybe automatically hook functions or something.

// In alpha state a protection subroutine is a 1:1 draw from the energy bar.
// The higher the optimization level the less it draws from the energy changing the ratio.
// Lets say gold draws 50% of the protection subroutine's percentage.

#if !defined(_SUBROUTINE_MGR_H_)
#define _SUBROUTINE_MGR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "ScreenSpriteMgr.h"

#define SUBROUTINE_DEFAULT_FILE		"Attributes\\Subroutines.txt"
#define SUBROUTINE_MAX_NAME			32
#define SUBROUTINE_MAX_FILE_PATH	64


class CSubroutineMgr;
extern CSubroutineMgr* g_pSubroutineMgr;


// the subroutinemgr needs to maintain two arrays of subroutines.
// the first list is all conceivable types of subroutines, and this
// contains the minimal information on that subroutine.  It's essentially
// the template for that subroutine.

// The second array is the list of subroutines that actually exist at
// any given point in time, in the player's inventory.  This second list
// contains a pointer to the actual subroutine in the first list, and
// adds the game/player-specific data that's required.

extern char * szRingTex[];

typedef enum
{
	VERSION_ALPHA = 0,
	VERSION_BETA,
	VERSION_GOLD,
	LAST_VERSION
} SubVersion;

typedef enum
{
	SUBSTATE_OKAY = 0,
	SUBSTATE_FOREIGN,
	SUBSTATE_CORRUPT,
	SUBSTATE_UNUSABLE,	// bad block or basecode
	SUBSTATE_DELETED,
	SUBSTATE_LAST
} SubState;

typedef enum
{
	FUNCTION_COMBAT = 0,
	FUNCTION_DEFENSE,
	FUNCTION_UTILITY,
	FUNCTION_BADBLOCK,
	FUNCTION_BASECODE,
	FUNCTION_LAST
} SubFunction;

// RETAIL: the layouts below are retail's (ctors 0x10091e50, 0x10091ee0,
// 0x10091f10, 0x10091f80; sizes 0x218, 0x2c, 0x13c, 0x64 by their
// debug_new_fns 0x10096390.., read by 0x10092410, 0x100927d0, 0x10092200).
// Members build 69 had and retail does not are kept at the end (PORT:
// for the build 69 code still using them); names of retail's new members
// are ours, after their attribute names.
struct TronSubroutine
{
	TronSubroutine();

	char		szName[SUBROUTINE_MAX_NAME];		// +0x0
	uint16		nNameId;							// +0x20
	uint16		nDescriptionId;						// +0x22
	uint16		nDescriptionAlphaId;				// +0x24 "DescriptionAlpha"
	uint16		nDescriptionBetaId;					// +0x26 "DescriptionBeta"
	uint16		nDescriptionGoldId;					// +0x28 "DescriptionGold"
	SubFunction	eFunction;							// +0x2c combat, defense, utility, Bad sector, base code
	char		szSprite[SUBROUTINE_MAX_FILE_PATH];	// +0x30
	char		szHUDIcon[SUBROUTINE_MAX_FILE_PATH];	// +0x70 (utility)
	char		szIconModel[SUBROUTINE_MAX_FILE_PATH];	// +0xb0
	char		szIconSkin[SUBROUTINE_MAX_FILE_PATH];	// +0xf0
	char		szCursorFX[SUBROUTINE_MAX_FILE_PATH];	// +0x130

	// RETAIL: by version (alpha, beta, gold)
	LTVector	vEnergyDrain;						// +0x170 "EnergyDrain"

	// defense-specific fields (by version)
	LTVector	vDeflection;						// +0x17c
	LTVector	vAntivirus;							// +0x188
	uint8		nArmorPiece;						// +0x194
	char		szArmorTex[SUBROUTINE_MAX_FILE_PATH];	// +0x195
	char		szCorruptTex[SUBROUTINE_MAX_FILE_PATH];	// +0x1d5
};

struct PlayerSubroutine
{
	PlayerSubroutine();

	TronSubroutine * pTronSubroutine;	// +0x0
	SubVersion	eVersion;			// +0x4 alpha,beta,gold
	int			nTempSlot;			// +0x8 temp location until player compiles
	int			nSlot;				// +0xc slot in player's system memory, -1 for library
	float		fPercentDone;		// +0x10 Amount of work that a procedural has done to fix a condition
	SubState	eState;				// +0x14 okay, foreign, corrupt, unusable, deleted
	struct Procedural *pProcedural;	// RETAIL: +0x18 the procedural working on it (0x100950c0)
	int			nTronUnknown1C;		// RETAIL: +0x1c..+0x24, 0 in the ctor
	float		fTronUnknown20;		// RETAIL: +0x20 (a time: 0x10094f10)
	int			nTronUnknown24;

	// HACK for Base code so that we know which model to use.
	bool		bPrevious;			// +0x28
	bool		bNext;				// +0x29

	// PORT: build 69's, not in retail's
	bool		bErased;			// is this subroutine currently erased?
	float		fPercentCorrupt;	// Percentage that a subroutine is corrupt
	bool		bWorking;			// is a procedural working on this subroutine?
	bool		bActive;			// used by Utility Subroutines
};

typedef std::vector<PlayerSubroutine *> PlayerSubroutineArray;

struct Procedural
{
	Procedural();

	char				szName[SUBROUTINE_MAX_NAME];	// +0x0
	uint16				nNameId;						// +0x20
	uint16				nDescriptionId;					// +0x22
	uint16				nVerbId;						// +0x24 "VerbId"
	uint16				nCompletionId;					// +0x26 string to display when a procedural finishes
	int					iProcSlot;						// +0x28 1-5
	bool				bPlayerHasThis;					// +0x2c
	LTVector			vTimeRequired;					// +0x30 "Time" (seconds, by version)
	LTVector			vAdditionalTimeRequired;		// +0x3c "AdditionalTimeRequired"
	LTIntPt				CenterPos;						// +0x48 centerpoint of procedural
	int					iRadius;						// +0x50 size (radius) of control on screen
	char				szWorkSound[SUBROUTINE_MAX_NAME];		// +0x54
	char				szToolTipIcon[SUBROUTINE_MAX_FILE_PATH];	// +0x74
	char				szDecisionIcon[SUBROUTINE_MAX_FILE_PATH];	// +0xb4
	char				szIndicatorIcon[SUBROUTINE_MAX_FILE_PATH];	// +0xf4
	SubState			eAffectState;					// +0x134 "Effect" (0..3): what kinds of subroutines does this affect?
	PlayerSubroutine *	pSub;							// +0x138

	// PORT: build 69's, not in retail's (its code still reads them)
	float				fTimeRequired;	// time (in seconds) to perform a complete operation
	char				szIntroFX[SUBROUTINE_MAX_NAME];
	char				szIdleSkin[SUBROUTINE_MAX_FILE_PATH];
	char				szWorkSkin[SUBROUTINE_MAX_FILE_PATH];
	char				szConditionSkin[SUBROUTINE_MAX_FILE_PATH];
	char				szProgressSkin[SUBROUTINE_MAX_FILE_PATH];
};

typedef std::vector<Procedural *> ProceduralArray;

struct Primitive
{
	Primitive();

	char				szName[SUBROUTINE_MAX_NAME];	// +0x0
	uint16				nNameId;						// +0x20
	bool				bPlayerHasThis;					// +0x22
	char				szSprite[SUBROUTINE_MAX_FILE_PATH];	// +0x23
};

typedef std::vector<Primitive *> PrimitiveArray;

class CSubroutineMgr : public CGameButeMgr
{
public:
	CSubroutineMgr();
	virtual ~CSubroutineMgr();

    LTBOOL      Init(const char* szAttributeFile=SUBROUTINE_DEFAULT_FILE);
	void		Term();

	void		Pause(LTBOOL bPause); // pause and unpause the game
	void		Update();	// advance all timers
	PlayerSubroutine*	GetSubroutineInSector(int iSector);	// RETAIL: 0x100938f0 (the name is ours)

	uint16		GetNumSubroutines() {return m_SubroutineArray.size();}
	// RETAIL: inlined in CHUDSubChooser::Init (retail 0x1003b5a6); the name is ours
	TronSubroutine*	GetSubroutineByIndex(uint16 nIndex) {return m_SubroutineArray[nIndex];}

	// Search functions
	TronSubroutine*		GetSubroutine(const char *pszName);
	TronSubroutine*		GetSubroutineByNameId(uint32 nNameId);	// RETAIL: 0x10094720 (the name is ours)
	Primitive*			GetPrimitiveByNameId(uint32 nNameId);	// RETAIL: 0x10094760 (the name is ours)
	// RETAIL: 0x100947a0, the player's subroutine of that name (not in
	// pExcludeState, when given). The name is ours.
	PlayerSubroutine*	GetPlayerSubroutine(const char *pszName, SubState *pExcludeState);
	Procedural *		GetProcedural(const char * name);
	Procedural *		GetProcedural(int iProcNum) {return m_ProceduralArray[iProcNum];}
	
	// Subroutine functions
	void				ClearPlayerSubroutines();
	void				ResetPrimitives();		// RETAIL: 0x100948d0 (the name is ours)
	void				ResetProcedurals();		// RETAIL: 0x10094920 (the name is ours)
	PlayerSubroutine*	GetPlayerSubroutineByNameId(uint32 nNameId);	// RETAIL: 0x10094810 (the name is ours)
	void				AddPlayerSubroutine(PlayerSubroutine *pSub);
	void				GivePlayerSubroutine(char * name, char * state, char * condition);
	void				PopulateSubroutineScreen();

	// Utility Subroutine Functions
	bool				IsUtilitySubroutineActive(char * name);
	SubVersion			GetActiveSubroutineVersion(char * name);
	float				GetActiveSubroutineCorruption(char * name);
	// RETAIL: 0x10094e70, 0x10095340, 0x10094f10, 0x10094fb0 (names ours)
	bool				IsSubroutineInstalled(TronSubroutine *pTronSub);
	bool				IsSubroutineInstalled(const char *name);
	float				GetSubroutineCorruption(PlayerSubroutine *pSub);
	bool				IsSubroutineCorrupt(const char *name);
	// RETAIL: 0x10095730, 0x10095760 (names ours)
	float				GetProfilerDistance();
	float				GetCorruptionSpreadTime();

	// Procedural functions
	void				GivePlayerProcedural(char * name);
	bool				CanProcHookSub(Procedural *pProc, PlayerSubroutine *pSub);
	void				HookProcToSub(Procedural *pProc, PlayerSubroutine *pSub);
	Procedural *		WhoHookedToSub(PlayerSubroutine *pSub);
	bool				IsProceduralActive() {return m_bProceduralActive;}

	// Primitive functions
	void				GivePlayerPrimitive(char * name);
	bool				DoesPlayerHavePrimitive(char * name);

	// Gameplay-specific functions
	void				ComputeArmorCoverage();
	// RETAIL: the player's subroutine giving the piece (0x100951c0; CHUDArmor
	// reads its state), not a bool
	PlayerSubroutine*	HaveArmorPiece(int iPiece);
	TronSubroutine*		GetArmorSubroutine(int iPiece);	// RETAIL: 0x10095210 (the name is ours)

	void				Compile();

	// To set the system memory configuration (bad blocks, base code, etc...)
	bool				SetSystemMemoryConfiguration(const char* pSystemMemoryConfig, int nTronUnknown = -1);	// RETAIL: 0x100954b0
	void				AddBadBlock(int iSectorNumber);	// create a subroutine that's unusable
	void				AddBaseCodeBlock(int iSectorNumber, bool bPrev, bool bNext); // create a subroutine that's unusable

protected:
	void				CleanArrays();
	void				LoadSubroutines();	// RETAIL: 0x10092410 (Init's; the name is ours)
	void				LoadProcedurals();	// RETAIL: 0x100927d0 (Init's; the name is ours)

	void				ProceduralFinished(Procedural * pProc); // clean up a procedural operation

	// RETAIL: the layout follows retail's (ctor 0x10091f90, Init 0x10092200);
	// the names of the members build 69 lacks are ours
	typedef std::vector<TronSubroutine *> SubroutineArray;
	SubroutineArray m_SubroutineArray;				// +0x111c

	PlayerSubroutineArray m_PlayerSubroutineArray;	// +0x1128

	ProceduralArray m_ProceduralArray;				// +0x1134

	PrimitiveArray m_PrimitiveArray;				// +0x1140

	int					m_nTronUnknown114C;		// RETAIL: +0x114c (0x100954b0's second argument)
	LTBOOL				m_bPause;				// +0x1150
	float				m_fLastUpdateTime;		// +0x1154
	bool				m_bProceduralActive;	// +0x1158
	float				m_fTronUnknown115C;		// RETAIL: +0x115c (a time)
	int					m_nTronUnknown1160;		// RETAIL: +0x1160
	float				m_afProfilerDistance[3];	// RETAIL: +0x1164 ("Distance" of the profiler, by version)
	float				m_afCorruptionSpreadTime[4];	// RETAIL: +0x1170 ("Corruption" "SpreadTime", by difficulty)
	void*				m_pTronUnknown1180;		// RETAIL: +0x1180
	uint8				m_aTronUnknown1184[24];	// RETAIL: +0x1184 (one per system memory sector)
	uint8				m_aTronUnknown119C[4];	// RETAIL: +0x119c
	char				* m_SystemMemoryArray;	// RETAIL: +0x11a0 (0 in the ctor and Init)
	float				m_fTronUnknown11A4;		// RETAIL: +0x11a4 (0x10095390 sums into it)
	void*				m_pTronUnknown11A8;		// RETAIL: +0x11a8 (made by 0x10095df0, Init(&m_buteMgr))
};

#endif // !defined(_SUBROUTINE_MGR_H_)
