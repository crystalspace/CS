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

struct iObjectRegistry;

/**
 * Particle factory loader.
 */
class csParticlesFactoryLoader : public iLoaderPlugin
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csRef<iLoader> loader;
  csRef<iVFS> vfs;
  csStringHash xmltokens;

public:
  SCF_DECLARE_IBASE;

  /// Constructor
  csParticlesFactoryLoader (iBase*);
  /// Destructor
  virtual ~csParticlesFactoryLoader ();

  /// Setup the plugin with the system driver
  bool Initialize (iObjectRegistry *objreg);

  /// Parse the given node block and build the particles factory
  csPtr<iBase> Parse (iDocumentNode *node, iLoaderContext *ldr_context,
    iBase* context);

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

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csParticlesFactoryLoader);
    virtual bool Initialize (iObjectRegistry *p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
};

/**
 * Particle factory saver.
 */
class csParticlesFactorySaver : public iSaverPlugin
{
private:
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor
  csParticlesFactorySaver (iBase*);

  /// Destructor
  virtual ~csParticlesFactorySaver ();

  /// Register plugin with system driver
  bool Initialize (iObjectRegistry *objreg);

  /// Store the particles into a string and add it to iFile
  void WriteDown (iBase *obj, iFile *file);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csParticlesFactorySaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
};

/**
 * Particles loader.
 */
class csParticlesObjectLoader : public iLoaderPlugin
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csRef<iLoader> loader;
  csRef<iVFS> vfs;
  csStringHash xmltokens;

public:
  SCF_DECLARE_IBASE;

  /// Constructor
  csParticlesObjectLoader (iBase*);

  /// Destructor
  virtual ~csParticlesObjectLoader ();

  /// Register plugin with system driver
  bool Initialize (iObjectRegistry *objreg);

  /// Parse the given block to create a new particles object
  csPtr<iBase> Parse (iDocumentNode* node, iLoaderContext* ldr_context,
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

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csParticlesObjectLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
};

/**
 * Particles saver.
 */
class csParticlesObjectSaver : public iSaverPlugin
{
private:
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor
  csParticlesObjectSaver (iBase*);

  /// Destructor
  virtual ~csParticlesObjectSaver ();

  /// Register plugin with the system driver
  bool Initialize (iObjectRegistry *objreg);

  /// Write the particles object as a string and add to the iFile
  void WriteDown (iBase* obj, iFile* file);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csParticlesObjectSaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
};

#endif // __CS_PARTICLESLDR_H__
