//-------------------------------------------------------------------------
//
// MODULE  : MenuPieceModel.h
//
// PURPOSE : A model of the interface (a layout's MenuPiece), attachable
//			 to a socket of another, and the list that keeps them attached
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1004c9a0..
// 0x1004dd3b). Neither class has a vtable, so no name survives: the class
// and member names are ours. The subroutine screens build their models
// with it (ArcCtrl's TU at 0x10001xxx, the procedural and optimizer
// controls at 0x1004e460.., the subroutine screen's list at its +0x3cd4).
//
//-------------------------------------------------------------------------

#ifndef __MENU_PIECE_MODEL_H
#define __MENU_PIECE_MODEL_H

#include "BaseScaleFX.h"
#include "TronLayoutMgr.h"

class CMenuPieceModelMgr;

class CMenuPieceModel
{
public:
	CMenuPieceModel();								// RETAIL: 0x1004c9a0
	~CMenuPieceModel() { Term(); }					// RETAIL: 0x1004c9d0

	void		Create(char * szPieceName);			// RETAIL: 0x1004c9e0
	void		Create(INT_MENUPIECE * pPiece);		// RETAIL: 0x1004ca20
	void		Term();								// RETAIL: 0x1004d4d0

	void		Update();							// RETAIL: 0x1004d530: follows its parent's socket
	void		Show(bool bShow);					// RETAIL: 0x1004d7b0
	void		Pause(bool bPause);					// RETAIL: 0x1004d7f0
	bool		PlayAnim(char * szAnim, bool bLoop, bool bPause);	// RETAIL: 0x1004d840
	void		SetNextAnim(char * szAnim);			// RETAIL: 0x1004d900: looped once this one ends
	void		SetAnimTime(uint32 nTime);			// RETAIL: 0x1004d920: paused there
	bool		IsAnimDone();						// RETAIL: 0x1004d990
	void		Attach(char * szParentName, char * szSocketName);	// RETAIL: 0x1004da00

	HOBJECT		GetObject()		{return m_hObject;}
	CBaseScaleFX * GetFX()		{return m_pFX;}
	bool		IsCreated()		{return m_bCreated;}
	bool		IsShown()		{return m_bShown;}
	char *		GetName()		{return m_szName;}
	void		SetUseSocketRotation(bool bUse) {m_bUseSocketRotation = bUse;}

private:
	friend class CMenuPieceModelMgr;

	HOBJECT			m_hObject;				// +0x00
	bool			m_bCreated;				// +0x04
	bool			m_bShown;				// +0x05: added to the interface's FX
	bool			m_bPaused;				// +0x06
	bool			m_bLooping;				// +0x07
	bool			m_bUseSocketRotation;	// +0x08: else turned to face the camera
	CBaseScaleFX *	m_pFX;					// +0x0c
	HOBJECT			m_hParent;				// +0x10
	char			m_szName[64];			// +0x14: the piece's name
	char			m_szParentName[64];		// +0x54
	char			m_szParentSocket[32];	// +0x94
	char			m_szNextAnim[32];		// +0xb4
};

// RETAIL: the subroutine screen's (its +0x3cd4); the active one is
// retail's global 0x1026de24
class CMenuPieceModelMgr
{
public:
	CMenuPieceModelMgr();							// RETAIL: 0x1004da90
	~CMenuPieceModelMgr();							// RETAIL: 0x1004dab0

	void		Activate();							// RETAIL: 0x1004db60
	void		Deactivate();						// RETAIL: 0x1004dba0
	void		Update();							// RETAIL: 0x1004dbe0

	void		Add(CMenuPieceModel * pModel);		// RETAIL: 0x1004dc10
	void		Remove(CMenuPieceModel * pModel);	// RETAIL: 0x1004dc60
	CMenuPieceModel * Find(char * szName);			// RETAIL: 0x1004dcf0

private:
	typedef std::vector<CMenuPieceModel *> ModelArray;
	ModelArray		m_Models;
};

extern CMenuPieceModelMgr * g_pMenuPieceModelMgr;	// RETAIL: 0x1026de24

#endif // __MENU_PIECE_MODEL_H
