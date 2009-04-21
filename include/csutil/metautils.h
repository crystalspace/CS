/*
    Copyright (C) 2007 by Marten Svanfeldt

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


#ifndef __CS_CSUTIL_METAUTILS_H__
#define __CS_CSUTIL_METAUTILS_H__

namespace CS
{
  namespace Meta
  {   
    namespace Implementation
    {

      /// Helper to handle alignment of small types
      template<typename T>
      struct AlignmentOfHack
      {
        char c;
        T t;
        AlignmentOfHack();
      };

      /// Select smaller of two alignment values
      template<size_t A, size_t S>
      struct AlignmentLogic
      {
        static const unsigned int value = A < S ? A : S;
      };

      /// Compute alignment for type T
      template<typename T>
      struct AlignmentOfImpl
      {
        static const size_t value = AlignmentLogic<
          sizeof(AlignmentOfHack<T>) - sizeof(T),
          sizeof(T)>::value;
      };


      // Provide types forced aligned to certain boundaries
      struct Aligned1 {CS_ALIGNED_MEMBER(char c, 1);};
      struct Aligned2 {CS_ALIGNED_MEMBER(char c, 2);};
      struct Aligned4 {CS_ALIGNED_MEMBER(char c, 4);};
      struct Aligned8 {CS_ALIGNED_MEMBER(char c, 8);};
      struct Aligned16 {CS_ALIGNED_MEMBER(char c, 16);};     
      
    }

    /**
     * Return alignment of type T
     */
    template<typename T>
    struct AlignmentOf
    {
      static const unsigned int value = Implementation::AlignmentOfImpl<T>::value;
    };

    /**
     * Return the smallest size bigger than size of T aligned to given alignment.
     * Alignment should be a power of two.
     */
    template<typename T, unsigned int Alignment>
    struct AlignSize
    {
      static const unsigned int value = (sizeof(T) + (Alignment - 1)) & ~(Alignment-1);
    };

    /**
     * Get type with specified alignment
     */
    template<size_t Alignment>
    struct TypeWithAlignment
    {/* No default... */};

    template<> struct TypeWithAlignment<1> { typedef Implementation::Aligned1 Type; };
    template<> struct TypeWithAlignment<2> { typedef Implementation::Aligned2 Type; };
    template<> struct TypeWithAlignment<4> { typedef Implementation::Aligned4 Type; };
    template<> struct TypeWithAlignment<8> { typedef Implementation::Aligned8 Type; };
    template<> struct TypeWithAlignment<16> { typedef Implementation::Aligned16 Type; };

    /**
     * Get a unsigned integer type with a given number of bytes of storage
     */
    template<size_t Size>
    struct TypeOfSize
    { /*No default...*/ };

    template<> struct TypeOfSize<1> { typedef uint8 Type; };

    template<> struct TypeOfSize<2> { typedef uint16 Type; };

    template<> struct TypeOfSize<3> { typedef uint32 Type; };
    template<> struct TypeOfSize<4> { typedef uint32 Type; };

    template<> struct TypeOfSize<5> { typedef uint64 Type; };
    template<> struct TypeOfSize<6> { typedef uint64 Type; };
    template<> struct TypeOfSize<7> { typedef uint64 Type; };
    template<> struct TypeOfSize<8> { typedef uint64 Type; };


    /// Helper for log2 computation
    template<size_t R>
    struct Log2
    {
      static const unsigned int RShift = R >> 1;
      static const unsigned int value = Log2<RShift>::value + 1;
    };

    template<>
    struct Log2<1>
    {
      static const unsigned int value = 0;
    };

    template<>
    struct Log2<0>
    {
      static const unsigned int value = 0;
    };

    /// Meta-programming IsLog2 function
    template<size_t R>
    struct IsLog2
    {
      static const bool value = (!(R & (R - 1)) && R);
    };

    /// Helper class for inheriting from a type thats potentially void
    template<typename T>
    struct EBOptHelper : public T
    {};

    template<>
    struct EBOptHelper<void>
    {};
  }
}

#endif
