// exception.h
//
// Copyright (C) 2003, 2004 Jason Bevins
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or (at
// your option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License (COPYING.txt) for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// The developer's email is jlbezigvins@gmzigail.com (for great email, take
// off every 'zig'.)
//

#ifndef NOISE_EXCEPTION_H
#define NOISE_EXCEPTION_H

namespace CS {
namespace Math {
namespace Noise {

  /// @addtogroup libnoise
  /// @{
  /*
  /// Abstract base class for libnoise exceptions
  class CS_CRYSTALSPACE_EXPORT Exception
  {
  };

  /// Invalid parameter exception
  ///
  /// An invalid parameter was passed to a libnoise function or method.
  class CS_CRYSTALSPACE_EXPORT ExceptionInvalidParam: public Exception
  {
  };

  /// No module exception
  ///
  /// Could not retrieve a source module from a noise module.
  ///
  /// @note If one or more required source modules were not connected to a
  /// specific noise module, and its GetValue() method was called, that
  /// method will raise a debug assertion instead of this exception.  This
  /// is done for performance reasons.
  class CS_CRYSTALSPACE_EXPORT ExceptionNoModule: public Exception
  {
  };

  /// Out of memory exception
  ///
  /// There was not enough memory to perform an action.
  class CS_CRYSTALSPACE_EXPORT ExceptionOutOfMemory: public Exception
  {
  };

  /// Unknown exception
  ///
  /// libnoise raised an unknown exception.
  class CS_CRYSTALSPACE_EXPORT ExceptionUnknown: public Exception
  {
  };
  */
  /// @}

} // namespace Noise
} // namespace Math
} // namespace CS

#endif
