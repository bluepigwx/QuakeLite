#pragma once

#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

// 包文件头
typedef struct pakheader_t
{
	char head[4];
	int dir_offset;
	int dir_length;
} pakheader;

// 文件目录
typedef struct dirsection_t
{
	char filename[56];
	int file_position;
	int file_length;
} dirsection;

// pak文件记录结构
typedef struct pakfile_t
{
	FILE* handle;
	pakheader header;
	int section_cnt;
	dirsection* sections;
} pakfile;


// 加载pak文件目录表
pakfile* Pak_Load(char* filename);
// 关闭一个pak文件句柄并且释放目录表
void Pak_Close(pakfile* pak);
// 解压一个文件出来
int Pak_Extract_One_File(char* filename, pakfile* pk);
// 解压所有文件到指定目录
int Pak_Extract_All_File(pakfile* pak, char* destdir);
// 0 返回有效格式，非零返回错误
int Pak_IsValid(pakfile* pak);
// 打印pak文件目录树
void Pak_PrintTree(pakfile* pak);