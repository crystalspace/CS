/*
  Copyright (C) 2009 by Michael Gist

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_IMPMAN_H__
#define __CS_IMPMAN_H__

#include "csutil/scf_implementation.h"
#include "iengine/impman.h"
#include "iutil/eventh.h"

class csEngine;

class csImposterManager : public scfImplementation1<
	csImposterManager, iImposterManager>
{
private:
  /* Handles an event from the event handler */
  bool HandleEvent(iEvent &ev);

  class EventHandler : public scfImplementation1<EventHandler, iEventHandler>
  {
  private:
    csImposterManager* parent;

  public:
    EventHandler(csImposterManager* p)
      : scfImplementationType(this), parent(p) {}

    virtual ~EventHandler() {}

    virtual bool HandleEvent(iEvent& event)
    {
      return parent->HandleEvent(event);
    }

    CS_EVENTHANDLER_NAMES("csImposterManager");
    CS_EVENTHANDLER_NIL_CONSTRAINTS;
  };

  class TextureSpace : public CS::Utility::FastRefCount<TextureSpace>
  {
  public:
    TextureSpace(size_t width, size_t height, iMaterialWrapper* material,
      TextureSpace* parent = 0);

    TextureSpace* Allocate(size_t& width, size_t& height, csBox2& texCoords);

    bool Realloc(size_t& width, size_t& height, csBox2& texCoords) const;

    void Free();

    inline bool IsFull() const { return full; }

    iMaterialWrapper* GetMaterial() const { return material; }

    void GetRenderTextureDimensions(size_t& width, size_t& height) const;

    bool IsUsed() const;

  private:
    csRef<TextureSpace> firstSpace;
    csRef<TextureSpace> secondSpace;
    size_t width;
    size_t height;
    size_t childWidth;
    size_t childHeight;

    size_t minX;
    size_t minY;

    csRef<iMaterialWrapper> material;
    TextureSpace* parent;
    bool full;
    size_t rTexWidth;
    size_t rTexHeight;
  };

  csRefArray<TextureSpace> textureSpace;

  struct ImposterMat : CS::Utility::FastRefCount<ImposterMat>
  {
    csRef<csImposterMesh> mesh;
    bool init;
    bool update;
    bool remove;

    size_t texWidth;
    size_t texHeight;

    TextureSpace* allocatedSpace;

    csArray<ImposterShader> shaders;

    ImposterMat(iImposterMesh* imesh)
      : init(false), update(false),
      remove(false), allocatedSpace(0)
    {
      mesh = static_cast<csImposterMesh*>(imesh);
    }

    ~ImposterMat()
    {
      // Free allocated texture space.
      if(allocatedSpace)
      {
        allocatedSpace->Free();
      }
    }
  };

  /* Allocates texture space for r2t. */
  iMaterialWrapper* AllocateTexture(ImposterMat* imposter,
      csBox2& texCoords, size_t& width, size_t& height);

  /* Initialises an imposter. */
  bool InitialiseImposter(ImposterMat* imposter);

  /* Updated an imposter. */
  void UpdateImposter(ImposterMat* imposter);

  /* Hash of imposter mesh<->mat */
  csHash<csRef<ImposterMat>, csPtrKey<iImposterMesh> > imposterMats;

  /* Queues for processing updates to imposters */
  csRefArray<ImposterMat> initQueue;
  csRefArray<ImposterMat> updateQueue;
  csRefArray<ImposterMat> removeQueue;

  csEngine* engine;

  csRef<iGraphics3D> g3d;

  size_t maxWidth;
  size_t maxHeight;

  // Max number of imposters to update per frame.
  int updatePerFrame;
  
  /**
   * For management of non-instanced meshes.
   * One 'mesh' per sector, we need to keep it updated.
   */
  struct SectorImposter : CS::Utility::FastRefCount<SectorImposter>
  {
    csWeakRef<iSector> sector;
    csRef<csBatchedImposterMesh> sectorImposter;
  };

  csRefArray<SectorImposter> sectorImposters;

  void AddMeshToImposter(csImposterMesh* imposter);

  void RemoveMeshFromImposter(csImposterMesh* imposter);

public:
  csImposterManager(csEngine* engine);
  virtual ~csImposterManager();

  /////////////// iImposterManager ///////////////
  void Register(iImposterMesh* mesh);

  bool Update(iImposterMesh* mesh);

  void Unregister(iImposterMesh* mesh);
};

#endif // __CS_IMPMAN_H__
