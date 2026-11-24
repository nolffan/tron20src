// ----------------------------------------------------------------------- //
//
// MODULE  : TargetMgr.h
//
// PURPOSE : Definition of class to handle tracking whjat the player is aimed at.
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TARGET_MGR_H__
#define __TARGET_MGR_H__

#include "ActivationData.h"
#include "DamageTypes.h"


// RETAIL: Tron's target string is 1024 bytes (CTargetMgr's m_hLockedTarget
// is at +0x410, its m_hTarget at +0xc: ctor 0x1016d5f0)
const uint16	kMaxTargetStringLen = 1024;
const uint16 kMaxDebugStringLen = 1024;

// RETAIL: Tron's target string IDs keep build 51's numbers (NOLF2 1.3's
// ClientResShared.h moved them to 944 and up): CTargetMgr::Update
// (0x1016d700) and CTronTargetMgr::Update (0x100ae5f0) push these. Tron has
// no target lighter, thus "searching" is 464, not build 51's 465. Retail's
// CRes.dll has no strings for them. The names are ours
const uint16	kTronIDS_TargetSearch		= 449;
const uint16	kTronIDS_TargetTalk			= 452;
const uint16	kTronIDS_TargetUse			= 453;
const uint16	kTronIDS_TargetTake			= 454;
const uint16	kTronIDS_TargetSearching	= 464;
const uint16	kTronIDS_TargetRevive		= 469;
const uint16	kTronIDS_TargetWakeUp		= 502;

class CTargetMgr 
{
public:
	CTargetMgr();
	virtual ~CTargetMgr();

	virtual void Update();

	virtual HOBJECT GetTargetObject()		const	{return m_hTarget;}
	virtual float GetTargetRange()			const	{return m_fTargetRange;}
	virtual uint16	GetTargetStringID()		const	{return m_nString;}
	virtual	const char*	GetTargetString()	const	{return &m_szString[0];}
	virtual	const char*	GetDebugString()	const	{return &m_szDebugString[0];}
	uint8			GetTargetTeam()			const	{return m_nTargetTeam;}

	//instersect info to be sent to server
	virtual CActivationData GetActivationData() const {return m_ActivationData;}

	virtual	bool	IsSearchTarget()				{return m_bSearchTarget;}
	virtual	bool	IsMoveTarget()					{return m_bMoveTarget;}
	virtual bool	CanActivateTarget()				{return m_bCanActivate;}
	// RETAIL: inline in Tron (slot 11, 0x100ae580)
	virtual bool	IsTargetInRange()	{return (m_hTarget && m_fTargetRange <= m_fActivationDistance);}

	


	virtual bool IsGadgetActivatable(HOBJECT hObj);

	// RETAIL: no NOLF2 gadget queries (IsTargetGadgetActivatable,
	// RequiredGadgetDamageType): SetGadgetTarget tests the current ammo

	//make sure specified object is considered a valid target regardless of it's current flags
	virtual void LockTarget(HOBJECT hTarget) { m_hLockedTarget = hTarget; }
	virtual HOBJECT GetLockedTarget()	const	{return m_hLockedTarget;}

protected:

	// RETAIL: Tron's: Update (0x1016d700) keeps the "ActivationDistance"
	// and "TargetDistance" console values here (+0x4, +0x8); the
	// constructor (0x1016d5f0) leaves them alone. The names are ours
	float		m_fActivationDistance;
	float		m_fTargetDistance;

	HOBJECT		m_hTarget;
	char		m_szString[kMaxTargetStringLen];

	void		CheckForIntersect(float &fDistAway);

	void		SetTargetStringID(uint16 nID);
	void		SetGadgetTarget( bool bDisabling );
		
	
	HOBJECT		m_hLockedTarget;
	float		m_fTargetRange;

	char		m_szDebugString[kMaxDebugStringLen];
	uint16		m_nString;

	bool		m_bSearchTarget;
	bool		m_bMoveTarget;
	bool		m_bCanActivate;
	// PORT: NOLF2's; Tron's constructor (build 51's) sets neither, and Tron's Update
	// uses neither (m_bFirstUpdate is left unused). Retail
	// has room for them in the padding at +0x81d..+0x81f
	bool		m_bFirstUpdate;
	uint8		m_nTargetTeam;

	//intersect info
	CActivationData m_ActivationData;	// RETAIL: +0x820

	//check for character FX within an are around a point
	bool CheckForCharacters(LTVector vObjPos,LTVector vDims, uint8 nId);

	// RETAIL: no FirstUpdate or ClearTargetInfo in Tron (Update clears
	// the target info itself)
};

#endif