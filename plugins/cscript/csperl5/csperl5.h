/*
    Copyright (C) 2002 by Mat Sutcliffe <oktal@gmx.co.uk>

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

#include "ivaria/script.h"
#include "iutil/comp.h"
#include "ivaria/reporter.h"

#undef MIN
#undef MAX
#include <EXTERN.h>
#include <perl.h>

struct iObjectRegistry;

class csPerl5 : public iScript
{
  private:
  PerlInterpreter *perl;

  protected:
  csRef<iReporter> reporter;

  public:
  SCF_DECLARE_IBASE;

  csPerl5 (iBase *);
  virtual ~csPerl5 ();

  virtual bool Initialize (iObjectRegistry *);
  virtual bool RunText (const char *);
  virtual bool LoadModule (const char *);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csPerl5);
    virtual bool Initialize (iObjectRegistry *);
  } scfiComponent;
  friend struct eiComponent;

  /**
   * Store() is implementation-dependent and meant for internal use
   * tag is casted to (const char *)
   * its value determines what type data is casted to:
   * "i" : (int *)
   * "f" : (float *)
   * "s" : (const char *)
   */
  virtual bool Store (const char *name, void *data, void *tag);
};

