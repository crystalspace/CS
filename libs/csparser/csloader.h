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

class csPolygonTemplate;
class csThingTemplate;
class csTextureList;
class csPolyPlane;
class csCollection;
class csStatLight;
class csThing;
class csLibrary;
class ImageFile;
class csWorld;
class LanguageLayer;
class Archive;
class csCurveTemplate;
class csSoundBufferObject;
class csSpriteTemplate;
class csSprite3D;

enum { kTokenPSetVertex = 1, kTokenPSetPolygon, kTokenPSetBezier,
       kTokenPSetTexNr, kTokenPSetCircle,
       kTokenPSetTexlen, kTokenPSetTrigger, kTokenPSetActivate,
       kTokenPSetLightX, kTokenPSetBsp, kTokenPSetFog,
       kTokenPSetLast };

/**
 * The loader for CS worlds.
 */
class CSLoader
{
protected:
  ///
  class LoadStat
  {
    public:
    ///
    static int polygons_loaded;
    ///
    static int portals_loaded;
    ///
    static int sectors_loaded;
    ///
    static int things_loaded;
    ///
    static int lights_loaded;
    ///
    static int curves_loaded;
    ///
    static int sprites_loaded;
    ///
    static void Init()
    {
      polygons_loaded = portals_loaded = sectors_loaded = 0;
      things_loaded = lights_loaded = 0;
      curves_loaded = sprites_loaded = 0;
    }
  };

public:
  ///
  static csMatrix3 load_matrix (char* buf);
  ///
  static csVector3 load_vector (char* buf);
  ///
  static csPolyPlane* load_polyplane (char* buf, char* name = NULL);
  ///
  static csCollection* load_collection (char* name, csWorld* world, char* buf);
  ///
  static csStatLight* load_statlight (char* buf);
  ///
  static csPolygonSet& ps_process (csPolygonSet& ps, PSLoadInfo& info, int cmd,
                                 char* name, char* params);
  ///
  static void skydome_process (csSector& sector, char* name, char* buf,
			       csTextureHandle* texture);
  ///
  static csThing* load_sixface (char* name, csWorld* w, char* buf,
                              csTextureList* textures, csSector* sec);
  ///
  static csThing* load_thing (char* name, csWorld* w, char* buf, 
                            csTextureList* textures, csSector* sec);
  ///
  static csPolygon3D* load_poly3d (char* polyname, csWorld* w, char* buf, 
        csTextureList* textures, csTextureHandle* default_texture, float default_texlen,
        CLights* default_lightx, csSector* sec, csPolygonSet* parent);

  static csCurve* load_bezier (char* polyname, csWorld* w, char* buf, 
        csTextureList* textures, csTextureHandle* default_texture, float default_texlen,
        CLights* default_lightx, csSector* sec, csPolygonSet* parent);

  ///
  static ImageFile* load_image(char* name, csWorld* w, Archive* ar = NULL);
  ///
  static void txt_process (csTextureHandle* txt_handle, char* buf);
  ///
  static csPolygonTemplate* load_ptemplate (char* ptname, char* buf,
        csTextureList* textures, csTextureHandle* default_texture, float default_texlen,
        csThingTemplate* parent);
  ///
  static csThingTemplate* load_thingtpl(char* tname, char* buf,
                                      csTextureList* textures);
  ///
  static csCurveTemplate* load_beziertemplate (char* ptname, char* buf, 
					   csTextureList* textures, csTextureHandle* default_texture, float default_texlen,
					   csThingTemplate* parent);

  ///
  static csThingTemplate* load_sixtpl(char* tname,char* buf,csTextureList* textures);
  ///
  static csSector* load_room (char* secname, csWorld* w, char* buf, 
                            csTextureList* textures);
  ///
  static csSector* load_sector (char* secname, csWorld* w, char* buf, 
                              csTextureList* textures);

  ///
  static void load_light (char* name, char* buf);

  /// Load a library definition.
  static bool load_library_def (csLibrary* lib, csWorld* w, char* file, csTextureList* textures);
  /// Load a library definition.
  static bool load_library_def (csLibrary* lib, csWorld* w, Archive* ar, char* buf, csTextureList* textures);

  ///
  static csSoundBufferObject* load_sound (char* name, csWorld* w, Archive* ar);

  /// Load data into a world.
  static bool LoadWorld (csWorld* world, LanguageLayer* layer, char* buf);

  /// Load file into a world.
  static bool LoadWorldFile (csWorld* world, LanguageLayer* layer, char* filename);

  /**
   * Load all the texture descriptions from the world
   * file (no actual images).
   * If the optional 'ar' is given, that archive is checked
   * first before the others are checked.
   */
  static bool LoadTextures (csTextureList* textures, char* buf, csWorld* world, Archive* ar = NULL);

  /**
   * Load a library and add it to the world.
   * the library is automatically added to the world, if loading was successful.
   *
   * Parameters:
   * name:   a symbolic name for that library. usefull for later references
   * fname:  name of the file that contains the library (for instance "standard.zip")
   * return: a pointer to the loaded library, or NULL, if loading was not possible.
   */
  static csLibrary* LoadLibrary (csWorld* world, char* name, char* fname);

  /**
   * Load a texture and add it to the world.
   * The texture will be registered for 3d use only.
   */
  static csTextureHandle* LoadTexture (csWorld* world, char* name, char* fname, Archive* ar = NULL);

  /**
   * Load sounds from a SOUNDS(...) argument.
   * If 'ar' is given optionally load from that archive as well.
   * This function is normally called automatically by the parser.
   */
  static bool LoadSounds (csWorld* world, char* buf, Archive* ar = NULL);

  /// Load the sprite template from the world file.
  static bool LoadSpriteTemplate (csSpriteTemplate* stemp, char* buf, csTextureList* textures);

  /**
   * Load the sprite from the world file.
   */
  static bool LoadSprite (csSprite3D* spr, csWorld* w, char* buf, csTextureList* textures);

};

#endif
