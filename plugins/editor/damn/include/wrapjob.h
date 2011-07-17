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

#ifndef __CS_THREADING_WRAPJOB_H__
#define __CS_THREADING_WRAPJOB_H__

#include "csutil/scf_interface.h"
#include "iutil/job.h"
#include "csutil/threading/future.h"


namespace CS
{
namespace Threading
{

template<typename ReturnType, typename Callable>
class FunctorWrapperJob : public scfImplementation1<FunctorWrapperJob<ReturnType, Callable>, iJob>
{
public:
  //typedef typename std::tr1::result_of<Callable()>::type ReturnType;
	FunctorWrapperJob(Callable function) : scfImplementation1<FunctorWrapperJob<ReturnType, Callable>, iJob>(this), function (function)
	{
	}
	virtual void Run()
	{
		promise.Set(function());
	}
  Future<ReturnType> GetFuture() { return Future<ReturnType>(promise); }
private:
  Promise<ReturnType> promise;
  Callable function;
};


}
}


#endif // __CS_THREADING_WRAPJOB_H__

