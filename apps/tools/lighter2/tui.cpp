/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#include "crystalspace.h"

#include "common.h"
#include "config.h"
#include "statistics.h"
#include "tui.h"


namespace lighter
{
  TUI globalTUI;

  TUI::TUI ()
    : scfImplementationType (this), messageBufferEnd (0)
  {
  }

  void TUI::Redraw (int drawFlags) const
  {
    // Redraw the "static" background

    // Clear
    if (drawFlags & TUI_DRAW_CLEAR)
      csPrintf (CS_ANSI_CLEAR_SCREEN);


    // Screen layout (keep to 80x25...)
    if (drawFlags & TUI_DRAW_STATIC)
      DrawStatic ();

    // Draw progress
    if (drawFlags & TUI_DRAW_PROGRESS)
      DrawProgress ();

    // Draw messages
    if (drawFlags & TUI_DRAW_MESSAGES)
      DrawMessage ();

    // Draw RayCore stats
    if (drawFlags & TUI_DRAW_RAYCORE)
      DrawRayCore ();

    // Draw global settings
    if (drawFlags & TUI_DRAW_SETTINGS)
      DrawSettings ();
  }

  static const char* TUI_SEVERITY_TEXT[] = 
  {
    CS_ANSI_FW "BUG" CS_ANSI_RST,
    CS_ANSI_FR "ERR" CS_ANSI_RST,
    CS_ANSI_FY "WAR" CS_ANSI_RST,
    CS_ANSI_FW "NOT" CS_ANSI_RST,
    CS_ANSI_FW "DEB" CS_ANSI_RST
  };

  bool TUI::Report (iReporter* reporter, int severity, const char* msgId,
    const char* description)
  {
    csString& buf = messageBuffer[messageBufferEnd];
    buf.Clear ();

    buf.Append (TUI_SEVERITY_TEXT[severity]);
    buf.Append (" - ");
    csString tmp(description);
    buf.Append (tmp.Slice (0,69).PadRight (69));

    messageBufferEnd++;
    if(messageBufferEnd > 3) messageBufferEnd = 0;

    // Print them
    DrawMessage ();

    return true;
  }

  void TUI::DrawStatic () const
  {
    csPrintf ("/------------------------    Crystal Space Lighter    ------------------------\\\n");
    csPrintf ("| Total progress:                                                             |\n");
    csPrintf ("|                                                                             |\n");
    csPrintf ("| Part progress:                                                              |\n");
    csPrintf ("|                                                                             |\n");
    csPrintf ("|-----------------------------------------------------------------------------|\n");
    csPrintf ("| RayCore  | Settings   | Scene Stats                                         |\n");
    csPrintf ("| Time:    | [ ] DL     | Sectors:                                            |\n");
    csPrintf ("|          | [ ] GI     |                                                     |\n");
    csPrintf ("| Rays:    | [ ] LMs    | Objects:                                            |\n");
    csPrintf ("|          | [ ] AO     |                                                     |\n");
    csPrintf ("| Rays/s:  | PLM:       | Lightmaps:                                          |\n");
    csPrintf ("|          |            |                                                     |\n");
    csPrintf ("|          | ALM:       |                                                     |\n");
    csPrintf ("|          |            |                                                     |\n");
    csPrintf ("|          | Tu/u:      |                                                     |\n");
    csPrintf ("|          |            |                                                     |\n");
    csPrintf ("|          | Tv/u:      |                                                     |\n");
    csPrintf ("|          |            |                                                     |\n");
    csPrintf ("|- CS Messages ---------------------------------------------------------------|\n");
    csPrintf ("|                                                                             |\n");
    csPrintf ("|                                                                             |\n");
    csPrintf ("|                                                                             |\n");
    csPrintf ("|                                                                             |\n");
    csPrintf ("\\-----------------------------------------------------------------------------/\n");
    csPrintf (CS_ANSI_CURSOR(0,0));
  }

  void TUI::DrawSettings () const
  {
    csPrintf (CS_ANSI_CURSOR(14,7) "%s", globalConfig.GetLighterProperties ().doDirectLight ? "X" : "");
    csPrintf (CS_ANSI_CURSOR(14,8) "%s", globalConfig.GetLighterProperties ().doRadiosity ? "X" : "");
    csPrintf (CS_ANSI_CURSOR(14,9) "%s", true ? "X" : "");
    csPrintf (CS_ANSI_CURSOR(14,10) "%s", false ? "X" : "");
  
    csPrintf (CS_ANSI_CURSOR(13,12) "%#4.2g", globalConfig.GetDIProperties ().pointLightMultiplier);
    csPrintf (CS_ANSI_CURSOR(13,14) "%#4.2g", globalConfig.GetDIProperties ().areaLightMultiplier);

    csPrintf (CS_ANSI_CURSOR(13,16) "%#4.2g", globalConfig.GetLMProperties ().uTexelPerUnit);
    csPrintf (CS_ANSI_CURSOR(13,18) "%#4.2g", globalConfig.GetLMProperties ().vTexelPerUnit);

    csPrintf (CS_ANSI_CURSOR(0,0));
  }

  void TUI::DrawRayCore () const
  {
    // Time
    csString unit ("us");
    uint64 time = globalStats.raytracer.usRaytracing;

    if (time > CONST_UINT64(1000000))
    {
      time /= CONST_UINT64(1000000);
      unit = "s ";
    }
    else if (time > CONST_UINT64(1000))
    {
      time /= CONST_UINT64(1000);
      unit = "ms";
    }

    csPrintf (CS_ANSI_CURSOR(2,8) "%6" PRIu64 " %s", time, unit.GetDataSafe ());

    // Rays
    const char* siConv[] = {" ", "K", "M", "G", "T"};

    uint64 rays = globalStats.raytracer.numRays;
    int prefix = 0;
    
    while (rays > CONST_UINT64(10000) && prefix < 5)
    {
      rays /= CONST_UINT64(1000);
      prefix++;
    }

    csPrintf (CS_ANSI_CURSOR(2,10) "%6" PRIu64 " %s", rays, siConv[prefix]);

    // Rays per second
    if (globalStats.raytracer.usRaytracing < 10)
      return; 

    uint64 raysPerS = (globalStats.raytracer.numRays*CONST_UINT64(1000000)) / (globalStats.raytracer.usRaytracing);
    uint64 raysPerSfraction = raysPerS - (raysPerS / CONST_UINT64(1000))*CONST_UINT64(1000);
    raysPerSfraction *= CONST_UINT64(1000);

    prefix = 0;
    while (raysPerS > CONST_UINT64(10000) && prefix < 5)
    {
      raysPerS /= CONST_UINT64(1000);
      raysPerSfraction /= CONST_UINT64(1000);
      prefix++;
    }

    csPrintf (CS_ANSI_CURSOR(2,12) "%2" PRIu64 ".%03" PRIu64 " %s", raysPerS, raysPerSfraction, siConv[prefix]);
    csPrintf (CS_ANSI_CURSOR(0,0));
  }

  void TUI::DrawProgress () const
  {
    csPrintf (CS_ANSI_CURSOR(70, 1));
    csPrintf ("%3d%%", (uint)globalStats.progress.overallProgress);
    
    csPrintf (CS_ANSI_CURSOR(2, 2));
    csPrintf ("%s", GetProgressBar ((uint)globalStats.progress.overallProgress).GetDataSafe ());

    csPrintf (CS_ANSI_CURSOR(18, 3));
    csPrintf ("%s", globalStats.progress.taskName.Slice (0, 40).GetDataSafe ());

    csPrintf (CS_ANSI_CURSOR(70, 3));
    csPrintf ("%3d%%", (uint)globalStats.progress.taskProgress);

    csPrintf (CS_ANSI_CURSOR(2, 4));
    csPrintf ("%s", GetProgressBar ((uint)globalStats.progress.taskProgress).GetDataSafe ());
    csPrintf (CS_ANSI_CURSOR(0,0));
  }

  void TUI::DrawMessage () const
  {
    csPrintf (CS_ANSI_CURSOR(2, 20));

    // Draw the four buffers, starting with messageBufferEnd
    int row = messageBufferEnd-1;
    if(row <  0) row = 3;

    for(uint i = 0; i < 4; i++)
    {
      csPrintf ("%s", messageBuffer[row].GetDataSafe ());
      
      row--;
      if(row < 0) row = 3;
      csPrintf (CS_ANSI_CURSOR_DOWN(1));
      csPrintf (CS_ANSI_CURSOR_BWD(100) CS_ANSI_CURSOR_FWD(2));
    }
  }

  csString TUI::GetProgressBar (uint percent) const
  {
    csString result;

    percent = csClamp(percent, 100u, 0u);

    result.SetCapacity (73);
    //Draw a progess bar being in total 73+2 characters wide
    //|========== =--------- ---------- ---------- ---------- ---------- ---------- ---|
    
    result.Append ('|');

    uint numDone = (73*percent) / 100;
    uint numToDo = (73*(100-percent)) / 100;

    if(numDone + numToDo < 73) numToDo++;

    for(uint i = 0; i < numDone; ++i)
    {
      result.Append ('=');
    }

    for(uint i = 0; i < numToDo; ++i)
    {
      result.Append ('-');
    }

    result.Append ('|');

    return result;
  }
}
