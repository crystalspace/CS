/*
   This is the same algorithm contained in polyclip.inc but it is optimized
   for clipiing against a square rectangle (box).
*/

  UByte ClipCount = 0;

#ifdef BOXCLIP_HAVEBOX
  // Decide against which edges we have to clip
  UByte ClipEdges = 0;
  // Do we need to clip against left x line?
  if (BoundingBox.MinX () < region.MinX ())
    ClipEdges |= 1, ClipCount++;
  // Do we need to clip against right x line?
  if (BoundingBox.MaxX () > region.MaxX ())
    ClipEdges |= 2, ClipCount++;
  // Do we need to clip against bottom y line?
  if (BoundingBox.MinY () < region.MinY ())
    ClipEdges |= 4, ClipCount++;
  // Do we need to clip against top y line?
  if (BoundingBox.MaxY () > region.MaxY ())
    ClipEdges |= 8, ClipCount++;
#endif

  // Temporary storage for intermediate polygons
  csVector2 TempPoly [MAX_OUTPUT_VERTICES];
  // The currently clipped polygon
  csVector2 *InP = InPolygon;
  // Number of input vertices
  int InV = InCount;
  // The current output polygon
  csVector2 *OutP = (ClipCount & 1) ? OutPolygon : TempPoly;
  // Number of output vertices
  int OutV = -1;
  // Current input vertex
  int vert;
  // Flag: was input polygon clipped?
  bool Clipped = false;

#ifdef OUTPUT_VERTEX_STATUS
  // Temporary storage for per-vertex status
  csVertexStatus TempStatus [MAX_OUTPUT_VERTICES];
  // A pointer to output array with vertex status
  csVertexStatus *InS = (ClipCount & 1) ? TempStatus : OutStatus;
  csVertexStatus *OutS = (ClipCount & 1) ? OutStatus : TempStatus;
  // Fill the input vertex status array
  for (int vs = 0; vs < InCount; vs++)
  {
    InS [vs].Type = CS_VERTEX_ORIGINAL;
    InS [vs].Vertex = vs;
  }
#endif

#ifdef BOXCLIP_HAVEBOX
  // Do we need to clip against left x line?
  if (ClipEdges & 1)
#endif
#define BOXCLIP_INSIDE(x,y)			\
  (x >= region.MinX ())
#define BOXCLIP_INTERSECT			\
  /* t = (region.X - px) / (cx - px) */		\
  t = (region.MinX () - px) / (cx - px);	\
  tx = region.MinX ();				\
  if (t <= 0)					\
    ty = py;					\
  else if (t >= 1)				\
    ty = cy;					\
  else						\
    ty = py + t * (cy - py);
#include "edgeclip.inc"

#ifdef BOXCLIP_HAVEBOX
  // Do we need to clip against right x line?
  if (ClipEdges & 2)
#endif
#define BOXCLIP_INSIDE(x,y)			\
  (x <= region.MaxX ())
#define BOXCLIP_INTERSECT			\
  /* t = (region.X - px) / (cx - px) */		\
  t = (region.MaxX () - px) / (cx - px);	\
  tx = region.MaxX ();				\
  if (t <= 0)					\
    ty = py;					\
  else if (t >= 1)				\
    ty = cy;					\
  else						\
    ty = py + t * (cy - py);
#include "edgeclip.inc"

#ifdef BOXCLIP_HAVEBOX
  // Do we need to clip against bottom y line?
  if (ClipEdges & 4)
#endif
#define BOXCLIP_INSIDE(x,y)			\
  (y >= region.MinY ())
#define BOXCLIP_INTERSECT			\
  /* t = (region.Y - py) / (cy - py) */		\
  t = (region.MinY () - py) / (cy - py);	\
  ty = region.MinY ();				\
  if (t <= 0)					\
    tx = px;					\
  else if (t >= 1)				\
    tx = cx;					\
  else						\
    tx = px + t * (cx - px);
#include "edgeclip.inc"

#ifdef BOXCLIP_HAVEBOX
  // Do we need to clip against top y line?
  if (ClipEdges & 8)
#endif
#define BOXCLIP_INSIDE(x,y)			\
  (y <= region.MaxY ())
#define BOXCLIP_INTERSECT			\
  /* t = (region.Y - py) / (cy - py) */		\
  t = (region.MaxY () - py) / (cy - py);	\
  ty = region.MaxY ();				\
  if (t <= 0)					\
    tx = px;					\
  else if (t >= 1)				\
    tx = cx;					\
  else						\
    tx = px + t * (cx - px);
#include "edgeclip.inc"

  // The polygon is fully inside clipper polygon?
  if (OutV == -1)
  {
    OutV = InV;
    if (InP != OutPolygon)
      memcpy (OutPolygon, InP, OutV * sizeof (csVector2));
  }

//<debug>
else if (OutV > 2 && InP != OutPolygon)
asm("int $3");
//</debug>

#undef BOXCLIP_HAVEBOX
#undef OUTPUT_VERTEX_STATUS
