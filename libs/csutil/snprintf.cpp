/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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
#include "csutil/snprintf.h"

#include "csutil/formatter.h"

int cs_vsnprintf (char *buf, size_t len, const char *format,
		  va_list ap)
{
  csFmtDefaultWriter<utf8_char> writer ((utf8_char*)buf, len);
  csFmtDefaultReader<utf8_char> reader ((utf8_char*)format, strlen (format));
  csPrintfFormatter<csFmtDefaultWriter<utf8_char>, csFmtDefaultReader<utf8_char> >
    formatter (&reader, ap);
  formatter.Format (writer);
  return (int)(writer.GetTotal() - 1);
}

int cs_snprintf (char *buf, size_t len, const char *format,...)
{
  va_list ap;
  va_start(ap, format);
  int res = cs_vsnprintf (buf, len, format, ap);
  va_end(ap);
  return res;
}

int cs_asprintf (char** buf, const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  int res = cs_vasprintf (buf, fmt, ap);
  va_end(ap);
  return res;
}
    
int cs_vasprintf (char** buf, const char* fmt, va_list args)
{
  size_t newsize = 32, bufsize;

  *buf = 0;
  do
  {
    bufsize = newsize;
    *buf = (char*)realloc (*buf, bufsize);
    newsize = cs_vsnprintf (*buf, bufsize, fmt, args) + 1;
  }
  while (newsize >= bufsize);

  return (int)newsize;
}
