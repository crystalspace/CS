/*
    Copyright (C) 1998 by Jorrit Tyberghein and Steve Israelson.

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

#include <MacTypes.h>
#include <Errors.h>
#include <Files.h>
#include <Processes.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cssysdef.h"

/*------------------------------------------------------------------------------
	Case insensitve strcmp
------------------------------------------------------------------------------*/
int strcasecmp (const char *str1, const char *str2)
        {
        int f,l;

        do      {
                f = tolower(*str2);
                l = tolower(*str1);
                str2++;
                str1++;
                } while (f && f == l);
        return(f - l);
        }


/*------------------------------------------------------------------------------
	Counted case insensitve strcmp
------------------------------------------------------------------------------*/
int strncasecmp(char const *dst, char const *src, int maxLen)
	{
	int f,l;

	do 	{
		f = tolower(*dst);
		l = tolower(*src);
		dst++;
		src++;
		} while (--maxLen && f && f == l);
	return(f - l);
	}


/*------------------------------------------------------------------------------
	Allocate and copy a string
------------------------------------------------------------------------------*/
char *	strdup(const char *str)
	{
	int		l;
	char *	c;

	if ( str == NULL )
		return NULL;

	l = strlen( str ) + 1;
	c = (char *)malloc( l );
	if ( c )
		strcpy( c, str );

	return c;
	}
