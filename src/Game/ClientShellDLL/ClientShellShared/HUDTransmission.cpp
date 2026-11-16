// ----------------------------------------------------------------------- //
//
// MODULE  : HUDTransmission.cpp
//
// PURPOSE : Implementation of CHUDTransmission to display messages
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDTransmission.h"
#include "InterfaceMgr.h"

CHUDTransmission::CHUDTransmission()
{
	m_UpdateFlags = kHUDFrame;
	m_eLevel = kHUDRenderText;
}
	

LTBOOL CHUDTransmission::Init()
{
	UpdateLayout();

	MsgCreate fmt = m_MsgFormat;
	fmt.sString = " ";
	fmt.fDuration = 0.0f;
	m_Msg.Create(fmt);

	return LTTRUE;

}
void CHUDTransmission::Term()
{
	m_Msg.Destroy();
}

void CHUDTransmission::Render()
{
	if (!m_Msg.IsVisible()) return;
	m_Msg.Render();
}

void CHUDTransmission::Update()
{
	if (!m_Msg.IsVisible()) return;
	m_Msg.Update();

}

void CHUDTransmission::Show(int nMessageID)
{
	Show(LoadTempString(nMessageID));
}

void CHUDTransmission::Show(const char *pszString)
{
	MsgCreate fmt = m_MsgFormat;
	fmt.sString = pszString;
	m_Msg.Create(fmt);
	m_Msg.Show(LTTRUE);


	// RETAIL: Tron's (0x10187cd4) takes the message's base size from the
	// interface ratios; the width is not used
	uint16 nBaseWidth = (uint16)((float)m_Msg.GetWidth() / g_pInterfaceResMgr->GetXRatio());
	uint16 nBaseHeight = (uint16)((float)m_Msg.GetHeight() / g_pInterfaceResMgr->GetYRatio());

	LTIntPt pos = m_BasePos;
	pos.y -= nBaseHeight/2;

	m_Msg.SetBasePos(pos);
}

void CHUDTransmission::UpdateLayout()
{
	char *pTag = "Transmission";

	// RETAIL: Tron's (0x10187da0): the disc game types have their own layout
	if (!IsDiscGameType())
	{
		m_BasePos = g_pLayoutMgr->GetPoint(pTag,"BasePos");

		uint8 nFont = (uint8)g_pLayoutMgr->GetInt(pTag,"Font");

		m_MsgFormat.pFont = g_pInterfaceResMgr->GetFont(nFont);
		m_MsgFormat.nFontSize = (uint8)g_pLayoutMgr->GetInt(pTag,"FontSize");

		LTVector vCol = g_pLayoutMgr->GetVector(pTag,"TextColor");
		uint8 nR = (uint8)vCol.x;
		uint8 nG = (uint8)vCol.y;
		uint8 nB = (uint8)vCol.z;
		m_MsgFormat.nTextColor =  SET_ARGB(0xFF,nR,nG,nB);
		m_MsgFormat.fDuration = g_pLayoutMgr->GetFloat(pTag,"Time");
		m_MsgFormat.fFadeDur  = g_pLayoutMgr->GetFloat(pTag,"FadeTime");
		m_MsgFormat.nWidth  = (uint16) g_pLayoutMgr->GetInt(pTag,"Width");
	}
	else
	{
		m_BasePos = g_pLayoutMgr->GetPoint(pTag,"DiscMPBasePos");

		uint8 nFont = (uint8)g_pLayoutMgr->GetInt(pTag,"DiscMPFont");

		m_MsgFormat.pFont = g_pInterfaceResMgr->GetFont(nFont);
		m_MsgFormat.nFontSize = (uint8)g_pLayoutMgr->GetInt(pTag,"DiscMPFontSize");

		LTVector vCol = g_pLayoutMgr->GetVector(pTag,"DiscMPTextColor");
		uint8 nR = (uint8)vCol.x;
		uint8 nG = (uint8)vCol.y;
		uint8 nB = (uint8)vCol.z;
		m_MsgFormat.nTextColor =  SET_ARGB(0xFF,nR,nG,nB);
		m_MsgFormat.fDuration = g_pLayoutMgr->GetFloat(pTag,"DiscMPTime");
		m_MsgFormat.fFadeDur  = g_pLayoutMgr->GetFloat(pTag,"DiscMPFadeTime");
		m_MsgFormat.nWidth  = (uint16) g_pLayoutMgr->GetInt(pTag,"DiscMPWidth");
	}
	m_MsgFormat.eJustify = kMsgCenter;

}
