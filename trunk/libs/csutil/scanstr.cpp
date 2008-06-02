/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein

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


#include "cssysdef.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "csutil/scanstr.h"

#define CS_WHITE " \t\n\r\f"

int csScanStr (const char* in, const char* format, ...)
{
  va_list arg;
  va_start (arg, format);

  int num = 0;
  const char* orig_in = in;
  in += strspn (in, CS_WHITE);

  char c[2] = { '\0', '\0' };
  while (*format)
  {
    c[0] = *format;
    if (c[0] == '%')
    {
      format++;
      switch (*format)
      {
        case 'n':
        {
          int* a = va_arg (arg, int*);
          *a = in-orig_in;
          break;
	}
        case 'd':
	{
	  int* a = va_arg (arg, int*);
	  in += strspn (in, CS_WHITE);
	  if (*in)
	  {
	    *a = atoi (in);
	    in += strspn (in, "0123456789+-");
	    in += strspn (in, CS_WHITE);
	    num++;
	  }
	  else
	    *a = 0;
	  break;
	}
	case 'D':
	{
	  int i;
	  int* list = va_arg (arg, int*);
	  int* nr = va_arg (arg, int*);
	  in += strspn (in, CS_WHITE);
	  i = 0;
	  while ((*in >= '0' && *in <= '9') || *in == '+' || *in == '-')
	  {
	    list[i++] = atoi (in);
	    in += strspn (in, "0123456789+-");
	    in += strspn (in, CS_WHITE);
	    if (*in != ',') break;
	    in++;
	    in += strspn (in, CS_WHITE);
	  }
	  *nr = i;
	  num++;
	  break;
	}
        case 'b':
	{
	  bool* a = va_arg (arg, bool*);
	  in += strspn (in, CS_WHITE);
	  if (*in)
	  {
	    const char* in2 = in + strspn (in,
	      "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
	    int l = (int)(in2-in);
	    *a = !strncasecmp (in, "yes", l) || !strncasecmp (in, "true", l) ||
	         !strncasecmp (in, "on", l)  || !strncasecmp (in, "1", l);
	    in = in2 + strspn (in2, CS_WHITE);
	    num++;
	  }
	  else
	    *a = false;
	  break;
	}
	case 'f':
	{
	  float* a = va_arg (arg, float*);
	  in += strspn (in, CS_WHITE);
	  if (*in)
	  {
	    *a = atof (in);
	    in += strspn (in, "0123456789.eE+-");
	    in += strspn (in, CS_WHITE);
	    num++;
	  }
	  else
	    *a = 0.0f;
	  break;
	}
	case 'F':
	{
	  int i;
	  float* list = va_arg (arg, float*);
	  int* nr = va_arg (arg, int*);
	  in += strspn (in, CS_WHITE);
	  i = 0;
	  while ((*in >= '0' && *in <= '9') || *in == '.' || *in == '+' ||
	    *in == '-' || *in == 'e' || *in == 'E')
	  {
	    list[i++] = atof (in);
	    in += strspn (in, "0123456789.eE+-");
	    in += strspn (in, CS_WHITE);
	    if (*in != ',') break;
	    in++;
	    in += strspn (in, CS_WHITE);
	  }
	  *nr = i;
	  num++;
	  break;
	}
	case 's':
	{
	  char* a = va_arg (arg, char*);
	  in += strspn (in, CS_WHITE);
	  if (*in == '\'')
	  {
	    in++;
	    const char* in2 = strchr (in, '\'');
	    if (in2)
	    {
	      memcpy (a, in, (int)(in2-in));
	      a[(int)(in2-in)] = 0;
	      in = in2+1;
	    }
	    else
	    {
	      strcpy (a, in);
	      in = strchr (in, 0);
	    }
	    num++;
	  }
	  else if (*in != 0)
	  {
	    const char* in2 = in + strspn (in, "abcdefghijklmnopqrstuvwxyz"
	      "ABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789./-");
	    memcpy (a, in, (int)(in2-in));
	    a[(int)(in2-in)] = 0;
	    in = in2;
	    num++;
	  }
	  in += strspn (in, CS_WHITE);
	  break;
	}
	case 'S':
	{
	  char* a = va_arg (arg, char*);
	  in += strspn (in, CS_WHITE);
	  if (*in == '\"')
	  {
	    in++;
	    while (*in!= '\"')
	    {
	      if (*in == '\\')
	      {
		in++;
		switch (*in)
		{
		  case '\\':
		    *a++ = '\\';
		    break;
		  case 'n':
		    *a++ = '\n';
		    break;
		  case 'r':
		    *a++ = '\r';
		    break;
		  case 't':
		    *a++ = '\t';
		    break;
		  case '"':
		    *a++ = '"';
		    break;
		  default:
		    *a++ = '\\';
		    *a++ = *in;
		    break;
		} //switch
		in++;
	      }
	      else
	      {
		*a++ = *in++;
	      }
	    } //while in string
	    in++;
	    num++;
	  } //if string started

	  //terminate string
	  *a = '\0';
	  break;
	}
      }
      if (*format) format++;
    }
    else if (strpbrk(c, CS_WHITE))
    {
      format += strspn (format, CS_WHITE);
      in += strspn (in, CS_WHITE);
    }
    else if (*in == *format) { format++; in++; }
    else { num = -1; break; }
  }

  va_end (arg);
  return num;
}

