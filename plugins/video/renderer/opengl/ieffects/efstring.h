/*
    Copyright (C) 2002 by Anders Stenberg
    Minior rewrite by Marten Svanfeldt

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

#ifndef __CS_IVIDEO_EFSTRING_H__
#define __CS_IVIDEO_EFSTRING_H__

/**\file
 */

struct iEffectServer;

/// Document me!@@@
class csEffectStrings
{
public:
# define REGISTER_STRING( name, string ) \
  csStringID name;
# include "effectstrings.h"
# undef REGISTER_STRING

  void InitStrings (iEffectServer* server)
  {
    #  define REGISTER_STRING( name, string ) \
    name = server->RequestString( string );
    #  include "effectstrings.h"
    #  undef REGISTER_STRING
  }
};

#endif // __CS_IVIDEO_EFSTRING_H__
