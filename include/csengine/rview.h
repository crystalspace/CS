/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef RVIEW_H
#define RVIEW_H

#include "csgeom/math3d.h"
#include "csgeom/frustrum.h"
#include "csengine/camera.h"

class csClipper;
class csMatrix3;
class csVector3;
class csLight;
class csPolygon3D;
class csRenderView;
class csLightView;
struct csFog;
interface IGraphics3D;
interface IGraphics2D;

/// A callback function for csWorld::DrawFunc().
typedef void (csDrawFunc) (csRenderView* rview, int type, void* entity);

/// A callback function for csLight::LightingFunc().
typedef void (csLightingFunc) (csLightView* lview, int type, void* entity);

/**
 * Flags for the callbacks called via csWorld::DrawFunc() or
 * csLight::LightingFunc().
 * (type csDrawFunc or csLightingFunc).
 */
#define CALLBACK_POLYGON 1
#define CALLBACK_POLYGON2D 2
#define CALLBACK_POLYGONQ 3
#define CALLBACK_SECTOR 4
#define CALLBACK_SECTOREXIT 5
#define CALLBACK_THING 6
#define CALLBACK_THINGEXIT 7

/**
 * Information for vertex based fog. There is an instance of this
 * structure in the csRenderView struct for every fogged sector that
 * we encounter. It contains information which allows us to calculate
 * the thickness of the fog for any given ray through the incoming
 * and outgoing portals of the sector.
 */
class csFogInfo
{
public:
  /// Next in list (back in recursion time).
  csFogInfo* next;

  /// The incoming plane (plane of the portal).
  csPlane incoming_plane;
  /// The outgoing plane (also of a portal).
  csPlane outgoing_plane;
  /// If this is false then there is no incoming plane (the current sector has fog).
  bool has_incoming_plane;

  /// The structure describing the fog.
  csFog* fog;
};

/**
 * This structure represents all information needed for drawing
 * a scene. It is closely related to the csCamera class and modified
 * while rendering according to portals/warping portals and such.
 */
class csRenderView : public csCamera
{
public:
  /// The 2D polygon describing how everything drawn inside should be clipped.
  csClipper* view;

  /// The 3D graphics subsystem used for drawing.
  IGraphics3D* g3d;
  /// The 2D graphics subsystem used for drawing.
  IGraphics2D* g2d;

  /**
   * The portal polygon (or NULL if the first sector).
   */
  csPolygon3D* portal_polygon;

  /**
   * This variable holds the plane of the portal through which the camera
   * is looking.
   */
  csPlane clip_plane;

  /**
   * If true then we clip all objects to 'clip_plane'. In principle
   * one should always clip to 'clip_plane'. However, in many cases
   * this is not required because portals mostly arrive in at the
   * boundaries of a sector so there can actually be no objects
   * after the portal plane. But it is possible that portals arive
   * somewhere in the middle of a sector (for example with BSP sectors
   * or with Things containing portals). In that case this variable
   * will be set to true and clipping to 'clip_plane' is required.
   */
  bool do_clip_plane;

  /**
   * If true then we have to clip all objects to the portal frustrum
   * (either in 2D or 3D). Normally this is not needed but some portals
   * require this. If do_clip_plane is true then the value of this
   * field is also implied to be true. The top-level portal should
   * set do_clip_frustrum to true in order for all geometry to be
   * correctly clipped to screen boundaries.
   */
  bool do_clip_frustrum;

  /**
   * A callback function. If this is set then no drawing is done.
   * Instead the callback function is called.
   */
  csDrawFunc* callback;

  /// Userdata belonging to the callback.
  void* callback_data;

  /**
   * Every fogged sector we encountered results in an extra structure in the
   * following list. This is only used if we are doing vertex based fog.
   */
  csFogInfo* fog_info;

  /**
   * If the following variable is true then a fog_info was added in this
   * recursion level.
   */
  bool added_fog_info;

  ///
  csRenderView () : csCamera (), view (NULL), g3d (NULL), g2d (NULL),
  	portal_polygon (NULL), do_clip_plane (false), do_clip_frustrum (false),
	callback (NULL), callback_data (NULL),
	fog_info (NULL), added_fog_info (false) {}
  ///
  csRenderView (const csCamera& c) : csCamera (c), view (NULL), g3d (NULL), g2d (NULL),
  	portal_polygon (NULL), do_clip_plane (false), do_clip_frustrum (false),
	callback (NULL), callback_data (NULL),
	fog_info (NULL), added_fog_info (false) {}
  ///
  csRenderView (const csCamera& c, csClipper* v, IGraphics3D* ig3d, IGraphics2D* ig2d) :
	csCamera (c), view (v), g3d (ig3d), g2d (ig2d),
	portal_polygon (NULL), do_clip_plane (false), do_clip_frustrum (false),
	callback (NULL), callback_data (NULL),
	fog_info (NULL), added_fog_info (false) {}

  ///
  void SetView (csClipper* v) { view = v; }
  ///
  void SetClipPlane (csPlane& p) { clip_plane = p; }
};

/**
 * This class is a csFrustrum especially used for the lighting calculations.
 * It represents a shadow. It extends csFrustrum by adding 'next' and 'prev' for
 * living in a linked list and it adds the 'polygon' member so that we can find
 * for which polygon this frustrum was generated.
 */
class csShadowFrustrum : public csFrustrum
{
public:
  /// Linked list.
  csShadowFrustrum* next, * prev;

  /// Polygon which generated this shadow.
  csPolygon3D* polygon;

  /**
   * If true then this frustrum is relevant. This is
   * a temporary variable which is used during the lighting
   * calculation process. It may change value several times during
   * the life time of a shadow frustrum.
   */
  bool relevant;

public:
  /// Create empty frustrum.
  csShadowFrustrum (csVector3& origin) : csFrustrum (origin), next (NULL), prev (NULL), polygon (NULL) { }
};

/**
 * A list of frustrums.
 */
class csFrustrumList
{
private:
  csShadowFrustrum* first, * last;

public:
  /// Create an empty list.
  csFrustrumList () : first (NULL), last (NULL) { }

  /// Destroy the list but do not destroy the individual elements!
  virtual ~csFrustrumList () { }

  /// Destroy all frustrums in the list.
  void DeleteFrustrums ()
  {
    csShadowFrustrum* sf;
    while (first)
    {
      sf = first->next;
      CHK (delete first);
      first = sf;
    }
    last = NULL;
  }

  /// Clear the list (make empty but don't delete elements).
  void Clear () { first = last = NULL; }

  /// Get the first element in this list (or NULL if empty).
  csShadowFrustrum* GetFirst () { return first; }

  /// Get the last element in this list (or NULL if empty).
  csShadowFrustrum* GetLast () { return last; }

  /**
   * Append a list to this one. Note that you
   * should not do any modifications on the other list
   * after this is done.
   */
  void AppendList (csFrustrumList* list)
  {
    if (last)
    {
      last->next = list->GetFirst ();
      if (list->GetFirst ()) list->GetFirst ()->prev = last;
      if (list->GetLast ()) last = list->GetLast ();
    }
    else
    {
      first = list->GetFirst ();
      last = list->GetLast ();
    }
  }

  /**
   * Set the last element in this list. This basicly has
   * the effect of truncating the list to some specific element.
   * Note that this function only works if the frustrum is actually
   * part of the list. No checking is done. The elements which
   * are clipped of the list are unchanged (not deleted). You
   * can relink or delete them if you want. If the given frustrum
   * is NULL then this function has the same effect as making
   * the list empty.
   */
  void SetLast (csShadowFrustrum* frust)
  {
    if (frust)
    {
      frust->next = NULL;
      last = frust;
    }
    else { first = last = NULL; }
  }

  /// Add a new frustrum to the front of the list.
  void AddFirst (csShadowFrustrum* fr)
  {
    fr->prev = NULL;
    fr->next = first;
    if (first) first->prev = fr;
    first = fr;
    if (!last) last = fr;
  }

  /// Add a new frustrum to the back of the list.
  void AddLast (csShadowFrustrum* fr)
  {
    fr->next = NULL;
    fr->prev = last;
    if (last) last->next = fr;
    last = fr;
    if (!first) first = fr;
  }

  /// Unlink a shadow frustrum from the list.
  void Unlink (csShadowFrustrum* sf)
  {
    if (sf->next) sf->next->prev = sf->prev;
    else last = sf->prev;
    if (sf->prev) sf->prev->next = sf->next;
    else first = sf->next;
  }

  /**
   * Apply a transformation to all frustrums in this list.
   */
  void Transform (csTransform* trans)
  {
    csShadowFrustrum* sf = first;
    while (sf)
    {
      sf->Transform (trans);
      sf = sf->next;
    }
  }
};

/**
 * This structure represents all information needed for static lighting.
 * It is the basic information block that is passed between the various
 * static lighting routines.
 */
class csLightView
{
public:
  /// The light that we're processing.
  csLight* l;

  /**
   * The current color of the light. Initially this is the same as the
   * light in csStatLight but portals may change this.
   */
  float r, g, b;

  /// If space is mirrored.
  bool mirror;

  /**
   * If this structure is used for dynamic light frustrum calculation
   * then this flag is true.
   */
  bool dynamic;

  /**
   * If only gouraud shading should be updated then this flag is true.
   */
  bool gouraud_only;

  /**
   * If 'true' then the gouraud vertices need to be initialized (set to
   * black) first. Only the parent PolygonSet of a polygon can know this
   * because it is calculated using the current_light_frame_number.
   */
  bool gouraud_color_reset;

  /**
   * The frustrum for the light. Everthing that falls in this frustrum
   * is lit unless it also is in a shadow frustrum.
   */
  csFrustrum* light_frustrum;

  /**
   * The list of shadow frustrums. Note that this list will be
   * expanded with every traversal through a portal but it needs
   * to be restored to original state again before returning.
   */
  csFrustrumList shadows;

  /**
   * A callback function. If this is set then no actual
   * lighting is done.
   * Instead the callback function is called.
   */
  csLightingFunc* callback;

  /// Userdata belonging to the callback.
  void* callback_data;

public:
  ///
  csLightView () : light_frustrum (NULL), callback (NULL), callback_data (NULL) { }
 
  ///
  ~csLightView ()
  {
    CHK (delete light_frustrum);
  }
};


#endif /*RVIEW_H*/

