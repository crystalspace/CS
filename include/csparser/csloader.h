/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Ivan Avramovic <ivan@avramovic.com>
  
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

#ifndef _CSLOADER_H_
#define _CSLOADER_H_

#include "csengine/polyset.h"
#include "csparser/loadinfo.h"

struct iImage;
class csSkeletonLimb;
class csPolygonTemplate;
class csThingTemplate;
class csPolyPlane;
class csPolyTxtPlane;
class csCollection;
class csStatLight;
class csThing;
class csWorld;
class LanguageLayer;
class csCurveTemplate;
class csSoundDataObject;
class csSpriteTemplate;
class csSprite3D;
class csKeyValuePair;
class csMapNode;

/**
 * The loader for Crystal Space worlds.
 */
class csLoader
{
  /// Parse a matrix definition
  static bool load_matrix (char* buf, csMatrix3 &m);
  /// Parse a vector definition
  static bool load_vector (char* buf, csVector3 &v);
  /// Parse a polygon plane definition and return a new object
  static csPolyTxtPlane* load_polyplane (char* buf, char* name = NULL);
  /// Parse a collection definition and return a new object
  static csCollection* load_collection (char* name, char* buf);
  /// Parse a static light definition and return a new object
  static csStatLight* load_statlight (char* buf);
  /// Parse a key definition and return a new object
  static csKeyValuePair* load_key (char* buf, csObject* pParent);
  /// Parse a map node definition and return a new object
  static csMapNode* load_node (char* name, char* buf, csSector* sec);
  /// Default handler for objects derived from polygon set
  static csPolygonSet& ps_process (csPolygonSet& ps, PSLoadInfo& info, int cmd,
    char* name, char* params);
  /// Parse the definition for a skydome and create the corresponding objects
  static void skydome_process (csSector& sector, char* name, char* buf,
    csTextureHandle* texture);
  /// Parse the terrain engine's parameters
  static void terrain_process (csSector& sector, char* name, char* buf,
    csTextureHandle* texture);
  /// Load a sixface (i.e. box) definition (obsolete, should not be used)
  static csThing* load_sixface (char* name, char* buf, csSector* sec);
  /// Parse the definition for a thing and create a thing object
  static csThing* load_thing (char* name, char* buf, csSector* sec);
  /// Parse a 3D polygon definition and return a new object
  static csPolygon3D* load_poly3d (char* polyname, char* buf,
    csTextureHandle* default_texture, float default_texlen,
    CLights* default_lightx, csSector* sec, csPolygonSet* parent);
  /// Parse a Bezier surface definition and return a new object
  static csCurve* load_bezier (char* polyname, char* buf,
    csTextureHandle* default_texture, float default_texlen,
    CLights* default_lightx, csSector* sec, csPolygonSet* parent);

  /// Load a image and return an iImage object
  static iImage* load_image(const char* name);
  /// Parse and load a single texture
  static void txt_process (char *name, char* buf, const char* prefix = NULL);
  /// Parse polygon template definition and return a new object
  static csPolygonTemplate* load_ptemplate (char* ptname, char* buf,
    csTextureHandle* default_texture, float default_texlen,
    csThingTemplate* parent);
  /// Parse a thing template definition and return a new object
  static csThingTemplate* load_thingtpl (char* tname, char* buf);
  /// Parse a Bezier surface definition and return a new object
  static csCurveTemplate* load_beziertemplate (char* ptname, char* buf,
    csTextureHandle* default_texture, float default_texlen,
    csThingTemplate* parent);

  /// Create a thing template from a sixface (obsolete)
  static csThingTemplate* load_sixtpl(char* tname,char* buf);
  /// Parse a room definition (obsolete)
  static csSector* load_room (char* secname, char* buf);
  /// Parse a sector definition and return a new object
  static csSector* load_sector (char* secname, char* buf);

  /// Load a definition for an old-style dynamic light (obsolete)
  static void load_light (char* name, char* buf);

  /// Load a sound and return a new object
  static csSoundDataObject* load_sound (char* name, const char* filename);

  /// Load a skeleton part.
  static bool LoadSkeleton (csSkeletonLimb* limb, char* buf, bool is_connection);

  /// Load the sprite template from the world file.
  static bool LoadSpriteTemplate (csSpriteTemplate* stemp, char* buf);

  /**
   * Load the sprite from the world file.
   */
  static bool LoadSprite (csSprite3D* spr, char* buf);

  /**
   * Load sounds from a SOUNDS(...) argument.
   * If 'ar' is given optionally load from that archive as well.
   * This function is normally called automatically by the parser.
   */
  static bool LoadSounds (char* buf);

  /**
   * Load all the texture descriptions from the world file
   * (no actual images). If a prefix is given, all texture names will be
   * prefixed with the corresponding string.
   */
  static bool LoadTextures (char* buf, const char* prefix = NULL);

  /**
   * Load a library into given world.<p>
   * A library is just a world file that contains just sprite templates,
   * thing templates, sounds and textures.
   */
  static bool LoadLibrary (char* buf);

  /// World from a memory buffer
  static bool LoadWorld (char* buf);

public:
  /// Load file into a world.
  static bool LoadWorldFile (csWorld* world, LanguageLayer* layer, const char* filename);

  /// Load library from a VFS file
  static bool LoadLibraryFile (csWorld* world, const char* filename);

  /**
   * Load a texture and add it to the world.
   * The texture will be registered for 3d use only.
   */
  static csTextureHandle* LoadTexture (csWorld* world, const char* name,
    const char* fname);
};

#endif
