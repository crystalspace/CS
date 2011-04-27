/*
    Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
    
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

#include "csutil/stringconv.h"

#include <locale.h>

static float my_strtof (const char* s, const char** fail_pos)
{
  char* fp;
  float v;
#ifdef CS_HAVE_STRTOF
  v = strtof (s, &fp);
#else
  v = (float)strtod (s, &fp);
#endif
  if (fail_pos) *fail_pos = fp;
  return v;
}

typedef enum {
  G_ASCII_ALNUM  = 1 << 0,
  G_ASCII_ALPHA  = 1 << 1,
  G_ASCII_CNTRL  = 1 << 2,
  G_ASCII_DIGIT  = 1 << 3,
  G_ASCII_GRAPH  = 1 << 4,
  G_ASCII_LOWER  = 1 << 5,
  G_ASCII_PRINT  = 1 << 6,
  G_ASCII_PUNCT  = 1 << 7,
  G_ASCII_SPACE  = 1 << 8,
  G_ASCII_UPPER  = 1 << 9,
  G_ASCII_XDIGIT = 1 << 10
} GAsciiType;

static const uint16 ascii_table_data[256] = {
  0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
  0x004, 0x104, 0x104, 0x004, 0x104, 0x104, 0x004, 0x004,
  0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
  0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
  0x140, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
  0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
  0x459, 0x459, 0x459, 0x459, 0x459, 0x459, 0x459, 0x459,
  0x459, 0x459, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
  0x0d0, 0x653, 0x653, 0x653, 0x653, 0x653, 0x653, 0x253,
  0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253,
  0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253,
  0x253, 0x253, 0x253, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
  0x0d0, 0x473, 0x473, 0x473, 0x473, 0x473, 0x473, 0x073,
  0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073,
  0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073,
  0x073, 0x073, 0x073, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x004
  /* the upper 128 are all zeroes */
};


#define g_ascii_isdigit(c) \
  ((ascii_table_data[(unsigned char) (c)] & G_ASCII_DIGIT) != 0)
#define g_ascii_isspace(c) \
  ((ascii_table_data[(unsigned char) (c)] & G_ASCII_SPACE) != 0)
#define g_ascii_isxdigit(c) \
  ((ascii_table_data[(unsigned char) (c)] & G_ASCII_XDIGIT) != 0)


namespace CS
{
  namespace Utility
  {
    // This is actually g_ascii_strtod from glib.
    float strtof (const char* nptr, const char** endptr)
    {
      const char *fail_pos;
      float val;
      struct lconv *locale_data;
      const char *decimal_point;
      size_t decimal_point_len;
      const char *p, *decimal_point_pos;
      const char *end = NULL; /* Silence gcc */
      int strtod_errno;
    
      if (nptr == 0) return 0;
    
      fail_pos = NULL;
    
      locale_data = localeconv ();
      decimal_point = locale_data->decimal_point;
      decimal_point_len = strlen (decimal_point);
    
      CS_ASSERT (decimal_point_len != 0);
      
      decimal_point_pos = NULL;
      end = NULL;
    
      if (decimal_point[0] != '.' || 
	  decimal_point[1] != 0)
	{
	  p = nptr;
	  /* Skip leading space */
	  while (g_ascii_isspace (*p))
	    p++;
	  
	  /* Skip leading optional sign */
	  if (*p == '+' || *p == '-')
	    p++;
	  
	  if (p[0] == '0' && 
	      (p[1] == 'x' || p[1] == 'X'))
	    {
	      p += 2;
	      /* HEX - find the (optional) decimal point */
	      
	      while (g_ascii_isxdigit (*p))
		p++;
	      
	      if (*p == '.')
		decimal_point_pos = p++;
		  
	      while (g_ascii_isxdigit (*p))
		p++;
	      
	      if (*p == 'p' || *p == 'P')
		p++;
	      if (*p == '+' || *p == '-')
		p++;
	      while (g_ascii_isdigit (*p))
		p++;
    
	      end = p;
	    }
	  else if (g_ascii_isdigit (*p) || *p == '.')
	    {
	      while (g_ascii_isdigit (*p))
		p++;
	      
	      if (*p == '.')
		decimal_point_pos = p++;
	      
	      while (g_ascii_isdigit (*p))
		p++;
	      
	      if (*p == 'e' || *p == 'E')
		p++;
	      if (*p == '+' || *p == '-')
		p++;
	      while (g_ascii_isdigit (*p))
		p++;
    
	      end = p;
	    }
	  /* For the other cases, we need not convert the decimal point */
	}
    
      if (decimal_point_pos)
	{
	  char *copy, *c;
    
	  /* We need to convert the '.' to the locale specific decimal point */
	  copy = (char*)cs_malloc (end - nptr + 1 + decimal_point_len);
	  
	  c = copy;
	  memcpy (c, nptr, decimal_point_pos - nptr);
	  c += decimal_point_pos - nptr;
	  memcpy (c, decimal_point, decimal_point_len);
	  c += decimal_point_len;
	  memcpy (c, decimal_point_pos + 1, end - (decimal_point_pos + 1));
	  c += end - (decimal_point_pos + 1);
	  *c = 0;
    
	  errno = 0;
	  val = my_strtof (copy, &fail_pos);
	  strtod_errno = errno;
    
	  if (fail_pos)
	    {
	      if (fail_pos - copy > decimal_point_pos - nptr)
		fail_pos = (char *)nptr + (fail_pos - copy) - (decimal_point_len - 1);
	      else
		fail_pos = (char *)nptr + (fail_pos - copy);
	    }
	  
	  cs_free (copy);
	      
	}
      else if (end)
	{
	  char *copy;
	  
	  copy = (char*)cs_malloc (end - (char *)nptr + 1);
	  memcpy (copy, nptr, end - nptr);
	  *(copy + (end - (char *)nptr)) = 0;
	  
	  errno = 0;
	  val = my_strtof (copy, &fail_pos);
	  strtod_errno = errno;
    
	  if (fail_pos)
	    {
	      fail_pos = (char *)nptr + (fail_pos - copy);
	    }
	  
	  cs_free (copy);
	}
      else
	{
	  errno = 0;
	  val = my_strtof (nptr, &fail_pos);
	  strtod_errno = errno;
	}
    
      if (endptr)
	*endptr = fail_pos;
    
      errno = strtod_errno;
    
      return val;
    }

    csString ftostr (float f)
    {
      /*
       * - Thinking behind the format is pretty simple:
       *   float has 23bits of mantissa -> ~8 digits of precision.
       *   Width is not limited, so 'e' notation should be taken for smaller
       *   numbers.
       *   (If you know a better format or have a better justification,
       *   by all means improve this ;)
       * - Format() cares about the right decimal point.
       */
      return csString().Format ("%.8g", f);
    }
  } // namespace Utility
} // namespace CS
