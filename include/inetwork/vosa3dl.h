/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
    Copyright (C) 2004 by Peter Amstutz <tetron@interreality.org>
    Written by Peter Amstutz <tetron@interreality.org>

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

#ifndef __CS_INETWORK_VOSA3DL_H__
#define __CS_INETWORK_VOSA3DL_H__

#include "csutil/scf.h"

#include "csutil/set.h"

struct iMeshWrapper;
struct iProgressMeter;
struct iRigidBody;
struct iSector;

/** @file
    This defines the plugin interface for the Crystal Space
    Virtual Object System (VOS) Abstract 3D Layer plugin (A3DL).  VOS
    is an object-oriented distributed network architechture; A3DL is a
    set of extensions to VOS to describe 3D scenes.  This plugin makes
    it easy to access 3D virtual worlds described using A3DL with the
    VOS protocol and loads those worlds into the Crystal Space engine.
    For more information about VOS, see http://interreality.org.  You
    will need the download the VOS software from in order to use this
    plugin.

    @note This is very much a work in progress.  You can access the
    underlying VOS API using QUERY_INTERFACE to get iVosApi (defined
    in ivosapi.h).  Something that we'd like to do (but this does not
    currently support) is to be able to take the current CS engine
    state and export to remote clients using VOS, and generally wrap
    VOS enough that you could take a Crystal Space app that doesn't
    know anything about networking and sprinkle it with the magic VOS
    plugin powder and make it multiuser... :-)
*/


SCF_VERSION (iVosObject3D, 0, 1, 1);

/** This interface bridges between a VOS 3D object and the Crystal
    Space mesh wrapper created for that object.
    @bug presently this isn't very useful since nothing yet returns this
    interface.  Obviously that will change as the iVosSector interface
    is fleshed out (or an alternate interface is introduced instead of this...).
*/
struct iVosObject3D : public iBase
{
  /** Get the iMeshWrapper for this Object3D. */
  virtual csRef<iMeshWrapper> GetMeshWrapper() = 0;

  /** Get the iRigidBody collider for this Object3D. This can be used to
   *  control the forces o the object - useful for avatars
   *
   *  This will return no object if there is no iDynamicsSystem registered in
   *  the object registry
   */
  virtual csRef<iRigidBody> GetCollider() = 0;
};




SCF_VERSION (iVosSector, 0, 3, 0);

/** This interface bridges between a VOS sector and a Crystal Space
    sector. */
struct iVosSector : public iBase
{
  /** Begin loading this sector in the background.  Network activity
      occurs in another thread, so this method returns immediately.
      An event is posted to the global event queue when the download is complete.
      @param progress if supplied, this will be called back (in the CS thread)
      periodically to indicate download progress
  */
  virtual void Load(iProgressMeter* progress = 0) = 0;

  /** Get the Crystal Space iSector for this sector.  This will be
      empty until Load() is called. */
  virtual csRef<iSector> GetSector() = 0;

  /** Get the list of object3ds which have been loaded into this sector. This
   *  list will change in size as objects are loaded and removed - does not
   *  represent the list of objects in the A3DL sector
   */
  virtual const csSet< csPtrKey<iVosObject3D> > &GetObject3Ds() = 0;
};




SCF_VERSION (iVosA3DL, 0, 1, 1);

/** This is the initial component you retrieve from the registry to
    access the VOS A3DL plugin.  Here's how to get it:
    @code
    csInitializer::RequestPlugins (object_reg,
      ...
      CS_REQUEST_PLUGIN("crystalspace.network.vos.a3dl", iVosA3DL),
      ...
      CS_REQUEST_END)

    ...

    csRef<iVosA3DL> vosa3dl = CS_QUERY_REGISTRY (object_reg, iVosA3DL);
    @endcode

 */
struct iVosA3DL : public iBase
{
  /** Get a VOS sector given a VOS URL (such as
      "vop://interreality.org/world") for a sector object.  Doesn't
      load it (call iVosSector::Load() to do that).

      @bug no way (yet) to tell you if the sector doesn't exist
   */
  virtual csRef<iVosSector> GetSector(const char*) = 0;
};

#endif
