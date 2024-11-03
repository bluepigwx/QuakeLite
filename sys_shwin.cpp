#include "qcommon.h"
#include <windows.h>
#include <direct.h>
#include <io.h>

/*
================
Sys_Milliseconds
================
*/
int	curtime;
int Sys_Milliseconds(void)
{
	static int		base;
	static bool	initialized = false;

	if (!initialized)
	{	// let base retain 16 bits of effectively random data
		base = ::timeGetTime() & 0xffff0000;
		initialized = true;
	}
	curtime = ::timeGetTime() - base;

	return curtime;
}

void Sys_Mkdir(char* path)
{
	_mkdir(path);
}


void Sys_Error(const char* error, ...)
{

}



//============================================

char	findbase[MAX_OSPATH];
char	findpath[MAX_OSPATH];
int		findhandle;


static bool CompareAttributes(unsigned found, unsigned musthave, unsigned canthave)
{
	if ((found & _A_RDONLY) && (canthave & SFF_RDONLY))
		return false;
	if ((found & _A_HIDDEN) && (canthave & SFF_HIDDEN))
		return false;
	if ((found & _A_SYSTEM) && (canthave & SFF_SYSTEM))
		return false;
	if ((found & _A_SUBDIR) && (canthave & SFF_SUBDIR))
		return false;
	if ((found & _A_ARCH) && (canthave & SFF_ARCH))
		return false;

	if ((musthave & SFF_RDONLY) && !(found & _A_RDONLY))
		return false;
	if ((musthave & SFF_HIDDEN) && !(found & _A_HIDDEN))
		return false;
	if ((musthave & SFF_SYSTEM) && !(found & _A_SYSTEM))
		return false;
	if ((musthave & SFF_SUBDIR) && !(found & _A_SUBDIR))
		return false;
	if ((musthave & SFF_ARCH) && !(found & _A_ARCH))
		return false;

	return true;
}

char* Sys_FindFirst(char* path, unsigned musthave, unsigned canthave)
{
	struct _finddata_t findinfo;

	if (findhandle)
		Sys_Error("Sys_BeginFind without close");
	findhandle = 0;

	COM_FilePath(path, findbase);
	findhandle = static_cast<int>(_findfirst(path, &findinfo));
	if (findhandle == -1)
		return NULL;
	if (!CompareAttributes(findinfo.attrib, musthave, canthave))
		return NULL;
	Com_sprintf(findpath, sizeof(findpath), "%s/%s", findbase, findinfo.name);
	return findpath;
}

char* Sys_FindNext(unsigned musthave, unsigned canthave)
{
	struct _finddata_t findinfo;

	if (findhandle == -1)
		return NULL;
	if (_findnext(findhandle, &findinfo) == -1)
		return NULL;
	if (!CompareAttributes(findinfo.attrib, musthave, canthave))
		return NULL;

	Com_sprintf(findpath, sizeof(findpath), "%s/%s", findbase, findinfo.name);
	return findpath;
}

void Sys_FindClose(void)
{
	if (findhandle != -1)
		_findclose(findhandle);
	findhandle = 0;
}