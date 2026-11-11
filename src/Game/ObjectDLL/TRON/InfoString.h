// ----------------------------------------------------------------------- //
//
// MODULE  : InfoString.h
//
// PURPOSE : An activatable volume the player can query for a string ID
//
// RETAIL: Tron 2.0's, not in any source release; written from retail
// Object.lto (0x10021b70..0x10022260, its class def at 0x102d9760). The
// names of the class, its properties and its message are retail's; the
// rest are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __INFOSTRING_H__
#define __INFOSTRING_H__

#include "GameBase.h"

class InfoString : public GameBase
{
	public :

		InfoString();
		virtual ~InfoString() {}

	protected :

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual uint32	ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

		LTBOOL	ReadProp(ObjectCreateStruct *pStruct);
		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		// Flagged as an info string while it has an ID; its dims
		void	UpdateUserFlags();

		uint32		m_nID;		// The string the client is told of
		LTVector	m_vDims;
};

#endif // __INFOSTRING_H__
