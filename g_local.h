#pragma once


#include "q_shared.h"
#include "game.h"



extern cvar_t* g_maxclients;	// 最大容纳的客户端数目
extern cvar_t* deathmatch;
extern cvar_t* coop;
extern cvar_t* dmflags;
extern cvar_t* skill;
extern cvar_t* fraglimit;
extern cvar_t* timelimit;
extern cvar_t* password;
extern cvar_t* spectator_password;
extern cvar_t* needpass;
extern cvar_t* maxspectators;
extern cvar_t* maxentities;
extern cvar_t* g_select_empty;
extern cvar_t* g_dedicated;


extern edict_t* g_edicts;

#define	GAMEVERSION	"baseq2"


// 这个是z_malloc删除内存的时候用的标记
#define	TAG_GAME	765		// clear when unloading the dll
#define	TAG_LEVEL	766		// clear when loading a new level



//
// this structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//
// 用于在Quake II游戏中保存游戏的全局状态信息。这个结构体在整个游戏过程中保持不变，通常在DLL加载时初始化，并在保存游戏时读写到 server.ssv 文件中
typedef struct
{
	char		helpmessage1[512];
	char		helpmessage2[512];
	int			helpchanged;	// flash F1 icon if non 0, play sound
								// and increment only if 1, 2, or 3

	//gclient_t* clients;		// [maxclients]

	// can't store spawnpoint in level, because
	// it would get overwritten by the savegame restore
	char		spawnpoint[512];	// needed for coop respawns

	// store latched cvars here that we want to get at often
	int			maxclients;
	int			maxentities;

	// cross level triggers
	int			serverflags;

	// items
	int			num_items;

	bool	autosaved;
} game_locals_t;


extern	game_import_t	gi;
extern	game_export_t	globals;
extern	game_locals_t	game;

