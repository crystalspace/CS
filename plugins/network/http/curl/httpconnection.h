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

#ifndef __NETWORK_HTTP_H__
#define __NETWORK_HTTP_H__

#include "inetwork/http.h"

#include <csutil/scf_implementation.h>

#include <string>

typedef void CURL;

using namespace CS::Network::HTTP;

CS_PLUGIN_NAMESPACE_BEGIN(CSHTTP)
{
class HTTPConnection;

class Response : public scfImplementation1<Response,iResponse>
{
public:
  Response ();
  virtual ~Response ();
  
  // iResponse
  virtual int GetCode ();
  virtual const ResponseState& GetState ();
  virtual const char* GetError () ;
  virtual csRef<iDataBuffer> GetHeader ();
  virtual csRef<iDataBuffer> GetData ();

private:
  int code; 
  ResponseState state;
  char* error;
  csRef<iDataBuffer> header; 
  csRef<iDataBuffer> data; 
  friend class HTTPConnection;
};

class HTTPConnection : public scfImplementation1<HTTPConnection,iHTTPConnection>
{
public:
  HTTPConnection (const char* uri);
  virtual ~HTTPConnection ();
  
  // iHTTPConnection
  virtual csRef<iResponse> Get(const char* location, const char* params=0, iStringArray* headers=0);
  virtual csRef<iResponse> Head(const char* location, const char* params=0);
  virtual csRef<iResponse> Post(const char* location, const char* pdata=0, const char* format=0);
  virtual csRef<iResponse> Put(const char* location, const char* pdata=0, const char* format=0);
  virtual csRef<iResponse> Delete(const char* location);
  
  void SetProxy(const std::string& proxyURI, const std::string& proxyUser, const std::string& proxyPass);

private:
  std::string uri;
  std::string data;
  std::string header;
  
  std::string proxy;
  std::string userPass;

  CURL* curl;

  static int ProgressCallback(HTTPConnection* clientp, double dltotal, double dlnow, double ultotal, double ulnow);
  static int Write(char *data, size_t size, size_t nmemb, csString* buffer);
  static int Read(void* ptr, size_t size, size_t nitems, void* stream);

  csRef<Response> Perform(const std::string& source);   
};
}
CS_PLUGIN_NAMESPACE_END(CSHTTP)

#endif
