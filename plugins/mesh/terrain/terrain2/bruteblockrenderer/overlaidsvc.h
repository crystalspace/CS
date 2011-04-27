/*
  Copyright (C) 2006 by Kapoulkine Arseny
                2007 by Marten Svanfeldt

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

#ifndef __OVERLAIDSVC_H__
#define __OVERLAIDSVC_H__

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{

struct OverlaidShaderVariableContext : 
  public scfImplementation1<OverlaidShaderVariableContext, 
			    scfFakeInterface<iShaderVariableContext> >,
  public CS::Graphics::OverlayShaderVariableContextImpl
{
  OverlaidShaderVariableContext (iShaderVariableContext* parent = nullptr)
   : scfImplementationType (this), OverlayShaderVariableContextImpl (parent) {}
};

}
CS_PLUGIN_NAMESPACE_END(Terrain2)

#endif // __OVERLAIDSVC_H__
