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

#include "cssysdef.h"
#include "fileloader.h"

using namespace CS::Resource;

CS_PLUGIN_NAMESPACE_BEGIN(csloader)
{

SCF_IMPLEMENT_FACTORY (FileLoader)

class FileResource
  : public scfImplementation2<FileResource, scfFakeInterface<iResource>, iLoadingResource>,
  public CS::Resource::NoDepResource
{
public:
  FileResource (const char* name)
    : scfImplementationType (this)
  {
  }

  virtual const CS::Resource::TypeID GetTypeID () const
  {
    return CS::Resource::GetTypeID ("file");
  }

  void AddResource (iLoadingResource* resource)
  {
    resources.Push (resource);
  }

  virtual const char* GetName()
  {
    return name;
  }

  virtual bool Ready ()
  {
    for (size_t i = 0; i < resources.GetSize (); ++i)
    {
      if (!resources[i]->Ready ())
        return false;
    }

    return true;
  }

  virtual iResource* Get()
  {
    return this;
  }

  virtual void AddListener (iResourceListener* listener)
  {
    if (Ready ())
      listener->OnLoaded (this);
    else
      listeners.Push (listener);
  }

  virtual void RemoveListener (iResourceListener* listener) 
  {
    listeners.Delete (listener);
  }

  virtual void TriggerCallback()
  {
    for (size_t i = 0; i < listeners.GetSize (); i++)
      listeners.Get (i)->OnLoaded (this);

    listeners.DeleteAll ();
  }

private:
  csString name;
  csRefArray<iLoadingResource> resources;
  csRefArray<iResourceListener> listeners;
};

static int GeneratedNameVal = 0;

FileLoader::FileLoader (iBase* parent)
: scfImplementationType (this, parent)
{
}

FileLoader::~FileLoader ()
{
  if (eventHandler.IsValid ())
    eventQueue->RemoveListener(eventHandler);
}

bool FileLoader::Initialize (iObjectRegistry* obj_reg)
{
  eventQueue = csQueryRegistry<iEventQueue>(obj_reg);
  if (!eventQueue)
  {
    ResourceManager::ReportError ("No event queue loaded!\n");
    return false;
  }

  csEventID ProcessPerFrame = csevFrame (obj_reg);
  eventHandler.AttachNew (new LEventHandler (this, ProcessPerFrame));
  eventQueue->RegisterListener (eventHandler, ProcessPerFrame);

  return ResourceManager::Initialize (obj_reg);
}

csPtr<iLoadingResource> FileLoader::Get (TypeID type, const char* name)
{
  if (type == GetTypeID ("file"))
  {
    csRef<FileResource> resource;
    resource.AttachNew (new FileResource (name));

    // Load the file as a document.
    csRef<iDocument> doc = LoadDocument (name);
    if (!doc.IsValid ()) return 0;

    // Iterate over all nodes and load each one.
    csRef<iDocumentNodeIterator> nodes = doc->GetRoot ()->GetNodes ();
    while (nodes->HasNext ())
    {
      csRef<iDocumentNode> node = nodes->Next ();

      // Get or generate the name of the node.
      csString name = node->GetAttributeValue ("name");
      if (name.IsEmpty ())
      {
        name = "FileLoaderGenName";
        name.Append (GeneratedNameVal++);
      }

      // Get the type of the node.
      TypeID nodeType = GetTypeID (node->GetValue ());

      // Load the node.
      resource->AddResource (ResourceManager::Load (nodeType, name, node));
    }

    loadingFiles.Push (resource);
    return csPtr<iLoadingResource> (resource);
  }

  return ResourceManager::Get (type, name);
}

void FileLoader::Process ()
{
  for (size_t i = 0; i < loadingFiles.GetSize(); i++)
  {
    if (loadingFiles[i]->Ready ())
    {
      loadingFiles[i]->TriggerCallback ();
      loadingFiles.DeleteIndex (i);
    }
  }

  ResourceManager::ProcessResources ();
}

}
CS_PLUGIN_NAMESPACE_END(loaders)
