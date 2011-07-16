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

#include "cssysdef.h"
#include "csutil/scf.h"
#include "csutil/sysfunc.h"

#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "httpfactory.h"


CS_PLUGIN_NAMESPACE_BEGIN(CSHTTP)
{
  
SCF_IMPLEMENT_FACTORY (HTTPConnectionFactory)

HTTPConnectionFactory::HTTPConnectionFactory (iBase* parent)
 : scfImplementationType (this, parent), object_reg(0), setting(SystemProxy)
{
}

bool HTTPConnectionFactory::Initialize (iObjectRegistry* obj_reg)
{
  object_reg = obj_reg;

  return true;
}

HTTPConnectionFactory::~HTTPConnectionFactory ()
{
}

csRef<iHTTPConnection> HTTPConnectionFactory::Create (const char* uri)
{
  //CURL internally manages connections for reuse which is a lot faster for 
  //sequential requests to the same server, but curl handles aren't threadsafe
  //so return different connections for different threads.
  CS::Threading::ScopedReadLock lock(mutex);
  csRef<HTTPConnection> connection;
  ThreadConnections::const_iterator found = connections.find(CS::Threading::Thread::GetThreadID());
  if (found == connections.end()) 
  {
    connection.AttachNew(new HTTPConnection(uri));
    if (setting == CustomProxy)
    {
      connection->SetProxy(proxyURI, proxyUser, proxyPass);
    }
    else if (setting == NoProxy)
    {
      connection->SetProxy("", "", "");
    }
    CS::Threading::ScopedUpgradeableLock writelock(mutex);
    connections[CS::Threading::Thread::GetThreadID()] = connection;
    return connection;
  }
  return found->second;
}

void HTTPConnectionFactory::UseProxy (ProxySetting setting)
{
  this->setting = setting;
}

void HTTPConnectionFactory::SetCustomProxy(const char* uri, const char* user, const char* password)
{
  this->setting = CustomProxy;
  proxyURI = uri;
  proxyUser = user;
  proxyPass = password;
}


}
CS_PLUGIN_NAMESPACE_END(CSHTTP)
