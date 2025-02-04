#include "server.h"

// 服务器主控端游戏业务逻辑交互的模块


game_export_t* ge;	// 导出的游戏子模块功能

// 初始化游戏逻辑
void SV_InitGameProgs(void)
{
	
	game_import_t	import;

	// unload anything we have now
	if (ge)
	{
		//SV_ShutdownGameProgs();
	}


	// load a new game dll
	/*
	import.multicast = SV_Multicast;
	import.unicast = PF_Unicast;
	import.bprintf = SV_BroadcastPrintf;
	import.dprintf = PF_dprintf;
	import.cprintf = PF_cprintf;
	import.centerprintf = PF_centerprintf;
	import.error = PF_error;

	import.linkentity = SV_LinkEdict;
	import.unlinkentity = SV_UnlinkEdict;
	import.BoxEdicts = SV_AreaEdicts;
	import.trace = SV_Trace;
	import.pointcontents = SV_PointContents;
	import.setmodel = PF_setmodel;
	import.inPVS = PF_inPVS;
	import.inPHS = PF_inPHS;
	import.Pmove = Pmove;

	import.modelindex = SV_ModelIndex;
	import.soundindex = SV_SoundIndex;
	import.imageindex = SV_ImageIndex;

	import.configstring = PF_Configstring;
	import.sound = PF_StartSound;
	import.positioned_sound = SV_StartSound;

	import.WriteChar = PF_WriteChar;
	import.WriteByte = PF_WriteByte;
	import.WriteShort = PF_WriteShort;
	import.WriteLong = PF_WriteLong;
	import.WriteFloat = PF_WriteFloat;
	import.WriteString = PF_WriteString;
	import.WritePosition = PF_WritePos;
	import.WriteDir = PF_WriteDir;
	import.WriteAngle = PF_WriteAngle;
	*/

	import.TagMalloc = Z_TagMalloc;
	import.TagFree = Z_Free;
	import.FreeTags = Z_FreeTags;

	import.cvar = Cvar_Get;
	import.cvar_set = Cvar_Set;
	import.cvar_forceset = Cvar_ForceSet;

	import.argc = Cmd_Argc;
	import.argv = Cmd_Argv;
	import.args = Cmd_Args;
	import.AddCommandString = Cbuf_AddText;

	//import.DebugGraph = SCR_DebugGraph;
	//import.SetAreaPortalState = CM_SetAreaPortalState;
	//import.AreasConnected = CM_AreasConnected;

	ge = (game_export_t*)GetGameAPI(&import);

	if (!ge)
		Com_Error(ERR_DROP, "failed to load game DLL");
	if (ge->apiversion != GAME_API_VERSION)
		Com_Error(ERR_DROP, "game is version %i, not %i", ge->apiversion,
			GAME_API_VERSION);

	ge->Init();
}