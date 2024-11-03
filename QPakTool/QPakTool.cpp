// QPakTool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "lib_pak.h"


// 命令行参数
#define MAX_ARG_NUM 128  // 最大命令行参数个数
int COM_Argc = 0;
char* COM_Argv[MAX_ARG_NUM];


typedef void (*cmdfunc) (void);

struct PakCmd
{
	char name[32];
	cmdfunc func;
};


// 打印pak文件目录结构
void Cmd_Print()
{
	if (COM_Argc != 3)
	{
		printf("invalid param, type \'h\' to get help\n");
		return;
	}

	pakfile* pak = Pak_Load(COM_Argv[2]);
	if (pak == nullptr)
	{
		printf("file %s can not open\n", COM_Argv[2]);
		return;
	}

	Pak_PrintTree(pak);
	Pak_Close(pak);
}


// 打印使用方法
void Cmd_Help()
{
	printf("QPakTool usage:\n");
	printf("h	 show usage help\n");
	printf("p $pakfilename		print out the directory of specified pakfile\n");
	printf("e $pakfilename		extract specified pakfile to a directory\n");
}


// 解包pak文件到指定目录
void Cmd_Extract()
{
	pakfile* pak = Pak_Load(COM_Argv[2]);
	if (pak == nullptr)
	{
		printf("file %s can not open\n", COM_Argv[2]);
		return;
	}

	int ret = Pak_Extract_All_File(pak, COM_Argv[3]);
	if (ret != 0)
	{
		printf("extract file[%s] to dir[%s] fail\n", COM_Argv[2], COM_Argv[3]);
	}

	Pak_Close(pak);
}


PakCmd Cmd_List[] = {
	{"p", Cmd_Print},
	{"h", Cmd_Help},
	{"e", Cmd_Extract},
};


int main(int argc, char* argv[])
{
	COM_Argc = argc;
	for (int i = 0; i < argc; ++i)
	{ 
		COM_Argv[i] = argv[i];
	}

	if (argc == 1)
	{
		Cmd_Help();
		return 0;
	}

	int Found = 0;
	int CmdCnt = sizeof(Cmd_List) / sizeof(PakCmd);
	for (int i = 0; i < CmdCnt; ++i)
	{
		PakCmd* cmd = &Cmd_List[i];
		if (strcmp(cmd->name, COM_Argv[1]) == 0)
		{
			cmd->func();
			Found = 1;

			break;
		}
	}

	if (Found == 0)
	{
		Cmd_Help();
	}

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
