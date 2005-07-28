/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_CANVAS_FONTCACHE_H__
#define __CS_CSPLUGINCOMMON_CANVAS_FONTCACHE_H__

/**\file
 * Font glyph cache base
 */

#include "csextern.h"

#include "csutil/blockallocator.h"
#include "csutil/csunicode.h"
#include "csutil/set.h"
#include "ivideo/fontserv.h"


/**
 * \addtogroup plugincommon
 * @{ */

#define GLYPH_INDEX_UPPER_SHIFT	    9
#define GLYPH_INDEX_LOWER_COUNT	    512
#define GLYPH_INDEX_LOWER_MASK	    0x1ff

#define RELEVANT_WRITE_FLAGS	    CS_WRITE_NOANTIALIAS

/**
 * A cache for font glyphs. 
 * It is intended as a baseclass, which a canvas extends to store glyphs in
 * a canvas-dependent way. It provides facilities to quickly locate data
 * associated with a specific glyph of a specific font, as well as means
 * to manage glyphs if only a limited space to store them is present.
 */
class CS_CRYSTALSPACE_EXPORT csFontCache
{
public:
  struct KnownFont;
  /**
   * Some basic data associated with a glyph.
   */
  struct GlyphCacheData
  {
    /// Font
    KnownFont* font;
    /// Glyph
    utf32_char glyph;
    /// Glyph metrics
    csGlyphMetrics glyphMetrics;
    /// Does this font have this glyph?
    bool hasGlyph;
    /// Glyph flags.
    uint flags;
  };

protected:
  /**
   * An entry in the LRU list.
   */
  struct LRUEntry
  {
    /// Next entry
    LRUEntry* next;
    /// Previous entry
    LRUEntry* prev;

    /// Cache data associated with font.
    GlyphCacheData* cacheData;
  };
  /// First entry in LRU list
  LRUEntry* head;
  /// Last entry in LRU list
  LRUEntry* tail;

  /// Allocator for LRU list entries
  csBlockAllocator<LRUEntry> LRUAlloc;

  /**
   * Array of a number of glyphs.
   * To quickly access a specific glyph, basically a two-dimensional
   * array is used. This is the "second" dimension, a "plane". A plane
   * always contains #GLYPH_INDEX_LOWER_COUNT number of glyphs.
   */
  struct PlaneGlyphs
  {
    /// Pointer to glyph information
    LRUEntry* entries[GLYPH_INDEX_LOWER_COUNT];
    int usedGlyphs;

    PlaneGlyphs ()
    {
      memset (entries, 0, sizeof (entries));
      usedGlyphs = 0;
    }
  };

  class PlaneGlyphElementHandler : public csArrayElementHandler<PlaneGlyphs*>
  {
  public:
    static void Construct (PlaneGlyphs** address, PlaneGlyphs* const& src)
    {
      *address = src;
    }

    static void Destroy (PlaneGlyphs** /*address*/)
    {
    }

    static void InitRegion (PlaneGlyphs** address, size_t count)
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
  typedef csArray<PlaneGlyphs*, PlaneGlyphElementHandler> PlaneGlyphsArray;
public:
  /**
   * A font known to the cache.
   */
  struct KnownFont
  {
    iFont* font;
    /// The font size this font was cached for
    float fontSize;
    PlaneGlyphsArray planeGlyphs;
  };
  /// the current clipping rect
  int ClipX1, ClipY1, ClipX2, ClipY2;
protected:

  /// Array of known fonts.
  csArray<KnownFont*> knownFonts;
  csSet<csPtrKey<KnownFont> > purgeableFonts;

  /// Find an LRU entry for a specific font/glyph pair.
  LRUEntry* FindLRUEntry (KnownFont* font, utf32_char glyph);
  /// Find an LRU entry for a specific cache data.
  LRUEntry* FindLRUEntry (GlyphCacheData* cacheData);

  static int KnownFontArrayCompareItems (KnownFont* const& item1, 
    KnownFont* const& item2);
  static int KnownFontArrayCompareToKey (KnownFont* const& item1, 
    iFont* const& item2);
  static csArrayCmp<KnownFont*,iFont*> KnownFontArrayKeyFunctor(iFont* f)
    { return csArrayCmp<KnownFont*,iFont*>(f, KnownFontArrayCompareToKey); }

  /// Cache canvas-dependent information for a specific font/glyph pair.
  virtual GlyphCacheData* InternalCacheGlyph (KnownFont* font,
    utf32_char glyph, uint flags);
  /// Uncache canvas-dependent information.
  virtual void InternalUncacheGlyph (GlyphCacheData* cacheData);

  /// Store glyph-specific information, but omit some safety checks.
  GlyphCacheData* CacheGlyphUnsafe (KnownFont* font, 
    utf32_char glyph, uint flags);
  /// Fill the basic cache data.
  void SetupCacheData (GlyphCacheData* cacheData,
    KnownFont* font, utf32_char glyph, uint flags);

  /// Add a glyph to the cache.
  void AddCacheData (KnownFont* font, utf32_char glyph, GlyphCacheData* cacheData);
  /// Remove a glyph from the cache. @@@ Does not update PlaneGlyphs!
  void RemoveCacheData (GlyphCacheData* cacheData);
  /// Remove a glyph from the cache.
  void RemoveLRUEntry (LRUEntry* entry);
  /// Request cached data for a glyph of a known font.
  GlyphCacheData* InternalGetCacheData (KnownFont* font, utf32_char glyph);

  /**
   * Font deletion callback
   */
  struct FontDeleteNotify : public iFontDeleteNotify
  {
    csFontCache* cache;
    SCF_DECLARE_IBASE;
    
    FontDeleteNotify (csFontCache* cache);
    virtual ~FontDeleteNotify ();

    virtual void BeforeDelete (iFont* font);
  };
  FontDeleteNotify* deleteCallback;

  void CleanupCache ();
public:
  csFontCache ();
  virtual ~csFontCache ();
  
  /// Store glyph-specific information.
  GlyphCacheData* CacheGlyph (KnownFont* font, utf32_char glyph,
    uint flags);
  /// Uncache cached glyph data.
  void UncacheGlyph (GlyphCacheData* cacheData);

  /// Request whether a font is known already.
  KnownFont* GetCachedFont (iFont* font);
  /// Set up stuff to cache glyphs of this font.
  KnownFont* CacheFont (iFont* font);
  /// Uncache this font.
  void UncacheFont (iFont* font);
  /// Request cached data for a glyph of a known font.
  GlyphCacheData* GetCacheData (KnownFont* font, utf32_char glyph, 
    uint flags);
  /// Get cached data for the least used glyph.
  GlyphCacheData* GetLeastUsed ();

  /// Delete empty PlaneGlyphs from known fonts
  void PurgeEmptyPlanes ();

  void SetClipRect (int x1, int y1, int x2, int y2)
  { 
    ClipX1 = x1; ClipY1 = y1; ClipX2 = x2; ClipY2 = y2; 
  }

  /**
   * Draw a string.
   */
  virtual void WriteString (iFont *font, int x, int y, int fg, int bg, 
    const utf8_char* text, uint flags);
};

/** @} */

#endif // __CS_CSPLUGINCOMMON_CANVAS_FONTCACHE_H__
