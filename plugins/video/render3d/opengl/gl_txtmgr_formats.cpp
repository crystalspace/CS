/*
    Copyright (C) 1998-2004 by Jorrit Tyberghein
	      (C) 2003 by Philip Aumayr
	      (C) 2004 by Frank Richter

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

#include "gl_render3d.h"
#include "gl_txtmgr.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

void csGLTextureManager::InitFormats ()
{
  G3D->ext->InitGL_EXT_abgr ();

  if (G3D->ext->CS_GL_EXT_texture_compression_s3tc)
  {
    specialFormats.Put ("*dxt1", 
      TextureStorageFormat (GL_COMPRESSED_RGB_S3TC_DXT1_EXT));
    specialFormats.Put ("*dxt1a", 
      TextureStorageFormat (GL_COMPRESSED_RGBA_S3TC_DXT1_EXT));
    specialFormats.Put ("*dxt3", 
      TextureStorageFormat (GL_COMPRESSED_RGBA_S3TC_DXT3_EXT));
    specialFormats.Put ("*dxt5", 
      TextureStorageFormat (GL_COMPRESSED_RGBA_S3TC_DXT5_EXT));
  }
}

/* Format tables - maps component sizes to GL sizes.
 * A lot of source formats have the same 'type' bit only differ in 'format'.
 * So the tables below store the 'type' information for given component sizes
 * and the 'format' is chosen based on the input component order.
 */

struct FormatTemplate
{
  /// Component sizes
  int size[4];
  /// Target format index
  int targetFmtIndex;
  /// Source type
  GLenum srcType;
};

// Special type when a certain format/type combo is not possible
static const GLenum UNSUPPORTED                     = 0x7979ffff;
// "Pseudo-types" that are later translated to "real" types in FixupFormats()
static const GLenum UNSIGNED_SHORTS_16_16_16_16     = 0x79790000;
static const GLenum UNSIGNED_BYTES_8_8_8            = 0x79790001;
static const GLenum UNSIGNED_SHORTS_16_16_16        = 0x79790002;
static const GLenum UNSIGNED_SHORTS_16_16_16_16_REV = 0x79790003;

/* Add an indirection for the target format so we don't have to create
   a table if only the target format slightly varies. */
enum
{
  fmtColor4 = 0,
  fmtColor5_1,
  fmtColor8,
  fmtColor10_2,
  fmtColor16,

  fmtColorNum
};

static const GLenum targetRGBA[fmtColorNum] = { 
  GL_RGBA4, GL_RGB5_A1, GL_RGBA8, GL_RGB10_A2, GL_RGBA16
};

static const GLenum targetRGB[fmtColorNum] = { 
  GL_RGB4, GL_RGB5, GL_RGB8, GL_RGB10, GL_RGB16
};

static const FormatTemplate formatsRGBA[] = {
  {{ 4,  4,  4,  4}, fmtColor4,    GL_UNSIGNED_SHORT_4_4_4_4},
  {{ 5,  5,  5,  1}, fmtColor5_1,  GL_UNSIGNED_SHORT_5_5_5_1},
  {{ 8,  8,  8,  8}, fmtColor8,    GL_UNSIGNED_INT_8_8_8_8},
  {{10, 10, 10,  2}, fmtColor10_2, GL_UNSIGNED_INT_10_10_10_2},
  {{16, 16, 16, 16}, fmtColor16,   UNSIGNED_SHORTS_16_16_16_16},
  {{0, 0, 0, 0}, -1, 0}
};

static const FormatTemplate formatsRGB[] = {
  {{ 4,  4,  4,  0}, fmtColor4,    GL_UNSIGNED_SHORT_4_4_4_4},
  {{ 5,  5,  5,  0}, fmtColor5_1,  GL_UNSIGNED_SHORT_5_5_5_1},
  {{ 5,  6,  5,  0}, fmtColor5_1,  GL_UNSIGNED_SHORT_5_6_5},
  {{ 8,  8,  8,  0}, fmtColor8,    UNSIGNED_BYTES_8_8_8},
  {{10, 10, 10,  0}, fmtColor10_2, GL_UNSIGNED_INT_10_10_10_2},
  {{16, 16, 16,  0}, fmtColor16,   UNSIGNED_SHORTS_16_16_16},
  {{0, 0, 0, 0}, -1, 0}
};

static const FormatTemplate formatsARGB[] = {
  {{ 4,  4,  4,  4}, fmtColor4,    GL_UNSIGNED_SHORT_4_4_4_4_REV},
  {{ 1,  5,  5,  5}, fmtColor5_1,  GL_UNSIGNED_SHORT_1_5_5_5_REV},
  {{ 8,  8,  8,  8}, fmtColor8,    GL_UNSIGNED_INT_8_8_8_8_REV},
  {{ 2, 10, 10, 10}, fmtColor10_2, GL_UNSIGNED_INT_2_10_10_10_REV},
  {{16, 16, 16, 16}, fmtColor16,   UNSIGNED_SHORTS_16_16_16_16_REV},
  {{0, 0, 0, 0}, 0, 0}
};

enum
{
  fmtLum8 = 0,
  fmtLum16,

  fmtLumNum
};

// Luminance formats
static const GLenum targetLum[fmtLumNum] = { 
  GL_LUMINANCE8, GL_LUMINANCE16
};

static const FormatTemplate formatsLum[] = {
  {{ 8,  0,  0,  0}, fmtLum8,  GL_UNSIGNED_BYTE},
  {{16,  0,  0,  0}, fmtLum16, GL_UNSIGNED_SHORT},
  {{0, 0, 0, 0}, -1, 0}
};

// Luminance+alpha formats
static const GLenum targetALum[fmtLumNum] = { 
  GL_LUMINANCE8_ALPHA8, GL_LUMINANCE16_ALPHA16
};

static const FormatTemplate formatsALum[] = {
  {{ 8,  8,  0,  0}, fmtLum8,  GL_UNSIGNED_BYTE},
  {{16, 16,  0,  0}, fmtLum16, GL_UNSIGNED_SHORT},
  {{0, 0, 0, 0}, -1, 0}
};

// Check if a format/type combo is supported by the available extensions
bool csGLTextureManager::FormatSupported (GLenum srcFormat, GLenum srcType)
{
  if (srcFormat == UNSUPPORTED) return false;
  if ((srcFormat == GL_ABGR_EXT) && !G3D->ext->CS_GL_EXT_abgr) return false;
  if ((srcFormat == GL_BGR) && !G3D->ext->CS_GL_version_1_2) return false;
  if ((srcFormat == GL_BGRA) && !G3D->ext->CS_GL_version_1_2) return false;
  if ((srcType >= GL_UNSIGNED_BYTE_3_3_2) 
    && (srcType <= GL_UNSIGNED_INT_10_10_10_2)
    && !G3D->ext->CS_GL_version_1_2) return false;
  if ((srcType >= GL_UNSIGNED_BYTE_2_3_3_REV) 
    && (srcType <= GL_UNSIGNED_INT_2_10_10_10_REV)
    && !G3D->ext->CS_GL_version_1_2) return false;

  return true;
}

/* Some formats can be simplified. Some can be represented by GL, just
   with some different format/type combo but which can't quite be represented
   given the tables only contain a source type but no format - these
   are special-cased here.
 */
static void FixupFormatAndType (GLenum& srcFormat, GLenum& srcType)
{
  // Translate/simplify some special cases
  switch (srcType)
  {
    case GL_UNSIGNED_INT_8_8_8_8_REV:
      if ((srcFormat == GL_RGBA) || (srcFormat == GL_BGRA))
        srcType = GL_UNSIGNED_BYTE;
      break;
    case UNSIGNED_SHORTS_16_16_16_16:
      srcType = GL_UNSIGNED_SHORT;
      if (srcFormat == GL_RGBA)
        srcFormat = GL_ABGR_EXT;
      else if (srcFormat == GL_BGRA)
        // This would require an "ARGB" extension which does not exist.
        srcFormat = UNSUPPORTED;
      break;
    case UNSIGNED_BYTES_8_8_8:
      srcType = GL_UNSIGNED_BYTE;
      if (srcFormat == GL_RGB)
        srcFormat = GL_BGR;
      else if (srcFormat == GL_BGR)
        srcFormat = GL_RGB;
      break;
    case UNSIGNED_SHORTS_16_16_16:
      srcType = GL_UNSIGNED_SHORT;
      if (srcFormat == GL_RGB)
        srcFormat = GL_BGR;
      else if (srcFormat == GL_BGR)
        srcFormat = GL_RGB;
      break;
    case UNSIGNED_SHORTS_16_16_16_16_REV:
      if ((srcFormat == GL_RGBA) || (srcFormat == GL_BGRA))
        srcType = GL_UNSIGNED_SHORT;
      else if (srcFormat == GL_RGB)
      {
        srcFormat = GL_RGBA;
        srcType = GL_UNSIGNED_SHORT;
      }
      else if (srcFormat == GL_BGR)
      {
        srcFormat = GL_BGRA;
        srcType = GL_UNSIGNED_SHORT;
      }
      break;
  }
}

bool csGLTextureManager::DetermineGLFormat (
  const CS::StructuredTextureFormat& format, TextureStorageFormat& glFormat)
{
  switch (format.GetFormat())
  {
    case CS::StructuredTextureFormat::Integer:
      {
        // Small helper to simplify comparison of component orders.
      #define ORDER(a, b, c, d) \
        ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

        // Get an uint out of the format components...
        uint formatOrder = ORDER (format.GetComponent (0),
          format.GetComponent (1),
          format.GetComponent (2),
          format.GetComponent (3));

        FormatTemplate const * formats = 0;
        int compCount = 0;
        GLenum targetFormat = 0, sourceFormat = 0, sourceType = 0;
        GLenum const* targetTable = 0;
        // ... so we can conveniently switch() for the order.
        switch (formatOrder)
        {
          case ORDER('r','g','b','a'):
            formats = formatsRGBA;
            sourceFormat = GL_RGBA;
            compCount = 4;
            targetTable = targetRGBA;
            break;
          case ORDER('r','g','b','x'):
            /* The "junk" byte has the same size as an alpha component but
               only the RGB ones will be used. */
            formats = formatsRGBA;
            /* Or RGBA? Tho if I [res] read the spec right using RGB as the 
               source format should just ignore the 4th component. */
            sourceFormat = GL_RGB;
            compCount = 4;
            targetTable = targetRGB;
            break;
          case ORDER('r','g','b',0):
            formats = formatsRGB;
            sourceFormat = GL_RGB;
            compCount = 3;
            targetTable = targetRGB;
            break;
          case ORDER('b','g','r','a'):
            formats = formatsRGBA;
            sourceFormat = GL_BGRA;
            compCount = 4;
            targetTable = targetRGBA;
            break;
          case ORDER('b','g','r','x'):
            formats = formatsRGBA;
            sourceFormat = GL_BGR; // Or BGRA?
            compCount = 4;
            targetTable = targetRGB;
            break;
          case ORDER('b','g','r',0):
            formats = formatsRGB;
            sourceFormat = GL_BGR;
            compCount = 3;
            targetTable = targetRGB;
            break;
          case ORDER('a','r','g','b'):
            formats = formatsARGB;
            sourceFormat = GL_BGRA;
            compCount = 4;
            targetTable = targetRGBA;
            break;
          case ORDER('a','b','g','r'):
            formats = formatsARGB;
            sourceFormat = GL_RGBA;
            compCount = 4;
            targetTable = targetRGBA;
            break;
          case ORDER('l',0,0,0):
            formats = formatsLum;
            sourceFormat = GL_LUMINANCE;
            compCount = 1;
            targetTable = targetLum;
            break;
          case ORDER('a','l',0,0):
            formats = formatsALum;
            sourceFormat = GL_LUMINANCE_ALPHA;
            compCount = 2;
            targetTable = targetALum;
            break;
        }

        if (!formats) return false;

        // Search for a matching, valid component size combo.
        const FormatTemplate* foundFormat = 0;
        while (formats->targetFmtIndex >= 0)
        {
          targetFormat = targetTable[formats->targetFmtIndex];
          sourceType = formats->srcType;
          GLenum newSourceFormat = sourceFormat;
          FixupFormatAndType (newSourceFormat, sourceType);
          bool match = FormatSupported (newSourceFormat, sourceType);
          for (int c = 0; match && (c < compCount); c++)
            match = formats->size[c] == format.GetComponentSize (c);
          if (match)
          {
            sourceFormat = newSourceFormat;
            break;
          }
          else
            targetFormat = 0;
          formats++;
        }

        if (targetFormat == 0) return false;

        // Return what's found.
        glFormat = TextureStorageFormat (targetFormat, 
          sourceFormat, sourceType);

        return true;
      #undef ORDER
      }
      break;
    case CS::StructuredTextureFormat::Special:
      {
        // just look up the hash with the special formats.
        TextureStorageFormat newFormat (specialFormats.Get (
          format.GetSpecial(), TextureStorageFormat ()));
        if (newFormat.targetFormat != 0) 
        {
          glFormat = newFormat;
          return true;
        }
      }
      break;
    default:
      break;
  }
  return false;
}

}
CS_PLUGIN_NAMESPACE_END(gl3d)
