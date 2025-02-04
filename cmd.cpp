﻿#include "qcommon.h"


#define	MAX_ALIAS_NAME	32

typedef struct cmdalias_s
{
	struct cmdalias_s* next;
	char	name[MAX_ALIAS_NAME];
	char* value;
} cmdalias_t;

cmdalias_t* cmd_alias;

bool	cmd_wait;

#define	ALIAS_LOOP_COUNT	16
int		alias_count;		// for detecting runaway loops


/*
=============================================================================

						COMMAND BUFFER

=============================================================================
*/

sizebuf_t	cmd_text;
unsigned char cmd_text_buf[8192];
char defer_text_buf[8192];

/*
============
Cbuf_Init
============
*/
void Cbuf_Init(void)
{
	SZ_Init(&cmd_text, cmd_text_buf, sizeof(cmd_text_buf));
}

/*
============
Cbuf_AddText

Adds command text at the end of the buffer
============
*/
void Cbuf_AddText(const char* text)
{
	int		l;

	l = static_cast<int>(strlen(text));

	if (cmd_text.cursize + l >= cmd_text.maxsize)
	{
		Com_Printf("Cbuf_AddText: overflow\n");
		return;
	}
	SZ_Write(&cmd_text, (void*)text, static_cast<int>(strlen(text)));
}


/*
============
Cbuf_InsertText

Adds command text immediately after the current command
Adds a \n to the text
FIXME: actually change the command buffer to do less copying
============
*/
void Cbuf_InsertText(char* text)
{
	char* temp;
	int		templen;

	// copy off any commands still remaining in the exec buffer
	templen = cmd_text.cursize;
	if (templen)
	{
		temp = (char*)Z_Malloc(templen);
		memcpy(temp, cmd_text.data, templen);
		SZ_Clear(&cmd_text);
	}
	else
		temp = NULL;	// shut up compiler

// add the entire text of the file
	Cbuf_AddText(text);

	// add the copied off data
	if (templen)
	{
		SZ_Write(&cmd_text, temp, templen);
		Z_Free(temp);
	}
}


/*
============
Cbuf_CopyToDefer
============
*/
void Cbuf_CopyToDefer(void)
{
	memcpy(defer_text_buf, cmd_text_buf, cmd_text.cursize);
	defer_text_buf[cmd_text.cursize] = 0;
	cmd_text.cursize = 0;
}

/*
============
Cbuf_InsertFromDefer
============
*/
void Cbuf_InsertFromDefer(void)
{
	Cbuf_InsertText(defer_text_buf);
	defer_text_buf[0] = 0;
}


/*
============
Cbuf_ExecuteText
============
*/
void Cbuf_ExecuteText(int exec_when, char* text)
{
	switch (exec_when)
	{
	case EXEC_NOW:
		Cmd_ExecuteString(text);
		break;
	case EXEC_INSERT:
		Cbuf_InsertText(text);
		break;
	case EXEC_APPEND:
		Cbuf_AddText(text);
		break;
	default:
		Com_Error(ERR_FATAL, "Cbuf_ExecuteText: bad exec_when");
	}
}

/*
============
Cbuf_Execute
============
*/
void Cbuf_Execute(void)
{
	int		i;
	char* text;
	char	line[1024];
	int		quotes;

	alias_count = 0;		// don't allow infinite alias loops

	while (cmd_text.cursize)
	{
		// find a \n or ; line break
		text = (char*)cmd_text.data;

		quotes = 0;
		for (i = 0; i < cmd_text.cursize; i++)
		{
			if (text[i] == '"')
				quotes++;
			if (!(quotes & 1) && text[i] == ';')
				break;	// don't break if inside a quoted string
			if (text[i] == '\n')
				break;
		}

		memcpy(line, text, i);
		line[i] = 0;

		// delete the text from the command buffer and move remaining commands down
		// this is necessary because commands (exec, alias) can insert data at the
		// beginning of the text buffer

		if (i == cmd_text.cursize)
			cmd_text.cursize = 0;
		else
		{
			i++;
			cmd_text.cursize -= i;
			memmove(text, text + i, cmd_text.cursize);
		}

		// execute the command line
		Cmd_ExecuteString(line);

		if (cmd_wait)
		{
			// skip out while text still remains in buffer, leaving it
			// for next frame
			cmd_wait = false;
			break;
		}
	}
}


/*
===============
Cbuf_AddEarlyCommands

Adds command line parameters as script statements
Commands lead with a +, and continue until another +

Set commands are added early, so they are guaranteed to be set before
the client and server initialize for the first time.

Other commands are added late, after all initialization is complete.
===============
*/
void Cbuf_AddEarlyCommands(bool clear)
{
	int		i;
	const char* s;

	for (i = 0; i < COM_Argc(); i++)
	{
		s = COM_Argv(i);
		if (strcmp(s, "+set"))
			continue;
		Cbuf_AddText(va("set %s %s\n", COM_Argv(i + 1), COM_Argv(i + 2)));
		if (clear)
		{
			COM_ClearArgv(i);
			COM_ClearArgv(i + 1);
			COM_ClearArgv(i + 2);
		}
		i += 2;
	}
}




/*
==============================================================================

						SCRIPT COMMANDS

==============================================================================
*/


/*
============
Cmd_Wait_f

Causes execution of the remainder of the command buffer to be delayed until
next frame.  This allows commands like:
bind g "impulse 5 ; +attack ; wait ; -attack ; impulse 2"
============
*/
void Cmd_Wait_f(void)
{
	cmd_wait = true;
}

/*
===============
Cmd_Exec_f
===============
*/
void Cmd_Exec_f(void)
{
	char* f, * f2;
	int		len;

	if (Cmd_Argc() != 2)
	{
		Com_Printf("exec <filename> : execute a script file\n");
		return;
	}

	len = FS_LoadFile(Cmd_Argv(1), (void**)&f);
	if (!f)
	{
		Com_Printf("couldn't exec %s\n", Cmd_Argv(1));
		return;
	}
	Com_Printf("execing %s\n", Cmd_Argv(1));

	// the file doesn't have a trailing 0, so we need to copy it off
	f2 = (char*)Z_Malloc(len + 1);
	memcpy(f2, f, len);
	f2[len] = 0;

	Cbuf_InsertText(f2);

	Z_Free(f2);
	FS_FreeFile(f);
}


/*
===============
Cmd_Echo_f

Just prints the rest of the line to the console
===============
*/
void Cmd_Echo_f(void)
{
	int		i;

	for (i = 1; i < Cmd_Argc(); i++)
		Com_Printf("%s ", Cmd_Argv(i));
	Com_Printf("\n");
}


/*
===============
Cmd_Alias_f

Creates a new command that executes a command string (possibly ; seperated)
===============
*/
void Cmd_Alias_f(void)
{
	cmdalias_t* a;
	char		cmd[1024];
	int			i, c;
	const char* s;

	if (Cmd_Argc() == 1)
	{
		Com_Printf("Current alias commands:\n");
		for (a = cmd_alias; a; a = a->next)
			Com_Printf("%s : %s\n", a->name, a->value);
		return;
	}

	s = Cmd_Argv(1);
	if (strlen(s) >= MAX_ALIAS_NAME)
	{
		Com_Printf("Alias name is too long\n");
		return;
	}

	// if the alias already exists, reuse it
	for (a = cmd_alias; a; a = a->next)
	{
		if (!strcmp(s, a->name))
		{
			Z_Free(a->value);
			break;
		}
	}

	if (!a)
	{
		a = (cmdalias_t*)Z_Malloc(sizeof(cmdalias_t));
		a->next = cmd_alias;
		cmd_alias = a;
	}
	strcpy(a->name, s);

	// copy the rest of the command line
	cmd[0] = 0;		// start out with a null string
	c = Cmd_Argc();
	for (i = 2; i < c; i++)
	{
		strcat_s(cmd, Cmd_Argv(i));
		if (i != (c - 1))
			strcat_s(cmd, " ");
	}
	strcat_s(cmd, "\n");

	a->value = CopyString(cmd);
}



/*
=============================================================================

					COMMAND EXECUTION

=============================================================================
*/

typedef struct cmd_function_s
{
	struct cmd_function_s* next;
	char* name;
	xcommand_t				function;
} cmd_function_t;


static	int			cmd_argc;
static	char* cmd_argv[MAX_STRING_TOKENS];
static	const char* cmd_null_string = "";
static	char		cmd_args[MAX_STRING_CHARS];

static	cmd_function_t* cmd_functions;		// possible commands to execute

/*
============
Cmd_Argc
============
*/
int Cmd_Argc(void)
{
	return cmd_argc;
}

/*
============
Cmd_Argv
============
*/
const char* Cmd_Argv(int arg)
{
	if (arg >= cmd_argc)
		return cmd_null_string;
	return cmd_argv[arg];
}

/*
============
Cmd_Args

Returns a single string containing argv(1) to argv(argc()-1)
============
*/
char* Cmd_Args(void)
{
	return cmd_args;
}


/*
======================
Cmd_MacroExpandString
======================
*/
char* Cmd_MacroExpandString(char* text)
{
	int		i, j, count, len;
	bool	inquote;
	char* scan;
	static	char	expanded[MAX_STRING_CHARS];
	char	temporary[MAX_STRING_CHARS];
	const char* token;
	char* start;

	inquote = false;
	scan = text;

	len = static_cast<int>(strlen(scan));
	if (len >= MAX_STRING_CHARS)
	{
		Com_Printf("Line exceeded %i chars, discarded.\n", MAX_STRING_CHARS);
		return NULL;
	}

	count = 0;

	for (i = 0; i < len; i++)
	{
		if (scan[i] == '"')
			inquote ^= 1;
		if (inquote)
			continue;	// don't expand inside quotes
		if (scan[i] != '$')
			continue;
		// scan out the complete macro
		start = scan + i + 1;
		token = COM_Parse(&start);
		if (!start)
			continue;

		token = Cvar_VariableString(token);

		j = static_cast<int>(strlen(token));
		len += j;
		if (len >= MAX_STRING_CHARS)
		{
			Com_Printf("Expanded line exceeded %i chars, discarded.\n", MAX_STRING_CHARS);
			return NULL;
		}

		strncpy(temporary, scan, i);
		strcpy(temporary + i, token);
		strcpy(temporary + i + j, start);

		strcpy(expanded, temporary);
		scan = expanded;
		i--;

		if (++count == 100)
		{
			Com_Printf("Macro expansion loop, discarded.\n");
			return NULL;
		}
	}

	if (inquote)
	{
		Com_Printf("Line has unmatched quote, discarded.\n");
		return NULL;
	}

	return scan;
}


/*
============
Cmd_TokenizeString

Parses the given string into command line tokens.
$Cvars will be expanded unless they are in a quoted token
============
*/
void Cmd_TokenizeString(char* text, bool macroExpand)
{
	int		i;
	const char* com_token;

	// clear the args from the last string
	for (i = 0; i < cmd_argc; i++)
		Z_Free(cmd_argv[i]);

	cmd_argc = 0;
	cmd_args[0] = 0;

	// macro expand the text
	if (macroExpand)
		text = Cmd_MacroExpandString(text);
	if (!text)
		return;

	while (1)
	{
		// skip whitespace up to a /n
		while (*text && *text <= ' ' && *text != '\n')
		{
			text++;
		}

		if (*text == '\n')
		{	// a newline seperates commands in the buffer
			text++;
			break;
		}

		if (!*text)
			return;

		// set cmd_args to everything after the first arg
		if (cmd_argc == 1)
		{
			int		l;

			strcpy(cmd_args, text);

			// strip off any trailing whitespace
			l = static_cast<int>(strlen(cmd_args)) - 1;
			for (; l >= 0; l--)
				if (cmd_args[l] <= ' ')
					cmd_args[l] = 0;
				else
					break;
		}

		com_token = COM_Parse(&text);
		if (!text)
			return;

		if (cmd_argc < MAX_STRING_TOKENS)
		{
			cmd_argv[cmd_argc] = (char*)Z_Malloc(static_cast<int>(strlen(com_token)) + 1);
			strcpy(cmd_argv[cmd_argc], com_token);
			cmd_argc++;
		}
	}
}


/*
============
Cmd_AddCommand
============
*/
void Cmd_AddCommand(const char* cmd_name, xcommand_t function)
{
	cmd_function_t* cmd;

	// fail if the command is a variable name
	if (Cvar_VariableString(cmd_name)[0])
	{
		Com_Printf("Cmd_AddCommand: %s already defined as a var\n", cmd_name);
		return;
	}

	// fail if the command already exists
	for (cmd = cmd_functions; cmd; cmd = cmd->next)
	{
		if (!strcmp(cmd_name, cmd->name))
		{
			Com_Printf("Cmd_AddCommand: %s already defined\n", cmd_name);
			return;
		}
	}

	cmd = (cmd_function_t*)Z_Malloc(sizeof(cmd_function_t));
	cmd->name = (char*)cmd_name;
	cmd->function = function;
	cmd->next = cmd_functions;
	cmd_functions = cmd;
}

/*
============
Cmd_RemoveCommand
============
*/
void Cmd_RemoveCommand(char* cmd_name)
{
	cmd_function_t* cmd, ** back;

	back = &cmd_functions;
	while (1)
	{
		cmd = *back;
		if (!cmd)
		{
			Com_Printf("Cmd_RemoveCommand: %s not added\n", cmd_name);
			return;
		}
		if (!strcmp(cmd_name, cmd->name))
		{
			*back = cmd->next;
			Z_Free(cmd);
			return;
		}
		back = &cmd->next;
	}
}

/*
============
Cmd_Exists
============
*/
bool Cmd_Exists(char* cmd_name)
{
	cmd_function_t* cmd;

	for (cmd = cmd_functions; cmd; cmd = cmd->next)
	{
		if (!strcmp(cmd_name, cmd->name))
			return true;
	}

	return false;
}



/*
============
Cmd_CompleteCommand
============
*/
char* Cmd_CompleteCommand(char* partial)
{
	cmd_function_t* cmd;
	int				len;
	cmdalias_t* a;

	len = static_cast<int>(strlen(partial));

	if (!len)
		return NULL;

	// check for exact match
	for (cmd = cmd_functions; cmd; cmd = cmd->next)
		if (!strcmp(partial, cmd->name))
			return cmd->name;
	for (a = cmd_alias; a; a = a->next)
		if (!strcmp(partial, a->name))
			return a->name;

	// check for partial match
	for (cmd = cmd_functions; cmd; cmd = cmd->next)
		if (!strncmp(partial, cmd->name, len))
			return cmd->name;
	for (a = cmd_alias; a; a = a->next)
		if (!strncmp(partial, a->name, len))
			return a->name;

	return NULL;
}


/*
============
Cmd_ExecuteString

A complete command line has been parsed, so try to execute it
FIXME: lookupnoadd the token to speed search?
============
*/
void Cmd_ExecuteString(char* text)
{
	cmd_function_t* cmd;
	cmdalias_t* a;

	Cmd_TokenizeString(text, true);

	// execute the command line
	if (!Cmd_Argc())
		return;		// no tokens

	// check functions
	for (cmd = cmd_functions; cmd; cmd = cmd->next)
	{
		if (!Q_strcasecmp(cmd_argv[0], cmd->name))
		{
			if (!cmd->function)
			{	// forward to server command
				Cmd_ExecuteString(va("cmd %s", text));
			}
			else
				cmd->function();
			return;
		}
	}

	// check alias
	for (a = cmd_alias; a; a = a->next)
	{
		if (!Q_strcasecmp(cmd_argv[0], a->name))
		{
			if (++alias_count == ALIAS_LOOP_COUNT)
			{
				Com_Printf("ALIAS_LOOP_COUNT\n");
				return;
			}
			Cbuf_InsertText(a->value);
			return;
		}
	}

	// check cvars
	if (Cvar_Command())
		return;

	// send it as a server command if we are connected
	Cmd_ForwardToServer();
}

/*
============
Cmd_List_f
============
*/
void Cmd_List_f(void)
{
	cmd_function_t* cmd;
	int				i;

	i = 0;
	for (cmd = cmd_functions; cmd; cmd = cmd->next, i++)
		Com_Printf("%s\n", cmd->name);
	Com_Printf("%i commands\n", i);
}

/*
============
Cmd_Init
============
*/
void Cmd_Init(void)
{
	//
	// register our commands
	//
	Cmd_AddCommand("cmdlist", Cmd_List_f);
	Cmd_AddCommand("exec", Cmd_Exec_f);
	Cmd_AddCommand("echo", Cmd_Echo_f);
	Cmd_AddCommand("alias", Cmd_Alias_f);
	Cmd_AddCommand("wait", Cmd_Wait_f);
}