#ifndef D3D_STATES_H
#define D3D_STATES_H

#include "iSystem.h"

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

	static int s_Count;
	IDirect3DDevice3 * m_Dev;
	iSystem * m_Sys;

	LPDIRECT3DTEXTURE2 m_Texture[MAX_STAGES];

	bool m_AlphaBlendEnable;
	bool m_ZBufferEnable;
	bool m_ColorKeyEnable;

	D3DBLEND m_SrcBlend;
	D3DBLEND m_DstBlend;

	D3DTEXTUREADDRESS m_TextureAddress;

	DWORD m_StageState[MAX_STAGES][MAX_STATES];

public :
	void Initialize(IDirect3DDevice3 * Dev, iSystem* Sys)
	{
		ASSERT(Dev);
		ASSERT(Sys);

		s_Count = 0;
		m_Dev = Dev;
		m_Sys = Sys;

	// Set Default States
		// set modulate textureblend in both stages, just in case
		m_Dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		m_Dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_Dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE); 
		m_Dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1); 
		m_Dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

//		m_Dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
//		m_Dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);
//		m_Dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		m_Dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
		m_Dev->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_Dev->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT); 
		m_Dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2); 
		m_Dev->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

		m_Dev->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
		m_Dev->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_LINEAR);
		m_Dev->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFG_LINEAR);
		m_Dev->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
		m_Dev->SetTextureStageState(1, D3DTSS_MINFILTER, D3DTFG_LINEAR);
		m_Dev->SetTextureStageState(1, D3DTSS_MIPFILTER, D3DTFG_LINEAR);

		m_Dev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
		m_Dev->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);

		m_Dev->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
		m_Dev->SetTextureStageState(1, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);

	// Get Current States
		m_Dev->GetRenderState(D3DRENDERSTATE_SRCBLEND, (unsigned long *) &m_SrcBlend);
		m_Dev->GetRenderState(D3DRENDERSTATE_DESTBLEND, (unsigned long *) &m_DstBlend);
		m_Dev->GetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, (unsigned long *) &m_AlphaBlendEnable);
		m_Dev->GetRenderState(D3DRENDERSTATE_COLORKEYENABLE, (unsigned long *) &m_ColorKeyEnable);
		m_Dev->GetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, (unsigned long *) &m_TextureAddress);

		unsigned int i, j;
		for (i = 0; i < MAX_STAGES; i++)
		{
			m_Dev->GetTexture(i, &m_Texture[i]);
			if (m_Texture[i])
				HRESULT h = m_Texture[i]->Release();
			for (j = 1; j < MAX_STATES; j++)
			{
				HRESULT hResult;
				hResult = m_Dev->GetTextureStageState(i, (D3DTEXTURESTAGESTATETYPE) j,
														&m_StageState[i][j]);
			}
		}
	}	// end of Initialize()
			
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
#if 0
		if (m_TextureAddress != Address)
		{
			m_TextureAddress = Address;			
			VERIFY_RESULT(m_Dev->SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, Address), DD_OK);
		}
		else
		{
			SysPrintf(MSG_CONSOLE, "Dublicate Tex Address %X\n", Address);
		}
#else
		SetStageState(0, D3DTSS_ADDRESS, Address);
#endif
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

};	// end of csStateCacheDirect3DDx6

#endif // #ifndef D3D_STATES_H