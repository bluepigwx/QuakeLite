#include "qcommon.h"
#include "client.h"

client_static_t	cls;
client_state_t	cl;


void CL_InitLocal(void);

void CL_Init(void)
{
	CL_InitLocal();
}


void CL_Shutdown(void)
{}


void CL_Frame(int msec)
{}



/*
=====================
CL_ClearState

=====================
*/
void CL_ClearState(void)
{
	// wipe the entire cl structure
	memset(&cl, 0, sizeof(cl));
	//memset(&cl_entities, 0, sizeof(cl_entities));

	SZ_Clear(&cls.netchan.message);

}




#pragma region net work
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
	char	final[32];

	if (cls.state == ca_disconnected)
		return;

	cls.connect_time = 0;

	// send a disconnect message to the server
	final[0] = clc_stringcmd;
	strcpy((char*)final + 1, "disconnect");
	Netchan_Transmit(&cls.netchan, static_cast<int>(strlen(final)), (byte*)final);
	Netchan_Transmit(&cls.netchan, static_cast<int>(strlen(final)), (byte*)final);
	Netchan_Transmit(&cls.netchan, static_cast<int>(strlen(final)), (byte*)final);

	CL_ClearState();

	cls.state = ca_disconnected;
}


/*
=======================
CL_SendConnectPacket

We have gotten a challenge from the server, so try and
connect.
======================
*/
void CL_SendConnectPacket(void)
{
	netadr_t	adr;
	int		port;

	if (!NET_StringToAdr(cls.servername, &adr))
	{
		Com_Printf("Bad server address\n");
		cls.connect_time = 0;
		return;
	}
	if (adr.port == 0)
		adr.port = BigShort(PORT_SERVER);

	port = static_cast<int>(Cvar_VariableValue("qport"));
	userinfo_modified = false;

	// 这种outofband接口就是用来发送控制类协议的
	//Netchan_OutOfBandPrint(NS_CLIENT, adr, "connect %i %i %i \"%s\"\n",
	//	PROTOCOL_VERSION, port, cls.challenge, Cvar_Userinfo());
}


/*
=================
CL_CheckForResend

Resend a connect message if the last one has timed out
=================
*/
void CL_CheckForResend(void)
{
	// if the local server is running and we aren't
	// then connect
	if (cls.state == ca_disconnected && Com_ServerState())
	{
		cls.state = ca_connecting;
		strncpy(cls.servername, "localhost", sizeof(cls.servername) - 1);
		// we don't need a challenge on the localhost
		CL_SendConnectPacket();
		return;
		//		cls.connect_time = -99999;	// CL_CheckForResend() will fire immediately
	}

	/*
	netadr_t	adr;

	// resend if we haven't gotten a reply yet
	if (cls.state != ca_connecting)
		return;

	if (cls.realtime - cls.connect_time < 3000)
		return;

	if (!NET_StringToAdr(cls.servername, &adr))
	{
		Com_Printf("Bad server address\n");
		cls.state = ca_disconnected;
		return;
	}
	if (adr.port == 0)
		adr.port = BigShort(PORT_SERVER);

	cls.connect_time = cls.realtime;	// for retransmit requests

	Com_Printf("Connecting to %s...\n", cls.servername);

	Netchan_OutOfBandPrint(NS_CLIENT, adr, "getchallenge\n");
	*/
}


/*
==================
CL_SendCommand

==================
*/
void CL_SendCommand(void)
{
	// resend a connection request if necessary
	CL_CheckForResend();
}

#pragma endregion




#pragma region commands
//=====================================
//客户端命令集


/*
================
CL_Connect_f

================
*/
void CL_Connect_f(void)
{
	const char* server;

	if (Cmd_Argc() != 2)
	{
		Com_Printf("usage: connect <server>\n");
		return;
	}

	if (Com_ServerState())
	{	// if running a local server, kill it and reissue
		//SV_Shutdown(va("Server quit\n", msg), false);
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


#pragma endregion


/*
=================
CL_InitLocal
=================
*/
void CL_InitLocal(void)
{
	cls.state = ca_disconnected;
	cls.realtime = Sys_Milliseconds();

	//
	// register our commands
	//
	Cmd_AddCommand("connect", CL_Connect_f);

}