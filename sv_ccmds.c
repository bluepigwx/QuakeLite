//#include "server.h"



/*
==================
SV_GameMap_f

Saves the state of the map just being exited and goes to a new map.

If the initial character of the map string is '*', the next map is
in a new unit, so the current savegame directory is cleared of
map files.

Example:

*inter.cin+jail

Clears the archived maps, plays the inter.cin cinematic, then
goes to map jail.bsp.

先清理地图文件然后播放指定动画，最后加载指定地图
==================
*/

/*
void SV_GameMap_f(void)
{
	char* map;
	int			i;
	client_t* cl;
	bool* savedInuse;

	if (Cmd_Argc() != 2)
	{
		Com_Printf("USAGE: gamemap <map>\n");
		return;
	}

	Com_DPrintf("SV_GameMap(%s)\n", Cmd_Argv(1));

	//FS_CreatePath(va("%s/save/current/", FS_Gamedir()));

	// check for clearing the current savegame
	map = Cmd_Argv(1);
	if (map[0] == '*')
	{
		// wipe all the *.sav files
		//SV_WipeSavegame("current");
	}
	else
	{	// save the map just exited
		if (sv.state == ss_game)
		{
			// clear all the client inuse flags before saving so that
			// when the level is re-entered, the clients will spawn
			// at spawn points instead of occupying body shells
			savedInuse = malloc(maxclients->value * sizeof(bool));
			for (i = 0, cl = svs.clients; i < maxclients->value; i++, cl++)
			{
				savedInuse[i] = cl->edict->inuse;
				cl->edict->inuse = false;
			}

			//SV_WriteLevelFile();

			// we must restore these for clients to transfer over correctly
			for (i = 0, cl = svs.clients; i < maxclients->value; i++, cl++)
				cl->edict->inuse = savedInuse[i];
			free(savedInuse);
		}
	}

	// start up the next map
	//SV_Map(false, Cmd_Argv(1), false);

	// archive server state
	strncpy(svs.mapcmd, Cmd_Argv(1), sizeof(svs.mapcmd) - 1);

	// copy off the level to the autosave slot
	if (!dedicated->value)
	{
		//SV_WriteServerFile(true);
		//SV_CopySaveGame("current", "save0");
	}
}
*/

/*
==================
SV_Map_f

Goes directly to a given map without any savegame archiving.
For development work
==================
*/

/*
void SV_Map_f(void)
{
	char* map;
	char	expanded[MAX_QPATH];

	// if not a pcx, demo, or cinematic, check to make sure the level exists
	map = Cmd_Argv(1);
	if (!strstr(map, "."))
	{
		Com_sprintf(expanded, sizeof(expanded), "maps/%s.bsp", map);
		if (FS_LoadFile(expanded, NULL) == -1)
		{
			Com_Printf("Can't find %s\n", expanded);
			return;
		}
	}

	sv.state = ss_dead;		// don't save current level when changing
	//SV_WipeSavegame("current");
	SV_GameMap_f();
}
*/


/*
==================
SV_InitOperatorCommands
==================
*/

/*
void SV_InitOperatorCommands(void)
{

	//Cmd_AddCommand("kick", SV_Kick_f);
	//Cmd_AddCommand("status", SV_Status_f);
	//Cmd_AddCommand("serverinfo", SV_Serverinfo_f);
	Cmd_AddCommand("map", SV_Map_f);
	Cmd_AddCommand("gamemap", SV_GameMap_f);

	//if (dedicated->value)
	//	Cmd_AddCommand("say", SV_ConSay_f);

	//Cmd_AddCommand("save", SV_Savegame_f);
	//Cmd_AddCommand("load", SV_Loadgame_f);
}
*/
