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

#include <stdlib.h>
#include <string.h>
#include "cssysdef.h"
#include "csutil/token.h"

float csGetToken_float (char** buf)
{
  char* t = csGetToken (buf);
  if (!((*t >= '0' && *t <= '9') || *t == '-' || *t == '.'))
  {
    // Error situation. We handle this by return 0.
    return 0.;
  }
  float rc = atof (t);
  return rc;
}

int csGetToken_int (char** buf)
{
  char* t = csGetToken (buf);
  if (!((*t >= '0' && *t <= '9') || *t == '-'))
  {
    // Error situation. We handle this by return 0.
    return 0;
  }
  int rc = atoi (t);
  return rc;
}

char* csGetToken (char** buf)
{
  static char token[100];
  char* b = *buf, * t = token;
  while (true)
  {
    while (*b && (*b == ' ' || *b == '\n' || *b == 13 || *b == 10 || *b == '\t')) b++;
    if (*b == ';')
      while (*b && *b != '\n' && *b != 13 && *b != 10) b++;
    else break;
  }

  if (*b == '\'')
  {
    b++;
    while (*b && *b != '\'') { *t++ = *b++; }
    *t++ = 0;
    if (*b == '\'') b++;
  }
  else if (*b == '(' || *b == ')' || *b == '=' || *b == ',' || *b == '[' || *b == ']' ||
	*b == '%' || *b == ':' || *b == '{' || *b == '}')
  {
    *t++ = *b++;
    *t++ = 0;
  }
  else if (*b == '.' && (*(b+1) < '0' || *(b+1) > '9'))
  {
    *t++ = *b++;
    *t++ = 0;
  }
  else if ((*b >= '0' && *b <= '9') || *b == '.' || *b == '-')
  {
    while (*b && ((*b >= '0' && *b <= '9') || *b == '.' || *b == '-')) { *t++ = *b++; }
    *t++ = 0;
  }
  else if ((*b >= 'A' && *b <= 'Z') || (*b >= 'a' && *b <= 'z') || *b == '_')
  {
    while (*b && ((*b >= 'A' && *b <= 'Z') || (*b >= 'a' && *b <= 'z') || *b == '_')) { *t++ = *b++; }
    *t++ = 0;
  }
  else if (*b == 0)
  {
    *buf = b;
    return NULL;
  }
  else
  {
    // Error.
    return NULL;
  }

  *buf = b;
  return token;
}

void csSkipToken (char** buf, char* tok, char* /*msg*/)
{
  char* t = csGetToken (buf);
  if (strcmp (t, tok)) return; // Error
}
