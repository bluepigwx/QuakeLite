#pragma once

#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

// ���ļ�ͷ
typedef struct pakheader_t
{
	char head[4];
	int dir_offset;
	int dir_length;
} pakheader;

// �ļ�Ŀ¼
typedef struct dirsection_t
{
	char filename[56];
	int file_position;
	int file_length;
} dirsection;

// pak�ļ���¼�ṹ
typedef struct pakfile_t
{
	FILE* handle;
	pakheader header;
	int section_cnt;
	dirsection* sections;
} pakfile;


// ����pak�ļ�Ŀ¼��
pakfile* Pak_Load(char* filename);
// �ر�һ��pak�ļ���������ͷ�Ŀ¼��
void Pak_Close(pakfile* pak);
// ��ѹһ���ļ�����
int Pak_Extract_One_File(char* filename, pakfile* pk);
// ��ѹ�����ļ���ָ��Ŀ¼
int Pak_Extract_All_File(pakfile* pak, char* destdir);
// 0 ������Ч��ʽ�����㷵�ش���
int Pak_IsValid(pakfile* pak);
// ��ӡpak�ļ�Ŀ¼��
void Pak_PrintTree(pakfile* pak);