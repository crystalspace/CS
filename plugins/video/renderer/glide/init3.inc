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
 * Initialization for glide3
 */

  long ret, val;
  ret = GlideLib_grGet( GR_NUM_BOARDS, 4, &val);

  if( !ret || !val) 
    sys_fatalerror("csGraphics3DGlide::Open : No 3dfx chip found");

  GlideLib_grGlideInit();

  SysPrintf (CS_MSG_INITIALIZATION, " Glide %s detected.\n", GlideLib_grGetString( GR_VERSION ));
  
  const char* szBoard = GlideLib_grGetString ( GR_HARDWARE );
  board = 0;
  SysPrintf (CS_MSG_INITIALIZATION, " Board %d selected.\n", board);

  long pixelfx, texelfx, revtfx, revpfx, mempfx, memtfx;
  
  val = pixelfx = texelfx = revtfx = revpfx = mempfx = memtfx = 0;
  
  val = GlideLib_grGet ( GR_NUM_FB, 4, &pixelfx );
  val = GlideLib_grGet ( GR_NUM_TMU, 4, &texelfx );
  val = GlideLib_grGet ( GR_REVISION_FB, 4, &revpfx );
  val = GlideLib_grGet ( GR_REVISION_TMU, 4, &revtfx );
  val = GlideLib_grGet ( GR_MEMORY_FB, 4, &mempfx );
  val = GlideLib_grGet ( GR_MEMORY_TMU, 4, &memtfx );
  texelfx *= pixelfx;
  
  SysPrintf (CS_MSG_INITIALIZATION, " Board is a %s.\n", szBoard);
  SysPrintf (CS_MSG_INITIALIZATION, "  Number of Pixelfx Chips %d.\n", pixelfx);
  SysPrintf (CS_MSG_INITIALIZATION, "  Number of Texelfx Chips %d.\n", texelfx);
  SysPrintf (CS_MSG_INITIALIZATION, "  Pixelfx Revision %d.\n", revpfx);
  SysPrintf (CS_MSG_INITIALIZATION, "  Texelfx Revision %d.\n", revtfx);
  SysPrintf (CS_MSG_INITIALIZATION, "  Pixelfx Memory per pixelfx chip %d.\n", mempfx);
  SysPrintf (CS_MSG_INITIALIZATION, "  Texelfx Memory per texelfx chip %d.\n", memtfx);

  m_iMultiPass = (texelfx == 1);
  iTMUTexture  = 1;
  iTMULightMap = ( m_iMultiPass ? 0 : 1);
  
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
      m_TMUs[0].tmu_id=m_TMUs[1].tmu_id=GR_TMU0;
      m_TMUs[0].minAddress = GlideLib_grTexMinAddress(GR_TMU0);
      m_TMUs[0].maxAddress = GlideLib_grTexMaxAddress(GR_TMU0);
    } 
  else {
    m_TMUs[0].tmu_id=GR_TMU0;
    m_TMUs[0].minAddress = GlideLib_grTexMinAddress(GR_TMU0);
    m_TMUs[0].maxAddress = GlideLib_grTexMaxAddress(GR_TMU0);
    m_TMUs[1].tmu_id=GR_TMU1;
    m_TMUs[1].minAddress = GlideLib_grTexMinAddress(GR_TMU0);
    m_TMUs[1].maxAddress = GlideLib_grTexMaxAddress(GR_TMU0);
  }
  m_TMUs[0].memory_size = (m_TMUs[0].maxAddress -  m_TMUs[0].minAddress);
  m_TMUs[1].memory_size = (m_TMUs[1].maxAddress -  m_TMUs[1].minAddress);

  GlideLib_grSstSelect(board);

  // generate fogtable
  GlideLib_grGet( GR_FOG_TABLE_ENTRIES, 4, &val );
  fogtable = new GrFog_t[ val ];

  GlideLib_grGet( GR_GLIDE_STATE_SIZE, 4, &val );
  state = new UByte[ val ];

  GlideLib_grGet (GR_WDEPTH_MIN_MAX, 8, m_wminmax );

