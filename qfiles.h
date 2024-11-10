#pragma once



//
// qfiles.h: quake file formats
// This file must be identical in the quake and utils directories
//

/*
========================================================================

The .pak files are just a linear collapse of a directory tree

========================================================================
*/

#define IDPAKHEADER		(('K'<<24)+('C'<<16)+('A'<<8)+'P')

typedef struct
{
	char	name[56];
	int		filepos, filelen;
} dpackfile_t;

typedef struct
{
	int		ident;		// == IDPAKHEADER
	int		dirofs;
	int		dirlen;
} dpackheader_t;

#define	MAX_FILES_IN_PACK	4096