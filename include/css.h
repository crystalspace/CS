/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Based on CSScript module created by Brandon Ehle
    Copyright (C) 2002 by W.C.A. Wijngaards
  
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

// The queen of all include files.  Good for being a precompiled header, by
// utilizing the NO_* macros, while stop a complete build

#ifndef __CSS_H__
#define __CSS_H__

#ifndef CS_INCLUDE_NOTHING

// SysDef
#ifndef NO_CSSYSDEF
// make it safe so no double cssysdef.h includes
#ifndef __CS_CSSYSDEFS_H__

#define CS_SYSDEF_PROVIDE_CASE
#define CS_SYSDEF_PROVIDE_PATH
#define CS_SYSDEF_PROVIDE_MKDIR
#define CS_SYSDEF_PROVIDE_GETCWD
#define CS_SYSDEF_PROVIDE_TEMP
#define CS_SYSDEF_PROVIDE_DIR
#define CS_SYSDEF_PROVIDE_UNLINK
#define CS_SYSDEF_PROVIDE_ACCESS
#define CS_SYSDEF_PROVIDE_ALLOCA
#define CS_SYSDEF_PROVIDE_GETOPT
#define CS_SYSDEF_PROVIDE_SOCKETS
#define CS_SYSDEF_PROVIDE_SELECT
#include "cssysdef.h"
#endif
#endif 

// CS Version
#ifndef NO_CSVER
#include "csver.h"
#endif

// quick int and sqrt
#ifndef NO_CSQINTQSQRT
#include "qint.h"
#include "qsqrt.h"
#endif

// SCF
#ifndef NO_CSSCF
#include "csutil/scf.h"
#endif

// CS Sys
#ifndef NO_CSSYS
#include "cssys/sysdriv.h"
#include "cssys/csendian.h"
#include "cssys/csshlib.h"
#include "cssys/getopt.h"
#include "cssys/sysfunc.h"
#include "cssys/system.h"
#endif

// CS Interfaces
#ifndef NO_CSINTERFACE
#include "iaws/aws.h"
#include "iaws/awscnvs.h"
#include "iaws/awsdefs.h"
//#include "iaws/awsecomp.h"
#include "iaws/awsparm.h"
#include "iengine/camera.h"
#include "iengine/campos.h"
#include "iengine/collectn.h"
#include "iengine/dynlight.h"
#include "iengine/engine.h"
#include "iengine/fview.h"
#include "iengine/halo.h"
#include "iengine/light.h"
#include "iengine/lod.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/motion.h"
#include "iengine/movable.h"
#include "iengine/region.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "iengine/shadcast.h"
#include "iengine/shadows.h"
#include "iengine/skelbone.h"
#include "iengine/statlght.h"
#include "iengine/texture.h"
#include "iengine/viscull.h"
#include "igeom/clip2d.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "imap/parser.h"
#include "imap/reader.h"
#include "imap/services.h"
#include "imap/writer.h"
#include "imesh/ball.h"
#include "imesh/crossbld.h"
#include "imesh/emit.h"
#include "imesh/explode.h"
#include "imesh/fire.h"
#include "imesh/fountain.h"
#include "imesh/genmesh.h"
#include "imesh/haze.h"
#include "imesh/lighting.h"
#include "imesh/mdlconv.h"
#include "imesh/mdldata.h"
#include "imesh/metaball.h"
#include "imesh/metagen.h"
#include "imesh/object.h"
#include "imesh/particle.h"
#include "imesh/partsys.h"
#include "imesh/rain.h"
#include "imesh/skeleton.h"
#include "imesh/snow.h"
#include "imesh/spiral.h"
#include "imesh/sprite2d.h"
#include "imesh/sprite3d.h"
#include "imesh/stars.h"
#include "imesh/terrfunc.h"
#include "imesh/thing/curve.h"
#include "imesh/thing/lightmap.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/polytmap.h"
#include "imesh/thing/portal.h"
#include "imesh/thing/ptextype.h"
#include "imesh/thing/thing.h"
#include "inetwork/driver.h"
#include "inetwork/socket.h"
#include "isound/data.h"
#include "isound/driver.h"
#include "isound/handle.h"
#include "isound/listener.h"
#include "isound/loader.h"
#include "isound/renderer.h"
#include "isound/source.h"
#include "isound/wrapper.h"
#include "iutil/cfgfile.h"
#include "iutil/cfgmgr.h"
#include "iutil/cmdline.h"
#include "iutil/comp.h"
#include "iutil/config.h"
#include "iutil/csinput.h"
#include "iutil/databuff.h"
#include "iutil/evdefs.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/object.h"
#include "iutil/objref.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/string.h"
#include "iutil/strvec.h"
#include "iutil/vfs.h"
#include "iutil/virtclk.h"
#include "ivaria/collider.h"
#include "ivaria/conin.h"
#include "ivaria/conout.h"
#include "ivaria/iso.h"
#include "ivaria/keyval.h"
#include "ivaria/lexan.h"
#include "ivaria/mapnode.h"
#include "ivaria/perfstat.h"
#include "ivaria/pmeter.h"
#include "ivaria/polymesh.h"
#include "ivaria/reporter.h"
#include "ivaria/script.h"
#include "ivaria/sequence.h"
#include "ivaria/stdrep.h"
#include "ivaria/view.h"
#include "ivideo/codec.h"
#include "ivideo/cursor.h"
#include "ivideo/fontserv.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/halo.h"
#include "ivideo/material.h"
#include "ivideo/natwin.h"
#include "ivideo/sproctxt.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "ivideo/vbufmgr.h"
//#include "ivideo/xextf86vm.h"
//#include "ivideo/xextshm.h"
//#include "ivideo/xwindow.h"
#endif

// CS Geom
#ifndef NO_CSGEOM
#include "csgeom/box.h"
#include "csgeom/cspoint.h"
#include "csgeom/csrect.h"
#include "csgeom/csrectrg.h"
#include "csgeom/fastsqrt.h"
#include "csgeom/frustum.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/math3d_d.h"
#include "csgeom/matrix2.h"
#include "csgeom/matrix3.h"
#include "csgeom/path.h"
#include "csgeom/plane2.h"
#include "csgeom/plane3.h"
#include "csgeom/poly2d.h"
#include "csgeom/poly3d.h"
#include "csgeom/polyaa.h"
#include "csgeom/polyclip.h"
#include "csgeom/polyedge.h"
#include "csgeom/polyidx.h"
#include "csgeom/polypool.h"
#include "csgeom/quaterni.h"
#include "csgeom/segment.h"
#include "csgeom/sphere.h"
#include "csgeom/spline.h"
#include "csgeom/subrec.h"
#include "csgeom/tesselat.h"
#include "csgeom/textrans.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "csgeom/vtpool.h"
#endif

// CS Engine
#ifndef NO_CSENGINE
#include "csengine/arrays.h"
#include "csengine/bezier.h"
#include "csengine/bsp.h"
#include "csengine/bsp2d.h"
#include "csengine/bspbbox.h"
#include "csengine/camera.h"
#include "csengine/campos.h"
#include "csengine/cbufcube.h"
#include "csengine/cbuffer.h"
#include "csengine/crysball.h"
#include "csengine/cscoll.h"
#include "csengine/curve.h"
#include "csengine/engine.h"
#include "csengine/halo.h"
#include "csengine/lghtmap.h"
#include "csengine/light.h"
#include "csengine/lppool.h"
#include "csengine/lview.h"
#include "csengine/material.h"
#include "csengine/meshobj.h"
#include "csengine/movable.h"
#include "csengine/octree.h"
#include "csengine/pol2d.h"
#include "csengine/poledges.h"
#include "csengine/polygon.h"
#include "csengine/polyint.h"
#include "csengine/polyplan.h"
#include "csengine/polytext.h"
#include "csengine/polytmap.h"
#include "csengine/polytree.h"
#include "csengine/portal.h"
#include "csengine/radiosty.h"
#include "csengine/rdrprior.h"
#include "csengine/region.h"
#include "csengine/rview.h"
#include "csengine/sector.h"
#include "csengine/stats.h"
#include "csengine/texture.h"
#include "csengine/thing.h"
#include "csengine/treeobj.h"
#include "csengine/wirefrm.h"
#endif

// CS Tool
#ifndef NO_CSTOOL
#include "cstool/collider.h"
#include "cstool/csanim2d.h"
#include "cstool/csfxscr.h"
#include "cstool/cspixmap.h"
#include "cstool/csview.h"
#include "cstool/gentrtex.h"
#include "cstool/initapp.h"
#include "cstool/keyval.h"
#include "cstool/mapnode.h"
#include "cstool/mdldata.h"
#include "cstool/mdltool.h"
#include "cstool/prdots.h"
#include "cstool/prfire.h"
#include "cstool/proctex.h"
#include "cstool/prplasma.h"
#include "cstool/prsky.h"
#include "cstool/prwater.h"
#include "cstool/sndwrap.h"
#include "cstool/sprbuild.h"
#endif

// CS Util
#ifndef NO_CSUTIL
#include "csutil/2bitary.h"
#include "csutil/archive.h"
#include "csutil/bitarray.h"
#include "csutil/bitary2d.h"
#include "csutil/bitset.h"
#include "csutil/cfgacc.h"
#include "csutil/cfgfile.h"
#include "csutil/cfgmgr.h"
#include "csutil/cmdhelp.h"
#include "csutil/cmdline.h"
#include "csutil/cscolor.h"
#include "csutil/csctype.h"
#include "csutil/csdllist.h"
#include "csutil/csevcord.h"
#include "csutil/csevent.h"
#include "csutil/cseventq.h"
#include "csutil/csinput.h"
#include "csutil/cskeys.h"
#include "csutil/csmd5.h"
#include "csutil/csobject.h"
#include "csutil/csobjvec.h"
#include "csutil/cspmeter.h"
#include "csutil/csppulse.h"
#include "csutil/csqueue.h"
#include "csutil/csstring.h"
#include "csutil/csstrvec.h"
#include "csutil/cstreend.h"
#include "csutil/csvector.h"
#include "csutil/databuf.h"
#include "csutil/dataobj.h"
#include "csutil/datastrm.h"
#include "csutil/debug.h"
#include "csutil/evoutlet.h"
#include "csutil/flags.h"
#include "csutil/garray.h"
#include "csutil/halogen.h"
#include "csutil/hashmap.h"
#include "csutil/intarray.h"
#include "csutil/memfile.h"
#include "csutil/mmapio.h"
#include "csutil/nobjvec.h"
#include "csutil/objiter.h"
#include "csutil/objpool.h"
#include "csutil/objreg.h"
#include "csutil/parser.h"
#include "csutil/plugldr.h"
#include "csutil/plugmgr.h"
#include "csutil/prfxcfg.h"
#include "csutil/rng.h"
#include "csutil/sarray.h"
#include "csutil/scanstr.h"
#include "csutil/scf.h"
#include "csutil/scfstr.h"
#include "csutil/scfstrv.h"
#include "csutil/schedule.h"
#include "csutil/snprintf.h"
#include "csutil/sparse3d.h"
#include "csutil/strset.h"
#include "csutil/token.h"
#include "csutil/typedvec.h"
#include "csutil/util.h"
#include "csutil/virtclk.h"
//#include "csutil/zip.h"
#endif

// CS Gfx
#ifndef NO_CSGFX
#include "csgfx/bumpmap.h"
#include "csgfx/csimage.h"
#include "csgfx/inv_cmap.h"
#include "csgfx/memimage.h"
#include "csgfx/quantize.h"
#include "csgfx/rgbpixel.h"
#include "csgfx/xorpat.h"
#endif

// CS Phyzik
#ifndef NO_CSPHYZIK
#include "csphyzik/articula.h"
#include "csphyzik/bodyforc.h"
#include "csphyzik/contact.h"
#include "csphyzik/ctcat.h"
#include "csphyzik/ctcontct.h"
#include "csphyzik/ctmatrix.h"
#include "csphyzik/ctmspat.h"
#include "csphyzik/ctquat.h"
#include "csphyzik/ctvector.h"
#include "csphyzik/ctvspat.h"
#include "csphyzik/debug.h"
#include "csphyzik/entity.h"
#include "csphyzik/feathers.h"
#include "csphyzik/force.h"
#include "csphyzik/forces.h"
#include "csphyzik/ik.h"
#include "csphyzik/joint.h"
#include "csphyzik/kinemat.h"
#include "csphyzik/linklist.h"
#include "csphyzik/math3d.h"
#include "csphyzik/mathutil.h"
#include "csphyzik/mc.h"
#include "csphyzik/mtrxutil.h"
#include "csphyzik/odesolve.h"
#include "csphyzik/phyzent.h"
#include "csphyzik/phyziks.h"
#include "csphyzik/phyztype.h"
#include "csphyzik/point.h"
#include "csphyzik/ptforce.h"
#include "csphyzik/ptmass.h"
#include "csphyzik/qtrbconn.h"
#include "csphyzik/qtrigid.h"
#include "csphyzik/refframe.h"
#include "csphyzik/rigidbod.h"
#include "csphyzik/solver.h"
#include "csphyzik/world.h"
#endif

// CSWS
#ifndef NO_CSWS
#include "csws/csws.h"
#endif

#endif // CS_INCLUDE_NOTHING

#endif // __CSS_H__
