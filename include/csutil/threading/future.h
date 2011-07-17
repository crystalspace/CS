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

#ifndef __CS_CSUTIL_THREADING_FUTURE_H__
#define __CS_CSUTIL_THREADING_FUTURE_H__

#include "csutil/scf_interface.h"

#include "future_impl.h"

namespace CS
{
namespace Threading
{

template<typename T> 
class Promise
{
public:
  Promise ()
  {
    impl_.AttachNew(new Implementation::Promise_impl<T>);
  }
  
  Promise (const Promise& t)
  {
    impl_ = t.impl_; 
  }
  
  Promise& operator= (const Promise& t)
  {
    impl_ = t.impl_; 
    return *this;
  }

  void Set (const T& r) 
  {
    impl_->f_->SetValue(r, *impl_->value_);
  }

private:
  template <typename Y> friend class Future;
  csRef<Implementation::Promise_impl<T> > impl_;
};

template<typename T> 
class Future
{
public:
  Future() : impl_(), value_() 
  {
    Promise<T> p;
    impl_ = p.impl_->f_;
    value_ = p.impl_->value_;
  }

  Future(const Future& t) : impl_(t.impl_), value_(t.value_) {}
  Future(const Promise<T>& p) : impl_(p.impl_->f_), value_(p.impl_->value_) {}
  
  ~Future() {};
  
public:
  Future& operator=(const Future& t) 
  {
    impl_ = t.impl_; 
    value_ = t.value_;
    return *this;
  }

  bool HasValue() const 
  {
    return impl_->HasValue();
  }

  bool Ready() const 
  {
    return impl_->Ready();
  }

  bool Wait(csTicks timeout = 0) 
  {
    return impl_->Wait(timeout);
  }

  T Get() const {
    return impl_->Get(*value_);
  }
  
  void AddListener(iFutureListener* listener)
  {
    impl_->AddListener(listener);
  }
  
  void RemoveListener(iFutureListener* listener) 
  {
    impl_->RemoveListener(listener);
  }

private:
  template <typename Y> friend class Future;
  csRef<Implementation::Future_impl> impl_;
  csRef<Implementation::BaseReturnValue<T> > value_;
};

}
}

#endif // __CS_CSUTIL_THREADING_FUTURE_H__

