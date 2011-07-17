/*
    Copyright (C) 2011 by Jelle Hellemans

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

#ifndef __CS_CSUTIL_THREADING_FUTURE_IMPL_H__
#define __CS_CSUTIL_THREADING_FUTURE_IMPL_H__

#include "csutil/scf_interface.h"

#include "csutil/threading/condition.h"
#include "csutil/threading/mutex.h"
#include "csutil/threading/thread.h"

namespace CS
{
namespace Threading
{

template<typename R> class Future;
template<typename R> class Promise;

struct iFutureListener : public virtual iBase
{
  SCF_INTERFACE (iFutureListener, 1,0,0);
  virtual void OnReady () = 0;
};

namespace Implementation
{
template<typename T>
struct BaseReturnValue : public CS::Utility::AtomicRefCount
{
  virtual T Get() const = 0;
  virtual ~BaseReturnValue() {}
};

template<typename T>
class ReturnValue : public BaseReturnValue<T> 
{
public:
  ReturnValue() {}
  //explicit ReturnValue(const T &value) : value_(new T(value)) {}
  explicit ReturnValue(const T &value) : value_(value) {}
  //virtual T Get() const { return *value_;}
  virtual T Get() const { return value_;}
  //void Set(const T& value) { /*delete value_;*/ value_ = new T(value);}
  void Set(const T& value) { value_ = value; }
  virtual ~ReturnValue() { /*delete value_;*/ }
private:
  //T* value_;
  T value_;
};

/*
template<typename T,typename U>
class ReturnValueTypeAdaptor : public BaseReturnValue<T> 
{
public:
  ReturnValueTypeAdaptor(const csRef<BaseReturnValue<U> > &real_value) : value_(real_value) {}
  virtual T Get() const {return *value_;}
  virtual ~ReturnValueTypeAdaptor() {}
private:
  csRef<BaseReturnValue<U> > value_;
};
*/

class Future_impl : public CS::Utility::AtomicRefCount
{
public:
  Future_impl() : hasValue_(false), hasError_(false) {}
  
  bool HasValue () const 
  {
    CS::Threading::MutexScopedLock lock(mutex_);
    return hasValue_;
  }
  
  bool IsSuccessful() const {
	  CS::Threading::MutexScopedLock lock(mutex_);
	  return hasError_;
	}
  
  bool Ready () const 
  {
    CS::Threading::MutexScopedLock lock(mutex_);
    return (hasValue_ || hasError_);
  }
  
  bool Wait (csTicks timeout = 0) 
  {
    CS::Threading::MutexScopedLock lock(mutex_);
    while (!hasValue_ && !hasError_)
      if(!cond_.Wait(mutex_, timeout)) //cond_.Wait(lock)?
        return false; /* Timeout */
    return true;
  }

  template <typename T>
  T Get (const BaseReturnValue<T>& value) 
  {
    CS::Threading::MutexScopedLock lock(mutex_);
    while (!hasValue_ && !hasError_)
      cond_.Wait(mutex_);//cond_.Wait(lock)?
    return value.Get();
  }

  template <typename T>
  bool SetValue (const T& r, ReturnValue<T>& value) 
  {
    CS::Threading::MutexScopedLock lock(mutex_);
    if (hasValue_ || hasError_) return false;
    value.Set(r);
    hasValue_ = true;
    Notify();
    return true;
  }
  
  void SetError() 
  {
	  CS::Threading::MutexScopedLock lock(mutex_);
	  if (hasValue_ || hasError_) return;
	  hasError_ = true;
	  Notify();
	}
  
  void EndPromise() 
  {
    CS::Threading::MutexScopedLock lock(mutex_);
    if (hasValue_ || hasError_) return; // ignore 
    hasError_ = true;
    Notify();
  }
  
  void AddListener(iFutureListener* listener)
  {
    CS::Threading::MutexScopedLock lock(mutex_);
    if (hasValue_ || hasError_) {
      mutex_.Unlock(); // never call a callback within the mutex
      listener->OnReady(); //return empty callback_reference
      return;
    }
    callbacks_.Push(listener);
  }
  
  void RemoveListener(iFutureListener* listener) 
  {
    CS::Threading::MutexScopedLock lock(mutex_);
    if (callbacksInProgress_) 
    {
      while (callbacksInProgress_)
        cond_.Wait(mutex_);
      //notify already removed all callbacks
      return;
    }
    if (hasValue_ || hasError_) return; //ignore, already set, and automatically removed
    callbacks_.Delete(listener);
  }

private:
  void Notify() 
  {
	  callbacksInProgress_ = true;
	  cond_.NotifyAll();
	  csRefArray<iFutureListener> cbs(callbacks_);
	  mutex_.Unlock();
	  for (size_t i = 0; i < cbs.GetSize(); i++)
	    cbs.Get(i)->OnReady();
    // delete all callbacks - they will never be needed again
    // that is also why this clear is thread-safe outside the mutex
    callbacks_.DeleteAll();
	  // the below is in case someone tried to remove while we are calling
	  //CS::Threading::MutexScopedLock lock2(mutex_);
	  mutex_.Lock();
	  callbacksInProgress_ = false;
	  cond_.NotifyAll();
	}
	
private:
  bool hasValue_;
  bool hasError_;
  bool callbacksInProgress_;
  mutable CS::Threading::Mutex mutex_;
  mutable CS::Threading::Condition cond_;
  csRefArray<iFutureListener> callbacks_;
};


template<typename T> 
class Promise_impl : public CS::Utility::AtomicRefCount
{
public:
  Promise_impl()
  {
    f_.AttachNew(new Future_impl);
    value_.AttachNew(new ReturnValue<T>);
  }
  ~Promise_impl() 
  {
    f_->EndPromise();
  }
  csRef<Future_impl> f_;
  csRef<ReturnValue<T> > value_;
};

}
}
}

#endif // __CS_IMAP_FUTURE_IMPL_H__

