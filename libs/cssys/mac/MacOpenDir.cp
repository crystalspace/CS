#include <Files.h>
#define SYSDEF_PATH
#define SYSDEF_DIR
#include "cssysdef.h"

struct directory_scan {
	char d_name[ CS_MAXPATHLEN ];
	short index;
	short vRefNum;
	long dirID;
	struct dirent de;
};

DIR * opendir(const char *name)
{
	Str255		theName;
	OSErr 		theError; 
	CInfoPBRec	thePB;
	struct directory_scan *theDirInfo;

	strcpy( (char *)&theName[1], name );
	theName[0] = strlen( name );
	thePB.dirInfo.ioFDirIndex = 0; 
	thePB.dirInfo.ioDrDirID  = 0L;
	thePB.dirInfo.ioVRefNum = 0;
	thePB.dirInfo.ioNamePtr = theName;

	theError = PBGetCatInfoSync( &thePB );
	
	if ( theError != noErr )
		return NULL;

	theDirInfo = (struct directory_scan *)NewPtr( sizeof( directory_scan ));
	if ( theDirInfo ) {
		strcpy( theDirInfo->d_name, name );
		theDirInfo->index = 1;
		theDirInfo->vRefNum = thePB.dirInfo.ioVRefNum;
		theDirInfo->dirID = thePB.dirInfo.ioDrDirID;
	}

	return (DIR *)theDirInfo;
}

dirent *readdir (DIR *dirp)
{
	CInfoPBRec	thePB;
	Str255		theName;
	OSErr 		theError; 

	thePB.dirInfo.ioFDirIndex = ((struct directory_scan *)dirp)->index; 
	thePB.dirInfo.ioDrDirID = ((struct directory_scan *)dirp)->dirID;
	thePB.dirInfo.ioVRefNum = ((struct directory_scan *)dirp)->vRefNum;
	thePB.dirInfo.ioNamePtr = theName;
	theName[0] = 0;

	theError = PBGetCatInfoSync( &thePB );
	if ( theError != noErr ) {
		strcpy( ((struct directory_scan *)dirp)->de.d_name, ((struct directory_scan *)dirp)->d_name );
		((struct directory_scan *)dirp)->de.d_name[ strlen( ((struct directory_scan *)dirp)->de.d_name ) + 1 ] = '\0';
		((struct directory_scan *)dirp)->de.d_name[ strlen( ((struct directory_scan *)dirp)->de.d_name ) ] = PATH_SEPARATOR;
		theName[ theName[0] + 1 ] = 0;
		strcat( ((struct directory_scan *)dirp)->de.d_name, (char *)&theName[1] );
		((struct directory_scan *)dirp)->index += 1;

		return &((struct directory_scan *)dirp)->de;
	}

	return NULL;
}


int closedir (DIR *dirp)
{
	if ( dirp )
		DisposePtr( (Ptr)dirp );

	return 0;
}
