// 服务器初始化逻辑


#include "server.h"


server_t	sv;	// 服务器核心状态维护
server_static_t	svs;



/*
======================
SV_Map

  the full syntax is:

  map [*]<map>$<startspot>+<nextserver>

command from the console or progs.
Map can also be a.cin, .pcx, or .dm2 file
Nextserver is used to allow a cinematic to play, then proceed to
another level:
	map tram.cin+jail_e3
======================
*/
void SV_Map(bool attractloop, char* levelstring, bool loadgame)
{
	char	level[MAX_QPATH];
	char* ch;
	int		l;
	char	spawnpoint[MAX_QPATH];

	sv.loadgame = loadgame;
	sv.attractloop = attractloop;

	if (sv.state == ss_dead && !sv.loadgame)
		SV_InitGame();	// the game is just starting

	strcpy(level, levelstring);

	// if there is a + in the map, set nextserver to the remainder
	ch = strstr(level, "+");
	if (ch)
	{
		*ch = 0;
		Cvar_Set("nextserver", va("gamemap \"%s\"", ch + 1));
	}
	else
		Cvar_Set("nextserver", "");

	//ZOID special hack for end game screen in coop mode
	if (Cvar_VariableValue("coop") && !Q_stricmp(level, "victory.pcx"))
		Cvar_Set("nextserver", "gamemap \"*base1\"");

	// if there is a $, use the remainder as a spawnpoint
	ch = strstr(level, "$");
	if (ch)
	{
		*ch = 0;
		strcpy(spawnpoint, ch + 1);
	}
	else
		spawnpoint[0] = 0;

	// skip the end-of-unit flag if necessary
	if (level[0] == '*')
		strcpy(level, level + 1);

	l = (int)strlen(level);
	if (l > 4 && !strcmp(level + l - 4, ".cin"))
	{
		//SCR_BeginLoadingPlaque();			// for local system
		//SV_BroadcastCommand("changing\n");
		//SV_SpawnServer(level, spawnpoint, ss_cinematic, attractloop, loadgame);
	}
	else if (l > 4 && !strcmp(level + l - 4, ".dm2"))
	{
		//SCR_BeginLoadingPlaque();			// for local system
		//SV_BroadcastCommand("changing\n");
		//SV_SpawnServer(level, spawnpoint, ss_demo, attractloop, loadgame);
	}
	else if (l > 4 && !strcmp(level + l - 4, ".pcx"))
	{
		//SCR_BeginLoadingPlaque();			// for local system
		//SV_BroadcastCommand("changing\n");
		//SV_SpawnServer(level, spawnpoint, ss_pic, attractloop, loadgame);
	}
	else
	{
		//SCR_BeginLoadingPlaque();			// for local system
		//SV_BroadcastCommand("changing\n");
		//SV_SendClientMessages();
		//SV_SpawnServer(level, spawnpoint, ss_game, attractloop, loadgame);
		Cbuf_CopyToDefer();
	}

	//SV_BroadcastCommand("reconnect\n");
}


/*
==============
SV_InitGame

A brand new game has been started
这里指新开一局
==============
*/
void SV_InitGame(void)
{
	int		i;
	edict_t* ent;
	char	idmaster[32];

	if (svs.initialized)
	{
		// cause any connected clients to reconnect
		SV_Shutdown("Server restarted\n", true);
	}
	else
	{
		// make sure the client is down
		//CL_Drop();
		//SCR_BeginLoadingPlaque();
	}

	// get any latched variable changes (maxclients, etc)
	Cvar_GetLatchedVars();

	svs.initialized = true;

	if (Cvar_VariableValue("coop") && Cvar_VariableValue("deathmatch"))
	{
		Com_Printf("Deathmatch and Coop both set, disabling Coop\n");
		Cvar_FullSet("coop", "0", CVAR_SERVERINFO | CVAR_LATCH);
	}

	// dedicated servers are can't be single player and are usually DM
	// so unless they explicity set coop, force it to deathmatch
	if (dedicated->value)
	{
		if (!Cvar_VariableValue("coop"))
			Cvar_FullSet("deathmatch", "1", CVAR_SERVERINFO | CVAR_LATCH);
	}

	// init clients
	if (Cvar_VariableValue("deathmatch"))
	{
		if (maxclients->value <= 1)
			Cvar_FullSet("maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH);
		else if (maxclients->value > MAX_CLIENTS)
			Cvar_FullSet("maxclients", va("%i", MAX_CLIENTS), CVAR_SERVERINFO | CVAR_LATCH);
	}
	else if (Cvar_VariableValue("coop"))
	{
		if (maxclients->value <= 1 || maxclients->value > 4)
			Cvar_FullSet("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
#ifdef COPYPROTECT
		if (!sv.attractloop && !dedicated->value)
			Sys_CopyProtect();
#endif
	}
	else	// non-deathmatch, non-coop is one player
	{
		Cvar_FullSet("maxclients", "1", CVAR_SERVERINFO | CVAR_LATCH);
#ifdef COPYPROTECT
		if (!sv.attractloop)
			Sys_CopyProtect();
#endif
	}

	svs.spawncount = rand();
	svs.clients = (client_t*)Z_Malloc(sizeof(client_t) * (int)maxclients->value);
	svs.num_client_entities = (int)maxclients->value * UPDATE_BACKUP * 64;
	svs.client_entities = (entity_state_t*)Z_Malloc(sizeof(entity_state_t) * svs.num_client_entities);

	// init network stuff
	NET_Config((maxclients->value > 1));

	// heartbeats will always be sent to the id master
	svs.last_heartbeat = -99999;		// send immediately
	Com_sprintf(idmaster, sizeof(idmaster), "192.246.40.37:%i", PORT_MASTER);
	NET_StringToAdr(idmaster, &master_adr[0]);

	// init game
	SV_InitGameProgs();
	for (i = 0; i < maxclients->value; i++)
	{
		ent = EDICT_NUM(i + 1);
		ent->s.number = i + 1;
		svs.clients[i].edict = ent;
		memset(&svs.clients[i].lastcmd, 0, sizeof(svs.clients[i].lastcmd));
	}
}
