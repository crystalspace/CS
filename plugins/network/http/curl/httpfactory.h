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

#ifndef __NETWORK_HTTPFACTORY_H__
#define __NETWORK_HTTPFACTORY_H__

#include "inetwork/http.h"

#include <csutil/scf_implementation.h>

#include <csutil/threading/thread.h>

#include <csutil/threading/rwmutex.h>

#include <string>
#include <map>

#include "httpconnection.h"

using namespace CS::Network::HTTP;

CS_PLUGIN_NAMESPACE_BEGIN(CSHTTP)
{
class HTTPConnectionFactory : public scfImplementation2<HTTPConnectionFactory,iHTTPConnectionFactory,iComponent>
{
public:
  HTTPConnectionFactory (iBase* parent);
  virtual ~HTTPConnectionFactory ();
  
  // iComponent
  virtual bool Initialize (iObjectRegistry* obj_reg);
  
  // iHTTPConnectionFactory
  virtual csRef<iHTTPConnection> Create (const char* uri);
  virtual void UseProxy (ProxySetting setting);
  virtual void SetCustomProxy(const char* uri, const char* user=0, const char* password=0);

private:
  iObjectRegistry* object_reg;
  ProxySetting setting;
  std::string proxyURI;
  std::string proxyUser;
  std::string proxyPass;
  typedef std::map<CS::Threading::ThreadID, csRef<HTTPConnection> > ThreadConnections;
  ThreadConnections connections;
  CS::Threading::ReadWriteMutex mutex;
};
}
CS_PLUGIN_NAMESPACE_END(CSHTTP)

#endif
