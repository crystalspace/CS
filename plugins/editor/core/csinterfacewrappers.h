/*
    Copyright (C) 2007 by Seth Yastrov

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

#ifndef __CORE_CSINTERFACEWRAPPERS_H__
#define __CORE_CSINTERFACEWRAPPERS_H__

#include "ieditor/editor.h"
#include "ieditor/interfacewrappermanager.h"
#include "ieditor/interfacewrapper.h"

#include <csutil/scf_implementation.h>
#include <iutil/comp.h>

#include <iutil/object.h>
#include <iengine/scenenode.h>
#include <iengine/mesh.h>


using namespace CS::EditorApp;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

/// Registers wrappers for CS interfaces
class CSInterfaceWrappers : public scfImplementation1<CSInterfaceWrappers,iComponent>
{
public:
  CSInterfaceWrappers (iBase* parent);
  virtual ~CSInterfaceWrappers ();

  // iComponent
  virtual bool Initialize (iObjectRegistry* obj_reg);

private:
  iObjectRegistry* object_reg;

  csRef<iInterfaceWrapperManager> interfaceManager;
};

namespace InterfaceWrapper
{
  /// Wraps iObject
  class Object : public scfImplementation1<Object,iInterfaceWrapper> {
  public:
    Object (iObject* object, iInterfaceWrapperFactory* fact)
      : scfImplementationType (this), object (object), fact (fact)
    {}
    
    virtual ~Object () {}

    virtual iInterfaceWrapperFactory* GetFactory ()
    {
      return fact;
    }
    
    virtual const char* GetObjectName () const
    {
      return object->GetName ();
    }

    virtual void SetObjectName (const char* name)
    {
      object->SetName (name);
    }

    virtual iBase* GetObjectParent ()
    {
      return object->GetObjectParent ();
    }

    virtual bool SetObjectParent (iBase* parent)
    {
      csRef<iObject> parentObject = scfQueryInterface<iObject> (parent);
      if (!parentObject)
        return false;
      
      object->SetObjectParent (parentObject);
      return true;
    }

    class ObjectIterator : public scfImplementation1<ObjectIterator,iBaseIterator>
    {
    public:
      virtual bool HasNext () const
      { return it->HasNext (); }

      virtual iBase* Next ()
      { return it->Next (); }

      virtual void Reset ()
      { it->Reset (); }

    protected:
      ObjectIterator(iObject* obj)
      : scfImplementationType (this), it (obj->GetIterator ())
      { }

    private:
      csRef<iObjectIterator> it;

      friend class Object;
    };

    virtual csPtr<iBaseIterator> GetIterator ()
    {
      return csPtr<iBaseIterator> (new ObjectIterator(object));
    }

  private:
    csRef<iObject> object;
    csRef<iInterfaceWrapperFactory> fact;
  };

  /// Factory for iObject wrapper
  class ObjectFactory : public scfImplementation1<ObjectFactory,iInterfaceWrapperFactory> {
  public:
    ObjectFactory () : scfImplementationType (this) {}
    virtual ~ObjectFactory () {}

    virtual csPtr<iInterfaceWrapper> CreateInstance (iBase* object)
    {
      csRef<iObject> obj = scfQueryInterface<iObject> (object);
      return csPtr<iInterfaceWrapper> (new Object (obj, this));
    }

    virtual scfInterfaceID GetInterfaceID () const
    { return scfInterfaceTraits<iObject>::GetID (); }
    
    virtual EditorObjectType GetInterfaceType () const
    { return EditorObjectTypeUnknown; }

    virtual bool HasNameAttribute () const
    { return true; }

    virtual bool HasParentAttribute () const
    { return true; }
  };

  /// Wraps iSceneNode
  class SceneNode : public scfImplementation1<SceneNode,iInterfaceWrapper> {
  public:
    SceneNode (iSceneNode* object, iInterfaceWrapperFactory* fact)
      : scfImplementationType (this), object (object), fact (fact)
    {}
    
    virtual ~SceneNode () {}

    virtual iInterfaceWrapperFactory* GetFactory ()
    {
      return fact;
    }
    
    virtual const char* GetObjectName () const
    { return 0; }

    virtual void SetObjectName (const char* name)
    { }

    virtual iBase* GetObjectParent ()
    {
      return object->GetParent ();
    }

    virtual bool SetObjectParent (iBase* parent)
    {
      if (parent == 0)
      {
        object->SetParent (0);
        return true;
      }
      
      csRef<iSceneNode> parentObject = scfQueryInterface<iSceneNode> (parent);
      if (!parentObject)
        return false;
      
      object->SetParent (parentObject);
      return true;
    }

    class SceneNodeIterator : public scfImplementation1<SceneNodeIterator,iBaseIterator>
    {
    public:
      virtual bool HasNext () const
      { return idx < arr->GetSize (); }

      virtual iBase* Next ()
      { return arr->Get (idx++); }

      virtual void Reset ()
      { idx = 0; }

    protected:
      SceneNodeIterator(iSceneNode* obj)
      : scfImplementationType (this), arr (obj->GetChildrenArray ()), idx (0)
      { }

    private:
      csRef<iSceneNodeArray> arr;

      size_t idx;

      friend class SceneNode;
    };

    virtual csPtr<iBaseIterator> GetIterator ()
    {
      return csPtr<iBaseIterator> (new SceneNodeIterator(object));
    }

  private:
    csRef<iSceneNode> object;
    csRef<iInterfaceWrapperFactory> fact;
  };

  /// Factory for iSceneNode wrapper
  class SceneNodeFactory : public scfImplementation1<SceneNodeFactory,iInterfaceWrapperFactory> {
  public:
    SceneNodeFactory () : scfImplementationType (this) {}
    virtual ~SceneNodeFactory () {}

    virtual csPtr<iInterfaceWrapper> CreateInstance (iBase* object)
    {
      csRef<iSceneNode> obj = scfQueryInterface<iSceneNode> (object);
      return csPtr<iInterfaceWrapper> (new SceneNode (obj, this));
    }

    virtual scfInterfaceID GetInterfaceID () const
    { return scfInterfaceTraits<iSceneNode>::GetID (); }
    
    virtual EditorObjectType GetInterfaceType () const
    { return EditorObjectTypeInstance; }

    virtual bool HasNameAttribute () const
    { return false; }

    virtual bool HasParentAttribute () const
    { return true; }
  };

/// Wraps iMeshFactoryWrapper
  class MeshFactoryWrapper : public scfImplementation1<MeshFactoryWrapper,iInterfaceWrapper> {
  public:
    MeshFactoryWrapper (iMeshFactoryWrapper* object, iInterfaceWrapperFactory* fact)
      : scfImplementationType (this), object (object), fact (fact)
    {}
    
    virtual ~MeshFactoryWrapper () {}

    virtual iInterfaceWrapperFactory* GetFactory ()
    {
      return fact;
    }
    
    virtual const char* GetObjectName () const
    { return 0; }

    virtual void SetObjectName (const char* name)
    { }

    virtual iBase* GetObjectParent ()
    {
      return object->GetParentContainer ();
    }

    virtual bool SetObjectParent (iBase* parent)
    {
      if (parent == 0)
      {
        object->GetParentContainer()->GetChildren()->Remove(object);
        object->SetParentContainer (0);
        return true;
      }
      
      csRef<iMeshFactoryWrapper> parentObject = scfQueryInterface<iMeshFactoryWrapper> (parent);
      if (!parentObject)
        return false;
      
      parentObject->GetChildren()->Add(object);
      object->SetParentContainer (parentObject);
      return true;
    }

    class MeshFactoryWrapperIterator : public scfImplementation1<MeshFactoryWrapperIterator,iBaseIterator>
    {
    public:
      virtual bool HasNext () const
      { return idx < arr->GetCount (); }

      virtual iBase* Next ()
      { return arr->Get (idx++); }

      virtual void Reset ()
      { idx = 0; }

    protected:
      MeshFactoryWrapperIterator(iMeshFactoryWrapper* obj)
      : scfImplementationType (this), arr (obj->GetChildren()), idx (0)
      { }

    private:
      csRef<iMeshFactoryList> arr;

      int idx;

      friend class MeshFactoryWrapper;
    };

    virtual csPtr<iBaseIterator> GetIterator ()
    {
      return csPtr<iBaseIterator> (new MeshFactoryWrapperIterator(object));
    }

  private:
    csRef<iMeshFactoryWrapper> object;
    csRef<iInterfaceWrapperFactory> fact;
  };

  /// Factory for iMeshFactoryWrapper wrapper
  class MeshFactoryWrapperFactory : public scfImplementation1<MeshFactoryWrapperFactory,iInterfaceWrapperFactory> {
  public:
    MeshFactoryWrapperFactory () : scfImplementationType (this) {}
    virtual ~MeshFactoryWrapperFactory () {}

    virtual csPtr<iInterfaceWrapper> CreateInstance (iBase* object)
    {
      csRef<iMeshFactoryWrapper> obj = scfQueryInterface<iMeshFactoryWrapper> (object);
      return csPtr<iInterfaceWrapper> (new MeshFactoryWrapper (obj, this));
    }

    virtual scfInterfaceID GetInterfaceID () const
    { return scfInterfaceTraits<iMeshFactoryWrapper>::GetID (); }
    
    virtual EditorObjectType GetInterfaceType () const
    { return EditorObjectTypeFactory; }

    virtual bool HasNameAttribute () const
    { return false; }

    virtual bool HasParentAttribute () const
    { return true; }
  };
}

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
