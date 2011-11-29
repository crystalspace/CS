/*
    Copyright (C) 2011 by Michael Gist

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

#ifndef __CS_LOADER_LOADER_H__
#define __CS_LOADER_LOADER_H__

#include <csutil/scf_implementation.h>
#include <iutil/comp.h>

#include "resourcemanager.h"

CS_PLUGIN_NAMESPACE_BEGIN(csloader)
{

class FileLoader
  : public scfImplementationExt1<FileLoader, ResourceManager, iComponent>
{
public:
  FileLoader (iBase* parent);
  virtual ~FileLoader ();

  // iResourceManager
  virtual csPtr<iLoadingResource> Get (CS::Resource::TypeID type, const char* name);

  // iComponent
  virtual bool Initialize (iObjectRegistry* obj_reg);

protected:
  void Process ();

private:
  class LEventHandler : public scfImplementation1<LEventHandler, 
    iEventHandler>
  {
  public:
    LEventHandler(FileLoader* parent, csEventID ProcessPerFrame)
      : scfImplementationType (this), parent (parent),
      ProcessPerFrame (ProcessPerFrame)
    {
    }

    virtual ~LEventHandler()
    {
    }

    bool HandleEvent(iEvent& Event)
    {
      if(Event.Name == ProcessPerFrame)
      {
        parent->Process ();
      }

      return false;
    }

    CS_EVENTHANDLER_PHASE_LOGIC("crystalspace.loader.file")

  private:
    FileLoader* parent;
    csEventID ProcessPerFrame;
  };

  csRef<iEventQueue> eventQueue;
  csRef<iEventHandler> eventHandler;

  csRefArray<iLoadingResource> loadingFiles;
};

}
CS_PLUGIN_NAMESPACE_END(loaders)

#endif // __CS_LOADER_LOADER_H__
