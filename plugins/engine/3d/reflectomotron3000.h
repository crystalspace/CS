/*
    Copyright (C) 2007 by Frank Richter

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

#ifndef __CS_CSENGINE_ENVTEXTURE_H__
#define __CS_CSENGINE_ENVTEXTURE_H__

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  namespace EnvTex
  {
    class Holder
    {
      csEngine* engine;

      struct HeldTexture
      {
        csRef<iTextureHandle> texture;
        csTicks deathTime;
      };

      csArray<HeldTexture> usedTextures;
      csArray<HeldTexture> freeTextures;
    public:
      Holder (csEngine* engine) : engine (engine) {}

      void Clear ();
      void NextFrame ();

      iTextureHandle* QueryUnusedTexture (int size, csTicks lifetime);
    };
  
    class Accessor : public scfImplementation1<Accessor,
						iShaderVariableAccessor>
    {
      csMeshWrapper* meshwrap;

      iTextureHandle* envTex;

      uint lastUpdateFrame;
      csTicks lastUpdateTime;
    public:
      Accessor (csMeshWrapper* meshwrap);

      void PreGetValue (csShaderVariable *variable);
    };
  } // namespace EnvTex
}
CS_PLUGIN_NAMESPACE_END(Engine)

#endif // __CS_CSENGINE_ENVTEXTURE_H__
