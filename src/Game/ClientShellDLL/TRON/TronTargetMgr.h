// ----------------------------------------------------------------------- //
//
// MODULE  : TronTargetMgr.h
//
// PURPOSE : Definition of class to handle tracking what the player is aimed at.
//           Derived from CTargetMgr
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TRON_TARGET_MGR_H__
#define __TRON_TARGET_MGR_H__

#include "TargetMgr.h"

class CTronTargetMgr : public CTargetMgr
{
public:
	CTronTargetMgr();
	virtual ~CTronTargetMgr();

	void Update();
	void HandleTargetPropertiesMessage(ILTMessage_Read *pMsg);

	bool IsEnergyRequired() {return m_bEnergyRequired;}
	int		GetEnergyRequired() {return m_nEnergyRequired;}

	// RETAIL: Tron's (retail +0x874, +0x875: a permission set the target
	// needs, CTronPlayerMgr::DoActivate, 0x100a7a20; the names are ours)
	bool	IsPermissionRequired() {return m_bPermissionRequired;}
	uint8	GetPermissionRequired() {return m_nPermissionRequired;}

	// RETAIL: retail 0x100afa90 (the name is ours): the target's
	// properties (+0x878..+0x895) go
	void	ClearTargetProperties();

	// RETAIL: retail's +0x876: the crosshair shows its "innocent" reticle
	// while it is set (CHUDCrosshair::RenderArmed, 0x10024aae); the name is
	// ours, and the member is at the end of ours until the class's layout
	// is retail's
	bool	IsTronUnknown876() {return m_bTronUnknown876;}
private:
	// RETAIL: CTronTargetMgr is 0xca0 bytes; CTargetMgr ends at +0x85c.
	// HandleTargetPropertiesMessage (0x100af220) sets the four flags
	// +0x85c..+0x85f with their values +0x864..+0x870; the first pair is
	// the energy, the others are not yet named
	bool		m_bEnergyRequired;			// +0x85c
	bool		m_bTronUnknown85D;
	bool		m_bTronUnknown85E;
	bool		m_bTronUnknown85F;
	bool		m_bTronUnknown860;			// RETAIL: read before each of the four
	int			m_nEnergyRequired;			// +0x864
	int			m_nTronUnknown868;
	int			m_nTronUnknown86C;
	int			m_nTronUnknown870;
	// RETAIL: see IsPermissionRequired (the names are ours)
	bool		m_bPermissionRequired;		// +0x874
	uint8		m_nPermissionRequired;		// +0x875
	bool		m_bTronUnknown876;			// RETAIL: +0x876 (see IsTronUnknown876)
	bool		m_bTronUnknown877;
	// RETAIL: +0x878..+0xc9c: the target's properties. Retail builds them
	// in a member constructor (its vtable store comes last in 0x100ae410);
	// the struct and its names are ours
	struct TronTargetProps
	{
		TronTargetProps()
		{
			anTronUnknown0[0] = 0;
			anTronUnknown0[1] = 0;
			anTronUnknown0[2] = 0;
			anTronUnknown0[3] = 0;
			anTronUnknown0[4] = 0;
			anTronUnknown0[5] = 0;
			anTronUnknown0[6] = 0;
			anTronUnknown0[7] = 0;
			vTronUnknown10.Init();
			szText[0] = 0;
			bTronUnknown424 = false;
		}

		uint16		anTronUnknown0[8];		// +0x878
		LTVector	vTronUnknown10;			// +0x888
		uint8		nTronUnknown1C;			// +0x894
		// RETAIL: +0x895: the properties' text (strncat to 0x3ff
		// characters, 0x100af594)
		char		szText[1024];
		uint32		nTronUnknown420;		// +0xc98
		bool		bTronUnknown424;		// +0xc9c
	};
	TronTargetProps	m_Props;
};

#endif