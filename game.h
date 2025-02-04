#pragma once


// game.h -- game dll information visible to server

#define	GAME_API_VERSION	3


#define	MAX_ENT_CLUSTERS	16

typedef struct edict_s edict_t;


// 游戏中的实体定义，如玩家，飞行中的火箭，AI怪物等
struct edict_s
{
	entity_state_t	s;
	//struct gclient_s* client;
	bool		inuse;	// 实体由内存池维护，标识是否在使用中
	int			linkcount;

	// FIXME: move these fields to a server private sv_entity_t
	//link_t		area;				// linked to a division node or leaf

	int			num_clusters;		// if -1, use headnode instead
	int			clusternums[MAX_ENT_CLUSTERS];
	int			headnode;			// unused if num_clusters != -1
	int			areanum, areanum2;

	//================================

	int			svflags;			// SVF_NOCLIENT, SVF_DEADMONSTER, SVF_MONSTER, etc
	//vec3_t		mins, maxs;
	//vec3_t		absmin, absmax, size;
	//solid_t		solid;
	int			clipmask;
	edict_t* owner;

	// the game dll can add anything it wants after
	// this point in the structure
};



// 这里是引擎提供的功能
//
// functions provided by the main engine
//
typedef struct
{
	// special messages
	void	(*bprintf) (int printlevel, char* fmt, ...);
	void	(*dprintf) (const char* fmt, ...);
	void	(*cprintf) (edict_t* ent, int printlevel, char* fmt, ...);
	void	(*centerprintf) (edict_t* ent, char* fmt, ...);
	void	(*sound) (edict_t* ent, int channel, int soundindex, float volume, float attenuation, float timeofs);
	void	(*positioned_sound) (vec3_t origin, edict_t* ent, int channel, int soundinedex, float volume, float attenuation, float timeofs);

	// config strings hold all the index strings, the lightstyles,
	// and misc data like the sky definition and cdtrack.
	// All of the current configstrings are sent to clients when
	// they connect, and changes are sent to all connected clients.
	void	(*configstring) (int num, char* string);

	void	(*error) (char* fmt, ...);

	// the *index functions create configstrings and some internal server state
	int		(*modelindex) (char* name);
	int		(*soundindex) (char* name);
	int		(*imageindex) (char* name);

	void	(*setmodel) (edict_t* ent, char* name);

	// collision detection
	//trace_t(*trace) (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, edict_t* passent, int contentmask);
	//int		(*pointcontents) (vec3_t point);
	//qboolean(*inPVS) (vec3_t p1, vec3_t p2);
	//qboolean(*inPHS) (vec3_t p1, vec3_t p2);
	//void		(*SetAreaPortalState) (int portalnum, qboolean open);
	//qboolean(*AreasConnected) (int area1, int area2);

	// an entity will never be sent to a client or used for collision
	// if it is not passed to linkentity.  If the size, position, or
	// solidity changes, it must be relinked.
	void	(*linkentity) (edict_t* ent);
	void	(*unlinkentity) (edict_t* ent);		// call before removing an interactive edict
	int		(*BoxEdicts) (vec3_t mins, vec3_t maxs, edict_t** list, int maxcount, int areatype);
	//void	(*Pmove) (pmove_t* pmove);		// player movement code common with client prediction

	// network messaging
	//void	(*multicast) (vec3_t origin, multicast_t to);
	//void	(*unicast) (edict_t* ent, qboolean reliable);
	void	(*WriteChar) (int c);
	void	(*WriteByte) (int c);
	void	(*WriteShort) (int c);
	void	(*WriteLong) (int c);
	void	(*WriteFloat) (float f);
	void	(*WriteString) (char* s);
	void	(*WritePosition) (vec3_t pos);	// some fractional bits
	void	(*WriteDir) (vec3_t pos);		// single byte encoded, very coarse
	void	(*WriteAngle) (float f);

	// managed memory allocation
	void* (*TagMalloc) (int size, int tag);
	void	(*TagFree) (void* block);
	void	(*FreeTags) (int tag);

	// console variable interaction
	cvar_t* (*cvar) (const char* var_name, const char* value, int flags);
	cvar_t* (*cvar_set) (const char* var_name, const char* value);
	cvar_t* (*cvar_forceset) (char* var_name, char* value);

	// ClientCommand and ServerCommand parameter access
	int		(*argc) (void);
	const char* (*argv) (int n);
	char* (*args) (void);	// concatenation of all argv >= 1

	// add commands to the server console as if they were typed in
	// for map changing, etc
	void	(*AddCommandString) (const char* text);

	void	(*DebugGraph) (float value, int color);
} game_import_t;


// 这里是玩法子系统提供的功能
//
// functions exported by the game subsystem
//
typedef struct
{
	int			apiversion;

	// the init function will only be called when a game starts,
	// not each time a level is loaded.  Persistant data for clients
	// and the server can be allocated in init
	void		(*Init) (void);
	void		(*Shutdown) (void);

	// each new level entered will cause a call to SpawnEntities
	void		(*SpawnEntities) (char* mapname, char* entstring, char* spawnpoint);

	// Read/Write Game is for storing persistant cross level information
	// about the world state and the clients.
	// WriteGame is called every time a level is exited.
	// ReadGame is called on a loadgame.
	void		(*WriteGame) (char* filename, bool autosave);
	void		(*ReadGame) (char* filename);

	// ReadLevel is called after the default map information has been
	// loaded with SpawnEntities
	void		(*WriteLevel) (char* filename);
	void		(*ReadLevel) (char* filename);

	bool		(*ClientConnect) (edict_t* ent, char* userinfo);
	void		(*ClientBegin) (edict_t* ent);
	void		(*ClientUserinfoChanged) (edict_t* ent, char* userinfo);
	void		(*ClientDisconnect) (edict_t* ent);
	void		(*ClientCommand) (edict_t* ent);
	void		(*ClientThink) (edict_t* ent, usercmd_t* cmd);

	void		(*RunFrame) (void);

	// ServerCommand will be called when an "sv <command>" command is issued on the
	// server console.
	// The game can issue gi.argc() / gi.argv() commands to get the rest
	// of the parameters
	void		(*ServerCommand) (void);

	//
	// global variables shared between game and server
	//

	// The edict array is allocated in the game dll so it
	// can vary in size from one game to another.
	// 
	// The size will be fixed when ge->Init() is called
	struct edict_s* edicts;
	int			edict_size;
	int			num_edicts;		// current number, <= max_edicts
	int			max_edicts;
} game_export_t;

game_export_t* GetGameApi(game_import_t* import);