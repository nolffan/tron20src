// ----------------------------------------------------------------------- //
//
// MODULE  : Bombable.h
//
// PURPOSE : The Bombable object
//
// CREATED : 10/25/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BOMBABLE_H__
#define __BOMBABLE_H__

//
// Includes...
//

	#include "GadgetTarget.h"
	#include "Timer.h"

LINKTO_MODULE( Bombable );


//
// Classes...
//

class Bombable : public GadgetTarget
{
	public : // Methods...

		Bombable();
		~Bombable();


	protected : // Methods...

		uint32	EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );
		bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		void	Update();

		LTBOOL	ReadProp( ObjectCreateStruct *pStruct );

		LTBOOL	OnSave( ILTMessage_Write *pMsg ); 
		LTBOOL	OnLoad( ILTMessage_Read *pMsg );
	
		void	SetupDisabledState( LTBOOL bDestroyed );
		void	HandleActivateMsg( HOBJECT hSender );
		void	HandleStoppedMsg( const CParsedMsg &cMsg );

		void	SetupDefusedState( );
		void	SetupDetonatedState( );

		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

	protected : // Members...

		float		m_fDefuseTime;
		float		m_fDetonateTime;
		HSTRING		m_hstrDetonateCmd;
		HSTRING		m_hstrDefusedCmd;
		LTObjRefNotifier	m_hTimeBombModel;

		CTimer		m_Timer;
		
		bool		m_bStartPlanted;
		bool		m_bCanDefuse;
};


class CBombablePlugin : public CGadgetTargetPlugin 
{
	public:

		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);
};

#endif // __TIME_BOMB_H__