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

#include <string.h>

#include "sysdef.h"
#include "cssys/common/csendian.h"

//---------------------------------------------------------------------------

static unsigned long test_value = 0x12345678L;
// Test for endianness.
inline int test_endian ()
{
  return (*((unsigned char*)&test_value) == 0x12);
}

float convert_endian (float l)
{
  // @@@ Is this right?
  if (test_endian ())
  {
    union
    {
      float l;
      struct { char a, b, c, d; } s;
    } swap;
    char s;

    swap.l = l;
    s = swap.s.a; swap.s.a = swap.s.d; swap.s.d = s;
    s = swap.s.b; swap.s.b = swap.s.c; swap.s.c = s;
    l = swap.l;
  }
  return l;
}

long convert_endian (long l)
{
  if (test_endian ())
  {
    union
    {
      long l;
      struct { char a, b, c, d; } s;
    } swap;
    char s;

    swap.l = l;
    s = swap.s.a; swap.s.a = swap.s.d; swap.s.d = s;
    s = swap.s.b; swap.s.b = swap.s.c; swap.s.c = s;
    l = swap.l;
  }
  return l;
}

short convert_endian (short l)
{
  if (test_endian ())
  {
    union
    {
      short l;
      struct { char a, b; } s;
    } swap;
    char s;

    swap.l = l;
    s = swap.s.a; swap.s.a = swap.s.b; swap.s.b = s;
    l = swap.l;
  }
  return l;
}

void write_endian (FILE* fp, long l)
{
  if (test_endian ())
  {
    union
    {
      long l;
      struct { char a, b, c, d; } s;
    } swap;
    char s;

    swap.l = l;
    s = swap.s.a; swap.s.a = swap.s.d; swap.s.d = s;
    s = swap.s.b; swap.s.b = swap.s.c; swap.s.c = s;
    l = swap.l;
  }
  fwrite (&l, 4, 1, fp);
}

void write_endian (FILE* fp, char* str)
{
  unsigned char l = (unsigned char)strlen (str);
  fwrite (&l, 1, 1, fp);
  fwrite (str, l+1, 1, fp);
}

long read_long_endian (FILE* fp) 
{
  long l;
  fread (&l, 4, 1, fp);
  if (test_endian ())
  {
    union
    {
      long l;
      struct { char a, b, c, d; } s;
    } swap;
    char s;

    swap.l = l;
    s = swap.s.a; swap.s.a = swap.s.d; swap.s.d = s;
    s = swap.s.b; swap.s.b = swap.s.c; swap.s.c = s;
    l = swap.l;
  }
  return l;
}

char* read_str_endian (FILE* fp)
{
  unsigned char l;
  fread (&l, 1, 1, fp);
  CHK (char* str = new char [l+1]);
  fread (str, l+1, 1, fp);
  return str;
}

//-- Alternative routines for handling endianess ----------------------------
typedef union
{
 unsigned long l;
 struct { char b1,b2,b3,b4; } b;
} long_swap;

typedef union
{
 unsigned short s;
 struct { char b1,b2; } b;
} short_swap;

static unsigned long init_big_endian_long (unsigned long value);
static unsigned long init_little_endian_long (unsigned long value);
static unsigned short init_big_endian_short (unsigned short value);
static unsigned short init_little_endian_short (unsigned short value);
static unsigned long dummy_long (unsigned long value);
static unsigned short dummy_short (unsigned short value);
static unsigned long swap_long (unsigned long value);
static unsigned short swap_short (unsigned short value);

t_big_endian_long *big_endian_long = init_big_endian_long;
t_big_endian_short *big_endian_short = init_big_endian_short;
t_little_endian_long *little_endian_long = init_little_endian_long;
t_little_endian_short *little_endian_short = init_little_endian_short;

static void endian_init()
{
 if (test_endian ())
 {
  big_endian_long = dummy_long;
  big_endian_short = dummy_short;
  little_endian_long = swap_long;
  little_endian_short = swap_short;
 } else
 {
  little_endian_long = dummy_long;
  little_endian_short = dummy_short;
  big_endian_long = swap_long;
  big_endian_short = swap_short;
 } /* endif */
}

static unsigned long init_big_endian_long (unsigned long value)
{
 endian_init ();
 return big_endian_long (value);
}

static unsigned long init_little_endian_long (unsigned long value)
{
 endian_init ();
 return little_endian_long (value);
}

static unsigned short init_big_endian_short (unsigned short value)
{
 endian_init ();
 return big_endian_short (value);
}

static unsigned short init_little_endian_short (unsigned short value)
{
 endian_init ();
 return little_endian_short (value);
}

static unsigned long dummy_long (unsigned long value)
{
 return value;
}

static unsigned short dummy_short (unsigned short value)
{
 return value;
}

static unsigned long swap_long (unsigned long value)
{
 long_swap l1 = {value};
 long_swap l2;
 l2.b.b1 = l1.b.b4;
 l2.b.b2 = l1.b.b3;
 l2.b.b3 = l1.b.b2;
 l2.b.b4 = l1.b.b1;
 return l2.l;
}

static unsigned short swap_short (unsigned short value)
{
 short_swap s1 = {value};
 short_swap s2;
 s2.b.b1 = s1.b.b2;
 s2.b.b2 = s1.b.b1;
 return s2.s;
}

//---------------------------------------------------------------------------
