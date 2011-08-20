/*
Copyright (C) 2010 by Alin Baciu

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __VPLPARSER_H__
#define __VPLPARSER_H__

#include <iutil/comp.h>
#include <ivideodecode/mediaparser.h>
#include <csutil/scf_implementation.h>

struct iObjectRegistry;

/**
  * This is the implementation for our API and
  * also the implementation of the plugin.
  */
class vplParser : public scfImplementation2<vplParser,iMadiaParser,iComponent>
{
private:
  iObjectRegistry* object_reg;

public:
  vplParser (iBase* parent);
  virtual ~vplParser ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  virtual bool Parse (iDocumentNode* doc) ;

  virtual const char* GetMediaPath () ;

  virtual const char* GetMediaType () ;

  virtual csArray<Language> GetLanguages () ;
};

#endif // __VPLPARSER_H__
