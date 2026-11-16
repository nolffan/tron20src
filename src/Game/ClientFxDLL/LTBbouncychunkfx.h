//------------------------------------------------------------------
//
//   MODULE  : LTBBOUNCYCHUNKFX.H
//
//   PURPOSE : Defines class CLTBBouncyChunkFX
//
//   CREATED : On 12/3/98 At 6:34:45 PM
//
//------------------------------------------------------------------

#ifndef __LTBBOUNCYCHUNKFX__H_
	#define __LTBBOUNCYCHUNKFX__H_

	// Includes....

	#include "basefx.h"

	class CLTBBouncyChunkProps : public CBaseFXProps
	{
	public:
	
		CLTBBouncyChunkProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		// RETAIL: Tron's layout (CLTBBouncyChunkProps::CLTBBouncyChunkProps
		// 0x1000f670, ParseProperties 0x1000f730 in ClientFx.fxd): five skins
		// and four render styles as CLTBModelProps has, a random direction
		// range and a random speed bias.
		char							m_sModelName[128];
		char							m_sSkinName[5][128];
		char							m_sRenderStyle[4][128];

		bool							m_bPlayImpactSound;
		char							m_sImpactSound[128];

		LTVector						m_vGravity;
		float							m_fGravityAmount;

		float							m_fChunkSpeed;
		float							m_fChunkSpread;

		LTVector						m_vChunkDir;

		LTVector						m_vRandDirMin;
		LTVector						m_vRandDirMax;
		float							m_fRandomSpeedBiasMin;
		float							m_fRandomSpeedBiasMax;
	};

	class CLTBBouncyChunkFX : public CBaseFX
	{
		public :

			// Constuctor

											CLTBBouncyChunkFX();

			// Destructor

										   ~CLTBBouncyChunkFX();

			// Member Functions

			bool							Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps);
			bool							Update(float tmCur);
			void							ReadProps( CLinkList<FX_PROP> *pProps );
			void							Term();

			// RETAIL: Tron's (0x1000ff20). Moves vPos/rRot to where the
			// follow type puts the chunk. The name is ours.
			void							GetSpawnTransform(LTVector &vPos, LTRotation &rRot);

			// Accessors

		protected :

			const CLTBBouncyChunkProps*		GetProps()		{ return (const CLTBBouncyChunkProps*)m_pProps; }

			// Member Variables

			HOBJECT							m_hBouncyChunk;
			HLTSOUND						m_hImpactSound;

			LTVector						m_vVel;

			// RETAIL: Tron's (0x90): set by Init, cleared by the first Update,
			// which throws the chunk. The name is ours.
			bool							m_bFirstUpdate;

			// RETAIL: CBankedList<CLTBBouncyChunkFX> (0x100076d0) has 0xc0
			// bytes per object; no retail code seen yet touches 0x94..0xbf.
			uint8							m_xTronUnknown94[0x2c];
	};

#endif