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
#include <string.h>

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
