//----------------------------------------------------------------------------
//
//	MODULE:		ActivationObserver.h
//
//	PURPOSE:	RETAIL: Tron's IActivationObserver (the name is retail's,
//				from RTTI) and the notifier tying an observer to a CAI's
//				list at +0x10a0. Everything else here is ours: the shape
//				follows RelationChangeObserver.h, which retail's code
//				mirrors.
//
//				Retail: CAIGoalFollow (+0x74), CAIGoalFlee and
//				CAIGoalDistress (+0xa0) derive IActivationObserver and
//				hold an ActivationNotifier right after their own members;
//				their InitGoal(CAI*) registers it (0x102076a0, 0x10207fc0,
//				0x102099f0), their destructors unregister it. CAI's
//				activation handler (0x1012cf70) fills an ActivationInfo
//				and calls DoNotification (0x101e08b0) on every notifier
//				of its list through for_each/bind2nd/mem_fun.
//
//----------------------------------------------------------------------------

#ifndef __ACTIVATIONOBSERVER_H__
#define __ACTIVATIONOBSERVER_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "AI.h"

// RETAIL: what CAI's activation handler (0x1012cf70) passes to every
// notifier (names ours): the activating object, the AI's own m_hObject, and
// whether the AI can be activated now.
struct ActivationInfo
{
	HOBJECT		hActivator;
	HOBJECT		hAI;
	bool		bCanActivate;
};

//-------------------------------------------------------------------------
//
//	CLASS:		IActivationObserver
//
//	PURPOSE:	RETAIL: one virtual (retail vtable 0x1029d354, purecall);
//				the name of the function is ours.
//
//-------------------------------------------------------------------------
class IActivationObserver
{
public:
	virtual int OnActivation(ActivationInfo* pInfo) = 0;
};

typedef int (IActivationObserver::*ActivationCallback)(ActivationInfo* pInfo);

//-------------------------------------------------------------------------
//
//	CLASS:		ActivationNotifier
//
//	PURPOSE:	RETAIL: 0xc bytes: observer, callback, subject (names ours).
//				Not saved: InitGoal sets it up again after a load.
//
//-------------------------------------------------------------------------
class ActivationNotifier
{
public:
	ActivationNotifier() :
	m_pObserver( 0 ),
	m_pfnCallback( 0 ),
	m_pSubject( 0 )
	{
	}

	~ActivationNotifier()
	{
		if ( m_pSubject )
		{
			m_pSubject->RemoveTronUnknown10A0( this );
		}
	}

	void SetSubject(CAI* pSubject)
	{
		if ( !m_pSubject && pSubject )
		{
			m_pSubject = pSubject;
			m_pSubject->AddTronUnknown10A0( this );
		}
	}

	void SetObserver(IActivationObserver* pObserver)
	{
		if ( pObserver && !m_pObserver )
		{
			m_pObserver = pObserver;
		}
	}

	void SetCallback(ActivationCallback pfnCallback)
	{
		if ( pfnCallback )
		{
			m_pfnCallback = pfnCallback;
		}
	}

	// RETAIL: 0x101e08b0 (called through mem_fun by CAI's handler).
	int DoNotification(ActivationInfo* pInfo)
	{
		if ( m_pObserver )
		{
			(m_pObserver->*m_pfnCallback)( pInfo );
		}
		return 0;
	}

protected:
	ActivationNotifier(const ActivationNotifier& rhs) {}
	ActivationNotifier& operator=(const ActivationNotifier& rhs ) {}

private:

	IActivationObserver*	m_pObserver;
	ActivationCallback		m_pfnCallback;
	CAI*					m_pSubject;
};

#endif // __ACTIVATIONOBSERVER_H__
