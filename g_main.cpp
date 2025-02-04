#include "g_local.h"


game_import_t	gi;			// 引擎模块导入到游戏子系统的功能
game_export_t	globals;	// 游戏子系统导出的功能
game_locals_t	game;	// 游戏的全局信息

edict_t* g_edicts;	// 游戏中实体对象池


cvar_t* g_maxclients;	// 最大容纳的客户端数目
cvar_t* deathmatch;
cvar_t* coop;
cvar_t* dmflags;
cvar_t* skill;
cvar_t* fraglimit;
cvar_t* timelimit;
cvar_t* password;
cvar_t* spectator_password;
cvar_t* needpass;
cvar_t* maxspectators;
cvar_t* maxentities;
cvar_t* g_select_empty;
cvar_t* g_dedicated;


// 导出函数的前置声明
void InitGame(void);


/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/
game_export_t* GetGameAPI(game_import_t* import)
{
	gi = *import;

	globals.apiversion = GAME_API_VERSION;
	globals.Init = InitGame;
	//globals.Shutdown = ShutdownGame;
	//globals.SpawnEntities = SpawnEntities;

	//globals.WriteGame = WriteGame;
	//globals.ReadGame = ReadGame;
	//globals.WriteLevel = WriteLevel;
	//globals.ReadLevel = ReadLevel;

	//globals.ClientThink = ClientThink;
	//globals.ClientConnect = ClientConnect;
	//globals.ClientUserinfoChanged = ClientUserinfoChanged;
	//globals.ClientDisconnect = ClientDisconnect;
	//globals.ClientBegin = ClientBegin;
	//globals.ClientCommand = ClientCommand;

	//globals.RunFrame = G_RunFrame;

	//globals.ServerCommand = ServerCommand;

	globals.edict_size = sizeof(edict_t);

	return &globals;
}