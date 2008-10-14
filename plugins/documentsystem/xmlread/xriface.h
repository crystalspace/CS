/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_XRIFACE_H__
#define __CS_XRIFACE_H__

#include "iutil/document.h"
#include "csutil/scf_implementation.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLRead)
{

struct csXmlReadNode;
class TrDocumentNode;

/**
 * This is an SCF compatible wrapper for the TinyXml parser in csutil.
 */
class csXmlReadDocumentSystem :
  public scfImplementation1<csXmlReadDocumentSystem, iDocumentSystem>
{
public:
  csXmlReadDocumentSystem (iBase* parent);
  virtual ~csXmlReadDocumentSystem ();

  virtual csRef<iDocument> CreateDocument ();
};

}
CS_PLUGIN_NAMESPACE_END(XMLRead)

#endif // __CS_XRIFACE_H__
