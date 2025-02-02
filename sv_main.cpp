#include "server.h"


// 用于控制服务器当前这一局可以同时连接的最大客户端数量
cvar_t* maxclients;			// FIXME: rename sv_maxclients


void SV_Init(void)
{
	//SV_InitOperatorCommands();

	maxclients = Cvar_Get("maxclients", "1", CVAR_SERVERINFO | CVAR_LATCH);
}


void SV_Shutdown(char* finalmsg, bool reconnect)
{

}


void SV_Frame(int msec)
{

}