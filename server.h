#pragma once

#include "qcommon.h"
#include "game.h"

typedef enum {
	ss_dead,			// no map loaded
	ss_loading,			// spawning level edicts
	ss_game,			// actively running
	ss_cinematic,
	ss_demo,
	ss_pic
} server_state_t;
// some qc commands are only valid before the server has finished
// initializing (precache commands, static sounds / objects, etc)


// 服务器核心状态维护，应该是维护单局游戏生命期的数据
typedef struct
{
	server_state_t	state;			// precache commands are only valid during load
									// 当前服务器所属状态，见server_state_t描述

	bool	attractloop;		// running cinematics and demos for the local system only
	bool	loadgame;			// client begins should reuse existing entity

	unsigned	time;				// always sv.framenum * 100 msec
	int			framenum;			// 当前第几帧

	char		name[MAX_QPATH];			// map name, or cinematic name
	// 碰撞体的维护
	struct cmodel_s* models[MAX_MODELS];
	// 游戏的各类配置
	char		configstrings[MAX_CONFIGSTRINGS][MAX_QPATH];
	// 用于网络同步的实体状态数组
	entity_state_t	baselines[MAX_EDICTS];

	// the multicast buffer is used to send a message to a set of clients
	// it is only used to marshall data until SV_Multicast is called
	sizebuf_t	multicast;
	byte		multicast_buf[MAX_MSGLEN];
} server_t;


#define	LATENCY_COUNTS	16
#define	RATE_MESSAGES	10


// client_t 结构体在Quake2服务器端用于表示和管理每个连接到服务器的客户端，类似于UE的Connection？
typedef struct client_s
{
	//client_state_t	state;

	// 存储客户端的用户信息字符串，通常包含玩家的名字、团队等信息
	char			userinfo[MAX_INFO_STRING];		// name, etc	

	int				lastframe;			// for delta compression
	usercmd_t		lastcmd;			// for filling in big drops	记录上次客户端上传的用户操作输入

	int				commandMsec;		// every seconds this is reset, if user
										// commands exhaust it, assume time cheating

	int				frame_latency[LATENCY_COUNTS];
	int				ping;

	int				message_size[RATE_MESSAGES];	// used to rate drop packets
	int				rate;
	int				surpressCount;		// number of messages rate supressed

	edict_t*		edict;				// EDICT_NUM(clientnum+1)	// 这里应该指向自己对应的实体
	char			name[32];			// extracted from userinfo, high bits masked
	int				messagelevel;		// for filtering printed messages

	// The datagram is written to by sound calls, prints, temp ents, etc.
	// It can be harmlessly overflowed.
	sizebuf_t		datagram;
	byte			datagram_buf[MAX_MSGLEN];

	//client_frame_t	frames[UPDATE_BACKUP];	// updates can be delta'd from here

	byte* download;			// file being downloaded
	int				downloadsize;		// total bytes (can't use EOF because of paks)
	int				downloadcount;		// bytes sent

	int				lastmessage;		// sv.framenum when packet was last received
	int				lastconnect;

	int				challenge;			// challenge of this user, randomly generated

	netchan_t		netchan;
} client_t;
//client_t 结构体是 Quake 2 服务器端的核心数据结构之一，它全面地管理了每个客户端的状态、通信、游戏数据和安全信息。通过这个结构体，服务器能够有效地处理多个客户端的连接和交互，确保游戏的顺利进行。
//理解和维护好这个结构体对于开发高效、稳定的服务器至关重要。


// 这个结构与server_t不同在于他的生命期应该是整个服务器的生命期而非单局的，因此他这里维护的数据应该是超过单局生命期的数据
typedef struct
{
	bool		initialized;				// sv_init has completed
	int			realtime;					// always increasing, no clamping, etc

	char		mapcmd[MAX_TOKEN_CHARS];	// ie: *intro.cin+base	// 这里记录的是将要跳转的地图

	int			spawncount;					// incremented each server start
											// used to check late spawns

	client_t* clients;					// [maxclients->value];	有哪些客户端连接上来了
	int			num_client_entities;		// maxclients->value*UPDATE_BACKUP*MAX_PACKET_ENTITIES
	int			next_client_entities;		// next client_entity to use
	entity_state_t* client_entities;		// [num_client_entities]	这里保存每个客户端实体的要同步的状态？

	int			last_heartbeat;

	//challenge_t	challenges[MAX_CHALLENGES];	// to prevent invalid IPs from connecting

	// serverrecord values
	FILE* demofile;
	sizebuf_t	demo_multicast;
	byte		demo_multicast_buf[MAX_MSGLEN];
} server_static_t;



extern	server_t		sv;	// 单局内服务器核心状态维护
extern	server_static_t	svs; // 全局内服务器状态维护


extern	cvar_t* maxclients;	// 当前的最大客户端连接数



// 注册服务器运维命令，例如踢人，更换地图等等能力
void SV_InitOperatorCommands(void);
