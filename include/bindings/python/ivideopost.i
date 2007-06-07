%extend csSimpleRenderMesh
{
  void SetWithGenmeshFactory(iGeneralFactoryState *factory)
  {
    self->vertices = factory->GetVertices();
    self->vertexCount = factory->GetVertexCount();
    self->indices = (uint *)factory->GetTriangles();
    self->indexCount = factory->GetTriangleCount()*3;
    self->texcoords = factory->GetTexels();
  }
}
%include "bindings/python/pythvarg.i"

