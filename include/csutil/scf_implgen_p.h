
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
 * bin/gen-scf_impl.sh. Rerun this script if you wish to apply changes made
 * to any of the scf_impl* files.
 *
 * Note that the generation incurs C preprocessing; thus most of this file
 * is devoid of comments - check the original scf_impl* files for possible
 * notes and comments.
 */
#if !defined(SCF_IN_IMPLEMENTATION_H)
#error Do not include this file directly. Included from scf_implementation.h
#endif
// This is a big header, so help MSVC a bit
#ifdef CS_COMPILER_MSVC 
#pragma once
#endif
#include "csutil/deprecated_warn_off.h"
/* Pre-preprocessed code starts here */
template<class Class >
class scfImplementation0 :
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
  virtual size_t GetInterfaceMetadataCount () const
  {
    return 0 + scfImplementation<Class>::GetInterfaceMetadataCount ();
  }
  virtual void FillInterfaceMetadata (size_t n)
  {
    scfImplementation<Class>::FillInterfaceMetadata (n);
  }
private:
  void AddReftrackerAliases ()
  {
    AddReftrackerAlias<iBase>(this->GetSCFObject());
  }
  void RemoveReftrackerAliases ()
  {
    RemoveReftrackerAlias<iBase>(this->GetSCFObject());
  }
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
  template<typename IF>
  CS_FORCEINLINE_TEMPLATEMETHOD static void FillInterfaceMetadataIf (
    scfInterfaceMetadata* metadataArray, size_t pos)
  {
    metadataArray[pos].interfaceName = scfInterfaceTraits<IF>::GetName ();
    metadataArray[pos].interfaceID = scfInterfaceTraits<IF>::GetID ();
    metadataArray[pos].interfaceVersion = scfInterfaceTraits<IF>::GetVersion ();
  }
};
template<class Class ,class I1>
class scfImplementation1 :
  public scfImplementation<Class>
  ,public I1
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
    void *x;
    if((x = GetInterface<I1>(this->GetSCFObject(), id, version)) != 0) return x;
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
  virtual size_t GetInterfaceMetadataCount () const
  {
    return 1 + scfImplementation<Class>::GetInterfaceMetadataCount ();
  }
  virtual void FillInterfaceMetadata (size_t n)
  {
    FillInterfaceMetadataIf<I1> (this->scfAuxData->metadataList->metadata, n++);
    scfImplementation<Class>::FillInterfaceMetadata (n);
  }
private:
  void AddReftrackerAliases ()
  {
    AddReftrackerAlias<iBase>(this->GetSCFObject());
    AddReftrackerAlias<I1>(this->GetSCFObject());
  }
  void RemoveReftrackerAliases ()
  {
    RemoveReftrackerAlias<iBase>(this->GetSCFObject());
    RemoveReftrackerAlias<I1>(this->GetSCFObject());
  }
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
  template<typename IF>
  CS_FORCEINLINE_TEMPLATEMETHOD static void FillInterfaceMetadataIf (
    scfInterfaceMetadata* metadataArray, size_t pos)
  {
    metadataArray[pos].interfaceName = scfInterfaceTraits<IF>::GetName ();
    metadataArray[pos].interfaceID = scfInterfaceTraits<IF>::GetID ();
    metadataArray[pos].interfaceVersion = scfInterfaceTraits<IF>::GetVersion ();
  }
};
template<class Class ,class I1, class I2>
class scfImplementation2 :
  public scfImplementation<Class>
  ,public I1, public I2
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
    void *x;
    if((x = GetInterface<I1>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I2>(this->GetSCFObject(), id, version)) != 0) return x;
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
  virtual size_t GetInterfaceMetadataCount () const
  {
    return 2 + scfImplementation<Class>::GetInterfaceMetadataCount ();
  }
  virtual void FillInterfaceMetadata (size_t n)
  {
    FillInterfaceMetadataIf<I1> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I2> (this->scfAuxData->metadataList->metadata, n++);
    scfImplementation<Class>::FillInterfaceMetadata (n);
  }
private:
  void AddReftrackerAliases ()
  {
    AddReftrackerAlias<iBase>(this->GetSCFObject());
    AddReftrackerAlias<I1>(this->GetSCFObject());
    AddReftrackerAlias<I2>(this->GetSCFObject());
  }
  void RemoveReftrackerAliases ()
  {
    RemoveReftrackerAlias<iBase>(this->GetSCFObject());
    RemoveReftrackerAlias<I1>(this->GetSCFObject());
    RemoveReftrackerAlias<I2>(this->GetSCFObject());
  }
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
  template<typename IF>
  CS_FORCEINLINE_TEMPLATEMETHOD static void FillInterfaceMetadataIf (
    scfInterfaceMetadata* metadataArray, size_t pos)
  {
    metadataArray[pos].interfaceName = scfInterfaceTraits<IF>::GetName ();
    metadataArray[pos].interfaceID = scfInterfaceTraits<IF>::GetID ();
    metadataArray[pos].interfaceVersion = scfInterfaceTraits<IF>::GetVersion ();
  }
};
template<class Class ,class I1, class I2, class I3>
class scfImplementation3 :
  public scfImplementation<Class>
  ,public I1, public I2, public I3
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
    void *x;
    if((x = GetInterface<I1>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I2>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I3>(this->GetSCFObject(), id, version)) != 0) return x;
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
  virtual size_t GetInterfaceMetadataCount () const
  {
    return 3 + scfImplementation<Class>::GetInterfaceMetadataCount ();
  }
  virtual void FillInterfaceMetadata (size_t n)
  {
    FillInterfaceMetadataIf<I1> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I2> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I3> (this->scfAuxData->metadataList->metadata, n++);
    scfImplementation<Class>::FillInterfaceMetadata (n);
  }
private:
  void AddReftrackerAliases ()
  {
    AddReftrackerAlias<iBase>(this->GetSCFObject());


    AddReftrackerAlias<I1>(this->GetSCFObject());


    AddReftrackerAlias<I2>(this->GetSCFObject());


    AddReftrackerAlias<I3>(this->GetSCFObject());
  }
  void RemoveReftrackerAliases ()
  {
    RemoveReftrackerAlias<iBase>(this->GetSCFObject());
    RemoveReftrackerAlias<I1>(this->GetSCFObject());
    RemoveReftrackerAlias<I2>(this->GetSCFObject());
    RemoveReftrackerAlias<I3>(this->GetSCFObject());
  }
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
  template<typename IF>
  CS_FORCEINLINE_TEMPLATEMETHOD static void FillInterfaceMetadataIf (
    scfInterfaceMetadata* metadataArray, size_t pos)
  {
    metadataArray[pos].interfaceName = scfInterfaceTraits<IF>::GetName ();
    metadataArray[pos].interfaceID = scfInterfaceTraits<IF>::GetID ();
    metadataArray[pos].interfaceVersion = scfInterfaceTraits<IF>::GetVersion ();
  }
};
template<class Class ,class I1, class I2, class I3, class I4>
class scfImplementation4 :
  public scfImplementation<Class>
  ,public I1, public I2, public I3, public I4
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
    void *x;
    if((x = GetInterface<I1>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I2>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I3>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I4>(this->GetSCFObject(), id, version)) != 0) return x;
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
  virtual size_t GetInterfaceMetadataCount () const
  {
    return 4 + scfImplementation<Class>::GetInterfaceMetadataCount ();
  }
  virtual void FillInterfaceMetadata (size_t n)
  {
    FillInterfaceMetadataIf<I1> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I2> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I3> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I4> (this->scfAuxData->metadataList->metadata, n++);
    scfImplementation<Class>::FillInterfaceMetadata (n);
  }
private:
  void AddReftrackerAliases ()
  {
    AddReftrackerAlias<iBase>(this->GetSCFObject());
    AddReftrackerAlias<I1>(this->GetSCFObject());
    AddReftrackerAlias<I2>(this->GetSCFObject());
    AddReftrackerAlias<I3>(this->GetSCFObject());
    AddReftrackerAlias<I4>(this->GetSCFObject());
  }
  void RemoveReftrackerAliases ()
  {
    RemoveReftrackerAlias<iBase>(this->GetSCFObject());
    RemoveReftrackerAlias<I1>(this->GetSCFObject());
    RemoveReftrackerAlias<I2>(this->GetSCFObject());
    RemoveReftrackerAlias<I3>(this->GetSCFObject());
    RemoveReftrackerAlias<I4>(this->GetSCFObject());
  }
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
  template<typename IF>
  CS_FORCEINLINE_TEMPLATEMETHOD static void FillInterfaceMetadataIf (
    scfInterfaceMetadata* metadataArray, size_t pos)
  {
    metadataArray[pos].interfaceName = scfInterfaceTraits<IF>::GetName ();
    metadataArray[pos].interfaceID = scfInterfaceTraits<IF>::GetID ();
    metadataArray[pos].interfaceVersion = scfInterfaceTraits<IF>::GetVersion ();
  }
};
template<class Class ,class I1, class I2, class I3, class I4, class I5>
class scfImplementation5 :
  public scfImplementation<Class>
  ,public I1, public I2, public I3, public I4, public I5
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
    void *x;
    if((x = GetInterface<I1>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I2>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I3>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I4>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I5>(this->GetSCFObject(), id, version)) != 0) return x;
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
  virtual size_t GetInterfaceMetadataCount () const
  {
    return 5 + scfImplementation<Class>::GetInterfaceMetadataCount ();
  }
  virtual void FillInterfaceMetadata (size_t n)
  {
    FillInterfaceMetadataIf<I1> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I2> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I3> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I4> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I5> (this->scfAuxData->metadataList->metadata, n++);
    scfImplementation<Class>::FillInterfaceMetadata (n);
  }
private:
  void AddReftrackerAliases ()
  {
    AddReftrackerAlias<iBase>(this->GetSCFObject());
    AddReftrackerAlias<I1>(this->GetSCFObject());
    AddReftrackerAlias<I2>(this->GetSCFObject());
    AddReftrackerAlias<I3>(this->GetSCFObject());
    AddReftrackerAlias<I4>(this->GetSCFObject());
    AddReftrackerAlias<I5>(this->GetSCFObject());
  }
  void RemoveReftrackerAliases ()
  {
    RemoveReftrackerAlias<iBase>(this->GetSCFObject());
    RemoveReftrackerAlias<I1>(this->GetSCFObject());
    RemoveReftrackerAlias<I2>(this->GetSCFObject());
    RemoveReftrackerAlias<I3>(this->GetSCFObject());
    RemoveReftrackerAlias<I4>(this->GetSCFObject());
    RemoveReftrackerAlias<I5>(this->GetSCFObject());
  }
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
  template<typename IF>
  CS_FORCEINLINE_TEMPLATEMETHOD static void FillInterfaceMetadataIf (
    scfInterfaceMetadata* metadataArray, size_t pos)
  {
    metadataArray[pos].interfaceName = scfInterfaceTraits<IF>::GetName ();
    metadataArray[pos].interfaceID = scfInterfaceTraits<IF>::GetID ();
    metadataArray[pos].interfaceVersion = scfInterfaceTraits<IF>::GetVersion ();
  }
};
template<class Class ,class I1, class I2, class I3, class I4, class I5, class I6>
class scfImplementation6 :
  public scfImplementation<Class>
  ,public I1, public I2, public I3, public I4, public I5, public I6
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
    void *x;
    if((x = GetInterface<I1>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I2>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I3>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I4>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I5>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I6>(this->GetSCFObject(), id, version)) != 0) return x;
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
  virtual size_t GetInterfaceMetadataCount () const
  {
    return 6 + scfImplementation<Class>::GetInterfaceMetadataCount ();
  }
  virtual void FillInterfaceMetadata (size_t n)
  {
    FillInterfaceMetadataIf<I1> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I2> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I3> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I4> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I5> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I6> (this->scfAuxData->metadataList->metadata, n++);
    scfImplementation<Class>::FillInterfaceMetadata (n);
  }
private:
  void AddReftrackerAliases ()
  {
    AddReftrackerAlias<iBase>(this->GetSCFObject());
    AddReftrackerAlias<I1>(this->GetSCFObject());
    AddReftrackerAlias<I2>(this->GetSCFObject());
    AddReftrackerAlias<I3>(this->GetSCFObject());
    AddReftrackerAlias<I4>(this->GetSCFObject());
    AddReftrackerAlias<I5>(this->GetSCFObject());
    AddReftrackerAlias<I6>(this->GetSCFObject());
  }
  void RemoveReftrackerAliases ()
  {
    RemoveReftrackerAlias<iBase>(this->GetSCFObject());
    RemoveReftrackerAlias<I1>(this->GetSCFObject());
    RemoveReftrackerAlias<I2>(this->GetSCFObject());
    RemoveReftrackerAlias<I3>(this->GetSCFObject());
    RemoveReftrackerAlias<I4>(this->GetSCFObject());
    RemoveReftrackerAlias<I5>(this->GetSCFObject());
    RemoveReftrackerAlias<I6>(this->GetSCFObject());
  }
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
  template<typename IF>
  CS_FORCEINLINE_TEMPLATEMETHOD static void FillInterfaceMetadataIf (
    scfInterfaceMetadata* metadataArray, size_t pos)
  {
    metadataArray[pos].interfaceName = scfInterfaceTraits<IF>::GetName ();
    metadataArray[pos].interfaceID = scfInterfaceTraits<IF>::GetID ();
    metadataArray[pos].interfaceVersion = scfInterfaceTraits<IF>::GetVersion ();
  }
};
template<class Class ,class I1, class I2, class I3, class I4, class I5, class I6, class I7>
class scfImplementation7 :
  public scfImplementation<Class>
  ,public I1, public I2, public I3, public I4, public I5, public I6, public I7
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
    void *x;
    if((x = GetInterface<I1>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I2>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I3>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I4>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I5>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I6>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I7>(this->GetSCFObject(), id, version)) != 0) return x;
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
  virtual size_t GetInterfaceMetadataCount () const
  {
    return 7 + scfImplementation<Class>::GetInterfaceMetadataCount ();
  }
  virtual void FillInterfaceMetadata (size_t n)
  {
    FillInterfaceMetadataIf<I1> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I2> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I3> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I4> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I5> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I6> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I7> (this->scfAuxData->metadataList->metadata, n++);
    scfImplementation<Class>::FillInterfaceMetadata (n);
  }
private:
  void AddReftrackerAliases ()
  {
    AddReftrackerAlias<iBase>(this->GetSCFObject());
    AddReftrackerAlias<I1>(this->GetSCFObject());
    AddReftrackerAlias<I2>(this->GetSCFObject());
    AddReftrackerAlias<I3>(this->GetSCFObject());
    AddReftrackerAlias<I4>(this->GetSCFObject());
    AddReftrackerAlias<I5>(this->GetSCFObject());
    AddReftrackerAlias<I6>(this->GetSCFObject());
    AddReftrackerAlias<I7>(this->GetSCFObject());
  }
  void RemoveReftrackerAliases ()
  {
    RemoveReftrackerAlias<iBase>(this->GetSCFObject());
    RemoveReftrackerAlias<I1>(this->GetSCFObject());
    RemoveReftrackerAlias<I2>(this->GetSCFObject());
    RemoveReftrackerAlias<I3>(this->GetSCFObject());
    RemoveReftrackerAlias<I4>(this->GetSCFObject());
    RemoveReftrackerAlias<I5>(this->GetSCFObject());
    RemoveReftrackerAlias<I6>(this->GetSCFObject());
    RemoveReftrackerAlias<I7>(this->GetSCFObject());
  }
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
  template<typename IF>
  CS_FORCEINLINE_TEMPLATEMETHOD static void FillInterfaceMetadataIf (
    scfInterfaceMetadata* metadataArray, size_t pos)
  {
    metadataArray[pos].interfaceName = scfInterfaceTraits<IF>::GetName ();
    metadataArray[pos].interfaceID = scfInterfaceTraits<IF>::GetID ();
    metadataArray[pos].interfaceVersion = scfInterfaceTraits<IF>::GetVersion ();
  }
};
template<class Class ,class Super>
class scfImplementationExt0 :
  public Super
 
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
    return Super::QueryInterface(id, version);
  }
protected:
  template<class Class_>
  scfImplementationExt0(Class_ *object)
    : Super()
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1>
  scfImplementationExt0(Class *object, T1 t1)
    : Super(t1)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2>
  scfImplementationExt0(Class *object, T1 t1, T2 t2)
    : Super(t1, t2)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3>
  scfImplementationExt0(Class *object, T1 t1, T2 t2, T3 t3)
    : Super(t1, t2, t3)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4>
  scfImplementationExt0(Class *object, T1 t1, T2 t2, T3 t3, T4 t4)
    : Super(t1, t2, t3, t4)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4, class T5>
  scfImplementationExt0(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    : Super(t1, t2, t3, t4, t5)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4, class T5, class T6>
  scfImplementationExt0(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    : Super(t1, t2, t3, t4, t5, t6)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  Class* GetSCFObject() { return static_cast<Class*> (this); }
  const Class* GetSCFObject() const { return static_cast<const Class*> (this); }
  virtual ~scfImplementationExt0()
  {
    RemoveReftrackerAliases();
  }
  typedef scfImplementationExt0<Class ,Super> scfImplementationType;
  typedef Class scfClassType;
  virtual size_t GetInterfaceMetadataCount () const
  {
    return 0 + Super::GetInterfaceMetadataCount ();
  }
  virtual void FillInterfaceMetadata (size_t n)
  {
    Super::FillInterfaceMetadata (n);
  }
private:
  void AddReftrackerAliases ()
  {
    csRefTrackerAccess::AddAlias(this->GetSCFObject(),
     static_cast<Super*> (this->GetSCFObject()));
  }
  void RemoveReftrackerAliases ()
  {
    csRefTrackerAccess::RemoveAlias(this->GetSCFObject(),
        static_cast<Super*> (this->GetSCFObject()));
  }
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
  template<typename IF>
  CS_FORCEINLINE_TEMPLATEMETHOD static void FillInterfaceMetadataIf (
    scfInterfaceMetadata* metadataArray, size_t pos)
  {
    metadataArray[pos].interfaceName = scfInterfaceTraits<IF>::GetName ();
    metadataArray[pos].interfaceID = scfInterfaceTraits<IF>::GetID ();
    metadataArray[pos].interfaceVersion = scfInterfaceTraits<IF>::GetVersion ();
  }
};
template<class Class ,class Super ,class I1>
class scfImplementationExt1 :
  public Super
  ,public I1
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
    void *x;
    if((x = GetInterface<I1>(this->GetSCFObject(), id, version)) != 0) return x;
    return Super::QueryInterface(id, version);
  }
protected:
  template<class Class_>
  scfImplementationExt1(Class_ *object)
    : Super()
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1>
  scfImplementationExt1(Class *object, T1 t1)
    : Super(t1)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2>
  scfImplementationExt1(Class *object, T1 t1, T2 t2)
    : Super(t1, t2)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3>
  scfImplementationExt1(Class *object, T1 t1, T2 t2, T3 t3)
    : Super(t1, t2, t3)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4>
  scfImplementationExt1(Class *object, T1 t1, T2 t2, T3 t3, T4 t4)
    : Super(t1, t2, t3, t4)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4, class T5>
  scfImplementationExt1(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    : Super(t1, t2, t3, t4, t5)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4, class T5, class T6>
  scfImplementationExt1(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    : Super(t1, t2, t3, t4, t5, t6)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  Class* GetSCFObject() { return static_cast<Class*> (this); }
  const Class* GetSCFObject() const { return static_cast<const Class*> (this); }
  virtual ~scfImplementationExt1()
  {
    RemoveReftrackerAliases();
  }
  typedef scfImplementationExt1<Class ,Super ,I1> scfImplementationType;
  typedef Class scfClassType;
  virtual size_t GetInterfaceMetadataCount () const
  {
    return 1 + Super::GetInterfaceMetadataCount ();
  }
  virtual void FillInterfaceMetadata (size_t n)
  {
    FillInterfaceMetadataIf<I1> (this->scfAuxData->metadataList->metadata, n++);
    Super::FillInterfaceMetadata (n);
  }
private:
  void AddReftrackerAliases ()
  {
    csRefTrackerAccess::AddAlias(this->GetSCFObject(),
     static_cast<Super*> (this->GetSCFObject()));
    AddReftrackerAlias<I1>(this->GetSCFObject());
  }
  void RemoveReftrackerAliases ()
  {
    csRefTrackerAccess::RemoveAlias(this->GetSCFObject(),
        static_cast<Super*> (this->GetSCFObject()));
    RemoveReftrackerAlias<I1>(this->GetSCFObject());
  }
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
  template<typename IF>
  CS_FORCEINLINE_TEMPLATEMETHOD static void FillInterfaceMetadataIf (
    scfInterfaceMetadata* metadataArray, size_t pos)
  {
    metadataArray[pos].interfaceName = scfInterfaceTraits<IF>::GetName ();
    metadataArray[pos].interfaceID = scfInterfaceTraits<IF>::GetID ();
    metadataArray[pos].interfaceVersion = scfInterfaceTraits<IF>::GetVersion ();
  }
};
template<class Class ,class Super ,class I1, class I2>
class scfImplementationExt2 :
  public Super
  ,public I1, public I2
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
    void *x;
    if((x = GetInterface<I1>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I2>(this->GetSCFObject(), id, version)) != 0) return x;
    return Super::QueryInterface(id, version);
  }
protected:
  template<class Class_>
  scfImplementationExt2(Class_ *object)
    : Super()
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1>
  scfImplementationExt2(Class *object, T1 t1)
    : Super(t1)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2>
  scfImplementationExt2(Class *object, T1 t1, T2 t2)
    : Super(t1, t2)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3>
  scfImplementationExt2(Class *object, T1 t1, T2 t2, T3 t3)
    : Super(t1, t2, t3)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4>
  scfImplementationExt2(Class *object, T1 t1, T2 t2, T3 t3, T4 t4)
    : Super(t1, t2, t3, t4)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4, class T5>
  scfImplementationExt2(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    : Super(t1, t2, t3, t4, t5)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4, class T5, class T6>
  scfImplementationExt2(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    : Super(t1, t2, t3, t4, t5, t6)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  Class* GetSCFObject() { return static_cast<Class*> (this); }
  const Class* GetSCFObject() const { return static_cast<const Class*> (this); }
  virtual ~scfImplementationExt2()
  {
    RemoveReftrackerAliases();
  }
  typedef scfImplementationExt2<Class ,Super ,I1, I2> scfImplementationType;
  typedef Class scfClassType;
  virtual size_t GetInterfaceMetadataCount () const
  {
    return 2 + Super::GetInterfaceMetadataCount ();
  }
  virtual void FillInterfaceMetadata (size_t n)
  {
    FillInterfaceMetadataIf<I1> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I2> (this->scfAuxData->metadataList->metadata, n++);
    Super::FillInterfaceMetadata (n);
  }
private:
  void AddReftrackerAliases ()
  {
    csRefTrackerAccess::AddAlias(this->GetSCFObject(),
     static_cast<Super*> (this->GetSCFObject()));
    AddReftrackerAlias<I1>(this->GetSCFObject());
    AddReftrackerAlias<I2>(this->GetSCFObject());
  }
  void RemoveReftrackerAliases ()
  {
    csRefTrackerAccess::RemoveAlias(this->GetSCFObject(),
        static_cast<Super*> (this->GetSCFObject()));
    RemoveReftrackerAlias<I1>(this->GetSCFObject());
    RemoveReftrackerAlias<I2>(this->GetSCFObject());
  }
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
  template<typename IF>
  CS_FORCEINLINE_TEMPLATEMETHOD static void FillInterfaceMetadataIf (
    scfInterfaceMetadata* metadataArray, size_t pos)
  {
    metadataArray[pos].interfaceName = scfInterfaceTraits<IF>::GetName ();
    metadataArray[pos].interfaceID = scfInterfaceTraits<IF>::GetID ();
    metadataArray[pos].interfaceVersion = scfInterfaceTraits<IF>::GetVersion ();
  }
};
template<class Class ,class Super ,class I1, class I2, class I3>
class scfImplementationExt3 :
  public Super
  ,public I1, public I2, public I3
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
    void *x;
    if((x = GetInterface<I1>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I2>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I3>(this->GetSCFObject(), id, version)) != 0) return x;
    return Super::QueryInterface(id, version);
  }
protected:
  template<class Class_>
  scfImplementationExt3(Class_ *object)
    : Super()
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1>
  scfImplementationExt3(Class *object, T1 t1)
    : Super(t1)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2>
  scfImplementationExt3(Class *object, T1 t1, T2 t2)
    : Super(t1, t2)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3>
  scfImplementationExt3(Class *object, T1 t1, T2 t2, T3 t3)
    : Super(t1, t2, t3)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4>
  scfImplementationExt3(Class *object, T1 t1, T2 t2, T3 t3, T4 t4)
    : Super(t1, t2, t3, t4)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4, class T5>
  scfImplementationExt3(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    : Super(t1, t2, t3, t4, t5)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4, class T5, class T6>
  scfImplementationExt3(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    : Super(t1, t2, t3, t4, t5, t6)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  Class* GetSCFObject() { return static_cast<Class*> (this); }
  const Class* GetSCFObject() const { return static_cast<const Class*> (this); }
  virtual ~scfImplementationExt3()
  {
    RemoveReftrackerAliases();
  }
  typedef scfImplementationExt3<Class ,Super ,I1, I2, I3> scfImplementationType;
  typedef Class scfClassType;
  virtual size_t GetInterfaceMetadataCount () const
  {
    return 3 + Super::GetInterfaceMetadataCount ();
  }
  virtual void FillInterfaceMetadata (size_t n)
  {
    FillInterfaceMetadataIf<I1> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I2> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I3> (this->scfAuxData->metadataList->metadata, n++);
    Super::FillInterfaceMetadata (n);
  }
private:
  void AddReftrackerAliases ()
  {
    csRefTrackerAccess::AddAlias(this->GetSCFObject(),
     static_cast<Super*> (this->GetSCFObject()));
    AddReftrackerAlias<I1>(this->GetSCFObject());
    AddReftrackerAlias<I2>(this->GetSCFObject());
    AddReftrackerAlias<I3>(this->GetSCFObject());
  }
  void RemoveReftrackerAliases ()
  {
    csRefTrackerAccess::RemoveAlias(this->GetSCFObject(),
        static_cast<Super*> (this->GetSCFObject()));
    RemoveReftrackerAlias<I1>(this->GetSCFObject());
    RemoveReftrackerAlias<I2>(this->GetSCFObject());
    RemoveReftrackerAlias<I3>(this->GetSCFObject());
  }
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
  template<typename IF>
  CS_FORCEINLINE_TEMPLATEMETHOD static void FillInterfaceMetadataIf (
    scfInterfaceMetadata* metadataArray, size_t pos)
  {
    metadataArray[pos].interfaceName = scfInterfaceTraits<IF>::GetName ();
    metadataArray[pos].interfaceID = scfInterfaceTraits<IF>::GetID ();
    metadataArray[pos].interfaceVersion = scfInterfaceTraits<IF>::GetVersion ();
  }
};
template<class Class ,class Super ,class I1, class I2, class I3, class I4>
class scfImplementationExt4 :
  public Super
  ,public I1, public I2, public I3, public I4
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
    void *x;
    if((x = GetInterface<I1>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I2>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I3>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I4>(this->GetSCFObject(), id, version)) != 0) return x;
    return Super::QueryInterface(id, version);
  }
protected:
  template<class Class_>
  scfImplementationExt4(Class_ *object)
    : Super()
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1>
  scfImplementationExt4(Class *object, T1 t1)
    : Super(t1)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2>
  scfImplementationExt4(Class *object, T1 t1, T2 t2)
    : Super(t1, t2)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3>
  scfImplementationExt4(Class *object, T1 t1, T2 t2, T3 t3)
    : Super(t1, t2, t3)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4>
  scfImplementationExt4(Class *object, T1 t1, T2 t2, T3 t3, T4 t4)
    : Super(t1, t2, t3, t4)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4, class T5>
  scfImplementationExt4(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    : Super(t1, t2, t3, t4, t5)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4, class T5, class T6>
  scfImplementationExt4(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    : Super(t1, t2, t3, t4, t5, t6)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  Class* GetSCFObject() { return static_cast<Class*> (this); }
  const Class* GetSCFObject() const { return static_cast<const Class*> (this); }
  virtual ~scfImplementationExt4()
  {
    RemoveReftrackerAliases();
  }
  typedef scfImplementationExt4<Class ,Super ,I1, I2, I3, I4> scfImplementationType;
  typedef Class scfClassType;
  virtual size_t GetInterfaceMetadataCount () const
  {
    return 4 + Super::GetInterfaceMetadataCount ();
  }
  virtual void FillInterfaceMetadata (size_t n)
  {
    FillInterfaceMetadataIf<I1> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I2> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I3> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I4> (this->scfAuxData->metadataList->metadata, n++);
    Super::FillInterfaceMetadata (n);
  }
private:
  void AddReftrackerAliases ()
  {
    csRefTrackerAccess::AddAlias(this->GetSCFObject(),
     static_cast<Super*> (this->GetSCFObject()));
    AddReftrackerAlias<I1>(this->GetSCFObject());
    AddReftrackerAlias<I2>(this->GetSCFObject());
    AddReftrackerAlias<I3>(this->GetSCFObject());
    AddReftrackerAlias<I4>(this->GetSCFObject());
  }
  void RemoveReftrackerAliases ()
  {
    csRefTrackerAccess::RemoveAlias(this->GetSCFObject(),
        static_cast<Super*> (this->GetSCFObject()));
    RemoveReftrackerAlias<I1>(this->GetSCFObject());
    RemoveReftrackerAlias<I2>(this->GetSCFObject());
    RemoveReftrackerAlias<I3>(this->GetSCFObject());
    RemoveReftrackerAlias<I4>(this->GetSCFObject());
  }
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
  template<typename IF>
  CS_FORCEINLINE_TEMPLATEMETHOD static void FillInterfaceMetadataIf (
    scfInterfaceMetadata* metadataArray, size_t pos)
  {
    metadataArray[pos].interfaceName = scfInterfaceTraits<IF>::GetName ();
    metadataArray[pos].interfaceID = scfInterfaceTraits<IF>::GetID ();
    metadataArray[pos].interfaceVersion = scfInterfaceTraits<IF>::GetVersion ();
  }
};
template<class Class ,class Super ,class I1, class I2, class I3, class I4, class I5>
class scfImplementationExt5 :
  public Super
  ,public I1, public I2, public I3, public I4, public I5
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
    void *x;
    if((x = GetInterface<I1>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I2>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I3>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I4>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I5>(this->GetSCFObject(), id, version)) != 0) return x;
    return Super::QueryInterface(id, version);
  }
protected:
  template<class Class_>
  scfImplementationExt5(Class_ *object)
    : Super()
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1>
  scfImplementationExt5(Class *object, T1 t1)
    : Super(t1)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2>
  scfImplementationExt5(Class *object, T1 t1, T2 t2)
    : Super(t1, t2)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3>
  scfImplementationExt5(Class *object, T1 t1, T2 t2, T3 t3)
    : Super(t1, t2, t3)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4>
  scfImplementationExt5(Class *object, T1 t1, T2 t2, T3 t3, T4 t4)
    : Super(t1, t2, t3, t4)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4, class T5>
  scfImplementationExt5(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    : Super(t1, t2, t3, t4, t5)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4, class T5, class T6>
  scfImplementationExt5(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    : Super(t1, t2, t3, t4, t5, t6)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  Class* GetSCFObject() { return static_cast<Class*> (this); }
  const Class* GetSCFObject() const { return static_cast<const Class*> (this); }
  virtual ~scfImplementationExt5()
  {
    RemoveReftrackerAliases();
  }
  typedef scfImplementationExt5<Class ,Super ,I1, I2, I3, I4, I5> scfImplementationType;
  typedef Class scfClassType;
  virtual size_t GetInterfaceMetadataCount () const
  {
    return 5 + Super::GetInterfaceMetadataCount ();
  }
  virtual void FillInterfaceMetadata (size_t n)
  {
    FillInterfaceMetadataIf<I1> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I2> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I3> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I4> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I5> (this->scfAuxData->metadataList->metadata, n++);
    Super::FillInterfaceMetadata (n);
  }
private:
  void AddReftrackerAliases ()
  {
    csRefTrackerAccess::AddAlias(this->GetSCFObject(),
     static_cast<Super*> (this->GetSCFObject()));
    AddReftrackerAlias<I1>(this->GetSCFObject());
    AddReftrackerAlias<I2>(this->GetSCFObject());
    AddReftrackerAlias<I3>(this->GetSCFObject());
    AddReftrackerAlias<I4>(this->GetSCFObject());
    AddReftrackerAlias<I5>(this->GetSCFObject());
  }
  void RemoveReftrackerAliases ()
  {
    csRefTrackerAccess::RemoveAlias(this->GetSCFObject(),
        static_cast<Super*> (this->GetSCFObject()));
    RemoveReftrackerAlias<I1>(this->GetSCFObject());
    RemoveReftrackerAlias<I2>(this->GetSCFObject());
    RemoveReftrackerAlias<I3>(this->GetSCFObject());
    RemoveReftrackerAlias<I4>(this->GetSCFObject());
    RemoveReftrackerAlias<I5>(this->GetSCFObject());
  }
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
  template<typename IF>
  CS_FORCEINLINE_TEMPLATEMETHOD static void FillInterfaceMetadataIf (
    scfInterfaceMetadata* metadataArray, size_t pos)
  {
    metadataArray[pos].interfaceName = scfInterfaceTraits<IF>::GetName ();
    metadataArray[pos].interfaceID = scfInterfaceTraits<IF>::GetID ();
    metadataArray[pos].interfaceVersion = scfInterfaceTraits<IF>::GetVersion ();
  }
};
template<class Class ,class Super ,class I1, class I2, class I3, class I4, class I5, class I6>
class scfImplementationExt6 :
  public Super
  ,public I1, public I2, public I3, public I4, public I5, public I6
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
    void *x;
    if((x = GetInterface<I1>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I2>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I3>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I4>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I5>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I6>(this->GetSCFObject(), id, version)) != 0) return x;
    return Super::QueryInterface(id, version);
  }
protected:
  template<class Class_>
  scfImplementationExt6(Class_ *object)
    : Super()
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1>
  scfImplementationExt6(Class *object, T1 t1)
    : Super(t1)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2>
  scfImplementationExt6(Class *object, T1 t1, T2 t2)
    : Super(t1, t2)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3>
  scfImplementationExt6(Class *object, T1 t1, T2 t2, T3 t3)
    : Super(t1, t2, t3)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4>
  scfImplementationExt6(Class *object, T1 t1, T2 t2, T3 t3, T4 t4)
    : Super(t1, t2, t3, t4)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4, class T5>
  scfImplementationExt6(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    : Super(t1, t2, t3, t4, t5)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4, class T5, class T6>
  scfImplementationExt6(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    : Super(t1, t2, t3, t4, t5, t6)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  Class* GetSCFObject() { return static_cast<Class*> (this); }
  const Class* GetSCFObject() const { return static_cast<const Class*> (this); }
  virtual ~scfImplementationExt6()
  {
    RemoveReftrackerAliases();
  }
  typedef scfImplementationExt6<Class ,Super ,I1, I2, I3, I4, I5, I6> scfImplementationType;
  typedef Class scfClassType;
  virtual size_t GetInterfaceMetadataCount () const
  {
    return 6 + Super::GetInterfaceMetadataCount ();
  }
  virtual void FillInterfaceMetadata (size_t n)
  {
    FillInterfaceMetadataIf<I1> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I2> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I3> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I4> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I5> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I6> (this->scfAuxData->metadataList->metadata, n++);
    Super::FillInterfaceMetadata (n);
  }
private:
  void AddReftrackerAliases ()
  {
    csRefTrackerAccess::AddAlias(this->GetSCFObject(),
     static_cast<Super*> (this->GetSCFObject()));
    AddReftrackerAlias<I1>(this->GetSCFObject());
    AddReftrackerAlias<I2>(this->GetSCFObject());
    AddReftrackerAlias<I3>(this->GetSCFObject());
    AddReftrackerAlias<I4>(this->GetSCFObject());
    AddReftrackerAlias<I5>(this->GetSCFObject());
    AddReftrackerAlias<I6>(this->GetSCFObject());
  }
  void RemoveReftrackerAliases ()
  {
    csRefTrackerAccess::RemoveAlias(this->GetSCFObject(),
        static_cast<Super*> (this->GetSCFObject()));
    RemoveReftrackerAlias<I1>(this->GetSCFObject());
    RemoveReftrackerAlias<I2>(this->GetSCFObject());
    RemoveReftrackerAlias<I3>(this->GetSCFObject());
    RemoveReftrackerAlias<I4>(this->GetSCFObject());
    RemoveReftrackerAlias<I5>(this->GetSCFObject());
    RemoveReftrackerAlias<I6>(this->GetSCFObject());
  }
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
  template<typename IF>
  CS_FORCEINLINE_TEMPLATEMETHOD static void FillInterfaceMetadataIf (
    scfInterfaceMetadata* metadataArray, size_t pos)
  {
    metadataArray[pos].interfaceName = scfInterfaceTraits<IF>::GetName ();
    metadataArray[pos].interfaceID = scfInterfaceTraits<IF>::GetID ();
    metadataArray[pos].interfaceVersion = scfInterfaceTraits<IF>::GetVersion ();
  }
};
template<class Class ,class Super ,class I1, class I2, class I3, class I4, class I5, class I6, class I7>
class scfImplementationExt7 :
  public Super
  ,public I1, public I2, public I3, public I4, public I5, public I6, public I7
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
    void *x;
    if((x = GetInterface<I1>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I2>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I3>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I4>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I5>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I6>(this->GetSCFObject(), id, version)) != 0) return x;
    if((x = GetInterface<I7>(this->GetSCFObject(), id, version)) != 0) return x;
    return Super::QueryInterface(id, version);
  }
protected:
  template<class Class_>
  scfImplementationExt7(Class_ *object)
    : Super()
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1>
  scfImplementationExt7(Class *object, T1 t1)
    : Super(t1)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2>
  scfImplementationExt7(Class *object, T1 t1, T2 t2)
    : Super(t1, t2)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3>
  scfImplementationExt7(Class *object, T1 t1, T2 t2, T3 t3)
    : Super(t1, t2, t3)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4>
  scfImplementationExt7(Class *object, T1 t1, T2 t2, T3 t3, T4 t4)
    : Super(t1, t2, t3, t4)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4, class T5>
  scfImplementationExt7(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    : Super(t1, t2, t3, t4, t5)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  template<class T1, class T2, class T3, class T4, class T5, class T6>
  scfImplementationExt7(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    : Super(t1, t2, t3, t4, t5, t6)
  {
    AddReftrackerAliases();
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
  }
  Class* GetSCFObject() { return static_cast<Class*> (this); }
  const Class* GetSCFObject() const { return static_cast<const Class*> (this); }
  virtual ~scfImplementationExt7()
  {
    RemoveReftrackerAliases();
  }
  typedef scfImplementationExt7<Class ,Super ,I1, I2, I3, I4, I5, I6, I7> scfImplementationType;
  typedef Class scfClassType;
  virtual size_t GetInterfaceMetadataCount () const
  {
    return 7 + Super::GetInterfaceMetadataCount ();
  }
  virtual void FillInterfaceMetadata (size_t n)
  {
    FillInterfaceMetadataIf<I1> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I2> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I3> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I4> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I5> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I6> (this->scfAuxData->metadataList->metadata, n++);
    FillInterfaceMetadataIf<I7> (this->scfAuxData->metadataList->metadata, n++);
    Super::FillInterfaceMetadata (n);
  }
private:
  void AddReftrackerAliases ()
  {
    csRefTrackerAccess::AddAlias(this->GetSCFObject(),
     static_cast<Super*> (this->GetSCFObject()));
    AddReftrackerAlias<I1>(this->GetSCFObject());
    AddReftrackerAlias<I2>(this->GetSCFObject());
    AddReftrackerAlias<I3>(this->GetSCFObject());
    AddReftrackerAlias<I4>(this->GetSCFObject());
    AddReftrackerAlias<I5>(this->GetSCFObject());
    AddReftrackerAlias<I6>(this->GetSCFObject());
    AddReftrackerAlias<I7>(this->GetSCFObject());
  }
  void RemoveReftrackerAliases ()
  {
    csRefTrackerAccess::RemoveAlias(this->GetSCFObject(),
        static_cast<Super*> (this->GetSCFObject()));
    RemoveReftrackerAlias<I1>(this->GetSCFObject());
    RemoveReftrackerAlias<I2>(this->GetSCFObject());
    RemoveReftrackerAlias<I3>(this->GetSCFObject());
    RemoveReftrackerAlias<I4>(this->GetSCFObject());
    RemoveReftrackerAlias<I5>(this->GetSCFObject());
    RemoveReftrackerAlias<I6>(this->GetSCFObject());
    RemoveReftrackerAlias<I7>(this->GetSCFObject());
  }
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
  template<typename IF>
  CS_FORCEINLINE_TEMPLATEMETHOD static void FillInterfaceMetadataIf (
    scfInterfaceMetadata* metadataArray, size_t pos)
  {
    metadataArray[pos].interfaceName = scfInterfaceTraits<IF>::GetName ();
    metadataArray[pos].interfaceID = scfInterfaceTraits<IF>::GetID ();
    metadataArray[pos].interfaceVersion = scfInterfaceTraits<IF>::GetVersion ();
  }
};
/* Pre-preprocessed code ends here */
#include "csutil/deprecated_warn_on.h"
