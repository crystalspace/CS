/*  -*- Mode: C++; -*- */
/*
    Crystal Space 3D engine
    Copyright (C) 2000 by Jorrit Tyberghein
  
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

/**
* Automatic selection of the board.
* Algorithm used:
* If multiple boards, select which one in that order, (The Most RAM if equivalent):
* If it is a Voodoo2 with sli, it is selected
* If it is a Voodoo2, it is selected
* If it is a Voodoo with sli, it is selected
* If it is a Voodoo, it is selected
* Else, the first one
* At this moment, there is no documented way to detect Banshee !
**/

  GrHwConfiguration hwconfig;

  GlideLib_grSstQueryBoards(&hwconfig);
  if(hwconfig.num_sst==0) 
    SystemFatalError ("csGraphics3DGlide2x::Open : No 3dfx chip found");
  GlideLib_grGlideInit();
  if(!GlideLib_grSstQueryHardware(&hwconfig))
    SystemFatalError ("csGraphics3DGlide2x::Open : Unable to find any 3dfx chip");
        
  char szVersion[80];
  GlideLib_grGlideGetVersion(szVersion);

  SysPrintf (CS_MSG_INITIALIZATION, " Glide %s detected.\n",szVersion);

  int i;
  board=0;
  for(i=0; i<hwconfig.num_sst; i++)
    {
      switch(hwconfig.SSTs[i].type)
        {
#ifndef GLIDE24_ONLY
        case GR_SSTTYPE_Voodoo2:
          if(hwconfig.SSTs[board].type!=GR_SSTTYPE_Voodoo2)
            {
              board=i;
            }
          else
            {
              if(hwconfig.SSTs[board].sstBoard.Voodoo2Config.sliDetect==FXTRUE)
                {
                  if(hwconfig.SSTs[i].sstBoard.Voodoo2Config.sliDetect==FXTRUE)
                    {   // Equivalent => Most RAM
                      if(hwconfig.SSTs[i].sstBoard.Voodoo2Config.fbRam>hwconfig.SSTs[board].sstBoard.Voodoo2Config.fbRam)
                        {
                          board=i;
                        }
                    }
                }
              else
                {
                  if(hwconfig.SSTs[i].sstBoard.Voodoo2Config.sliDetect==FXTRUE)
                    {   
                      board=i;
                    }
                  else
                    {   // Equivalent => Most RAM
                      if(hwconfig.SSTs[i].sstBoard.Voodoo2Config.fbRam>hwconfig.SSTs[board].sstBoard.Voodoo2Config.fbRam)
                        {
                          board=i;
                        }
                    }
                }
            }
          break;
#endif

        case GR_SSTTYPE_VOODOO:
#ifndef GLIDE24_ONLY
          if(hwconfig.SSTs[board].type==GR_SSTTYPE_Voodoo2)
            break;
#endif

          if(hwconfig.SSTs[board].type!=GR_SSTTYPE_VOODOO)
            {
              board=i;
            }
          else
            {
              if(hwconfig.SSTs[board].sstBoard.VoodooConfig.sliDetect==FXTRUE)
                {
                  if(hwconfig.SSTs[i].sstBoard.VoodooConfig.sliDetect==FXTRUE)
                    {   // Equivalent => Most RAM
                      if(hwconfig.SSTs[i].sstBoard.VoodooConfig.fbRam>hwconfig.SSTs[board].sstBoard.VoodooConfig.fbRam)
                        {
                          board=i;
                        }
                    }
                }
              else
                {
                  if(hwconfig.SSTs[i].sstBoard.VoodooConfig.sliDetect==FXTRUE)
                    {   
                      board=i;
                    }
                  else
                    {   // Equivalent => Most RAM
                      if(hwconfig.SSTs[i].sstBoard.VoodooConfig.fbRam>hwconfig.SSTs[board].sstBoard.VoodooConfig.fbRam)
                        {
                          board=i;
                        }
                    }
                }
            }
          break;

        default:
          break;
        }
    }

  SysPrintf (CS_MSG_INITIALIZATION, " Board %d selected.\n",board);

/**
* This Function gets information from the Current Board selected.
* Those informations are written to the Initialization Log
* It also use those informations to initialize the rendering process
* This initialisation includes:
*   - Standard init of the board
*   - Speficic init for rendering pass (One or two based on the number of TMUs)
**/

  switch(hwconfig.SSTs[board].type)
  {
#ifndef GLIDE24_ONLY
  case GR_SSTTYPE_Voodoo2:
    {
          SysPrintf (CS_MSG_INITIALIZATION, " Board is a Voodoo2.\n");
          SysPrintf (CS_MSG_INITIALIZATION, "  Pixelfx Revision %d.\n",hwconfig.SSTs[board].sstBoard.Voodoo2Config.fbiRev);
          SysPrintf (CS_MSG_INITIALIZATION, "  Pixelfx Memory : %d MB.\n",hwconfig.SSTs[board].sstBoard.Voodoo2Config.fbRam);
          SysPrintf (CS_MSG_INITIALIZATION, "  Number of Texelfx Chips %d.\n",hwconfig.SSTs[board].sstBoard.Voodoo2Config.nTexelfx);
          for(i=0;i<hwconfig.SSTs[board].sstBoard.Voodoo2Config.nTexelfx;i++)
          {
            SysPrintf (CS_MSG_INITIALIZATION, "   Texelfx nb %d has revision %d and %d MB of RAM.\n",i+1,
                        hwconfig.SSTs[board].sstBoard.Voodoo2Config.tmuConfig[i].tmuRev,
                        hwconfig.SSTs[board].sstBoard.Voodoo2Config.tmuConfig[i].tmuRam);
          }
          if (hwconfig.SSTs[board].sstBoard.Voodoo2Config.sliDetect)
            SysPrintf (CS_MSG_INITIALIZATION, " SLI Detected.\n");

          if (hwconfig.SSTs[board].sstBoard.Voodoo2Config.nTexelfx==1)
          {
            m_iMultiPass=true;
            iTMUTexture=1;
            iTMULightMap=0;
          }
          else
          {
            m_iMultiPass=false;
            iTMULightMap=/*hwconfig.SSTs[board].sstBoard.Voodoo2Config.nTexelfx/2*/1;
            iTMUTexture=/*hwconfig.SSTs[board].sstBoard.Voodoo2Config.nTexelfx-iTMULightMap*/1;
          }
    } break;
#endif

  case GR_SSTTYPE_VOODOO:
    {
      SysPrintf (CS_MSG_INITIALIZATION, " Board is a Voodoo Graphics.\n");
      SysPrintf (CS_MSG_INITIALIZATION, "  Pixelfx Revision %d.\n",hwconfig.SSTs[board].sstBoard.VoodooConfig.fbiRev);
      SysPrintf (CS_MSG_INITIALIZATION, "  Pixelfx Memory : %d MB.\n",hwconfig.SSTs[board].sstBoard.VoodooConfig.fbRam);
      SysPrintf (CS_MSG_INITIALIZATION, "  Number of Texelfx Chips %d.\n",hwconfig.SSTs[board].sstBoard.VoodooConfig.nTexelfx);
      for(i=0;i<hwconfig.SSTs[board].sstBoard.VoodooConfig.nTexelfx;i++)
        {
          SysPrintf (CS_MSG_INITIALIZATION, "   Texelfx nb %d has revision %d and %d MB of RAM.\n",i+1,
                     hwconfig.SSTs[board].sstBoard.VoodooConfig.tmuConfig[i].tmuRev,
                     hwconfig.SSTs[board].sstBoard.VoodooConfig.tmuConfig[i].tmuRam);
        }
      if (hwconfig.SSTs[board].sstBoard.VoodooConfig.sliDetect)
        SysPrintf (CS_MSG_INITIALIZATION, " SLI Detected.\n");

      if (hwconfig.SSTs[board].sstBoard.VoodooConfig.nTexelfx==1)
        {
          m_iMultiPass=true;
          iTMUTexture=1;
          iTMULightMap=0;
        }
      else
        {
          m_iMultiPass=false;
          iTMULightMap=/*hwconfig.SSTs[board].sstBoard.VoodooConfig.nTexelfx/2*/1;
          iTMUTexture=/*hwconfig.SSTs[board].sstBoard.VoodooConfig.nTexelfx-iTMULightMap*/1;
        }
    } break;

  case GR_SSTTYPE_SST96:
    {
      SysPrintf (CS_MSG_INITIALIZATION, " Board is a Rush.\n");
      SysPrintf (CS_MSG_INITIALIZATION, "  Pixelfx Memory : %d MB.\n",hwconfig.SSTs[board].sstBoard.SST96Config.fbRam);
      SysPrintf (CS_MSG_INITIALIZATION, "  Number of Texelfx Chips %d.\n",hwconfig.SSTs[board].sstBoard.SST96Config.nTexelfx);
      SysPrintf (CS_MSG_INITIALIZATION, "   Texelfx nb %d has revision %d and %d MB of RAM.\n",1,
                 hwconfig.SSTs[board].sstBoard.SST96Config.tmuConfig.tmuRev,
                 hwconfig.SSTs[board].sstBoard.SST96Config.tmuConfig.tmuRam);
      m_iMultiPass=true;
      iTMUTexture=1;
      iTMULightMap=0;
    } break;

  case GR_SSTTYPE_AT3D:
    {
      SysPrintf (CS_MSG_INITIALIZATION, " Board is an AT3D.\n");
      SysPrintf (CS_MSG_INITIALIZATION, "  Chipset Revision %d.\n",hwconfig.SSTs[board].sstBoard.AT3DConfig.rev);
      m_iMultiPass=true;
      iTMUTexture=1;
      iTMULightMap=0;
    }break;

  default:
    {
      SysPrintf (CS_MSG_INITIALIZATION, " Board is of an unknown type.(%x)\n",hwconfig.SSTs[board].type);
      m_iMultiPass=true;
      iTMUTexture=1;
      iTMULightMap=0;
    } break;
  }
  if(config->GetBool("Video.Glide.ForceMultipass",false)&& m_iMultiPass==false)
    {
      SysPrintf (CS_MSG_INITIALIZATION, " MultiPass Rendering enable by user.\n");
      m_iMultiPass=true;
    }

  if(m_iMultiPass)
    {
      SysPrintf (CS_MSG_INITIALIZATION, " Will use MultiPass Rendering.\n");
    }
  else
    {
      SysPrintf (CS_MSG_INITIALIZATION, " Will use SinglePass Rendering.\n");
      SysPrintf (CS_MSG_INITIALIZATION, " Affected %d TMU for Texture and %d for LightMap.\n",iTMUTexture,iTMULightMap);
    }
  m_TMUs = new TMUInfo[2];

  if (m_iMultiPass)
    {
      m_TMUs[0].tmu_id=m_TMUs[1].tmu_id=0;
      m_TMUs[0].minAddress = GlideLib_grTexMinAddress(0);
      m_TMUs[0].maxAddress = GlideLib_grTexMaxAddress(0);
    } 
  else {
    m_TMUs[0].tmu_id=0;
    m_TMUs[0].minAddress = GlideLib_grTexMinAddress(0);
    m_TMUs[0].maxAddress = GlideLib_grTexMaxAddress(0);
    m_TMUs[1].tmu_id=1;
    m_TMUs[1].minAddress = GlideLib_grTexMinAddress(1);
    m_TMUs[1].maxAddress = GlideLib_grTexMaxAddress(1);
  }
  m_TMUs[0].memory_size = (m_TMUs[0].maxAddress -  m_TMUs[0].minAddress);
  m_TMUs[1].memory_size = (m_TMUs[1].maxAddress -  m_TMUs[1].minAddress);

  GlideLib_grSstSelect(board);
  state = new GrState;
  fogtable = new GrFog_t[ GR_FOG_TABLE_SIZE ];
