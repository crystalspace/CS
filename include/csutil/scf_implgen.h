/*
  Crystal Space Shared Class Facility (SCF)
  This header contains the parts of SCF that is needed when creating
  new classes which implements SCF interfaces.

  Copyright (C) 2005 by Marten Svanfeldt
            (C) 2005 by Michael Adams

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

#if !defined(SCF_IN_IMPLEMENTATION_H) && !defined(DOXYGEN_RUN)
#error Do not include this file directly. Included from scf_implementation.h
#endif

#if 0
/* WTF... seems without a class here doxygen will ignore all the stuff 
 * generated below. Whatever, take it, keep it. */
class __Doxygen_Workaround__ {};
#endif

#define SCF_IN_IMPLGEN_H 1
// Instead of duplicating the code for every scfImplementationN and
// scfImplementationExtN, the code is factored out into an include file
// that we include multiple times.
#define SCF_IMPL_N 0
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 1
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 2
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 3
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 4
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 5
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 6
#include "scf_impl.h"
#undef SCF_IMPL_N
/*
#define SCF_IMPL_N 7
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 8
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 9
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 10
#include "scf_impl.h"
#undef SCF_IMPL_N
*/
// Now all the scfImplementationExt are defined
#define SCF_IMPL_EXT

#define SCF_IMPL_N 0
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 1
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 2
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 3
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 4
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 5
#include "scf_impl.h"
#undef SCF_IMPL_N
/*
#define SCF_IMPL_N 6
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 7
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 8
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 9
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 10
#include "scf_impl.h"
#undef SCF_IMPL_N
*/
#undef SCF_IMPL_EXT
#undef SCF_IN_IMPLGEN_H
