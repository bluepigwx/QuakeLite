#include <windows.h>
#include <winsock.h>
#include "qcommon.h"


cvar_t* net_shownet;
static cvar_t* noudp;


SOCKET ip_sockets[2];

/*
====================
NET_ErrorString
====================
*/
static const char* NET_ErrorString(void)
{
	int		code;

	code = WSAGetLastError();
	switch (code)
	{
	case WSAEINTR: return "WSAEINTR";
	case WSAEBADF: return "WSAEBADF";
	case WSAEACCES: return "WSAEACCES";
	case WSAEDISCON: return "WSAEDISCON";
	case WSAEFAULT: return "WSAEFAULT";
	case WSAEINVAL: return "WSAEINVAL";
	case WSAEMFILE: return "WSAEMFILE";
	case WSAEWOULDBLOCK: return "WSAEWOULDBLOCK";
	case WSAEINPROGRESS: return "WSAEINPROGRESS";
	case WSAEALREADY: return "WSAEALREADY";
	case WSAENOTSOCK: return "WSAENOTSOCK";
	case WSAEDESTADDRREQ: return "WSAEDESTADDRREQ";
	case WSAEMSGSIZE: return "WSAEMSGSIZE";
	case WSAEPROTOTYPE: return "WSAEPROTOTYPE";
	case WSAENOPROTOOPT: return "WSAENOPROTOOPT";
	case WSAEPROTONOSUPPORT: return "WSAEPROTONOSUPPORT";
	case WSAESOCKTNOSUPPORT: return "WSAESOCKTNOSUPPORT";
	case WSAEOPNOTSUPP: return "WSAEOPNOTSUPP";
	case WSAEPFNOSUPPORT: return "WSAEPFNOSUPPORT";
	case WSAEAFNOSUPPORT: return "WSAEAFNOSUPPORT";
	case WSAEADDRINUSE: return "WSAEADDRINUSE";
	case WSAEADDRNOTAVAIL: return "WSAEADDRNOTAVAIL";
	case WSAENETDOWN: return "WSAENETDOWN";
	case WSAENETUNREACH: return "WSAENETUNREACH";
	case WSAENETRESET: return "WSAENETRESET";
	case WSAECONNABORTED: return "WSWSAECONNABORTEDAEINTR";
	case WSAECONNRESET: return "WSAECONNRESET";
	case WSAENOBUFS: return "WSAENOBUFS";
	case WSAEISCONN: return "WSAEISCONN";
	case WSAENOTCONN: return "WSAENOTCONN";
	case WSAESHUTDOWN: return "WSAESHUTDOWN";
	case WSAETOOMANYREFS: return "WSAETOOMANYREFS";
	case WSAETIMEDOUT: return "WSAETIMEDOUT";
	case WSAECONNREFUSED: return "WSAECONNREFUSED";
	case WSAELOOP: return "WSAELOOP";
	case WSAENAMETOOLONG: return "WSAENAMETOOLONG";
	case WSAEHOSTDOWN: return "WSAEHOSTDOWN";
	case WSASYSNOTREADY: return "WSASYSNOTREADY";
	case WSAVERNOTSUPPORTED: return "WSAVERNOTSUPPORTED";
	case WSANOTINITIALISED: return "WSANOTINITIALISED";
	case WSAHOST_NOT_FOUND: return "WSAHOST_NOT_FOUND";
	case WSATRY_AGAIN: return "WSATRY_AGAIN";
	case WSANO_RECOVERY: return "WSANO_RECOVERY";
	case WSANO_DATA: return "WSANO_DATA";
	default: return "NO ERROR";
	}
}


/*
====================
NET_Init
====================
*/
void NET_Init(void)
{
	static WSADATA		winsockdata;

	WORD	wVersionRequested;
	int		r;

	wVersionRequested = MAKEWORD(1, 1);

	r = WSAStartup(MAKEWORD(1, 1), &winsockdata);
	if (r)
		Com_Error(ERR_FATAL, "Winsock initialization failed.");

	Com_Printf("Winsock Initialized\n");

	noudp = Cvar_Get("noudp", "0", CVAR_NOSET);
	net_shownet = Cvar_Get("net_shownet", "0", 0);
}


/*
====================
NET_Shutdown
====================
*/
void NET_Shutdown(void)
{
	WSACleanup();
}


bool NET_StringToSockaddr(char* s, struct sockaddr* sadr)
{
	struct hostent* h;
	char* colon;
	char	copy[128];

	memset(sadr, 0, sizeof(*sadr));

	((struct sockaddr_in*)sadr)->sin_family = AF_INET;
	((struct sockaddr_in*)sadr)->sin_port = 0;

	strcpy(copy, s);
	// 先取得端口号
	// strip off a trailing :port if present
	for (colon = copy; *colon; colon++)
		if (*colon == ':')
		{
			*colon = 0;
			((struct sockaddr_in*)sadr)->sin_port = htons((short)atoi(colon + 1));
		}

	// 再取得ipv4地址
	if (copy[0] >= '0' && copy[0] <= '9')
	{
		*(int*)&((struct sockaddr_in*)sadr)->sin_addr = inet_addr(copy);
	}
	else
	{
		if (!(h = gethostbyname(copy)))
			return 0;
		*(int*)&((struct sockaddr_in*)sadr)->sin_addr = *(int*)h->h_addr_list[0];
	}

	return true;
}


/*
====================
NET_Socket
// 创建一个套接字，同时指定绑定的端口，客户端服务端都可以为创建好的套接字指定端口
====================
*/
SOCKET NET_IPSocket(char* net_interface, int port)
{
	SOCKET newsocket;
	struct sockaddr_in	address;
	u_long _true = 1;
	int i = 1;
	int err;

	if ((newsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		err = WSAGetLastError();
		if (err != WSAEAFNOSUPPORT)
			Com_Printf("WARNING: UDP_OpenSocket: socket: %s", NET_ErrorString());
		return 0;
	}

	// make it non-blocking
	if (ioctlsocket(newsocket, FIONBIO, &_true) == -1)
	{
		Com_Printf("WARNING: UDP_OpenSocket: ioctl FIONBIO: %s\n", NET_ErrorString());
		return 0;
	}

	// make it broadcast capable
	if (setsockopt(newsocket, SOL_SOCKET, SO_BROADCAST, (char*)&i, sizeof(i)) == -1)
	{
		Com_Printf("WARNING: UDP_OpenSocket: setsockopt SO_BROADCAST: %s\n", NET_ErrorString());
		return 0;
	}

	if (!net_interface || !net_interface[0] || !_stricmp(net_interface, "localhost"))
		address.sin_addr.s_addr = INADDR_ANY;
	else
		NET_StringToSockaddr(net_interface, (struct sockaddr*)&address);

	if (port == PORT_ANY)
		address.sin_port = 0;
	else
		address.sin_port = htons((short)port);

	address.sin_family = AF_INET;

	if (bind(newsocket, (sockaddr*)&address, sizeof(address)) == -1)
	{
		Com_Printf("WARNING: UDP_OpenSocket: bind: %s\n", NET_ErrorString());
		closesocket(newsocket);
		return 0;
	}

	return newsocket;
}



/*
====================
NET_OpenIP
====================
*/
void NET_OpenIP(void)
{
	cvar_t* ip;
	int		port;
	int		dedicated;

	ip = Cvar_Get("ip", "localhost", CVAR_NOSET);

	dedicated = static_cast<int>(Cvar_VariableValue("dedicated"));

	// 服务端套接字
	if (!ip_sockets[NS_SERVER])
	{
		port = static_cast<int>(Cvar_Get("ip_hostport", "0", CVAR_NOSET)->value);
		if (!port)
		{
			port = static_cast<int>(Cvar_Get("hostport", "0", CVAR_NOSET)->value);
			if (!port)
			{
				port = static_cast<int>(Cvar_Get("port", va("%i", PORT_SERVER), CVAR_NOSET)->value);
			}
		}
		ip_sockets[NS_SERVER] = NET_IPSocket(ip->string, port);
		if (!ip_sockets[NS_SERVER] && dedicated)
			Com_Error(ERR_FATAL, "Couldn't allocate dedicated server IP port");
	}


	// dedicated servers don't need client ports
	if (dedicated)
		return;

	// 客户端套接字
	if (!ip_sockets[NS_CLIENT])
	{
		port = static_cast<int>(Cvar_Get("ip_clientport", "0", CVAR_NOSET)->value);
		if (!port)
		{
			port = static_cast<int>(Cvar_Get("clientport", va("%i", PORT_CLIENT), CVAR_NOSET)->value);
			if (!port)
				port = PORT_ANY;
		}
		ip_sockets[NS_CLIENT] = NET_IPSocket(ip->string, port);
		if (!ip_sockets[NS_CLIENT])
			ip_sockets[NS_CLIENT] = NET_IPSocket(ip->string, PORT_ANY);
	}
}


/*
====================
NET_Config

A single player game will only use the loopback code
multiplayer为true的条件是游戏开始时看游戏玩家人数是否大于1，只有1说明是本地游戏，不需要建立网络连接
====================
*/
void NET_Config(bool multiplayer)
{
	int	i;
	static	bool old_config;

	if (old_config == multiplayer)
		return;

	old_config = multiplayer;

	if (!multiplayer)
	{	// shut down any existing sockets
		// 单人游戏关闭所有socket
		for (i = 0; i < 2; i++)
		{
			if (ip_sockets[i])
			{
				closesocket(ip_sockets[i]);
				ip_sockets[i] = 0;
			}
		}
	}
	else
	{	// open sockets
		// 如果是多人游戏则配置好socket
		if (!noudp->value)
			NET_OpenIP();
	}
}