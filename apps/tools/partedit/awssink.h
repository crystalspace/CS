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
  static void RegisterSectionSelection(unsigned long, intptr_t sk, iAwsSource *source);

  /// Static callback to handle left sidebar list box registration during .def file parsing.
  static void FillSectionList(unsigned long, intptr_t sk, iAwsSource *source);

  /// Static callback to handle selection/state changes in the section list box
  static void SectionListSelectionChanged(unsigned long, intptr_t sk, iAwsSource *source);



  /// Static callback to handle graphic file selection window registration during .def file parsing.
  static void RegisterGraphicSelection(unsigned long, intptr_t sk, iAwsSource *source);
  /// Static callback to handle graphic file filter text box registration during .def file parsing.
  static void RegisterGraphicFilter(unsigned long, intptr_t sk, iAwsSource *source);
  /// Static callback to handle graphic file list registration during .def file parsing
  static void RegisterGraphicFileList(unsigned long, intptr_t sk, iAwsSource *source);
  /// Static callback to handle graphic file filter change.
  static void AwsSetGraphicFilter(unsigned long, intptr_t sk, iAwsSource *source);
  /// Static callback to handle graphic file selection change
  static void AwsGraphicFileSelected(unsigned long, intptr_t sk, iAwsSource *source);


  /// Static callback to handle emitter state option window registration during .def file parsing.
  static void RegisterEmitterState(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterParticleCount(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterParticleMaxAge(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterLighting(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterAlphaBlend(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterRectParticlesRadio(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterRegParticlesRadio(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterRectParticlesWidth(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterRectParticlesHeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterRegParticlesNumber(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterRegParticlesRadius(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterUseBoundingBox(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterBBoxMinX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterBBoxMinY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterBBoxMinZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterBBoxMaxX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterBBoxMaxY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterBBoxMaxZ(unsigned long, intptr_t sk, iAwsSource *source);

  static void AwsSetParticleCount(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetParticleMaxAge(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetLighting(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetAlphaBlend(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetParticleType(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetRectangularWidth(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetRectangularHeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetRegularNumber(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetRegularRadius(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetUseBoundingBox(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetBBoxMinX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetBBoxMinY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetBBoxMinZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetBBoxMaxX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetBBoxMaxY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetBBoxMaxZ(unsigned long, intptr_t sk, iAwsSource *source);

  /// Static callback to handle initial position option window registration during .def file parsing.
  static void RegisterInitialPosition(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPFPX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPFPY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPFPZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPFWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPLSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPLSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPLSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPLEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPLEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPLEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPLWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPBMX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPBMY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPBMZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPBXX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPBXY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPBXZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPBWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPSCX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPSCY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPSCZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPSMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPSMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPSWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCNOX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCNOY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCNOZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCNElev(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCNAzim(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCNAper(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCNMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCNMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCNWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPSTCX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPSTCY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPSTCZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPSTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPSTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPSTWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIPCYTWeight(unsigned long, intptr_t sk, iAwsSource *source);

  static void AwsSetIPFPositionX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPFPositionY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPFPositionZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPFWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPLStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPLStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPLStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPLEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPLEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPLEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPLWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPBMinX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPBMinY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPBMinZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPBMaxX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPBMaxY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPBMaxZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPBWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPSCenterX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPSCenterY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPSCenterZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPSMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPSMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPSWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNOriginX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNOriginY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNOriginZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNElev(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNAzim(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNAper(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCNWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPSTCenterX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPSTCenterY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPSTCenterZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPSTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPSTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPSTWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIPCYTWeight(unsigned long, intptr_t sk, iAwsSource *source);


  /// Static callback to handle initial speed option window registration during .def file parsing.
  static void RegisterInitialSpeed(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISFPX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISFPY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISFPZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISFWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISLSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISLSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISLSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISLEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISLEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISLEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISLWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISBMX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISBMY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISBMZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISBXX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISBXY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISBXZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISBWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISSCX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISSCY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISSCZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISSMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISSMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISSWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCNOX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCNOY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCNOZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCNElev(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCNAzim(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCNAper(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCNMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCNMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCNWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISSTCX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISSTCY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISSTCZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISSTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISSTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISSTWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYTSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYTSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYTSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYTEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYTEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYTEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterISCYTWeight(unsigned long, intptr_t sk, iAwsSource *source);

  static void AwsSetISFPositionX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISFPositionY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISFPositionZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISFWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISLStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISLStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISLStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISLEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISLEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISLEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISLWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISBMinX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISBMinY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISBMinZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISBMaxX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISBMaxY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISBMaxZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISBWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISSCenterX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISSCenterY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISSCenterZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISSMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISSMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISSWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCNOriginX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCNOriginY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCNOriginZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCNElev(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCNAzim(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCNAper(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCNMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCNMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCNWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISSTCenterX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISSTCenterY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISSTCenterZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISSTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISSTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISSTWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetISCYTWeight(unsigned long, intptr_t sk, iAwsSource *source);




  /// Static callback to handle initial acceleration option window registration during .def file parsing.
  static void RegisterInitialAcceleration(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIAFPX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIAFPY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIAFPZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIAFWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIALSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIALSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIALSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIALEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIALEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIALEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIALWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIABMX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIABMY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIABMZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIABXX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIABXY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIABXZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIABWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIASCX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIASCY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIASCZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIASMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIASMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIASWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACNOX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACNOY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACNOZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACNElev(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACNAzim(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACNAper(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACNMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACNMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACNWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIASTCX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIASTCY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIASTCZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIASTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIASTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIASTWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYTSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYTSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYTSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYTEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYTEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYTEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterIACYTWeight(unsigned long, intptr_t sk, iAwsSource *source);

  static void AwsSetIAFPositionX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIAFPositionY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIAFPositionZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIAFWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIALStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIALStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIALStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIALEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIALEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIALEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIALWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIABMinX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIABMinY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIABMinZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIABMaxX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIABMaxY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIABMaxZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIABWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIASCenterX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIASCenterY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIASCenterZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIASMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIASMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIASWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACNOriginX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACNOriginY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACNOriginZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACNElev(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACNAzim(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACNAper(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACNMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACNMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACNWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIASTCenterX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIASTCenterY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIASTCenterZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIASTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIASTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIASTWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetIACYTWeight(unsigned long, intptr_t sk, iAwsSource *source);



  /// Static callback to handle field speed option window registration during .def file parsing.
  static void RegisterFieldSpeed(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSActive(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSFPX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSFPY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSFPZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSFWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSLSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSLSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSLSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSLEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSLEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSLEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSLWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSBMX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSBMY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSBMZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSBXX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSBXY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSBXZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSBWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSSCX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSSCY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSSCZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSSMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSSMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSSWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCNOX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCNOY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCNOZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCNElev(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCNAzim(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCNAper(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCNMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCNMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCNWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSSTCX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSSTCY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSSTCZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSSTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSSTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSSTWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSCYTWeight(unsigned long, intptr_t sk, iAwsSource *source);

  static void AwsSetFSActive(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSFPositionX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSFPositionY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSFPositionZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSFWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSLStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSLStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSLStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSLEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSLEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSLEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSLWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSBMinX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSBMinY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSBMinZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSBMaxX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSBMaxY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSBMaxZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSBWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSSCenterX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSSCenterY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSSCenterZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSSMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSSMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSSWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNOriginX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNOriginY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNOriginZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNElev(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNAzim(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNAper(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCNWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSSTCenterX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSSTCenterY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSSTCenterZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSSTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSSTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSSTWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSCYTWeight(unsigned long, intptr_t sk, iAwsSource *source);


  /// Static callback to handle field acceleration option window registration during .def file parsing.
  static void RegisterFieldAccel(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFAActive(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFAFPX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFAFPY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFAFPZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFAFWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFALSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFALSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFALSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFALEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFALEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFALEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFALWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFABMX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFABMY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFABMZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFABXX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFABXY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFABXZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFABWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFASCX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFASCY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFASCZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFASMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFASMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFASWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACNOX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACNOY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACNOZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACNElev(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACNAzim(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACNAper(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACNMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACNMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACNWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFASTCX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFASTCY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFASTCZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFASTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFASTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFASTWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYTSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYTSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYTSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYTEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYTEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYTEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFACYTWeight(unsigned long, intptr_t sk, iAwsSource *source);

  static void AwsSetFAActive(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFAFPositionX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFAFPositionY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFAFPositionZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFAFWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFALStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFALStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFALStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFALEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFALEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFALEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFALWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFABMinX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFABMinY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFABMinZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFABMaxX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFABMaxY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFABMaxZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFABWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFASCenterX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFASCenterY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFASCenterZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFASMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFASMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFASWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACNOriginX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACNOriginY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACNOriginZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACNElev(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACNAzim(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACNAper(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACNMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACNMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACNWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFASTCenterX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFASTCenterY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFASTCenterZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFASTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFASTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFASTWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFACYTWeight(unsigned long, intptr_t sk, iAwsSource *source);





  /// Static callback to handle attractor option window registration during .def file parsing.
  static void RegisterAttractor(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATForce(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATFPX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATFPY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATFPZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATFWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATLSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATLSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATLSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATLEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATLEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATLEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATLWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATBMX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATBMY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATBMZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATBXX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATBXY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATBXZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATBWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATSCX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATSCY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATSCZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATSMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATSMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATSWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCNOX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCNOY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCNOZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCNElev(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCNAzim(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCNAper(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCNMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCNMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCNWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATSTCX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATSTCY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATSTCZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATSTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATSTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATSTWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYTSX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYTSY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYTSZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYTEX(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYTEY(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYTEZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterATCYTWeight(unsigned long, intptr_t sk, iAwsSource *source);


  static void AwsSetATForce(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATFPositionX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATFPositionY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATFPositionZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATFWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATLStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATLStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATLStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATLEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATLEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATLEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATLWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATBMinX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATBMinY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATBMinZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATBMaxX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATBMaxY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATBMaxZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATBWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATSCenterX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATSCenterY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATSCenterZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATSMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATSMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATSWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCNOriginX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCNOriginY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCNOriginZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCNElev(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCNAzim(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCNAper(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCNMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCNMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCNWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATSTCenterX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATSTCenterY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATSTCenterZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATSTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATSTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATSTWeight(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTStartX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTStartY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTStartZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTEndX(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTEndY(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTEndZ(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTMin(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTMax(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetATCYTWeight(unsigned long, intptr_t sk, iAwsSource *source);



  static void RegisterFSWindow(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSLabel(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSTextBox(unsigned long, intptr_t sk, iAwsSource *source);
  static void RegisterFSScrollBar(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSTextBox(unsigned long, intptr_t sk, iAwsSource *source);
  static void AwsSetFSScrollBar(unsigned long, intptr_t sk, iAwsSource *source);

  static void FreeScrollSetComponent(bool floatval,intptr_t value_pointer,iAwsComponent *associated,bool *invalidate_pointer);

  /// Static callback to handle Load/Save option window registration during .def file parsing.
  static void RegisterLoadSave(unsigned long, intptr_t sk, iAwsSource *source);
  /// Static callback to handle Aging Moments option window registration during .def file parsing.
  static void RegisterAgingMoments(unsigned long, intptr_t sk, iAwsSource *source);


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
