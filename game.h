#pragma once


#define	MAX_ENT_CLUSTERS	16

typedef struct edict_s edict_t;


// 游戏中的实体定义，如玩家，飞行中的火箭，AI怪物等
struct edict_s
{
	//entity_state_t	s;
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