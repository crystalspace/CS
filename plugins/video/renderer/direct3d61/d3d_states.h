#ifndef D3D_STATES_H
#define D3D_STATES_H

#include "isys/isystem.h"

//====================================================
// csStateCacheDirect3DDx6
// attempts to minimize redundant render state changes
//====================================================
class csStateCacheDirect3DDx6
{
private :
  enum
  {
    MAX_STAGES = 2,
    MAX_STATES = 24,
  };

  IDirect3DDevice3 * m_Dev;
  iSystem * m_Sys;

//-- render states
  LPDIRECT3DTEXTURE2 m_Texture[MAX_STAGES];

  bool m_AlphaBlendEnable;
  bool m_ZBufferEnable;
  bool m_ColorKeyEnable;
  D3DBLEND m_SrcBlend;
  D3DBLEND m_DstBlend;
  D3DTEXTUREADDRESS m_TextureAddress;
  D3DCMPFUNC m_ZFunc;

  DWORD m_StageState[MAX_STAGES][MAX_STATES];

//-- pushed states

  LPDIRECT3DTEXTURE2 m_PushTexture[MAX_STAGES];
  bool m_PushAlphaBlendEnable;
  bool m_PushZBufferEnable;
  bool m_PushColorKeyEnable;
  D3DBLEND m_PushSrcBlend;
  D3DBLEND m_PushDstBlend;
  D3DTEXTUREADDRESS m_PushTextureAddress;
  D3DCMPFUNC m_PushZFunc;

public :
  void Initialize(IDirect3DDevice3 * Dev, iSystem* Sys)
  {
    ASSERT(Dev);
    ASSERT(Sys);

    m_Dev = Dev;
    m_Sys = Sys;

  // Get Current States
    m_Dev->GetRenderState(D3DRENDERSTATE_SRCBLEND, (unsigned long *) &m_SrcBlend);
    m_Dev->GetRenderState(D3DRENDERSTATE_DESTBLEND, (unsigned long *) &m_DstBlend);
    m_Dev->GetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, (unsigned long *) &m_AlphaBlendEnable);
    m_Dev->GetRenderState(D3DRENDERSTATE_COLORKEYENABLE, (unsigned long *) &m_ColorKeyEnable);
    m_Dev->GetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, (unsigned long *) &m_TextureAddress);
    m_Dev->GetRenderState(D3DRENDERSTATE_ZFUNC, (unsigned long *) &m_ZFunc);

    unsigned int i, j;
    for (i = 0; i < MAX_STAGES; i++)
    {
      m_Dev->GetTexture(i, &m_Texture[i]);
      if (m_Texture[i])
        m_Texture[i]->Release ();
      for (j = 1; j < MAX_STATES; j++)
      {
        HRESULT hResult;
        hResult = m_Dev->GetTextureStageState(i, (D3DTEXTURESTAGESTATETYPE) j,
                            &m_StageState[i][j]);
      }
    }
  } // end of Initialize()

//--- set render state implementations ---
      
  inline void SetTexture(int Stage, LPDIRECT3DTEXTURE2 Tex)
  {
    if (m_Texture[Stage] != Tex)
    {
      m_Texture[Stage] = Tex;
      VERIFY_RESULT(m_Dev->SetTexture(Stage, Tex), DD_OK);
    }
  }

  inline void SetAlphaBlendEnable(bool Flag)
  {
    if (m_AlphaBlendEnable != Flag)
    {
      m_AlphaBlendEnable = Flag;
      VERIFY_RESULT(m_Dev->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, Flag), DD_OK);
    }
  }

  inline void SetSrcBlend(D3DBLEND Blend)
  {
    if (m_SrcBlend != Blend)
    {
      m_SrcBlend = Blend;
      VERIFY_RESULT(m_Dev->SetRenderState(D3DRENDERSTATE_SRCBLEND, Blend), DD_OK);
    }
  }

  inline void SetDstBlend(D3DBLEND Blend)
  {
    if (m_DstBlend != Blend)
    {
      m_DstBlend = Blend;
      VERIFY_RESULT(m_Dev->SetRenderState(D3DRENDERSTATE_DESTBLEND, Blend), DD_OK);
    }
  }

  inline void SetColorKeyEnable(bool Flag)
  {
    if (m_ColorKeyEnable != Flag)
    {
      m_ColorKeyEnable = Flag;
      VERIFY_RESULT(m_Dev->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, Flag), DD_OK);
    }
  }

  inline void SetTextureAddress(D3DTEXTUREADDRESS Address)
  {
    SetStageState(0, D3DTSS_ADDRESS, Address);
  }
#if 0
    if (m_TextureAddress != Address)
    {
      m_TextureAddress = Address;     
      VERIFY_RESULT(m_Dev->SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, Address), DD_OK);
    }
    else
    {
      m_piSystem->Printf(MSG_CONSOLE, "Dublicate Tex Address %X\n", Address);
    }
#endif

  inline void SetZFunc(D3DCMPFUNC Func)
  {
    if (m_ZFunc != Func)
    {
      m_ZFunc = Func;
      VERIFY_RESULT(m_Dev->SetRenderState(D3DRENDERSTATE_ZFUNC, Func), DD_OK);
    }
  }

  inline void SetStageState(unsigned int Stage,
                D3DTEXTURESTAGESTATETYPE State,
                DWORD Value)
  {
    ASSERT(Stage <= MAX_STAGES);
    if (m_StageState[Stage][State] != Value)
    {
      m_StageState[Stage][State] = Value;
      VERIFY_RESULT(m_Dev->SetTextureStageState(Stage, State, Value), DD_OK);
    }
  }

//--- push/pop implementations ---
//--- NOTE : DX7 has this in it's new features I believe ---

  void PushTexture(int Stage) { m_PushTexture[Stage] = m_Texture[Stage]; }
  // this may not work very well because of texture caching
  void PopTexture(int Stage) { SetTexture(Stage, m_PushTexture[Stage]); }
  void PushAlphaBlendEnable() { m_PushAlphaBlendEnable = m_AlphaBlendEnable; }
  void PopAlphaBlendEnable() { SetAlphaBlendEnable(m_PushAlphaBlendEnable); }
  void PushSrcBlend() { m_PushSrcBlend = m_SrcBlend; }
  void PopSrcBlend() { SetSrcBlend(m_PushSrcBlend); }
  void PushDstBlend() { m_PushDstBlend = m_DstBlend; }
  void PopDstBlend() { SetDstBlend(m_PushDstBlend); }
  void PushColorKeyEnable() { m_PushColorKeyEnable = m_ColorKeyEnable; }
  void PopColorKeyEnable() { SetColorKeyEnable(m_PushColorKeyEnable); }
  void PushTextureAddress() { m_PushTextureAddress = m_TextureAddress; }
  void PopTextureAddress() { SetTextureAddress(m_PushTextureAddress); }
  void PushZFunc() { m_PushZFunc = m_ZFunc; }
  void PopZFunc() { SetZFunc(m_PushZFunc); }

};  // end of csStateCacheDirect3DDx6

#endif // #ifndef D3D_STATES_H
