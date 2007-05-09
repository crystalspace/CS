/*
    Crystal Space Shared Class Facility (SCF)
    Copyright (C) 2005 by Marten Svanfeldt and Michael D. Adams

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

/*
 * This header have been generated from scf_implgen.h by using
 * gcc -E -x c -P -DSCF_IN_IMPLEMENTATION_H -I.. scf_implgen.h
 * after which the result was put into scf_implgen_p_template.h manually!
 */

#if !defined(SCF_IN_IMPLEMENTATION_H)
#error Do not include this file directly. Included from scf_implementation.h
#endif

// This is a big header, so help MSVC a bit
#ifdef CS_COMPILER_MSVC 
#pragma once
#endif

#include "csutil/win32/msvc_deprecated_warn_off.h"

template<class Class >
class CS_CRYSTALSPACE_EXPORT scfImplementation0 :
  public scfImplementation<Class>
 
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
    return scfImplementation<Class>::QueryInterface(id, version);
  }

protected:

  scfImplementation0(Class *object, iBase *parent=0)
    : scfImplementation<Class>(object, parent)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }
  virtual ~scfImplementation0()
  {
    RemoveReftrackerAliases();
  }

  typedef scfImplementation0<Class > scfImplementationType;
  typedef Class scfClassType;

private:
  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void* GetInterface (
    Class* scfObject, scfInterfaceID id, scfInterfaceVersion version)
  {
    if (id == scfInterfaceTraits<I>::GetID() &&
      scfCompatibleVersion(version, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject);
    }
    else
    {
      return 0;
    }
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void AddReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::AddAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void RemoveReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::RemoveAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  void AddReftrackerAliases ()
  {

    AddReftrackerAlias<iBase>(this->scfObject);
  }

  void RemoveReftrackerAliases ()
  {

    RemoveReftrackerAlias<iBase>(this->scfObject);
  }
};
template<class Class ,class I1>
class CS_CRYSTALSPACE_EXPORT scfImplementation1 :
  public scfImplementation<Class>
  ,public I1
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {

    void *x;
    if((x = GetInterface<I1>(this->scfObject, id, version)) != 0) return x;
    return scfImplementation<Class>::QueryInterface(id, version);
  }

protected:

  scfImplementation1(Class *object, iBase *parent=0)
    : scfImplementation<Class>(object, parent)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }
  virtual ~scfImplementation1()
  {
    RemoveReftrackerAliases();
  }

  typedef scfImplementation1<Class ,I1> scfImplementationType;
  typedef Class scfClassType;

private:
  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void* GetInterface (
    Class* scfObject, scfInterfaceID id, scfInterfaceVersion version)
  {
    if (id == scfInterfaceTraits<I>::GetID() &&
      scfCompatibleVersion(version, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject);
    }
    else
    {
      return 0;
    }
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void AddReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::AddAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void RemoveReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::RemoveAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  void AddReftrackerAliases ()
  {

    AddReftrackerAlias<iBase>(this->scfObject);


    AddReftrackerAlias<I1>(this->scfObject);
  }

  void RemoveReftrackerAliases ()
  {

    RemoveReftrackerAlias<iBase>(this->scfObject);


    RemoveReftrackerAlias<I1>(this->scfObject);
  }
};
template<class Class ,class I1, class I2>
class CS_CRYSTALSPACE_EXPORT scfImplementation2 :
  public scfImplementation<Class>
  ,public I1, public I2
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {

    void *x;
    if((x = GetInterface<I1>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I2>(this->scfObject, id, version)) != 0) return x;
    return scfImplementation<Class>::QueryInterface(id, version);
  }

protected:

  scfImplementation2(Class *object, iBase *parent=0)
    : scfImplementation<Class>(object, parent)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }
  virtual ~scfImplementation2()
  {
    RemoveReftrackerAliases();
  }

  typedef scfImplementation2<Class ,I1, I2> scfImplementationType;
  typedef Class scfClassType;

private:
  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void* GetInterface (
    Class* scfObject, scfInterfaceID id, scfInterfaceVersion version)
  {
    if (id == scfInterfaceTraits<I>::GetID() &&
      scfCompatibleVersion(version, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject);
    }
    else
    {
      return 0;
    }
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void AddReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::AddAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void RemoveReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::RemoveAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  void AddReftrackerAliases ()
  {

    AddReftrackerAlias<iBase>(this->scfObject);


    AddReftrackerAlias<I1>(this->scfObject);


    AddReftrackerAlias<I2>(this->scfObject);
  }

  void RemoveReftrackerAliases ()
  {

    RemoveReftrackerAlias<iBase>(this->scfObject);


    RemoveReftrackerAlias<I1>(this->scfObject);


    RemoveReftrackerAlias<I2>(this->scfObject);
  }
};
template<class Class ,class I1, class I2, class I3>
class CS_CRYSTALSPACE_EXPORT scfImplementation3 :
  public scfImplementation<Class>
  ,public I1, public I2, public I3
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {

    void *x;
    if((x = GetInterface<I1>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I2>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I3>(this->scfObject, id, version)) != 0) return x;
    return scfImplementation<Class>::QueryInterface(id, version);
  }

protected:

  scfImplementation3(Class *object, iBase *parent=0)
    : scfImplementation<Class>(object, parent)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }
  virtual ~scfImplementation3()
  {
    RemoveReftrackerAliases();
  }

  typedef scfImplementation3<Class ,I1, I2, I3> scfImplementationType;
  typedef Class scfClassType;

private:
  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void* GetInterface (
    Class* scfObject, scfInterfaceID id, scfInterfaceVersion version)
  {
    if (id == scfInterfaceTraits<I>::GetID() &&
      scfCompatibleVersion(version, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject);
    }
    else
    {
      return 0;
    }
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void AddReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::AddAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void RemoveReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::RemoveAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  void AddReftrackerAliases ()
  {

    AddReftrackerAlias<iBase>(this->scfObject);


    AddReftrackerAlias<I1>(this->scfObject);


    AddReftrackerAlias<I2>(this->scfObject);


    AddReftrackerAlias<I3>(this->scfObject);
  }

  void RemoveReftrackerAliases ()
  {

    RemoveReftrackerAlias<iBase>(this->scfObject);


    RemoveReftrackerAlias<I1>(this->scfObject);


    RemoveReftrackerAlias<I2>(this->scfObject);


    RemoveReftrackerAlias<I3>(this->scfObject);
  }
};
template<class Class ,class I1, class I2, class I3, class I4>
class CS_CRYSTALSPACE_EXPORT scfImplementation4 :
  public scfImplementation<Class>
  ,public I1, public I2, public I3, public I4
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {

    void *x;
    if((x = GetInterface<I1>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I2>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I3>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I4>(this->scfObject, id, version)) != 0) return x;
    return scfImplementation<Class>::QueryInterface(id, version);
  }

protected:

  scfImplementation4(Class *object, iBase *parent=0)
    : scfImplementation<Class>(object, parent)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }
  virtual ~scfImplementation4()
  {
    RemoveReftrackerAliases();
  }

  typedef scfImplementation4<Class ,I1, I2, I3, I4> scfImplementationType;
  typedef Class scfClassType;

private:
  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void* GetInterface (
    Class* scfObject, scfInterfaceID id, scfInterfaceVersion version)
  {
    if (id == scfInterfaceTraits<I>::GetID() &&
      scfCompatibleVersion(version, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject);
    }
    else
    {
      return 0;
    }
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void AddReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::AddAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void RemoveReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::RemoveAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  void AddReftrackerAliases ()
  {

    AddReftrackerAlias<iBase>(this->scfObject);


    AddReftrackerAlias<I1>(this->scfObject);


    AddReftrackerAlias<I2>(this->scfObject);


    AddReftrackerAlias<I3>(this->scfObject);


    AddReftrackerAlias<I4>(this->scfObject);
  }

  void RemoveReftrackerAliases ()
  {

    RemoveReftrackerAlias<iBase>(this->scfObject);


    RemoveReftrackerAlias<I1>(this->scfObject);


    RemoveReftrackerAlias<I2>(this->scfObject);


    RemoveReftrackerAlias<I3>(this->scfObject);


    RemoveReftrackerAlias<I4>(this->scfObject);
  }
};
template<class Class ,class I1, class I2, class I3, class I4, class I5>
class CS_CRYSTALSPACE_EXPORT scfImplementation5 :
  public scfImplementation<Class>
  ,public I1, public I2, public I3, public I4, public I5
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {

    void *x;
    if((x = GetInterface<I1>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I2>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I3>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I4>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I5>(this->scfObject, id, version)) != 0) return x;
    return scfImplementation<Class>::QueryInterface(id, version);
  }

protected:

  scfImplementation5(Class *object, iBase *parent=0)
    : scfImplementation<Class>(object, parent)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }
  virtual ~scfImplementation5()
  {
    RemoveReftrackerAliases();
  }

  typedef scfImplementation5<Class ,I1, I2, I3, I4, I5> scfImplementationType;
  typedef Class scfClassType;

private:
  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void* GetInterface (
    Class* scfObject, scfInterfaceID id, scfInterfaceVersion version)
  {
    if (id == scfInterfaceTraits<I>::GetID() &&
      scfCompatibleVersion(version, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject);
    }
    else
    {
      return 0;
    }
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void AddReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::AddAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void RemoveReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::RemoveAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  void AddReftrackerAliases ()
  {

    AddReftrackerAlias<iBase>(this->scfObject);


    AddReftrackerAlias<I1>(this->scfObject);


    AddReftrackerAlias<I2>(this->scfObject);


    AddReftrackerAlias<I3>(this->scfObject);


    AddReftrackerAlias<I4>(this->scfObject);


    AddReftrackerAlias<I5>(this->scfObject);
  }

  void RemoveReftrackerAliases ()
  {

    RemoveReftrackerAlias<iBase>(this->scfObject);


    RemoveReftrackerAlias<I1>(this->scfObject);


    RemoveReftrackerAlias<I2>(this->scfObject);


    RemoveReftrackerAlias<I3>(this->scfObject);


    RemoveReftrackerAlias<I4>(this->scfObject);


    RemoveReftrackerAlias<I5>(this->scfObject);
  }
};
template<class Class ,class I1, class I2, class I3, class I4, class I5, class I6>
class CS_CRYSTALSPACE_EXPORT scfImplementation6 :
  public scfImplementation<Class>
  ,public I1, public I2, public I3, public I4, public I5, public I6
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {

    void *x;
    if((x = GetInterface<I1>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I2>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I3>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I4>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I5>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I6>(this->scfObject, id, version)) != 0) return x;
    return scfImplementation<Class>::QueryInterface(id, version);
  }

protected:

  scfImplementation6(Class *object, iBase *parent=0)
    : scfImplementation<Class>(object, parent)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }
  virtual ~scfImplementation6()
  {
    RemoveReftrackerAliases();
  }

  typedef scfImplementation6<Class ,I1, I2, I3, I4, I5, I6> scfImplementationType;
  typedef Class scfClassType;

private:
  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void* GetInterface (
    Class* scfObject, scfInterfaceID id, scfInterfaceVersion version)
  {
    if (id == scfInterfaceTraits<I>::GetID() &&
      scfCompatibleVersion(version, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject);
    }
    else
    {
      return 0;
    }
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void AddReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::AddAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void RemoveReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::RemoveAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  void AddReftrackerAliases ()
  {

    AddReftrackerAlias<iBase>(this->scfObject);


    AddReftrackerAlias<I1>(this->scfObject);


    AddReftrackerAlias<I2>(this->scfObject);


    AddReftrackerAlias<I3>(this->scfObject);


    AddReftrackerAlias<I4>(this->scfObject);


    AddReftrackerAlias<I5>(this->scfObject);


    AddReftrackerAlias<I6>(this->scfObject);
  }

  void RemoveReftrackerAliases ()
  {

    RemoveReftrackerAlias<iBase>(this->scfObject);


    RemoveReftrackerAlias<I1>(this->scfObject);


    RemoveReftrackerAlias<I2>(this->scfObject);


    RemoveReftrackerAlias<I3>(this->scfObject);


    RemoveReftrackerAlias<I4>(this->scfObject);


    RemoveReftrackerAlias<I5>(this->scfObject);


    RemoveReftrackerAlias<I6>(this->scfObject);
  }
};
template<class Class ,class I1, class I2, class I3, class I4, class I5, class I6, class I7>
class CS_CRYSTALSPACE_EXPORT scfImplementation7 :
  public scfImplementation<Class>
  ,public I1, public I2, public I3, public I4, public I5, public I6, public I7
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {

    void *x;
    if((x = GetInterface<I1>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I2>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I3>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I4>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I5>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I6>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I7>(this->scfObject, id, version)) != 0) return x;
    return scfImplementation<Class>::QueryInterface(id, version);
  }

protected:

  scfImplementation7(Class *object, iBase *parent=0)
    : scfImplementation<Class>(object, parent)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }
  virtual ~scfImplementation7()
  {
    RemoveReftrackerAliases();
  }

  typedef scfImplementation7<Class ,I1, I2, I3, I4, I5, I6, I7> scfImplementationType;
  typedef Class scfClassType;

private:
  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void* GetInterface (
    Class* scfObject, scfInterfaceID id, scfInterfaceVersion version)
  {
    if (id == scfInterfaceTraits<I>::GetID() &&
      scfCompatibleVersion(version, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject);
    }
    else
    {
      return 0;
    }
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void AddReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::AddAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void RemoveReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::RemoveAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  void AddReftrackerAliases ()
  {

    AddReftrackerAlias<iBase>(this->scfObject);


    AddReftrackerAlias<I1>(this->scfObject);


    AddReftrackerAlias<I2>(this->scfObject);


    AddReftrackerAlias<I3>(this->scfObject);


    AddReftrackerAlias<I4>(this->scfObject);


    AddReftrackerAlias<I5>(this->scfObject);


    AddReftrackerAlias<I6>(this->scfObject);


    AddReftrackerAlias<I7>(this->scfObject);
  }

  void RemoveReftrackerAliases ()
  {

    RemoveReftrackerAlias<iBase>(this->scfObject);


    RemoveReftrackerAlias<I1>(this->scfObject);


    RemoveReftrackerAlias<I2>(this->scfObject);


    RemoveReftrackerAlias<I3>(this->scfObject);


    RemoveReftrackerAlias<I4>(this->scfObject);


    RemoveReftrackerAlias<I5>(this->scfObject);


    RemoveReftrackerAlias<I6>(this->scfObject);


    RemoveReftrackerAlias<I7>(this->scfObject);
  }
};
template<class Class ,class Super>
class CS_CRYSTALSPACE_EXPORT scfImplementationExt0 :
  public Super
 
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
    return Super::QueryInterface(id, version);
  }

protected:
  scfImplementationExt0(Class *object)
    : Super(), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1>
  scfImplementationExt0(Class *object, T1 t1)
    : Super(t1), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2>
  scfImplementationExt0(Class *object, T1 t1, T2 t2)
    : Super(t1, t2), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3>
  scfImplementationExt0(Class *object, T1 t1, T2 t2, T3 t3)
    : Super(t1, t2, t3), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4>
  scfImplementationExt0(Class *object, T1 t1, T2 t2, T3 t3, T4 t4)
    : Super(t1, t2, t3, t4), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4, class T5>
  scfImplementationExt0(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    : Super(t1, t2, t3, t4, t5), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4, class T5, class T6>
  scfImplementationExt0(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    : Super(t1, t2, t3, t4, t5, t6), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  Class *scfObject;


  virtual ~scfImplementationExt0()
  {
    RemoveReftrackerAliases();
  }

  typedef scfImplementationExt0<Class ,Super> scfImplementationType;
  typedef Class scfClassType;

private:
  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void* GetInterface (
    Class* scfObject, scfInterfaceID id, scfInterfaceVersion version)
  {
    if (id == scfInterfaceTraits<I>::GetID() &&
      scfCompatibleVersion(version, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject);
    }
    else
    {
      return 0;
    }
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void AddReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::AddAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void RemoveReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::RemoveAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  void AddReftrackerAliases ()
  {
  }

  void RemoveReftrackerAliases ()
  {
  }
};
template<class Class ,class Super ,class I1>
class CS_CRYSTALSPACE_EXPORT scfImplementationExt1 :
  public Super
  ,public I1
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {

    void *x;
    if((x = GetInterface<I1>(this->scfObject, id, version)) != 0) return x;
    return Super::QueryInterface(id, version);
  }

protected:
  scfImplementationExt1(Class *object)
    : Super(), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1>
  scfImplementationExt1(Class *object, T1 t1)
    : Super(t1), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2>
  scfImplementationExt1(Class *object, T1 t1, T2 t2)
    : Super(t1, t2), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3>
  scfImplementationExt1(Class *object, T1 t1, T2 t2, T3 t3)
    : Super(t1, t2, t3), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4>
  scfImplementationExt1(Class *object, T1 t1, T2 t2, T3 t3, T4 t4)
    : Super(t1, t2, t3, t4), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4, class T5>
  scfImplementationExt1(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    : Super(t1, t2, t3, t4, t5), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4, class T5, class T6>
  scfImplementationExt1(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    : Super(t1, t2, t3, t4, t5, t6), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  Class *scfObject;


  virtual ~scfImplementationExt1()
  {
    RemoveReftrackerAliases();
  }

  typedef scfImplementationExt1<Class ,Super ,I1> scfImplementationType;
  typedef Class scfClassType;

private:
  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void* GetInterface (
    Class* scfObject, scfInterfaceID id, scfInterfaceVersion version)
  {
    if (id == scfInterfaceTraits<I>::GetID() &&
      scfCompatibleVersion(version, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject);
    }
    else
    {
      return 0;
    }
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void AddReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::AddAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void RemoveReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::RemoveAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  void AddReftrackerAliases ()
  {




    AddReftrackerAlias<I1>(this->scfObject);
  }

  void RemoveReftrackerAliases ()
  {




    RemoveReftrackerAlias<I1>(this->scfObject);
  }
};
template<class Class ,class Super ,class I1, class I2>
class CS_CRYSTALSPACE_EXPORT scfImplementationExt2 :
  public Super
  ,public I1, public I2
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {

    void *x;
    if((x = GetInterface<I1>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I2>(this->scfObject, id, version)) != 0) return x;
    return Super::QueryInterface(id, version);
  }

protected:
  scfImplementationExt2(Class *object)
    : Super(), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1>
  scfImplementationExt2(Class *object, T1 t1)
    : Super(t1), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2>
  scfImplementationExt2(Class *object, T1 t1, T2 t2)
    : Super(t1, t2), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3>
  scfImplementationExt2(Class *object, T1 t1, T2 t2, T3 t3)
    : Super(t1, t2, t3), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4>
  scfImplementationExt2(Class *object, T1 t1, T2 t2, T3 t3, T4 t4)
    : Super(t1, t2, t3, t4), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4, class T5>
  scfImplementationExt2(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    : Super(t1, t2, t3, t4, t5), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4, class T5, class T6>
  scfImplementationExt2(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    : Super(t1, t2, t3, t4, t5, t6), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  Class *scfObject;


  virtual ~scfImplementationExt2()
  {
    RemoveReftrackerAliases();
  }

  typedef scfImplementationExt2<Class ,Super ,I1, I2> scfImplementationType;
  typedef Class scfClassType;

private:
  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void* GetInterface (
    Class* scfObject, scfInterfaceID id, scfInterfaceVersion version)
  {
    if (id == scfInterfaceTraits<I>::GetID() &&
      scfCompatibleVersion(version, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject);
    }
    else
    {
      return 0;
    }
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void AddReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::AddAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void RemoveReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::RemoveAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  void AddReftrackerAliases ()
  {




    AddReftrackerAlias<I1>(this->scfObject);


    AddReftrackerAlias<I2>(this->scfObject);
  }

  void RemoveReftrackerAliases ()
  {




    RemoveReftrackerAlias<I1>(this->scfObject);


    RemoveReftrackerAlias<I2>(this->scfObject);
  }
};
template<class Class ,class Super ,class I1, class I2, class I3>
class CS_CRYSTALSPACE_EXPORT scfImplementationExt3 :
  public Super
  ,public I1, public I2, public I3
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {

    void *x;
    if((x = GetInterface<I1>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I2>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I3>(this->scfObject, id, version)) != 0) return x;
    return Super::QueryInterface(id, version);
  }

protected:
  scfImplementationExt3(Class *object)
    : Super(), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1>
  scfImplementationExt3(Class *object, T1 t1)
    : Super(t1), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2>
  scfImplementationExt3(Class *object, T1 t1, T2 t2)
    : Super(t1, t2), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3>
  scfImplementationExt3(Class *object, T1 t1, T2 t2, T3 t3)
    : Super(t1, t2, t3), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4>
  scfImplementationExt3(Class *object, T1 t1, T2 t2, T3 t3, T4 t4)
    : Super(t1, t2, t3, t4), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4, class T5>
  scfImplementationExt3(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    : Super(t1, t2, t3, t4, t5), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4, class T5, class T6>
  scfImplementationExt3(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    : Super(t1, t2, t3, t4, t5, t6), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  Class *scfObject;


  virtual ~scfImplementationExt3()
  {
    RemoveReftrackerAliases();
  }

  typedef scfImplementationExt3<Class ,Super ,I1, I2, I3> scfImplementationType;
  typedef Class scfClassType;

private:
  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void* GetInterface (
    Class* scfObject, scfInterfaceID id, scfInterfaceVersion version)
  {
    if (id == scfInterfaceTraits<I>::GetID() &&
      scfCompatibleVersion(version, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject);
    }
    else
    {
      return 0;
    }
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void AddReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::AddAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void RemoveReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::RemoveAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  void AddReftrackerAliases ()
  {




    AddReftrackerAlias<I1>(this->scfObject);


    AddReftrackerAlias<I2>(this->scfObject);


    AddReftrackerAlias<I3>(this->scfObject);
  }

  void RemoveReftrackerAliases ()
  {




    RemoveReftrackerAlias<I1>(this->scfObject);


    RemoveReftrackerAlias<I2>(this->scfObject);


    RemoveReftrackerAlias<I3>(this->scfObject);
  }
};
template<class Class ,class Super ,class I1, class I2, class I3, class I4>
class CS_CRYSTALSPACE_EXPORT scfImplementationExt4 :
  public Super
  ,public I1, public I2, public I3, public I4
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {

    void *x;
    if((x = GetInterface<I1>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I2>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I3>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I4>(this->scfObject, id, version)) != 0) return x;
    return Super::QueryInterface(id, version);
  }

protected:
  scfImplementationExt4(Class *object)
    : Super(), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1>
  scfImplementationExt4(Class *object, T1 t1)
    : Super(t1), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2>
  scfImplementationExt4(Class *object, T1 t1, T2 t2)
    : Super(t1, t2), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3>
  scfImplementationExt4(Class *object, T1 t1, T2 t2, T3 t3)
    : Super(t1, t2, t3), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4>
  scfImplementationExt4(Class *object, T1 t1, T2 t2, T3 t3, T4 t4)
    : Super(t1, t2, t3, t4), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4, class T5>
  scfImplementationExt4(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    : Super(t1, t2, t3, t4, t5), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4, class T5, class T6>
  scfImplementationExt4(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    : Super(t1, t2, t3, t4, t5, t6), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  Class *scfObject;


  virtual ~scfImplementationExt4()
  {
    RemoveReftrackerAliases();
  }

  typedef scfImplementationExt4<Class ,Super ,I1, I2, I3, I4> scfImplementationType;
  typedef Class scfClassType;

private:
  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void* GetInterface (
    Class* scfObject, scfInterfaceID id, scfInterfaceVersion version)
  {
    if (id == scfInterfaceTraits<I>::GetID() &&
      scfCompatibleVersion(version, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject);
    }
    else
    {
      return 0;
    }
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void AddReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::AddAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void RemoveReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::RemoveAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  void AddReftrackerAliases ()
  {




    AddReftrackerAlias<I1>(this->scfObject);


    AddReftrackerAlias<I2>(this->scfObject);


    AddReftrackerAlias<I3>(this->scfObject);


    AddReftrackerAlias<I4>(this->scfObject);
  }

  void RemoveReftrackerAliases ()
  {




    RemoveReftrackerAlias<I1>(this->scfObject);


    RemoveReftrackerAlias<I2>(this->scfObject);


    RemoveReftrackerAlias<I3>(this->scfObject);


    RemoveReftrackerAlias<I4>(this->scfObject);
  }
};
template<class Class ,class Super ,class I1, class I2, class I3, class I4, class I5>
class CS_CRYSTALSPACE_EXPORT scfImplementationExt5 :
  public Super
  ,public I1, public I2, public I3, public I4, public I5
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {

    void *x;
    if((x = GetInterface<I1>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I2>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I3>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I4>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I5>(this->scfObject, id, version)) != 0) return x;
    return Super::QueryInterface(id, version);
  }

protected:
  scfImplementationExt5(Class *object)
    : Super(), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1>
  scfImplementationExt5(Class *object, T1 t1)
    : Super(t1), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2>
  scfImplementationExt5(Class *object, T1 t1, T2 t2)
    : Super(t1, t2), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3>
  scfImplementationExt5(Class *object, T1 t1, T2 t2, T3 t3)
    : Super(t1, t2, t3), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4>
  scfImplementationExt5(Class *object, T1 t1, T2 t2, T3 t3, T4 t4)
    : Super(t1, t2, t3, t4), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4, class T5>
  scfImplementationExt5(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    : Super(t1, t2, t3, t4, t5), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4, class T5, class T6>
  scfImplementationExt5(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    : Super(t1, t2, t3, t4, t5, t6), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  Class *scfObject;


  virtual ~scfImplementationExt5()
  {
    RemoveReftrackerAliases();
  }

  typedef scfImplementationExt5<Class ,Super ,I1, I2, I3, I4, I5> scfImplementationType;
  typedef Class scfClassType;

private:
  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void* GetInterface (
    Class* scfObject, scfInterfaceID id, scfInterfaceVersion version)
  {
    if (id == scfInterfaceTraits<I>::GetID() &&
      scfCompatibleVersion(version, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject);
    }
    else
    {
      return 0;
    }
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void AddReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::AddAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void RemoveReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::RemoveAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  void AddReftrackerAliases ()
  {




    AddReftrackerAlias<I1>(this->scfObject);


    AddReftrackerAlias<I2>(this->scfObject);


    AddReftrackerAlias<I3>(this->scfObject);


    AddReftrackerAlias<I4>(this->scfObject);


    AddReftrackerAlias<I5>(this->scfObject);
  }

  void RemoveReftrackerAliases ()
  {




    RemoveReftrackerAlias<I1>(this->scfObject);


    RemoveReftrackerAlias<I2>(this->scfObject);


    RemoveReftrackerAlias<I3>(this->scfObject);


    RemoveReftrackerAlias<I4>(this->scfObject);


    RemoveReftrackerAlias<I5>(this->scfObject);
  }
};
template<class Class ,class Super ,class I1, class I2, class I3, class I4, class I5, class I6>
class CS_CRYSTALSPACE_EXPORT scfImplementationExt6 :
  public Super
  ,public I1, public I2, public I3, public I4, public I5, public I6
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {

    void *x;
    if((x = GetInterface<I1>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I2>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I3>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I4>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I5>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I6>(this->scfObject, id, version)) != 0) return x;
    return Super::QueryInterface(id, version);
  }

protected:
  scfImplementationExt6(Class *object)
    : Super(), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1>
  scfImplementationExt6(Class *object, T1 t1)
    : Super(t1), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2>
  scfImplementationExt6(Class *object, T1 t1, T2 t2)
    : Super(t1, t2), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3>
  scfImplementationExt6(Class *object, T1 t1, T2 t2, T3 t3)
    : Super(t1, t2, t3), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4>
  scfImplementationExt6(Class *object, T1 t1, T2 t2, T3 t3, T4 t4)
    : Super(t1, t2, t3, t4), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4, class T5>
  scfImplementationExt6(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    : Super(t1, t2, t3, t4, t5), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4, class T5, class T6>
  scfImplementationExt6(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    : Super(t1, t2, t3, t4, t5, t6), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  Class *scfObject;


  virtual ~scfImplementationExt6()
  {
    RemoveReftrackerAliases();
  }

  typedef scfImplementationExt6<Class ,Super ,I1, I2, I3, I4, I5, I6> scfImplementationType;
  typedef Class scfClassType;

private:
  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void* GetInterface (
    Class* scfObject, scfInterfaceID id, scfInterfaceVersion version)
  {
    if (id == scfInterfaceTraits<I>::GetID() &&
      scfCompatibleVersion(version, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject);
    }
    else
    {
      return 0;
    }
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void AddReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::AddAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void RemoveReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::RemoveAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  void AddReftrackerAliases ()
  {




    AddReftrackerAlias<I1>(this->scfObject);


    AddReftrackerAlias<I2>(this->scfObject);


    AddReftrackerAlias<I3>(this->scfObject);


    AddReftrackerAlias<I4>(this->scfObject);


    AddReftrackerAlias<I5>(this->scfObject);


    AddReftrackerAlias<I6>(this->scfObject);
  }

  void RemoveReftrackerAliases ()
  {




    RemoveReftrackerAlias<I1>(this->scfObject);


    RemoveReftrackerAlias<I2>(this->scfObject);


    RemoveReftrackerAlias<I3>(this->scfObject);


    RemoveReftrackerAlias<I4>(this->scfObject);


    RemoveReftrackerAlias<I5>(this->scfObject);


    RemoveReftrackerAlias<I6>(this->scfObject);
  }
};
template<class Class ,class Super ,class I1, class I2, class I3, class I4, class I5, class I6, class I7>
class CS_CRYSTALSPACE_EXPORT scfImplementationExt7 :
  public Super
  ,public I1, public I2, public I3, public I4, public I5, public I6, public I7
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {

    void *x;
    if((x = GetInterface<I1>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I2>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I3>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I4>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I5>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I6>(this->scfObject, id, version)) != 0) return x;


    if((x = GetInterface<I7>(this->scfObject, id, version)) != 0) return x;
    return Super::QueryInterface(id, version);
  }

protected:
  scfImplementationExt7(Class *object)
    : Super(), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1>
  scfImplementationExt7(Class *object, T1 t1)
    : Super(t1), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2>
  scfImplementationExt7(Class *object, T1 t1, T2 t2)
    : Super(t1, t2), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3>
  scfImplementationExt7(Class *object, T1 t1, T2 t2, T3 t3)
    : Super(t1, t2, t3), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4>
  scfImplementationExt7(Class *object, T1 t1, T2 t2, T3 t3, T4 t4)
    : Super(t1, t2, t3, t4), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4, class T5>
  scfImplementationExt7(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    : Super(t1, t2, t3, t4, t5), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  template<class T1, class T2, class T3, class T4, class T5, class T6>
  scfImplementationExt7(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    : Super(t1, t2, t3, t4, t5, t6), scfObject(object)
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases();
  }

  Class *scfObject;


  virtual ~scfImplementationExt7()
  {
    RemoveReftrackerAliases();
  }

  typedef scfImplementationExt7<Class ,Super ,I1, I2, I3, I4, I5, I6, I7> scfImplementationType;
  typedef Class scfClassType;

private:
  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void* GetInterface (
    Class* scfObject, scfInterfaceID id, scfInterfaceVersion version)
  {
    if (id == scfInterfaceTraits<I>::GetID() &&
      scfCompatibleVersion(version, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject);
    }
    else
    {
      return 0;
    }
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void AddReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::AddAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void RemoveReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::RemoveAlias(
      static_cast<
        typename scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  void AddReftrackerAliases ()
  {




    AddReftrackerAlias<I1>(this->scfObject);


    AddReftrackerAlias<I2>(this->scfObject);


    AddReftrackerAlias<I3>(this->scfObject);


    AddReftrackerAlias<I4>(this->scfObject);


    AddReftrackerAlias<I5>(this->scfObject);


    AddReftrackerAlias<I6>(this->scfObject);


    AddReftrackerAlias<I7>(this->scfObject);
  }

  void RemoveReftrackerAliases ()
  {




    RemoveReftrackerAlias<I1>(this->scfObject);


    RemoveReftrackerAlias<I2>(this->scfObject);


    RemoveReftrackerAlias<I3>(this->scfObject);


    RemoveReftrackerAlias<I4>(this->scfObject);


    RemoveReftrackerAlias<I5>(this->scfObject);


    RemoveReftrackerAlias<I6>(this->scfObject);


    RemoveReftrackerAlias<I7>(this->scfObject);
  }
};

#include "csutil/win32/msvc_deprecated_warn_on.h"
