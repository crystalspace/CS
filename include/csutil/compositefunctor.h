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

#ifndef __CS_CSUTIL_COMPOSITEFUNCTOR_H__
#define __CS_CSUTIL_COMPOSITEFUNCTOR_H__

namespace CS
{
namespace Meta
{

  template<typename Fn1, typename Fn2>
  class CompositeFunctorType2
  {
  public:
    CompositeFunctorType2 (Fn1& fn1, Fn2& fn2)
      : fn1 (fn1), fn2 (fn2)
    {}

    void operator() ()
    {
      fn1();
      fn2();
    }

    template<typename A1>
    void operator() (A1& a1)
    {
      fn1(a1);
      fn2(a1);
    }

    template<typename A1, typename A2>
    void operator() (A1& a1, A2& a2)
    {
      fn1(a1, a2);
      fn2(a1, a2);
    }

    template<typename A1, typename A2, typename A3>
    void operator() (A1& a1, A2& a2, A3& a3)
    {
      fn1(a1, a2, a3);
      fn2(a1, a2, a3);
    }

    template<typename A1, typename A2, typename A3, typename A4>
    void operator() (A1& a1, A2& a2, A3& a3, A4& a4)
    {
      fn1(a1, a2, a3, a4);
      fn2(a1, a2, a3, a4);
    }

  private:
    Fn1& fn1;
    Fn2& fn2;
  };

  template<typename Fn1, typename Fn2, typename Fn3>
  class CompositeFunctorType3
  {
  public:
    CompositeFunctorType3 (Fn1& fn1, Fn2& fn2, Fn3& fn3)
      : fn1 (fn1), fn2 (fn2), fn3 (fn3)
    {}

    void operator() ()
    {
      fn1();
      fn2();
      fn3();
    }

    template<typename A1>
    void operator() (A1& a1)
    {
      fn1(a1);
      fn2(a1);
      fn3(a1);
    }

    template<typename A1, typename A2>
    void operator() (A1& a1, A2& a2)
    {
      fn1(a1, a2);
      fn2(a1, a2);
      fn3(a1, a2);
    }

    template<typename A1, typename A2, typename A3>
    void operator() (A1& a1, A2& a2, A3& a3)
    {
      fn1(a1, a2, a3);
      fn2(a1, a2, a3);
      fn3(a1, a2, a3);
    }

    template<typename A1, typename A2, typename A3, typename A4>
    void operator() (A1& a1, A2& a2, A3& a3, A4& a4)
    {
      fn1(a1, a2, a3, a4);
      fn2(a1, a2, a3, a4);
      fn3(a1, a2, a3, a4);
    }

  private:
    Fn1& fn1;
    Fn2& fn2;
    Fn3& fn3;
  };

  template<typename Fn1, typename Fn2>
  CompositeFunctorType2<Fn1, Fn2> CompositeFunctor (Fn1& fn1, Fn2& fn2)
  {
    return CompositeFunctorType2<Fn1, Fn2> (fn1, fn2);
  }

  template<typename Fn1, typename Fn2, typename Fn3>
  CompositeFunctorType3<Fn1, Fn2, Fn3> CompositeFunctor (Fn1& fn1, Fn2& fn2, Fn3& fn3)
  {
    return CompositeFunctorType3<Fn1, Fn2, Fn3> (fn1, fn2, fn3);
  }
}
}

#endif
