// ----------------------------------------------------------------------- //
//
// MODULE  : ContinuousFireHelper.h
//
// PURPOSE : Tron's server helper for the continuous fire weapons (PRod,
//           EnergyClaw). Retail only: RTTI .?AVCContinuousFireHelper@@,
//           vtable 0x1029966c; Object.lto 0x101606f0..0x10161670.
//           CCharacter owns one per weapon (CPRodFireHelper,
//           CEnergyClawFireHelper derive from it). Member and function
//           names are ours where not said otherwise.
//
// ----------------------------------------------------------------------- //

#ifndef  _ContinuousFireHelper_h_INCLUDED_
#define  _ContinuousFireHelper_h_INCLUDED_

#include "ltbasedefs.h"
#include <list>

struct WEAPON;
struct AMMO;

// RETAIL: an 8 byte value the fire data sends with WriteType (aligned to
// 4, so not a uint64). What it holds is not known.
struct ContinuousFireValue8
{
	uint32	m_nTronUnknown0;
	uint32	m_nTronUnknown4;
};

// RETAIL: blobs of the fire data's union, by their sizes in bytes.
struct ContinuousFireBlob16  { uint8 m_aData[16]; };
struct ContinuousFireBlob12  { float m_aData[3]; };
struct ContinuousFireBlob148 { uint8 m_aData[148]; };
struct ContinuousFireBlob20  { uint8 m_aData[20]; };
struct ContinuousFireBlob20b { uint8 m_aData[20]; };
struct ContinuousFireBlob140 { uint8 m_aData[140]; };

// RETAIL: 0xcc bytes (CContinuousFireHelper +0x28; Read 0x10161400,
// Write 0x10161530). Eleven optional fields, each with a flag pair; only
// the first flag of a pair says if the field is sent. The first eight
// fields share one union (all of them are read and written at +0x18).
struct ContinuousFireData
{
	enum { kNumFields = 11 };

	struct FieldFlags
	{
		bool	m_bSet;
		bool	m_bTronUnknown1;
	};

	FieldFlags	m_aFlags[kNumFields];
	uint8		m_aPad16[2];

	union
	{
		ContinuousFireBlob16	m_Field0;
		ContinuousFireBlob148	m_Field1;
		ContinuousFireBlob16	m_Field2;
		ContinuousFireBlob12	m_Field3;	// RETAIL: sent as 96 bits (a vector?)
		ContinuousFireBlob20	m_Field4;
		ContinuousFireBlob20	m_Field5;
		ContinuousFireBlob20b	m_Field6;
		ContinuousFireBlob140	m_Field7;
	};

	ContinuousFireValue8	m_Field8;
	LTVector				m_Field9;
	ContinuousFireValue8	m_Field10;
};

class CContinuousFireHelper
{
public:

	CContinuousFireHelper();
	~CContinuousFireHelper();

	// RETAIL: slot 0, 0x10160b50
	virtual void	Update();

protected:

	// RETAIL: slot 1 (0x100e2900, an empty one): extra data of the fire
	// message.
	virtual void	WriteExtra(ILTMessage_Write *pMsg) {}

	// RETAIL: slot 2 (0x10061ac0, "return 0"): the fire message's ID.
	virtual uint8	GetMessageID() { return 0; }

public:

	void	Init(HOBJECT hObject);						// RETAIL: 0x10160870
	void	Save(ILTMessage_Write *pMsg);				// RETAIL: 0x101608b0
	void	Load(ILTMessage_Read *pMsg);				// RETAIL: 0x101609b0

	void	SetTronUnknown18(uint8 nValue);				// RETAIL: 0x10160b00
	void	SetWeapon(uint8 nWeaponId);					// RETAIL: 0x10160b10
	void	SetAmmo(uint8 nAmmoId);						// RETAIL: 0x10160b30

	bool	AddTarget(HOBJECT hTarget);					// RETAIL: 0x10160ba0
	bool	RemoveTarget(HOBJECT hTarget);				// RETAIL: 0x10160c70
	bool	GetFirePos(LTVector *pvPos);				// RETAIL: 0x10160cb0

	bool	Start(ILTMessage_Read *pMsg);				// RETAIL: 0x10160d50
	void	Stop();										// RETAIL: 0x10160de0

protected:

	bool	UpdatePlayer();								// RETAIL: 0x10160e50
	bool	UpdateAI();									// RETAIL: 0x101610f0
	void	RemoveDeadTargets();						// RETAIL: 0x10161380
	void	ReleaseTarget(HOBJECT hTarget);				// RETAIL: 0x101613c0

	void	ReadFireData(ILTMessage_Read *pMsg);		// RETAIL: 0x10161400
	void	WriteFireData(ILTMessage_Write *pMsg);		// RETAIL: 0x10161530

	typedef std::list<LTObjRef> TargetList;

	LTObjRef			m_hObject;			// +0x04, the firing character
	TargetList			m_lstTargets;		// +0x14
	uint8				m_nTronUnknown18;	// +0x18
	bool				m_bOn;				// +0x19
	WEAPON const*		m_pWeapon;			// +0x1c
	AMMO const*			m_pAmmo;			// +0x20
	float				m_fTronUnknown24;	// +0x24
	ContinuousFireData	m_FireData;			// +0x28
};

// RETAIL: the two helpers CCharacter owns (CCharacter +0xc64, +0xc68).
// Their code is outside this file (Object.lto 0x10194160..0x10194320):
// declared here for CCharacter.

// RETAIL: RTTI .?AVCPRodFireHelper@@, vtable 0x1029a980 (constructor
// 0x10194300; GetMessageID 0x101942f0 returns 6).
class CPRodFireHelper : public CContinuousFireHelper
{
public:
	CPRodFireHelper();

protected:
	virtual uint8	GetMessageID();
};

// RETAIL: RTTI .?AVCEnergyClawFireHelper@@, vtable 0x1029a970 (constructor
// 0x10194170; Update 0x10194190, WriteExtra 0x101942c0, GetMessageID
// 0x10194160 returns 13).
class CEnergyClawFireHelper : public CContinuousFireHelper
{
public:
	CEnergyClawFireHelper();

	virtual void	Update();

protected:
	virtual void	WriteExtra(ILTMessage_Write *pMsg);
	virtual uint8	GetMessageID();
};

#endif //_ContinuousFireHelper_h_INCLUDED_
