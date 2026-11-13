//-------------------------------------------------------------------------
//
// MODULE  : MenuPieceModel.cpp
//
// PURPOSE : A model of the interface (a layout's MenuPiece), attachable
//			 to a socket of another, and the list that keeps them attached
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1004c9a0..
// 0x1004dd3b; MenuPieceModel.h).
//
//-------------------------------------------------------------------------

#include "stdafx.h"
#include "MenuPieceModel.h"
#include "InterfaceMgr.h"

CMenuPieceModelMgr * g_pMenuPieceModelMgr = LTNULL;

CMenuPieceModel::CMenuPieceModel()
{
	m_bCreated = false;
	m_bShown = false;
	m_bPaused = false;
	m_bLooping = false;
	m_bUseSocketRotation = true;
	m_pFX = LTNULL;
	m_hObject = LTNULL;
	m_hParent = LTNULL;
	m_szParentSocket[0] = 0;
}

void CMenuPieceModel::Create(char * szPieceName)
{
	if (m_bCreated) return;
	if (!szPieceName || !szPieceName[0]) return;

	INT_MENUPIECE * pPiece = ((CTronLayoutMgr *)g_pLayoutMgr)->GetMenuPiece(szPieceName);
	if (pPiece)
	{
		Create(pPiece);
	}
}

// RETAIL: Build 69's CScreenSubroutines::CreateMenuPiece, made a member
// of its own
void CMenuPieceModel::Create(INT_MENUPIECE * pPiece)
{
	if (!pPiece) return;

	m_pFX = LTNULL;
	m_hObject = LTNULL;

	HOBJECT hCamera = g_pInterfaceMgr->GetInterfaceCamera();
	if (!hCamera) return;

	LTRotation rRot;
	BSCREATESTRUCT bcs;
	LTVector vPos, vTemp, vScale(1.0f,1.0f,1.0f);

	g_pLTClient->GetObjectPos(hCamera, &vPos);
	g_pLTClient->GetObjectRotation(hCamera, &rRot);

	char modName[128];

	SAFE_STRCPY(modName, pPiece->szModel);

	LTVector vU = rRot.Up();
	LTVector vR = rRot.Right();
	LTVector vF = rRot.Forward();

	VEC_SET(vScale,1.0f,1.0f,1.0f);

	LTVector vModPos = pPiece->vPos;

	LTFLOAT fRot = 0.0f;
	fRot  = MATH_PI + DEG2RAD(fRot);
	rRot.Rotate(vU, fRot);

	VEC_MULSCALAR(vTemp, vF, vModPos.z);
	VEC_ADD(vPos, vPos, vTemp);

	VEC_MULSCALAR(vTemp, vR, vModPos.x);
	VEC_ADD(vPos, vPos, vTemp);

	VEC_MULSCALAR(vTemp, vU, vModPos.y);
	VEC_ADD(vPos, vPos, vTemp);

	VEC_COPY(bcs.vPos, vPos);
	bcs.rRot = rRot;
	VEC_COPY(bcs.vInitialScale, vScale);
	VEC_COPY(bcs.vFinalScale, vScale);
	VEC_SET(bcs.vInitialColor, 1.0f, 1.0f, 1.0f);
	VEC_SET(bcs.vFinalColor, 1.0f, 1.0f, 1.0f);
	bcs.bUseUserColors = LTTRUE;

	bcs.pFilename = modName;
	bcs.pSkinReader = &(pPiece->blrSkins);
	bcs.pRenderStyleReader = &(pPiece->blrRenderStyles);
	bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE;
	bcs.nMenuLayer = (uint8)pPiece->nLayer;

	bcs.nType = OT_MODEL;
	bcs.fInitialAlpha = 0.99f;
	bcs.fFinalAlpha = 0.99f;
	bcs.fLifeTime = 1000000.0f;
	bcs.bLoop = LTFALSE;			// all models start out as non-looping

	bcs.fMinRotateVel = 1.0f;
	bcs.fMaxRotateVel = 1.0f;

	m_pFX = debug_new(CBaseScaleFX);

	if (!m_pFX->Init(&bcs))
	{
		debug_delete(m_pFX);
		m_pFX = LTNULL;
		return;
	}

	m_pFX->CreateObject(g_pLTClient);
	m_hObject = m_pFX->GetObject();
	if (m_hObject)
	{
		if (pPiece->szIntroAnim[0])
		{
			g_pLTClient->SetModelAnimation(m_hObject, g_pLTClient->GetAnimIndex(m_hObject, pPiece->szIntroAnim));
		}
		else if (pPiece->szLoopAnim[0])
		{
			g_pLTClient->SetModelAnimation(m_hObject, g_pLTClient->GetAnimIndex(m_hObject, pPiece->szLoopAnim));
		}
	}

	SAFE_STRCPY(m_szName, pPiece->szName);

	if (g_pMenuPieceModelMgr)
	{
		g_pMenuPieceModelMgr->Add(this);
	}

	m_bCreated = true;
	m_szNextAnim[0] = 0;

	Attach(pPiece->szParentPiece, pPiece->szParentSocket);
}

void CMenuPieceModel::Term()
{
	if (!m_bCreated) return;

	Show(false);

	if (m_pFX)
	{
		m_pFX->Reset();
		m_pFX->Term();
		debug_delete(m_pFX);
		m_pFX = LTNULL;
	}

	m_hObject = LTNULL;
	m_hParent = LTNULL;
	m_szParentSocket[0] = 0;
	m_szName[0] = 0;
	m_bCreated = false;

	if (g_pMenuPieceModelMgr)
	{
		g_pMenuPieceModelMgr->Remove(this);
	}
}

void CMenuPieceModel::Update()
{
	if (!m_bCreated || !m_hParent || !m_hObject || !m_szParentSocket[0]) return;

	HMODELSOCKET hSocket;
	LTransform transform;
	g_pModelLT->GetSocket(m_hParent, m_szParentSocket, hSocket);
	if (g_pModelLT->GetSocketTransform(m_hParent, hSocket, transform, true) == LT_OK)
	{
		if (m_bUseSocketRotation)
		{
			g_pLTClient->SetObjectPosAndRotation(m_hObject, &transform.m_Pos, &transform.m_Rot);
		}
		else
		{
			LTRotation rRot(0.0f, MATH_PI, 0.0f);
			g_pLTClient->SetObjectPosAndRotation(m_hObject, &transform.m_Pos, &rRot);
		}
	}

	if (m_szNextAnim[0])
	{
		uint32 dwState = 0;
		ANIMTRACKERID nTracker;
		g_pLTClient->GetModelLT()->GetMainTracker(m_hObject, nTracker);
		g_pLTClient->GetModelLT()->GetPlaybackState(m_hObject, nTracker, dwState);
		if (dwState & MS_PLAYDONE)
		{
			uint32 nAnim = g_pLTClient->GetAnimIndex(m_hObject, m_szNextAnim);
			g_pLTClient->SetModelAnimation(m_hObject, nAnim);
			m_bLooping = true;
			g_pLTClient->SetModelLooping(m_hObject, true);
			m_szNextAnim[0] = 0;
		}
	}
}

void CMenuPieceModel::Show(bool bShow)
{
	if (!m_bCreated) return;
	if (m_bShown == bShow) return;

	m_bShown = bShow;
	if (bShow)
	{
		g_pInterfaceMgr->AddInterfaceSFX(m_pFX, IFX_MENU_ATTACH);
	}
	else
	{
		g_pInterfaceMgr->RemoveInterfaceSFX(m_pFX);
	}
}

void CMenuPieceModel::Pause(bool bPause)
{
	m_bPaused = bPause;

	ANIMTRACKERID nTracker;
	if (g_pModelLT->GetMainTracker(m_hObject, nTracker) == LT_OK)
	{
		g_pModelLT->SetPlaying(m_hObject, nTracker, !m_bPaused);
	}
}

bool CMenuPieceModel::PlayAnim(char * szAnim, bool bLoop, bool bPause)
{
	if (!m_bCreated) return false;
	if (!szAnim) return false;
	if (!szAnim[0]) return false;

	m_szNextAnim[0] = 0;

	if (m_hObject)
	{
		uint32 nAnim = g_pLTClient->GetAnimIndex(m_hObject, szAnim);
		g_pLTClient->SetModelAnimation(m_hObject, nAnim);
		if (nAnim != INVALID_MODEL_ANIM)
		{
			m_bLooping = bLoop;
			g_pLTClient->SetModelLooping(m_hObject, bLoop);

			m_bPaused = bPause;
			ANIMTRACKERID nTracker;
			if (g_pModelLT->GetMainTracker(m_hObject, nTracker) == LT_OK)
			{
				g_pModelLT->SetPlaying(m_hObject, nTracker, !m_bPaused);
			}
		}
	}

	return true;
}

void CMenuPieceModel::SetNextAnim(char * szAnim)
{
	SAFE_STRCPY(m_szNextAnim, szAnim);
}

void CMenuPieceModel::SetAnimTime(uint32 nTime)
{
	if (!m_bCreated) return;
	if (!m_hObject) return;

	ANIMTRACKERID nTracker;
	if (g_pModelLT->GetMainTracker(m_hObject, nTracker) == LT_OK)
	{
		m_bPaused = true;
		g_pModelLT->SetCurAnimTime(m_hObject, nTracker, nTime);
		g_pModelLT->SetPlaying(m_hObject, nTracker, !m_bPaused);
	}
}

bool CMenuPieceModel::IsAnimDone()
{
	if (!m_bCreated || m_bPaused || m_bLooping) return true;

	uint32 dwState = 0;
	ANIMTRACKERID nTracker;
	g_pLTClient->GetModelLT()->GetMainTracker(m_hObject, nTracker);
	g_pLTClient->GetModelLT()->GetPlaybackState(m_hObject, nTracker, dwState);
	return (dwState & MS_PLAYDONE) ? true : false;
}

void CMenuPieceModel::Attach(char * szParentName, char * szSocketName)
{
	if (!m_bCreated) return;

	m_hParent = LTNULL;
	m_szParentName[0] = 0;
	m_szParentSocket[0] = 0;

	if (!szParentName || !szSocketName || !szParentName[0] || !szSocketName[0]) return;

	SAFE_STRCPY(m_szParentSocket, szSocketName);
	SAFE_STRCPY(m_szParentName, szParentName);

	if (g_pMenuPieceModelMgr)
	{
		CMenuPieceModel * pParent = g_pMenuPieceModelMgr->Find(szParentName);
		if (pParent)
		{
			m_hParent = pParent->m_hObject;
		}
	}
}


CMenuPieceModelMgr::CMenuPieceModelMgr()
{
	g_pMenuPieceModelMgr = LTNULL;
}

CMenuPieceModelMgr::~CMenuPieceModelMgr()
{
	Deactivate();
}

void CMenuPieceModelMgr::Activate()
{
	if (g_pMenuPieceModelMgr) return;

	g_pMenuPieceModelMgr = this;
	m_Models.clear();
}

void CMenuPieceModelMgr::Deactivate()
{
	if (!g_pMenuPieceModelMgr) return;

	g_pMenuPieceModelMgr = LTNULL;
	m_Models.clear();
}

void CMenuPieceModelMgr::Update()
{
	for (ModelArray::iterator iter = m_Models.begin(); iter != m_Models.end(); iter++)
	{
		if ((*iter)->m_bShown)
		{
			(*iter)->Update();
		}
	}
}

void CMenuPieceModelMgr::Add(CMenuPieceModel * pModel)
{
	if (!pModel) return;

	for (ModelArray::iterator iter = m_Models.begin(); iter != m_Models.end(); iter++)
	{
		if (pModel == *iter) return;
	}

	m_Models.push_back(pModel);
}

void CMenuPieceModelMgr::Remove(CMenuPieceModel * pModel)
{
	if (!pModel) return;

	ModelArray::iterator iter;
	for (iter = m_Models.begin(); iter != m_Models.end(); iter++)
	{
		if (pModel == *iter)
		{
			m_Models.erase(iter);
			break;
		}
	}

	// let go of its children
	for (iter = m_Models.begin(); iter != m_Models.end(); iter++)
	{
		CMenuPieceModel * pChild = *iter;
		if (!_strcmpi(pChild->m_szParentName, pModel->m_szName))
		{
			pChild->Attach(LTNULL, LTNULL);
		}
	}
}

CMenuPieceModel * CMenuPieceModelMgr::Find(char * szName)
{
	if (!szName || !szName[0]) return LTNULL;

	for (ModelArray::iterator iter = m_Models.begin(); iter != m_Models.end(); iter++)
	{
		if (!_strcmpi(szName, (*iter)->m_szName))
		{
			return *iter;
		}
	}

	return LTNULL;
}
