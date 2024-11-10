#include "qcommon.h"
#include "client.h"

client_static_t	cls;


void CL_Init(void)
{}


void CL_Shutdown(void)
{}


void CL_Frame(int msec)
{}



/*
=====================
CL_Disconnect

Goes from a connected state to full screen console state
Sends a disconnect message to the server
This is also called on Com_Error, so it shouldn't cause any errors
=====================
*/
void CL_Disconnect(void)
{
	byte	final[32];

	if (cls.state == ca_disconnected)
		return;

	cls.connect_time = 0;

	// send a disconnect message to the server
	final[0] = clc_stringcmd;
	strcpy((char*)final + 1, "disconnect");
	Netchan_Transmit(&cls.netchan, strlen(final), final);
	Netchan_Transmit(&cls.netchan, strlen(final), final);
	Netchan_Transmit(&cls.netchan, strlen(final), final);

	CL_ClearState();


	cls.state = ca_disconnected;
}



//=====================================
//客户端命令集


/*
================
CL_Connect_f

================
*/
void CL_Connect_f(void)
{
	char* server;

	if (Cmd_Argc() != 2)
	{
		Com_Printf("usage: connect <server>\n");
		return;
	}

	if (Com_ServerState())
	{	// if running a local server, kill it and reissue
		SV_Shutdown(va("Server quit\n", msg), false);
	}
	else
	{
		CL_Disconnect();
	}

	server = Cmd_Argv(1);

	NET_Config(true);		// allow remote

	CL_Disconnect();

	cls.state = ca_connecting;
	strncpy(cls.servername, server, sizeof(cls.servername) - 1);
	cls.connect_time = -99999;	// CL_CheckForResend() will fire immediately
}