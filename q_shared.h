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




/*
==============================================================

MATHLIB

==============================================================
*/

typedef float vec_t;
typedef vec_t vec3_t[3];
typedef vec_t vec5_t[5];

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif


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

short	BigShort(short l);
short	LittleShort(short l);
int		BigLong(int l);
int		LittleLong(int l);
float	BigFloat(float l);
float	LittleFloat(float l);
void	Swap_Init(void);
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


//
// config strings are a general means of communication from
// the server to all connected clients.
// Each config string can be at most MAX_QPATH characters.
//
// server_t结构中char configstrings[MAX_CONFIGSTRINGS][MAX_QPATH]; 每个配置项的索引定义
#define	CS_NAME				0	// 服务器名称
#define	CS_CDTRACK			1	// 背景音乐曲目
#define	CS_SKY				2	// 天空贴图
#define	CS_SKYAXIS			3		// %f %f %f format
#define	CS_SKYROTATE		4
#define	CS_STATUSBAR		5		// display program string

#define CS_AIRACCEL			29		// air acceleration control
#define	CS_MAXCLIENTS		30		// 最大客户端数量
#define	CS_MAPCHECKSUM		31		// for catching cheater maps

#define	CS_MODELS			32	// 模型起始索引
#define	CS_SOUNDS			(CS_MODELS+MAX_MODELS)	// 声音起始索引
#define	CS_IMAGES			(CS_SOUNDS+MAX_SOUNDS)	// 图像起始索引
#define	CS_LIGHTS			(CS_IMAGES+MAX_IMAGES)	// 光照起始索引
#define	CS_ITEMS			(CS_LIGHTS+MAX_LIGHTSTYLES)	// 物品起始索引
#define	CS_PLAYERSKINS		(CS_ITEMS+MAX_ITEMS)	// 玩家皮肤起始索引
#define CS_GENERAL			(CS_PLAYERSKINS+MAX_CLIENTS)	// 通用配置起始索引
#define	MAX_CONFIGSTRINGS	(CS_GENERAL+MAX_GENERAL)



//
// per-level limits
//
// 这是针对每个关卡的限制
#define	MAX_CLIENTS			256		// absolute limit 解释: 这是服务器允许的最大客户端连接数。设置为256意味着服务器最多可以同时处理256个玩家的连接。
#define	MAX_EDICTS			1024	// must change protocol to increase more 解释: edict 是 Quake 引擎中表示游戏实体的结构体。这个宏定义了地图中可以存在的最大实体数量。
									// 设置为1024是为了确保游戏性能和网络带宽的平衡。如果需要增加这个限制，可能需要修改网络协议以支持更大的数据包。
#define	MAX_LIGHTSTYLES		256
#define	MAX_MODELS			256		// these are sent over the net as bytes
									// 解释: 这是游戏中可以使用的最大模型数量。由于模型数据需要在网络上传输，并且以字节形式表示，增加这个数量会影响网络带宽和性能。
#define	MAX_SOUNDS			256		// so they cannot be blindly increased
#define	MAX_IMAGES			256
#define	MAX_ITEMS			256		// 解释: 这是游戏中可以存在的最大物品数量。物品通常指游戏中的可拾取物品或装饰性物品。
#define MAX_GENERAL			(MAX_CLIENTS*2)	// general config strings
									// 解释: 这是通用配置字符串的最大数量。通用配置字符串用于存储一些动态的游戏配置信息，如玩家的皮肤、游戏模式等。设置为 MAX_CLIENTS*2 是为了确保每个客户端都有足够的配置字符串可用。


// 用于定义游戏中的碰撞体
typedef struct cmodel_s
{
	vec3_t		mins, maxs;
	vec3_t		origin;		// for sounds or lights
	int			headnode;
} cmodel_t;



// 网络通讯定义///////////////////////////////////////////////////////////////////////
// in an update message about entities that the client will
// need to render in some way
typedef struct entity_state_s
{
	int		number;			// edict index	// 服务器实体数组中对应的实体下标

	vec3_t	origin;
	vec3_t	angles;
	vec3_t	old_origin;		// for lerpingold_origin 是一个三维向量，表示实体在前一帧的位置。
							// 用于客户端进行线性插值（lerping），使实体移动看起来更平滑。
	int		modelindex;		// modelindex 是模型的索引号，表示实体当前使用的模型
	int		modelindex2, modelindex3, modelindex4;	// weapons, CTF flags, etc
													// 这些是额外的模型索引号，用于表示实体可能使用的额外模型，如武器、CTF旗帜等
	int		frame;			// frame 表示模型动画的当前帧数
	int		skinnum;		// skinnum 是皮肤的索引号，表示实体当前使用的皮肤。
	unsigned int		effects;		// PGM - we're filling it, so it needs to be unsigned
										// effects 是一个无符号整数，用于表示实体的特效，如发光、烟雾等。
	int		renderfx;		// renderfx 表示渲染特效，如模糊、闪烁等

	int		solid;			// for client side prediction, 8*(bits 0-4) is x/y radius
							// 8*(bits 5-9) is z down distance, 8(bits10-15) is z up
							// gi.linkentity sets this properly
	int		sound;			// for looping sounds, to guarantee shutoff
	int		event;			// impulse events -- muzzle flashes, footsteps, etc
							// events only go out for a single frame, they
							// are automatically cleared each frame
} entity_state_t;


// usercmd_t is sent to the server each client frame
// 客户端每一帧上报一次的用户输入
typedef struct usercmd_s
{
	byte	msec;
	byte	buttons;
	short	angles[3];
	short	forwardmove, sidemove, upmove;
	byte	impulse;		// remove?
	byte	lightlevel;		// light level the player is standing on
} usercmd_t;