 #ifndef __AWS_H__
 #define __AWS_H__
/**************************************************************************
    Copyright (C) 2000-2001 by Christopher Nelson 
    
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
*****************************************************************************/


/****
  
  This is the alternate windowing system plugin.  It defines a simple, lightweight alternative to the current CSWS
windowing system.  It supports simple skinning via the .skn defintions, and creation of windows from .win definitions.

 ****/
 

class awsComponent
{
   

public:



};

class awsManager : public iAws
{

public:
    DECLARE_IBASE;

    awsManager(iBase *p);
    virtual ~awsManager();
    
    bool Initialize(iSystem *sys);

    virtual void Load(char *defs_file);

  // Implement iPlugIn interface.
  struct eiPlugIn : public iPlugIn
  {
    DECLARE_EMBEDDED_IBASE(awsManager);
    virtual bool Initialize(iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent(iEvent&) { return false; }
  } scfiPlugIn;

};
 
#endif
