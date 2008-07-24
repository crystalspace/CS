#include "oceancell.h"

using namespace CS::Plugins::WaterMesh;

csOceanCell::csOceanCell(float len, float wid, OceanLOD level)
{
	this->len = len;
	this->wid = wid;
	
	type = level;
	
	oHeight = 0;
	
	gc = csVector2(0, 0);
	
	isSetup = false;
	
	bufferHoldersNeedSetup = true;
	buffersNeedSetup = false;
}

csOceanCell::~csOceanCell()
{
	
}

void csOceanCell::SetupVertices()
{
	if(!isSetup)
	{
		float gran;
		switch(type)
		{
			default:
			case LOD_LEVEL_1:
				gran = 0.1;
				break;
			case LOD_LEVEL_2:
				gran = 0.2;
				break;
			case LOD_LEVEL_3:
				gran = 0.5;
				break;
			case LOD_LEVEL_4:
				gran = 1.0;
				break;
			case LOD_LEVEL_5:
				gran = 2.0;
				break;
		}
	
		for(float j = 0; j < len * gran; j+=1)
		{
			for(float i = 0; i < wid * gran; i+=1)
			{
				verts.Push(csVector3 ((i * wid / (wid * gran - 1)), oHeight, (j * len / (len * gran - 1))));
				norms.Push(csVector3 (0, 1, 0));
				cols.Push(csColor (0.17,0.27,0.26));
				texs.Push(csVector2(i, j));
				//texs.Push(csVector2((i * wid / (wid * gran - 1)) / 1.5, (j * len / (len * gran - 1)) / 1.5));
			}
		}

		for(uint j = 0; j < (len * gran) - 1; j++)
		{
			for(uint i = 0; i < (wid * gran) - 1; i++)
			{
				tris.Push(csTriangle ((int)(j * (wid * gran) + i), 
										(int)((j + 1) * (wid * gran) + i), 
										(int)(j * (wid * gran) + i + 1)));
				tris.Push(csTriangle ((int)(j * (wid * gran) + i + 1),
										(int)((j + 1) * (wid * gran) + i),
										(int)((j + 1) * (wid * gran) + i + 1)));
			}
		}
	
		buffersNeedSetup = true;
		isSetup = true;
	}
}

void csOceanCell::SetupBuffers()
{
	if(!buffersNeedSetup)
		return;
	
	if (!vertex_buffer)
    {
      // Create a buffer that doesn't copy the data.
      vertex_buffer = csRenderBuffer::CreateRenderBuffer (
        verts.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
        3);
    }
    vertex_buffer->CopyInto (verts.GetArray(), verts.GetSize());

    if (!texel_buffer)
    {
      // Create a buffer that doesn't copy the data.
      texel_buffer = csRenderBuffer::CreateRenderBuffer (
		verts.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
        2);
    }
    texel_buffer->CopyInto (texs.GetArray(), verts.GetSize());

    if (!index_buffer)
	{
      index_buffer = csRenderBuffer::CreateIndexRenderBuffer (
      tris.GetSize()*3,
      CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT,
      0, verts.GetSize()-1);
	}
    index_buffer->CopyInto (tris.GetArray(), tris.GetSize()*3);

    if (!normal_buffer)
	{            
	  // Create a buffer that doesn't copy the data.
      normal_buffer = csRenderBuffer::CreateRenderBuffer (
        norms.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
        3);
	}
    normal_buffer->CopyInto (norms.GetArray(), norms.GetSize());

    if (!color_buffer)
	{            
	  // Create a buffer that doesn't copy the data.
      color_buffer = csRenderBuffer::CreateRenderBuffer (
        cols.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
        3);
	}
    color_buffer->CopyInto (cols.GetArray(), cols.GetSize());

	buffersNeedSetup = false;
}

void csOceanCell::SetupBufferHolder()
{
	if(!bufferHoldersNeedSetup)
		return;
	
	if(bufferHolder == 0)
		bufferHolder.AttachNew(new csRenderBufferHolder);

	bufferHolder->SetRenderBuffer(CS_BUFFER_INDEX, index_buffer);
	bufferHolder->SetRenderBuffer(CS_BUFFER_POSITION, vertex_buffer);
	bufferHolder->SetRenderBuffer(CS_BUFFER_TEXCOORD0, texel_buffer);
	
	//Ocean color and normals shouldn't change..
	bufferHolder->SetRenderBuffer(CS_BUFFER_NORMAL, normal_buffer);
	bufferHolder->SetRenderBuffer(CS_BUFFER_COLOR, color_buffer);
	
	bufferHoldersNeedSetup = false;
}

////////////// csOceanNode //////////////////


csOceanNode::csOceanNode(csVector2 pos, float len, float wid)
{
	gc = pos;
	this->len = len;
	this->wid = wid;
	oHeight = 0;
	
	CalculateBBox();
}

csOceanNode::~csOceanNode()
{
	
}

csOceanNode csOceanNode::GetLeft() const
{
	return csOceanNode(csVector2(gc.x + len, gc.y), len, wid);
}

csOceanNode csOceanNode::GetRight() const
{
	return csOceanNode(csVector2(gc.x - len, gc.y), len, wid);
}

csOceanNode csOceanNode::GetUp() const
{
	return csOceanNode(csVector2(gc.x, gc.y + wid), len, wid);
}

csOceanNode csOceanNode::GetDown() const
{
	return csOceanNode(csVector2(gc.x, gc.y - wid), len, wid);
}

csVector3 csOceanNode::GetCenter() const
{
	return csVector3(gc.x + (len / 2), oHeight, gc.y + (wid / 2));
}

void csOceanNode::CalculateBBox()
{
	bbox = csBox3(gc.x, oHeight + EPSILON, gc.y, gc.x + len, oHeight - EPSILON, gc.y + wid);
}