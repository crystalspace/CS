/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef ICONFIG_H
#define ICONFIG_H

#include "cscom/com.h"

enum csVariantType
{
  CSVAR_LONG,
  CSVAR_BOOL,
  CSVAR_CMD,
  CSVAR_FLOAT
};

struct csVariant
{
  csVariantType type;
  union value
  {
    long lVal;
    bool bVal;
    float fVal;
  } v;
};

struct csOptionDescription
{
  int id;
  char* name;		// Short name of this option.
  char* description;	// Description for this option.
  csVariantType type;	// Type to use for this option.
};

extern const IID IID_IConfig;

/**
 * Interface to a configurator object. If a COM module
 * has an object implementing this interface then this can
 * be used to query/set configuration options.
 */
interface IConfig : public IUnknown
{
  ///
  COM_METHOD_DECL SetOption (int id, csVariant* value) PURE;
  ///
  COM_METHOD_DECL GetOption (int id, csVariant* value) PURE;
  ///
  COM_METHOD_DECL GetNumberOptions (int& num) PURE;
  ///
  COM_METHOD_DECL GetOptionDescription (int idx, csOptionDescription* option) PURE;
};

#endif //ICONFIG_H

