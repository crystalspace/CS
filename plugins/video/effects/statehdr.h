/*
    Copyright (C) 2002 by Anders Stenberg
    Written by Anders Stenberg

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

#ifndef __STATEHANDLER_H__
#define __STATEHANDLER_H__

#include "csutil/strset.h"

#define CS_STATETYPE_FLOAT    1
#define CS_STATETYPE_STRING    2
#define CS_STATETYPE_OPAQUE    3


struct statedata 
{
  char type;
  csStringID name;
  union 
  {
    float float_value;
    csStringID string_value;
    void *opaque_value;
  };
  statedata( csStringID state, float value ) { name = state; float_value = value; type = CS_STATETYPE_FLOAT; }
  statedata( csStringID state, csStringID value ) { name = state; string_value = value; type = CS_STATETYPE_STRING; }
  statedata( csStringID state, void* value ) { name = state; opaque_value = value; type = CS_STATETYPE_OPAQUE; }
};

class csStateHandler : public iBase
{
private:
  csHashMap *states;
  csHashIterator* iterator;

public:

  SCF_DECLARE_IBASE;
  
  csStateHandler();
  virtual ~csStateHandler();
  void SetStateFloat( csStringID state, float value );
  void SetStateString( csStringID state, csStringID value );
  void SetStateOpaque( csStringID state, void *value );
  
  float GetStateFloat( csStringID state );
  csStringID GetStateString( csStringID state );
  void *GetStateOpaque( csStringID state );

  csStringID GetFirstState();
  csStringID GetNextState();
};

#endif // __STATEHANDLER_H__

