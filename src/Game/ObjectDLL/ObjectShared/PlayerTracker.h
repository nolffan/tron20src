// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerTracker.h
//
// PURPOSE : Tracks list of players.  Used to track players sent messages
//			 that the server needs responses to.	
//
// CREATED : 02/21/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYERTRACKER_H__
#define __PLAYERTRACKER_H__

#include "LTObjRef.h"

class CPlayerObj;

// Add this interface to your class that uses a playertracker.
class IPlayerTrackerReceiver
{
	public:

		// Handles when all players have broken their links.
		virtual void OnPlayerTrackerAbort( ) = 0;
};

class PlayerTracker : public ILTObjRefReceiver
{
	public :

		PlayerTracker( );
		virtual ~PlayerTracker( );

		// Initializes the object by taking a snapsnot of the current players.  Returns
		// false if no players.
		virtual bool	Init( IPlayerTrackerReceiver& playerTrackerReceiver );

		// Terminates the object.
		virtual void	Term( );

		// Removes a client from the list.  Returns playerobj removed.
		CPlayerObj*		RemoveClient( HCLIENT hClient );

		// Returns true if all players removed.
		bool			IsEmpty( ) { return ( m_PlayerList.size( ) == 0 ); }

		// Call update once per frame.
		void			Update( );

		// Handle broken player links.
		virtual void	OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		// Expose the aborted flag so classes that have more than one tracker know which one was updating and was aborted.
		// RETAIL: Tron's PlayerTracker has no aborted flag (its constructor and
		// Term, retail 0x10193130 and 0x10193400: a list and the receiver); NOLF2's
		// save/load code asking it stands in, told no
		bool			Aborted( ) const { return false; }

	// Data.
	private :
		

		// List of players we are tracking.
		typedef ObjRefNotifierList PlayerList;
		PlayerList		m_PlayerList;

		// Receiver of any notifications.
		IPlayerTrackerReceiver*	m_pPlayerTrackerReceiver;
};

#endif // __PLAYERTRACKER_H__