/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#ifndef __CSUTIL_TYPETRAITS_H__
#define __CSUTIL_TYPETRAITS_H__

/**\file
 * General type-traits classes used to do compile-time checking and operations
 * on types.
 */

namespace CS
{
  namespace Meta
  {
    namespace Implementation
    {
      // Types for checking yes/no

      /// Boolean true type
      typedef char YesType;

      /// Boolean false type
      struct NoType
      {
        char padding[8];
      };

      /// Simple dummy-wrapper
      template <class T>
      struct Wrap { Wrap () {} };

      template <class T> T&(* IsReferenceHelper1(Wrap<T>) )(Wrap<T>);
      char IsReferenceHelper1(...);

      template <class T> NoType IsReferenceHelper2(T&(*)(Wrap<T>));
      YesType IsReferenceHelper2(...);

      template <class T>
      struct IsReferenceImpl
      {
        static const bool value = sizeof(IsReferenceHelper2 (
          IsReferenceHelper1 (Wrap<T>()))) == 1;
      };


      template <bool b1, bool b2, bool b3 = true, bool b4 = true, bool b5 = true, 
                bool b6 = true, bool b7 = true>
      struct TraitAnd;

      template <bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7>
      struct TraitAnd
      {
        static const bool value = false;
      };

      template <>
      struct TraitAnd<true, true, true, true, true, true, true>
      {
        static const bool value = true;
      };

      template <bool b1, bool b2, bool b3 = false, bool b4 = false, bool b5 = false,
                bool b6 = false, bool b7 = false>
      struct TraitOr;

      template <bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7>
      struct TraitOr
      {
        static const bool value = true;
      };

      template<>
      struct TraitOr<false, false, false, false, false, false, false>
      {
        static const bool value = false;
      };

      template <class T>
      YesType IsSameTester(T*, T*);

      NoType IsSameTester(...);

      template <class T, class U>
      struct IsSameImpl
      {
        static T* t;
        static U* u;

        static const bool value = TraitAnd<
          sizeof(YesType) == sizeof(IsSameTester(t, u)),
          IsReferenceImpl<T>::value == IsReferenceImpl<U>::value,
          sizeof(T) == sizeof(U)>::value;
      };

      template<typename B, typename D>
      struct IsBaseOfHelper
      {
        operator const B*() const;
        operator const D*();
      };

      template<typename B, typename D>
      struct IsBaseOfImpl
      {
        template<typename T>
        static YesType Test(const D*,T);
        static NoType Test(const B*,int);
        static const bool value = TraitOr<
          sizeof(YesType) == sizeof(Test(IsBaseOfHelper<B,D>(),0)),
          IsSameImpl<B,D>::value>::value;
      };
    } // namespace Implementation


    /**
     * Check if Type is a reference or simple type.
     */
    template <class Type>
    struct IsReference
    {
      static const bool value = CS::Meta::Implementation
	::IsReferenceImpl<Type>::value;
    };

    /**
     * Check if two types are the same.
     */
    template <class Type1, class Type2>
    struct IsSame
    {
      static const bool value = CS::Meta::Implementation
	::IsSameImpl<Type1, Type2>::value;
    };

    /**
     * Check if Type1 is a base of Type2.
     */
    template <class Type1, class Type2>
    struct IsBaseOf
    {
      static const bool value = CS::Meta::Implementation
        ::IsBaseOfImpl<Type1, Type2>::value;
    };
  } // namespace TypeTraits

} // namespace CS

#endif 
