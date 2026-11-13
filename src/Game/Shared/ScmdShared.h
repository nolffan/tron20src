// ----------------------------------------------------------------------- //
//
// MODULE  : ScmdShared.h
//
// PURPOSE : Shared handling of SCMD commands.  Provides remote control of
//				server.
//
// CREATED : 10/21/02
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef SCMDSHARED_H
#define SCMDSHARED_H

// RETAIL: Tron's commands, 0..12 (the handlers' CommandId, TRONSrv.exe
// 0x405840..0x405e40, and the server's switch, Object.lto 0x101c5e80): NOLF2's
// without its NextRound, ListGameOptions and SetGameOption.
enum ScmdCommand
{
	kScmdCommandHelp,
	kScmdCommandLogin,
	kScmdCommandLogout,
	kScmdCommandListClients,
	kScmdCommandListMissions,
	kScmdCommandNextMission,
	kScmdCommandSetMission,
	kScmdCommandBootName,
	kScmdCommandBootId,
	kScmdCommandAddBan,
	kScmdCommandRemoveBan,
	kScmdCommandListBans,
	kScmdCommandBanClient,
};


enum ScmdCommandStatus
{
	kScmdCommandStatusOk,
	kScmdCommandStatusFailed,
	kScmdCommandStatusAdminAlreadyLoggedIn,
	kScmdCommandStatusNotLoggedIn,
	kScmdCommandStatusIncorrectPassword,
};


#endif // SCMDSHARED_H