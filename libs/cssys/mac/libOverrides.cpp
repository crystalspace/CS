/*
	Copyright (C) 1999 by Jorrit Tyberghein and K. Robert Bate.
  
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.
  
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.
  
	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free
	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*----------------------------------------------------------------
	Written by K. Robert Bate 1998.

	 3/1999		-	Created.
	
----------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <ansi_files.h>
#include <buffer_io.h>
#include <critical_regions.h>
#include <file_io.h>
#include <misc_io.h>
#include <stat.h>
#include <string.h>
#ifdef __cplusplus 
	extern "C" {
#endif
#include <fsp_fopen.h>
#ifdef __cplusplus 
}
#endif
#define SYSDEF_UNLINK
#define SYSDEF_ACCESS
#define SYSDEF_MKDIR
#include "cssysdef.h"

#include <Files.h>
#include <Aliases.h>
#include <Errors.h>


/*
 *	FixupFilePath
 *
 *	Change the file path that is passed in from unix style to mac style.
 */

static void FixupFilePath( const char *source, char *dest )
{
	/*
	 *	If the path is relative, start the path with a colon.
	 */
	if ( strchr( source, '/' ) && ( source[0] != '/' ) && ( source[0] != '.' )) {
		*dest++ = ':';
	}

	/*
	 *	If the path is absolute, remove the first slash.
	 */
	if ( source[0] == '/' ) {
		++source;
	}

	while (*source)
	{
		/*
		 *	If the character is a slash replace it with colons.
		 */
		if (*source == '/')
			*dest++ = ':';
		/*
		 *	If the characters are double periods with a slash replace the
		 *	periods with a colon.  This will end up with two colons which
		 *	is the mac way of moving up the directory tree.
		 */
		else if (( *source == '.' ) && ( *(source+1) == '.') && (*(source+2) == '/' )) {
			*dest++ = ':';
			source++;
		} else
			*dest++ = *source;
		source++;
	}
	*dest = 0;
}

#ifdef __cplusplus 
	#ifdef _MSL_USING_NAMESPACE
		namespace std {
	#endif
	extern "C" {
#endif

/*
 *	fopen
 *
 *	Same as the library fopen except convert the 
 *	unix path into a mac path then open the file.
 */

FILE * fopen(const char * filename, const char * mode)
{
	FILE *			file;
	char			new_filename[257];
	FSSpec			theFileSpec;
	OSErr			theError;

	/*
	 *	Convert the path from unix to mac
	 */

	FixupFilePath( filename, &(new_filename[1]) );
	new_filename[0] = strlen( &(new_filename[1]) );
	theError = FSMakeFSSpec( 0, 0, (Byte *)new_filename, &theFileSpec );
	if (( theError != noErr ) && ( theError != fnfErr ))
		return NULL;

	/*
	 *	This code is taken from fopen in the MSL library source.
	 */

	__begin_critical_region( files_access );
	
	file = FSp_fopen( &theFileSpec, mode );
	
	__end_critical_region( files_access );
	
	return file;
}

/*
 *	fgets
 *
 *	Handle the three different line endings Mac (cr), Unix (lf), and DOS (crlf)
 *	when getting a line from a file.
 */

char * fgets(char * s, int n, FILE * file)
{
	char *	p = s;
	int			c;
	
	if (--n < 0)
		return(NULL);
	
	if (n)
		do
		{
			c = getc(file);
			
			if (c == EOF)
				if (file->state.eof && p != s)
					break;
				else
					return(NULL);

			if (c == '\r') {
				c = getc(file);
				ungetc( c, file );
				if ( c == '\n' ) {
					continue;
				} else {
					c = '\n';
				}
				
			}

			*p++ = c;
		}
		while (c != '\n' && --n);
	
	*p = 0;
	
	return(s);
}

#ifdef __cplusplus
	}
	#ifdef _MSL_USING_NAMESPACE
		}
	#endif
#endif

#ifdef __cplusplus 
	extern "C" {
#endif

/*------------------------------------------------------------------------------
	Get state information on a file
------------------------------------------------------------------------------*/
#if __MSL__	>=0x6000
int _access( const char *path, int /* mode */ )
#else
int access( const char *path, int /* mode */ )
#endif
{
	Byte			new_path[ FILENAME_MAX + 1 ];
	HFileInfo		fpb;
	OSErr			err;

	/* Convert the path from unix to mac */
	FixupFilePath( path, (char *)&new_path[1] );
	/* convert the path string to pascal style */
	new_path[0] = strlen( (char *)&new_path[1] );

	fpb.ioNamePtr = new_path;
	fpb.ioFDirIndex = 0;
	fpb.ioVRefNum = 0;
	fpb.ioDirID = 0L;

	/* get the file's catalog info */
	err = PBGetCatInfoSync((CInfoPBPtr)&fpb);

	if (err != noErr)
		errno = err;
	return (err == noErr ? 0 : -1);
}

/*
 *	int mkdir(const char *path, int mode)
 *
 *		Creates a directory. (NB: mode is ignored on the mac)
 */
#if __MSL__	>=0x6000
int _mkdir(const char *path)
#else
int mkdir(const char *path, int /* mode */)
#endif
{
	HFileParam		fpb;
	OSErr			err = -1;
	Byte			new_path[ FILENAME_MAX + 1 ];

	if (path) {
		/* convert the into a mac path */
		FixupFilePath( path, (char *)&new_path[1] );
		/* convert the c string into a pascal string */
		new_path[0] = strlen( (char *)&new_path[1] );

		fpb.ioNamePtr = new_path;
		fpb.ioVRefNum = 0;
		fpb.ioDirID = 0L;
		err = PBDirCreateSync((HParmBlkPtr)&fpb);
	}

	if (err != noErr)
		errno = err;
	return (err == noErr ? 0 : -1);
}


/*
 *	int unlink(const char *path)
 *	
 *		Unlink (i.e. delete) a file.
 */
#if __MSL__	>=0x6000
int _unlink(const char *path)
#else
int unlink(const char *path)
#endif
{
	FileParam		pb;
	OSErr			err = noErr;
	Byte			new_path[ FILENAME_MAX + 1 ];

	if ( path ) {
		/* convert the into a mac path */
		FixupFilePath( path, (char *)&new_path[1] );
		/* convert the c string into a pascal string */
		new_path[0] = strlen( (char *)&new_path[1] );

		pb.ioNamePtr = new_path;
		pb.ioVRefNum = 0;
		pb.ioFVersNum = 0;

		err = PBDeleteSync((ParmBlkPtr) &pb);

		if (err != noErr)
			errno = err;
	}

	return (err == noErr ? 0 : -1);
}

/*
 *	int rmdir(const char *path)
 *	
 *		remove (i.e. delete) a directory.
 */
#if __MSL__	>=0x6000
int _rmdir(const char *path)
#else
int rmdir(const char *path)
#endif
{
	FileParam		pb;
	OSErr			err = noErr;
	Byte			new_path[ FILENAME_MAX + 1 ];

	if ( path ) {
		/* convert the into a mac path */
		FixupFilePath( path, (char *)&new_path[1] );
		/* convert the c string into a pascal string */
		new_path[0] = strlen((char *) &new_path[1] );

		pb.ioNamePtr = new_path;
		pb.ioVRefNum = 0;
		pb.ioFVersNum = 0;

		err = PBDeleteSync((ParmBlkPtr) &pb);

		if (err != noErr)
			errno = err;
	}

	return (err == noErr ? 0 : -1);
}


/*
 *	int chdir(const char *path)
 *
 *		Changes the current working directory (actually changes lowmem globals
 *		SFSaveDisk and CurDirStore which are used by open to open a file).
 */
#if __MSL__	>=0x6000
int _chdir(const char *path)
#else
int chdir(const char *path)
#endif
{
	WDPBRec			wdpb;
	Byte			new_path[ FILENAME_MAX + 1 ];
	OSErr			err = -1;

	if (path) {
		/* convert the into a mac path */
		FixupFilePath( path, (char *)&new_path[1] );
		/* convert the c string into a pascal string */
		new_path[0] = strlen( (char *)&new_path[1] );
	
		if (new_path[new_path[0]] != ':')
			new_path[++new_path[0]] = ':';

		wdpb.ioNamePtr = new_path;
		wdpb.ioVRefNum = 0;
		wdpb.ioWDDirID = 0;
		err = PBHSetVolSync(&wdpb);
	}

	/* if we reach here we have an error */
	if (err != noErr)
		errno = err;
	return (err == noErr ? 0 : -1);
}

/* local globals */
static OSErr error;


/*
 *	static long getdirname(Str255 s, short vrefnum, long dirnum)
 *
 *		Returns the current directory's name and it's parent's DirID.
 */
static long getdirname(Str255 s, short vrefnum, long dirnum)
{
	CInfoPBRec pb;

	pb.dirInfo.ioNamePtr = s;
	pb.dirInfo.ioVRefNum = vrefnum;
	pb.dirInfo.ioFDirIndex = -1;
	pb.dirInfo.ioDrDirID = dirnum;

	PBGetCatInfoSync(&pb);
	return(pb.dirInfo.ioDrParID);
}

/*
 *	static short catdirname(char *buf, int size, short vrefnum, long dirnum)
 *
 *		Recursive call to return the full path to a directory.
 */
 
static void catdirname(char *buf, int size, short vrefnum, long dirnum)
{
	Str255			dirname;

	if (dirnum == 2)
		return;

	catdirname(buf, size, vrefnum, getdirname(dirname, vrefnum, dirnum));
	if (error || (buf[0] + dirname[0] + 2 > size))
		return;

	memcpy(&buf[buf[0] + 1], &dirname[1], dirname[0]);
	buf[0] = buf[0] + dirname[0];
	buf[buf[0] + 1] = '/';
	buf[0] += 1;
}


/*
 *	char *getcwd(char *buf, int size)
 *
 *		Returns the path to the current directory.
 */
 
#if __MSL__	>=0x6000
char *_getcwd(char *buf, int size)
#else
char *getcwd(char *buf, int size)
#endif
{
	short			vrefnum;
	long			dirid;
	HVolumeParam	vpb;
	WDPBRec			wdpb;
	int				i,j;

	error = -1;

	if (size > 0 && buf) {
		buf[0] = 1;
		buf[1] = '/';

		wdpb.ioNamePtr = NULL;
		error = PBHGetVolSync(&wdpb);

		vrefnum = wdpb.ioWDVRefNum;
		dirid = wdpb.ioWDDirID;

		if (error == noErr) {
			vpb.ioVolIndex = vrefnum;
			vpb.ioNamePtr = (StringPtr)buf;
			vpb.ioVRefNum = 0;
	
			error = PBHGetVInfoSync((HParmBlkPtr)&vpb);
			if (error == noErr) {
				buf[buf[0] + 1] = '/';
				buf[0] += 1;
				if (dirid != 2) {
					error = noErr;
					catdirname(buf, size, vrefnum, dirid);
				}
			}
		}
	}
	if (error == noErr) {	/* convert into a C string */
		for (i = buf[0], j = 0; j < i; j++)
			buf[j] = buf[j+1];
		buf[i] = '\0';
	}

	if (error != noErr)
		errno = error;
	return (error == noErr ? buf : NULL);
}


#ifdef __cplusplus
	}
#endif
