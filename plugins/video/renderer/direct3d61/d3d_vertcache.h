#ifndef	D3D_VERTCACHE_H
#define D3D_VERTCACHE_H

class csVertexCacheDirect3D
{
private :
	IDirect3DDevice3 * m_Dev;

	unsigned int m_BufferSize;
	D3DTLVERTEX * m_pVertexBuffer;
	D3DTLVERTEX * m_pEndOfVertexBuffer;
	D3DTLVERTEX * m_pCurrentVertex;

public :
	csVertexCacheDirect3D() :
		m_pVertexBuffer(0),
		m_pEndOfVertexBuffer(0),
		m_pCurrentVertex(0)
	{
	}

	void Initialize(IDirect3DDevice3 * Dev, unsigned int BufferSize)
	{
		if (m_pVertexBuffer)
			return;		
		ASSERT(BufferSize);
		ASSERT(Dev);

		m_Dev = Dev;
		m_pVertexBuffer = new D3DTLVERTEX[BufferSize*3];
		m_pEndOfVertexBuffer = m_pVertexBuffer + BufferSize*3;
		m_pCurrentVertex = m_pVertexBuffer;
	}

	void ShutDown()
	{
		if (m_pVertexBuffer)
		{
			delete[] m_pVertexBuffer;
			m_pVertexBuffer = 0;
		}
	}

	inline D3DTLVERTEX * AddVertex()
	{
		m_pCurrentVertex++;
		if (m_pCurrentVertex >= m_pEndOfVertexBuffer)
			EmptyBuffer();
		return m_pCurrentVertex;
	}

	inline D3DTLVERTEX * AddPolygon()
	{		
		if (m_pCurrentVertex >= m_pEndOfVertexBuffer)
		{
			VERIFY_RESULT( m_Dev->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX,
												m_pVertexBuffer, 
												m_pCurrentVertex - m_pVertexBuffer,
												D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT), DD_OK);
			m_pCurrentVertex = m_pVertexBuffer;
		}

		D3DTLVERTEX * V = m_pCurrentVertex;
		m_pCurrentVertex += 3;
		return V;
	}

	inline void EmptyBuffer()
	{
		if (m_pCurrentVertex >= m_pVertexBuffer)
		{
			VERIFY_RESULT( m_Dev->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX,
												m_pVertexBuffer, 
												m_pCurrentVertex - m_pVertexBuffer,
												D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT), D3D_OK);

			m_pCurrentVertex = m_pVertexBuffer;
		}
	}

};	// end of class csVertexCacheDirect3D

#endif	// #ifdef D3D_VERTCACHE_H