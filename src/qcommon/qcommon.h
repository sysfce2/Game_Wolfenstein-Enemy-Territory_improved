/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Wolfenstein: Enemy Territory GPL Source Code (Wolf ET Source Code).  

Wolf ET Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Wolf ET Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Wolf ET Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Wolf: ET Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Wolf ET Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
// qcommon.h -- definitions common between client and server, but not game.or ref modules
#ifndef _QCOMMON_H_
#define _QCOMMON_H_

#include <sys/types.h>
#include "cm_public.h"

//Ignore __attribute__ on non-gcc/clang platforms
#if !defined(__GNUC__) && !defined(__clang__)
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

/* C99 defines __func__ */
#if __STDC_VERSION__ < 199901L 
#if __GNUC__ >= 2 || _MSC_VER >= 1300 || defined(__clang__)
#define __func__ __FUNCTION__ 
#else 
#define __func__ "(unknown)" 
#endif
#endif

#if defined (_WIN32) || defined(__linux__)
#define USE_AFFINITY_MASK
#endif

// stringify macro
#define XSTRING(x)	STRING(x)
#define STRING(x)	#x

#define DELAY_WRITECONFIG

//============================================================================

//
// msg.c
//
typedef struct {
	qboolean	allowoverflow;	// if false, do a Com_Error
	qboolean	overflowed;		// set to true if the buffer size failed (with allowoverflow set)
	qboolean	oob;			// raw out-of-band operation, no static huffman encoding/decoding
	byte	*data;
	int		maxsize;
	int		maxbits;			// maxsize in bits, for overflow checks
	int		cursize;
	int		uncompsize;			// NERVE - SMF - net debugging
	int		readcount;
	int		bit;				// for bitwise reads and writes
} msg_t;

void MSG_Init( msg_t *buf, byte *data, int length );
void MSG_InitOOB( msg_t *buf, byte *data, int length );
void MSG_Clear( msg_t *buf );
void MSG_WriteData( msg_t *buf, const void *data, int length );
void MSG_Bitstream( msg_t *buf );
void MSG_Uncompressed( msg_t *buf );

// TTimo
// copy a msg_t in case we need to store it as is for a bit
// (as I needed this to keep an msg_t from a static var for later use)
// sets data buffer as MSG_Init does prior to do the copy
void MSG_Copy(msg_t *buf, byte *data, int length, const msg_t *src);

struct usercmd_s;
struct entityState_s;
struct playerState_s;

void MSG_WriteBits( msg_t *msg, int value, int bits );

void MSG_WriteChar (msg_t *sb, int c);
void MSG_WriteByte (msg_t *sb, int c);
void MSG_WriteShort (msg_t *sb, int c);
void MSG_WriteLong (msg_t *sb, int c);
void MSG_WriteFloat (msg_t *sb, float f);
void MSG_WriteString (msg_t *sb, const char *s);
void MSG_WriteBigString (msg_t *sb, const char *s);
void MSG_WriteAngle16 (msg_t *sb, float f);
int MSG_HashKey(const char *string, int maxlen);

void	MSG_BeginReading (msg_t *sb);
void	MSG_BeginReadingOOB(msg_t *sb);
void	MSG_BeginReadingUncompressed( msg_t *msg );

int		MSG_ReadChar (msg_t *sb);
int		MSG_ReadByte (msg_t *sb);
int		MSG_ReadShort (msg_t *sb);
int		MSG_ReadLong (msg_t *sb);
float	MSG_ReadFloat (msg_t *sb);
const char *MSG_ReadString (msg_t *sb);
const char *MSG_ReadBigString (msg_t *sb);
const char *MSG_ReadStringLine (msg_t *sb);
float MSG_ReadAngle16 (msg_t *sb);
void  MSG_ReadData(msg_t *sb, void *buffer, int size);
int   MSG_ReadEntitynum(msg_t *sb);

void MSG_WriteDeltaUsercmdKey( msg_t *msg, int key, const usercmd_t *from, const usercmd_t *to );
void MSG_ReadDeltaUsercmdKey( msg_t *msg, int key, const usercmd_t *from, usercmd_t *to );

void MSG_WriteDeltaEntity( msg_t *msg, const entityState_t *from, const entityState_t *to, qboolean force );
void MSG_ReadDeltaEntity( msg_t *msg, const entityState_t *from, entityState_t *to, int number );

void MSG_WriteDeltaPlayerstate( msg_t *msg, const playerState_t *from, const playerState_t *to );
void MSG_ReadDeltaPlayerstate( msg_t *msg, const playerState_t *from, playerState_t *to );

void MSG_ReportChangeVectors_f( void );

//============================================================================

/*
==============================================================

NET

==============================================================
*/
#ifndef DISABLE_IPV6
#define USE_IPV6
#endif

#define NET_ENABLEV4            0x01
#define NET_ENABLEV6            0x02
// if this flag is set, always attempt ipv6 connections instead of ipv4 if a v6 address is found.
#define NET_PRIOV6              0x04
// disables ipv6 multicast support if set.
#define NET_DISABLEMCAST        0x08


#define	PACKET_BACKUP	32	// number of old messages that must be kept on client and
							// server for delta comrpession and ping estimation
#define	PACKET_MASK		(PACKET_BACKUP-1)

#define	MAX_PACKET_USERCMDS		32		// max number of usercmd_t in a packet

#define	MAX_SNAPSHOT_ENTITIES	2048//256

#define	PORT_ANY			-1

// RF, increased this, seems to keep causing problems when set to 64, especially when loading
// a savegame, which is hard to fix on that side, since we can't really spread out a loadgame
// among several frames
//#define	MAX_RELIABLE_COMMANDS	64			// max string commands buffered for restransmit
//#define	MAX_RELIABLE_COMMANDS	128			// max string commands buffered for restransmit
#define MAX_RELIABLE_COMMANDS   256 // bigger!

typedef enum {
	NA_BAD = 0,					// an address lookup failed
	NA_BOT,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
#ifdef USE_IPV6
	NA_IP6,
	NA_MULTICAST6,
#endif
	NA_UNSPEC
} netadrtype_t;


typedef enum {
	NS_CLIENT,
	NS_SERVER
} netsrc_t;


#define NET_ADDRSTRMAXLEN 48	// maximum length of an IPv6 address string including trailing '\0'

typedef struct {
	netadrtype_t	type;
	union {
		byte	_4[4];
#ifdef USE_IPV6
		byte	_6[16];
#endif
	} ipv;
	uint16_t	port;
#ifdef USE_IPV6
	uint32_t scope_id;	// Needed for IPv6 link-local addresses
#endif
} netadr_t;

void		NET_Init( void );
//void		NET_Shutdown( void );
void		NET_FlushPacketQueue(void);
void		NET_SendPacket( netsrc_t sock, int length, const void *data, const netadr_t *to );
void		QDECL NET_OutOfBandPrint( netsrc_t net_socket, const netadr_t *adr, const char *format, ...) FORMAT_PRINTF(3, 4);
void		NET_OutOfBandCompress( netsrc_t sock, const netadr_t *adr, const byte *data, int len );

qboolean	NET_CompareAdr( const netadr_t *a, const netadr_t *b );
qboolean	NET_CompareBaseAdrMask( const netadr_t *a, const netadr_t *b, unsigned int netmask );
qboolean	NET_CompareBaseAdr( const netadr_t *a, const netadr_t *b );
qboolean	NET_IsLocalAddress( const netadr_t *adr );
const char	*NET_AdrToString( const netadr_t *a );
const char	*NET_AdrToStringwPort( const netadr_t *a );
int         NET_StringToAdr( const char *s, netadr_t *a, netadrtype_t family );
qboolean	NET_GetLoopPacket( netsrc_t sock, netadr_t *net_from, msg_t *net_message );
#ifdef USE_IPV6
void		NET_JoinMulticast6( void );
void		NET_LeaveMulticast6( void );
#endif
qboolean	NET_Sleep( int timeout );

#define	MAX_PACKETLEN	1400	// max size of a network packet

//----(SA)	increased for larger submodel entity counts
#define MAX_MSGLEN		32768       // max length of a message, which may
//#define	MAX_MSGLEN		16384		// max length of a message, which may
								// be fragmented into multiple packets

#define	MAX_MSGLEN_BUF	(MAX_MSGLEN+8)	// real buffer size that we need to allocate
										// to safely handle overflows

#define MAX_DOWNLOAD_WINDOW		48	// ACK window of 48 download chunks. Cannot set this higher, or clients
						// will overflow the reliable commands buffer
#define MAX_DOWNLOAD_BLKSIZE		1024	// 896 byte block chunks

#define NETCHAN_GENCHECKSUM(challenge, sequence) ((challenge) ^ ((sequence) * (challenge)))

/*
Netchan handles packet fragmentation and out of order / duplicate suppression
*/

typedef struct {
	netsrc_t	sock;

	int			dropped;			// between last packet and previous

	netadr_t	remoteAddress;
	int			qport;				// qport value to write when transmitting

	// sequencing variables
	int			incomingSequence;
	int			outgoingSequence;

	// incoming fragment assembly buffer
	int			fragmentSequence;
	int			fragmentLength;	
	byte		fragmentBuffer[MAX_MSGLEN];

	// outgoing fragment buffer
	// we need to space out the sending of large fragmented messages
	qboolean	unsentFragments;
	int			unsentFragmentStart;
	int			unsentLength;
	byte		unsentBuffer[MAX_MSGLEN];

	int			challenge;
	int			lastSentTime;
	int			lastSentSize;

	qboolean	compat; // ioq3 extension
	qboolean	isLANAddress;

} netchan_t;

void Netchan_Init( int qport );
void Netchan_Setup( netsrc_t sock, netchan_t *chan, const netadr_t *adr, int port, int challenge, qboolean compat );

void Netchan_Transmit( netchan_t *chan, int length, const byte *data );
void Netchan_TransmitNextFragment( netchan_t *chan );

qboolean Netchan_Process( netchan_t *chan, msg_t *msg );


/*
==============================================================

PROTOCOL

==============================================================
*/

// sent by the server, printed on connection screen, works for all clients
// (restrictions: does not handle \n, no more than 256 chars)
#define PROTOCOL_MISMATCH_ERROR "ERROR: Protocol Mismatch Between Client and Server.\
The server you are attempting to join is running an incompatible version of the game."

// long version used by the client in diagnostic window
#define PROTOCOL_MISMATCH_ERROR_LONG "ERROR: Protocol Mismatch Between Client and Server.\n\n\
The server you attempted to join is running an incompatible version of the game.\n\
You or the server may be running older versions of the game. Press the auto-update\
 button if it appears on the Main Menu screen."

#define GAMENAME_STRING "et"
// 2.56 - protocol 83
// 2.4 - protocol 80
// 1.33 - protocol 59
// 1.4 - protocol 60
#define VERY_OLD_PROTOCOL_VERSION 83
#define	OLD_PROTOCOL_VERSION	84

// new protocol with UDP spoofing protection:
#define	NEW_PROTOCOL_VERSION	88

#define DEFAULT_PROTOCOL_VERSION	OLD_PROTOCOL_VERSION


// maintain a list of compatible protocols for demo playing
// NOTE: that stuff only works with two digits protocols
extern const int demo_protocols[];

// NERVE - SMF - wolf multiplayer master servers
#ifndef MASTER_SERVER_NAME
	#define MASTER_SERVER_NAME      "etmaster.idsoftware.com"
#endif
#define MOTD_SERVER_NAME        "etmaster.idsoftware.com"    //"etmotd.idsoftware.com"			// ?.?.?.?

// TTimo: override autoupdate server for testing
#ifndef AUTOUPDATE_SERVER_NAME
//	#define AUTOUPDATE_SERVER_NAME "127.0.0.1"
	#define AUTOUPDATE_SERVER_NAME "au2rtcw2.activision.com"
#endif

// TTimo: allow override for easy dev/testing..
// FIXME: not planning to support more than 1 auto update server
// see cons -- update_server=myhost
#define MAX_AUTOUPDATE_SERVERS  5
#if !defined( AUTOUPDATE_SERVER_NAME )
  #define AUTOUPDATE_SERVER1_NAME   "au2rtcw1.activision.com"            // DHM - Nerve
  #define AUTOUPDATE_SERVER2_NAME   "au2rtcw2.activision.com"            // DHM - Nerve
  #define AUTOUPDATE_SERVER3_NAME   "au2rtcw3.activision.com"            // DHM - Nerve
  #define AUTOUPDATE_SERVER4_NAME   "au2rtcw4.activision.com"            // DHM - Nerve
  #define AUTOUPDATE_SERVER5_NAME   "au2rtcw5.activision.com"            // DHM - Nerve
#else
  #define AUTOUPDATE_SERVER1_NAME   AUTOUPDATE_SERVER_NAME
  #define AUTOUPDATE_SERVER2_NAME   AUTOUPDATE_SERVER_NAME
  #define AUTOUPDATE_SERVER3_NAME   AUTOUPDATE_SERVER_NAME
  #define AUTOUPDATE_SERVER4_NAME   AUTOUPDATE_SERVER_NAME
  #define AUTOUPDATE_SERVER5_NAME   AUTOUPDATE_SERVER_NAME
#endif

#define PORT_MASTER         27950
#define PORT_MOTD           27951
#define PORT_SERVER         27960
#define NUM_SERVER_PORTS    4       // broadcast scan this many ports after
									// PORT_SERVER so a single machine can
									// run multiple servers


// the svc_strings[] array in cl_parse.c should mirror this
//
// server to client
//
enum svc_ops_e {
	svc_bad,
	svc_nop,
	svc_gamestate,
	svc_configstring,			// [short] [string] only in gamestate messages
	svc_baseline,				// only in gamestate messages
	svc_serverCommand,			// [string] to be executed by client game module
	svc_download,				// [short] size [size bytes]
	svc_snapshot,
	svc_EOF,

	// new commands, supported only by ioquake3 protocol but not legacy
	svc_voipSpeex,     // not wrapped in USE_VOIP, so this value is reserved.
	svc_voipOpus,      //
};


//
// client to server
//
enum clc_ops_e {
	clc_bad,
	clc_nop, 		
	clc_move,				// [[usercmd_t]
	clc_moveNoDelta,		// [[usercmd_t]
	clc_clientCommand,		// [string] message
	clc_EOF,

	// new commands, supported only by ioquake3 protocol but not legacy
	clc_voipSpeex,   // not wrapped in USE_VOIP, so this value is reserved.
	clc_voipOpus,    //
};

/*
==============================================================

VIRTUAL MACHINE

==============================================================
*/
typedef struct vm_s vm_t;

typedef enum {
	VMI_NATIVE,
} vmInterpret_t;

typedef enum {
	VM_BAD = -1,
	VM_GAME = 0,
#ifndef DEDICATED
	VM_CGAME,
	VM_UI,
#endif
	VM_COUNT
} vmIndex_t;

void	VM_Init( void );
void	VM_Shutdown( void );
vm_t	*VM_Create( vmIndex_t index, syscall_t systemCalls, dllSyscall_t dllSyscalls, vmInterpret_t interpret );

// module should be bare: "cgame", not "cgame.dll" or "vm/cgame.qvm"

void	VM_Free( vm_t *vm );
void	VM_Clear(void);
void	VM_Forced_Unload_Start(void);
void	VM_Forced_Unload_Done(void);
vm_t	*VM_Restart( vm_t *vm );

#define GET_VM_MACRO(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, NAME, ...) NAME

#define VM_Call_0(vm, callNum) VM_CallFunc(vm, 0, callNum)
#define VM_Call_1(vm, callNum, a1) VM_CallFunc(vm, 1, callNum, (intptr_t)(a1))
#define VM_Call_2(vm, callNum, a1, a2) VM_CallFunc(vm, 2, callNum, (intptr_t)(a1), (intptr_t)(a2))
#define VM_Call_3(vm, callNum, a1, a2, a3) VM_CallFunc(vm, 3, callNum, (intptr_t)(a1), (intptr_t)(a2), (intptr_t)(a3))
#define VM_Call_4(vm, callNum, a1, a2, a3, a4) VM_CallFunc(vm, 4, callNum, (intptr_t)(a1), (intptr_t)(a2), (intptr_t)(a3), (intptr_t)(a4))
#define VM_Call_5(vm, callNum, a1, a2, a3, a4, a5) VM_CallFunc(vm, 5, callNum, (intptr_t)(a1), (intptr_t)(a2), (intptr_t)(a3), (intptr_t)(a4), (intptr_t)(a5))
#define VM_Call_6(vm, callNum, a1, a2, a3, a4, a5, a6) VM_CallFunc(vm, 6, callNum, (intptr_t)(a1), (intptr_t)(a2), (intptr_t)(a3), (intptr_t)(a4), (intptr_t)(a5), (intptr_t)(a6))
#define VM_Call_7(vm, callNum, a1, a2, a3, a4, a5, a6, a7) VM_CallFunc(vm, 7, callNum, (intptr_t)(a1), (intptr_t)(a2), (intptr_t)(a3), (intptr_t)(a4), (intptr_t)(a5), (intptr_t)(a6), (intptr_t)(a7))
#define VM_Call_8(vm, callNum, a1, a2, a3, a4, a5, a6, a7, a8) VM_CallFunc(vm, 8, callNum, (intptr_t)(a1), (intptr_t)(a2), (intptr_t)(a3), (intptr_t)(a4), (intptr_t)(a5), (intptr_t)(a6), (intptr_t)(a7), (intptr_t)(a8))
#define VM_Call_9(vm, callNum, a1, a2, a3, a4, a5, a6, a7, a8, a9) VM_CallFunc(vm, 9, callNum, (intptr_t)(a1), (intptr_t)(a2), (intptr_t)(a3), (intptr_t)(a4), (intptr_t)(a5), (intptr_t)(a6), (intptr_t)(a7), (intptr_t)(a8), (intptr_t)(a9))
#define VM_Call_10(vm, callNum, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) VM_CallFunc(vm, 10, callNum, (intptr_t)(a1), (intptr_t)(a2), (intptr_t)(a3), (intptr_t)(a4), (intptr_t)(a5), (intptr_t)(a6), (intptr_t)(a7), (intptr_t)(a8), (intptr_t)(a9), (intptr_t)(a10))
#define VM_Call_11(vm, callNum, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) VM_CallFunc(vm, 11, callNum, (intptr_t)(a1), (intptr_t)(a2), (intptr_t)(a3), (intptr_t)(a4), (intptr_t)(a5), (intptr_t)(a6), (intptr_t)(a7), (intptr_t)(a8), (intptr_t)(a9), (intptr_t)(a10), (intptr_t)(a11))
#define VM_Call_12(vm, callNum, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) VM_CallFunc(vm, 12, callNum, (intptr_t)(a1), (intptr_t)(a2), (intptr_t)(a3), (intptr_t)(a4), (intptr_t)(a5), (intptr_t)(a6), (intptr_t)(a7), (intptr_t)(a8), (intptr_t)(a9), (intptr_t)(a10), (intptr_t)(a11), (intptr_t)(a12))

#define VM_Call(...) EXPAND(GET_VM_MACRO(__VA_ARGS__, VM_Call_12, VM_Call_11, VM_Call_10, VM_Call_9, VM_Call_8, VM_Call_7, VM_Call_6, VM_Call_5, VM_Call_4, VM_Call_3, VM_Call_2, VM_Call_1, VM_Call_0)(__VA_ARGS__))

intptr_t QDECL VM_CallFunc( vm_t *vm, int nargs, int callNum, ... );

void	VM_Debug( int level );

void	*GVM_ArgPtr( intptr_t intValue );

#define	VMA(x) VM_ArgPtr(args[x])
static ID_INLINE float _vmf(intptr_t x)
{
	floatint_t v;
	v.i = (int)x;
	return v.f;
}
#define	VMF(x)	_vmf(args[x])


/*
==============================================================

CMD

Command text buffering and command execution

==============================================================
*/

/*

Any number of commands can be added in a frame, from several different sources.
Most commands come from either keybindings or console line input, but entire text
files can be execed.

*/

#define MAX_CMD_LINE 1024

void Cbuf_Init( void );
// allocates an initial text buffer that will grow as needed

void Cbuf_AddText( const char *text );
// Adds command text at the end of the buffer, does NOT add a final \n

void Cbuf_NestedAdd( const char *text );
// Adds nested command text at the specified position of the buffer, adds \n when needed

void Cbuf_NestedReset( void );
// Resets nested cmd offset

void Cbuf_InsertText( const char *text );
// Adds command text at the beginning of the buffer, add \n

void Cbuf_ExecuteText( cbufExec_t exec_when, const char *text );
// this can be used in place of either Cbuf_AddText or Cbuf_InsertText

void Cbuf_Execute( void );
// Pulls off \n terminated lines of text from the command buffer and sends
// them through Cmd_ExecuteString.  Stops when the buffer is empty.
// Normally called once per frame, but may be explicitly invoked.
// Do not call inside a command function, or current args will be destroyed.

void Cbuf_Wait( void );
// Checks if wait command timeout remaining

//===========================================================================

/*

Command execution takes a null terminated string, breaks it into tokens,
then searches for a command or variable that matches the first token.

*/

typedef void (*xcommand_t) (void);
typedef void (*xcommandCompFunc_t)( char *args, int argNum );

typedef struct cmdListItem_s {
	const char	*name;
	xcommand_t	func;
	xcommandCompFunc_t	complete;
} cmdListItem_t;

void	Cmd_Init( void );

void	Cmd_AddCommand( const char *cmd_name, xcommand_t function );
// called by the init functions of other parts of the program to
// register commands and functions to call for them.
// The cmd_name is referenced later, so it should not be in temp memory
// if function is NULL, the command will be forwarded to the server
// as a clc_clientCommand instead of executed locally

void	Cmd_RemoveCommand( const char *cmd_name );

void Cmd_RegisterList( const cmdListItem_t* cmds, int count, module_t module );
void Cmd_UnregisterList( const cmdListItem_t* cmds, int count );
#define Cmd_RegisterArray( a, m )  Cmd_RegisterList( a, ARRAY_LEN(a), m )
#define Cmd_UnregisterArray( a )   Cmd_UnregisterList( a, ARRAY_LEN(a) )

void Cmd_SetModule( const char *cmd_name, module_t modulue );

// Remove commands by module, and only if they match this module
void Cmd_UnregisterModule( module_t module );

// don't allow VMs to remove system commands
void	Cmd_RemoveCommandSafe( const char *cmd_name );

void	Cmd_CommandCompletion( void(*callback)(const char *s) );
// callback with each valid string
void	Cmd_SetCommandCompletionFunc( const char *command, xcommandCompFunc_t complete );
qboolean Cmd_CompleteArgument( const char *command, char *args, int argNum );
void	Cmd_CompleteWriteCfgName( char *args, int argNum );

int			Cmd_Argc( void );
void		Cmd_Clear( void );
const char	*Cmd_Argv( int arg );
void		Cmd_ArgvBuffer( int arg, char *buffer, int bufferLength );
const char	*Cmd_ArgsFrom( int arg );
const char	*Cmd_ArgsRange( int arg, int count );
void		Cmd_ArgsBuffer( char *buffer, int bufferLength );
const char	*Cmd_Cmd( void );
void		Cmd_Args_Sanitize( const char *separators, qboolean caplength );
// The functions that execute commands get their parameters with these
// functions. Cmd_Argv () will return an empty string, not a NULL
// if arg > argc, so string operations are allways safe.

void	Cmd_TokenizeString( const char *text );
void	Cmd_TokenizeStringIgnoreQuotes( const char *text_in );
// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.

void	Cmd_ExecuteString( const char *text );
// Parses a single line of text into arguments and tries to execute it
// as if it was typed at the console


/*
==============================================================

CVAR

==============================================================
*/

/*

cvar_t variables are used to hold scalar or string variables that can be changed
or displayed at the console or prog code as well as accessed directly
in C code.

The user can access cvars from the console in three ways:
r_draworder			prints the current value
r_draworder 0		sets the current value to 0
set r_draworder 0	as above, but creates the cvar if not present

Cvars are restricted from having the same names as commands to keep this
interface from being ambiguous.

The are also occasionally used to communicated information between different
modules of the program.

*/

cvar_t *Cvar_Get( const char *var_name, const char *value, int flags );
// creates the variable if it doesn't exist, or returns the existing one
// if it exists, the value will not be changed, but flags will be ORed in
// that allows variables to be unarchived without needing bitflags
// if value is "", the value will not override a previously set value.

void	Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags, int privateFlag );
// basically a slightly modified Cvar_Get for the interpreted modules

void	Cvar_Update( vmCvar_t *vmCvar, int privateFlag );
// updates an interpreted modules' version of a cvar

void 	Cvar_Set( const char *var_name, const char *value );
// will create the variable with no flags if it doesn't exist

cvar_t	*Cvar_Set2(const char *var_name, const char *value, qboolean force);
// same as Cvar_Set, but allows more control over setting of cvar

void	Cvar_SetSafe( const char *var_name, const char *value );
// sometimes we set variables from an untrusted source: fail if flags & CVAR_PROTECTED

void	Cvar_SetLatched( const char *var_name, const char *value);
// don't set the cvar immediately

void	Cvar_SetValue( const char *var_name, float value );
void	Cvar_SetIntegerValue( const char *var_name, int value );
void	Cvar_SetValueSafe( const char *var_name, float value );
// expands value to a string and calls Cvar_Set/Cvar_SetSafe

qboolean Cvar_SetModified( const char *var_name, qboolean modified );

float	Cvar_VariableValue( const char *var_name );
int		Cvar_VariableIntegerValue( const char *var_name );
// returns 0 if not defined or non numeric

const char *Cvar_VariableString( const char *var_name );
void	Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void	Cvar_VariableStringBufferSafe( const char *var_name, char *buffer, int bufsize, int flag );
// returns an empty string if not defined
const char *Cvar_LatchedVariableString( const char *var_name );
void    Cvar_LatchedVariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void	Cvar_LatchedVariableStringBufferSafe( const char *var_name, char *buffer, int bufsize, int flag );
// Gordon: returns the latched value if there is one, else the normal one, empty string if not defined as usual

unsigned Cvar_Flags( const char *var_name );
// returns CVAR_NONEXISTENT if cvar doesn't exist or the flags of that particular CVAR.

void	Cvar_CommandCompletion( void(*callback)(const char *s) );
// callback with each valid string

void 	Cvar_Reset( const char *var_name );
void 	Cvar_ForceReset(const char *var_name);

void	Cvar_SetCheatState( void );
// reset all testing vars to a safe value

qboolean Cvar_Command( void );
// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)

void 	Cvar_WriteVariables( fileHandle_t f, qboolean forcewrite, qboolean nodefault );
// writes lines containing "set variable value" for all variables
// with the archive flag set to true.

void	Cvar_Init( void );

const char *Cvar_InfoString( int bit, qboolean *truncated );
const char *Cvar_InfoString_Big( int bit, qboolean *truncated );
// returns an info string containing all the cvars that have the given bit set
// in their flags ( CVAR_USERINFO, CVAR_SERVERINFO, CVAR_SYSTEMINFO, etc )
void	Cvar_InfoStringBuffer( int bit, char *buff, int buffsize );
void	Cvar_CheckRange( cvar_t *cv, const char *minVal, const char *maxVal, cvarValidator_t type );
void	Cvar_SetDescription( cvar_t *var, const char *var_description );

void	Cvar_SetGroup( cvar_t *var, cvarGroup_t group );
int		Cvar_CheckGroup( cvarGroup_t group );
void	Cvar_ResetGroup( cvarGroup_t group, qboolean resetModifiedFlags );

void	Cvar_Restart( qboolean unsetVM );

void	Cvar_CompleteCvarName( char *args, int argNum );

extern	int			cvar_modifiedFlags;
// whenever a cvar is modifed, its flags will be OR'd into this, so
// a single check can determine if any CVAR_USERINFO, CVAR_SERVERINFO,
// etc, variables have been modified since the last check.  The bit
// can then be cleared to allow another change detection.

unsigned int crc32_buffer( const byte *buf, unsigned int len );

/*
==============================================================

FILESYSTEM

No stdio calls should be used by any part of the game, because
we need to deal with all sorts of directory and seperator char
issues.
==============================================================
*/

#define BASEGAME "etmain"

// referenced flags
// these are in loop specific order so don't change the order
#define FS_GENERAL_REF	0x01
#define FS_UI_REF		0x02
#define FS_CGAME_REF	0x04
// number of id paks that will never be autodownloaded from baseq3
#define NUM_ID_PAKS     9

typedef enum {
	H_SYSTEM,
	H_RENDERER,
	H_QAGAME,
	H_CGAME,
	H_Q3UI,
	H_MAX
} handleOwner_t;

#define FS_MATCH_EXTERN (1<<0)
#define FS_MATCH_PURE   (1<<1)
#define FS_MATCH_UNPURE (1<<2)
#define FS_MATCH_STICK  (1<<3)
#define FS_MATCH_PK3s   (FS_MATCH_PURE | FS_MATCH_UNPURE)
#define FS_MATCH_ANY    (FS_MATCH_EXTERN | FS_MATCH_PURE | FS_MATCH_UNPURE)

#define	MAX_FILE_HANDLES	64
#define	FS_INVALID_HANDLE	0

#define	MAX_FOUND_FILES		0x5000

#ifdef DEDICATED
#define Q3CONFIG_CFG "etconfig_server.cfg"
#define CONSOLE_HISTORY_FILE "ethistory_server"
#else
#define Q3CONFIG_CFG "etconfig.cfg"
#define CONSOLE_HISTORY_FILE "ethistory"
#endif

#ifdef _WIN32
	#define Q_rmdir _rmdir
#else
	#define Q_rmdir rmdir
#endif

typedef	time_t fileTime_t;
#if defined  (_MSC_VER) && defined (__clang__)
typedef	_off_t  fileOffset_t;
#else
typedef	off_t  fileOffset_t;
#endif

qboolean FS_Initialized( void );

void	FS_InitFilesystem ( void );
void	FS_Shutdown( qboolean closemfp );

qboolean	FS_ConditionalRestart( int checksumFeed, qboolean clientRestart );

void	FS_Restart( int checksumFeed );
// shutdown and restart the filesystem so changes to fs_gamedir can take effect

void	FS_Reload( void );

char	**FS_ListFiles( const char *directory, const char *extension, int *numfiles );
// directory should not have either a leading or trailing /
// if extension is "/", only subdirectories will be returned
// the returned files will not include any directories or /

char	**FS_ListFilesEx( const char *path, const char **extensions, int numExts, int *numfiles );

void	FS_FreeFileList( char **list );

qboolean FS_FileExists( const char *file );

char   *FS_BuildOSPath( const char *base, const char *game, const char *qpath );

qboolean FS_CompareZipChecksum( const char *zipfile );
int		FS_GetZipChecksum( const char *zipfile );

int		FS_LoadStack( void );

int		FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize );

fileHandle_t	FS_FOpenFileWrite( const char *qpath );
fileHandle_t	FS_FOpenFileAppend( const char *filename );
// will properly create any needed paths and deal with seperater character issues

qboolean FS_ResetReadOnlyAttribute( const char *filename );

qboolean FS_SV_FileExists( const char *file );

fileHandle_t FS_SV_FOpenFileWrite( const char *filename );
int		FS_SV_FOpenFileRead( const char *filename, fileHandle_t *fp );
void	FS_SV_Rename( const char *from, const char *to );
int		FS_FOpenFileRead( const char *qpath, fileHandle_t *file, qboolean uniqueFILE );
// if uniqueFILE is true, then a new FILE will be fopened even if the file
// is found in an already open pak file.  If uniqueFILE is false, you must call
// FS_FCloseFile instead of fclose, otherwise the pak FILE would be improperly closed
// It is generally safe to always set uniqueFILE to true, because the majority of
// file IO goes through FS_ReadFile, which Does The Right Thing already.
/* TTimo
show_bug.cgi?id=506
added exclude flag to filter out regular dirs or pack files on demand
would rather have used FS_FOpenFileRead(..., int filter_flag = 0)
but that's a C++ construct ..
*/
#define FS_EXCLUDE_DIR 0x1
#define FS_EXCLUDE_PK3 0x2
#define FS_EXCLUDE_ETMAIN 0x4
#define FS_EXCLUDE_OTHERGAMES 0x8
int FS_FOpenFileRead_Filtered( const char *qpath, fileHandle_t *file, qboolean uniqueFILE, int filter_flag );

void FS_SetFilterFlag( int flag );

int FS_TouchFileInPak( const char *filename );

void FS_BypassPure( void );
void FS_RestorePure( void );

int FS_Home_FOpenFileRead( const char *filename, fileHandle_t *file );

qboolean FS_FileIsInPAK( const char *filename, int *pChecksum, char *pakName );
// returns qtrue if a file is in the PAK file, otherwise qfalse

int     FS_Delete( const char *filename );    // only works inside the 'save' directory (for deleting savegames/images)

int		FS_PakIndexForHandle( fileHandle_t f );

// returns pak index or -1 if file is not in pak
extern int fs_lastPakIndex;

extern qboolean fs_reordered;

int		FS_Write( const void *buffer, int len, fileHandle_t f );

int		FS_Read( void *buffer, int len, fileHandle_t f );
// properly handles partial reads and reads from other dlls

void	FS_FCloseFile( fileHandle_t f );
// note: you can't just fclose from another DLL, due to MS libc issues

int		FS_ReadFile( const char *qpath, void **buffer );
int		FS_ReadFile_Filtered( const char *qpath, void **buffer, int filter_flag );
// returns the length of the file
// a null buffer will just return the file length without loading
// as a quick check for existence. -1 length == not present
// A 0 byte will always be appended at the end, so string ops are safe.
// the buffer should be considered read-only, because it may be cached
// for other uses.

void	FS_ForceFlush( fileHandle_t f );
// forces flush on files we're writing to.

void	FS_FreeFile( void *buffer );
// frees the memory returned by FS_ReadFile

void	FS_WriteFile( const char *qpath, const void *buffer, int size );
// writes a complete file, creating any subdirectories needed

int		FS_FTell( fileHandle_t f );
// where are we?

void	FS_Flush( fileHandle_t f );

void 	QDECL FS_Printf( fileHandle_t f, const char *fmt, ... ) FORMAT_PRINTF(2, 3);
// like fprintf

int		FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );
// opens a file for reading, writing, or appending depending on the value of mode

int		FS_Seek( fileHandle_t f, long offset, fsOrigin_t origin );
// seek on a file

qboolean FS_FilenameCompare( const char *s1, const char *s2 );

const char *FS_LoadedPakNames( qboolean fake, qboolean *overflowed );
const char *FS_LoadedPakChecksums( qboolean *overflowed, int *numpaks );
// Returns a space separated string containing the checksums of all loaded pk3 files.
// Servers with sv_pure set will get this string and pass it to clients.

qboolean FS_ExcludeReference( void );
const char *FS_ReferencedPakNames( qboolean *overflowed );
const char *FS_ReferencedPakChecksums( qboolean *overflowed );
const char *FS_ReferencedPakPureChecksums( int maxlen );
// Returns a space separated string containing the checksums of all loaded 
// AND referenced pk3 files. Servers with sv_pure set will get this string 
// back from clients for pure validation 

void FS_ClearPakReferences( int flags );
// clears referenced booleans on loaded pk3s

// clear pure server pak status
void FS_ClearPureServerPaks( void );

void FS_PureServerSetReferencedPaks( const char *pakSums, const char *pakNames );
void FS_PureServerSetLoadedPaks( const char *pakSums, const char *pakNames );
// If the string is empty, all data sources will be allowed.
// If not empty, only pk3 files that match one of the space
// separated checksums will be checked for files, with the
// sole exception of .cfg files.

qboolean FS_IsPureChecksum( int sum );

qboolean FS_InvalidGameDir( const char *gamedir );
qboolean FS_idPak( const char *pak, const char *base );
qboolean FS_ComparePaks( char *neededpaks, int len, qboolean dlstring );

void FS_Rename( const char *from, const char *to );

void FS_Remove( const char *osPath );
void FS_HomeRemove( const char *homePath );

void	FS_FilenameCompletion( const char *dir, const char *ext,
		qboolean stripExt, void(*callback)(const char *s), int flags );

int FS_VM_OpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode, handleOwner_t owner );
int FS_VM_ReadFile( void *buffer, int len, fileHandle_t f, handleOwner_t owner );
int FS_VM_WriteFile( void *buffer, int len, fileHandle_t f, handleOwner_t owner );
int FS_VM_SeekFile( fileHandle_t f, long offset, fsOrigin_t origin, handleOwner_t owner );
void FS_VM_CloseFile( fileHandle_t f, handleOwner_t owner );
void FS_VM_CloseFiles( handleOwner_t owner );

const char *FS_GetCurrentGameDir( void );
const char *FS_GetBaseGameDir( void );

const char *FS_GetHomePath( void );

qboolean FS_StripExt( char *filename, const char *ext );
qboolean FS_AllowedExtension( const char *fileName, qboolean allowPk3s, const char **ext );

void *FS_LoadLibrary( const char *name );

typedef qboolean ( *fnamecallback_f )( const char *filename, int length );

void FS_SetFilenameCallback( fnamecallback_f func ); 

char *FS_BuildOSPath( const char *base, const char *game, const char *qpath );

#if !defined( DEDICATED )
extern int cl_connectedToPureServer;
qboolean FS_CL_ExtractFromPakFile( const char *path, const char *gamedir, const char *filename, const char *cvar_lastVersion );
#endif

void FS_CopyFile( const char *fromOSPath, const char *toOSPath );

qboolean FS_CreatePath( const char *OSPath );

char *FS_CopyString( const char *in );

// AVI pipes

fileHandle_t FS_PipeOpenWrite( const char *cmd, const char *filename );
void FS_PipeClose( fileHandle_t f );

gameMod_t FS_GetGameMod( void );


/*
==============================================================

DOWNLOAD

==============================================================
*/

typedef enum {
	DL_CONTINUE = 0,
	DL_DONE,
	DL_FAILED
} dlStatus_t;

// bitmask
typedef enum {
	DL_FLAG_DISCON = 0,
	DL_FLAG_URL
} dlFlags_t;


int PC_AddGlobalDefine(const char *string);
void PC_RemoveAllGlobalDefines( void );
int PC_LoadSourceHandle( const char *filename );
int PC_FreeSourceHandle( int handle );
int PC_ReadTokenHandle( int handle, pc_token_t *pc_token );
int PC_SourceFileAndLine( int handle, char *filename, int *line );
void PC_UnreadLastTokenHandle( int handle );





/*
==============================================================

Edit fields and command line history/completion

==============================================================
*/

#define	MAX_EDIT_LINE 512
#if MAX_EDIT_LINE > MAX_CMD_LINE
#error "MAX_EDIT_LINE > MAX_CMD_LINE"
#endif
typedef struct {
	int		cursor;
	int		scroll;
	int		widthInChars;
	char	buffer[MAX_EDIT_LINE];
} field_t;

void Field_Clear( field_t *edit );
void Field_AutoComplete( field_t *edit );
void Field_CompleteKeyname( void );
void Field_CompleteKeyBind( int key );
void Field_CompleteFilename( const char *dir, const char *ext, qboolean stripExt, int flags );
void Field_CompleteStringList( const char **strings, int numstrings );
void Field_CompleteIntRange( const int minval, const int maxval );
void Field_CompleteCommand( char *cmd, qboolean doCommands, qboolean doCvars );

void Con_ResetHistory( void );
void Con_SaveField( const field_t *field );
qboolean Con_HistoryGetPrev( field_t *field );
qboolean Con_HistoryGetNext( field_t *field );

/*
==============================================================

MISC

==============================================================
*/

// customizable client window title
extern char cl_title[ MAX_CVAR_VALUE_STRING ];

extern	int	CPU_Flags;

// x86 flags
#define CPU_FCOM   0x01
#define CPU_MMX    0x02
#define CPU_SSE    0x04
#define CPU_SSE2   0x08
#define CPU_SSE3   0x10
#define CPU_SSE41  0x20
#define CPU_SSE42  0x40

// ARM flags
#define CPU_ARMv7  0x01
#define CPU_IDIVA  0x02
#define CPU_VFPv3  0x04

// ALTIVEC
#define CPU_ALTIVEC 0x800

typedef struct gameInfo_s {
	qboolean spEnabled;
	int spGameTypes;
	int defaultSPGameType;
	int coopGameTypes;
	int defaultCoopGameType;
	int defaultGameType;
	qboolean usesProfiles;
} gameInfo_t;

extern gameInfo_t com_gameInfo;
extern int currentGameMod;

void Com_GetGameInfo( void );
void Com_UpdateDefaultGametype( void );

// TTimo
// centralized and cleaned, that's the max string you can send to a Com_Printf / Com_DPrintf (above gets truncated)
// bump to 8192 as 4096 may be not enough to print some data like gl extensions - CE
#define	MAXPRINTMSG	8192

char		*CopyString( const char *in );
void		Info_Print( const char *s );

void		Com_BeginRedirect (char *buffer, int buffersize, void (*flush)(const char *));
void		Com_EndRedirect( void );
void 		QDECL Com_Printf( const char *fmt, ... ) FORMAT_PRINTF(1, 2);
void 		QDECL Com_DPrintf( const char *fmt, ... ) FORMAT_PRINTF(1, 2);
void 		NORETURN QDECL Com_Error( errorParm_t code, const char *fmt, ... ) FORMAT_PRINTF(2, 3);
void 		Com_Quit_f( void );
void		Com_GameRestart( int checksumFeed, qboolean clientRestart );

int			Com_EventLoop( void );
int			Com_Milliseconds( void );	// will be journaled properly

// MD4 functions
unsigned	Com_BlockChecksum( const void *buffer, int length );

// MD5 functions

char		*Com_MD5File(const char *filename, int length, const char *prefix, int prefix_len);
char		*Com_MD5Buf( const char *data, int length, const char *data2, int length2 );
#ifndef DEDICATED
char		*Com_PBMD5File( const char *filename );
#endif

// stateless challenge functions
void		Com_MD5Init( void );
int			Com_MD5Addr( const netadr_t *addr, int timestamp );

qboolean	Com_EarlyParseCmdLine( char *commandLine, char *con_title, int title_size, int *vid_xpos, int *vid_ypos );
int			Com_Split( char *in, char **out, int outsz, int delim );

int			Com_Filter( const char *filter, const char *name );
qboolean	Com_FilterExt( const char *filter, const char *name );
qboolean	Com_HasPatterns( const char *str );
int			Com_FilterPath( const char *filter, const char *name );
int			Com_RealTime(qtime_t *qtime);
qboolean	Com_SafeMode( void );
void		Com_RunAndTimeServerPacket( const netadr_t *evFrom, msg_t *buf );

void		Com_StartupVariable( const char *match );
void        Com_SetRecommended( void );
// checks for and removes command line "+set var arg" constructs
// if match is NULL, all set commands will be executed, otherwise
// only a set with the exact name.  Only used during startup.

void		Com_WriteConfiguration( void );
int			Com_HexStrToInt( const char *str );
qboolean	Com_GetHashColor( const char *str, byte *color );


static ID_INLINE unsigned int log2pad( unsigned int v, int roundup )
{
	unsigned int x = 1;

	while ( x < v ) x <<= 1;

	if ( roundup == 0 ) {
		if ( x > v ) {
			x >>= 1;
		}
	}

	return x;
}



//bani - profile functions
void Com_TrackProfile( const char *profile_path );
qboolean Com_CheckProfile( const char *profile_path );
qboolean Com_WriteProfile( const char *profile_path );
#ifndef DEDICATED
void Com_CheckDefaultProfile( void );
#endif

extern cvar_t  *com_crashed;

extern cvar_t  *com_ignorecrash;    //bani

extern cvar_t  *com_pid;    //bani

extern	cvar_t	*com_developer;
extern	cvar_t	*com_dedicated;
extern	cvar_t	*com_speeds;
extern	cvar_t	*com_timescale;
extern	cvar_t	*com_viewlog;			// 0 = hidden, 1 = visible, 2 = minimized
extern	cvar_t	*com_version;
extern	cvar_t	*com_journal;
extern	cvar_t	*com_protocol;
extern	qboolean com_protocolCompat;
extern	cvar_t	*com_legacyVersion;

// both client and server must agree to pause
extern	cvar_t	*sv_paused;
extern	cvar_t	*sv_packetdelay;
extern	cvar_t	*sv_packetloss;
extern	cvar_t	*com_sv_running;

#ifndef DEDICATED
extern	cvar_t	*cl_paused;
extern	cvar_t	*cl_packetdelay;
extern	cvar_t	*cl_packetloss;
extern	cvar_t	*com_cl_running;
extern	cvar_t	*com_yieldCPU;
#endif

#ifdef USE_AFFINITY_MASK
extern	cvar_t	*com_affinityMask;
#endif

// com_speeds times
extern	int		time_game;
extern	int		time_frontend;
extern	int		time_backend;		// renderer backend time

extern	int		com_frameTime;

extern int com_expectedhunkusage;
extern int com_hunkusedvalue;

#ifndef DEDICATED
extern	qboolean	gw_minimized;
extern	qboolean	gw_active;
#endif

#ifndef DEDICATED
extern	int		cl_optimizedPatchServer;
#endif

extern	qboolean	com_errorEntered;

extern	fileHandle_t	com_journalDataFile;

extern	char	rconPassword2[ MAX_CVAR_VALUE_STRING ];

typedef enum {
	TAG_FREE,
	TAG_GENERAL,
	TAG_PACK,
	TAG_SEARCH_PATH,
	TAG_SEARCH_PACK,
	TAG_SEARCH_DIR,
	TAG_BOTLIB,
	TAG_RENDERER,
	TAG_CLIENTS,
	TAG_SMALL,
	TAG_STATIC,
	TAG_COUNT
} memtag_t;

/*

--- low memory ----
server vm
server clipmap
---mark---
renderer initialization (shaders, etc)
UI vm
cgame vm
renderer map
renderer models

---free---

temp file loading
--- high memory ---

*/

#if defined(_DEBUG) && !defined(BSPC)
	#define ZONE_DEBUG
#endif

#ifdef ZONE_DEBUG
#define Z_TagMalloc(size, tag)			Z_TagMallocDebug(size, tag, #size, __FILE__, __LINE__)
#define Z_Malloc(size)					Z_MallocDebug(size, #size, __FILE__, __LINE__)
#define S_Malloc(size)					S_MallocDebug(size, #size, __FILE__, __LINE__)
void *Z_TagMallocDebug( int size, memtag_t tag, char *label, char *file, int line );	// NOT 0 filled memory
void *Z_MallocDebug( int size, char *label, char *file, int line );			// returns 0 filled memory
void *S_MallocDebug( int size, char *label, char *file, int line );			// returns 0 filled memory
#else
void *Z_TagMalloc( int size, memtag_t tag );	// NOT 0 filled memory
void *Z_Malloc( int size );			// returns 0 filled memory
void *S_Malloc( int size );			// NOT 0 filled memory only for small allocations
#endif
void Z_Free( void *ptr );
int Z_FreeTags( memtag_t tag );
int Z_AvailableMemory( void );
void Z_LogHeap( void );

void Hunk_Clear( void );
void Hunk_ClearToMark( void );
void Hunk_SetMark( void );
qboolean Hunk_CheckMark( void );
void Hunk_ClearTempMemory( void );
void *Hunk_AllocateTempMemory( int size );
void Hunk_FreeTempMemory( void *buf );
int	Hunk_MemoryRemaining( void );
void Hunk_Log( void);

void Com_TouchMemory( void );

// commandLine should not include the executable name (argv[0])
void Com_Init( char *commandLine );
void Com_FrameInit( void );
void Com_Frame( qboolean noDelay );

/*
==============================================================

CLIENT / SERVER SYSTEMS

==============================================================
*/

//
// client interface
//
void CL_Init( void );
void CL_ClearStaticDownload( void );
qboolean CL_Disconnect( qboolean showMainMenu );
void CL_ResetOldGame( void );
void CL_Shutdown( const char *finalmsg, qboolean quit );
void CL_Frame( int msec, int realMsec );
qboolean CL_GameCommand( void );
qboolean CL_CgameRunning( void );
void CL_KeyEvent (int key, qboolean down, unsigned time);

void CL_CharEvent( int key );
// char events are for field typing, not game control

void CL_MouseEvent( int dx, int dy /*, int time*/ );

void CL_JoystickEvent( int axis, int value /*, int time*/ );

void CL_PacketEvent( const netadr_t *from, msg_t *msg );

void CL_ConsolePrint( const char *text );

void CL_MapLoading( void );
// do a screen update before starting to load a map
// when the server is going to load a new map, the entire hunk
// will be cleared, so the client must shutdown cgame, ui, and
// the renderer

void	CL_ForwardCommandToServer( const char *string );
// adds the current command line as a clc_clientCommand to the client message.
// things like godmode, noclip, etc, are commands directed to the server,
// so when they are typed in at the console, they will need to be forwarded.

void CL_CDDialog( void );
// bring up the "need a cd to play" dialog

void CL_ShutdownAll( void );
// shutdown all the client stuff

void CL_ClearMemory( void );
// clear memory

void CL_FlushMemory( void );
// dump all memory on an error

void CL_StartHunkUsers( void );
// start all the client stuff using the hunk

//void CL_Snd_Restart(void);
// Restart sound subsystem

void Key_KeynameCompletion( void(*callback)(const char *s) );
// for keyname autocompletion

void Key_WriteBindings( fileHandle_t f );
// for writing the config files

void S_ClearSoundBuffer( qboolean killStreaming );  //----(SA)	modified
// call before filesystem access

void SCR_DebugGraph( float value );   // FIXME: move logging to common?

void CL_SystemInfoChanged( qboolean onlyGame );
qboolean CL_GameSwitch( void );

qboolean CL_ConnectedToRemoteServer( void );

// AVI files have the start of pixel lines 4 byte-aligned
#define AVI_LINE_PADDING 4

//
// server interface
//
void SV_Init( void );
void SV_Shutdown( const char *finalmsg );
void SV_Frame( int msec );
void SV_TrackCvarChanges( void );
void SV_PacketEvent( const netadr_t *from, msg_t *msg );
int SV_FrameMsec( void );
qboolean SV_GameCommand( void );
int SV_SendQueuedPackets( void );

void SV_AddDedicatedCommands( void );
void SV_RemoveDedicatedCommands( void );


//
// UI interface
//
qboolean UI_GameCommand( void );
qboolean UI_usesUniqueCDKey(void);

/*
==============================================================

NON-PORTABLE SYSTEM SERVICES

==============================================================
*/

typedef enum {
	AXIS_SIDE,
	AXIS_FORWARD,
	AXIS_UP,
	AXIS_ROLL,
	AXIS_YAW,
	AXIS_PITCH,
	MAX_JOYSTICK_AXIS
} joystickAxis_t;

typedef enum {
  // bk001129 - make sure SE_NONE is zero
	SE_NONE = 0,	// evTime is still valid
	SE_KEY,		// evValue is a key code, evValue2 is the down flag
	SE_CHAR,	// evValue is an ascii char
	SE_MOUSE,	// evValue and evValue2 are relative signed x / y moves
	SE_JOYSTICK_AXIS,	// evValue is an axis number and evValue2 is the current state (-127 to 127)
	SE_CONSOLE,	// evPtr is a char*
	SE_MAX,
} sysEventType_t;

typedef struct {
	int				evTime;
	sysEventType_t	evType;
	int				evValue, evValue2;
	int				evPtrLength;	// bytes of data pointed to by evPtr, for journaling
	void			*evPtr;			// this must be manually freed if not NULL
} sysEvent_t;

void	Sys_Init( void );
void	Sys_QueEvent( int evTime, sysEventType_t evType, int value, int value2, int ptrLength, void *ptr );
void	Sys_SendKeyEvents( void );
void	Sys_Sleep( int msec );
char	*Sys_ConsoleInput( void );

qboolean	Sys_IsNumLockDown( void );

int Sys_GetPID( void );

// general development dll loading for virtual machine testing
void	*QDECL Sys_LoadGameDll( const char *name, vmMain_t *entryPoint, dllSyscall_t systemcalls );

void	NORETURN QDECL Sys_Error( const char *error, ...) FORMAT_PRINTF(1, 2);
void	NORETURN Sys_Quit (void);
char	*Sys_GetClipboardData( void );	// note that this isn't journaled...
void	Sys_SetClipboardBitmap( const byte *bitmap, int length );

void	Sys_Print( const char *msg );

// dedicated console status, win32-only at the moment
void	QDECL Sys_SetStatus( const char *format, ...) FORMAT_PRINTF(1, 2);

#ifdef USE_AFFINITY_MASK
uint64_t Sys_GetAffinityMask( void );
qboolean Sys_SetAffinityMask( const uint64_t mask );
#endif

// Sys_Milliseconds should only be used for profiling purposes,
// any game related timing information should come from event timestamps
int		Sys_Milliseconds( void );
int64_t	Sys_Microseconds( void );

void	Sys_SnapVector( float *vector );

qboolean Sys_RandomBytes( byte *string, int len );


void	Sys_ShowConsole( int level, qboolean quitOnClose );
void	Sys_SetErrorText( const char *text );

void	Sys_SendPacket( int length, const void *data, const netadr_t *to );

qboolean	Sys_StringToAdr( const char *s, netadr_t *a, netadrtype_t family );
//Does NOT parse port numbers, only base addresses.

qboolean	Sys_IsLANAddress(const netadr_t *adr);
void		Sys_ShowIP(void);

qboolean	Sys_Mkdir( const char *path );
FILE	*Sys_FOpen( const char *ospath, const char *mode );
qboolean Sys_ResetReadOnlyAttribute( const char *ospath );
qboolean Sys_IsHiddenFolder( const char *ospath );

const char *Sys_Pwd( void );
const char *Sys_DefaultBasePath( void );
const char *Sys_DefaultHomePath( void );
const char *Sys_SteamPath( void );
const char *Sys_GogPath( void );
const char *Sys_MicrosoftStorePath( void );

char **Sys_ListFiles( const char *directory, const char *extension, const char *filter, int *numfiles, qboolean wantsubs );
void Sys_FreeFileList( char **list );

qboolean Sys_GetFileStats( const char *filename, fileOffset_t *size, fileTime_t *mtime, fileTime_t *ctime );
int Sys_PathIsDir( const char *path );

void Sys_BeginProfiling( void );
void Sys_EndProfiling( void );

qboolean Sys_LowPhysicalMemory( void );

void *Sys_LoadLibrary( const char *name );
void *Sys_LoadFunction( void *handle, const char *name );
int   Sys_LoadFunctionErrors( void );
void  Sys_UnloadLibrary( void *handle );

typedef struct {
	int commandId;
} renderOmnibot_t;

void	Sys_OmnibotLoad();
void	Sys_OmnibotUnLoad();
const void * Sys_OmnibotRender( const void *data );

#ifndef DEDICATED
void Sys_SteamInit();
void Sys_SteamShutdown();
#endif

// NOTE TTimo - on win32 the cwd is prepended .. non portable behaviour
void Sys_StartProcess( const char *exeName, qboolean doexit );            // NERVE - SMF
void Sys_OpenURL( const char *url, qboolean doexit );                       // NERVE - SMF
int Sys_GetHighQualityCPU();
float Sys_GetCPUSpeed( void );

#ifndef _WIN32
// TTimo only on linux .. maybe on Mac too?
// will OR with the existing mode (chmod ..+..)
void Sys_Chmod( const char *file, int mode );
#endif

// adaptive huffman functions
void Huff_Compress( msg_t *buf, int offset );
void Huff_Decompress( msg_t *buf, int offset );

// static huffman functions
void HuffmanPutBit( byte* fout, int32_t bitIndex, int bit );
int HuffmanPutSymbol( byte* fout, uint32_t offset, int symbol );
int HuffmanGetBit( const byte* buffer, int bitIndex );
int HuffmanGetSymbol( unsigned int* symbol, const byte* buffer, int bitIndex );

#define	SV_ENCODE_START		4
#define	SV_DECODE_START		12
#define	CL_ENCODE_START		12
#define	CL_DECODE_START		4


void Com_GetHunkInfo( int* hunkused, int* hunkexpected );

// functional gate syscall number
#define COM_TRAP_GETVALUE 700

// TTimo
// dll checksuming stuff, centralizing OS-dependent parts
// *_SHIFT is the shifting we applied to the reference string

#if defined( _WIN32 )

// qagame_mp_x86.dll
#define SYS_DLLNAME_QAGAME "qagame_mp_" ARCH_STRING DLL_EXT

// cgame_mp_x86.dll
#define SYS_DLLNAME_CGAME "cgame_mp_" ARCH_STRING DLL_EXT

// ui_mp_x86.dll
#define SYS_DLLNAME_UI "ui_mp_" ARCH_STRING DLL_EXT

#elif defined( __linux__ ) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)

// qagame.mp.i386.so
#define SYS_DLLNAME_QAGAME "qagame.mp." ARCH_STRING DLL_EXT

// cgame.mp.i386.so
#define SYS_DLLNAME_CGAME "cgame.mp." ARCH_STRING DLL_EXT

// ui.mp.i386.so
#define SYS_DLLNAME_UI "ui.mp." ARCH_STRING DLL_EXT

#elif defined(__APPLE__) || defined(__APPLE_CC__)

#ifdef _DEBUG
// qagame_d_mac
	#define SYS_DLLNAME_QAGAME "qagame_d_mac"

// cgame_d_mac
	#define SYS_DLLNAME_CGAME "cgame_d_mac"

// ui_d_mac
	#define SYS_DLLNAME_UI "ui_d_mac"
#else
// qagame_mac
	#define SYS_DLLNAME_QAGAME "qagame_mac"

// cgame_mac
	#define SYS_DLLNAME_CGAME "cgame_mac"

// ui_mac
	#define SYS_DLLNAME_UI "ui_mac"
#endif

#else

#error unknown OS

#endif

#endif // _QCOMMON_H_
