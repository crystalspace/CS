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

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "sysdef.h"
#include "csutil/scanstr.h"

int ScanStr (char* in, char* format, ...)
{
  va_list arg;
  va_start (arg, format);

  int num = 0;
  in += strspn (in, " \t\n\f");

  while (*format)
  {
    if (*format == '%')
    {
      format++;
      switch (*format)
      {
        case 'd':
	{
	  int* a = va_arg (arg, int*);
	  in += strspn (in, " \t\n\f");
	  *a = atoi (in);
	  in += strspn (in, "0123456789+- \t\n\f");
	  num++;
	  break;
	}
	case 'D':
	{
	  int i;
	  int* list = va_arg (arg, int*);
	  int* nr = va_arg (arg, int*);
	  in += strspn (in, " \t\n\f");
	  i = 0;
	  while ((*in >= '0' && *in <= '9') || *in == '+' || *in == '-')
	  {
	    list[i++] = atoi (in);
	    in += strspn (in, "0123456789+-");
	    in += strspn (in, " \t\n\f");
	    if (*in != ',') break;
	    in++;
	    in += strspn (in, " \t\n\f");
	  }
	  *nr = i;
	  num++;
	  break;
	}
        case 'b':
	{
	  int* a = va_arg (arg, int*);
	  in += strspn (in, " \t\n\f");
	  char* in2 = in + strspn (in, "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
	  int l = (int)(in2-in);
	  *a = !strncasecmp (in, "yes", l) || !strncasecmp (in, "true", l) || !strncasecmp (in, "on", l) ||
	  	!strncasecmp (in, "1", l);
	  in = in2 + strspn (in2, " \t\n\f");
	  num++;
	  break;
	}
	case 'f':
	{
	  float* a = va_arg (arg, float*);
	  in += strspn (in, " \t\n\f");
	  *a = atof (in);
	  in += strspn (in, "0123456789.eE+- \t\n\f");
	  num++;
	  break;
	}
	case 'F':
	{
	  int i;
	  float* list = va_arg (arg, float*);
	  int* nr = va_arg (arg, int*);
	  in += strspn (in, " \t\n\f");
	  i = 0;
	  while ((*in >= '0' && *in <= '9') || *in == '.' || *in == '+' || *in == '-' || *in == 'e' || *in == 'E')
	  {
	    list[i++] = atof (in);
	    in += strspn (in, "0123456789.eE+-");
	    in += strspn (in, " \t\n\f");
	    if (*in != ',') break;
	    in++;
	    in += strspn (in, " \t\n\f");
	  }
	  *nr = i;
	  num++;
	  break;
	}
	case 's':
	{
	  char* a = va_arg (arg, char*);
	  in += strspn (in, " \t\n\f");
	  if (*in == '\'')
	  {
	    in++;
	    char* in2 = strchr (in, '\'');
	    strncpy (a, in, (int)(in2-in));
	    a[(int)(in2-in)] = 0;
	    in = in2+1;
	  }
	  else
	  {
	    char* in2 = in + strspn (in, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");
	    strncpy (a, in, (int)(in2-in));
	    a[(int)(in2-in)] = 0;
	    in = in2;
	  }
	  in += strspn (in, " \t\n\f");
	  num++;
	  break;
	}
      }
      if (*format) format++;
    }
    else if (*format == ' ') { format++; in += strspn (in, " \t\n\f"); }
    else if (*in == *format) { in++; format++; }
    else { num = -1; break; }
  }

  va_end (arg);

  return num;
}
