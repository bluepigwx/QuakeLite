#include "qcommon.h"
#include <windows.h>
#include <direct.h>
#include <io.h>


// 交互用的控制台终端
static HANDLE hinput = nullptr;
static HANDLE houtput = nullptr;


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


int Sys_Mkdir(char* path)
{
	return _mkdir(path);
}


void Sys_Error(const char* error, ...)
{
	Qcommon_RequestExit();
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

void Sys_Init()
{
	// 在windows系统下如果是专用服务器或者明确指定要分配控制台都执行控制台分配
	bool need_console = (dedicated && dedicated->value !=0) || (alloc_console && alloc_console->value != 0);
	if (need_console)
	{
		if (!::AllocConsole())
		{
			Sys_Error("AllocConsole Failed!!!!");
		}

		hinput = GetStdHandle(STD_INPUT_HANDLE);
		houtput = GetStdHandle(STD_OUTPUT_HANDLE);
	}
}


/*
================
Sys_ConsoleInput
================
*/
static char	console_text[256];
static int	console_textlen;

const char* Sys_ConsoleInput(void)
{
	INPUT_RECORD	recs[32];
	int		ch;
	DWORD numevents, numread, dummy;

	if (!alloc_console || !alloc_console->value)
		return NULL;

	for (;; )
	{
		if (!::GetNumberOfConsoleInputEvents(hinput, &numevents))
			Sys_Error("Error getting # of console events");

		if (numevents <= 0)
			break;

		if (!::ReadConsoleInput(hinput, recs, 1, &numread))
			Sys_Error("Error reading console input");

		if (numread != 1)
			Sys_Error("Couldn't read console input");

		if (recs[0].EventType == KEY_EVENT)
		{
			if (!recs[0].Event.KeyEvent.bKeyDown)
			{
				ch = recs[0].Event.KeyEvent.uChar.AsciiChar;

				switch (ch)
				{
				case '\r':
					// 只有回车才返回完整的命令
					::WriteFile(houtput, "\r\n", 2, &dummy, NULL);
					if (console_textlen)
					{
						console_text[console_textlen] = 0;
						console_textlen = 0;
						return console_text;
					}
					break;

				case '\b':
					if (console_textlen)
					{
						console_textlen--;
						::WriteFile(houtput, "\b \b", 3, &dummy, NULL);
					}
					break;

				default:
					if (ch >= ' ')
					{
						if (console_textlen < sizeof(console_text) - 2)
						{
							::WriteFile(houtput, &ch, 1, &dummy, NULL);
							console_text[console_textlen] = ch;
							console_textlen++;
						}
					}
					break;
				}
			}
		}
	}

	return NULL;
}



/*
================
Sys_ConsoleOutput

Print text to the dedicated console
================
*/
void Sys_ConsoleOutput(char* string)
{
	DWORD dummy;
	char	text[256];

	if (!alloc_console || !alloc_console->value)
		return;

	if (console_textlen)
	{
		text[0] = '\r';
		memset(&text[1], ' ', console_textlen);
		text[console_textlen + 1] = '\r';
		text[console_textlen + 2] = 0;
		WriteFile(houtput, text, console_textlen + 2, &dummy, NULL);
	}

	WriteFile(houtput, string, (DWORD)strlen(string), &dummy, NULL);

	if (console_textlen)
		WriteFile(houtput, console_text, console_textlen, &dummy, NULL);
}