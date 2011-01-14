/*
    Copyright (C) 2011 by Frank Richter

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

#ifndef __TEXTURECLASS_H__
#define __TEXTURECLASS_H__

#include "csutil/hash.h"

struct iConfigFile;

/**
 * Encapsulate a renderer texture class
 * (the properties relevant for basemapgen, anyway).
 */
class TextureClass
{
  bool sharpenMips;
  bool sharpenPrecomputed;
public:
  TextureClass () : sharpenMips (true), sharpenPrecomputed (false) {}
  
  /// Parse texture class properties from config
  void Parse (iConfigFile* config, const char* prefix);
  
  bool GetSharpenMips() const { return sharpenMips; }
  bool GetSharpenPrecomputed() const { return sharpenPrecomputed; }
};

/**
 * Collect + manage texture classes.
 */
class TextureClassManager
{
  // Texture class used for "unknown" classes
  TextureClass unknownClass;
  
  csHash<TextureClass, csString> classes;
public:
  void Parse (iConfigFile* config);
  
  /**
   * Get properties of a texture class.
   * Falls back to "default" class if class name is 0.
   */
  const TextureClass& GetTextureClass (const char* className);
};

#endif // __TEXTURECLASS_H__
