#pragma once


#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


typedef unsigned char byte;


#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	80		// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		128		// max length of an individual token


#define	MAX_QPATH			64		// max length of a quake game pathname
#define	MAX_OSPATH			128		// max length of a filesystem pathname


#define	ERR_FATAL			0		// exit the entire game with a popup window
#define	ERR_DROP			1		// print to console and disconnect from game
#define	ERR_DISCONNECT		2		// don't kill server

//=============================================
// 
// portable case insensitive compare
int Q_stricmp(char* s1, char* s2);
int Q_strcasecmp(const char* s1, const char* s2);
int Q_strncasecmp(const char* s1, const char* s2, int n);

//=============================================



//============================================================================

const char* COM_Parse(char** data_p);
// data is an in/out parm, returns a parsed out token

//============================================================================


void Com_sprintf(char* dest, int size, const char* fmt, ...);

char* va(const char* format, ...);




/*
==========================================================

CVARS (console variables)

==========================================================
*/

#ifndef CVAR
#define	CVAR

#define	CVAR_ARCHIVE	1	// set to cause it to be saved to vars.rc
#define	CVAR_USERINFO	2	// added to userinfo  when changed
#define	CVAR_SERVERINFO	4	// added to serverinfo when changed
#define	CVAR_NOSET		8	// don't allow change from console at all,
// but can be set from the command line
#define	CVAR_LATCH		16	// save changes until server restart

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s
{
	char* name;
	char* string;
	char* latched_string;	// for CVAR_LATCH vars
	int flags;
	bool modified;	// set each time the cvar is changed
	float value;
	struct cvar_s* next;
} cvar_t;

#endif		// CVAR




//=============================================

//
// key / value info strings
//
#define	MAX_INFO_KEY		64
#define	MAX_INFO_VALUE		64
#define	MAX_INFO_STRING		512

const char* Info_ValueForKey(char* s, char* key);
void Info_RemoveKey(char* s, char* key);
void Info_SetValueForKey(char* s, char* key, char* value);
bool Info_Validate(char* s);



//=============================================

char* COM_SkipPath(char* pathname);
void COM_StripExtension(char* in, char* out);
void COM_FileBase(char* in, char* out);
void COM_FilePath(char* in, char* out);
void COM_DefaultExtension(char* path, char* extension);


/*
==============================================================

SYSTEM SPECIFIC

==============================================================
*/
extern	int	curtime;		// time returned by last Sys_Milliseconds

int Sys_Milliseconds(void);
int	Sys_Mkdir(char* path);


// directory searching
#define SFF_ARCH    0x01
#define SFF_HIDDEN  0x02
#define SFF_RDONLY  0x04
#define SFF_SUBDIR  0x08
#define SFF_SYSTEM  0x10


/*
** pass in an attribute mask of things you wish to REJECT
*/
char* Sys_FindFirst(char* path, unsigned musthave, unsigned canthave);
char* Sys_FindNext(unsigned musthave, unsigned canthave);
void Sys_FindClose(void);




// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error(const char* error, ...);
void Com_Printf(const char* msg, ...);

void Com_Error(int code, const char* fmt, ...);

char* va(char* format, ...);