/*
    Copyright (C) 2000 by W.C.A. Wijngaards
    Copyright (C) 2000 by Andrew Zabolotny

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

#define CS_SYSDEF_PROVIDE_PATH
#define CS_SYSDEF_PROVIDE_GETOPT
#include "cssysdef.h"
#include "cssys/sysdriv.h"
#include "cstool/initapp.h"
#include "ivideo/fontserv.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "isys/plugin.h"

CS_IMPLEMENT_APPLICATION

static iObjectRegistry* object_reg;

char *programversion = "0.0.1";
char *programname;

static struct option long_options[] =
{
  {"first", required_argument, 0, 'f'},
  {"glyphs", required_argument, 0, 'g'},
  {"size", required_argument, 0, 's'},
  {"output", required_argument, 0, 'o'},
  {"text", no_argument, 0, 't'},
  {"display", no_argument, 0, 'd'},
  {"help", no_argument, 0, 'h'},
  {"version", no_argument, 0, 'V'},
  {"verbose", no_argument, 0, 'v'},
  {0, no_argument, 0, 0}
};

static struct
{
  bool verbose;
  bool sourcecode;
  bool display;
  int fontsize;
  int first;
  int glyphs;
  char *output;
} opt =
{
  false,
  false,
  false,
  -1,
  0,
  256,
  NULL
};

static int lastglyph;

static int display_help ()
{
  printf ("Crystal Space font conversion/generation utility v%s\n", programversion);
  printf ("Copyright (C) 2000 by W.C.A. Wijngaards and Andrew Zabolotny\n\n");
  printf ("Usage: %s {option/s} [truetype font file] [...]\n\n", programname);
  printf ("This program allows to convert TTF font files to bitmap format CSF\n");
  printf ("which is faster to load although it is non-scalable. By default the\n");
  printf ("program will convert all the fonts given on command line to CSF.\n\n");
  printf ("  -d   --display     Display font rather than converting it\n");
  printf ("  -f#  --first=#     Start conversion at glyph # (default = 0)\n");
  printf ("  -g#  --glyphs=#    Convert just # (default = 256) glyphs of the font\n");
  printf ("  -s#  --size=#      Set font size # in points\n");
  printf ("  -o#  --output=#    Output CSF font to file #\n");
  printf ("  -t   --text        Generate text output (C++ code) rather than binary\n");
  printf ("  -h   --help        Display this help text\n");
  printf ("  -v   --verbose     Comment on what's happening\n");
  printf ("  -V   --version     Display program version\n");
  return 1;
}

static bool Display (iFontServer *fs, iFont *font)
{
  int c, l, i;
  for (c = opt.first; c < lastglyph; c++)
  {
    int w, h;
    uint8 *bitmap = font->GetGlyphBitmap (c, w, h);
    if (!bitmap || !w || !h)
      continue;

    printf ("---- Character:%d\n", c);
    for (l = 0; l < h; l++)
    {
      uint8 *line = bitmap + l * ((w + 7) / 8);
      for (i = 0; i < w; i++)
        printf ("%s", (line [i / 8] & (0x80 >> (i & 7))) ? "@" : ".");
      printf ("\n");
    }
  }
  font->DecRef ();
  fs->DecRef ();
  return true;
}

static bool Convert (const char *fontfile)
{
  if (opt.verbose)
    printf ("Loading font %s, size = %d\n", fontfile, opt.fontsize);

  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

  iFontServer *fs = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_FONTSERVER, iFontServer);
  if (!fs)
  {
    printf ("Font server plugin has not been loaded.\n");
    return false;
  }

  iFont *font = fs->LoadFont (fontfile);
  if (font == NULL)
  {
    printf ("Cannot load font file %s\n", fontfile);
    return false;
  }

  if (opt.fontsize > 0)
  {
    font->SetSize (opt.fontsize);
    int oldsize = opt.fontsize;
    opt.fontsize = font->GetSize ();
    if (opt.fontsize != oldsize)
      printf ("Could not set font size %d, using size %d\n", 
        oldsize, opt.fontsize);
  }
  else
    opt.fontsize = font->GetSize ();

  // max height of font
  int maxheight, maxwidth;
  font->GetMaxSize (maxwidth, maxheight);

  if (maxwidth > 255)
  {
    fprintf (stderr, "Font too large (%dx%d): CSF format supports only widths < 256\n", maxwidth, maxheight);
    return false;
  }

  if (opt.display)
    return Display (fs, font);

  char fontname [MAXPATHLEN + 1];
  char outfile [MAXPATHLEN + 1];
  csSplitPath (fontfile, NULL, 0, fontname, sizeof (fontname));
  if (fontname [0] == '*')
    strcpy (fontname, fontname + 1);
  char *dot = strchr (fontname, '.');
  if (dot) *dot = 0;
  sprintf (outfile, "%s%d.%s", fontname, opt.fontsize, opt.sourcecode ? "inc" : "csf");

  FILE *out = fopen (outfile, opt.sourcecode ? "w" : "wb");
  if (!out)
  {
    printf ("Could not open output file %s\n", outfile);
    return false;
  }

  int i, c, w, h;

  if (opt.sourcecode)
  {
    /// make a text version
    fprintf (out, "// %s.%d %dx%d font\n", fontname, opt.fontsize, maxwidth, maxheight);
    fprintf (out, "// FontDef: { \"%s%d\", %d, %d, %d, %d, font_%s%d, width_%s%d }\n",
      fontname, opt.fontsize, maxwidth, maxheight, opt.first, opt.glyphs,
      fontname, opt.fontsize, fontname, opt.fontsize);
    fprintf (out, "\n");
  }
  else
    fprintf (out, "CSF [Font=%s.%d Width=%d Height=%d First=%d Glyphs=%d]\n",
      fontname, opt.fontsize, maxwidth, maxheight, opt.first, opt.glyphs);

  int arrsize = 0;
  uint8 width [256];
  for (c = opt.first; c < lastglyph; c++)
  {
    uint8 *bitmap = font->GetGlyphBitmap (c, w, h);
    width [c] = (bitmap && h) ? w : 0;
    arrsize += ((width [c] + 7) / 8) * h;
  }

  // Character widths go first
  if (opt.sourcecode)
  {
    fprintf (out, "unsigned char width_%s%d [%d] =\n{\n  ",
      fontname, opt.fontsize, opt.glyphs);
    for (i = opt.first; i < lastglyph; i++)
    {
      fprintf (out, "%2d%s", width [i], (i < lastglyph - 1) ? "," : "");
      if ((i & 15) == 15)
      {
        fprintf (out, "\t// %02x..%02x\n", i - 15, i);
        if (i < lastglyph - 1)
          fprintf (out, "  ");
      }
    }
    if (opt.glyphs & 15)
      fprintf (out, "\n");
    fprintf (out, "};\n\n");
    fprintf (out, "unsigned char font_%s%d [%d] =\n{\n",
      fontname, opt.fontsize, arrsize);
  }
  else
    fwrite (width + opt.first, opt.glyphs, 1, out);

  // Output every character in turn
  for (c = opt.first; c < lastglyph; c++)
  {
    // get bitmap
    uint8 *bitmap = font->GetGlyphBitmap (c, w, h);
    if (opt.verbose)
    {
      if (!c) printf ("character ");
      printf ("%d%s", c, (c < lastglyph - 1) ? "," : "\n");
    }

    int bpc = ((width [c] + 7) / 8) * h;

    if (bitmap) 
      if (opt.sourcecode)
      {
        fprintf (out, "  ");
        for (i = 0; i < bpc; i++)
          fprintf (out, "0x%02x%s", bitmap [i], (i >= bpc - 1) && (c >= lastglyph - 1) ? "" : ",");
        fprintf (out, "\t// %02x\n", c);
      }
      else if (width [c])
        fwrite (bitmap, bpc, 1, out);
  }

  fprintf (out, "};\n\n");
  fclose (out);
  font->DecRef ();
  fs->DecRef ();
  return true;
}

int main (int argc, char* argv[])
{
#if defined (__EMX__)	// Expand wildcards on OS/2+GCC+EMX
  _wildcard (&argc, &argv);
#endif

  object_reg = csInitializer::CreateEnvironment ();
  if (!object_reg) return -1;

  if (!csInitializer::RequestPlugins (object_reg, argc, argv,
  	CS_REQUEST_VFS,
	CS_REQUEST_PLUGIN ("crystalspace.font.server.freetype:FontServer", iFontServer),
	CS_REQUEST_END))
  {
    fprintf (stderr, "couldn't init app! (perhaps some plugins are missing?)");
    return -1;
  }

  if (!csInitializer::Initialize (object_reg))
  {
    fprintf (stderr, "couldn't init app! (perhaps some plugins are missing?)");
    return -1;
  }

  programname = argv [0];

  int c;
  while ((c = getopt_long (argc, argv, "f:g:s:o:tdhvV", long_options, NULL)) != EOF)
    switch (c)
    {
      case '?':
        // unknown option
        return -1;
      case 'f':
        opt.first = atoi (optarg);
        if ((opt.first < 0)
         || (opt.first > 255))
        {
          fprintf (stderr, "ERROR: first glyph should be 0..255\n");
          return -2;
        }
        break;
      case 'g':
        opt.glyphs = atoi (optarg);
        if ((opt.glyphs < 1)
         || (opt.glyphs > 256))
        {
          fprintf (stderr, "ERROR: glyph count should be 1..256\n");
          return -2;
        }
        break;
      case 's':
        opt.fontsize = atoi (optarg);
        if ((opt.fontsize < 1)
         || (opt.fontsize > 1000))
        {
          fprintf (stderr, "ERROR: font size should be 1..1000\n");
          return -2;
        }
        break;
      case 'o':
        opt.output = optarg;
        break;
      case 't':
        opt.sourcecode = true;
        break;
      case 'd':
        opt.display = true;
        break;
      case 'h':
        return display_help ();
      case 'v':
        opt.verbose = true;
        break;
      case 'V':
        printf ("%s version %s\n\n", programname, programversion);
        printf ("This program is distributed in the hope that it will be useful,\n");
        printf ("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
        printf ("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n");
        printf ("GNU Library General Public License for more details.\n");
        return 0;
    } /* endswitch */

  if (optind >= argc)
    return display_help ();

  lastglyph = opt.first + opt.glyphs;
  if (lastglyph > 256)
  {
    fprintf (stderr, "WARNING: Last glyph = %d, limiting to 256\n", lastglyph);
    lastglyph = 256;
    opt.glyphs = 256 - opt.first;
  }

  // Interpret the non-option arguments as file names
  for (; optind < argc; ++optind)
    if (!Convert (argv [optind]))
      return -2;

  csInitializer::DestroyApplication (object_reg);

  return 0;
}
