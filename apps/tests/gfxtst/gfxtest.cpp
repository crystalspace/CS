/*

    Graphics File Loader: a test program for the image loader plugin
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

#include <string.h>
#include "cssysdef.h"

#include "csgfx/csimage.h"
#include "csqsqrt.h"
#include "cstool/initapp.h"
#include "csutil/cfgfile.h"
#include "csutil/cmdhelp.h"
#include "csutil/getopt.h"
#include "csutil/util.h"
#include "igraphic/imageio.h"
#include "iutil/comp.h"
#include "iutil/databuff.h"
#include "iutil/eventh.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"

CS_IMPLEMENT_APPLICATION

char *programversion = "0.0.1";
char *programname;
csRef<iImageIO> ImageLoader;

static struct option long_options[] =
{
  {"help", no_argument, 0, 'h'},
  {"verbose", no_argument, 0, 'v'},
  {"version", no_argument, 0, 'V'},
  {"formats", no_argument, 0, 'F'},
  {"dither", no_argument, 0, 'd'},
  {"scale", required_argument, 0, 's'},
  {"mipmap", required_argument, 0, 'm'},
  {"transp", required_argument, 0, 't'},
  {"paletted", no_argument, 0, '8'},
  {"truecolor", no_argument, 0, 'c'},
  {"strip-alpha", no_argument, 0, 'a'},
  {"sharpen", required_argument, 0, 'p'},
  {"save", optional_argument, 0, 'S'},
  {"mime", required_argument, 0, 'M'},
  {"options", required_argument, 0, 'O'},
  {"prefix", required_argument, 0, 'P'},
  {"suffix", required_argument, 0, 'U'},
  {"display", optional_argument, 0, 'D'},
  {"heightmap", optional_argument, 0, 'H'},
  {"info", no_argument, 0, 'I'},
  {0, no_argument, 0, 0}
};

static struct
{
  bool verbose;
  bool dither;
  bool paletted;
  bool truecolor;
  int scaleX, scaleY;
  int displayW, displayH;
  int mipmap;
  bool transp;
  int outputmode;
  float hmscale;
  bool info;
  bool stripalpha;
  bool addalpha;
  int sharpen;
} opt =
{
  false,
  true,
  false,
  false,
  0, 0,
  79, 24,
  -1,
  false,
  0,
  1/500.0f,
  false,
  false,
  0
};
// Dont move inside the struct!
static csRGBpixel transpcolor;
char output_name[512] = "";
char prefix_name[512] = "";
char suffix_name[512] = "";
char output_mime[512] = "image/png";
char output_opts[512] = "";

static int display_help ()
{
  printf ("Crystal Space Graphics File Loader test application v%s\n", programversion);
  printf ("Copyright (C) 2000 Andrew Zabolotny\n\n");
  printf ("Usage: %s {option/s} [image file] [...]\n\n", programname);
  printf ("  -h   --help          Display this help text\n");
  printf ("  -v   --verbose       Comment on what's happening\n");
  printf ("  -V   --version       Display program version\n");
  printf ("  -F   --formats       Display a list of supported image formats\n");
  printf ("------------------ Image manipulation:  ----------------------------------------\n");
  printf ("  -d   --dither        Apply Floyd-Steinberg dithering when reducing to 8 bpp\n");
  printf ("  -s   --scale=#[,#]   Re-scale the image to given size #\n");
  printf ("  -m   --mipmap=#      Create mipmap level # (>=0) from image\n");
  printf ("  -t   --transp=#,#,#  Treat color (R,G,B) as transparent\n");
  printf ("  -8   --paletted      Convert image to 8 bits-per-pixel paletted format\n");
  printf ("  -c   --truecolor     Convert image to truecolor format\n");
  printf ("  -a   --strip-alpha   Remove alpha channel, if present\n");
  printf ("  -A   --add-alpha     Add alpha channel, if not present\n");
  printf ("  -p   --sharpen=#     Sharpen the image, strength #\n");
  printf ("------------------ Output options (-S, -D, -H are exclusive):  -----------------\n");
  printf ("  -S   --save[=#]      Output an image (default)\n");
  printf ("  -M   --mime=#        Output file mime type (default: image/png)\n");
  printf ("  -O   --options=#     Optional output format options (e.g. \"progressive\")\n");
  printf ("  -P   --prefix=#      Add prefix before output filename\n");
  printf ("  -U   --suffix=#      Add suffix after output filename\n");
  printf ("  -D   --display=#,#   Display the image in ASCII format :-)\n");
  printf ("  -H   --heightmap[=#] Output a 3D heightmap in Crystal Space format\n");
  printf ("                       An optional scale argument may be specified\n");
  printf ("  -I   --info          Display image info (and don't do anything more)\n");
  return 1;
}

static int list_supported_formats (iObjectRegistry *r)
{
  const char *mask =
    "%-20s %-40s %-5s %-5s\n";

  const csImageIOFileFormatDescriptions& descr = ImageLoader->GetDescription ();
  size_t i;
  printf (mask, "MIME", "description", "load?", "save?");
  printf (mask, "----", "-----------", "-----", "-----");
  for (i = 0; i < descr.Length (); i++)
  {
    iImageIO::FileFormatDescription *format =
      (iImageIO::FileFormatDescription*) descr.Get (i);

    printf (mask, format->mime, format->subtype,
      (format->cap & CS_IMAGEIO_LOAD)?"yes":"no",
      (format->cap & CS_IMAGEIO_SAVE)?"yes":"no");
  }
  return 1;
}

static bool output_picture (const char *fname, const char *suffix, iImage *ifile)
{
  char outname [CS_MAXPATHLEN + 1];
  char* eol;
  if (prefix_name[0])
  {
    strcpy (outname, prefix_name);
    eol = strchr (outname, 0);
  }
  else eol = outname;

  if (output_name[0])
    strcpy (eol, output_name);
  else
  {
    strcpy (eol, fname);
    if (!suffix_name[0])
    {
      eol = strchr (eol, 0);
      while (eol > outname && *eol != '.') eol--;
      if (eol == outname) eol = strchr (outname, 0);
      strcpy (eol, suffix);
      const char* defext =  strchr (output_mime, '/');
      if (defext)
      {
	defext++;
	// skip a leading "x-" in the mime type (eg "image/x-jng")
	if (!strncmp (defext, "x-", 2)) defext += 2; 
        strcat (eol, ".");
        strcat (eol, defext); // default ext from mime type
      }
    }
  }
  if (suffix_name[0])
  {
    strcat (eol, suffix_name);
  }		 

  printf ("Saving output file %s\n", outname);

  csRef<iDataBuffer> db (ImageLoader->Save (ifile, output_mime, output_opts));
  if (db)
  {
    FILE *f = fopen (outname, "wb");
    if (f)
      fwrite (db->GetData (), 1, db->GetSize (), f);
    fclose (f);
  }
  else
  {
    printf ("Failed to save %s. Plugin returned no data.\n", outname);
  }
  return true;
}

static bool display_picture (iImage *ifile)
{
  static char imgchr [] = " .,;+*oO";
  ifile->Rescale (opt.displayW, opt.displayH);
  csRGBpixel *rgb = ifile->GetPalette ();
  uint8 *idx = (uint8 *)ifile->GetImageData ();
  int y, x;
  for (y = 0; y < opt.displayH; y++)
  {
    for (x = 0; x < opt.displayW; x++)
    {
      csRGBpixel &src = rgb [*idx++];
      int gray = int (csQsqrt (src.red * src.red + src.green * src.green +
        src.blue * src.blue) * 8 / 442);
      putc (imgchr [gray], stdout);
    }
    putc ('\n', stdout);
  }
  return true;
}

static bool output_heightmap (const char *fname, iImage *ifile)
{
  char outname [CS_MAXPATHLEN + 1];
  strcpy (outname, fname);
  char *eol = strchr (outname, 0);
  while (eol > outname && *eol != '.') eol--;
  if (eol == outname) eol = strchr (outname, 0);
  strcpy (eol, ".csm");
  printf ("Saving output file %s\n", outname);

  FILE *f = fopen (outname, "w");
  if (!f)
  {
    printf ("%s: cannot write file %s\n", programname, fname);
    return false;
  }

  // Compute the intensity of each color
  int w = ifile->GetWidth ();
  int h = ifile->GetHeight ();
  uint8 *img = (uint8 *)ifile->GetImageData ();

  int height [257], tc = 0;
  csRGBpixel *pal = ifile->GetPalette ();
  height [256] = transpcolor.Intensity ();
  int i;
  for (i = 0; i < 256; i++)
    if (opt.transp && transpcolor.eq (pal [i]))
      height [tc = i] = -1;
    else
      height [i] = pal [i].Intensity ();

  // And now write the 3D file
  *eol = 0;
  csSplitPath (outname, 0, 0, outname, sizeof (outname));
  fprintf (f, "WORLD\n(\n  THING '%s'\n  (\n", outname);

  int x, y;
  int size = w * h;
  int *idx = new int [size];
  int src = 0, curidx = 0;
  int px [3][3];

  // Initialized to stop MSVC 5/6/7 whining.
  px [0][0] = px [1][0] = px [1][0] = px [2][0] =
    px [0][1] = px [1][1] = px [1][1] = px [2][1] =
    px [0][2] = px [1][2] = px [1][2] = px [2][2] = 0;

  for (y = 0; y < h; y++)
  {
    px [0][0] = px [0][1] = px [0][2] =
    px [1][0] = px [1][1] = px [1][2] = tc;
    bool tr = (y > 0), br = (y < h - 1);
    px [2][0] = tr ? img [src - w] : tc;
    px [2][1] = img [src];
    px [2][0] = br ? img [src + w] : tc;
    for (x = 0; x < w; x++, src++)
    {
      px [0][0] = px [1][0]; px [1][0] = px [2][0];
      px [0][1] = px [1][1]; px [1][1] = px [2][1];
      px [0][2] = px [1][2]; px [1][2] = px [2][2];
      if (x < w - 1)
      {
        px [2][0] = tr ? img [src - w + 1] : tc;
        px [2][1] = img [src + 1];
        px [2][2] = br ? img [src + w + 1] : tc;
      }
      else
        px [2][0] = px [2][1] = px [2][2] = tc;

      int pix = px [1][1];
      if (height [pix] < 0)
      {
        // Check if all surrounding pixels are transparent
        if (height [px [0][0]] >= 0
         || height [px [1][0]] >= 0
         || height [px [2][0]] >= 0
         || height [px [0][1]] >= 0
         || height [px [2][1]] >= 0
         || height [px [0][2]] >= 0
         || height [px [1][2]] >= 0
         || height [px [2][2]] >= 0)
          pix = 256;
      }

      if (height [pix] >= 0)
      {
        idx [src] = curidx++;
        fprintf (f, "    VERTEX (%g,%g,%g)\n", (x - w / 2) / 10.0,
          height [img [src]] * opt.hmscale, (h / 2 - y) / 10.0);
      }
      else
        idx [src] = -1;
    }
  }

  for (y = 1; y < h; y++)
    for (x = 1; x < w; x++)
    {
      src = (y - 1) * w + x - 1;
      int h00 = idx [src];
      int h10 = idx [src + 1];
      int h01 = idx [src + w];
      int h11 = idx [src + w + 1];
      if (h00 < 0 || h10 < 0 || h01 < 0 || h11 < 0)
      {
        if (h00 >= 0 && h10 >= 0 && h11 >= 0)
          fprintf (f, "    POLYGON(VERTICES(%d,%d,%d))\n", h00, h10, h11);
        else if (h00 >= 0 && h11 >= 0 && h01 >= 0)
          fprintf (f, "    POLYGON(VERTICES(%d,%d,%d))\n", h00, h11, h01);
        else if (h10 >= 0 && h11 >= 0 && h01 >= 0)
          fprintf (f, "    POLYGON(VERTICES(%d,%d,%d))\n", h10, h11, h01);
        else if (h00 >= 0 && h10 >= 0 && h01 >= 0)
          fprintf (f, "    POLYGON(VERTICES(%d,%d,%d))\n", h00, h10, h01);
      }
      else
        fprintf (f, "    POLYGON(VERTICES(%d,%d,%d)) POLYGON(VERTICES(%d,%d,%d))\n",
          h00, h10, h11, h00, h11, h01);
    }

  delete idx;

  fprintf (f, "  )\n)\n");
  fclose (f);

  return true;
}

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
    printf ("Reading %ld bytes from file\n", (long)fsize);

  uint8 *buffer = new uint8 [fsize];
  if (fread (buffer, 1, fsize, f) < fsize)
  {
    printf ("%s: unexpected EOF while reading file %s\n", programname, fname);
    return false;
  }

  fclose (f);

  int fmt;
  if (opt.outputmode > 0
   || opt.paletted)
    fmt = CS_IMGFMT_PALETTED8;
  else if (opt.truecolor)
    fmt = CS_IMGFMT_TRUECOLOR;
  else
    fmt = CS_IMGFMT_ANY;

  csRef<iImage> ifile (
  	ImageLoader->Load (buffer, fsize, fmt | CS_IMGFMT_ALPHA));
  delete [] buffer;
  if (!ifile)
  {
    printf ("%s: failed to recognise image format for %s\n",
    	programname, fname);
    return false;
  }

  if (opt.verbose || opt.info)
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

  if (opt.scaleX > 0)
  {
    int scaleY = opt.scaleY;
    if (scaleY == -1)
    {
      scaleY = (ifile->GetHeight () * opt.scaleX) / ifile->GetWidth ();
    }
    printf ("Rescaling image to %d x %d\n", opt.scaleX, scaleY);
    ifile->Rescale (opt.scaleX, scaleY);
    sprintf (strchr (suffix, 0), "-s%dx%d", opt.scaleX, scaleY);
  }

  if (opt.mipmap >= 0)
  {
    printf ("Creating mipmap level %d from image\n", opt.mipmap);
    csRef<iImage> ifile2 (csPtr<iImage> (ifile->MipMap (opt.mipmap,
      opt.transp ? &transpcolor : 0)));
    ifile = ifile2;
    sprintf (strchr (suffix, 0), "-m%d", opt.mipmap);
  }

  if (opt.stripalpha)
  {
    int format = ifile->GetFormat ();
    if (format & CS_IMGFMT_ALPHA)
    {
      printf ("Removing alpha channel from image\n");
      ifile->SetFormat (format & ~CS_IMGFMT_ALPHA);
    }
  }

  if (opt.addalpha)
  {
    int format = ifile->GetFormat ();
    if (!(format & CS_IMGFMT_ALPHA))
    {
      printf ("Adding alpha channel from image\n");
      ifile->SetFormat (format | CS_IMGFMT_ALPHA);

      // merge keycolor into alpha
      if (ifile->HasKeycolor())
      {
	int pixels = ifile->GetWidth() * ifile->GetHeight();

	int key_r, key_g, key_b;
	ifile->GetKeycolor (key_r, key_g, key_b);
	csRGBcolor key (key_r, key_g, key_b);

	int i;
	switch (format & CS_IMGFMT_MASK)
	{
	case CS_IMGFMT_PALETTED8:
	  {
	    uint8 *data = (uint8*)ifile->GetImageData();
	    csRGBpixel *pal = ifile->GetPalette();
	    uint8 *alphamap = (uint8*)ifile->GetAlpha();
	    for (i = 0; i < pixels; i++)
	    {
	      if (pal[data[i]] == key)
	      {
		alphamap[i] = 0;
	      }
	    }
	  }
	  break;
	case CS_IMGFMT_TRUECOLOR:
	  {
	    csRGBpixel *data = (csRGBpixel*)ifile->GetImageData();
	    for (i = 0; i < pixels; i++)
	    {
	      if (data[i] == key)
	      {
		data[i].alpha = 0;
	      }
	    }
	  }
	  break;
	}
      }
    }
  }

  if (opt.sharpen)
  {
    printf ("Sharpening image with strength %d\n", opt.sharpen);
    csRef<iImage> ifile2 (csPtr<iImage> (
    	ifile->Sharpen (opt.transp ? &transpcolor : 0, opt.sharpen)));
    ifile = ifile2;
  }

  bool success = false;
  switch (opt.outputmode)
  {
    case 0:
      success = output_picture (fname, suffix, ifile);
      break;
    case 1:
      success = display_picture (ifile);
      break;
    case 2:
      success = output_heightmap (fname, ifile);
      break;
    case 3:
      success = true;
      break;
  }

  // Destroy the image object

  return success;
}

int gfxtest_main (iObjectRegistry* object_reg, int argc, char *argv[])
{
  programname = argv [0];

  int c;
  while ((c = getopt_long (argc, argv, "8cdaAs:m:t:p:D:S::M:O:P:U::H::IhvVF", long_options, 0)) != EOF)
    switch (c)
    {
      case '?':
        // unknown option
        return -1;
      case '8':
        opt.paletted = true;
        break;
      case 'c':
        opt.truecolor = true;
        break;
      case 'd':
        opt.dither = true;
        break;
      case 'a':
        opt.stripalpha = true;
        break;
      case 'A':
        opt.addalpha = true;
        break;
      case 'P':
	if (optarg && sscanf (optarg, "%s", prefix_name) != 1)
	{
          printf ("%s: expecting <prefix> after -P\n", programname);
          return -1;
	}
        break;
      case 'U':
	if (optarg && sscanf (optarg, "%s", suffix_name) != 1)
	{
          printf ("%s: expecting <suffix> after -U\n", programname);
          return -1;
	}
        break;
      case 'S':
        opt.outputmode = 0;
	if (optarg) sscanf (optarg, "%s", output_name);
	break;
      case 'M':
	if (optarg && sscanf (optarg, "%s", output_mime) != 1)
	{
          printf ("%s: expecting <mime-type> after -M\n", programname);
          return -1;
	}
	break;
      case 'O':
	if (optarg && sscanf (optarg, "%s", output_opts) != 1)
	{
          printf ("%s: expecting <output-options> after -O\n", programname);
          return -1;
	}
	break;
      case 'D':
        opt.outputmode = 1;
        if (optarg &&
            sscanf (optarg, "%d,%d", &opt.displayW, &opt.displayH) != 2)
        {
          printf ("%s: expecting <width>,<height> after -d\n", programname);
          return -1;
        }
        break;
      case 's':
	{
          int rc = sscanf (optarg, "%d,%d", &opt.scaleX, &opt.scaleY);
	  if (rc != 1 && rc != 2)
          {
            printf ("%s: expecting <width>[,<height>] after -s\n", programname);
            return -1;
          }
	  if (rc == 1) opt.scaleY = -1;
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
          printf ("%s: expecting <mipmap level> which is >=0 after -m\n", programname);
          return -1;
        }
        if (opt.mipmap < 0)
        {
          printf ("%s: bad mipmap level (%d): should be >=0\n", programname, opt.mipmap);
          return -1;
        }
        break;
      case 'H':
        opt.outputmode = 2;
        if (optarg)
        {
          sscanf (optarg, "%g", &opt.hmscale);
          opt.hmscale *= 1/500.0f;
        }
        break;
      case 'I':
        opt.outputmode = 3;
        opt.info = true;
        break;
      case 'h':
	return display_help ();
      case 'v':
        opt.verbose = true;
        break;
      case 'p':
        if (sscanf (optarg, "%d", &opt.sharpen) != 1)
        {
          printf ("%s: expecting <sharpening strength> after -p\n", programname);
          return -1;
        }
      case 'V':
        printf ("%s version %s\n\n", programname, programversion);
        printf ("This program is distributed in the hope that it will be useful,\n");
        printf ("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
        printf ("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n");
        printf ("GNU Library General Public License for more details.\n");
        return 0;
	break;
      case 'F':
	return list_supported_formats (object_reg);
    } /* endswitch */

  if (optind >= argc)
  {
    return display_help ();
  }

  ImageLoader->SetDithering (opt.dither);

  for (; optind < argc; ++optind)
    process_file (argv [optind]);
  ImageLoader = 0;

  return 0;
}

int main (int argc, char *argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return -1;

  if (!csInitializer::SetupConfigManager (object_reg, 0))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.graphics3d.gfxtest",
	"Error initializing system !");
    return -1;
  }

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_OPENGL3D,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.graphics3d.gfxtest",
	"Error initializing system !");
    return -1;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    ImageLoader = 0;
    csInitializer::DestroyApplication (object_reg);
    exit (0);
  }

  ImageLoader = CS_QUERY_REGISTRY (object_reg, iImageIO);
  if (!ImageLoader)
  {
    printf("could not load image loader\n");
    csInitializer::DestroyApplication (object_reg);
    return -1;
  }

  int ret = gfxtest_main (object_reg, argc, argv);
  
  ImageLoader = 0;
  csInitializer::DestroyApplication (object_reg);
  return ret;
}
