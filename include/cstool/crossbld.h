/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
  
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

// Crystal Space crossbuild factorys. Given a converter instance, extracts
// and constructs various Crystal Space structures from the converter,
// including frames, actions, sprite templates, and things.
// The resulting structures can be used in a Crystal Space world.

#ifndef __CS_CROSSBLD_H__
#define __CS_CROSSBLD_H__

#include "cstool/impexp.h"

struct iSprite3DFactoryState;
struct iMeshObjectFactory;
struct iSpriteAction;
struct iSpriteFrame;
struct iThingState;
struct iTextureWrapper;
struct iEngine;
struct iImage;

/**
 * The general cross builder interface.  All cross builders inherit from
 * this class, replacing the two CrossBuild() methods to build various
 * types of CS objects from source data
 */
class csCrossBuild_Factory
{
protected:
  iSystem *System;

  iImage *LoadImage (UByte* iBuffer, ULong iSize, int iFormat);

public:
    /// Constructor.  By default you will probably not do much here.
    csCrossBuild_Factory(iSystem *sys);

    /**
     * Destructor.  Don't go off killing the converter, as you don't
     * own it!
     */
    virtual ~csCrossBuild_Factory();

    /**
     * Call this function to actually construct whatever object it
     * is that you want.  This object could be a frame, sprite template,
     * thing, sector, etc...
     */
    virtual void *CrossBuild(converter& buildsource) = 0;

    /**
     * This is another variant to override. It takes the object to construct
     * as a parameter.
     */
    virtual void CrossBuild(void* object, converter& buildsource) = 0;
};

/**
 * The sprite template factory makes a whole sprite template by
 * extracting all the frames from a converter and stuffing them
 * into a iMeshObjectFactory object.
 * Typical usage is done by combining this with the 'converter'
 * class.  Example, assuming the texture exists and is already
 * loaded:
 *
========================================================================
  // read in the model file
  converter * filedata = new converter;
  if (filedata->ivcon (filename) == ERROR)
  {
    Sys->Printf (CS_MSG_CONSOLE, "There was an error reading the data!\n");
    delete filedata;
    return;
  }

  // convert data from the 'filedata' structure into a CS sprite template
  csCrossBuild_SpriteTemplateFactory builder;
  iMeshObjectFactory *t = (iMeshObjectFactory *)builder.CrossBuild (*filedata);
  delete filedata;

  // add this sprite to the engine
  t->SetName(templatename);
  t->SetTexture (Sys->view->GetEngine ()->GetTextures (), txtname);

  Sys->view->GetEngine ()->sprite_templates.Push (t);
==========================================================================
 *
 */
class csCrossBuild_SpriteTemplateFactory : public csCrossBuild_Factory
{
  public:
    /// Constructor.  There are currently no options
    csCrossBuild_SpriteTemplateFactory(iSystem *sys);

    /**
     * Destructor.  Does not delete the sprite templates it has
     * made, since they may be in use.  Delete them yourself when
     * you are done with them.
     */
    ~csCrossBuild_SpriteTemplateFactory();

    /**
     * Makes a sprite template out of frames stored in the
     * converter object
     */
    void* CrossBuild(converter&);

    /**
     * Makes a sprite template out of frames stored in the
     * converter object
     */
    void CrossBuild(void*, converter&);

  private:
    /**
     * make a single new frame by extracting data from
     * the converter object
     */
    void Build_Frame(iSprite3DFactoryState*, converter&);

    /**
     * make a triangle mesh by extracting data from the
     * converter data
     */
    void Build_TriangleMesh(iSprite3DFactoryState*, converter&);
};


/**
 * The thing template factory makes a whole thing template by
 * extracting the first frame from a converter and stuffing it
 * into a iThingState object.
 * Note that the converted thing will be using gouraud shading.
 */
class csCrossBuild_ThingTemplateFactory : public csCrossBuild_Factory
{
  public:
    /// Constructor.  There are currently no options
    csCrossBuild_ThingTemplateFactory(iSystem *sys);

    /**
     * Destructor.  Does not delete the Thing templates it has
     * made, since they may be in use.  Delete them yourself when
     * you are done with them.
     */
    ~csCrossBuild_ThingTemplateFactory();

    /**
     * Makes a thing template out of the first frame stored in the
     * converter object
     */
    void* CrossBuild(converter&);

    /**
     * Makes a thing template out of the first frame stored in the
     * converter object
     */
    void CrossBuild(void*, converter&);

  private:
    /**
     * Add all vertices to the thing template.
     */
    void Add_Vertices (iThingState*, converter&);

    /**
     * Make triangle mesh by extracting data from the
     * converter data
     */
    void Build_TriangleMesh (iThingState*, converter&);
};

/**
 * The quake2 build factory imports a geometry file (assumed to be a 
 * quake 2 MD2 format file), imports the textures associated with that
 * file as 'skins', and maps all the frames into Crystal space actions
 * corresponding to Quake 2 equivalents, i.e., the 'stand###' frames
 * are mapped to the CS action 'stand' and so on.  Skins are given
 * names consisting of the sprite name (NOT the file name) plus the
 * texture filename, separated
 * by a dash, such as 'bobafett-ROTJ_Fett' which represents the
 * 'ROTJ_Fett.pcx' skin packed with the 'bobafett' model.
 * Typical usage, assuming 'bob.zip' is a zip file holdin the
 * MD2 file and skins:
=================================================
 filename = 'bob.zip'
 csCrossBuild_Quake2Importer importer;
 iMeshObjectFactory *newtemplate =
   importer.ImportQuake2Pack(filename,'bob',engine);
 iSprite3DFactoryState* fstat = SCF_QUERY_INTERFACE (newtemplate, iSprite3DFactoryState);
 fstate->SetMaterial(bobMaterial);
 iMeshObject* obj = newtemplate->NewInstance ();
=================================================
 *
 */
class csCrossBuild_Quake2Importer
{
  private:
    // the system driver
    iSystem *System;

    // VFS to use.  May be the default, or it may have
    // .zip files mounted containing the sprites
    iVFS *localVFS;

    // find a MD2 geometry file, load and return it, with all standard
    // actions already created
    iMeshObjectFactory* Import_Quake2MeshObjectFactory(iFile*) const;

    // find textures in a directory and add to the engine.  the texture names
    // are made by concatinating the modelname passed in and the
    // texture file name
    iTextureWrapper* Import_Quake2Textures(char const* skinpath,
      char const* modelname, iEngine*) const;

    // given a prefix representing an action name, make a csSpriteAction
    // by concatinating all the frames that start with that prefix
    iSpriteAction* Make_NamedAction(iSprite3DFactoryState*,
      char const *prefixstring, int delay) const;

    // build all the standard quake 2 actions, assuming the sprite
    // has frames with names that start with the proper action names
    void Build_Quake2Actions(iSprite3DFactoryState*) const;

    /// Dummy asignment operator to get rid of a MSVC warning.
    void operator= (csCrossBuild_Quake2Importer const&) {};

  public:
    /**
     * Constructor needs a VFS to map from the WAD, sprite respository, or
     * whatever, to files.  If no VFS is supplied it will use the system VFS
     */
    csCrossBuild_Quake2Importer(iSystem *sys);
    csCrossBuild_Quake2Importer(iSystem *sys, iVFS *specialVFS);
    ~csCrossBuild_Quake2Importer();

    /**
     * Import Quake 2 data by reading in a specified MD2 file.  Any skins are
     * loaded by looking for image files in the directory 'skinpath'.  If
     * 'skinpath' is NULL, looks in the same directory as the geometry file.
     */
    iMeshObjectFactory *Import_Quake2File(char const* md2filebase,
      char const* skinpath, char const* modelname, iEngine*) const;
};

#endif // __CS_CROSSBLD_H__
