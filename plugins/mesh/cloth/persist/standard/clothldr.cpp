
#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "csgeom/math3d.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "clothldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "iutil/document.h"
#include "imesh/clothmesh.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/object.h"
#include "iengine/material.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/services.h"
#include "imap/ldrctxt.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_DEFAULT = 1,
  XMLTOKEN_LIGHTING,
  XMLTOKEN_COLOR,
  XMLTOKEN_TEXMAP,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_FACTORY,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_MANUALCOLORS,
  XMLTOKEN_NUMTRI,
  XMLTOKEN_NUMVT,
  XMLTOKEN_V,
  XMLTOKEN_P,
  XMLTOKEN_N,
  XMLTOKEN_COLORS,
  XMLTOKEN_AUTONORMALS
};

SCF_IMPLEMENT_IBASE (csClothFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csClothFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csClothFactoryLoader)

SCF_IMPLEMENT_IBASE (csClothFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csClothFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csClothFactorySaver)

SCF_IMPLEMENT_IBASE (csClothMeshLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
 SCF_IMPLEMENT_IBASE_END
 
SCF_IMPLEMENT_EMBEDDED_IBASE (csClothMeshLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csClothMeshLoader)

SCF_EXPORT_CLASS_TABLE (clothldr)
  SCF_EXPORT_CLASS (csClothFactoryLoader,"crystalspace.mesh.loader.factory.cloth",
    "Crystal Space Cloth Mesh Factory Loader")
  SCF_EXPORT_CLASS (csClothFactorySaver, "crystalspace.mesh.saver.factory.cloth",
    "Crystal Space Cloth Mesh Factory Saver")
  SCF_EXPORT_CLASS (csClothMeshLoader, "crystalspace.mesh.loader.cloth","Crystal Space Cloth Mesh Object Loader")
  //SCF_EXPORT_CLASS (csClothMeshSaver, "crystalspace.mesh.saver.cloth","Crystal Space General Mesh Mesh Saver")
SCF_EXPORT_CLASS_TABLE_END

static void ReportError (iReporter* reporter, const char* id,
	const char* description, ...)
{
  va_list arg;
  va_start (arg, description);

  if (reporter)
  {
    reporter->ReportV (CS_REPORTER_SEVERITY_ERROR, id, description, arg);
  }
  else
  {
    char buf[1024];
    vsprintf (buf, description, arg);
    csPrintf ("Error ID: %s\n", id);
    csPrintf ("Description: %s\n", buf);
  }
  va_end (arg);
};

/*
 bool csClothFactoryLoader::ParsePolygonAndTriangulate (  
                               iDocumentNode*       node,
                               iLoaderContext*      ldr_context,
                               iEngine*             engine,
                               csTriangle*          TriangleBuffer,
                               uint*                TriangleOffset,
	                           float                default_texlen,
	                           iClothFactoryState*  state )
{
  iMaterialWrapper* mat = NULL;

  if (!type)
  {
    csRef<iPluginManager> plugin_mgr (
    	CS_QUERY_REGISTRY (object_reg, iPluginManager));
    CS_ASSERT (plugin_mgr != NULL);
    type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	  "crystalspace.mesh.object.cloth", iMeshObjectType);
    if (!type)
      type = CS_LOAD_PLUGIN (plugin_mgr,
    	  "crystalspace.mesh.object.cloth", iMeshObjectType);
  }

  CS_ASSERT (type != NULL);
  //csRef<iThingEnvironment> te (SCF_QUERY_INTERFACE (*type, iThingEnvironment));

  uint V_nodes = 0;
  uint texspec = 0;
  int tx_uv_i1 = 0;
  int tx_uv_i2 = 0;
  int tx_uv_i3 = 0;
  csVector2 tx_uv1;
  csVector2 tx_uv2;
  csVector2 tx_uv3;

  csVector3 tx_orig (0, 0, 0), tx1 (0, 0, 0), tx2 (0, 0, 0);
  csVector3 tx_len (default_texlen, default_texlen, default_texlen);

  csMatrix3 tx_matrix;
  csVector3 tx_vector (0, 0, 0);
  char plane_name[100];
  plane_name[0] = 0;
  csVector2 uv_shift (0, 0);

  bool do_mirror = false;
  int set_colldet = 0; // If 1 then set, if -1 then reset, else default.

  bool init_gouraud_poly = false;
  csRef<iPolyTexFlat> fs;
  csRef<iPolyTexGouraud> gs;
  int num_uv = 0;
  int num_col = 0;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_MATERIAL:
       // mat = ldr_context->FindMaterial (child->GetContentsValue ());
       // if (mat == NULL)
       // {
       //   ReportError ("crystalspace.syntax.polygon", child,
       //     "Couldn't find material named '%s'!", child->GetContentsValue ());
       //   return false;
       //  }
       // poly3d->SetMaterial (mat);
        break;
      case XMLTOKEN_TEXMAP:
	//if (!synldr->ParseTextureMapping (child, thing_state->GetVertices (), texspec,
	//		   tx_orig, tx1, tx2, tx_len,
	//		   tx_matrix, tx_vector,
	//		   uv_shift,
	//		   tx_uv_i1, tx_uv1,
	//		   tx_uv_i2, tx_uv2,
	//		   tx_uv_i3, tx_uv3,
	//		   plane_name, poly3d->QueryObject ()->GetName ()))
	//{
	 // return false;
	//}
  {
	  
  }
        break;
      case XMLTOKEN_V:
        {
			switch(V_nodes)
			{
				case 0:
					TriangleBuffer[ *TriangleOffset ].a = child->GetContentsValueAsInt ();
				    V_nodes++;
				  break;
				case 1:
					TriangleBuffer[ *TriangleOffset ].b = child->GetContentsValueAsInt ();
				    V_nodes++;
				  break;
				case 2:
					TriangleBuffer[ *TriangleOffset ].c = child->GetContentsValueAsInt ();
				    V_nodes++;
				  break;
				default:
					(*TriangleOffset)++ ;
				    TriangleBuffer[ *TriangleOffset ].a 
				      = TriangleBuffer[ *TriangleOffset - 1 ].a;
				    TriangleBuffer[ *TriangleOffset ].b 
				      = TriangleBuffer[ *TriangleOffset - 1 ].c;				
				    V_nodes=2;
				    								    
					//ReportError ("crystalspace.syntax.polygon", child," triangle with too much vertices!");
		            //return false;
				
			};
		}
        break;
      case XMLTOKEN_SHADING:
	{
	  int shading;
	  const char* shad = child->GetContentsValue ();
          if (!strcasecmp (shad, "none"))
	    shading = POLYTXT_NONE;
	  else if (!strcasecmp (shad, "flat"))
	    shading = POLYTXT_FLAT;
	  else if (!strcasecmp (shad, "gouraud"))
	    shading = POLYTXT_GOURAUD;
	  else if (!strcasecmp (shad, "lightmap"))
	    shading = POLYTXT_LIGHTMAP;
	  else
	  {
	    ReportError ("crystalspace.syntax.polygon", child,
	      "Bad 'shading' specification '%s'!", shad);
            return false;
	  }
          poly3d->SetTextureType (shading);
	}
        break;
     case XMLTOKEN_UV:
        {
	  float u = child->GetAttributeValueAsFloat ("u");
	  float v = child->GetAttributeValueAsFloat ("v");
	  if (!init_gouraud_poly)
	  {
            poly3d->SetTextureType (POLYTXT_GOURAUD);
	    init_gouraud_poly = true;
	  }
	  if (!fs)
	  {
	    iPolyTexType* ptt = poly3d->GetPolyTexType ();
	    fs = SCF_QUERY_INTERFACE (ptt, iPolyTexFlat);
	    fs->Setup (poly3d);
	  }
	  if (num_uv >= poly3d->GetVertexCount ())
	  {
	    ReportError ("crystalspace.syntax.polygon", child,
	      "Too many <uv> statements in polygon!");
	    return false;
	  }
	  fs->SetUV (num_uv, u, v);
	  num_uv++;
	}
	break;
      case XMLTOKEN_UVA:
        {
	  float angle = child->GetAttributeValueAsFloat ("angle");
	  float scale = child->GetAttributeValueAsFloat ("scale");
	  float shift = child->GetAttributeValueAsFloat ("shift");
	  if (!init_gouraud_poly)
	  {
            poly3d->SetTextureType (POLYTXT_GOURAUD);
	    init_gouraud_poly = true;
	  }
	  if (!fs)
	  {
	    iPolyTexType* ptt = poly3d->GetPolyTexType ();
	    fs = SCF_QUERY_INTERFACE (ptt, iPolyTexFlat);
	    fs->Setup (poly3d);
	  }
	  if (num_uv >= poly3d->GetVertexCount ())
	  {
	    ReportError ("crystalspace.syntax.polygon", child,
	      "Too many <uva> statements in polygon!");
	    return false;
	  }
          float a = angle * TWO_PI / 360.;
          fs->SetUV (num_uv, cos (a) * scale + shift, sin (a) * scale + shift);
	  num_uv++;
	}
	break;
      case XMLTOKEN_COLOR:
        {
	  float r = child->GetAttributeValueAsFloat ("red");
	  float g = child->GetAttributeValueAsFloat ("green");
	  float b = child->GetAttributeValueAsFloat ("blue");
	  if (!init_gouraud_poly)
	  {
            poly3d->SetTextureType (POLYTXT_GOURAUD);
	    init_gouraud_poly = true;
	  }
	  if (!gs)
	  {
	    iPolyTexType* ptt = poly3d->GetPolyTexType ();
	    gs = SCF_QUERY_INTERFACE (ptt, iPolyTexGouraud);
	    gs->Setup (poly3d);
	  }
	  if (num_col >= poly3d->GetVertexCount ())
	  {
	    ReportError ("crystalspace.syntax.polygon", child,
	      "Too many <color> statements in polygon!");
	    return false;
	  }
	  gs->SetColor (num_col, csColor (r, g, b));
	  num_col++;
	}
	break;
      default:
        ReportBadToken (child);
        return false;
    }
  }

  if (poly3d->GetVertexCount () < 3)
  {
    ReportError ("crystalspace.syntax.polygon", node,
      "Polygon '%s' contains just %d vertices!",
      poly3d->QueryObject()->GetName(),
      poly3d->GetVertexCount ());
    return false;
  }

  if (set_colldet == 1)
    poly3d->GetFlags ().Set (CS_POLY_COLLDET);
  else if (set_colldet == -1)
    poly3d->GetFlags ().Reset (CS_POLY_COLLDET);

  if (texspec & CSTEX_UV)
  {
    poly3d->SetTextureSpace (
			     poly3d->GetVertex (tx_uv_i1), tx_uv1,
			     poly3d->GetVertex (tx_uv_i2), tx_uv2,
			     poly3d->GetVertex (tx_uv_i3), tx_uv3);
  }
  else if (texspec & CSTEX_V1)
  {
    if (texspec & CSTEX_V2)
    {
      if ((tx1-tx_orig) < SMALL_EPSILON)
      {
        ReportError ("crystalspace.syntax.polygon", node,
          "Bad texture specification!");
	return false;
      }
      else if ((tx2-tx_orig) < SMALL_EPSILON)
      {
        ReportError ("crystalspace.syntax.polygon", node,
          "Bad texture specification!");
	return false;
      }
      else poly3d->SetTextureSpace (tx_orig, tx1, tx_len.y, tx2, tx_len.z);
    }
    else
    {
      if ((tx1-tx_orig) < SMALL_EPSILON)
      {
        ReportError ("crystalspace.syntax.polygon", node,
          "Bad texture specification!");
	return false;
      }
      else poly3d->SetTextureSpace (tx_orig, tx1, tx_len.x);
    }
  }
  else if (plane_name[0])
  {
    iPolyTxtPlane* pl = te->FindPolyTxtPlane (plane_name);
    if (!pl)
    {
      ReportError ("crystalspace.syntax.polygon", node,
        "Can't find plane '%s' for polygon '%s'",
      	plane_name, poly3d->QueryObject ()->GetName ());
      return false;
    }
    poly3d->SetTextureSpace (pl);
  }
  else if (tx_len.x)
  {
    // If a length is given (with 'LEN') we will first see if the polygon
    // is coplanar with the X, Y, or Z plane. In that case we will use
    // a standard plane. Otherwise we will just create a plane specific
    // for this case given the first two vertices.
    bool same_x = true, same_y = true, same_z = true;
    const csVector3& v = poly3d->GetVertex (0);
    for (int i = 1 ; i < poly3d->GetVertexCount () ; i++)
    {
      const csVector3& v2 = poly3d->GetVertex (i);
      if (same_x && ABS (v.x-v2.x) >= SMALL_EPSILON) same_x = false;
      if (same_y && ABS (v.y-v2.y) >= SMALL_EPSILON) same_y = false;
      if (same_z && ABS (v.z-v2.z) >= SMALL_EPSILON) same_z = false;
    }
    if (same_x)
    {
      char buf[200];
      sprintf (buf, "__X_%g,%g__", v.x, tx_len.x);
      csRef<iPolyTxtPlane> pl (te->FindPolyTxtPlane (buf));
      if (!pl)
      {
        pl = te->CreatePolyTxtPlane ();
        pl->QueryObject()->SetName (buf);
        pl->SetTextureSpace (csVector3 (v.x, 0, 0), csVector3 (v.x, 0, 1),
			     tx_len.x, csVector3 (v.x, 1, 0), tx_len.x);
      }
      poly3d->SetTextureSpace (pl);
    }
    else if (same_y)
    {
      char buf[200];
      sprintf (buf, "__Y_%g,%g__", v.y, tx_len.x);
      csRef<iPolyTxtPlane> pl (te->FindPolyTxtPlane (buf));
      if (!pl)
      {
        pl = te->CreatePolyTxtPlane ();
        pl->QueryObject()->SetName (buf);
        pl->SetTextureSpace (csVector3 (0, v.y, 0), csVector3 (1, v.y, 0),
			     tx_len.x, csVector3 (0, v.y, 1), tx_len.x);
      }
      poly3d->SetTextureSpace (pl);
    }
    else if (same_z)
    {
      char buf[200];
      sprintf (buf, "__Z_%g,%g__", v.z, tx_len.x);
      csRef<iPolyTxtPlane> pl (te->FindPolyTxtPlane (buf));
      if (!pl)
      {
        pl = te->CreatePolyTxtPlane ();
        pl->QueryObject()->SetName (buf);
        pl->SetTextureSpace (csVector3 (0, 0, v.z), csVector3 (1, 0, v.z),
			     tx_len.x, csVector3 (0, 1, v.z), tx_len.x);
      }
      poly3d->SetTextureSpace (pl);
    }
    else
      poly3d->SetTextureSpace (poly3d->GetVertex (0), poly3d->GetVertex (1),
			       tx_len.x);
  }
  else
    poly3d->SetTextureSpace (tx_matrix, tx_vector);

  if (texspec & CSTEX_UV_SHIFT)
  {
    iPolyTexType* ptt = poly3d->GetPolyTexType ();
    csRef<iPolyTexLightMap> plm (SCF_QUERY_INTERFACE (ptt, iPolyTexLightMap));
    if (plm)
    {
      plm->GetPolyTxtPlane ()->GetTextureSpace (tx_matrix, tx_vector);
      // T = Mot * (O - Vot)
      // T = Mot * (O - Vot) + Vuv      ; Add shift Vuv to final texture map
      // T = Mot * (O - Vot) + Mot * Mot-1 * Vuv
      // T = Mot * (O - Vot + Mot-1 * Vuv)
      csVector3 shift (uv_shift.x, uv_shift.y, 0);
      tx_vector -= tx_matrix.GetInverse () * shift;
      poly3d->SetTextureSpace (tx_matrix, tx_vector);
    }
  }

  if (do_mirror)
    poly3d->GetPortal ()->SetWarp (csTransform::GetReflect (
    	poly3d->GetWorldPlane () ));

  OptimizePolygon (poly3d);

  return true;
}








*/

csClothFactoryLoader::csClothFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
};

csClothMeshLoader::csClothMeshLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
};

csClothFactoryLoader::~csClothFactoryLoader ()
{
};

csClothMeshLoader::~csClothMeshLoader ()
{
};

bool csClothFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csClothFactoryLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("default", XMLTOKEN_DEFAULT);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("texmap", XMLTOKEN_TEXMAP);	
  //xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("numtri", XMLTOKEN_NUMTRI);
  xmltokens.Register ("numvt", XMLTOKEN_NUMVT);
  xmltokens.Register ("v", XMLTOKEN_V);
  xmltokens.Register ("p", XMLTOKEN_P);
  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("autonormals", XMLTOKEN_AUTONORMALS);
  xmltokens.Register ("n", XMLTOKEN_N);
  
  return true;
};


bool csClothMeshLoader::Initialize (iObjectRegistry* object_reg)
{
  csClothMeshLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  //xmltokens.Register ("box", XMLTOKEN_BOX);
  //xmltokens.Register ("move" , XMLTOKEN_MOVE );	
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  //xmltokens.Register ("numtri", XMLTOKEN_NUMTRI);
  //xmltokens.Register ("numvt", XMLTOKEN_NUMVT);
  //xmltokens.Register ("v", XMLTOKEN_V);
  //xmltokens.Register ("p", XMLTOKEN_T);
  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("autonormals", XMLTOKEN_AUTONORMALS);
  xmltokens.Register ("n", XMLTOKEN_N);
  return true;
};



csPtr<iBase> csClothMeshLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iClothMeshState> meshstate;
  csRef<iMeshWrapper> sprite;
	
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_MANUALCOLORS:
	{
	  bool r;
	  if (!synldr->ParseBool (child, r, true))
	    return NULL;
	  meshstate->SetManualColors (r);
	}
	break;
	  case XMLTOKEN_LIGHTING:
	{
	  bool r;
	  if (!synldr->ParseBool (child, r, true))
	    return NULL;
	  meshstate->SetLighting (r);
	}
	break;
      case XMLTOKEN_COLOR:
	{
	  csColor col;
	  if (!synldr->ParseColor (child, col))
	    return NULL;
	  meshstate->SetColor (col);
	}
	break;
      case XMLTOKEN_FACTORY:
	{
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
	  if (!fact)
	  {
      	    synldr->ReportError (
		"crystalspace.clothmeshloader.parse.unknownfactory",
		child, "Couldn't find factory '%s'!", factname);
	    return NULL;
	  }
	  mesh      = fact->GetMeshObjectFactory ()->NewInstance ();
      meshstate = SCF_QUERY_INTERFACE (mesh, iClothMeshState);
	}
	break;
      case XMLTOKEN_MATERIAL:
	{
	  const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	  if (!mat)
	  {
      	    synldr->ReportError (
		"crystalspace.clothmeshloader.parse.unknownmaterial",
		child, "Couldn't find material '%s'!", matname);
            return NULL;
	  }
	  meshstate->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_MIXMODE:
        {
	  uint mm;
	  if (!synldr->ParseMixmode (child, mm))
	    return NULL;
          meshstate->SetMixMode (mm);
	}
	break;
      default:
        synldr->ReportBadToken (child);
	return NULL;
    }
  }

  // Incref to avoid smart pointer from releasing.
  if (mesh) mesh->IncRef ();
  return csPtr<iBase> (mesh);
}





csPtr<iBase> csClothFactoryLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase* /* context */)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,"crystalspace.mesh.object.cloth", iMeshObjectType);
  if (!type)
  {  //try with CS_LOAD_PLUGIN
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.cloth",
    	iMeshObjectType);
  }
  if (!type)
  {  //report error
    synldr->ReportError (
		"crystalspace.clothfactoryloader.setup.objecttype",
		node, "Could not load the cloth mesh object plugin!");
    return NULL;
  }
  
  csRef<iMeshObjectFactory> fact;
  csRef<iClothFactoryState> state;
  csRef<iMeshObject>        mesh;
  csRef<iClothMeshState>    meshstate;
  
  fact = type->NewFactory ();
  state = SCF_QUERY_INTERFACE (fact, iClothFactoryState);
  
  csVector3*  VerticeBuffer   ;
  csVector2*  TexelBuffer     ;
  csColor*    ColorBuffer     ;  
  csTriangle* TriangleBuffer  ; 
  
  
  type->DecRef ();

  int num_tri = 0;
  int num_nor = 0;
  int num_col = 0;
  uint        VerticeOffset   = 0;
  uint        TriangleOffset  = 0;
  bool auto_normals = false;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  
  csRef<iDocumentNode>              nose;
  csRef<iDocumentNodeIterator>      peeker;
  csRef<iDocumentNode>              atr;
  csRef<iDocumentNodeIterator> itat;
  csRef<iDocumentAttribute>    atr2;
  csRef<iDocumentAttributeIterator> itat2;
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value      = child->GetValue ();
    csStringID id          = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_MATERIAL:
	{ // apply material to factory
	  const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	  if (!mat)
	  {
      	    synldr->ReportError (
		"crystalspace.clothfactoryloader.parse.unknownmaterial",
		child, "Couldn't find material '%s'!", matname);
            return NULL;
	  }
	  state->SetMaterialWrapper (mat);
	}
	break;
	    case XMLTOKEN_DEFAULT:
        { //built a default fabric
	  	  state->GenerateFabric (40 , 40);
	     }
        break;
      case XMLTOKEN_AUTONORMALS:
	  {  //flag autonormals to compute after finish parsing
        if (!synldr->ParseBool (child, auto_normals, true))
	  return NULL;
	}
	break;
      case XMLTOKEN_NUMTRI:
	  {  //set number of triangles
		  state->SetTriangleCount (child->GetContentsValueAsInt ());
		  TriangleBuffer = state->GetTriangles();		  
	  }
	break;
      case XMLTOKEN_NUMVT:
	  {  //set number of vertices
		  printf(" OOPS! %u \n", child->GetContentsValueAsInt() );
		  state->SetVertexCount (child->GetContentsValueAsInt ());
		  VerticeBuffer = state->GetVertices();
	      TexelBuffer   = state->GetTexels();
		  ColorBuffer   = state->GetColors();
	  }
	break;
      case XMLTOKEN_P:
  	{   // parse polygons
		peeker = child->GetNodes();
		printf(" actual: %s ", child->GetValue() );
		//itat   = child->GetAttributes();
		//while (itat->HasNext() ) 
			//{ atr = itat->Next();
			  //printf(" attributes : %s = %s \n ",atr->GetName() , atr->GetValue() ); };
		uint V_nodes = 0;
    while( peeker->HasNext() ) 
	{
		nose = peeker->Next();
		printf(" \n  this node: %s = %s ", nose->GetValue() ,nose->GetContentsValue() ); 
		value = nose->GetValue();
		csStringID polygonToken = xmltokens.Request ( value );
		switch (polygonToken) 
		{
			case XMLTOKEN_V:
			{
				switch(V_nodes)
				{
					case 0:
					{
						TriangleBuffer[ TriangleOffset ].a = nose->GetContentsValueAsInt ();
						V_nodes++;
					}
						break;
					case 1:
					{
						TriangleBuffer[ TriangleOffset ].b = nose->GetContentsValueAsInt ();
						V_nodes++;
					}
						break;
					case 2:
					{
						TriangleBuffer[ TriangleOffset ].c = nose->GetContentsValueAsInt ();
						TriangleOffset++ ;
						V_nodes++;
					}
						break;
					default:
					TriangleOffset++ ;
					TriangleBuffer[ TriangleOffset ].a = TriangleBuffer[ TriangleOffset - 1 ].a;
					TriangleBuffer[ TriangleOffset ].b = TriangleBuffer[ TriangleOffset - 1 ].c;				
					TriangleBuffer[ TriangleOffset ].c = nose->GetContentsValueAsInt();
					V_nodes++;
				};
			}
			break;
			case XMLTOKEN_MATERIAL:
			{	//do nothing, cloths support one material			
			}
			break;
			case XMLTOKEN_TEXMAP:
			{
				itat    = nose->GetNodes();
		        while (itat->HasNext() ) 
				{
					atr = itat->Next();
					printf(" child : %s = %s ",atr->GetValue(), atr->GetContentsValue() ); 
					if ( !strcasecmp("uv",atr->GetValue()) )
					{
						
					  switch( atr->GetAttributeValueAsInt("idx") )
					  {
						  case 0:
						  {
						TexelBuffer
						 [ TriangleBuffer[ TriangleOffset - (V_nodes - 2) ].a  ].Set(
							  atr->GetAttributeValueAsFloat("u") , atr->GetAttributeValueAsFloat("v") );
						  }
						  break;
						  case 1:
						  {
						TexelBuffer
						 [ TriangleBuffer[ TriangleOffset - (V_nodes - 2) ].b  ].Set(
							  atr->GetAttributeValueAsFloat("u") , atr->GetAttributeValueAsFloat("v") );	  
						  }
						  break;
						  case 2:
						  {
						TexelBuffer
						 [ TriangleBuffer[ TriangleOffset - (V_nodes - 2) ].c  ].Set(
							  atr->GetAttributeValueAsFloat("u") , atr->GetAttributeValueAsFloat("v") );
						  }
						  break;
						  default:
						  synldr->ReportError (
						  "crystalspace.clothfactoryloader.parse.frame.badformat",
						  atr,
						  "Too many idx indices!!");
						  return NULL;							  
					  };	//end idx switch 					
				 };  //end if uv 
			  };
			}        //end CASE XMLTOKEN_TEXMAP 
			break;
			default:
				synldr->ReportError (
		    "crystalspace.clothfactoryloader.parse.polygon.badformat",
		    child, "token '%s' doesnt exist", value);
		};
		
		   
		};
				printf(" \n EXIT_P_BLOCK \n");
	}
        break;
      case XMLTOKEN_N:
    {  //set a normal
	  csVector3* no = state->GetNormals ();
	  if (num_nor >= state->GetVertexCount ())
	  {
	    synldr->ReportError (
		    "crystalspace.clothfactoryloader.parse.frame.badformat",
		    child, "Too many normals for a cloth mesh factory!");
	    return NULL;
	  }
	  float x, y, z;
	  x = child->GetAttributeValueAsFloat ("x");
	  y = child->GetAttributeValueAsFloat ("y");
	  z = child->GetAttributeValueAsFloat ("z");
	  no[num_nor].Set (x, y, z);
	  num_nor++;
        }
        break;
      case XMLTOKEN_COLOR:
    {  //set a color
	  csColor* co = state->GetColors ();
	  if (num_col >= state->GetVertexCount ())
	  {
	    synldr->ReportError (
		    "crystalspace.clothfactoryloader.parse.frame.badformat",
		    child, "Too many colors for a cloth mesh factory!");
	    return NULL;
	  }
	  float r, g, b;
	  r = child->GetAttributeValueAsFloat ("red");
	  g = child->GetAttributeValueAsFloat ("green");
	  b = child->GetAttributeValueAsFloat ("blue");
	  co[num_col].Set (r, g, b);
	  num_col++;
	}
	break;
      case XMLTOKEN_V:
    {  //set a vertex
			peeker = child->GetNodes();
		printf(" actual: %s  \n", child->GetValue() );
		//printf(" next  : %s  \n", peeker->Next()->GetNodes()->Next()->GetValue() );
	  if (VerticeOffset >= state->GetVertexCount ())
	  {
		  printf(" %u %u \n", VerticeOffset ,state->GetVertexCount());
	    synldr->ReportError (
		    "crystalspace.clothfactoryloader.parse.frame.badformat",
		    child, "Too many uv texels for a cloth mesh factory!");
	    return NULL;
	  }
	  float x, y, z, u, v;
	  x = child->GetAttributeValueAsFloat ("x");
	  y = child->GetAttributeValueAsFloat ("y");
	  z = child->GetAttributeValueAsFloat ("z");
	  u = child->GetAttributeValueAsFloat ("u");
	  v = child->GetAttributeValueAsFloat ("v");
	  printf(" print triangle with %f , %f , %f \n",x,y,u);
	  VerticeBuffer[VerticeOffset].Set (x, y, z);
	  ColorBuffer  [VerticeOffset].Set (1.0f, 1.0f, 1.0f);
	  TexelBuffer  [VerticeOffset].Set (u, v);
	  VerticeOffset++;
	}
        break;
      default:
	synldr->ReportBadToken (child);
	return NULL;
    }
  }

  if (auto_normals)
    state->CalculateNormals ();

  // Incref to prevent smart pointer from releasing object.
  if (fact) fact->IncRef ();
  return csPtr<iBase> (fact);
  //csRef<iMeshObject> mesh (fact->NewInstance());
  //if (mesh) mesh->IncRef();
//	  return csPtr<iBase> (mesh);
  
}
//---------------------------------------------------------------------------


csClothFactorySaver::csClothFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
};

csClothFactorySaver::~csClothFactorySaver ()
{
};

bool csClothFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csClothFactorySaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  
};

void csClothFactorySaver::WriteDown (iBase *obj, iFile *file)
{
	obj?file:NULL;
};