﻿#include "qcommon.h"


/*
* 如果是dedicate模式那么默认就是控制台程序，忽略alloc_console的值
* 如果是客户端模式，根据alloc_console的值判断是否分配控制台
*/
cvar_t* dedicated = nullptr;		// 是否是专属服务器的配置
cvar_t* alloc_console = nullptr;	// 是否分配控制台
cvar_t* developer = nullptr;		// debug开关


// 日志相关
cvar_t* logfile_active = nullptr;	// 打印日志的配置 1 = buffer log, 2 = flush after each print
FILE* logfile = nullptr;	// 日志文件句柄

cvar_t* frame_delta = nullptr;	// 每帧固定间隔

#define	MAXPRINTMSG	4096
#define MAX_NUM_ARGVS	50


#pragma region Zone Memory
/*
==============================================================================

						ZONE MEMORY ALLOCATION

just cleared malloc with counters now...

==============================================================================
*/

#define	Z_MAGIC		0x1d1d

typedef struct zhead_s
{
	struct zhead_s* prev, * next;
	short	magic;
	short	tag;			// for group free
	int		size;
} zhead_t;

zhead_t z_chain;

// 记录全局的内存分配情况
int z_count, z_bytes;

/*
========================
Z_Free
========================
*/
void Z_Free(void* ptr)
{
	zhead_t* z;

	z = ((zhead_t*)ptr) - 1;

	if (z->magic != Z_MAGIC)
		Com_Error(ERR_FATAL, "Z_Free: bad magic");

	z->prev->next = z->next;
	z->next->prev = z->prev;

	z_count--;
	z_bytes -= z->size;
	free(z);
}


/*
========================
Z_Stats_f
========================
*/
void Z_Stats_f(void)
{
	Com_Printf("%i bytes in %i blocks\n", z_bytes, z_count);
}

/*
========================
Z_FreeTags
========================
*/
void Z_FreeTags(int tag)
{
	zhead_t* z, * next;

	for (z = z_chain.next; z != &z_chain; z = next)
	{
		next = z->next;
		if (z->tag == tag)
			Z_Free((void*)(z + 1));
	}
}

/*
========================
Z_TagMalloc
========================
*/
void* Z_TagMalloc(int size, int tag)
{
	zhead_t* z;

	size = size + sizeof(zhead_t);
	z = (zhead_t*)malloc(size);
	if (!z)
		Com_Error(ERR_FATAL, "Z_Malloc: failed on allocation of %i bytes", size);

	memset(z, 0, size);
	z_count++;
	z_bytes += size;
	z->magic = Z_MAGIC;
	z->tag = tag;
	z->size = size;

	z->next = z_chain.next;
	z->prev = &z_chain;
	z_chain.next->prev = z;
	z_chain.next = z;

	return (void*)(z + 1);
}

/*
========================
Z_Malloc
========================
*/
void* Z_Malloc(int size)
{
	return Z_TagMalloc(size, 0);
}

//===========================================================================
#pragma endregion




#pragma region size buffer
//===========================================================================

void SZ_Init(sizebuf_t* buf, unsigned char* data, int length)
{
	memset(buf, 0, sizeof(*buf));
	buf->data = data;
	buf->maxsize = length;
}

void SZ_Clear(sizebuf_t* buf)
{
	buf->cursize = 0;
	buf->overflowed = false;
}

// 从sizebuf分配出length指定的内存
void* SZ_GetSpace(sizebuf_t* buf, int length)
{
	void* data;

	if (buf->cursize + length > buf->maxsize)
	{
		if (!buf->allowoverflow)
			Com_Error(ERR_FATAL, "SZ_GetSpace: overflow without allowoverflow set");

		if (length > buf->maxsize)
			Com_Error(ERR_FATAL, "SZ_GetSpace: %i is > full buffer size", length);

		Com_Printf("SZ_GetSpace: overflow\n");
		SZ_Clear(buf);
		buf->overflowed = true;
	}

	data = buf->data + buf->cursize;
	buf->cursize += length;

	return data;
}

void SZ_Write(sizebuf_t* buf, void* data, int length)
{
	memcpy(SZ_GetSpace(buf, length), data, length);
}

void SZ_Print(sizebuf_t* buf, char* data)
{
	int		len;

	len = static_cast<int>(strlen(data)) + 1;

	if (buf->cursize)
	{
		if (buf->data[buf->cursize - 1])
			memcpy((unsigned char*)SZ_GetSpace(buf, len), data, len); // no trailing 0
		else
			memcpy((unsigned char*)SZ_GetSpace(buf, len - 1) - 1, data, len); // write over trailing 0
	}
	else
		memcpy((unsigned char*)SZ_GetSpace(buf, len), data, len);
}

//============================================================================

#pragma endregion



#pragma region command line
//============================================================================

#define MAX_NUM_ARGVS	50

int com_argc;
const char* com_argv[MAX_NUM_ARGVS + 1];

/*
================
COM_CheckParm

Returns the position (1 to argc-1) in the program's argument list
where the given parameter apears, or 0 if not present
================
*/
int COM_CheckParm(const char* parm)
{
	int		i;

	for (i = 1; i < com_argc; i++)
	{
		if (!strcmp(parm, com_argv[i]))
			return i;
	}

	return 0;
}

int COM_Argc(void)
{
	return com_argc;
}

const char* COM_Argv(int arg)
{
	if (arg < 0 || arg >= com_argc || !com_argv[arg])
		return "";
	return com_argv[arg];
}

void COM_ClearArgv(int arg)
{
	if (arg < 0 || arg >= com_argc || !com_argv[arg])
		return;
	com_argv[arg] = "";
}


/*
================
COM_InitArgv
================
*/
void COM_InitArgv(int argc, const char** argv)
{
	int		i;

	if (argc > MAX_NUM_ARGVS)
		Com_Error(ERR_FATAL, "argc > MAX_NUM_ARGVS");

	com_argc = argc;
	for (i = 0; i < argc; i++)
	{
		if (!argv[i] || strlen(argv[i]) >= MAX_TOKEN_CHARS)
			com_argv[i] = "";
		else
			com_argv[i] = argv[i];
	}
}


//============================================================================
#pragma endregion




#pragma region commands
//============================================================================

/*
=============
Com_Error_f

Just throw a fatal error to
test error shutdown procedures
=============
*/
void Com_Error_f(void)
{
	Com_Error(ERR_FATAL, "%s", Cmd_Argv(1));
}

#pragma endregion
//==============================================================================




#pragma region Common
//==============================================================================

void Qcommon_Init(int argc, const char** argv)
{
	// 初始化低阶系统///////////////////////////////////////////////////
	z_chain.next = z_chain.prev = &z_chain;
	// prepare enough of the subsystems to handle
	// cvar and command buffer management
	COM_InitArgv(argc, argv); 
	
	Swap_Init();

	Cbuf_Init();
	
	Cmd_Init();
	
	Cvar_Init();

	// we need to add the early commands twice, because
	// a basedir or cddir needs to be set before execing
	// config files, but we want other parms to override
	// the settings of the config files
	// 这里其实就是将命令行里+set xx xx一类命令执行一次，但不要从命令行缓冲区内移除，
	// 因为想在执行玩default.cfg和config.cfg之后再将同名的+set xx xx命令覆盖掉来保证命令行内的设置优先级最高
	Cbuf_AddEarlyCommands(false);
	Cbuf_Execute();
	
	FS_InitFilesystem();

	Cbuf_AddText("exec default.cfg\n");
	Cbuf_AddText("exec config.cfg\n");

	Cbuf_AddEarlyCommands(true);
	Cbuf_Execute();

	//
	// init commands and vars
	//
	Cmd_AddCommand("z_stats", Z_Stats_f);
	Cmd_AddCommand("error", Com_Error_f);

#ifdef DEDICATED_ONLY
	dedicated = Cvar_Get("dedicated", "1", CVAR_NOSET);
#else
	dedicated = Cvar_Get("dedicated", "0", CVAR_NOSET);
#endif
	// 控制台分配控制
	alloc_console = Cvar_Get("alloc_console", "0", CVAR_NOSET);

	// 设置每帧固定间隔
	frame_delta = Cvar_Get("frame_delta", "16.6", CVAR_NOSET);
	// 初始化高阶系统 //////////////////////////////////
	Sys_Init();

	// 初始化网络模块
	NET_Init();

	// 初始化服务端逻辑
	SV_Init();

	// 初始化客户端逻辑
	CL_Init();
}

void Qcommon_Loop(void)
{
	int oldTime = Sys_Milliseconds();
	int deltaTime = static_cast<int>(frame_delta->value);
	int accumulator = 0;
	while (!Qcommon_Exit())
	{
		if (frame_delta->modified)
		{
			deltaTime = static_cast<int>(frame_delta->value);
			frame_delta->modified = false;
		}
		
		int newTime = Sys_Milliseconds();
		int elapsed = newTime - oldTime;
		
		accumulator += elapsed;
		while (accumulator >= deltaTime)
		{
			Qcommon_Frame(deltaTime);
			accumulator -= deltaTime;
		}
	}
}

void Qcommon_Frame(int msec)
{
	const char* s = nullptr;

	while (s = Sys_ConsoleInput())
	{
		Cbuf_AddText(va("%s\n", s));
	}
	// 执行控制台指令
	Cbuf_Execute();

	// 客户端主循环
	CL_Frame(msec);

	// 服务端主循环
	SV_Frame(msec);
}


void Qcommon_Shutdown(void)
{	
	// 关闭日志文件
	if (logfile)
	{
		fclose(logfile);
	}
}


static bool Exit = false;
bool Qcommon_Exit()
{
	return Exit;
}


void Qcommon_RequestExit()
{
	Exit = true;
}

//==============================================================================
#pragma endregion




/*
=============
Com_Printf

Both client and server can use this, and it will output
to the apropriate place.
=============
*/
void Com_Printf(const char* fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);

	// also echo to debugging console
	Sys_ConsoleOutput(msg);

	// logfile
	if (logfile_active && logfile_active->value)
	{
		char	name[MAX_QPATH];

		if (!logfile)
		{
			Com_sprintf(name, sizeof(name), "%s/qconsole.log", FS_Gamedir());
			logfile = fopen(name, "w");
		}
		if (logfile)
			fprintf(logfile, "%s", msg);
		if (logfile_active->value > 1)
			fflush(logfile);		// force it to save every time
	}
}


/*
================
Com_DPrintf

A Com_Printf that only shows up if the "developer" cvar is set
================
*/
void Com_DPrintf(const char* fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	if (!developer || !developer->value)
		return;			// don't confuse non-developers with techie stuff...

	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);

	Com_Printf("%s", msg);
}


/*
=============
Com_Error

Both client and server can use this, and it will
do the apropriate things.
=============
*/
void Com_Error(int code, const char* fmt, ...)
{

}


int server_state;
/*
==================
Com_ServerState
==================
*/
int Com_ServerState(void)
{
	return server_state;
}

void Cmd_ForwardToServer()
{

}

//============================================================================


//============================================================================
// misc
char* CopyString(const char* in)
{
	char* out;

	out = (char*)Z_Malloc(static_cast<int>(strlen(in)) + 1);
	strcpy(out, in);
	return out;
}