/*

    Graphics File Loader: a test program for csgfxldr library
    Copyright (C) 2000 Andrew Zabolotny <bit@eltech.ru>

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

#define SYSDEF_PATH
#define SYSDEF_GETOPT
#include "sysdef.h"
#include "csgfxldr/csimage.h"
#include "csgfxldr/pngsave.h"

#include <string.h>

char *programversion = "0.0.1";
char *programname;

/*

  NOTE: If your Losing Operating System {tm} (R) does not have getopt_long,
  please use the one in support/gnu instead. Please don't comment out blocks
  of code, dont #ifdef and so on. It is ugly.

*/

static struct option long_options[] =
{
  {"help", no_argument, 0, 'h'},
  {"display", optional_argument, 0, 'd'},
  {"scale", required_argument, 0, 's'},
  {"mipmap", required_argument, 0, 'm'},
  {"transp", required_argument, 0, 't'},
  {"paletted", no_argument, 0, '8'},
  {"verbose", no_argument, 0, 'v'},
  {"version", no_argument, 0, 'V'},
  {0, no_argument, 0, 0}
};

static struct
{
  bool verbose;
  bool save;
  bool display;
  bool paletted;
  int scaleX, scaleY;
  int displayW, displayH;
  int mipmap;
  bool transp;
} opt =
{
  false,
  true,
  false,
  false,
  0, 0,
  79, 24,
  -1,
  false
};
// Dont move inside the struct!
static RGBPixel transpcolor;

static int display_help ()
{
  printf ("Crystal Space Graphics File Loader test application v%s\n", programversion);
  printf ("Copyright (C) 2000 Andrew Zabolotny\n\n");
  printf ("Usage: %s {option/s} [image file] [...]\n\n", programname);
  printf ("  -d   --display{=#,#} Display the image in ASCII format :-)\n");
  printf ("  -s   --scale=#,#     Re-scale the image to given size #\n");
  printf ("  -m   --mipmap=#      Create mipmap level # (=0,1,2,3) from image\n");
  printf ("  -t   --transp=#,#,#  Treat color (R,G,B) as transparent\n");
  printf ("  -8   --paletted      Load/handle/save image in paletted format\n");
  printf ("  -h   --help          Display this help text\n");
  printf ("  -v   --verbose       Comment on what's happening\n");
  printf ("  -V   --version       Display program version\n");
  return 1;
}

#if 0
// PNM is a very simple format that is handy for quick-and-dirty programs.
// Many image processing programs understands it...
static bool SavePNM (const char *fname, void *image, int w, int h, bool rgb)
{
  FILE *f = fopen (fname, "wb");
  if (!f)
  {
    fprintf (stderr, "%s: cannot open output file %s\n", programname, fname);
    return false;
  }
  char header [100];
  sprintf (header, "P%c\n%d %d\n%d\n", rgb ? '6' : '5', w, h, 255);
  fwrite (header, 1, strlen (header), f);
  if (rgb)
    for (int i = w * h; i > 0; i--)
    {
      fwrite (image, 1, 3, f);
      image = ((RGBPixel *)image) + 1;
    }
  else
    fwrite (image, 1, w * h, f);
  fclose (f);
  return true;
}
#endif

static bool process_file (const char *fname)
{
  printf ("Loading file %s\n", fname);

  FILE *f = fopen (fname, "rb");
  if (!f)
  {
    printf ("%s: cannot open file %s\n", programname, fname);
    return false;
  }

  fseek (f, 0, SEEK_END);
  size_t fsize = ftell (f);
  fseek (f, 0, SEEK_SET);

  if (opt.verbose)
    printf ("Reading %ld bytes from file\n", fsize);

  UByte *buffer = new UByte [fsize];
  if (fread (buffer, 1, fsize, f) < fsize)
  {
    printf ("%s: unexpected EOF while reading file %s\n", programname, fname);
    return false;
  }

  fclose (f);

  iImage *ifile = csImageLoader::Load (buffer, fsize,
    opt.paletted ? CS_IMGFMT_PALETTED8 : CS_IMGFMT_TRUECOLOR);
  delete [] buffer;
  if (!ifile)
  {
    printf ("%s: failed to recognise image format for %s\n", programname, fname);
    return false;
  }

  if (opt.verbose)
  {
    printf ("Image size: %d x %d pixels, %d bytes\n", ifile->GetWidth (),
      ifile->GetHeight (), ifile->GetSize ());
    int fmt = ifile->GetFormat ();
    printf ("Image format: %s, alpha channel: %s\n",
      (fmt & CS_IMGFMT_MASK) == CS_IMGFMT_NONE ? "none" :
      (fmt & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8 ? "paletted, 256 colors" :
      (fmt & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR ? "truecolor" : "unknown",
      (fmt & CS_IMGFMT_ALPHA) ? "present" : "not present");
  }

  char suffix [20];
  suffix [0] = 0;

  if (opt.scaleX > 0 && opt.scaleY > 0)
  {
    printf ("Rescaling image to %d x %d\n", opt.scaleX, opt.scaleY);
    ifile->Resize (opt.scaleX, opt.scaleY);
    sprintf (strchr (suffix, 0), "-s%dx%d", opt.scaleX, opt.scaleY);
  }

  if (opt.mipmap >= 0)
  {
    printf ("Creating mipmap level %d from image\n", opt.mipmap);
    iImage *ifile2 = ifile->MipMap (opt.mipmap,
      opt.transp ? &transpcolor : NULL);
    ifile->DecRef ();
    ifile = ifile2;
    sprintf (strchr (suffix, 0), "-m%d", opt.mipmap);
  }

  if (opt.save)
  {
    char outname [MAXPATHLEN + 1];
    strcpy (outname, fname);
    char *eol = strchr (outname, 0);
    while (eol > outname && *eol != '.') eol--;
    if (eol == outname) eol = strchr (outname, 0);
    strcpy (eol, suffix);
    strcat (eol, ".png");
    printf ("Saving output file %s\n", outname);

#if 1
    if (!csSavePNG (outname, ifile))
      return false;
#else
    if (!SavePNM (outname, ifile->GetImageData (), ifile->GetWidth (), ifile->GetHeight (),
         (ifile->GetFormat () & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR))
      return false;
#endif
  }

  if (opt.display)
  {
    static char imgchr [] = " .,;+*oO";
    ifile->Resize (opt.displayW, opt.displayH);
    RGBPixel *rgb;
    UByte *idx = NULL;
    bool truecolor = (ifile->GetFormat () & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR;
    if (truecolor)
      rgb = (RGBPixel *)ifile->GetImageData ();
    else
    {
      idx = (UByte *)ifile->GetImageData ();
      rgb = ifile->GetPalette ();
    }
    for (int y = 0; y < opt.displayH; y++)
    {
      for (int x = 0; x < opt.displayW; x++)
      {
        RGBPixel &src = truecolor ? *rgb++ : rgb [*idx++];
        int gray = int (sqrt (src.red * src.red + src.green * src.green +
          src.blue * src.blue) * 8 / 442);
        putc (imgchr [gray], stdout);
      }
      putc ('\n', stdout);
    }
  }

  // Destroy the image object
  ifile->DecRef ();

  return true;
}

int main (int argc, char **argv)
{
#if defined (__EMX__)	// Expand wildcards on OS/2+GCC+EMX
  _wildcard (&argc, &argv);
#endif

  programname = argv [0];

  int c;
  while ((c = getopt_long (argc, argv, "8d::s:m:t:hvV", long_options, NULL)) != EOF)
    switch (c)
    {
      case '?':
        // unknown option
        return -1;
      case '8':
        opt.paletted = true;
        break;
      case 'd':
        opt.save = false;
        opt.display = true;
        if (optarg &&
            sscanf (optarg, "%d,%d", &opt.displayW, &opt.displayH) != 2)
        {
          printf ("%s: expecting <width>,<height> after -d\n", programname);
          return -1;
        }
        break;
      case 's':
        if (sscanf (optarg, "%d,%d", &opt.scaleX, &opt.scaleY) != 2)
        {
          printf ("%s: expecting <width>,<height> after -s\n", programname);
          return -1;
        }
        break;
      case 't':
      {
        opt.transp = true;
        int r,g,b;
        if (sscanf (optarg, "%d,%d,%d", &r, &g, &b) != 3)
        {
          printf ("%s: expecting <R>,<G>,<B> after -t\n", programname);
          return -1;
        }
        transpcolor.red   = r > 255 ? 255 : r < 0 ? 0 : r;
        transpcolor.green = g > 255 ? 255 : g < 0 ? 0 : g;
        transpcolor.blue  = b > 255 ? 255 : b < 0 ? 0 : b;
        break;
      }
      case 'm':
        if (sscanf (optarg, "%d", &opt.mipmap) != 1)
        {
          printf ("%s: expecting <mipmap level> which is one of 0,1,2,3 after -m\n", programname);
          return -1;
        }
        if (opt.mipmap < 0 || opt.mipmap > 3)
        {
          printf ("%s: bad mipmap level (%d): should be one of 0,1,2,3\n", programname, opt.mipmap);
          return -1;
        }
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

  for (; optind < argc; ++optind)
    if (!process_file (argv [optind]))
      return -1;

  return 0;
}
