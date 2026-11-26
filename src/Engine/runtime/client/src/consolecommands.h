#ifndef __CONSOLECOMMANDS_H__
#define __CONSOLECOMMANDS_H__

#ifndef __CONCOMMAND_H__
#include "concommand.h"
#endif

// The main client console state, to be used with the concommand functions.
extern ConsoleState g_ClientConsoleState;


// Initializes all the base DirectEngine console variables.
void			c_InitConsoleCommands();
void			c_TermConsoleCommands();

// Call this to run a console command.
void			c_CommandHandler(const char *pCommand);

#ifdef TRON20SRC_MODERN
// MODERN.md EN-70: a line typed into the console: run, then say what it did
// where the command handler says nothing (a variable set, a name unknown).
void			c_ModernTypedCommand(const char *pCommand);

// MODERN.md EN-70: each command (the engine's and the game's) and each
// variable whose name starts with pPrefix, for Tab. A name can come two
// times (an engine variable that was set is a console variable too).
typedef void	(*ModernNameFn)(const char *pName, void *pUser);
void			c_ModernEnumNames(const char *pPrefix, ModernNameFn fn, void *pUser);
#endif


#endif  // __CONSOLECOMMANDS_H__

