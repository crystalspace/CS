/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    Copyright (C) 2000 by Samuel Humphreys

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

#ifndef __PROCTEX_H__
#define __PROCTEX_H__

#include <stdarg.h>
#include "csutil/csobject.h"

struct iTextureWrapper;
struct iMaterialWrapper;
struct iEngine;

struct iSystem;
struct iGraphics2D;
struct iGraphics3D;
struct iTextureManager;
struct iTextureWrapper;

class csProcTexture : public csObject
{
protected:
  // Will be set to true as soon as pt is initialized.
  bool ptReady;

  // Flags uses for the texture.
  int texFlags;

  // Texture wrapper.
  iTextureWrapper* tex;
  // Dimensions of texture.
  int mat_w, mat_h; 
  // Procedural G3D and G2D.
  iGraphics3D* ptG3D;
  iGraphics2D* ptG2D;
  iTextureManager* ptTxtMgr;
  iSystem* sys;
  bool anim_prepared;

  bool key_color;
  int key_red, key_green, key_blue;

  // If true (default) then proc texture will register a callback
  // so that the texture is automatically updated (Animate is called)
  // whenever it is used.
  bool use_cb;
  // The current time the previous time the callback was called.
  // This is used to detect if the callback is called multiple times
  // in one frame.
  csTime last_cur_time;

private:
  static void ProcCallback (iTextureWrapper* txt, void* data);

public:
  csProcTexture ();
  virtual ~csProcTexture ();

  /**
   * Disable auto-update. By default csProcTexture will register
   * a callback so that every time the texture is visible Animate
   * will automatically be called. If you don't want this and you want
   * to call Animate on your own then you can disable this feature.
   * You need to call DisableAutoUpdate() before calling Initialize().
   */
  void DisableAutoUpdate () { use_cb = false; }

  /**
   * Do everything needed to initialize this texture.
   * At this stage only will settings like the key color be used.
   * After Initialize has been called you can call Prepare() on the
   * texture handle or PrepareTextures. The correct init sequence is:
   * <ul>
   * <li>csProcTexture::Initialize()
   * <li>iTextureWrapper::Register()
   * <li>iTextureHandle::Prepare() or iTextureManager::PrepareTextures()
   * <li>csProcTexture::PrepareAnim()
   * </ul>
   * Alternatively you can use Initialize(engine,name) which does all this
   * work for you.
   */
  virtual bool Initialize (iSystem* system);

  /**
   * Initialize this procedural texture, create a material associated
   * with it, properly register the texture and material and prepare
   * them. This function assumes that the texture manager has already
   * been set up and PrepareTextures has already been called for the
   * other loaded textures. It is a conveniance function that offers
   * less flexibility but is sufficient for most cases. The texture and
   * material will get the name that is given by this routine.
   */
  iMaterialWrapper* Initialize (iSystem* system, iEngine* engine,
      	iTextureManager* txtmgr, const char* name);

  /**
   * Prepare the animation for use. This needs to be done after
   * the texture has been prepared.
   */
  virtual bool PrepareAnim ();

  /// Set the key color to use for this texture.
  void SetKeyColor (int red, int green, int blue)
  {
    key_color = true;
    key_red = red;
    key_green = green;
    key_blue = blue;
  }
  
  /**
   * Animate this texture. Subclasses of csProcTexture must implement
   * this to implement some kind of animation on the procedural texture.
   */
  virtual void Animate (csTime current_time) = 0;

  /// Get the texture corresponding with this procedural texture.
  iTextureWrapper* GetTextureWrapper () { return tex; }

  static int GetRandom (int max)
  {
    return (int)(float(max)*rand()/(RAND_MAX+1.0));
  }
};


#endif // __PROCTEX_H__

