/*
    Copyright (C) 2008 by Frank Richter
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.
  
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"

#include "glshader_cg.h"
#include "glshader_cgcommon.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{

// 
// PLANE CLIPPING
//

/*
   Plane clipping sucks a bit since there is not one good way to do it.
   
   - ARB_vertex_program: user clip planes not supported unless position
                         invariance is used. User clip planes for general
                         transformations were left to future extensions.
                         None was ever made.
   - NVIDIA VPs: NVidia VPs have a special semantic for "clip distances" for
                 vertex programs. This is really nice and we want that.
                 But it obviously doesn't work on non-NVIDIA hardware.
   - FP clip() [ie fragment kill]: works everywhere, but is not very efficient
                                   as it needs a per-fragment check and
                                   prevents Z buffer optimizations such as
                                   hierarchical Z to kick in.
   
   So clip planes may actually be specified in one of 3 ways:
   - NV VP: use clip plane semantic
   - ATI, others with position invariance: use glClipPlane()
   - Other: fragment program clip
   
   Unfortunately, to accomodate all 3 ways, setting up a clip plane is
   cumbersome. Luckily, most of this is encapsulated by the Cg plugin.
   To define a clip plane:
    - Use a <clip ... > tag for each plane
    - Invoke APPLY_CLIPPING_VP or APPLY_CLIPPING_VP_POSINV in the VP
    - Invoke APPLY_CLIPPING_FP or APPLY_CLIPPING_FP_POSINV in the FP
 */

void csShaderGLCGCommon::ClipsToVmap ()
{
  size_t c;
  for (c = 0; c < clips.GetSize(); c++)
  {
    const Clip& clip = clips[c];
    
    csString destnamePlane;
    destnamePlane.Format ("_clipPlanes.plane%zu", c);
    
    VariableMapEntry vme (CS::InvalidShaderVarStringID, destnamePlane);
    if (clip.plane.IsConstant ())
    {
      vme.mappingParam = clip.plane;
      vme.name = clip.plane.name;
    }
    else
    {
      CS::ShaderVarStringID svName ((uint)svClipPlane - (uint)c);
      clipPlane[c].AttachNew (new csShaderVariable (svName));
      clipPlane[c]->SetValue (0.0f);
      vme.name = svName;
      vme.mappingParam.name = svName;
    }
    ShaderParameter* sparam = shaderPlug->paramAlloc.Alloc();
    sparam->assumeConstant = false;
    vme.userVal = reinterpret_cast<intptr_t> (sparam);
    variablemap.Push (vme);
  }
  
  for (int i = 0; i < 2; i++)
  {
    CS::ShaderVarStringID svName (
      (i == 0) ? svClipPackedDist0 : svClipPackedDist1);
    clipPackedDists[i].AttachNew (new csShaderVariable (svName));
    clipPackedDists[i]->SetValue (0.0f);
  
    csString destnameDist;
    destnameDist.Format ("_clipPlanes.packedDists%d", i);
    
    VariableMapEntry vme (svName, destnameDist);
    ShaderParameter* sparam = shaderPlug->paramAlloc.Alloc();
    sparam->assumeConstant = false;
    vme.userVal = reinterpret_cast<intptr_t> (sparam);
    variablemap.Push (vme);
  }
}

#define COND_STYLE_NV     \
  "(" \
  "defined(VERT_PROFILE_VP40) " \
  "|| defined(VERT_PROFILE_GP4VP)" \
  ")"
#define COND_STYLE_ATI    "defined(VENDOR_ATI)"

static const char clipVaryingMacro[] =
  "#ifdef PROGRAM_TYPE_VERTEX\n"
  "  struct _ClipPlanes\n"
  "  {\n"
  "    uniform float4 plane0;\n"
  "    uniform float4 plane1;\n"
  "    uniform float4 plane2;\n"
  "    uniform float4 plane3;\n"
  "    uniform float4 plane4;\n"
  "    uniform float4 plane5;\n"
  "    uniform float4 packedDists0;\n"
  "    uniform float4 packedDists1;\n"
  "  };\n"
  "  _ClipPlanes _clipPlanes;\n"
  "\n"
  "  #if " COND_STYLE_NV "\n"
  "    #define _CLIP_OUTPUT(N)  out varying float _clip_ ## N : CLP ## N;\n"
  "    #define _CLIP_OUTPUT_UNUSED(N)  float _clip_ ## N;\n"
  "  #else\n"
  "    #define _CLIP_OUTPUT(N)\n"
  "    #define _CLIP_OUTPUT_UNUSED(N)\n"
  "    #ifndef PARAM__clip_out_packed_distances1_UNUSED\n"
  "      out varying float4 _clip_out_packed_distances1 : TEXCOORD7;\n"
  "    #endif\n"
  "    #ifndef PARAM__clip_out_packed_distances2_UNUSED\n"
  "      out varying float4 _clip_out_packed_distances2 : TEXCOORD6;\n"
  "    #endif\n"
  "  #endif\n"
  "#else\n"
  "  #if !(" COND_STYLE_NV ")\n"
  "    #ifndef PARAM__clip_out_packed_distances1_UNUSED\n"
  "      in varying float4 _clip_out_packed_distances1 : TEXCOORD7;\n"
  "    #else\n"
  "      float4 _clip_out_packed_distances1 = float4(0);\n"
  "    #endif\n"
  "    #ifndef PARAM__clip_out_packed_distances2_UNUSED\n"
  "      in varying float4 _clip_out_packed_distances2 : TEXCOORD6;\n"
  "    #else\n"
  "      float4 _clip_out_packed_distances2 = float4(0);\n"
  "    #endif\n"
  "  #endif\n"
  "#endif\n"
  "\n";

void csShaderGLCGCommon::OutputClipPreamble (csString& str)
{
  str += clipVaryingMacro;
}

static const char clipApplPreamble[] =
  "#ifdef PROGRAM_TYPE_VERTEX\n"
  "  #define _CLIP_COMPUTE_DIST(N, CLIP_POS) \\\n"
  "    float _clip_dist ## N = dot (_clipPlanes.plane ## N, CLIP_POS); \\\n"
  "    _clip_dist ## N -= (N < 4) \\\n"
  "	? _clipPlanes.packedDists0[N] \\\n"
  "	: _clipPlanes.packedDists1[N-4];\n"
  "  #define _CLIP_COMPUTE_DIST_CD(N, CLIP_POS, CLIP_DIST) \\\n"
  "    float _clip_dist ## N = dot (_clipPlanes.plane ## N, CLIP_POS); \\\n"
  "    _clip_dist ## N -= CLIP_DIST;\n"
  "  #define _CLIP_UNUSED(N) \\\n"
  "    float _clip_dist ## N = 0;\n"
  "#else\n"
  "#endif\n"
  "\n"
  ;

static const char clipApplPostamble[] =
  "#ifdef PROGRAM_TYPE_VERTEX\n"
  "  #if " COND_STYLE_NV "\n"
  "    #define APPLY_CLIPPING_VP(P_Eye, P_World, P_Object) \\\n"
  "      _CLIP_COMPUTE_DISTS(P_Eye, P_World, P_Object) \\\n"
  "      _clip_0 = _clip_dist0; \\\n"
  "      _clip_1 = _clip_dist1; \\\n"
  "      _clip_2 = _clip_dist2; \\\n"
  "      _clip_3 = _clip_dist3; \\\n"
  "      _clip_4 = _clip_dist4; \\\n"
  "      _clip_5 = _clip_dist5;\n"
  "    #define APPLY_CLIPPING_VP_POSINV(P_Eye, P_World, P_Object) APPLY_CLIPPING_VP(P_Eye, P_World, P_Object)\n"
  "  #else\n"
       // PosInv clipping always uses clip planes
  "    #define APPLY_CLIPPING_VP_POSINV(P_Eye, P_World, P_Object)\n"
  "    #if " COND_STYLE_ATI "\n"
	 // ATI: can always use clip planes
  "      #define APPLY_CLIPPING_VP(P_Eye, P_World, P_Object) APPLY_CLIPPING_VP_POSINV(P_Eye, P_World, P_Object)\n"
  "    #else\n"
  "      #ifndef PARAM__clip_out_packed_distances1_UNUSED\n"
  "        #define _CLIP_PACK_DISTS_1 \\\n"
  "          _clip_out_packed_distances1.x = _clip_dist0; \\\n"
  "          _clip_out_packed_distances1.y = _clip_dist1; \\\n"
  "          _clip_out_packed_distances1.z = _clip_dist2; \\\n"
  "          _clip_out_packed_distances1.w = _clip_dist3;\n"
  "      #else\n"
  "        #define _CLIP_PACK_DISTS_1\n"
  "      #endif\n"
  "      #ifndef PARAM__clip_out_packed_distances2_UNUSED\n"
  "        #define _CLIP_PACK_DISTS_2 \\\n"
  "          _clip_out_packed_distances2.x = _clip_dist4; \\\n"
  "          _clip_out_packed_distances2.y = _clip_dist5; \\\n"
  "          _clip_out_packed_distances2.z = 0; \\\n"
  "          _clip_out_packed_distances2.w = 0;\n"
  "      #else\n"
  "        #define _CLIP_PACK_DISTS_2\n"
  "      #endif\n"
  "      #define APPLY_CLIPPING_VP(P_Eye, P_World, P_Object) \\\n"
  "        _CLIP_COMPUTE_DISTS(P_Eye, P_World, P_Object) \\\n"
  "        _CLIP_PACK_DISTS_1 \\\n"
  "        _CLIP_PACK_DISTS_2\n"
  "    #endif\n"
  "  #endif\n"
  "#else\n"
  "  #define APPLY_CLIPPING_FP_POSINV\n"
  "  #if (" COND_STYLE_NV ")\n"
  "    #define APPLY_CLIPPING_FP\n"
  "  #else\n"
  "    #if " COND_STYLE_ATI "\n"
  "      #define APPLY_CLIPPING_FP APPLY_CLIPPING_FP_POSINV\n"
  "    #else\n"
  "      #define APPLY_CLIPPING_FP _CLIP_TO_DISTS\n"
  "    #endif\n"
  "  #endif\n"
  "#endif\n"
  "\n"
  ;

void csShaderGLCGCommon::WriteClipApplications (csString& str)
{
  str += clipApplPreamble;
  
  size_t c;
  csString strClipDistsVP;
  csString strClipDistsFP;
  csString clipOutStr;
  
  strClipDistsVP += "#define _CLIP_COMPUTE_DISTS(P_Eye, P_World, P_Object) ";
  for (c = 0; c < clips.GetSize(); c++)
  {
    const Clip& clip = clips[c];
    
    const char* posStr = 0;
    switch (clip.space)
    {
      case ShaderProgramPluginGL::ClipPlanes::Eye:    posStr = "P_Eye"; break;
      case ShaderProgramPluginGL::ClipPlanes::Object: posStr = "P_World"; break;
      case ShaderProgramPluginGL::ClipPlanes::World:  posStr = "P_Object"; break;
    }
    
    if (clip.distance.IsConstant())
    {
      csVector4 constDist;
      clip.distance.var->GetValue (constDist);
      strClipDistsVP.AppendFmt ("\\\n_CLIP_COMPUTE_DIST_CD(%zu, %s, %f) ", c,
	posStr, constDist[clip.distComp]);
    }
    else
    {
      strClipDistsVP.AppendFmt ("\\\n_CLIP_COMPUTE_DIST(%zu, %s) ", c,
	posStr);
    }
    
    clipOutStr.AppendFmt ("_CLIP_OUTPUT(%zu)\n", c);
  }
  for (; c < 6; c++)
  {
    strClipDistsVP.AppendFmt ("\\\n_CLIP_UNUSED(%zu) ", c);
    clipOutStr.AppendFmt ("_CLIP_OUTPUT_UNUSED(%zu)\n", c);
  }
  
  str << "#ifdef PROGRAM_TYPE_VERTEX\n";
  str << strClipDistsVP;
  str << "\n";
  str << clipOutStr;
  str << "#endif\n\n";
  
  strClipDistsFP += "#define _CLIP_TO_DISTS ";
  if (clips.GetSize() > 0)
    strClipDistsFP += "\\\nclip (_clip_out_packed_distances1); ";
  if (clips.GetSize() >= 4)
    strClipDistsFP += "\\\nclip (_clip_out_packed_distances2); ";
  
  str << "#ifdef PROGRAM_TYPE_FRAGMENT\n";
  str << strClipDistsFP;
  str << "\n#endif\n\n";

  str += clipApplPostamble;
}

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)
