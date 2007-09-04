/*
    Copyright (C) 2006 by Dariusz Dawidowski

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

#ifndef __CS_IVARIA_TRANSLATOR_H__
#define __CS_IVARIA_TRANSLATOR_H__

/**\file
 * Translator utility plugin interface
 */

#include "csutil/scf.h"

/**
 * Translator interface.
 */
struct iTranslator : public virtual iBase
{
  SCF_INTERFACE (iTranslator, 0, 0, 1);

  /// Get translated message.
  virtual const char* GetMsg (const char* src) const = 0;
};

#endif // __CS_IVARIA_TRANSLATOR_H__
