#include "g_local.h"



/*
============
InitGame

This will be called when the dll is first loaded, which
only happens when a new game is started or a save game
is loaded.
============
*/
void InitGame(void)
{
	//gi.dprintf("==== InitGame ====\n");

	/*
	gun_x = gi.cvar("gun_x", "0", 0);
	gun_y = gi.cvar("gun_y", "0", 0);
	gun_z = gi.cvar("gun_z", "0", 0);

	//FIXME: sv_ prefix is wrong for these
	sv_rollspeed = gi.cvar("sv_rollspeed", "200", 0);
	sv_rollangle = gi.cvar("sv_rollangle", "2", 0);
	sv_maxvelocity = gi.cvar("sv_maxvelocity", "2000", 0);
	sv_gravity = gi.cvar("sv_gravity", "800", 0);

	// noset vars
	dedicated = gi.cvar("dedicated", "0", CVAR_NOSET);

	// latched vars
	sv_cheats = gi.cvar("cheats", "0", CVAR_SERVERINFO | CVAR_LATCH);
	*/

	//======================
	gi.cvar("gamename", GAMEVERSION, CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar("gamedate", __DATE__, CVAR_SERVERINFO | CVAR_LATCH);

	g_maxclients = gi.cvar("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	maxspectators = gi.cvar("maxspectators", "4", CVAR_SERVERINFO);
	deathmatch = gi.cvar("deathmatch", "0", CVAR_LATCH);
	coop = gi.cvar("coop", "0", CVAR_LATCH);
	skill = gi.cvar("skill", "1", CVAR_LATCH);
	maxentities = gi.cvar("maxentities", "1024", CVAR_LATCH);

	// change anytime vars
	/*
	dmflags = gi.cvar("dmflags", "0", CVAR_SERVERINFO);
	fraglimit = gi.cvar("fraglimit", "0", CVAR_SERVERINFO);
	timelimit = gi.cvar("timelimit", "0", CVAR_SERVERINFO);
	password = gi.cvar("password", "", CVAR_USERINFO);
	spectator_password = gi.cvar("spectator_password", "", CVAR_USERINFO);
	needpass = gi.cvar("needpass", "0", CVAR_SERVERINFO);
	filterban = gi.cvar("filterban", "1", 0);

	g_select_empty = gi.cvar("g_select_empty", "0", CVAR_ARCHIVE);

	run_pitch = gi.cvar("run_pitch", "0.002", 0);
	run_roll = gi.cvar("run_roll", "0.005", 0);
	bob_up = gi.cvar("bob_up", "0.005", 0);
	bob_pitch = gi.cvar("bob_pitch", "0.002", 0);
	bob_roll = gi.cvar("bob_roll", "0.002", 0);
	*/

	// flood control
	//flood_msgs = gi.cvar("flood_msgs", "4", 0);
	//flood_persecond = gi.cvar("flood_persecond", "4", 0);
	//flood_waitdelay = gi.cvar("flood_waitdelay", "10", 0);

	// dm map list
	//sv_maplist = gi.cvar("sv_maplist", "", 0);

	// items
	//InitItems();

	//Com_sprintf(game.helpmessage1, sizeof(game.helpmessage1), "");

	//Com_sprintf(game.helpmessage2, sizeof(game.helpmessage2), "");

	// initialize all entities for this game
	game.maxentities = static_cast<int>(maxentities->value);
	g_edicts = (edict_t*)gi.TagMalloc(game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;
	globals.max_edicts = game.maxentities;

	// initialize all clients for this game
	//game.maxclients = (int)maxclients->value;
	//game.clients = (gclient_t*)gi.TagMalloc(game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	globals.num_edicts = game.maxclients + 1;
}