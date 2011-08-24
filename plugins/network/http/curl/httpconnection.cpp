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
#include "iutil/stringarray.h"

#include "csutil/databuf.h"

#include "httpconnection.h"


#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <curl/curl.h>


CS_PLUGIN_NAMESPACE_BEGIN(CSHTTP)
{

typedef struct {
    const char *buf;
    curl_off_t len;
    curl_off_t pos;
} readarg_t;

Response::Response () : scfImplementationType (this), code(0), error(0)
{
}

Response::~Response ()
{
  delete[] error;
}

int Response::GetCode () { return code; }
const ResponseState& Response::GetState () { return state; }
const char* Response::GetError () { return error; }
csRef<iDataBuffer> Response::GetHeader () { return header; }
csRef<iDataBuffer> Response::GetData () { return data; }

//----------------------------------------------------------------------------
HTTPConnection::HTTPConnection (const char* uri)
 : scfImplementationType (this), uri(uri)
{
  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_HEADER, 0);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

  curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
 
  // Progress
  curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, &HTTPConnection::ProgressCallback);
  curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);

  // Data
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &HTTPConnection::Write);

  // Upload data
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, &HTTPConnection::Read);
}

HTTPConnection::~HTTPConnection ()
{
  curl_easy_cleanup(curl);
}

csRef<iResponse> HTTPConnection::Get(const char* location, const char* params, iStringArray* headersArray)
{
  std::stringstream source;
  source << uri << location;

  if (params) source << "?" << params;

  curl_easy_setopt(curl, CURLOPT_URL, source.str().c_str());
  
  struct curl_slist* headers=0;
  if (headersArray)
  {
    for (size_t i = 0; i < headersArray->GetSize(); i++)
    {
      headers = curl_slist_append(headers, headersArray->Get(i));
    }
  }

  // pass our list of custom made headers
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  csRef<iResponse> reponse = Perform(source.str());
  
  // free the header list
  curl_slist_free_all(headers);  
  
  //Reset some stuffv
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, 0);

  return reponse;    
}

csRef<iResponse> HTTPConnection::Head(const char* location, const char* params)
{
  std::stringstream source;
  source << uri << location;

  if (params) source << "?" << params;

  curl_easy_setopt(curl, CURLOPT_URL, source.str().c_str());
  
  curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

  csRef<iResponse> reponse = Perform(source.str());
  
  //Reset some stuff
  curl_easy_setopt(curl, CURLOPT_HTTPGET, true);
  
  //curl_easy_setopt(curl, CURLOPT_NOBODY, 0);  CURLOPT_HTTPGET should already reset it.

  return reponse; 
}

csRef<iResponse> HTTPConnection::Post(const char* location, const char* pdata, const char* format)
{
  std::stringstream source;
  source << uri << location;

  curl_easy_setopt(curl, CURLOPT_URL, source.str().c_str());

  struct curl_slist* headers=0;
  if (format) // "Content-Type: application/json"
  {
    std::string content = "Content-Type: "; content =+ format;
    headers = curl_slist_append(headers, content.c_str());
  }

  // pass our list of custom made headers
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pdata);

  csRef<iResponse> reponse = Perform(source.str());

  // free the header list
  curl_slist_free_all(headers);  
  
  //Reset some stuffv
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, 0);
  
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, 0);
  
  curl_easy_setopt(curl, CURLOPT_HTTPGET, true); 

  return reponse;
}

csRef<iResponse> HTTPConnection::Put(const char* location, const char* pdata, const char* format)
{
  std::stringstream source;
  source << uri << location;

  curl_easy_setopt(curl, CURLOPT_URL, source.str().c_str());

  struct curl_slist* headers=0;  
  if (format) // "Content-Type: application/json"
  {
    std::string content = "Content-Type: "; content =+ format;
    headers = curl_slist_append(headers, content.c_str());
  }

  // pass our list of custom made headers
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  readarg_t rarg;
  rarg.buf = pdata;//assigning buffer for uploading
  rarg.len = (curl_off_t)strlen(pdata);
  rarg.pos = 0;

  curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
  curl_easy_setopt(curl, CURLOPT_PUT, 1);

  curl_easy_setopt(curl, CURLOPT_READDATA, &rarg);
  curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, rarg.len);


  csRef<iResponse> reponse = Perform(source.str());

  // free the header list
  curl_slist_free_all(headers);  
  
  //Reset some stuff
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, 0);
  
  curl_easy_setopt(curl, CURLOPT_UPLOAD, 0);
  curl_easy_setopt(curl, CURLOPT_PUT, 0);

  curl_easy_setopt(curl, CURLOPT_READDATA, 0);
  curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, 0);

  return reponse;
}

csRef<iResponse> HTTPConnection::Delete(const char* location)
{
  std::stringstream source;
  source << uri << location;

  curl_easy_setopt(curl, CURLOPT_URL, source.str().c_str());

  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

  csRef<iResponse> reponse = Perform(source.str());

  return reponse;
}

void HTTPConnection::SetProxy(const std::string& proxyURI, const std::string& proxyUser, const std::string& proxyPass)
{
  proxy = proxyURI;
  userPass = proxyUser.empty()?"":proxyUser+":"+proxyPass;
  curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str()); 
  curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, userPass.c_str()); 
}

csRef<Response> HTTPConnection::Perform(const std::string& source)
{
  csRef<Response> response;
  response.AttachNew(new Response());
  
  csString header("");
  csString buffer("");
  
  // Direct to buffers.
  response->error = new char[CURL_ERROR_SIZE];
  curl_easy_setopt (curl, CURLOPT_ERRORBUFFER, response->error);
  
  curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &header);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

  // Perform
  CURLcode result = curl_easy_perform(curl);
  curl_easy_getinfo (curl, CURLINFO_HTTP_CODE, &response->code);

  if (result != CURLE_OK)
  {
    if (result == CURLE_COULDNT_CONNECT)
      response->state = CouldNotConnect;
    else if (result == CURLE_COULDNT_RESOLVE_HOST)
      response->state = CouldNotResolve;
    else
      response->state = Other;
  }
  size_t hlength = header.Length ();
  response->header.AttachNew(new csDataBuffer(header.Detach(), hlength));
  
  size_t dlength = buffer.Length ();
  response->data.AttachNew(new csDataBuffer(buffer.Detach(), dlength));

  // We're done so broadcast 100%.
  ProgressCallback(this, 1.0, 1.0, 1.0, 1.0);

  return response;
}

int HTTPConnection::ProgressCallback(HTTPConnection* clientp,
                            double dltotal,
                            double dlnow,
                            double ultotal,
                            double ulnow)
{
  double val = 0.0;
  if (dltotal != 0.0) val = (dlnow)/dltotal;
  //if (clientp->onProgress) (*clientp->onProgress)(val);
  return 0;
}

int HTTPConnection::Read(void* ptr, size_t size, size_t nitems, void* stream)
{
  readarg_t *rarg = (readarg_t *)stream;
  curl_off_t len = rarg->len - rarg->pos;
  if ((size_t)len > size * nitems)
    len = size * nitems;
  memcpy(ptr, rarg->buf + rarg->pos, len);
  rarg->pos += len;
  return len;
}

int HTTPConnection::Write(char *data, size_t size, size_t nmemb, csString* buffer)
{
  int result = 0;

  if(buffer != NULL) 
  {
    buffer->Append(data, size * nmemb);
    result = size * nmemb;
  }

  return result;
}

}
CS_PLUGIN_NAMESPACE_END(CSHTTP)
