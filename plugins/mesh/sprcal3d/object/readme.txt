sprcal3d requires that you have built cal3d from cvs and that the library is
findable by the CS build process, and that the shared library can be found at
runtime. (I put mine in the CS root dir.)

I implemented a substitute for .spr format files in xml, which I've called
.cal3d files. There are a couple of examples in the sprcal3dldr directory.  To
run these, move them to CS root and type: viewmesh /this/<name>.cal3d

The default cal3d models, such as cally and paladin, are very large (100 units
high), and so you will probably need: viewmesh -RoomSize=100 /this/<name>.cal3d 

Incidentally, viewmesh has been modified to take this parameter (optionally)
and use it.

N.B. : The materials will not be shown on the default cal3d models because
they are in a file format not supported by CS.  With your own exports, if you
simply supply the texture filenames as the material files (or names of
preloaded CS materials if you desire) it will work fine.

Also N.B. : cal3d seems to use the reverse Z/Y axis order.  Maybe someone can
solve this with an additional transform in the O2T thing, but simply rotating
the models to lie them down on their backs in Max before exporting is also
easy enough.

Currently there is no API to externally change from the default animation.  I
intend to do this next.
