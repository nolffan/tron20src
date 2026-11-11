
// ----------------------------------------------------------------------- //
//
// MODULE  : ActivationData.h
//
// PURPOSE : Structure for handling activation data
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_ACTIVATION_DATA_H_)
#define _ACTIVATION_DATA_H_

#include "MsgIDs.h"
#include "LTObjRef.h"

class CActivationData
{
public:
	CActivationData() { Init(); }

	void Init()
	{
		m_vPos.Init();
		m_vDir.Init();
		m_nType = MID_ACTIVATE_NORMAL;
		m_hTarget = NULL;
		m_vIntersect.Init();
		m_nSurfaceType = NULL;
	}

	void Write(ILTMessage_Write *pMsg)
	{
		pMsg->WriteLTVector(m_vPos);
		pMsg->WriteLTVector(m_vDir);
		pMsg->Writeuint8(m_nType);
		pMsg->WriteObject(m_hTarget);
		pMsg->WriteLTVector(m_vIntersect);
		pMsg->Writeuint8(m_nSurfaceType);
	}

	void Read(ILTMessage_Read *pMsg)
	{
		m_vPos			= pMsg->ReadLTVector();
		m_vDir			= pMsg->ReadLTVector();
		m_nType			= pMsg->Readuint8();
		m_hTarget		= pMsg->ReadObject();
		m_vIntersect	= pMsg->ReadLTVector();
		m_nSurfaceType	= pMsg->Readuint8();
	}

	LTVector	m_vPos;
	LTVector	m_vDir;	// RETAIL: Tron's, as build 51's (CShell 0x10128180 writes a vector)
	uint8		m_nType;
	LTObjRef	m_hTarget;
	LTVector	m_vIntersect;
	uint8		m_nSurfaceType;

};


#endif