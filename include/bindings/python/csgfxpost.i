%include "bindings/python/pyshadervar.i"

%pythoncode %{
  def CS_REQUEST_IMAGELOADER ():
    return core.CS_REQUEST_PLUGIN("crystalspace.graphic.image.io.multiplexer",
      iImageIO)
%}

