#pragma once

#include "qcommon.h"

/*
==================================================================

the client_static_t structure is persistant through an arbitrary number
of server connections

==================================================================
*/

typedef enum {
	ca_uninitialized,
	ca_disconnected, 	// not talking to a server
	ca_connecting,		// sending request packets to the server
	ca_connected,		// netchan_t established, waiting for svc_serverdata
	ca_active			// game views should be displayed
} connstate_t;


typedef struct
{
	float		connect_time;		// for connection retransmits
	connstate_t	state;

	// connection information
	char		servername[MAX_OSPATH];	// name of server from original connect

	netchan_t	netchan;
} client_static_t;

extern client_static_t	cls;

//
// the client_state_t structure is wiped completely at every
// server map change
//
typedef struct
{
	int			timeoutcount;
} client_state_t;

extern	client_state_t	cl;


