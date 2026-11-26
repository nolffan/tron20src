
===========================================================================
Readme for "TRON 2.0" Update 1 and Dedicated Server 
Copyright Disney 2003

GREETINGS PROGRAMS!

===========================================================================

About This Document:

This document contains information about TRON 2.0 Update 1, including 
details on setting up and running the TRON 2.0 Standalone Server
application. Please be sure to read the TRON 2.0 E-Manual before
attempting to run a dedicated server. 


===========================================================================

                        TABLE OF CONTENTS

                I.      System Requirements
                II.     Installation
                III.    Fixes and Additions
                IV.     Setting up your Server
                V.      Running your Server
                VI.     The SCMD System
                VII.    SCMD Commands
                VIII.   Limiting Bandwidth
                IX.     Recommended Player Limits 
                X.      Dedicated servers and Hardware T&L


I. System Requirements
===========================================================================

The following items are required to run a TRON 2.0 dedicated server:

1. A broadband Internet connection or LAN connection
2. An installation of TRON 2.0 and this update.
3. A system that meets or exceeds the minimum system requirements:

        * Windows 98,Windows98 SE, Windows ME, Windows 2000 or
          Windows XP. 
        * 500 MHz Pentium® III or AMD Athlon™ processor.
        * 256 MB RAM
        * 32MB Direct3D-compatible video card with Hardware T&L,
          32-bit color, DirectX™ 9-compatible driver (e.g., 
          NVIDIA GeForce 2 class)
        * A 16-bit DirectX compatible sound card. 
        * 100 BaseT LAN connection for Light Cycle Multi-play
        * DirectX™ 9 (included)

It is possible to run a dedicated server on a machine without a
hardware T&L card. Please see the section near the bottom of this
document entitled "Dedicated servers and Hardware T&L" for 
details.


II. Installation
===========================================================================

Copy TRON-Update_1x1.exe to a temp directory and then click on it to
begin the update installation. The dedicated server files will be copied to
your root directory (i.e. C:\Program Files\Buena Vista Interactive\Tron
2.0). After installation of the update, the version number in the
lower-right corner of the main menu screen will change from v1.010 to 
v1.020.


III. Fixes and Additions
===========================================================================

This update contains numerous bug fixes in including the following:

        * ATI 7000 and 8000 class Radeons users should see a dramatic 
          increase in performance as well as correctly panning
          textures. Please keep in mind that while performance has 
          improved greatly, these cards may be overly taxed when using 
          higher detail settings. In addition to installing the update, 
          users of these cards should get the latest drivers for their 
          cards at http:\\www.ati.com.
        * All retail versions of the game should now be compatible
          for Internet play.
        * Save files containing the pipe (|) character no longer
          generate unreadable save files.
        * Numerous networking performance optimizations have been 
          implemented.
        * Player health will no longer be refilled after each stage
          of the battle with the Fcon monster.
        * Counters in Lightcycle matches should now stay synchronized 
          correctly through subsequent races.
        * Pressing F9 during single player Lightcycle play should no
          longer results in unexpected behavior.
        * Custom Bandwidth settings are now correctly saved.
        * Powerup availability in Lightcycle custom play should more 
          accurately reflect the configuration settings.

This update contains the following additions:

        * Multiplayer games can now be hosted using the new dedicated 
          server. Operating instructions appear below.
        * Selecting No on the Retry? menu during Single Player
          Lightcycle matches will now allow players to skip the 
          Lightcycle match. This feature was added because we received 
          some feedback that the Lightcycle matches were too difficult.
          In addition to the new skip option, the difficulty can also 
          be changed at any point during the game. Please note that the 
          new skipping feature only applies to regular Single Player 
          play and not Lightcycle Custom play.


IV. Setting up your Server
===========================================================================

To run a dedicated server, a Server Configuration must first be
created in the game. just follow these steps:

1. Select Multiplayer from the main menu
2. Click Host
3. Select the game type (i.e. Disc Arena, Disc Tournament or Light
   Cycle), set the other options, and then select the maps you would like 
   to play. For help, please refer to the E-Manual. A configuration file
   with at least one map must be created for the dedicated server to run.
4. If the Dedicated Server field on the Connection tab is set to Yes, 
   clicking the Launch button will exit out of the game and then launch 
   the dedicated server with your selected server configuration.
   TRONSrv.exe can also be launched directly from outside the game. When
   starting a server from outside the game, select your server 
   configuration from the drop-down menu that appears when clicking on 
   TRONSrv.exe (this menu only appears if you have more than one server
   config.)

Once your server is running, clients can search for and join it in the 
same way they would a normal host.

Once your server is configured and launched for the first time, you
will only need to run the game application again if you wish to change
settings.


V. Running your Server
===========================================================================

When the server starts running, the following information appears:

SERVER INFO
-----------

SERVER NAME:            This is the name of your server as seen by
                        players 
                        when they are searching for servers to join.  

GAME TYPE:              Displays the game mode that the server is 
                        currently running, such as Disc Arena, Disc 
                        Tournament or Light Cycle. 

RUNNING TIME:           The time elapsed since the server was
                        launched. 

PEAK PLAYERS:           This shows the highest number of players that
                        were on your server at one time.

TOTAL/UNIQUE PLAYERS:   This shows the total number of players that
                        have joined your server since it was last 
                        launched.

STOP SERVER:            This will shut down the server. 

NEXT LEVEL:             This will load the next map in the map list.

SELECT LEVEL:           This will load the highlighted map in the map
list.


PLAYERS
-------

PLAYERS:                This section provides information about the
                        players that are currently connected to the 
                        server, including their player name, ping, 
                        derezzes, wins and the time elapsed since they
                        began playing. 

CURRENT PLAYERS:        This shows the number of players currently
                        connected and the maximum number of players
                        you specified when you launched the server. 

AVERAGE PING:           The average ping of the players currently
                        connected to your server. 

BOOT PLAYER:            Clicking this button will disconnect the
                        player whose name is currently highlighted. To
                        highlight a player name, just click on it. 

CONSOLE
-------

This shows a scrolling display of the server's internal commands and
status. Player chat, connections, disconnections, and other text is also
displayed here. 

You can use the console to chat with other clients. To broadcast a
message, type Say followed by your text in quotes. Chat text must be 
contained within quotes. Here's an example for syntax:

Say "Stop derezzing me while I'm trying to watch the match, Program."


VI. The SCMD System
===========================================================================

The SCMD interface is used to execute server side commands through a
text-based console. SCMD sends commands to the server to control settings
from the hosting client, a remote client or the standalone server. It
offers a generalized user interface that works on all three of these entry
points. On a hosted or remote client, commands are entered through the chat
interface. On the Stand-alone server, commands are entered through
the console interface. 

To use SCMD, just follow these steps:

1. Before launching your server (either hosted or dedicated), set the
   "Allow SCMD" option to "Yes" and then specify a password. 
2. Launch the server.
3. Type SCMD and press <Enter> on the command line of your
   dedicated server, or in the chat interface on a host or client to see
   a list of available commands. You will need to scroll up to see the 
   entire list.
4. Log in to the system by typing SCMD followed by the password that
   you specified in the host options. 

Once you have successfully logged in, you can then use any of the
other SCMD commands. Note that only one host or client can be logged into
the system at once. The format for all SCMD commands is:

SCMD command [arg1] [...]

Example:  You are logged into SCMD through a client connected to a
dedicated server, and one of the other players becomes abusive. With
SCMD, you can boot the player through your client like this:

Type "SCMD listclients".  This will display text similar to
the following:

Client List format: clientid) [name][ping]
0) [Smiley][0]
1) [Cheater][80]
2) [Player0][120]

From here, you can type "SCMD bootname Cheater", or "SCMD bootid 1" to
remove the problem player from the server. 

Then, if the player logs in again and continues to cause trouble, you
can locate his IP address from the client list, and then ban that IP. For
instance, if the IP address was 123.456.789.012, you would ban it by
typing "SCMD addban 123.456.789.012".  For a full list of commands, 
please see the section below. 


VII. SCMD Commands
===========================================================================

Following is the list of commands currently supported by SCMD:

Command                 Description
---------------------------------------------------------------------------

Help                    Shows a list of all commands
Login                   Logs into server.
Logout                  Logs out of server.
ListClients             Lists all clients connected.
BootName                Boots a player by name
BootId                  Boots a player by clientid
ListBans                Shows all currently banned players or IP's.
AddBan                  Bans IP with mask support (e.g.
                        123.123.123.012 or 123.123.*)
RemoveBan               Removes an player from the banned list.
BanClient               Adds a client to the list of bans.
ListMaps                List all maps in current config.
NextMap                 Goes to next map
SetMap                  Sets the map by index.


VIII. Limiting Bandwidth
===========================================================================

TRON 2.0 allows server operators to limit the amount of data 
that's being sent to players on high bandwidth connections.  When
used correctly, this feature can greatly improve performance.

You can adjust your bandwidth limit in the host options screen before
launching your server via the bandwidth or custom bandwidth options.  

The preset values available to you are as follows:

Preset          Max bandwidth
------          -------------
56k             56kbps
DSL (low)       128kbps
DSL (High)      256kbps
Cable           256kbps
T1              1500kbps
T3              10000kbps

When you first set up your server, you should set this option to
match as closely as possible with the upstream bandwidth rating of your
Internet connection. This information can be obtained from your ISP. If 
none of the available options are suitable for your connection, use the
custom bandwidth setting to enter the correct value. 

It is VERY important that you set this value correctly.  Setting
the limit too high may result in very poor performance for players
with a high ping.  Similarly, setting it too low may result in
choppiness for low ping players. You may want to experiment with 
different values to find the best setting for your particular situation.


IX. Recommended Player Limits
===========================================================================

The number of players that your server can realistically handle
depends heavily on your connection speed. Use the table below to determine 
the maximum number of players you should specify in the host options when
setting up your server. 

Connection                               # of Players 
--------------------------------------------------------------
56K                                     | 2 (including host) |
128k Upstream (low-end DSL)             | 4                  |
256k Upstream (High-end DSL/Cable       | 8                  |
1500k+ (T1-T3)                          | 16                 |
--------------------------------------------------------------


X. Dedicated Servers and Hardware T&L
===========================================================================

The dedicated server does not require a hardware T&L enabled graphics
card. Unfortunately, the TRON 2.0 menu interface that must be used to set up
your server does. This can present a problem for server operators who
don't want to devote a T&L card to a machine that will only be used for
this purpose. However, there is a way around this. When you set up your 
server, the data is saved to your player profile and other mode-related 
text files. If you install to a machine without a hardware T&L card, and 
then copy these files over to that machine, then you can run a server 
without ever launching the TRON 2.0. 

Here's the list of files and folders that you will need to copy in
order to do this (assuming you installed to the default folder). 

C:\Program Files\Buena Vista Interactive\Tron 2.0\ServerData

===========================================================================


PRIVACY NOTICE

Purpose of this Privacy Notice:

Buena Vista Games (“BVG”) is providing you this Privacy Notice so
that you will understand how TRON2.0 gathers, uses, retains and discloses 
information about players of this game. We hope that this information will 
enable you to make informed choices about how you play and allow others to 
play TRON2.0

Information Required by for Multiplayer Use of TRON2.0:

TRON2.0 may be played in three ways:  (1) as a stand alone game by a
single player; (2) online in a multiplayer environment through use of the
Internet; or, (3) in a multiplayer environment through use of a Local 
Area Network (which is commonly referred to as a “LAN”). When TRON2.0 is
played as a stand alone game, there is no need for the player to supply 
any information in order to play the game. However, when TRON2.0 is
played online in a multiplayer environment (through use of either the 
Internet or a LAN), the player has the option to create a Player Profile 
before beginning to play the game.  To help you understand how this 
Profile works, we will use the example of a hypothetical player named 
“John Smith.” To complete his Profile, John must create and enter a 
screen name.  To avoid revealing his identity, John should not use a 
screen name that is based upon his real name, nor should John’s screen 
name include other information that would permit identification of John 
in the offline world.  For example, John’s screen name should not be 
“JohnSmith,” nor should the screen name include any portion of John‘s 
address.  The player can choose not to create a profile; in such a case,
the player’s screen name would default to “Program”.

How the TRON2.0 “List Server” Collects and Retains Information:  
 
After John has completed his Profile (if he chooses to do so), he may
begin playing TRON2.0 online in the multiplayer environment. When John 
hits the “Multiplay” button, John’s computer will connect through the 
Internet to a large computer operated by GameSpy Industries in the U.S.A.
This computer is the “list server” for multiplayer playing of TRON2.0 
This list server will read the IP (Internet Protocol) address for John’s
computer and John’s screen name, if he has chosen one. (An IP address 
identifies the place from which a user enters the Internet.) Except as 
required by law, the list server will retain this information only 
during the period that John is online in the TRON2.0 game environment. 
BVG does not receive any personally identifiable information about 
players from the list server.

How the TRON2.0 “List Server” Uses Information:

After the list server has read John’s IP address, the list server
will transmit to John’s personal computer a list of TRON2.0 multiplayer 
game sessions that are available through the list server.  When John 
receives the list of available TRON2.0 game sessions, John may then 
either decide to join one of these game sessions as a player
or may decide to initiate his own game session.

If John joins an existing multiplayer TRON2.0 game session, the list
server will transmit John’s IP address to the personal computer of 
the player who is hosting the game session that John has decided to 
join. This player’s personal computer is called the “game server” for 
that game session. The game server will then establish a direct
peer-to-peer connection to John’s personal computer by using John’s 
IP address. 

If John decides to host his own multiplayer TRON2.0 game session,
John’s personal computer will become a game server and will receive 
the IP addresses of other players who join the TRON2.0 game session 
that John is hosting.  John’s IP address also will be sent to each of 
the players who join John’s game session so that all of those
players’ personal computers may communicate in a peer-to-peer 
fashion with John’s game server. This also enables the players in a 
game to chat with each other. Neither GameSpy nor BVG monitors the 
in-game chat and BVG hereby disclaims any responsibility with 
respect to the content of such in-game chat.

Players’ Ability to Choose How Information Is Collected, Used,
Retained and Disclosed:
  
Of course, a player may avoid any collection, use, retention and
disclosure of information by not playing TRON2.0 in the online 
multiplayer environment.  A player may also choose to use the 
default “Program” screen name intentionally or by not creating a 
player profile. Finally, a player may limit the content of the 
player’s own chat so that no sensitive information is revealed.

Players’ Ability to Access and Change Information:

A player can change their screen name at any time by using the Player
Setup option in the main menu.  Because the information that is 
collected by the list server is retained by that server only while the 
player is online in the TRON2.0 multiplayer environment (except as 
otherwise required by law), there is no reasonable or practical
way to enable the player to access the information on that list 
server in such a way as to permit the alteration or deletion of that 
information, other than for the player to exit the TRON2.0 online game 
environment.  In addition, the list server has no control over
information once that information is transmitted to a game server.  

Security and Integrity of the Information Required for Multiplayer
Online Use of TRON2.0:

The TRON2.0 list server is protected by firewalls and other
technical, administrative and physical measures that are designed 
to protect the security and integrity of the information that resides 
on the list server. The information used for multiplayer online play 
of TRON2.0 is not encrypted during transmission. Finally, the 
security and integrity of this information as it resides on players’ 
personal computers (including those computers that function as 
game servers) is determined by the measures taken by the
manufacturers and owners of those computers, except that the TRON2.0
software has been designed to minimize the loss or corruption of 
the information required to enable multiplayer online play.

GameSpy Arcade:

TRON2.0 is compatible with GameSpy Arcade, an online gaming service
hosted by GameSpy Industries that enables you to find other TRON2.0 
games and players online.  The same underlying technology that powers 
the in-game player matching in TRON2.0 powers the matching in GameSpy 
Arcade. GameSpy Arcade’s registration process requires players to give
their names, email addresses, and age. GameSpy Arcade also collects 
other optional personal information, if supplied by the player.  BVG 
does not receive any of the personal information collected by 
GameSpy Arcade. Finally, use of GameSpy Arcade is NOT required to play
multiplayer games of TRON2.0 online.

FilePlanet:

FilePlanet is a website hosted by GameSpy Industries.  Game patches,
game play movies and demos are available for download on FilePlanet. 
Players who access the FilePlanet website must register through the 
GameSpy Arcade registration application. BVG does not receive any 
personally identifiable information about players from FilePlanet.
