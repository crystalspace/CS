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
    csRef<iMaterialWrapper> material;

    TextureSpace(size_t width, size_t height, TextureSpace* parent = 0);

    TextureSpace* Allocate(size_t& width, size_t& height, csBox2& texCoords);

    void Free();

    inline bool IsFull() const { return full; }

  private:
    csRef<TextureSpace> firstSpace;
    csRef<TextureSpace> secondSpace;
    size_t childWidth;
    size_t childHeight;

    size_t minX;
    size_t minY;

    TextureSpace* parent;

    bool full;
  };

  csRefArray<TextureSpace> textureSpace;

  struct ImposterMat
  {
    csRef<iImposterMesh> mesh;
    bool init;
    bool update;
    bool remove;

    size_t lastDistance;
    size_t texWidth;
    size_t texHeight;

    TextureSpace* allocatedSpace;

    ImposterMat(iImposterMesh* mesh)
      : mesh(mesh), init(false), update(false),
      remove(false), lastDistance(size_t(-1)),
      allocatedSpace(0)
    {
    }
  };

  /* Allocates texture space for r2t. */
  iMaterialWrapper* AllocateTexture(ImposterMat* imposter,
      csBox2& texCoords);

  /* Initialises an imposter. */
  void InitialiseImposter(ImposterMat* imposter);

  /* Updated an imposter. */
  void UpdateImposter(ImposterMat* imposter);

  csArray<ImposterMat*> imposterMats;
  csArray<ImposterMat*> updateQueue;

  csEngine* engine;

  csRef<iGraphics3D> g3d;

  size_t maxWidth;
  size_t maxHeight;

  bool shaderLoaded;

public:
  csImposterManager(csEngine* engine);
  virtual ~csImposterManager();

  /////////////// iImposterManager ///////////////
  void Register(iImposterMesh* mesh);

  void Update(iImposterMesh* mesh);

  void Unregister(iImposterMesh* mesh);
};

#endif // __CS_IMPMAN_H__
