#include "g_local.h"


game_import_t	gi;			// ����ģ�鵼�뵽��Ϸ��ϵͳ�Ĺ���
game_export_t	globals;	// ��Ϸ��ϵͳ�����Ĺ���
game_locals_t	game;	// ��Ϸ��ȫ����Ϣ

edict_t* g_edicts;	// ��Ϸ��ʵ������


cvar_t* g_maxclients;	// ������ɵĿͻ�����Ŀ
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


// ����������ǰ������
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