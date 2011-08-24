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

#ifndef __INETWORK_HTTP_H__
#define __INETWORK_HTTP_H__

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>

struct iDataBuffer;


namespace CS {
namespace Network {
namespace HTTP {

enum ResponseState
{
  OK = 0,
  CouldNotConnect = 1,
  CouldNotResolve = 2,
  Other = 2
};

/**
 *
 */
struct iResponse : public virtual iBase
{
  SCF_INTERFACE(iResponse, 0, 1, 0);

  virtual int GetCode () = 0;
  
  virtual const ResponseState& GetState () = 0;
  
  virtual const char* GetError () = 0;
  
  virtual csRef<iDataBuffer> GetHeader () = 0;
  
  virtual csRef<iDataBuffer> GetData () = 0;
};


/**
 *
 */
struct iHTTPConnection : public virtual iBase
{
  SCF_INTERFACE (iHTTPConnection, 0, 0, 1);
  
  /**
   * Perform a GET request.
   * Example: bool success = Get("search", "term=hello&lang=world");
   */
  virtual csRef<iResponse> Get(const char* location, const char* params=0, iStringArray* headers=0) = 0;
  
  /**
   * Perform a HEAD request.
   * Example: bool success = Head("search", "term=hello&lang=world");
   */
  virtual csRef<iResponse> Head(const char* location, const char* params=0) = 0;
  
  /**
   * Perform a POST request.
   * Example: bool success = Post("/testobjs/", "{\"id\": 1}", "application/json");
   */
  virtual csRef<iResponse> Post(const char* location, const char* pdata=0, const char* format=0) = 0;
  
  /**
   * Perform a PUT request.
   * Example: bool success = Put("/testobjs/1/", "{\"name\": \"test\"}", "application/json");
   */
  virtual csRef<iResponse> Put(const char* location, const char* pdata=0, const char* format=0) = 0;
  
  /**
   * Perform a DELETE request.
   * Example: bool success = Delete("/testobjs/1/");
   */
  virtual csRef<iResponse> Delete(const char* location) = 0;
};

enum ProxySetting
{
  NoProxy = 0,
  SystemProxy = 1,
  CustomProxy = 2
};


/**
 * 
 */
struct iHTTPConnectionFactory : public virtual iBase
{
  SCF_INTERFACE (iHTTPConnectionFactory, 0, 0, 1);
  
  /**
   * Create a HTTP connection to a specified server.
   * Note that the socket-connection isn't made until the first request is made
   * and that using the same socket-connection for consecutive request is an 
   * optimization left to the implementation.
   */
  virtual csRef<iHTTPConnection> Create (const char* uri) = 0;
  
  /**
   * Set what kind of proxy settings to use.
   */
  virtual void UseProxy (ProxySetting setting) = 0;
  
  /**
   * uri example: "proxy.host.com:8080"
   * Leave user and password empty to not use them.
   */
  virtual void SetCustomProxy(const char* uri, const char* user=0, const char* password=0) = 0;
};

} // namespace HTTP
} // namespace Network
} // namespace CS

#endif
