/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

//
// Crystal Space crossbuild factorys.  Given a converter instance, extracts
// and constructs various
// Crystal Space structures from the converter, including frames,
// actions, sprite templates, and things.  The resulting structures
// can be used in a Crystal Space world.
// 

#ifndef __CROSSBLD_H__
#define __CROSSBLD_H__

#include "csutil/impexp.h"
#include "csengine/cssprite.h"
#include "csengine/triangle.h"

/**
 * The general cross builder interface.  All cross builders inherit from
 * this class, replacing the CrossBuild() method to build various
 * types of CS objects from source data
 */
class csCrossBuild_Factory
{
  public:
    /// Constructor.  By default you will probably not do much here.
    csCrossBuild_Factory();

    /// destructor.  Don't go off killing the converter, as you don't
    /// own it!
    virtual ~csCrossBuild_Factory();

    /**
     * call this function to actually construct whatever object it
     * is that you want.  This object could be a frame, sprite template,
     * thing, sector, etc...
     */
    virtual csBase *CrossBuild(converter& buildsource)=0;
};

/**
 * The sprite template factory makes a whole sprite template by
 * extracting all the frames from a converter and stuffing them
 * into a csSpriteTemplate object
 */
class csCrossBuild_SpriteTemplateFactory : public csCrossBuild_Factory
{
  public:
    /// Constructor.  There are currently no options
    csCrossBuild_SpriteTemplateFactory();

    /// Destructor.  Does not delete the sprite templates it has
    /// made, since they may be in use.  Delete them yourself when
    /// you are done with them.
    ~csCrossBuild_SpriteTemplateFactory();

    /// Makes a sprite template out of frames stored in the
    /// converter object
    /// if your compiler chokes on the uncommented version, try
    /// using the 'csBase' version instead
    csBase *CrossBuild(converter& buildsource);
    //csSpriteTemplate *CrossBuild(converter& buildsource);

  private:
    /// make a single new frame by extracting data from
    /// the converter object
    void Build_Frame(csSpriteTemplate& framesource, converter& buildsource);

    /// make a triangle mesh by extracting data from the
    /// converter data
    void Build_TriangleMesh(csSpriteTemplate& meshsource, converter& buildsource);
};

#endif // ifndef __CROSSBLD_H__

