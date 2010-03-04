/*
Copyright (C) 2003 by Mat Sutcliffe <oktal@gmx.co.uk>
2007 by Marten Svanfeldt

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

#ifndef __CS_UTIL_HASHCOMPUTER_H__
#define __CS_UTIL_HASHCOMPUTER_H__

/**\addtogroup util_containers
* @{ */


/**
* Compute a hash key for a null-terminated string.
* <p>
* Note that these keys are non-unique; some dissimilar strings may generate
* the same key. For unique keys, see csStringSet.
*/
CS_CRYSTALSPACE_EXPORT
unsigned int csHashCompute (char const*);

/**
* Compute a hash key for a string of a given length.
* <p>
* Note that these keys are non-unique; some dissimilar strings may generate
* the same key. For unique keys, see csStringSet.
*/
CS_CRYSTALSPACE_EXPORT
unsigned int csHashCompute (char const*, size_t length);

/**
* Template for hash value computing.
* Expects that T has a method 'uint GetHash() const'.
*/
template <class T>
class csHashComputer
{
public:
  /// Compute a hash value for \a key.
  static uint ComputeHash (const T& key)
  {
    return key.GetHash();
  }
};

/**
* Template for hash value computing, suitable for integral types and types 
* that can be casted to such.
*/
template <class T>
class csHashComputerIntegral
{
public:
  /// Compute a hash value for \a key.
  static uint ComputeHash (const T& key)
  {
    return (uintptr_t)key;
  }
};

//@{
/**
* csHashComputer<> specialization for an integral type.
*/
template<class T>
class csHashComputer<T*> : public csHashComputerIntegral<T*> {};

template<>
class csHashComputer<int> : public csHashComputerIntegral<int> {}; 
template<>
class csHashComputer<unsigned int> : 
  public csHashComputerIntegral<unsigned int> {}; 

template<>
class csHashComputer<long> : public csHashComputerIntegral<long> {}; 
template<>
class csHashComputer<unsigned long> : 
  public csHashComputerIntegral<unsigned long> {}; 

#if (CS_LONG_SIZE < 8)    
template<>
class csHashComputer<longlong> : 
  public csHashComputerIntegral<longlong> {}; 
template<>
class csHashComputer<ulonglong> : 
  public csHashComputerIntegral<ulonglong> {}; 
#endif

template<>
class csHashComputer<float>
{
public:
  /// Compute a hash value for \a key.
  static uint ComputeHash (float key)
  {
    union
    {
      float f;
      uint u;
    } float2uint;
    float2uint.f = key;
    return float2uint.u;
  }
};
template<>
class csHashComputer<double>
{
public:
  /// Compute a hash value for \a key.
  static uint ComputeHash (double key)
  {
    union
    {
      double f;
      uint u;
    } double2uint;
    double2uint.f = key;
    return double2uint.u;
  }
};


/**
* Template that can be used as a base class for hash computers for 
* string types (must support cast to const char*).
* Example:
* \code
* template<> class csHashComputer<MyString> : 
*   public csHashComputerString<MyString> {};
* \endcode
*/
template <class T>
class csHashComputerString
{
public:
  static uint ComputeHash (const T& key)
  {
    return csHashCompute ((const char*)key);
  }
};

/**
* csHashComputer<> specialization for strings that uses csHashCompute().
*/
template<>
class csHashComputer<const char*> : public csHashComputerString<const char*> {};

/**
* Template that can be used as a base class for hash computers for POD 
* structs.
* Example:
* \code
* template<> class csHashComputer<MyStruct> : 
*   public csHashComputerStruct<MyStruct> {};
* \endcode
*/
template <class T>
class csHashComputerStruct
{
public:
  static uint ComputeHash (const T& key)
  {
    return csHashCompute ((char*)&key, sizeof (T));
  }
};

//@}

namespace CS
{
  namespace Utility
  {

    class HashFoldingFNV1
    {
    public:
      static uint32 FoldHash (uint32 input)
      {
        static const uint32 fnvprime = 0x1000193;
        static const uint32 fnvoffset = 0x811C9DC5;

        uint32 fold = fnvoffset;
        for (size_t i = 0; i < 4; ++i)
        {
          fold = fold ^ (input & 0xFF);
          fold *= fnvprime;
          input >>= 8;
        }
        
        return fold;
      }
    };

    class HashFoldingNone
    {
    public:
      static uint32 FoldHash (uint32 input)
      {
        return input;
      }
    };

  }
}

#endif
