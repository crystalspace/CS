/*
    Copyright (C) 2000 by Norman Kramer
    Copyright (C) 2000 by W.C.A. Wijngaards
    original unplugged code and fonts by Andrew Zabolotny

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

#ifndef __CS_CSFONT_H__
#define __CS_CSFONT_H__

#include "ivideo/fontserv.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/hash.h"
#include "csutil/hashhandlers.h"
#include "csutil/refarr.h"
#include "csutil/parray.h"
#include "csutil/weakref.h"
#include "iutil/plugin.h"
#include "iutil/databuff.h"

struct iObjectRegistry;
struct iPluginManager;
class csDefaultFontServer;

#define GLYPH_INDEX_UPPER_SHIFT	    8
#define GLYPH_INDEX_LOWER_COUNT	    256
#define GLYPH_INDEX_LOWER_MASK	    0xff

class csParasiticDataBuffer : public iDataBuffer
{
  csRef<iDataBuffer> parentBuffer;
  size_t size;
  uint8* data;
public:
  SCF_DECLARE_IBASE;

  csParasiticDataBuffer (iDataBuffer* parent, size_t offs,
    size_t size = (size_t)~0)
  {
    SCF_CONSTRUCT_IBASE(0);
    parentBuffer = parent;
    data = parent->GetUint8 () + offs;
    if (size == (size_t)~0)
      csParasiticDataBuffer::size = parent->GetSize () - offs;
    else
      csParasiticDataBuffer::size = size;
  }
  virtual ~csParasiticDataBuffer ()
  {
    SCF_DESTRUCT_IBASE();
  }

  /// Query the buffer size
  virtual size_t GetSize () const
  { return size; }
  /// Get the buffer as an abstract pointer
  virtual char* GetData () const
  { return (char*)data; }
  /// Get the buffer as an (char *) pointer
  inline char *operator * () const
  { return (char *)GetData (); }
  /// Get as an int8 *
  inline int8* GetInt8 ()
  { return (int8 *)GetData (); }
  /// Get as an uint8 *
  inline uint8* GetUint8 ()
  { return (uint8 *)GetData (); }
};


/**
 * Bitmap font
 */
class csDefaultFont : public iFont
{
public:
  struct CharRange
  {
    utf32_char startChar;
    int charCount;
  };

  struct Glyph
  {
    size_t bitmapOffs;
    size_t bitmapSize;
    size_t alphaOffs;
    size_t alphaSize;
    csGlyphMetrics gMetrics;
    csBitmapMetrics bMetrics;
    csBitmapMetrics aMetrics;

    Glyph () { bitmapSize = alphaSize = (size_t)~0; }  
  };

  /**
   * Array of a number of glyphs.
   * To quickly access a specific glyph, basically a two-dimensional
   * array is used. This is the "second" dimension, a "plane". A plane
   * always contains #GLYPH_INDEX_LOWER_COUNT number of glyphs.
   */
  struct PlaneGlyphs
  {
    /// Pointer to glyph information
    Glyph entries[GLYPH_INDEX_LOWER_COUNT];
  };

  class PlaneGlyphElementHandler : public csArrayElementHandler<PlaneGlyphs*>
  {
  public:
    static void Construct (PlaneGlyphs** address, PlaneGlyphs* const& src)
    {
      *address = 0;
    }

    static void Destroy (PlaneGlyphs** address)
    {
    }

    static void InitRegion (PlaneGlyphs** address, int count)
    {
      memset (address, 0, count * sizeof (PlaneGlyphs*));
    }
  };

  /**
   * Array of a number of glyphs.
   * This is the "first" dimension of the glyphs array, and consists of
   * a variable number of "planes". If a plane doesn't contain a glyph,
   * it doesn't take up memory.
   */
  csArray<PlaneGlyphs*, PlaneGlyphElementHandler> Glyphs;

  char *Name;
  int Ascent, Descent;
  int MaxWidth;
  int Height;
  csRef<iDataBuffer> bitData;
  csRef<iDataBuffer> alphaData;
  csRef<csDefaultFontServer> Parent;
  csRefArray<iFontDeleteNotify> DeleteCallbacks;

  SCF_DECLARE_IBASE;

  /// Create the font object
  csDefaultFont (csDefaultFontServer *parent, const char *name, 
    CharRange* glyphs, int height, int ascent, int descent,
    csGlyphMetrics* gMetrics,
    iDataBuffer* bitmap, csBitmapMetrics* bMetrics,
    iDataBuffer* alpha = 0, csBitmapMetrics* aMetrics = 0);

  /// Destroy the font object
  virtual ~csDefaultFont ();

  /**
   * Set the size for this font.
   * All other methods will change their behaviour as soon as you call
   * this method; but not all font managers supports rescalable fonts
   * in which case this method will be unimplemented.
   */
  virtual void SetSize (int iSize);

  /**
   * Query current font size. If server does not support rescalable
   * fonts, this method returns 0.
   */
  virtual int GetSize ();

  /**
   * Return the maximum width and height of a single glyph.
   * Return -1 if it could not be determined.
   */
  virtual void GetMaxSize (int &oW, int &oH);

  /**
   * Return the metrics of a glyph.
   */
  virtual bool GetGlyphMetrics (utf32_char c, csGlyphMetrics& metrics);

  /**
   * Return a pointer to a bitmap containing a rendered character.
   */
  virtual csPtr<iDataBuffer> GetGlyphBitmap (utf32_char c,
    csBitmapMetrics& metrics);

  /**
   * Return a pointer to the alpha bitmap for rendered character.
   */
  virtual csPtr<iDataBuffer> GetGlyphAlphaBitmap (utf32_char c,
    csBitmapMetrics& metrics);

  /**
   * Return the width and height of text written with this font.
   */
  virtual void GetDimensions (const char *text, int &oW, int &oH);
  virtual void GetDimensions (const char *text, int &oW, int &oH, int &desc);

  /**
   * Determine how much characters from this string can be written
   * without exceeding given width (in pixels)
   */
  virtual int GetLength (const char *text, int maxwidth);

  /**
   * Add a call-on-font-delete callback routine.
   */
  virtual void AddDeleteCallback (iFontDeleteNotify* func);

  /**
   * Remove a font delete notification callback.
   */
  virtual bool RemoveDeleteCallback (iFontDeleteNotify* func);

  /**
   * Get the font's descent in pixels.
   */
  virtual int GetDescent (); 

  /**
   * Get the font's ascent in pixels.
   */
  virtual int GetAscent (); 

  virtual bool HasGlyph (utf32_char c); 
};

/**
 * Default font server.
 */
class csDefaultFontServer : public iFontServer
{
private:
  iObjectRegistry* object_reg;

  // A list of csDefaultFont pointers.
  //csRefArray<csDefaultFont> fonts;
  csHash<csDefaultFont*, csStrKey, csConstCharHashKeyHandler> fonts;

  /// read a font file from vfs
  csDefaultFont *ReadFontFile(const char *file);

public:
  SCF_DECLARE_IBASE;

  /// Create the plugin object
  csDefaultFontServer (iBase *pParent);
  /// destroy it
  virtual ~csDefaultFontServer ();

  /**
   * Load a font by name.
   * Returns a new iFont object or 0 on failure.
   */
  virtual csPtr<iFont> LoadFont (const char *filename,
    int size = 10);

  /// Called by child fonts to be added to font registry
  void NotifyCreate (csDefaultFont *font);

  /// This function is called by iFont objects when they are destroyed
  void NotifyDelete (csDefaultFont *font);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csDefaultFontServer);
    virtual bool Initialize(iObjectRegistry* p);
  } scfiComponent;
  friend struct eiComponent;
};

#endif // __CS_CSFONT_H__

