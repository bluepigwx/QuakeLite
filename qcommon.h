#pragma once

#include "q_shared.h"
#include "qfiles.h"


#define	BASEDIRNAME	"baseq2"



extern	cvar_t* dedicated;

//============================================================================

void Z_Free(void* ptr);
void* Z_Malloc(int size);			// returns 0 filled memory
void* Z_TagMalloc(int size, int tag);
void Z_FreeTags(int tag);

//============================================================================




//============================================================================

typedef struct sizebuf_s
{
	bool	allowoverflow;	// if false, do a Com_Error
	bool	overflowed;		// set to true if the buffer size failed �Ƿ�������һ�λ���д���ʱ��������ÿռ䲻�������д��ռ�����ݳ��
	unsigned char* data;
	int		maxsize;
	int		cursize;
	int		readcount;
} sizebuf_t;


void SZ_Init(sizebuf_t* buf, unsigned char* data, int length);
void SZ_Clear(sizebuf_t* buf);
void* SZ_GetSpace(sizebuf_t* buf, int length);
void SZ_Write(sizebuf_t* buf, void* data, int length);
void SZ_Print(sizebuf_t* buf, char* data);	// strcats onto the sizebuf

//==============================================================================




//============================================================================
// ����ϵͳ������ά���ӿ�

int	COM_Argc(void);
const char* COM_Argv(int arg);	// range and null checked
void COM_ClearArgv(int arg);
int COM_CheckParm(char* parm);
void COM_AddParm(char* parm);
void COM_InitArgv(int argc, char** argv);

//============================================================================




//==============================================================================

void Qcommon_Init(int argc, char** argv);
void Qcommon_Frame(int msec);
void Qcommon_Shutdown(void);

//==============================================================================




//==============================================================================
/*
CMD

Command text buffering and command execution

Any number of commands can be added in a frame, from several different sources.
Most commands come from either keybindings or console line input, but remote
servers can also send across commands and entire text files can be execed.

The + command line options are also added to the command buffer.

The game starts with a Cbuf_AddText ("exec quake.rc\n"); Cbuf_Execute ();

*/

#define	EXEC_NOW	0		// don't return until completed
#define	EXEC_INSERT	1		// insert at current position, but don't run yet
#define	EXEC_APPEND	2		// add to end of the command buffer

void Cbuf_Init(void);
// allocates an initial text buffer that will grow as needed

void Cbuf_AddText(const char* text);
// as new commands are generated from the console or keybindings,
// the text is added to the end of the command buffer.

void Cbuf_InsertText(char* text);
// when a command wants to issue other commands immediately, the text is
// inserted at the beginning of the buffer, before any remaining unexecuted
// commands.

void Cbuf_ExecuteText(int exec_when, char* text);
// this can be used in place of either Cbuf_AddText or Cbuf_InsertText

void Cbuf_AddEarlyCommands(bool clear);
// adds all the +set commands from the command line

bool Cbuf_AddLateCommands(void);
// adds all the remaining + commands from the command line
// Returns true if any late commands were added, which
// will keep the demoloop from immediately starting

void Cbuf_Execute(void);
// Pulls off \n terminated lines of text from the command buffer and sends
// them through Cmd_ExecuteString.  Stops when the buffer is empty.
// Normally called once per frame, but may be explicitly invoked.
// Do not call inside a command function!

void Cbuf_CopyToDefer(void);
void Cbuf_InsertFromDefer(void);
// These two functions are used to defer any pending commands while a map
// is being loaded

//===========================================================================




//===========================================================================
/*

Command execution takes a null terminated string, breaks it into tokens,
then searches for a command or variable that matches the first token.

*/

typedef void (*xcommand_t) (void);

void Cmd_Init(void);

void Cmd_AddCommand(const char* cmd_name, xcommand_t function);
// called by the init functions of other parts of the program to
// register commands and functions to call for them.
// The cmd_name is referenced later, so it should not be in temp memory
// if function is NULL, the command will be forwarded to the server
// as a clc_stringcmd instead of executed locally
void Cmd_RemoveCommand(char* cmd_name);

bool Cmd_Exists(char* cmd_name);
// used by the cvar code to check for cvar / command name overlap

char* Cmd_CompleteCommand(char* partial);
// attempts to match a partial command for automatic command line completion
// returns NULL if nothing fits


// ���ڽ�����token���ַ���������ͨ������Ľӿڷ������Ĳ���
int Cmd_Argc(void);
const char* Cmd_Argv(int arg);
char* Cmd_Args(void);
// The functions that execute commands get their parameters with these
// functions. Cmd_Argv () will return an empty string, not a NULL
// if arg > argc, so string operations are always safe.

void Cmd_TokenizeString(char* text, bool macroExpand);
// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.

void Cmd_ExecuteString(char* text);
// Parses a single line of text into arguments and tries to execute it
// as if it was typed at the console

void Cmd_ForwardToServer(void);
// adds the current command line as a clc_stringcmd to the client message.
// things like godmode, noclip, etc, are commands directed to the server,
// so when they are typed in at the console, they will need to be forwarded.

//===========================================================================




//===========================================================================
/*
CVAR

cvar_t variables are used to hold scalar or string variables that can be changed or displayed at the console or prog code as well as accessed directly
in C code.

The user can access cvars from the console in three ways:
r_draworder			prints the current value
r_draworder 0		sets the current value to 0
set r_draworder 0	as above, but creates the cvar if not present
Cvars are restricted from having the same names as commands to keep this
interface from being ambiguous.
*/

extern	cvar_t* cvar_vars;

cvar_t* Cvar_Get(const char* var_name, const char* value, int flags);
// creates the variable if it doesn't exist, or returns the existing one
// if it exists, the value will not be changed, but flags will be ORed in
// that allows variables to be unarchived without needing bitflags

cvar_t* Cvar_Set(const char* var_name, const char* value);
// will create the variable if it doesn't exist

cvar_t* Cvar_ForceSet(char* var_name, char* value);
// will set the variable even if NOSET or LATCH

cvar_t* Cvar_FullSet(const char* var_name, const char* value, int flags);

void Cvar_SetValue(char* var_name, float value);
// expands value to a string and calls Cvar_Set

float Cvar_VariableValue(char* var_name);
// returns 0 if not defined or non numeric

const char* Cvar_VariableString(const char* var_name);
// returns an empty string if not defined

char* Cvar_CompleteVariable(char* partial);
// attempts to match a partial variable name for command line completion
// returns NULL if nothing fits

void Cvar_GetLatchedVars(void);
// any CVAR_LATCHED variables that have been set will now take effect

bool Cvar_Command(void);
// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)

void Cvar_WriteVariables(char* path);
// appends lines containing "set variable value" for all variables
// with the archive flag set to true.

void Cvar_Init(void);

char* Cvar_Userinfo(void);
// returns an info string containing all the CVAR_USERINFO cvars

char* Cvar_Serverinfo(void);
// returns an info string containing all the CVAR_SERVERINFO cvars

extern	bool userinfo_modified;
// this is set each time a CVAR_USERINFO variable is changed
// so that the client knows to send it to the server

//==========================================================================




//==========================================================================
/*
FILESYSTEM

*/

void FS_InitFilesystem(void);
void FS_SetGamedir(char* dir);
char* FS_Gamedir(void);
char* FS_NextPath(char* prevpath);
void FS_ExecAutoexec(void);
int	FS_LoadFile(const char* path, void** buffer);
void FS_FreeFile(void* buffer);

//==========================================================================



char* CopyString(const char* in);
void Com_Printf(const char* fmt, ...);
int Com_ServerState(void);		// this should have just been a cvar...
void Com_DPrintf(const char* fmt, ...);