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

#ifndef __TOKEN_H__
#define __TOKEN_H__

/// Get a token
extern char* csGetToken (char** buf);
/// Get a floating-point number
extern float csGetToken_float (char** buf);
/// Get a integer number
extern int csGetToken_int (char** buf);
/// Skip a token
extern void csSkipToken (char** buf, char* tok, char* msg = NULL);

#endif // __TOKEN_H__
