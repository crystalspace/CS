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
#include "csutil/ansicolor.h"
#include "csutil/sysfunc.h"

#include "teststrings.h"

CS_IMPLEMENT_PLATFORM_APPLICATION

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  {
    const char** qbf = quickBrownFox;
    while (*qbf != 0)
    {
      csPrintf ("%-10s %s\n", qbf[1], qbf[0]);
      qbf += 2;
    }
  }
  csPrintf ("\n");
  {
    const char** iceg = iCanEatGlass;
    while (*iceg != 0)
    {
      csPrintf ("%-10s %s\n", iceg[1], iceg[0]);
      iceg += 2;
    }
  }
  csPrintf ("\n");
  csPrintf ("Adding some " 
    CS_ANSI_FI CS_ANSI_FC "c" CS_ANSI_FY "o" CS_ANSI_FM "l" CS_ANSI_FY "o" 
    CS_ANSI_FW "r" CS_ANSI_RST 
    " for " CS_ANSI_BW CS_ANSI_FR "f" CS_ANSI_FG "u" CS_ANSI_FB "n" 
    CS_ANSI_RST "\n");
  csPrintf ("For change, a csPrintf()\n");
  csPrintfErr ("csPrintfErr()\n");
  csPrintf ("csPrintf() again\n");

  csPrintf ("\n");

  char const* nully = 0;
  csPrintf ("Formatting tests:\n");
  csPrintf (" %%%%: %%  null %%s: %s  null %%p: %p\n", nully, nully);
  static const char charStr[] = "A char string \xe2\x98\xba";
  static const wchar_t wcharStr[] = L"A wchar_t string \x263A";
  csPrintf (" %s\n", charStr);
  csPrintf (" %ls\n", wcharStr);
  char const* bogus = " (some) bogus formats: %bogus %- 0#10.2y %jd %kd\n";
  // The 123 should replace the only valid format in the string (i.e. the %jd).
  csPrintf (bogus, (intmax_t)123);

  csPrintf ("\n");
  csPrintf ("Examples shamelessly stolen from libc manual:\n");
  csPrintf ("%c%c%c%c%c", 'h', 'e', 'l', 'l', 'o');
  csPrintf ("%3s%-6s", "no", "where");
  errno = 1;
  csPrintf ("can't open `%s': %m\n", "filename");
  int nchar;
  csPrintf ("%d %s%n\n", 3, "bears", &nchar);
  csPrintf ("chars printed: %d\n", nchar);
  csPrintf ("\n");

  uint i;
  {
    const int intTest[] = {0, 1, -1, 100000};
    for (i = 0; i < sizeof (intTest) / sizeof (int); i++)
    {
      int v = intTest[i];
      csPrintf ("|%5d|%-5d|%+5d|%+-5d|% 5d|%05d|%5.0d|%5.2d|%d|\n", 
	v, v, v, v, v, v, v, v, v);
    }
    csPrintf ("\n");
  }

  {
    const uint uintTest[] = {0, 1, 100000};
    for (i = 0; i < sizeof (uintTest) / sizeof (uint); i++)
    {
      uint v = uintTest[i];
      csPrintf ("|%5u|%5o|%5x|%5X|%#5o|%#5x|%#5X|%#10.8x|\n",
	v, v, v, v, v, v, v, v);
    }
    csPrintf ("\n");
  }

  {
    const double floatTest[] = {0.0, 0.5, 1.0, -1.0, 100.0, 1000.0, 10000.0, 
      12345.0, 100000.0, 123456.0};
    for (i = 0; i < sizeof (floatTest) / sizeof (double); i++)
    {
      double v = floatTest[i];
      csPrintf ("|%13.4a|%13.4f|%13.4e|%13.4g|\n",
	v, v, v, v);
    }
    csPrintf ("\n");
  }

  {
    const long double floatTest[] = {0.0, 0.5, 1.0, -1.0, 100.0, 1000.0, 10000.0, 
      12345.0, 100000.0, 123456.0};
    for (i = 0; i < sizeof (floatTest) / sizeof (long double); i++)
    {
      long double v = floatTest[i];
      csPrintf ("|%13.4La|%13.4Lf|%13.4Le|%13.4Lg|\n",
	v, v, v, v);
    }
    csPrintf ("\n");
  }

  csPrintf ("%p\n", "testing");

  csPrintf ("%f ", 123.456); 
  csPrintf ("%0*f ", 12, 123.456);
  csPrintf ("%0.*f ", 4, 123.456);
  csPrintf ("%0*.*f ", 12, 4, 123.456);
  csPrintf ("\n");
  char const* pacifygcc = "%5$d %3$s %2$d %1$d\n";
  csPrintf (pacifygcc, 1, 2, "3", 4, 5);
  return 0;
}

