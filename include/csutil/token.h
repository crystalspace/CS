/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>

extern char* spaces;
extern char* get_token (char** buf);
extern float get_token_float (char** buf);
extern int get_token_int (char** buf);
#ifdef COMP_WCC
extern void skip_token (char** buf, char* tok);
extern void skip_token (char** buf, char* tok, char* msg);
#else
extern void skip_token (char** buf, char* tok, char* msg = NULL);
#endif

#endif /*TOKEN_H*/
