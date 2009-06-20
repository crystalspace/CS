/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A. Wijngaards

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

#ifndef __CS_SPR2DLDR_H__
#define __CS_SPR2DLDR_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "imap/services.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"
#include "csutil/scf_implementation.h"

struct iReporter;
struct iObjectRegistry;
struct iSprite2DFactoryState;

/**
 * Sprite 2D factory loader.
 */
class csSprite2DFactoryLoader :
  public scfImplementation2<csSprite2DFactoryLoader,
    iLoaderPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;
  csStringHash xmltokens;

  bool ParseAnim (iDocumentNode* node, iReporter* reporter, 
		       iSprite2DFactoryState* spr2dLook, 
		       const char *animname);
public:
  /// Constructor.
  csSprite2DFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csSprite2DFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  virtual bool IsThreadSafe() { return true; }
};

/**
 * Sprite2D factory saver.
 */
class csSprite2DFactorySaver :
  public scfImplementation2<csSprite2DFactorySaver,
    iSaverPlugin, iComponent>
{
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csSprite2DFactorySaver (iBase*);

  /// Destructor.
  virtual ~csSprite2DFactorySaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

/**
 * Sprite 2D loader.
 */
class csSprite2DLoader :
  public scfImplementation2<csSprite2DLoader,
    iLoaderPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;
  csStringHash xmltokens;

public:
  /// Constructor.
  csSprite2DLoader (iBase*);

  /// Destructor.
  virtual ~csSprite2DLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  virtual bool IsThreadSafe() { return true; }
};

/**
 * Sprite2D saver.
 */
class csSprite2DSaver :
  public scfImplementation2<csSprite2DSaver,
    iSaverPlugin, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csSprite2DSaver (iBase*);

  /// Destructor.
  virtual ~csSprite2DSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

#endif // __CS_SPR2DLDR_H__
