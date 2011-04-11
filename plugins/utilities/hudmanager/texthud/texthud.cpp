/*
  Copyright (C) 2010-11 Christian Van Brussel, Communications and Remote
  Sensing Laboratory of the School of Engineering at the 
  Universite catholique de Louvain, Belgium
  http://www.tele.ucl.ac.be

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "texthud.h"
#include "csutil/scfstringarray.h"
#include "ivaria/reporter.h"

CS_PLUGIN_NAMESPACE_BEGIN(TextHUDManager)
{

/**
 * Error reporting
 */
static const char* objectId = "crystalspace.utilities.texthud";

bool ReportError (iObjectRegistry* registry, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (registry, CS_REPORTER_SEVERITY_ERROR, objectId, description, arg);
  va_end (arg);
  return false;
}

bool ReportWarning (iObjectRegistry* registry, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (registry, CS_REPORTER_SEVERITY_WARNING, objectId, description, arg);
  va_end (arg);
  return false;
}

// ------------------------ TextHUDManager ------------------------

SCF_IMPLEMENT_FACTORY (TextHUDManager);

TextHUDManager::TextHUDManager (iBase* parent)
  : scfImplementationType (this, parent), cslogo (nullptr), enabled (true), frameCount (0),
  frameTime (0), currentFPS (0.0f), currentKeyPage (0)
{}

TextHUDManager::~TextHUDManager ()
{
  // Unregister from the event queue
  if (eventQueue)
    eventQueue->RemoveListener (this);

  // Delete the logo
  delete cslogo;
}

bool TextHUDManager::Initialize (iObjectRegistry* registry)
{
  // Find references to the engine objects
  g3d = csQueryRegistry<iGraphics3D> (registry);
  if (!g3d) return ReportError (registry, "Failed to locate 3D renderer!");

  g2d = csQueryRegistry<iGraphics2D> (registry);
  if (!g2d) return ReportError (registry, "Failed to locate 2D renderer!");

  vc = csQueryRegistry<iVirtualClock> (registry);
  if (!vc) return ReportError (registry, "Failed to locate virtual clock!");

  csRef<iLoader> loader = csQueryRegistry<iLoader> (registry);
  if (!loader) return ReportError (registry, "Failed to locate main loader!");

  eventQueue = csQueryRegistry<iEventQueue> (registry);
  if (!eventQueue) return ReportError (registry, "Failed to locate event queue!");

  // Load the font
  csRef<iFontServer> fontServer = g2d->GetFontServer ();
  if (!fontServer)
    return ReportError (registry, "Failed to locate font server!");
  else
  {
    font = fontServer->LoadFont (CSFONT_COURIER);
    if (!font)
      return ReportError (registry, "Failed to load font!");
  }

  // Load the Crystal Space logo image
  csRef<iTextureWrapper> texture = loader->LoadTexture
    ("cslogo2", "/lib/std/cslogo2.png", CS_TEXTURE_2D, 0, true, true, true);
  if (!texture.IsValid ())
    ReportWarning (registry, "Failed to load CS logo!\n");

  else
  {
    // Create a 2D sprite for the logo
    iTextureHandle* textureHandle = texture->GetTextureHandle ();
    if (textureHandle)
      cslogo = new csSimplePixmap (textureHandle);
  }

  // Register to the event queue
  eventQueue->RegisterListener (this, csevFrame (registry));

  // Initialize the key and state arrays
  keyDescriptions.AttachNew (new scfStringArray ());
  stateDescriptions.AttachNew (new scfStringArray ());

  return true;
}

bool TextHUDManager::HandleEvent (iEvent& event)
{
  if (!enabled) return false;

  // Tell the 3D driver we're going to display 2D things.
  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS))
    return false;

  // Get the elasped time
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Update the Frame Per Second data
  frameCount++;
  frameTime += elapsed_time;

  if (frameTime > 500)
  {
    currentFPS = ((float) (frameCount * 1000)) / (float) frameTime;
    frameCount = 0;
    frameTime = 0;
  }

  int margin = 15;
  int logoMargin = 5;
  int lineSize = 18;
  int fontColor = g2d->FindRGB (255, 150, 100);

  // Display the available keys
  if (keyDescriptions->GetSize ())
  {
    int y = margin;

    // Check if there is enough room to display all keys
    maxKeys = (uint) ((g2d->GetHeight () - 2 * margin
		       - (stateDescriptions->GetSize () + 5) * lineSize)
		      / lineSize);
    if (keyDescriptions->GetSize () < maxKeys)
    {
      currentKeyPage = 0;
      WriteShadow (margin, y, fontColor, "Keys available:");
    }
    else
      WriteShadow (margin, y, fontColor, "Keys available (%i/%i):",
		   currentKeyPage + 1, keyDescriptions->GetSize () / maxKeys + 1);
    y += lineSize;

    // Write all keys
    for (size_t i = 0; i < keyDescriptions->GetSize (); i++)
    {
      if (i / maxKeys == currentKeyPage)
      {
	WriteShadow (margin + 5, y, fontColor, keyDescriptions->Get (i));
	y += lineSize;
      }
    }

    if (keyDescriptions->GetSize () > maxKeys)
    {
      WriteShadow (margin, y, fontColor, "F1: more keys");
      y += lineSize;
    }
  }

  // Display the state descriptions
  int y = g2d->GetHeight () - margin - lineSize;

  WriteShadow (margin, y, fontColor, "FPS: %.2f", currentFPS);
  y -= lineSize;

  for (int i = stateDescriptions->GetSize () - 1; i >= 0; i--)
  {
    WriteShadow (margin, y, fontColor, stateDescriptions->Get (i));
    y -= lineSize;
  }

  // Display the Crystal Space logo
  if (cslogo)
    cslogo->Draw (g3d,
		  g2d->GetWidth () - cslogo->Width() - logoMargin,
		  logoMargin);

  return false;
}

void TextHUDManager::SwitchKeysPage ()
{
  if (keyDescriptions->GetSize ())
    currentKeyPage =
      (currentKeyPage + 1) % (keyDescriptions->GetSize () / maxKeys + 1);
}

void TextHUDManager::WriteShadow (int x, int y, int fg, const char *str,...) const
{
  csString buf;
  va_list arg;

  va_start (arg, str);
  buf.FormatV (str, arg);
  va_end (arg);

  Write (x + 1, y - 1, 0, -1, "%s", buf.GetData ());
  Write (x, y, fg, -1, "%s", buf.GetData ());
}

void TextHUDManager::Write (int x, int y, int fg, int bg, const char *str,...) const
{
  csString buf;
  va_list arg;

  va_start (arg,str);
  buf.FormatV (str, arg);
  va_end (arg);

  g2d->Write (font, x, y, fg, bg, buf);
}

void TextHUDManager::SetEnabled (bool enabled)
{
  this->enabled = enabled;
}

bool TextHUDManager::GetEnabled () const
{
  return enabled;
}

iStringArray* TextHUDManager::GetKeyDescriptions ()
{
  return keyDescriptions;
}

iStringArray* TextHUDManager::GetStateDescriptions ()
{
  return stateDescriptions;
}

}
CS_PLUGIN_NAMESPACE_END(TextHUDManager)
