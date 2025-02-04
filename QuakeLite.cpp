﻿// QuakeLite.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "qcommon.h"
#include <iostream>
#include <windows.h>


#define	MAX_NUM_ARGVS	128
int argc;
const char* argv[MAX_NUM_ARGVS];

/*
==================
ParseCommandLine

==================
*/
static void ParseCommandLine(LPSTR lpCmdLine)
{
	argc = 1;
	argv[0] = "exe";

	while (*lpCmdLine && (argc < MAX_NUM_ARGVS))
	{
		while (*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
			lpCmdLine++;

		if (*lpCmdLine)
		{
			argv[argc] = lpCmdLine;
			argc++;

			while (*lpCmdLine && ((*lpCmdLine > 32) && (*lpCmdLine <= 126)))
				lpCmdLine++;

			if (*lpCmdLine)
			{
				*lpCmdLine = 0;
				lpCmdLine++;
			}

		}
	}
}


//int main(int argc, char* argv[])
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ParseCommandLine(lpCmdLine);

    Qcommon_Init(argc, argv);

	Qcommon_Loop();

    Qcommon_Shutdown();
}
