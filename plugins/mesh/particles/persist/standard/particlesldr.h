/*
    Copyright (C) 2003 by Jorrit Tyberghein, John Harger

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

#ifndef __CS_PARTICLESLDR_H__
#define __CS_PARTICLESLDR_H__

#include "imap/loader.h"
#include "imap/reader.h"
#include "imap/services.h"
#include "imap/writer.h"
#include "iutil/comp.h"
#include "iutil/vfs.h"
#include "csutil/strhash.h"
#include "csutil/scf_implementation.h"

struct iObjectRegistry;

/**
 * Particle factory loader.
 */
class csParticlesFactoryLoader :
  public scfImplementation2<csParticlesFactoryLoader,
    iLoaderPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csRef<iVFS> vfs;

  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "plugins/mesh/particles/persist/standard/particlesldr.tok"
#include "cstool/tokenlist.h"

public:
  /// Constructor
  csParticlesFactoryLoader (iBase*);
  /// Destructor
  virtual ~csParticlesFactoryLoader ();

  /// Setup the plugin with the system driver
  bool Initialize (iObjectRegistry *objreg);

  /// Parse the given node block and build the particles factory
  csPtr<iBase> Parse (iDocumentNode *node,
  	iStreamSource*, iLoaderContext *ldr_context, iBase* context);

  /// Parse the emitter block
  bool ParseEmitter (iDocumentNode *node, iParticlesFactoryState *state);

  /// Parse the force block
  bool ParseForce (iDocumentNode *node, iParticlesFactoryState *state);

  /// Parse constant color block
  bool ParseColorConstant (iDocumentNode *node, iParticlesFactoryState *state);

  /// Parse linear color block
  bool ParseColorLinear (iDocumentNode *node, iParticlesFactoryState *state);

  /// Parse looping color block
  bool ParseColorLooping (iDocumentNode *node, iParticlesFactoryState *state);

  /// Parse heat color block
  bool ParseColorHeat (iDocumentNode *node, iParticlesFactoryState *state);

  /// Parse the color gradient block
  bool ParseGradient (iDocumentNode *node, iParticlesFactoryState *state);
};

/**
 * Particle factory saver.
 */
class csParticlesFactorySaver :
  public scfImplementation2<csParticlesFactorySaver,
    iSaverPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor
  csParticlesFactorySaver (iBase*);

  /// Destructor
  virtual ~csParticlesFactorySaver ();

  /// Register plugin with system driver
  bool Initialize (iObjectRegistry *objreg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);

  /// Write down the emitter block.
  virtual bool WriteEmitter (iParticlesFactoryState *obj,
  	iDocumentNode* parent);

  /// Write down colormethode block.
  virtual bool WriteColorMethode (iParticlesFactoryState *obj,
  	iDocumentNode* parent);
};

/**
 * Particles loader.
 */
class csParticlesObjectLoader :
  public scfImplementation2<csParticlesObjectLoader,
    iLoaderPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csRef<iVFS> vfs;

  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "plugins/mesh/particles/persist/standard/particlesldr.tok"
#include "cstool/tokenlist.h"

public:
  /// Constructor
  csParticlesObjectLoader (iBase*);

  /// Destructor
  virtual ~csParticlesObjectLoader ();

  /// Register plugin with system driver
  bool Initialize (iObjectRegistry *objreg);

  /// Parse the given block to create a new particles object
  csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context,
    iBase *context);

  /// Parse the emitter block
  bool ParseEmitter (iDocumentNode *node, iParticlesObjectState *state);

  /// Parse the force block
  bool ParseForce (iDocumentNode *node, iParticlesObjectState *state);

  /// Parse constant color block
  bool ParseColorConstant (iDocumentNode *node, iParticlesObjectState *state);

  /// Parse linear color block
  bool ParseColorLinear (iDocumentNode *node, iParticlesObjectState *state);

  /// Parse looping color block
  bool ParseColorLooping (iDocumentNode *node, iParticlesObjectState *state);

  /// Parse heat color block
  bool ParseColorHeat (iDocumentNode *node, iParticlesObjectState *state);

  /// Parse the color gradient block
  bool ParseGradient (iDocumentNode *node, iParticlesObjectState *state);
};

/**
 * Particles saver.
 */
class csParticlesObjectSaver :
  public scfImplementation2<csParticlesObjectSaver,
    iSaverPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor
  csParticlesObjectSaver (iBase*);

  /// Destructor
  virtual ~csParticlesObjectSaver ();

  /// Register plugin with the system driver
  bool Initialize (iObjectRegistry *objreg);

  /// Write down given object and add to iDocumentNode.
  bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);

  /// Write the emitter block
  bool WriteEmitter (iParticlesObjectState *state, iDocumentNode *node);

  /// Write constant color block
  bool WriteColorMethode (iParticlesObjectState *state, iDocumentNode *node);
};

#endif // __CS_PARTICLESLDR_H__
