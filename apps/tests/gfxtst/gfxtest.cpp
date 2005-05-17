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

#include "cssysdef.h"
#include <string.h>

#include "csgfx/bakekeycolor.h"
#include "csgfx/imagemanipulate.h"
#include "csgfx/imagetools.h"
#include "csgfx/memimage.h"
#include "csqsqrt.h"
#include "cstool/initapp.h"
#include "csutil/cfgfile.h"
#include "csutil/databuf.h"
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
  {"applykey", no_argument, 0, 'T'},
  {"paletted", no_argument, 0, '8'},
  {"truecolor", no_argument, 0, 'c'},
  {"strip-alpha", no_argument, 0, 'a'},
  {"add-alpha", no_argument, 0, 'A'},
  {"sharpen", required_argument, 0, 'p'},
  {"mipmaps", no_argument, 0, 'N'},
  {"save", optional_argument, 0, 'S'},
  {"mime", required_argument, 0, 'M'},
  {"options", required_argument, 0, 'O'},
  {"prefix", required_argument, 0, 'P'},
  {"suffix", required_argument, 0, 'U'},
  {"display", optional_argument, 0, 'D'},
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
  bool applykey;
  int outputmode;
  float hmscale;
  bool info;
  bool stripalpha;
  bool addalpha;
  int sharpen;
  bool mipmaps;
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
  false,
  0,
  1/500.0f,
  false,
  false,
  0,
  false
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
  csPrintf ("Crystal Space Graphics File Loader test application v%s\n", programversion);
  csPrintf ("Copyright (C) 2000 Andrew Zabolotny\n\n");
  csPrintf ("Usage: %s {option/s} [image file] [...]\n\n", programname);
  csPrintf ("  -h   --help          Display this help text\n");
  csPrintf ("  -v   --verbose       Comment on what's happening\n");
  csPrintf ("  -V   --version       Display program version\n");
  csPrintf ("  -F   --formats       Display a list of supported image formats\n");
  csPrintf ("------------------ Image manipulation:  ----------------------------------------\n");
  csPrintf ("  -d   --dither        Apply Floyd-Steinberg dithering when reducing to 8 bpp\n");
  csPrintf ("  -s   --scale=#[,#]   Re-scale the image to given size #\n");
  csPrintf ("  -m   --mipmap=#      Create mipmap level # (>=0) from image\n");
  csPrintf ("  -t   --transp=#,#,#  Treat color (R,G,B) as transparent\n");
  csPrintf ("  -T   --applykey      Apply key color into alpha channel\n");
  csPrintf ("  -8   --paletted      Convert image to 8 bits-per-pixel paletted format\n");
  csPrintf ("  -c   --truecolor     Convert image to truecolor format\n");
  csPrintf ("  -a   --strip-alpha   Remove alpha channel, if present\n");
  csPrintf ("  -A   --add-alpha     Add alpha channel, if not present\n");
  csPrintf ("  -p   --sharpen=#     Sharpen the image, strength #\n");
  csPrintf ("  -N   --mipmaps       Generate mipmaps for the image\n");
  csPrintf ("------------------ Output options (-S, -D, -H are exclusive):  -----------------\n");
  csPrintf ("  -S   --save[=#]      Output an image (default)\n");
  csPrintf ("  -M   --mime=#        Output file mime type (default: image/png)\n");
  csPrintf ("  -O   --options=#     Optional output format options (e.g. \"progressive\")\n");
  csPrintf ("  -P   --prefix=#      Add prefix before output filename\n");
  csPrintf ("  -U   --suffix=#      Add suffix after output filename\n");
  csPrintf ("  -D   --display=#,#   Display the image in ASCII format :-)\n");
  csPrintf ("                       An optional scale argument may be specified\n");
  csPrintf ("  -I   --info          Display image info (and don't do anything more)\n");
  return 1;
}

static int list_supported_formats (iObjectRegistry *r)
{
  const char *mask =
    "%-20s %-40s %-5s %-5s\n";

  const csImageIOFileFormatDescriptions& descr = ImageLoader->GetDescription ();
  size_t i;
  csPrintf (mask, "MIME", "description", "load?", "save?");
  csPrintf (mask, "----", "-----------", "-----", "-----");
  for (i = 0; i < descr.Length (); i++)
  {
    iImageIO::FileFormatDescription *format =
      (iImageIO::FileFormatDescription*) descr.Get (i);

    csPrintf (mask, format->mime, format->subtype,
      (format->cap & CS_IMAGEIO_LOAD)?"yes":"no",
      (format->cap & CS_IMAGEIO_SAVE)?"yes":"no");
  }
  return 1;
}

static bool output_picture (const char *fname, const char *suffix, csRef<iImage> ifile)
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

  csPrintf ("Saving output file %s\n", outname);

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
    csPrintf ("Failed to save %s. Plugin returned no data.\n", outname);
  }
  return true;
}

static bool display_picture (csRef<iImage> ifile)
{
  static char imgchr [] = " .,;+*oO";
  ifile = csImageManipulate::Rescale (ifile, opt.displayW, opt.displayH);
  const csRGBpixel *rgb = ifile->GetPalette ();
  const uint8 *idx = (uint8 *)ifile->GetImageData ();
  int y, x;
  for (y = 0; y < opt.displayH; y++)
  {
    for (x = 0; x < opt.displayW; x++)
    {
      const csRGBpixel &src = rgb [*idx++];
      int gray = int (csQsqrt (src.red * src.red + src.green * src.green +
        src.blue * src.blue) * 8 / 442);
      putc (imgchr [gray], stdout);
    }
    putc ('\n', stdout);
  }
  return true;
}

static bool process_file (const char *fname)
{
  csPrintf ("Loading file %s\n", fname);

  FILE *f = fopen (fname, "rb");
  if (!f)
  {
    csPrintf ("%s: cannot open file %s\n", programname, fname);
    return false;
  }

  fseek (f, 0, SEEK_END);
  size_t fsize = ftell (f);
  fseek (f, 0, SEEK_SET);

  if (opt.verbose)
    csPrintf ("Reading %zu bytes from file\n", fsize);

  csRef<iDataBuffer> buf;
  {
    char* buffer = new char[fsize];
    if (fread (buffer, 1, fsize, f) < fsize)
    {
      csPrintf ("%s: unexpected EOF while reading file %s\n", programname, fname);
      return false;
    }
    buf.AttachNew (new csDataBuffer (buffer, fsize, true));
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

  csRef<iImage> ifile = ImageLoader->Load (buf, fmt | CS_IMGFMT_ALPHA);
  if (!ifile)
  {
    csPrintf ("%s: failed to recognise image format for %s\n",
    	programname, fname);
    return false;
  }

  if (opt.verbose || opt.info)
  {
    csPrintf ("Image size: %d x %d pixels, %zu bytes\n", ifile->GetWidth (),
      ifile->GetHeight (), csImageTools::ComputeDataSize(ifile));
    int fmt = ifile->GetFormat ();
    csPrintf ("Image format: %s, alpha channel: %s\n",
      (fmt & CS_IMGFMT_MASK) == CS_IMGFMT_NONE ? "none" :
      (fmt & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8 ? "paletted, 256 colors" :
      (fmt & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR ? "truecolor" : "unknown",
      (fmt & CS_IMGFMT_ALPHA) ? "present" : "not present");
  }

  if (opt.applykey && (opt.transp || ifile->HasKeyColor()))
  {
    csRGBpixel transp;
    if (opt.transp)
      transp = transpcolor;
    else
    {
      int kr, kg, kb;
      ifile->GetKeyColor (kr, kg, kb);
      transp.Set (kr, kg, kb);
    }
    ifile = csBakeKeyColor::Image (ifile, transp);
  }

  csString suffix;

  if (opt.scaleX > 0)
  {
    int scaleY = opt.scaleY;
    if (scaleY == -1)
    {
      scaleY = (ifile->GetHeight () * opt.scaleX) / ifile->GetWidth ();
    }
    csPrintf ("Rescaling image to %d x %d\n", opt.scaleX, scaleY);
    ifile = csImageManipulate::Rescale (ifile, opt.scaleX, scaleY);
    suffix.AppendFmt ("-s%dx%d", opt.scaleX, scaleY);
  }

  if (opt.mipmap >= 0)
  {
    csPrintf ("Creating mipmap level %d from image\n", opt.mipmap);
    ifile = csImageManipulate::Mipmap (ifile, opt.mipmap,
      opt.transp ? &transpcolor : 0);
    suffix.AppendFmt ("-m%d", opt.mipmap);
  }

  if (opt.stripalpha)
  {
    int format = ifile->GetFormat ();
    if (format & CS_IMGFMT_ALPHA)
    {
      csPrintf ("Removing alpha channel from image\n");
      ifile.AttachNew (new csImageMemory (ifile, format & ~CS_IMGFMT_ALPHA));
    }
  }

  if (opt.addalpha)
  {
    int format = ifile->GetFormat ();
    if (!(format & CS_IMGFMT_ALPHA))
    {
      csPrintf ("Adding alpha channel from image\n");
      ifile.AttachNew (new csImageMemory (ifile, format | CS_IMGFMT_ALPHA));

      // merge keycolor into alpha
      if (ifile->HasKeyColor())
      {
	int pixels = ifile->GetWidth() * ifile->GetHeight();

	int key_r, key_g, key_b;
	ifile->GetKeyColor (key_r, key_g, key_b);
	csRGBcolor key (key_r, key_g, key_b);

	int i;
	switch (format & CS_IMGFMT_MASK)
	{
	case CS_IMGFMT_PALETTED8:
	  {
	    csImageMemory* newImg = new csImageMemory (ifile);
	    const uint8 *data = (uint8*)ifile->GetImageData();
	    const csRGBpixel *pal = ifile->GetPalette();
	    uint8 *alphamap = newImg->GetAlphaPtr();
	    for (i = 0; i < pixels; i++)
	    {
	      if (pal[data[i]] == key)
	      {
		alphamap[i] = 0;
	      }
	    }
	    ifile.AttachNew (newImg);
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
    csPrintf ("Sharpening image with strength %d\n", opt.sharpen);
    ifile = csImageManipulate::Sharpen (ifile, opt.sharpen,
      opt.transp ? &transpcolor : 0);
  }

  if (opt.mipmaps)
  {
    csImageMemory* newImage = new csImageMemory (ifile);
    csRef<iImage> mip = ifile;
    uint m = 1;
    while ((mip->GetWidth() > 1) || (mip->GetHeight() > 1) 
      || (mip->GetDepth() > 1))
    {
      mip = csImageManipulate::Mipmap (mip, 1, opt.transp ? &transpcolor : 0);
      newImage->SetMipmap (m, mip);
      m++;
    }
    ifile.AttachNew (newImage);
  }

  bool success = false;
  switch (opt.outputmode)
  {
    case 0:
      success = output_picture (fname, suffix.GetDataSafe(), ifile);
      break;
    case 1:
      success = display_picture (ifile);
      break;
    case 2:
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
  while ((c = getopt_long (argc, argv, "8cdaAs:m:t:p:D:S::M:O:P:U::H::IhvVFTN", long_options, 0)) != EOF)
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
      case 'N':
        opt.mipmaps = true;
        break;
      case 'P':
	if (optarg && sscanf (optarg, "%s", prefix_name) != 1)
	{
          csPrintf ("%s: expecting <prefix> after -P\n", programname);
          return -1;
	}
        break;
      case 'U':
	if (optarg && sscanf (optarg, "%s", suffix_name) != 1)
	{
          csPrintf ("%s: expecting <suffix> after -U\n", programname);
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
          csPrintf ("%s: expecting <mime-type> after -M\n", programname);
          return -1;
	}
	break;
      case 'O':
	if (optarg && sscanf (optarg, "%s", output_opts) != 1)
	{
          csPrintf ("%s: expecting <output-options> after -O\n", programname);
          return -1;
	}
	break;
      case 'D':
        opt.outputmode = 1;
        if (optarg &&
            sscanf (optarg, "%d,%d", &opt.displayW, &opt.displayH) != 2)
        {
          csPrintf ("%s: expecting <width>,<height> after -d\n", programname);
          return -1;
        }
        break;
      case 's':
	{
          int rc = sscanf (optarg, "%d,%d", &opt.scaleX, &opt.scaleY);
	  if (rc != 1 && rc != 2)
          {
            csPrintf ("%s: expecting <width>[,<height>] after -s\n", programname);
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
          csPrintf ("%s: expecting <R>,<G>,<B> after -t\n", programname);
          return -1;
        }
        transpcolor.red   = r > 255 ? 255 : r < 0 ? 0 : r;
        transpcolor.green = g > 255 ? 255 : g < 0 ? 0 : g;
        transpcolor.blue  = b > 255 ? 255 : b < 0 ? 0 : b;
        break;
      }
      case 'T':
      {
	opt.applykey = true;
	break;
      }
      case 'm':
        if (sscanf (optarg, "%d", &opt.mipmap) != 1)
        {
          csPrintf ("%s: expecting <mipmap level> which is >=0 after -m\n", programname);
          return -1;
        }
        if (opt.mipmap < 0)
        {
          csPrintf ("%s: bad mipmap level (%d): should be >=0\n", programname, opt.mipmap);
          return -1;
        }
        break;
      case 'I':
        opt.outputmode = 2;
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
          csPrintf ("%s: expecting <sharpening strength> after -p\n", programname);
          return -1;
        }
      case 'V':
        csPrintf ("%s version %s\n\n", programname, programversion);
        csPrintf ("This program is distributed in the hope that it will be useful,\n");
        csPrintf ("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
        csPrintf ("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n");
        csPrintf ("GNU Library General Public License for more details.\n");
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
    csPrintf("could not load image loader\n");
    csInitializer::DestroyApplication (object_reg);
    return -1;
  }

  int ret = gfxtest_main (object_reg, argc, argv);
  
  ImageLoader = 0;
  csInitializer::DestroyApplication (object_reg);
  return ret;
}
