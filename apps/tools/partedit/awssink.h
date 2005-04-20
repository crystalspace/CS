/*
    Copyright (C) 2004 by Andrew Mann

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

#ifndef __AWS_SINK_H__
#define __AWS_SINK_H__

#include "iaws/aws.h"
#include "iutil/string.h"

class csString;

typedef struct st_EmitterState {
    int particle_count,particle_max_age;
    bool lighting;
    bool rectangular_particles;
    float rect_w,rect_h;
    int reg_number;
    float reg_radius;
    bool using_bounding_box;
    float bbox_minx,bbox_miny,bbox_minz;
    float bbox_maxx,bbox_maxy,bbox_maxz;
    bool alpha_blend;
} EmitterState;

typedef struct st_Emitter3DState {
    csVector3 fixed_position;
    float fixed_weight;
    csVector3 box_min,box_max;
    float box_weight;
    csVector3 sphere_center;
    float sphere_min,sphere_max;
    float sphere_weight;
    csVector3 cone_origin;
    float cone_elevation,cone_azimuth,cone_aperture,cone_min,cone_max;
    float cone_weight;
    csVector3 line_start,line_end;
    float line_weight;
    csVector3 cylinder_start,cylinder_end;
    float cylinder_min,cylinder_max;
    float cylinder_weight;
    csVector3 spheretangent_center;
    float spheretangent_min,spheretangent_max;
    float spheretangent_weight;
    csVector3 cylindertangent_start,cylindertangent_end;
    float cylindertangent_min,cylindertangent_max;
    float cylindertangent_weight;
} Emitter3DState;

typedef struct st_AttractorState {
  Emitter3DState e3d_state;
  float force;
} AttractorState;

typedef struct st_FieldState {
  Emitter3DState e3d_state;
  bool active;
} FieldState;

typedef struct st_AgingMomentsState {

  int moment_count;

} AgingMomentsState;


typedef struct st_LoadSaveFileState {
  bool commandqueued;
  bool loading;
} LoadSaveFileState;

enum Sections {
  SECTION_GRAPHIC = 0,
  SECTION_EMITTER_STATE,
  SECTION_INIT_POSITION,
  SECTION_INIT_SPEED,
  SECTION_INIT_ACCELERATION,
  SECTION_FIELD_SPEED,
  SECTION_FIELD_ACCELERATION,
  SECTION_ATTRACTOR,
  SECTION_AGING_MOMENTS,
  SECTION_LOADSAVE,
  SECTION_FINAL_SECTION
};

enum Emitters {
  EMITTER_NONE = 0,
  EMITTER_POINT,
  EMITTER_LINE,
  EMITTER_BOX,
  EMITTER_SPHERE,
  EMITTER_CYLINDER,
  EMITTER_CONE,
  EMITTER_SPHERETANGENT,
  EMITTER_CYLINDERTANGENT,
  EMITTER_MIX
};

#define SECTION_COUNT SECTION_FINAL_SECTION

class PartEditSink
{

  iAws     *wmgr;
  csRef<iAwsSink> sink;
  static PartEditSink *asink;
  csRef<iVFS> vfs;

  struct st_FreeScrollData {
    iAwsComponent *iawscomponent_FSWindow;
    iAwsComponent *iawscomponent_FSLabel;
    iAwsComponent *iawscomponent_FSTextBox;
    iAwsComponent *iawscomponent_FSScrollBar;

    iAwsComponent *iawscomponent_AssociatedTextBox;
    bool floatval; // false if integer
    bool *invalidate_pointer;
    intptr_t value_pointer;
  } FreeScrollData;

  
  struct st_GraphicSelectionData {
    ///  The iAwsComponent interface to the to the window for selecting the graphics file.
    iAwsComponent *iawscomponent_GraphicSelection;
    ///  The iAwsComponent interface to the to the window for selecting the graphics file.
    iAwsComponent *iawscomponent_GraphicFileList;
    /// The iAwsComponent interface to the to the text box containing the current graphics filter
    iAwsComponent *iawscomponent_GraphicFilter;
    csRef<scfString> filter;
    csRef<scfString> currentdirectory;
    csRef<iString> currentfilepath;
  } GraphicSelectionData;


  struct st_EmitterStateData {
    ///  The iAwsComponent interface to the to the window for setting emitter state parameters.
    iAwsComponent *iawscomponent_EmitterState;
    iAwsComponent *iawscomponent_ParticleCount;
    iAwsComponent *iawscomponent_ParticleMaxAge;
    iAwsComponent *iawscomponent_Lighting;
    iAwsComponent *iawscomponent_AlphaBlend;
    iAwsComponent *iawscomponent_RectParticlesRadio;
    iAwsComponent *iawscomponent_RegParticlesRadio;
    iAwsComponent *iawscomponent_RectParticlesWidth;
    iAwsComponent *iawscomponent_RectParticlesHeight;
    iAwsComponent *iawscomponent_RegParticlesNumber;
    iAwsComponent *iawscomponent_RegParticlesRadius;
    iAwsComponent *iawscomponent_UseBoundingBox;
    iAwsComponent *iawscomponent_BBoxMinX;
    iAwsComponent *iawscomponent_BBoxMinY;
    iAwsComponent *iawscomponent_BBoxMinZ;
    iAwsComponent *iawscomponent_BBoxMaxX;
    iAwsComponent *iawscomponent_BBoxMaxY;
    iAwsComponent *iawscomponent_BBoxMaxZ;
    EmitterState state;
    bool settings_changed;
  } EmitterStateData;



  struct st_InitialPositionData {
    iAwsComponent *iawscomponent_InitialPosition;
    iAwsComponent *iawscomponent_IPFPX;
    iAwsComponent *iawscomponent_IPFPY;
    iAwsComponent *iawscomponent_IPFPZ;
    iAwsComponent *iawscomponent_IPFWeight;
    iAwsComponent *iawscomponent_IPLSX;
    iAwsComponent *iawscomponent_IPLSY;
    iAwsComponent *iawscomponent_IPLSZ;
    iAwsComponent *iawscomponent_IPLEX;
    iAwsComponent *iawscomponent_IPLEY;
    iAwsComponent *iawscomponent_IPLEZ;
    iAwsComponent *iawscomponent_IPLWeight;
    iAwsComponent *iawscomponent_IPBMX;
    iAwsComponent *iawscomponent_IPBMY;
    iAwsComponent *iawscomponent_IPBMZ;
    iAwsComponent *iawscomponent_IPBXX;
    iAwsComponent *iawscomponent_IPBXY;
    iAwsComponent *iawscomponent_IPBXZ;
    iAwsComponent *iawscomponent_IPBWeight;
    iAwsComponent *iawscomponent_IPSCX;
    iAwsComponent *iawscomponent_IPSCY;
    iAwsComponent *iawscomponent_IPSCZ;
    iAwsComponent *iawscomponent_IPSMin;
    iAwsComponent *iawscomponent_IPSMax;
    iAwsComponent *iawscomponent_IPSWeight;
    iAwsComponent *iawscomponent_IPCNOX;
    iAwsComponent *iawscomponent_IPCNOY;
    iAwsComponent *iawscomponent_IPCNOZ;
    iAwsComponent *iawscomponent_IPCNElevation;
    iAwsComponent *iawscomponent_IPCNAzimuth;
    iAwsComponent *iawscomponent_IPCNAperture;
    iAwsComponent *iawscomponent_IPCNMin;
    iAwsComponent *iawscomponent_IPCNMax;
    iAwsComponent *iawscomponent_IPCNWeight;
    iAwsComponent *iawscomponent_IPCYSX;
    iAwsComponent *iawscomponent_IPCYSY;
    iAwsComponent *iawscomponent_IPCYSZ;
    iAwsComponent *iawscomponent_IPCYEX;
    iAwsComponent *iawscomponent_IPCYEY;
    iAwsComponent *iawscomponent_IPCYEZ;
    iAwsComponent *iawscomponent_IPCYMin;
    iAwsComponent *iawscomponent_IPCYMax;
    iAwsComponent *iawscomponent_IPCYWeight;
    iAwsComponent *iawscomponent_IPSTCX;
    iAwsComponent *iawscomponent_IPSTCY;
    iAwsComponent *iawscomponent_IPSTCZ;
    iAwsComponent *iawscomponent_IPSTMin;
    iAwsComponent *iawscomponent_IPSTMax;
    iAwsComponent *iawscomponent_IPSTWeight;
    iAwsComponent *iawscomponent_IPCYTSX;
    iAwsComponent *iawscomponent_IPCYTSY;
    iAwsComponent *iawscomponent_IPCYTSZ;
    iAwsComponent *iawscomponent_IPCYTEX;
    iAwsComponent *iawscomponent_IPCYTEY;
    iAwsComponent *iawscomponent_IPCYTEZ;
    iAwsComponent *iawscomponent_IPCYTMin;
    iAwsComponent *iawscomponent_IPCYTMax;
    iAwsComponent *iawscomponent_IPCYTWeight;
    Emitter3DState state;
    bool settings_changed;
  } InitialPositionData;

  struct st_InitialSpeedData {
    iAwsComponent *iawscomponent_InitialSpeed;
    iAwsComponent *iawscomponent_ISFPX;
    iAwsComponent *iawscomponent_ISFPY;
    iAwsComponent *iawscomponent_ISFPZ;
    iAwsComponent *iawscomponent_ISFWeight;
    iAwsComponent *iawscomponent_ISLSX;
    iAwsComponent *iawscomponent_ISLSY;
    iAwsComponent *iawscomponent_ISLSZ;
    iAwsComponent *iawscomponent_ISLEX;
    iAwsComponent *iawscomponent_ISLEY;
    iAwsComponent *iawscomponent_ISLEZ;
    iAwsComponent *iawscomponent_ISLWeight;
    iAwsComponent *iawscomponent_ISBMX;
    iAwsComponent *iawscomponent_ISBMY;
    iAwsComponent *iawscomponent_ISBMZ;
    iAwsComponent *iawscomponent_ISBXX;
    iAwsComponent *iawscomponent_ISBXY;
    iAwsComponent *iawscomponent_ISBXZ;
    iAwsComponent *iawscomponent_ISBWeight;
    iAwsComponent *iawscomponent_ISSCX;
    iAwsComponent *iawscomponent_ISSCY;
    iAwsComponent *iawscomponent_ISSCZ;
    iAwsComponent *iawscomponent_ISSMin;
    iAwsComponent *iawscomponent_ISSMax;
    iAwsComponent *iawscomponent_ISSWeight;
    iAwsComponent *iawscomponent_ISCNOX;
    iAwsComponent *iawscomponent_ISCNOY;
    iAwsComponent *iawscomponent_ISCNOZ;
    iAwsComponent *iawscomponent_ISCNElevation;
    iAwsComponent *iawscomponent_ISCNAzimuth;
    iAwsComponent *iawscomponent_ISCNAperture;
    iAwsComponent *iawscomponent_ISCNMin;
    iAwsComponent *iawscomponent_ISCNMax;
    iAwsComponent *iawscomponent_ISCNWeight;
    iAwsComponent *iawscomponent_ISCYSX;
    iAwsComponent *iawscomponent_ISCYSY;
    iAwsComponent *iawscomponent_ISCYSZ;
    iAwsComponent *iawscomponent_ISCYEX;
    iAwsComponent *iawscomponent_ISCYEY;
    iAwsComponent *iawscomponent_ISCYEZ;
    iAwsComponent *iawscomponent_ISCYMin;
    iAwsComponent *iawscomponent_ISCYMax;
    iAwsComponent *iawscomponent_ISCYWeight;
    iAwsComponent *iawscomponent_ISSTCX;
    iAwsComponent *iawscomponent_ISSTCY;
    iAwsComponent *iawscomponent_ISSTCZ;
    iAwsComponent *iawscomponent_ISSTMin;
    iAwsComponent *iawscomponent_ISSTMax;
    iAwsComponent *iawscomponent_ISSTWeight;
    iAwsComponent *iawscomponent_ISCYTSX;
    iAwsComponent *iawscomponent_ISCYTSY;
    iAwsComponent *iawscomponent_ISCYTSZ;
    iAwsComponent *iawscomponent_ISCYTEX;
    iAwsComponent *iawscomponent_ISCYTEY;
    iAwsComponent *iawscomponent_ISCYTEZ;
    iAwsComponent *iawscomponent_ISCYTMin;
    iAwsComponent *iawscomponent_ISCYTMax;
    iAwsComponent *iawscomponent_ISCYTWeight;
    Emitter3DState state;
    bool settings_changed;

  } InitialSpeedData;

  struct st_InitialAccelerationData {
    iAwsComponent *iawscomponent_InitialAcceleration;
    iAwsComponent *iawscomponent_IAFPX;
    iAwsComponent *iawscomponent_IAFPY;
    iAwsComponent *iawscomponent_IAFPZ;
    iAwsComponent *iawscomponent_IAFWeight;
    iAwsComponent *iawscomponent_IALSX;
    iAwsComponent *iawscomponent_IALSY;
    iAwsComponent *iawscomponent_IALSZ;
    iAwsComponent *iawscomponent_IALEX;
    iAwsComponent *iawscomponent_IALEY;
    iAwsComponent *iawscomponent_IALEZ;
    iAwsComponent *iawscomponent_IALWeight;
    iAwsComponent *iawscomponent_IABMX;
    iAwsComponent *iawscomponent_IABMY;
    iAwsComponent *iawscomponent_IABMZ;
    iAwsComponent *iawscomponent_IABXX;
    iAwsComponent *iawscomponent_IABXY;
    iAwsComponent *iawscomponent_IABXZ;
    iAwsComponent *iawscomponent_IABWeight;
    iAwsComponent *iawscomponent_IASCX;
    iAwsComponent *iawscomponent_IASCY;
    iAwsComponent *iawscomponent_IASCZ;
    iAwsComponent *iawscomponent_IASMin;
    iAwsComponent *iawscomponent_IASMax;
    iAwsComponent *iawscomponent_IASWeight;
    iAwsComponent *iawscomponent_IACNOX;
    iAwsComponent *iawscomponent_IACNOY;
    iAwsComponent *iawscomponent_IACNOZ;
    iAwsComponent *iawscomponent_IACNElevation;
    iAwsComponent *iawscomponent_IACNAzimuth;
    iAwsComponent *iawscomponent_IACNAperture;
    iAwsComponent *iawscomponent_IACNMin;
    iAwsComponent *iawscomponent_IACNMax;
    iAwsComponent *iawscomponent_IACNWeight;
    iAwsComponent *iawscomponent_IACYSX;
    iAwsComponent *iawscomponent_IACYSY;
    iAwsComponent *iawscomponent_IACYSZ;
    iAwsComponent *iawscomponent_IACYEX;
    iAwsComponent *iawscomponent_IACYEY;
    iAwsComponent *iawscomponent_IACYEZ;
    iAwsComponent *iawscomponent_IACYMin;
    iAwsComponent *iawscomponent_IACYMax;
    iAwsComponent *iawscomponent_IACYWeight;
    iAwsComponent *iawscomponent_IASTCX;
    iAwsComponent *iawscomponent_IASTCY;
    iAwsComponent *iawscomponent_IASTCZ;
    iAwsComponent *iawscomponent_IASTMin;
    iAwsComponent *iawscomponent_IASTMax;
    iAwsComponent *iawscomponent_IASTWeight;
    iAwsComponent *iawscomponent_IACYTSX;
    iAwsComponent *iawscomponent_IACYTSY;
    iAwsComponent *iawscomponent_IACYTSZ;
    iAwsComponent *iawscomponent_IACYTEX;
    iAwsComponent *iawscomponent_IACYTEY;
    iAwsComponent *iawscomponent_IACYTEZ;
    iAwsComponent *iawscomponent_IACYTMin;
    iAwsComponent *iawscomponent_IACYTMax;
    iAwsComponent *iawscomponent_IACYTWeight;
    Emitter3DState state;
    bool settings_changed;
  } InitialAccelerationData;

  struct st_FieldSpeedData {
    iAwsComponent *iawscomponent_FieldSpeed;
    iAwsComponent *iawscomponent_FSActive;
    iAwsComponent *iawscomponent_FSFPX;
    iAwsComponent *iawscomponent_FSFPY;
    iAwsComponent *iawscomponent_FSFPZ;
    iAwsComponent *iawscomponent_FSFWeight;
    iAwsComponent *iawscomponent_FSLSX;
    iAwsComponent *iawscomponent_FSLSY;
    iAwsComponent *iawscomponent_FSLSZ;
    iAwsComponent *iawscomponent_FSLEX;
    iAwsComponent *iawscomponent_FSLEY;
    iAwsComponent *iawscomponent_FSLEZ;
    iAwsComponent *iawscomponent_FSLWeight;
    iAwsComponent *iawscomponent_FSBMX;
    iAwsComponent *iawscomponent_FSBMY;
    iAwsComponent *iawscomponent_FSBMZ;
    iAwsComponent *iawscomponent_FSBXX;
    iAwsComponent *iawscomponent_FSBXY;
    iAwsComponent *iawscomponent_FSBXZ;
    iAwsComponent *iawscomponent_FSBWeight;
    iAwsComponent *iawscomponent_FSSCX;
    iAwsComponent *iawscomponent_FSSCY;
    iAwsComponent *iawscomponent_FSSCZ;
    iAwsComponent *iawscomponent_FSSMin;
    iAwsComponent *iawscomponent_FSSMax;
    iAwsComponent *iawscomponent_FSSWeight;
    iAwsComponent *iawscomponent_FSCNOX;
    iAwsComponent *iawscomponent_FSCNOY;
    iAwsComponent *iawscomponent_FSCNOZ;
    iAwsComponent *iawscomponent_FSCNElevation;
    iAwsComponent *iawscomponent_FSCNAzimuth;
    iAwsComponent *iawscomponent_FSCNAperture;
    iAwsComponent *iawscomponent_FSCNMin;
    iAwsComponent *iawscomponent_FSCNMax;
    iAwsComponent *iawscomponent_FSCNWeight;
    iAwsComponent *iawscomponent_FSCYSX;
    iAwsComponent *iawscomponent_FSCYSY;
    iAwsComponent *iawscomponent_FSCYSZ;
    iAwsComponent *iawscomponent_FSCYEX;
    iAwsComponent *iawscomponent_FSCYEY;
    iAwsComponent *iawscomponent_FSCYEZ;
    iAwsComponent *iawscomponent_FSCYMin;
    iAwsComponent *iawscomponent_FSCYMax;
    iAwsComponent *iawscomponent_FSCYWeight;
    iAwsComponent *iawscomponent_FSSTCX;
    iAwsComponent *iawscomponent_FSSTCY;
    iAwsComponent *iawscomponent_FSSTCZ;
    iAwsComponent *iawscomponent_FSSTMin;
    iAwsComponent *iawscomponent_FSSTMax;
    iAwsComponent *iawscomponent_FSSTWeight;
    iAwsComponent *iawscomponent_FSCYTSX;
    iAwsComponent *iawscomponent_FSCYTSY;
    iAwsComponent *iawscomponent_FSCYTSZ;
    iAwsComponent *iawscomponent_FSCYTEX;
    iAwsComponent *iawscomponent_FSCYTEY;
    iAwsComponent *iawscomponent_FSCYTEZ;
    iAwsComponent *iawscomponent_FSCYTMin;
    iAwsComponent *iawscomponent_FSCYTMax;
    iAwsComponent *iawscomponent_FSCYTWeight;
    FieldState state;
    bool settings_changed;
  } FieldSpeedData;

  struct st_FieldAccelerationData {
    iAwsComponent *iawscomponent_FieldAccel;
    iAwsComponent *iawscomponent_FAActive;
    iAwsComponent *iawscomponent_FAFPX;
    iAwsComponent *iawscomponent_FAFPY;
    iAwsComponent *iawscomponent_FAFPZ;
    iAwsComponent *iawscomponent_FAFWeight;
    iAwsComponent *iawscomponent_FALSX;
    iAwsComponent *iawscomponent_FALSY;
    iAwsComponent *iawscomponent_FALSZ;
    iAwsComponent *iawscomponent_FALEX;
    iAwsComponent *iawscomponent_FALEY;
    iAwsComponent *iawscomponent_FALEZ;
    iAwsComponent *iawscomponent_FALWeight;
    iAwsComponent *iawscomponent_FABMX;
    iAwsComponent *iawscomponent_FABMY;
    iAwsComponent *iawscomponent_FABMZ;
    iAwsComponent *iawscomponent_FABXX;
    iAwsComponent *iawscomponent_FABXY;
    iAwsComponent *iawscomponent_FABXZ;
    iAwsComponent *iawscomponent_FABWeight;
    iAwsComponent *iawscomponent_FASCX;
    iAwsComponent *iawscomponent_FASCY;
    iAwsComponent *iawscomponent_FASCZ;
    iAwsComponent *iawscomponent_FASMin;
    iAwsComponent *iawscomponent_FASMax;
    iAwsComponent *iawscomponent_FASWeight;
    iAwsComponent *iawscomponent_FACNOX;
    iAwsComponent *iawscomponent_FACNOY;
    iAwsComponent *iawscomponent_FACNOZ;
    iAwsComponent *iawscomponent_FACNElevation;
    iAwsComponent *iawscomponent_FACNAzimuth;
    iAwsComponent *iawscomponent_FACNAperture;
    iAwsComponent *iawscomponent_FACNMin;
    iAwsComponent *iawscomponent_FACNMax;
    iAwsComponent *iawscomponent_FACNWeight;
    iAwsComponent *iawscomponent_FACYSX;
    iAwsComponent *iawscomponent_FACYSY;
    iAwsComponent *iawscomponent_FACYSZ;
    iAwsComponent *iawscomponent_FACYEX;
    iAwsComponent *iawscomponent_FACYEY;
    iAwsComponent *iawscomponent_FACYEZ;
    iAwsComponent *iawscomponent_FACYMin;
    iAwsComponent *iawscomponent_FACYMax;
    iAwsComponent *iawscomponent_FACYWeight;
    iAwsComponent *iawscomponent_FASTCX;
    iAwsComponent *iawscomponent_FASTCY;
    iAwsComponent *iawscomponent_FASTCZ;
    iAwsComponent *iawscomponent_FASTMin;
    iAwsComponent *iawscomponent_FASTMax;
    iAwsComponent *iawscomponent_FASTWeight;
    iAwsComponent *iawscomponent_FACYTSX;
    iAwsComponent *iawscomponent_FACYTSY;
    iAwsComponent *iawscomponent_FACYTSZ;
    iAwsComponent *iawscomponent_FACYTEX;
    iAwsComponent *iawscomponent_FACYTEY;
    iAwsComponent *iawscomponent_FACYTEZ;
    iAwsComponent *iawscomponent_FACYTMin;
    iAwsComponent *iawscomponent_FACYTMax;
    iAwsComponent *iawscomponent_FACYTWeight;
    FieldState state;
    bool settings_changed;
  } FieldAccelerationData;

  struct st_AttractorData {
    iAwsComponent *iawscomponent_Attractor;
    iAwsComponent *iawscomponent_ATForce;
    iAwsComponent *iawscomponent_ATFPX;
    iAwsComponent *iawscomponent_ATFPY;
    iAwsComponent *iawscomponent_ATFPZ;
    iAwsComponent *iawscomponent_ATFWeight;
    iAwsComponent *iawscomponent_ATLSX;
    iAwsComponent *iawscomponent_ATLSY;
    iAwsComponent *iawscomponent_ATLSZ;
    iAwsComponent *iawscomponent_ATLEX;
    iAwsComponent *iawscomponent_ATLEY;
    iAwsComponent *iawscomponent_ATLEZ;
    iAwsComponent *iawscomponent_ATLWeight;
    iAwsComponent *iawscomponent_ATBMX;
    iAwsComponent *iawscomponent_ATBMY;
    iAwsComponent *iawscomponent_ATBMZ;
    iAwsComponent *iawscomponent_ATBXX;
    iAwsComponent *iawscomponent_ATBXY;
    iAwsComponent *iawscomponent_ATBXZ;
    iAwsComponent *iawscomponent_ATBWeight;
    iAwsComponent *iawscomponent_ATSCX;
    iAwsComponent *iawscomponent_ATSCY;
    iAwsComponent *iawscomponent_ATSCZ;
    iAwsComponent *iawscomponent_ATSMin;
    iAwsComponent *iawscomponent_ATSMax;
    iAwsComponent *iawscomponent_ATSWeight;
    iAwsComponent *iawscomponent_ATCNOX;
    iAwsComponent *iawscomponent_ATCNOY;
    iAwsComponent *iawscomponent_ATCNOZ;
    iAwsComponent *iawscomponent_ATCNElevation;
    iAwsComponent *iawscomponent_ATCNAzimuth;
    iAwsComponent *iawscomponent_ATCNAperture;
    iAwsComponent *iawscomponent_ATCNMin;
    iAwsComponent *iawscomponent_ATCNMax;
    iAwsComponent *iawscomponent_ATCNWeight;
    iAwsComponent *iawscomponent_ATCYSX;
    iAwsComponent *iawscomponent_ATCYSY;
    iAwsComponent *iawscomponent_ATCYSZ;
    iAwsComponent *iawscomponent_ATCYEX;
    iAwsComponent *iawscomponent_ATCYEY;
    iAwsComponent *iawscomponent_ATCYEZ;
    iAwsComponent *iawscomponent_ATCYMin;
    iAwsComponent *iawscomponent_ATCYMax;
    iAwsComponent *iawscomponent_ATCYWeight;
    iAwsComponent *iawscomponent_ATSTCX;
    iAwsComponent *iawscomponent_ATSTCY;
    iAwsComponent *iawscomponent_ATSTCZ;
    iAwsComponent *iawscomponent_ATSTMin;
    iAwsComponent *iawscomponent_ATSTMax;
    iAwsComponent *iawscomponent_ATSTWeight;
    iAwsComponent *iawscomponent_ATCYTSX;
    iAwsComponent *iawscomponent_ATCYTSY;
    iAwsComponent *iawscomponent_ATCYTSZ;
    iAwsComponent *iawscomponent_ATCYTEX;
    iAwsComponent *iawscomponent_ATCYTEY;
    iAwsComponent *iawscomponent_ATCYTEZ;
    iAwsComponent *iawscomponent_ATCYTMin;
    iAwsComponent *iawscomponent_ATCYTMax;
    iAwsComponent *iawscomponent_ATCYTWeight;

    AttractorState state;
    bool settings_changed;
  } AttractorData;

  struct st_AgingMomentsData {
    iAwsComponent *iawscomponent_AgingMoments;
  } AgingMomentsData;

  struct st_LoadSaveData {
    iAwsComponent *iawscomponent_LoadSave;
    LoadSaveFileState state;
    bool settings_changed;
  } LoadSaveData;

  /*  The iAwsComponent interface to the left sidebar (Section Selection Menu)
   *    This is initialized when the .def file is loaded and parsed.
   */
  iAwsComponent *iawscomponent_SectionSelection;

  /*  The iAwsComponent interface to the to the list box in the left sidebar
   *    This is initialized when the .def file is loaded and parsed.
   */
  iAwsComponent *iawscomponent_SectionList;


//  iAwsWindow *test;

  bool SectionState[SECTION_COUNT];

private:

  /// Static callback to handle left sidebar registration during .def file parsing.
  static void RegisterSectionSelection(intptr_t sk, iAwsSource *source);

  /// Static callback to handle left sidebar list box registration during .def file parsing.
  static void FillSectionList(intptr_t sk, iAwsSource *source);

  /// Static callback to handle selection/state changes in the section list box
  static void PartEditSink::SectionListSelectionChanged(intptr_t sk, iAwsSource *source);



  /// Static callback to handle graphic file selection window registration during .def file parsing.
  static void RegisterGraphicSelection(intptr_t sk, iAwsSource *source);
  /// Static callback to handle graphic file filter text box registration during .def file parsing.
  static void RegisterGraphicFilter(intptr_t sk, iAwsSource *source);
  /// Static callback to handle graphic file list registration during .def file parsing
  static void RegisterGraphicFileList(intptr_t sk, iAwsSource *source);
  /// Static callback to handle graphic file filter change.
  static void AwsSetGraphicFilter(intptr_t sk, iAwsSource *source);
  /// Static callback to handle graphic file selection change
  static void AwsGraphicFileSelected(intptr_t sk, iAwsSource *source);


  /// Static callback to handle emitter state option window registration during .def file parsing.
  static void RegisterEmitterState(intptr_t sk, iAwsSource *source);
  static void RegisterParticleCount(intptr_t sk, iAwsSource *source);
  static void RegisterParticleMaxAge(intptr_t sk, iAwsSource *source);
  static void RegisterLighting(intptr_t sk, iAwsSource *source);
  static void RegisterAlphaBlend(intptr_t sk, iAwsSource *source);
  static void RegisterRectParticlesRadio(intptr_t sk, iAwsSource *source);
  static void RegisterRegParticlesRadio(intptr_t sk, iAwsSource *source);
  static void RegisterRectParticlesWidth(intptr_t sk, iAwsSource *source);
  static void RegisterRectParticlesHeight(intptr_t sk, iAwsSource *source);
  static void RegisterRegParticlesNumber(intptr_t sk, iAwsSource *source);
  static void RegisterRegParticlesRadius(intptr_t sk, iAwsSource *source);
  static void RegisterUseBoundingBox(intptr_t sk, iAwsSource *source);
  static void RegisterBBoxMinX(intptr_t sk, iAwsSource *source);
  static void RegisterBBoxMinY(intptr_t sk, iAwsSource *source);
  static void RegisterBBoxMinZ(intptr_t sk, iAwsSource *source);
  static void RegisterBBoxMaxX(intptr_t sk, iAwsSource *source);
  static void RegisterBBoxMaxY(intptr_t sk, iAwsSource *source);
  static void RegisterBBoxMaxZ(intptr_t sk, iAwsSource *source);

  static void AwsSetParticleCount(intptr_t sk, iAwsSource *source);
  static void AwsSetParticleMaxAge(intptr_t sk, iAwsSource *source);
  static void AwsSetLighting(intptr_t sk, iAwsSource *source);
  static void AwsSetAlphaBlend(intptr_t sk, iAwsSource *source);
  static void AwsSetParticleType(intptr_t sk, iAwsSource *source);
  static void AwsSetRectangularWidth(intptr_t sk, iAwsSource *source);
  static void AwsSetRectangularHeight(intptr_t sk, iAwsSource *source);
  static void AwsSetRegularNumber(intptr_t sk, iAwsSource *source);
  static void AwsSetRegularRadius(intptr_t sk, iAwsSource *source);
  static void AwsSetUseBoundingBox(intptr_t sk, iAwsSource *source);
  static void AwsSetBBoxMinX(intptr_t sk, iAwsSource *source);
  static void AwsSetBBoxMinY(intptr_t sk, iAwsSource *source);
  static void AwsSetBBoxMinZ(intptr_t sk, iAwsSource *source);
  static void AwsSetBBoxMaxX(intptr_t sk, iAwsSource *source);
  static void AwsSetBBoxMaxY(intptr_t sk, iAwsSource *source);
  static void AwsSetBBoxMaxZ(intptr_t sk, iAwsSource *source);

  /// Static callback to handle initial position option window registration during .def file parsing.
  static void RegisterInitialPosition(intptr_t sk, iAwsSource *source);
  static void RegisterIPFPX(intptr_t sk, iAwsSource *source);
  static void RegisterIPFPY(intptr_t sk, iAwsSource *source);
  static void RegisterIPFPZ(intptr_t sk, iAwsSource *source);
  static void RegisterIPFWeight(intptr_t sk, iAwsSource *source);
  static void RegisterIPLSX(intptr_t sk, iAwsSource *source);
  static void RegisterIPLSY(intptr_t sk, iAwsSource *source);
  static void RegisterIPLSZ(intptr_t sk, iAwsSource *source);
  static void RegisterIPLEX(intptr_t sk, iAwsSource *source);
  static void RegisterIPLEY(intptr_t sk, iAwsSource *source);
  static void RegisterIPLEZ(intptr_t sk, iAwsSource *source);
  static void RegisterIPLWeight(intptr_t sk, iAwsSource *source);
  static void RegisterIPBMX(intptr_t sk, iAwsSource *source);
  static void RegisterIPBMY(intptr_t sk, iAwsSource *source);
  static void RegisterIPBMZ(intptr_t sk, iAwsSource *source);
  static void RegisterIPBXX(intptr_t sk, iAwsSource *source);
  static void RegisterIPBXY(intptr_t sk, iAwsSource *source);
  static void RegisterIPBXZ(intptr_t sk, iAwsSource *source);
  static void RegisterIPBWeight(intptr_t sk, iAwsSource *source);
  static void RegisterIPSCX(intptr_t sk, iAwsSource *source);
  static void RegisterIPSCY(intptr_t sk, iAwsSource *source);
  static void RegisterIPSCZ(intptr_t sk, iAwsSource *source);
  static void RegisterIPSMin(intptr_t sk, iAwsSource *source);
  static void RegisterIPSMax(intptr_t sk, iAwsSource *source);
  static void RegisterIPSWeight(intptr_t sk, iAwsSource *source);
  static void RegisterIPCNOX(intptr_t sk, iAwsSource *source);
  static void RegisterIPCNOY(intptr_t sk, iAwsSource *source);
  static void RegisterIPCNOZ(intptr_t sk, iAwsSource *source);
  static void RegisterIPCNElev(intptr_t sk, iAwsSource *source);
  static void RegisterIPCNAzim(intptr_t sk, iAwsSource *source);
  static void RegisterIPCNAper(intptr_t sk, iAwsSource *source);
  static void RegisterIPCNMin(intptr_t sk, iAwsSource *source);
  static void RegisterIPCNMax(intptr_t sk, iAwsSource *source);
  static void RegisterIPCNWeight(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYSX(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYSY(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYSZ(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYEX(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYEY(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYEZ(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYMin(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYMax(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYWeight(intptr_t sk, iAwsSource *source);
  static void RegisterIPSTCX(intptr_t sk, iAwsSource *source);
  static void RegisterIPSTCY(intptr_t sk, iAwsSource *source);
  static void RegisterIPSTCZ(intptr_t sk, iAwsSource *source);
  static void RegisterIPSTMin(intptr_t sk, iAwsSource *source);
  static void RegisterIPSTMax(intptr_t sk, iAwsSource *source);
  static void RegisterIPSTWeight(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTSX(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTSY(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTSZ(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTEX(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTEY(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTEZ(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTMin(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTMax(intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTWeight(intptr_t sk, iAwsSource *source);

  static void AwsSetIPFPositionX(intptr_t sk, iAwsSource *source);
  static void AwsSetIPFPositionY(intptr_t sk, iAwsSource *source);
  static void AwsSetIPFPositionZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIPFWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetIPLStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetIPLStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetIPLStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIPLEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetIPLEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetIPLEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIPLWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetIPBMinX(intptr_t sk, iAwsSource *source);
  static void AwsSetIPBMinY(intptr_t sk, iAwsSource *source);
  static void AwsSetIPBMinZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIPBMaxX(intptr_t sk, iAwsSource *source);
  static void AwsSetIPBMaxY(intptr_t sk, iAwsSource *source);
  static void AwsSetIPBMaxZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIPBWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetIPSCenterX(intptr_t sk, iAwsSource *source);
  static void AwsSetIPSCenterY(intptr_t sk, iAwsSource *source);
  static void AwsSetIPSCenterZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIPSMin(intptr_t sk, iAwsSource *source);
  static void AwsSetIPSMax(intptr_t sk, iAwsSource *source);
  static void AwsSetIPSWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNOriginX(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNOriginY(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNOriginZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNElev(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNAzim(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNAper(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNMin(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNMax(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYMin(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYMax(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetIPSTCenterX(intptr_t sk, iAwsSource *source);
  static void AwsSetIPSTCenterY(intptr_t sk, iAwsSource *source);
  static void AwsSetIPSTCenterZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIPSTMin(intptr_t sk, iAwsSource *source);
  static void AwsSetIPSTMax(intptr_t sk, iAwsSource *source);
  static void AwsSetIPSTWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTMin(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTMax(intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTWeight(intptr_t sk, iAwsSource *source);


  /// Static callback to handle initial speed option window registration during .def file parsing.
  static void RegisterInitialSpeed(intptr_t sk, iAwsSource *source);
  static void RegisterISFPX(intptr_t sk, iAwsSource *source);
  static void RegisterISFPY(intptr_t sk, iAwsSource *source);
  static void RegisterISFPZ(intptr_t sk, iAwsSource *source);
  static void RegisterISFWeight(intptr_t sk, iAwsSource *source);
  static void RegisterISLSX(intptr_t sk, iAwsSource *source);
  static void RegisterISLSY(intptr_t sk, iAwsSource *source);
  static void RegisterISLSZ(intptr_t sk, iAwsSource *source);
  static void RegisterISLEX(intptr_t sk, iAwsSource *source);
  static void RegisterISLEY(intptr_t sk, iAwsSource *source);
  static void RegisterISLEZ(intptr_t sk, iAwsSource *source);
  static void RegisterISLWeight(intptr_t sk, iAwsSource *source);
  static void RegisterISBMX(intptr_t sk, iAwsSource *source);
  static void RegisterISBMY(intptr_t sk, iAwsSource *source);
  static void RegisterISBMZ(intptr_t sk, iAwsSource *source);
  static void RegisterISBXX(intptr_t sk, iAwsSource *source);
  static void RegisterISBXY(intptr_t sk, iAwsSource *source);
  static void RegisterISBXZ(intptr_t sk, iAwsSource *source);
  static void RegisterISBWeight(intptr_t sk, iAwsSource *source);
  static void RegisterISSCX(intptr_t sk, iAwsSource *source);
  static void RegisterISSCY(intptr_t sk, iAwsSource *source);
  static void RegisterISSCZ(intptr_t sk, iAwsSource *source);
  static void RegisterISSMin(intptr_t sk, iAwsSource *source);
  static void RegisterISSMax(intptr_t sk, iAwsSource *source);
  static void RegisterISSWeight(intptr_t sk, iAwsSource *source);
  static void RegisterISCNOX(intptr_t sk, iAwsSource *source);
  static void RegisterISCNOY(intptr_t sk, iAwsSource *source);
  static void RegisterISCNOZ(intptr_t sk, iAwsSource *source);
  static void RegisterISCNElev(intptr_t sk, iAwsSource *source);
  static void RegisterISCNAzim(intptr_t sk, iAwsSource *source);
  static void RegisterISCNAper(intptr_t sk, iAwsSource *source);
  static void RegisterISCNMin(intptr_t sk, iAwsSource *source);
  static void RegisterISCNMax(intptr_t sk, iAwsSource *source);
  static void RegisterISCNWeight(intptr_t sk, iAwsSource *source);
  static void RegisterISCYSX(intptr_t sk, iAwsSource *source);
  static void RegisterISCYSY(intptr_t sk, iAwsSource *source);
  static void RegisterISCYSZ(intptr_t sk, iAwsSource *source);
  static void RegisterISCYEX(intptr_t sk, iAwsSource *source);
  static void RegisterISCYEY(intptr_t sk, iAwsSource *source);
  static void RegisterISCYEZ(intptr_t sk, iAwsSource *source);
  static void RegisterISCYMin(intptr_t sk, iAwsSource *source);
  static void RegisterISCYMax(intptr_t sk, iAwsSource *source);
  static void RegisterISCYWeight(intptr_t sk, iAwsSource *source);
  static void RegisterISSTCX(intptr_t sk, iAwsSource *source);
  static void RegisterISSTCY(intptr_t sk, iAwsSource *source);
  static void RegisterISSTCZ(intptr_t sk, iAwsSource *source);
  static void RegisterISSTMin(intptr_t sk, iAwsSource *source);
  static void RegisterISSTMax(intptr_t sk, iAwsSource *source);
  static void RegisterISSTWeight(intptr_t sk, iAwsSource *source);
  static void RegisterISCYTSX(intptr_t sk, iAwsSource *source);
  static void RegisterISCYTSY(intptr_t sk, iAwsSource *source);
  static void RegisterISCYTSZ(intptr_t sk, iAwsSource *source);
  static void RegisterISCYTEX(intptr_t sk, iAwsSource *source);
  static void RegisterISCYTEY(intptr_t sk, iAwsSource *source);
  static void RegisterISCYTEZ(intptr_t sk, iAwsSource *source);
  static void RegisterISCYTMin(intptr_t sk, iAwsSource *source);
  static void RegisterISCYTMax(intptr_t sk, iAwsSource *source);
  static void RegisterISCYTWeight(intptr_t sk, iAwsSource *source);

  static void AwsSetISFPositionX(intptr_t sk, iAwsSource *source);
  static void AwsSetISFPositionY(intptr_t sk, iAwsSource *source);
  static void AwsSetISFPositionZ(intptr_t sk, iAwsSource *source);
  static void AwsSetISFWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetISLStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetISLStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetISLStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetISLEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetISLEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetISLEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetISLWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetISBMinX(intptr_t sk, iAwsSource *source);
  static void AwsSetISBMinY(intptr_t sk, iAwsSource *source);
  static void AwsSetISBMinZ(intptr_t sk, iAwsSource *source);
  static void AwsSetISBMaxX(intptr_t sk, iAwsSource *source);
  static void AwsSetISBMaxY(intptr_t sk, iAwsSource *source);
  static void AwsSetISBMaxZ(intptr_t sk, iAwsSource *source);
  static void AwsSetISBWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetISSCenterX(intptr_t sk, iAwsSource *source);
  static void AwsSetISSCenterY(intptr_t sk, iAwsSource *source);
  static void AwsSetISSCenterZ(intptr_t sk, iAwsSource *source);
  static void AwsSetISSMin(intptr_t sk, iAwsSource *source);
  static void AwsSetISSMax(intptr_t sk, iAwsSource *source);
  static void AwsSetISSWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetISCNOriginX(intptr_t sk, iAwsSource *source);
  static void AwsSetISCNOriginY(intptr_t sk, iAwsSource *source);
  static void AwsSetISCNOriginZ(intptr_t sk, iAwsSource *source);
  static void AwsSetISCNElev(intptr_t sk, iAwsSource *source);
  static void AwsSetISCNAzim(intptr_t sk, iAwsSource *source);
  static void AwsSetISCNAper(intptr_t sk, iAwsSource *source);
  static void AwsSetISCNMin(intptr_t sk, iAwsSource *source);
  static void AwsSetISCNMax(intptr_t sk, iAwsSource *source);
  static void AwsSetISCNWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYMin(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYMax(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetISSTCenterX(intptr_t sk, iAwsSource *source);
  static void AwsSetISSTCenterY(intptr_t sk, iAwsSource *source);
  static void AwsSetISSTCenterZ(intptr_t sk, iAwsSource *source);
  static void AwsSetISSTMin(intptr_t sk, iAwsSource *source);
  static void AwsSetISSTMax(intptr_t sk, iAwsSource *source);
  static void AwsSetISSTWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTMin(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTMax(intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTWeight(intptr_t sk, iAwsSource *source);




  /// Static callback to handle initial acceleration option window registration during .def file parsing.
  static void RegisterInitialAcceleration(intptr_t sk, iAwsSource *source);
  static void RegisterIAFPX(intptr_t sk, iAwsSource *source);
  static void RegisterIAFPY(intptr_t sk, iAwsSource *source);
  static void RegisterIAFPZ(intptr_t sk, iAwsSource *source);
  static void RegisterIAFWeight(intptr_t sk, iAwsSource *source);
  static void RegisterIALSX(intptr_t sk, iAwsSource *source);
  static void RegisterIALSY(intptr_t sk, iAwsSource *source);
  static void RegisterIALSZ(intptr_t sk, iAwsSource *source);
  static void RegisterIALEX(intptr_t sk, iAwsSource *source);
  static void RegisterIALEY(intptr_t sk, iAwsSource *source);
  static void RegisterIALEZ(intptr_t sk, iAwsSource *source);
  static void RegisterIALWeight(intptr_t sk, iAwsSource *source);
  static void RegisterIABMX(intptr_t sk, iAwsSource *source);
  static void RegisterIABMY(intptr_t sk, iAwsSource *source);
  static void RegisterIABMZ(intptr_t sk, iAwsSource *source);
  static void RegisterIABXX(intptr_t sk, iAwsSource *source);
  static void RegisterIABXY(intptr_t sk, iAwsSource *source);
  static void RegisterIABXZ(intptr_t sk, iAwsSource *source);
  static void RegisterIABWeight(intptr_t sk, iAwsSource *source);
  static void RegisterIASCX(intptr_t sk, iAwsSource *source);
  static void RegisterIASCY(intptr_t sk, iAwsSource *source);
  static void RegisterIASCZ(intptr_t sk, iAwsSource *source);
  static void RegisterIASMin(intptr_t sk, iAwsSource *source);
  static void RegisterIASMax(intptr_t sk, iAwsSource *source);
  static void RegisterIASWeight(intptr_t sk, iAwsSource *source);
  static void RegisterIACNOX(intptr_t sk, iAwsSource *source);
  static void RegisterIACNOY(intptr_t sk, iAwsSource *source);
  static void RegisterIACNOZ(intptr_t sk, iAwsSource *source);
  static void RegisterIACNElev(intptr_t sk, iAwsSource *source);
  static void RegisterIACNAzim(intptr_t sk, iAwsSource *source);
  static void RegisterIACNAper(intptr_t sk, iAwsSource *source);
  static void RegisterIACNMin(intptr_t sk, iAwsSource *source);
  static void RegisterIACNMax(intptr_t sk, iAwsSource *source);
  static void RegisterIACNWeight(intptr_t sk, iAwsSource *source);
  static void RegisterIACYSX(intptr_t sk, iAwsSource *source);
  static void RegisterIACYSY(intptr_t sk, iAwsSource *source);
  static void RegisterIACYSZ(intptr_t sk, iAwsSource *source);
  static void RegisterIACYEX(intptr_t sk, iAwsSource *source);
  static void RegisterIACYEY(intptr_t sk, iAwsSource *source);
  static void RegisterIACYEZ(intptr_t sk, iAwsSource *source);
  static void RegisterIACYMin(intptr_t sk, iAwsSource *source);
  static void RegisterIACYMax(intptr_t sk, iAwsSource *source);
  static void RegisterIACYWeight(intptr_t sk, iAwsSource *source);
  static void RegisterIASTCX(intptr_t sk, iAwsSource *source);
  static void RegisterIASTCY(intptr_t sk, iAwsSource *source);
  static void RegisterIASTCZ(intptr_t sk, iAwsSource *source);
  static void RegisterIASTMin(intptr_t sk, iAwsSource *source);
  static void RegisterIASTMax(intptr_t sk, iAwsSource *source);
  static void RegisterIASTWeight(intptr_t sk, iAwsSource *source);
  static void RegisterIACYTSX(intptr_t sk, iAwsSource *source);
  static void RegisterIACYTSY(intptr_t sk, iAwsSource *source);
  static void RegisterIACYTSZ(intptr_t sk, iAwsSource *source);
  static void RegisterIACYTEX(intptr_t sk, iAwsSource *source);
  static void RegisterIACYTEY(intptr_t sk, iAwsSource *source);
  static void RegisterIACYTEZ(intptr_t sk, iAwsSource *source);
  static void RegisterIACYTMin(intptr_t sk, iAwsSource *source);
  static void RegisterIACYTMax(intptr_t sk, iAwsSource *source);
  static void RegisterIACYTWeight(intptr_t sk, iAwsSource *source);

  static void AwsSetIAFPositionX(intptr_t sk, iAwsSource *source);
  static void AwsSetIAFPositionY(intptr_t sk, iAwsSource *source);
  static void AwsSetIAFPositionZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIAFWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetIALStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetIALStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetIALStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIALEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetIALEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetIALEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIALWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetIABMinX(intptr_t sk, iAwsSource *source);
  static void AwsSetIABMinY(intptr_t sk, iAwsSource *source);
  static void AwsSetIABMinZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIABMaxX(intptr_t sk, iAwsSource *source);
  static void AwsSetIABMaxY(intptr_t sk, iAwsSource *source);
  static void AwsSetIABMaxZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIABWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetIASCenterX(intptr_t sk, iAwsSource *source);
  static void AwsSetIASCenterY(intptr_t sk, iAwsSource *source);
  static void AwsSetIASCenterZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIASMin(intptr_t sk, iAwsSource *source);
  static void AwsSetIASMax(intptr_t sk, iAwsSource *source);
  static void AwsSetIASWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetIACNOriginX(intptr_t sk, iAwsSource *source);
  static void AwsSetIACNOriginY(intptr_t sk, iAwsSource *source);
  static void AwsSetIACNOriginZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIACNElev(intptr_t sk, iAwsSource *source);
  static void AwsSetIACNAzim(intptr_t sk, iAwsSource *source);
  static void AwsSetIACNAper(intptr_t sk, iAwsSource *source);
  static void AwsSetIACNMin(intptr_t sk, iAwsSource *source);
  static void AwsSetIACNMax(intptr_t sk, iAwsSource *source);
  static void AwsSetIACNWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYMin(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYMax(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetIASTCenterX(intptr_t sk, iAwsSource *source);
  static void AwsSetIASTCenterY(intptr_t sk, iAwsSource *source);
  static void AwsSetIASTCenterZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIASTMin(intptr_t sk, iAwsSource *source);
  static void AwsSetIASTMax(intptr_t sk, iAwsSource *source);
  static void AwsSetIASTWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTMin(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTMax(intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTWeight(intptr_t sk, iAwsSource *source);



  /// Static callback to handle field speed option window registration during .def file parsing.
  static void RegisterFieldSpeed(intptr_t sk, iAwsSource *source);
  static void RegisterFSActive(intptr_t sk, iAwsSource *source);
  static void RegisterFSFPX(intptr_t sk, iAwsSource *source);
  static void RegisterFSFPY(intptr_t sk, iAwsSource *source);
  static void RegisterFSFPZ(intptr_t sk, iAwsSource *source);
  static void RegisterFSFWeight(intptr_t sk, iAwsSource *source);
  static void RegisterFSLSX(intptr_t sk, iAwsSource *source);
  static void RegisterFSLSY(intptr_t sk, iAwsSource *source);
  static void RegisterFSLSZ(intptr_t sk, iAwsSource *source);
  static void RegisterFSLEX(intptr_t sk, iAwsSource *source);
  static void RegisterFSLEY(intptr_t sk, iAwsSource *source);
  static void RegisterFSLEZ(intptr_t sk, iAwsSource *source);
  static void RegisterFSLWeight(intptr_t sk, iAwsSource *source);
  static void RegisterFSBMX(intptr_t sk, iAwsSource *source);
  static void RegisterFSBMY(intptr_t sk, iAwsSource *source);
  static void RegisterFSBMZ(intptr_t sk, iAwsSource *source);
  static void RegisterFSBXX(intptr_t sk, iAwsSource *source);
  static void RegisterFSBXY(intptr_t sk, iAwsSource *source);
  static void RegisterFSBXZ(intptr_t sk, iAwsSource *source);
  static void RegisterFSBWeight(intptr_t sk, iAwsSource *source);
  static void RegisterFSSCX(intptr_t sk, iAwsSource *source);
  static void RegisterFSSCY(intptr_t sk, iAwsSource *source);
  static void RegisterFSSCZ(intptr_t sk, iAwsSource *source);
  static void RegisterFSSMin(intptr_t sk, iAwsSource *source);
  static void RegisterFSSMax(intptr_t sk, iAwsSource *source);
  static void RegisterFSSWeight(intptr_t sk, iAwsSource *source);
  static void RegisterFSCNOX(intptr_t sk, iAwsSource *source);
  static void RegisterFSCNOY(intptr_t sk, iAwsSource *source);
  static void RegisterFSCNOZ(intptr_t sk, iAwsSource *source);
  static void RegisterFSCNElev(intptr_t sk, iAwsSource *source);
  static void RegisterFSCNAzim(intptr_t sk, iAwsSource *source);
  static void RegisterFSCNAper(intptr_t sk, iAwsSource *source);
  static void RegisterFSCNMin(intptr_t sk, iAwsSource *source);
  static void RegisterFSCNMax(intptr_t sk, iAwsSource *source);
  static void RegisterFSCNWeight(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYSX(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYSY(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYSZ(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYEX(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYEY(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYEZ(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYMin(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYMax(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYWeight(intptr_t sk, iAwsSource *source);
  static void RegisterFSSTCX(intptr_t sk, iAwsSource *source);
  static void RegisterFSSTCY(intptr_t sk, iAwsSource *source);
  static void RegisterFSSTCZ(intptr_t sk, iAwsSource *source);
  static void RegisterFSSTMin(intptr_t sk, iAwsSource *source);
  static void RegisterFSSTMax(intptr_t sk, iAwsSource *source);
  static void RegisterFSSTWeight(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTSX(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTSY(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTSZ(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTEX(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTEY(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTEZ(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTMin(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTMax(intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTWeight(intptr_t sk, iAwsSource *source);

  static void AwsSetFSActive(intptr_t sk, iAwsSource *source);
  static void AwsSetFSFPositionX(intptr_t sk, iAwsSource *source);
  static void AwsSetFSFPositionY(intptr_t sk, iAwsSource *source);
  static void AwsSetFSFPositionZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFSFWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetFSLStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetFSLStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetFSLStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFSLEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetFSLEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetFSLEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFSLWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetFSBMinX(intptr_t sk, iAwsSource *source);
  static void AwsSetFSBMinY(intptr_t sk, iAwsSource *source);
  static void AwsSetFSBMinZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFSBMaxX(intptr_t sk, iAwsSource *source);
  static void AwsSetFSBMaxY(intptr_t sk, iAwsSource *source);
  static void AwsSetFSBMaxZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFSBWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetFSSCenterX(intptr_t sk, iAwsSource *source);
  static void AwsSetFSSCenterY(intptr_t sk, iAwsSource *source);
  static void AwsSetFSSCenterZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFSSMin(intptr_t sk, iAwsSource *source);
  static void AwsSetFSSMax(intptr_t sk, iAwsSource *source);
  static void AwsSetFSSWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNOriginX(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNOriginY(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNOriginZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNElev(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNAzim(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNAper(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNMin(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNMax(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYMin(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYMax(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetFSSTCenterX(intptr_t sk, iAwsSource *source);
  static void AwsSetFSSTCenterY(intptr_t sk, iAwsSource *source);
  static void AwsSetFSSTCenterZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFSSTMin(intptr_t sk, iAwsSource *source);
  static void AwsSetFSSTMax(intptr_t sk, iAwsSource *source);
  static void AwsSetFSSTWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTMin(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTMax(intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTWeight(intptr_t sk, iAwsSource *source);


  /// Static callback to handle field acceleration option window registration during .def file parsing.
  static void RegisterFieldAccel(intptr_t sk, iAwsSource *source);
  static void RegisterFAActive(intptr_t sk, iAwsSource *source);
  static void RegisterFAFPX(intptr_t sk, iAwsSource *source);
  static void RegisterFAFPY(intptr_t sk, iAwsSource *source);
  static void RegisterFAFPZ(intptr_t sk, iAwsSource *source);
  static void RegisterFAFWeight(intptr_t sk, iAwsSource *source);
  static void RegisterFALSX(intptr_t sk, iAwsSource *source);
  static void RegisterFALSY(intptr_t sk, iAwsSource *source);
  static void RegisterFALSZ(intptr_t sk, iAwsSource *source);
  static void RegisterFALEX(intptr_t sk, iAwsSource *source);
  static void RegisterFALEY(intptr_t sk, iAwsSource *source);
  static void RegisterFALEZ(intptr_t sk, iAwsSource *source);
  static void RegisterFALWeight(intptr_t sk, iAwsSource *source);
  static void RegisterFABMX(intptr_t sk, iAwsSource *source);
  static void RegisterFABMY(intptr_t sk, iAwsSource *source);
  static void RegisterFABMZ(intptr_t sk, iAwsSource *source);
  static void RegisterFABXX(intptr_t sk, iAwsSource *source);
  static void RegisterFABXY(intptr_t sk, iAwsSource *source);
  static void RegisterFABXZ(intptr_t sk, iAwsSource *source);
  static void RegisterFABWeight(intptr_t sk, iAwsSource *source);
  static void RegisterFASCX(intptr_t sk, iAwsSource *source);
  static void RegisterFASCY(intptr_t sk, iAwsSource *source);
  static void RegisterFASCZ(intptr_t sk, iAwsSource *source);
  static void RegisterFASMin(intptr_t sk, iAwsSource *source);
  static void RegisterFASMax(intptr_t sk, iAwsSource *source);
  static void RegisterFASWeight(intptr_t sk, iAwsSource *source);
  static void RegisterFACNOX(intptr_t sk, iAwsSource *source);
  static void RegisterFACNOY(intptr_t sk, iAwsSource *source);
  static void RegisterFACNOZ(intptr_t sk, iAwsSource *source);
  static void RegisterFACNElev(intptr_t sk, iAwsSource *source);
  static void RegisterFACNAzim(intptr_t sk, iAwsSource *source);
  static void RegisterFACNAper(intptr_t sk, iAwsSource *source);
  static void RegisterFACNMin(intptr_t sk, iAwsSource *source);
  static void RegisterFACNMax(intptr_t sk, iAwsSource *source);
  static void RegisterFACNWeight(intptr_t sk, iAwsSource *source);
  static void RegisterFACYSX(intptr_t sk, iAwsSource *source);
  static void RegisterFACYSY(intptr_t sk, iAwsSource *source);
  static void RegisterFACYSZ(intptr_t sk, iAwsSource *source);
  static void RegisterFACYEX(intptr_t sk, iAwsSource *source);
  static void RegisterFACYEY(intptr_t sk, iAwsSource *source);
  static void RegisterFACYEZ(intptr_t sk, iAwsSource *source);
  static void RegisterFACYMin(intptr_t sk, iAwsSource *source);
  static void RegisterFACYMax(intptr_t sk, iAwsSource *source);
  static void RegisterFACYWeight(intptr_t sk, iAwsSource *source);
  static void RegisterFASTCX(intptr_t sk, iAwsSource *source);
  static void RegisterFASTCY(intptr_t sk, iAwsSource *source);
  static void RegisterFASTCZ(intptr_t sk, iAwsSource *source);
  static void RegisterFASTMin(intptr_t sk, iAwsSource *source);
  static void RegisterFASTMax(intptr_t sk, iAwsSource *source);
  static void RegisterFASTWeight(intptr_t sk, iAwsSource *source);
  static void RegisterFACYTSX(intptr_t sk, iAwsSource *source);
  static void RegisterFACYTSY(intptr_t sk, iAwsSource *source);
  static void RegisterFACYTSZ(intptr_t sk, iAwsSource *source);
  static void RegisterFACYTEX(intptr_t sk, iAwsSource *source);
  static void RegisterFACYTEY(intptr_t sk, iAwsSource *source);
  static void RegisterFACYTEZ(intptr_t sk, iAwsSource *source);
  static void RegisterFACYTMin(intptr_t sk, iAwsSource *source);
  static void RegisterFACYTMax(intptr_t sk, iAwsSource *source);
  static void RegisterFACYTWeight(intptr_t sk, iAwsSource *source);

  static void AwsSetFAActive(intptr_t sk, iAwsSource *source);
  static void AwsSetFAFPositionX(intptr_t sk, iAwsSource *source);
  static void AwsSetFAFPositionY(intptr_t sk, iAwsSource *source);
  static void AwsSetFAFPositionZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFAFWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetFALStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetFALStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetFALStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFALEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetFALEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetFALEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFALWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetFABMinX(intptr_t sk, iAwsSource *source);
  static void AwsSetFABMinY(intptr_t sk, iAwsSource *source);
  static void AwsSetFABMinZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFABMaxX(intptr_t sk, iAwsSource *source);
  static void AwsSetFABMaxY(intptr_t sk, iAwsSource *source);
  static void AwsSetFABMaxZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFABWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetFASCenterX(intptr_t sk, iAwsSource *source);
  static void AwsSetFASCenterY(intptr_t sk, iAwsSource *source);
  static void AwsSetFASCenterZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFASMin(intptr_t sk, iAwsSource *source);
  static void AwsSetFASMax(intptr_t sk, iAwsSource *source);
  static void AwsSetFASWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetFACNOriginX(intptr_t sk, iAwsSource *source);
  static void AwsSetFACNOriginY(intptr_t sk, iAwsSource *source);
  static void AwsSetFACNOriginZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFACNElev(intptr_t sk, iAwsSource *source);
  static void AwsSetFACNAzim(intptr_t sk, iAwsSource *source);
  static void AwsSetFACNAper(intptr_t sk, iAwsSource *source);
  static void AwsSetFACNMin(intptr_t sk, iAwsSource *source);
  static void AwsSetFACNMax(intptr_t sk, iAwsSource *source);
  static void AwsSetFACNWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYMin(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYMax(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetFASTCenterX(intptr_t sk, iAwsSource *source);
  static void AwsSetFASTCenterY(intptr_t sk, iAwsSource *source);
  static void AwsSetFASTCenterZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFASTMin(intptr_t sk, iAwsSource *source);
  static void AwsSetFASTMax(intptr_t sk, iAwsSource *source);
  static void AwsSetFASTWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTMin(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTMax(intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTWeight(intptr_t sk, iAwsSource *source);





  /// Static callback to handle attractor option window registration during .def file parsing.
  static void RegisterAttractor(intptr_t sk, iAwsSource *source);
  static void RegisterATForce(intptr_t sk, iAwsSource *source);
  static void RegisterATFPX(intptr_t sk, iAwsSource *source);
  static void RegisterATFPY(intptr_t sk, iAwsSource *source);
  static void RegisterATFPZ(intptr_t sk, iAwsSource *source);
  static void RegisterATFWeight(intptr_t sk, iAwsSource *source);
  static void RegisterATLSX(intptr_t sk, iAwsSource *source);
  static void RegisterATLSY(intptr_t sk, iAwsSource *source);
  static void RegisterATLSZ(intptr_t sk, iAwsSource *source);
  static void RegisterATLEX(intptr_t sk, iAwsSource *source);
  static void RegisterATLEY(intptr_t sk, iAwsSource *source);
  static void RegisterATLEZ(intptr_t sk, iAwsSource *source);
  static void RegisterATLWeight(intptr_t sk, iAwsSource *source);
  static void RegisterATBMX(intptr_t sk, iAwsSource *source);
  static void RegisterATBMY(intptr_t sk, iAwsSource *source);
  static void RegisterATBMZ(intptr_t sk, iAwsSource *source);
  static void RegisterATBXX(intptr_t sk, iAwsSource *source);
  static void RegisterATBXY(intptr_t sk, iAwsSource *source);
  static void RegisterATBXZ(intptr_t sk, iAwsSource *source);
  static void RegisterATBWeight(intptr_t sk, iAwsSource *source);
  static void RegisterATSCX(intptr_t sk, iAwsSource *source);
  static void RegisterATSCY(intptr_t sk, iAwsSource *source);
  static void RegisterATSCZ(intptr_t sk, iAwsSource *source);
  static void RegisterATSMin(intptr_t sk, iAwsSource *source);
  static void RegisterATSMax(intptr_t sk, iAwsSource *source);
  static void RegisterATSWeight(intptr_t sk, iAwsSource *source);
  static void RegisterATCNOX(intptr_t sk, iAwsSource *source);
  static void RegisterATCNOY(intptr_t sk, iAwsSource *source);
  static void RegisterATCNOZ(intptr_t sk, iAwsSource *source);
  static void RegisterATCNElev(intptr_t sk, iAwsSource *source);
  static void RegisterATCNAzim(intptr_t sk, iAwsSource *source);
  static void RegisterATCNAper(intptr_t sk, iAwsSource *source);
  static void RegisterATCNMin(intptr_t sk, iAwsSource *source);
  static void RegisterATCNMax(intptr_t sk, iAwsSource *source);
  static void RegisterATCNWeight(intptr_t sk, iAwsSource *source);
  static void RegisterATCYSX(intptr_t sk, iAwsSource *source);
  static void RegisterATCYSY(intptr_t sk, iAwsSource *source);
  static void RegisterATCYSZ(intptr_t sk, iAwsSource *source);
  static void RegisterATCYEX(intptr_t sk, iAwsSource *source);
  static void RegisterATCYEY(intptr_t sk, iAwsSource *source);
  static void RegisterATCYEZ(intptr_t sk, iAwsSource *source);
  static void RegisterATCYMin(intptr_t sk, iAwsSource *source);
  static void RegisterATCYMax(intptr_t sk, iAwsSource *source);
  static void RegisterATCYWeight(intptr_t sk, iAwsSource *source);
  static void RegisterATSTCX(intptr_t sk, iAwsSource *source);
  static void RegisterATSTCY(intptr_t sk, iAwsSource *source);
  static void RegisterATSTCZ(intptr_t sk, iAwsSource *source);
  static void RegisterATSTMin(intptr_t sk, iAwsSource *source);
  static void RegisterATSTMax(intptr_t sk, iAwsSource *source);
  static void RegisterATSTWeight(intptr_t sk, iAwsSource *source);
  static void RegisterATCYTSX(intptr_t sk, iAwsSource *source);
  static void RegisterATCYTSY(intptr_t sk, iAwsSource *source);
  static void RegisterATCYTSZ(intptr_t sk, iAwsSource *source);
  static void RegisterATCYTEX(intptr_t sk, iAwsSource *source);
  static void RegisterATCYTEY(intptr_t sk, iAwsSource *source);
  static void RegisterATCYTEZ(intptr_t sk, iAwsSource *source);
  static void RegisterATCYTMin(intptr_t sk, iAwsSource *source);
  static void RegisterATCYTMax(intptr_t sk, iAwsSource *source);
  static void RegisterATCYTWeight(intptr_t sk, iAwsSource *source);


  static void AwsSetATForce(intptr_t sk, iAwsSource *source);
  static void AwsSetATFPositionX(intptr_t sk, iAwsSource *source);
  static void AwsSetATFPositionY(intptr_t sk, iAwsSource *source);
  static void AwsSetATFPositionZ(intptr_t sk, iAwsSource *source);
  static void AwsSetATFWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetATLStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetATLStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetATLStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetATLEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetATLEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetATLEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetATLWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetATBMinX(intptr_t sk, iAwsSource *source);
  static void AwsSetATBMinY(intptr_t sk, iAwsSource *source);
  static void AwsSetATBMinZ(intptr_t sk, iAwsSource *source);
  static void AwsSetATBMaxX(intptr_t sk, iAwsSource *source);
  static void AwsSetATBMaxY(intptr_t sk, iAwsSource *source);
  static void AwsSetATBMaxZ(intptr_t sk, iAwsSource *source);
  static void AwsSetATBWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetATSCenterX(intptr_t sk, iAwsSource *source);
  static void AwsSetATSCenterY(intptr_t sk, iAwsSource *source);
  static void AwsSetATSCenterZ(intptr_t sk, iAwsSource *source);
  static void AwsSetATSMin(intptr_t sk, iAwsSource *source);
  static void AwsSetATSMax(intptr_t sk, iAwsSource *source);
  static void AwsSetATSWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetATCNOriginX(intptr_t sk, iAwsSource *source);
  static void AwsSetATCNOriginY(intptr_t sk, iAwsSource *source);
  static void AwsSetATCNOriginZ(intptr_t sk, iAwsSource *source);
  static void AwsSetATCNElev(intptr_t sk, iAwsSource *source);
  static void AwsSetATCNAzim(intptr_t sk, iAwsSource *source);
  static void AwsSetATCNAper(intptr_t sk, iAwsSource *source);
  static void AwsSetATCNMin(intptr_t sk, iAwsSource *source);
  static void AwsSetATCNMax(intptr_t sk, iAwsSource *source);
  static void AwsSetATCNWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYMin(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYMax(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetATSTCenterX(intptr_t sk, iAwsSource *source);
  static void AwsSetATSTCenterY(intptr_t sk, iAwsSource *source);
  static void AwsSetATSTCenterZ(intptr_t sk, iAwsSource *source);
  static void AwsSetATSTMin(intptr_t sk, iAwsSource *source);
  static void AwsSetATSTMax(intptr_t sk, iAwsSource *source);
  static void AwsSetATSTWeight(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTStartX(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTStartY(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTStartZ(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTEndX(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTEndY(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTEndZ(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTMin(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTMax(intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTWeight(intptr_t sk, iAwsSource *source);



  static void RegisterFSWindow(intptr_t sk, iAwsSource *source);
  static void RegisterFSLabel(intptr_t sk, iAwsSource *source);
  static void RegisterFSTextBox(intptr_t sk, iAwsSource *source);
  static void RegisterFSScrollBar(intptr_t sk, iAwsSource *source);
  static void AwsSetFSTextBox(intptr_t sk, iAwsSource *source);
  static void AwsSetFSScrollBar(intptr_t sk, iAwsSource *source);

  static void FreeScrollSetComponent(bool floatval,intptr_t value_pointer,iAwsComponent *associated,bool *invalidate_pointer);

  /// Static callback to handle Load/Save option window registration during .def file parsing.
  static void RegisterLoadSave(intptr_t sk, iAwsSource *source);
  /// Static callback to handle Aging Moments option window registration during .def file parsing.
  static void RegisterAgingMoments(intptr_t sk, iAwsSource *source);


public:
  PartEditSink();
  ~PartEditSink();

  void SetSink(iAwsSink *s);
  iAwsSink *GetSink() { return sink; }

  void SetWindowManager(iAws *_wmgr);

 

  void FillGraphicFileList();

  const char *GetGraphicFile();
  void SetGraphicFile(const char *filestr);
  const char *GetGraphicFilter();
  void SetGraphicFilter(const char *filterstr);
  const char *GetGraphicCWD();
  void SetGraphicCWD(const char *cwd);


  bool EmitterStateChanged();
  void ClearEmitterStateChanged();
  EmitterState *GetEmitterState();
  void SetEmitterState(EmitterState *source);
  void UpdateEmitterStateDisplay();

  bool InitialPositionStateChanged();
  void ClearInitialPositionStateChanged();
  Emitter3DState *GetInitialPositionState();
  void SetInitialPositionState(Emitter3DState *source);
  void UpdateInitialPositionStateDisplay();

  bool InitialSpeedStateChanged();
  void ClearInitialSpeedStateChanged();
  Emitter3DState *GetInitialSpeedState();
  void SetInitialSpeedState(Emitter3DState *source);
  void UpdateInitialSpeedStateDisplay();

  bool InitialAccelerationStateChanged();
  void ClearInitialAccelerationStateChanged();
  Emitter3DState *GetInitialAccelerationState();
  void SetInitialAccelerationState(Emitter3DState *source);
  void UpdateInitialAccelerationStateDisplay();


  bool FieldSpeedStateChanged();
  void ClearFieldSpeedStateChanged();
  FieldState *GetFieldSpeedState();
  void SetFieldSpeedState(FieldState *source);
  void UpdateFieldSpeedStateDisplay();

  bool FieldAccelStateChanged();
  void ClearFieldAccelStateChanged();
  FieldState *GetFieldAccelState();
  void SetFieldAccelState(FieldState *source);
  void UpdateFieldAccelStateDisplay();

  bool AttractorStateChanged();
  void ClearAttractorStateChanged();
  AttractorState *GetAttractorState();
  void SetAttractorState(AttractorState *source);
  void UpdateAttractorStateDisplay();


  bool LoadSaveStateChanged() { return LoadSaveData.settings_changed; };
  void ClearLoadSaveStateChanged() { LoadSaveData.settings_changed=false; };
  LoadSaveFileState *GetLoadSaveState() { return &(LoadSaveData.state); } ;
//  void SetLoadSaveState(LoadSaveFileState *source);
//  void UpdateLoadSaveStateDisplay();


  void SetVFS(csRef<iVFS> newvfs);
  csRef<iVFS> GetVFS();

};

#endif // __AWS_SINK_TEST_H__
