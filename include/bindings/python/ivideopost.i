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

%pythoncode %{
  CS_REQUEST_PLUGIN = core.CS_REQUEST_PLUGIN
  def CS_REQUEST_NULL3D ():
    return CS_REQUEST_PLUGIN("crystalspace.graphics3d.null", iGraphics3D)

  def CS_REQUEST_SOFTWARE3D ():
    return CS_REQUEST_PLUGIN("crystalspace.graphics3d.software", iGraphics3D)

  def CS_REQUEST_OPENGL3D ():
    return CS_REQUEST_PLUGIN("crystalspace.graphics3d.opengl", iGraphics3D)

  def CS_REQUEST_FONTSERVER ():
    return CS_REQUEST_PLUGIN("crystalspace.font.server.default", iFontServer)

%}

%include "bindings/python/pythvarg.i"

