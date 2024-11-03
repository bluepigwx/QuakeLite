#include "lib_pak.h"



static void FS_CreatePath(char* path)
{
	char* ofs;

	for (ofs = path + 1; *ofs; ofs++)
	{
		if (*ofs == '/')
		{	// create the directory
			*ofs = 0;
			_mkdir(path);
			*ofs = '/';
		}
	}
}


int Pak_IsValid(pakfile* pak)
{
	pakheader* header = &pak->header;

	if (header->head[0] != 'P' || header->head[1] != 'A' || header->head[2] != 'C' || header->head[3] != 'K')
		return -1;  //ident corrupt

	if (header->dir_offset < (sizeof(header->head) + 1) || header->dir_length < 1)
		return -2;  //header corrupt

	return 0;
}


void Pak_PrintTree(pakfile* pak)
{
	for (int i = 0; i < pak->section_cnt; ++i)
	{
		dirsection* section = &pak->sections[i];

		char* name = section->filename;
		printf("%25s", name);
		printf("\t\t");
		printf("%8i", section->file_length); //give out the size in bytes
		printf("  Bytes\n");
	}
}


pakfile* Pak_Load(char* filename)
{
	FILE* fp = fopen(filename, "rb");
	if (nullptr == fp)
	{
		return nullptr;
	}

	dirsection info[4096];

	pakfile* pak = (pakfile*)malloc(sizeof(pakfile));
	pak->handle = fp;

	// 读取文件头信息
	fread(&pak->header, sizeof(pakheader), 1, pak->handle);
	// 寻址到目录信息并读取到info临时buff
	fseek(fp, pak->header.dir_offset, SEEK_SET);
	fread(info, pak->header.dir_length, 1, fp);

	pak->section_cnt = pak->header.dir_length / sizeof(dirsection);
	pak->sections = (dirsection*)malloc(sizeof(dirsection) * pak->section_cnt);

	for (int i = 0; i < pak->section_cnt; ++i)
	{
		dirsection* section = &pak->sections[i];
		strncpy(section->filename, info[i].filename, sizeof(section->filename));
		section->file_length = info[i].file_length;
		section->file_position = info[i].file_position;
	}

	return pak;
}


void Pak_Close(pakfile* pak)
{
	free(pak->sections);
	fclose(pak->handle);
}


int Pak_Extract_One_File(char* filename, pakfile* pk)
{
	char full_name[256];
	for (int i = 0; i < pk->section_cnt; ++i)
	{
		dirsection* section = &pk->sections[i];
		if (!strcmp(filename, section->filename))
		{
			sprintf(full_name, "%s", filename);
			FS_CreatePath(full_name);

			fseek(pk->handle, section->file_position, SEEK_SET);
			byte* buff = (byte*)malloc(section->file_length);
			fread(buff, section->file_length, 1, pk->handle);

			FILE* out = fopen(full_name, "w+");
			if (out)
			{
				fwrite(buff, section->file_length, 1, out);
				fclose(out);
			}

			break;
		}
	}

	return 0;
}

int Pak_Extract_All_File(pakfile* pk, char* destdir)
{
	char full_name[1024];
	for (int i = 0; i < pk->section_cnt; ++i)
	{
		dirsection* section = &pk->sections[i];

		sprintf(full_name, "%s/%s", destdir, section->filename);
		FS_CreatePath(full_name);

		fseek(pk->handle, section->file_position, SEEK_SET);
		byte* buff = (byte*)malloc(section->file_length);
		fread(buff, section->file_length, 1, pk->handle);

		FILE* out = fopen(full_name, "w+");
		if (out)
		{
			fwrite(buff, section->file_length, 1, out);
			fclose(out);
		}

		free(buff);
	}

	return 0;
}
