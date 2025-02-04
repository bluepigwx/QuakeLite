#include "server.h"


// 客户端用来浏览服务器用的结构体
netadr_t	master_adr[MAX_MASTERS];	// address of group servers

// 用于控制服务器当前这一局可以同时连接的最大客户端数量
cvar_t* maxclients;			// FIXME: rename sv_maxclients


void SV_Init(void)
{
	SV_InitOperatorCommands();

	// 初始化默认为一个玩家的游戏
	maxclients = Cvar_Get("maxclients", "1", CVAR_SERVERINFO | CVAR_LATCH);
}


void SV_Shutdown(const char* finalmsg, bool reconnect)
{

}


void SV_Frame(int msec)
{

}