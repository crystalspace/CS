/*
 *  LodGen.cpp
 *  cs
 *
 *  Created by Eduardo Poyart on 6/14/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <iostream>
using namespace std;

#include "csgeom.h"
#include "LodGen.h"

inline float dot(const csVector3& v0, const csVector3& v1) { return v0 * v1; }

/*
inline void PointTriangleDistance26(float aa, float bb, float cc, float dd, float ee, float ff, float& ss, float& tt, float& dd2)
{
  float tmp0 = bb+dd;
  float tmp1 = cc+ee;
  if (tmp1 > tmp0)
  {
    float numer = tmp1-tmp0;
    float denom = aa - 2.0*bb + cc;
    if (numer >= denom)
    {
      ss = 1.0;
      tt = 0.0;
      //dd2 = aa + 2.0*bb + ff;
      dd2 = aa + 2.0*dd + ff;
    }
    else
    {
      ss = numer/denom;
      tt = 1.0-ss;
      dd2 = ss*(aa*ss + bb*tt + 2.0*dd) + tt*(bb*ss + cc*tt + 2.0*ee) + ff;
    }
  }
  else
  {
    ss = 0.0;
    if (tmp1 <= 0.0)
    {
      tt = 1.0;
      dd2 = cc + 2.0*ee + ff;
    }
    else
    {
      if (ee >= 0.0)
      {
        tt = 0.0;
        dd2 = ff;
      }
      else
      {
        tt = -ee/cc;
        dd2 = ee*tt + ff;
      }
    }
  }  
}

void PointTriangleDistance(const csVector3& P, const csVector3& P0, const csVector3& P1, const csVector3& P2, float& s, float& t, float& d2)
{
  const csVector3& B = P0;
  csVector3 E0 = P1 - P0;
  csVector3 E1 = P2 - P0;
  csVector3 D = B - P;
  float a = dot(E0, E0);
  float b = dot(E0, E1);
  float c = dot(E1, E1);
  float d = dot(E0, D);  
  float e = dot(E1, D);
  float f = dot(D, D);
  
  float det = a*c - b*b;
  s = b*e - c*d;
  t = b*d - a*e;
  
  if (s+t <= det)
  {
    if (s < 0.0)
    {
      if (t < 0.0)
      {
        // region 4
        if (d < 0.0)
        {
          t = 0.0;
          if (-d >= a)
          {
            s = 1.0;
            d2 = a + 2.0*d + f;
            return;
          }
          else
          {
            s = -d/a;
            d2 = d*s + f;
            return;
          }
        }
        else
        {
          s = 0.0;
          if (e >= 0.0)
          {
            t = 0.0;
            d2 = f;
            return;
          }
          else
          {
            if (-e >= c)
            {
              t = 1.0;
              d2 = c + 2.0*e + f;
              return;
            }
            else
            {
              t = -e/c;
              d2 = e*t + f;
              return;
            }
          }
        }
      }
      else 
      { 
        // region 3
        s = 0.0;
        if (e >= 0.0)
        {
          t = 0.0;
          d2 = f;
          return;
        }
        else
        {
          if (-e >= c)
          {
            t = 1.0;
            d2 = c + 2.0*e + f;
            return;
          }
          else
          {
            t = -e/c;
            d2 = e*t + f;
            return;
          }
        }
      }
    }
    else if (t < 0.0) 
    {
      // region 5
      t = 0.0;
      if (d >= 0.0)
      {
        s = 0.0;
        d2 = f;
        return;
      }
      else
      {
        if (-d >= a)
        {
          s = 1.0;
          d2 = a + d*s + f;
          return;
        }
        else
        {
          s = -d/a;
          d2 = d*s + f;
          return;
        }
      }
    }
    else 
    {
      // region 0
      float invdet = 1.0 / det;
      s *= invdet;
      t *= invdet;
      d2 = s*(a*s + b*t + 2.0*d) + t*(b*s + c*t + 2.0*e) + f;
      return;
    }
  }
  else
  {
    if (s < 0.0)
    {      
      // region 2
      PointTriangleDistance26(a, b, c, d, e, f, s, t, d2);
      return;
    }
    else if (t < 0.0)
    {
      // region 6
      PointTriangleDistance26(c, b, a, e, d, f, t, s, d2);
      return;
    }
    else 
    {
      // region 1
      float numer = c+e-b-d;
      if (numer <= 0.0)
      {
        s = 0.0;
        t = 1.0;
        d2 = c + 2.0*e + f;
      }
      else
      {
        float denom = a - 2.0*b + c;
        if (numer >= denom)
        {
          s = 1.0;
          t = 0.0;
          d2 = a + 2.0*d + f;
        }
        else
        {
          s = numer/denom;
          t = 1.0 - s;
          d2 = s*(a*s + b*t + 2.0*d) + t*(b*s + c*t + 2.0*e) + f;
        }
      }
    }
  }
}

*/

void PointTriangleDistance(const csVector3& P, const csVector3& P0, const csVector3& P1, const csVector3& P2, float& s, float& t, float& d2)
{
  // From http://www.geometrictools.com/Documentation/DistancePoint3Triangle3.pdf
  csVector3 diff = P0 - P;
  csVector3 edge0 = P1 - P0;
  csVector3 edge1 = P2 - P0;
  float a00 = dot(edge0, edge0);
  float a01 = dot(edge0, edge1);
  float a11 = dot(edge1, edge1);
  float b0 = dot(diff, edge0);
  float b1 = dot(diff, edge1);
  float c = dot(diff, diff);
  float det = fabs(a00*a11 - a01*a01);
  s = a01*b1 - a11*b0;
  t = a01*b0 - a00*b1;
  
  if (s + t <= det)
  {
    if (s < 0.0)
    {
      if (t < 0.0)  // region 4
      {
        if (b0 < 0.0)
        {
          t = 0.0;
          if (-b0 >= a00)
          {
            s = 1.0;
            d2 = a00 + 2.0*b0 + c;
          }
          else
          {
            s = -b0/a00;
            d2 = b0*s + c;
          }
        }
        else
        {
          s = 0.0;
          if (b1 >= 0.0)
          {
            t = 0.0;
            d2 = c;
          }
          else if (-b1 >= a11)
          {
            t = 1.0;
            d2 = a11 + 2.0*b1 + c;
          }
          else
          {
            t = -b1/a11;
            d2 = b1*t + c;
          }
        }
      }
      else  // region 3
      {
        s = 0.0;
        if (b1 >= 0.0)
        {
          t = 0.0;
          d2 = c;
        }
        else if (-b1 >= a11)
        {
          t = 1.0;
          d2 = a11 + 2.0*b1 + c;
        }
        else
        {
          t = -b1/a11;
          d2 = b1*t + c;
        }
      }
    }
    else if (t < 0.0)  // region 5
    {
      t = 0.0;
      if (b0 >= 0.0)
      {
        s = 0.0;
        d2 = c;
      }
      else if (-b0 >= a00)
      {
        s = 1.0;
        d2 = a00 + 2.0*b0 + c;
      }
      else
      {
        s = -b0/a00;
        d2 = b0*s + c;
      }
    }
    else  // region 0
    {
      float invDet = 1.0/det;
      s *= invDet;
      t *= invDet;
      d2 = s*(a00*s + a01*t + 2.0*b0) + t*(a01*s + a11*t + 2.0*b1) + c;
    }
  }
  else
  {
    if (s < 0.0)  // region 2
    {
      float tmp0 = a01 + b0;
      float tmp1 = a11 + b1;
      if (tmp1 > tmp0)
      {
        float numer = tmp1 - tmp0;
        float denom = a00 - 2.0*a01 + a11;
        if (numer >= denom)
        {
          s = 1.0;
          t = 0.0;
          d2 = a00 + 2.0*b0 + c;
        }
        else
        {
          s = numer/denom;
          t = 1.0 - s;
          d2 = s*(a00*s + a01*t + 2.0*b0) + t*(a01*s + a11*t + 2.0*b1) + c;
        }
      }
      else
      {
        s = 0.0;
        if (tmp1 <= 0.0)
        {
          t = 1.0;
          d2 = a11 + 2.0*b1 + c;
        }
        else if (b1 >= 0.0)
        {
          t = 0.0;
          d2 = c;
        }
        else
        {
          t = -b1/a11;
          d2 = b1*t + c;
        }
      }
    }
    else if (t < 0.0)  // region 6
    {
      float tmp0 = a01 + b1;
      float tmp1 = a00 + b0;
      if (tmp1 > tmp0)
      {
        float numer = tmp1 - tmp0;
        float denom = a00 - 2.0*a01 + a11;
        if (numer >= denom)
        {
          t = 1.0;
          s = 0.0;
          d2 = a11 + 2.0*b1 + c;
        }
        else
        {
          t = numer/denom;
          s = 1.0 - t;
          d2 = s*(a00*s + a01*t + 2.0*b0) + t*(a01*s + a11*t + 2.0*b1) + c;
        }
      }
      else
      {
        t = 0.0;
        if (tmp1 <= 0.0)
        {
          s = 1.0;
          d2 = a00 + 2.0*b0 + c;
        }
        else if (b0 >= 0.0)
        {
          s = 0.0;
          d2 = c;
        }
        else
        {
          s = -b0/a00;
          d2 = b0*s + c;
        }
      }
    }
    else  // region 1
    {
      float numer = a11 + b1 - a01 - b0;
      if (numer <= 0.0)
      {
        s = 0.0;
        t = 1.0;
        d2 = a11 + 2.0*b1 + c;
      }
      else
      {
        float denom = a00 - 2.0*a01 + a11;
        if (numer >= denom)
        {
          s = 1.0;
          t = 0.0;
          d2 = a00 + 2.0*b0 + c;
        }
        else
        {
          s = numer/denom;
          t = 1.0 - s;
          d2 = s*(a00*s + a01*t + 2.0*b0) + t*(a01*s + a11*t + 2.0*b1) + c;
        }
      }
    }
  }
  
  // Account for numerical round-off error
  if (d2 < 0.0)
  {
    d2 = 0.0;
  }
}

void unittest1(const csVector3& p0, const csVector3& p1, const csVector3& p2, const csVector3& p, float expected)
{
  printf("p = %6.4g, %6.4g, %6.4g        ", p.x, p.y, p.z);
  float s, t, d2;
  PointTriangleDistance(p, p0, p1, p2, s, t, d2);
  csVector3 c(p0 + s*(p1-p0) + t*(p2-p0));
  float d = sqrtf(d2);
  printf("d = %6.4g       c = %6.4g, %6.4g, %6.4g\n", d, c.x, c.y, c.z);
  assert(expected == -1.0 || fabs(expected - d) < 0.0001);
}

void unittests(float z)
{
  csVector3 p0(1.0, 1.0, z);
  csVector3 p1(2.0, 2.0, z);
  csVector3 p2(3.0, 1.0, z);
  unittest1(p0, p1, p2, csVector3(1.0, 1.0, z), 0.0);
  unittest1(p0, p1, p2, csVector3(2.0, 2.0, z), 0.0);
  unittest1(p0, p1, p2, csVector3(3.0, 1.0, z), 0.0);
  unittest1(p0, p1, p2, csVector3(2.0, 1.5, z), 0.0);
  unittest1(p0, p1, p2, csVector3(1.0, 1.0, z + 1.0), 1.0);
  unittest1(p0, p1, p2, csVector3(2.0, 2.0, z + 1.0), 1.0);
  unittest1(p0, p1, p2, csVector3(3.0, 1.0, z + 1.0), 1.0);
  unittest1(p0, p1, p2, csVector3(2.0, 1.5, z + 1.0), 1.0);
  
  unittest1(p0, p1, p2, csVector3(0.0, 0.0, z), sqrtf(2.0)); 
  unittest1(p0, p1, p2, csVector3(0.5, 0.5, z), sqrtf(2.0) / 2.0); 
  unittest1(p0, p1, p2, csVector3(0.0, 1.0, z), 1.0); 
  unittest1(p0, p1, p2, csVector3(0.5, 1.0, z), 0.5);  
  unittest1(p0, p1, p2, csVector3(1.5, 1.5, z), 0.0); 
  unittest1(p0, p1, p2, csVector3(1.5, 2.0, z), 0.3536); 
  unittest1(p0, p1, p2, csVector3(2.0, 3.0, z), 1.0); 
  unittest1(p0, p1, p2, csVector3(2.5, 2.0, z), 0.3536); 
  unittest1(p0, p1, p2, csVector3(2.5, 1.5, z), 0.0); 
  unittest1(p0, p1, p2, csVector3(3.5, 1.0, z), 0.5);  
  unittest1(p0, p1, p2, csVector3(4.0, 1.0, z), 1.0); 
  unittest1(p0, p1, p2, csVector3(3.5, 0.5, z), sqrtf(2.0) / 2.0); 
  unittest1(p0, p1, p2, csVector3(4.0, 0.0, z), sqrtf(2.0)); 
  unittest1(p0, p1, p2, csVector3(2.5, 0.0, z), 1.0); 
  unittest1(p0, p1, p2, csVector3(2.0, 1.0, z), 0.0); 
  unittest1(p0, p1, p2, csVector3(1.5, 0.0, z), 1.0); 
}  

void PointTriangleDistanceUnitTests()
{
  unittests(0.0);
  unittests(1.0);
}

float LodGen::SumOfSquareDist(const WorkMesh& k) const
{
  float s, t, d2;
  float sum = 0.0;
  const SlidingWindow& sw = k.sliding_windows[k.sliding_windows.GetSize()-1];
  for (int i = 0; i < num_vertices; i++)
  {
    const csVector3& v = vertices[i];
    float min_d2 = FLT_MAX;
    for (int j = sw.start_index; j < sw.end_index; j++)
    {
      const csTriangle& tri = k.tri_buffer[k.tri_indices[j]];
      const csVector3& p0 = vertices[tri[0]];
      const csVector3& p1 = vertices[tri[1]];
      const csVector3& p2 = vertices[tri[2]];
      PointTriangleDistance(v, p0, p1, p2, s, t, d2);
      if (d2 < min_d2)
      {
        min_d2 = d2;
        if (min_d2 == 0.0)
          break;
      }
    }
    assert(min_d2 < FLT_MAX);
    sum += min_d2;
  }
  return sum;
}

void LodGen::AddTriangle(WorkMesh& k, int itri)
{
  csTriangle& tri = k.tri_buffer[itri];
  assert(k.tri_indices.Find(itri) == csArrayItemNotFound);
  k.tri_indices.Push(itri);
  for (int i = 0; i < 3; i++)
    k.incident_tris[tri[i]].PushSmart(itri);
}

void LodGen::RemoveTriangleFromIncidentTris(WorkMesh& k, int itri)
{
  csTriangle& tri = k.tri_buffer[itri];
  for (int i = 0; i < 3; i++)
  {
    bool result = k.incident_tris[tri[i]].Delete(itri);
    assert(result);
  }
}

inline bool LodGen::IsDegenerate(const csTriangle& tri) const
{
  return tri[0] == tri[1] || tri[0] == tri[2] || tri[1] == tri[2];
}

bool LodGen::IsTriangleCoincident(const csTriangle& t0, const csTriangle& t1) const
{
  for (int i = 0; i < 3; i++)
    if (t0[i] != t1[0] && t0[i] != t1[1] && t0[i] != t1[2])
      return false;
  return true;
}

bool LodGen::IsCoincident(const WorkMesh& k, const csTriangle& tri) const
{
  for (int i = 0; i < 3; i++)
  {
    const IncidentTris& incident = k.incident_tris[tri[i]];
    for (unsigned int j = 0; j < incident.GetSize(); j++)
    {
      if (IsTriangleCoincident(k.tri_buffer[incident[j]], tri))
        return true;
    }
  }
  return false;
}

int LodGen::FindInWindow(const WorkMesh& k, const SlidingWindow& sw, int itri) const
{
  for (int i = sw.start_index; i < sw.end_index; i++)
    if (k.tri_indices[i] == itri)
      return i;
  assert(0);
}

void LodGen::SwapIndex(WorkMesh& k, int i0, int i1)
{
  int temp = k.tri_indices[i0];
  k.tri_indices[i0] = k.tri_indices[i1];
  k.tri_indices[i1] = temp;
}

bool LodGen::Collapse(WorkMesh& k, int v0, int v1)
{
  SlidingWindow sw = k.sliding_windows[k.sliding_windows.GetSize()-1]; // copy
  sw.start_index += window_shift;
  sw.end_index += window_shift;
  
  IncidentTris incident = k.incident_tris[v0]; // copy
  for (unsigned int i = 0; i < incident.GetSize(); i++)
  {
    int itri = incident[i];
    int h = FindInWindow(k, sw, itri);
    if (h >= top_limit)
      return false;
    csTriangle new_tri = k.tri_buffer[itri]; // copy
    RemoveTriangleFromIncidentTris(k, itri);
    SwapIndex(k, sw.start_index, h);
    for (int j = 0; j < 3; j++)
      k.edges.Delete(Edge(new_tri[j], new_tri[(j+1)%3]));
    //cout << "Rem " << itri << " = " << new_tri[0] << " " << new_tri[1] << " " << new_tri[2] << endl;
    sw.start_index++;

    assert(incident.GetSize() > k.incident_tris[v0].GetSize());
    for (int j = 0; j < 3; j++)
      if (new_tri[j] == v0)
        new_tri[j] = v1;
    if (!IsDegenerate(new_tri) && !IsCoincident(k, new_tri))
    {
      k.tri_buffer.Push(new_tri);
      AddTriangle(k, k.tri_buffer.GetSize()-1);
      //cout << "Add " << k.tri_buffer.GetSize()-1 << " = " << new_tri[0] << " " << new_tri[1] << " " << new_tri[2] << endl;
      sw.end_index++;
    }
  }
  k.sliding_windows.Push(sw);

  /*
  for (int i = sw.start_index; i < sw.end_index; i++)
    for (int j = 0; j < 3; j++)
      assert(k.tri_buffer[k.tri_indices[i]][j] != v0);
  */
  return true;
}

struct MeshVerification
{
  Edge e;
  int num_t;
  //int t0;
  //int t1;
  bool operator==(const MeshVerification& m) const { return e == m.e; }
};

void LodGen::VerifyMesh(WorkMesh& k)
{
  csArray<MeshVerification> mvs;
  SlidingWindow& sw = k.sliding_windows[k.sliding_windows.GetSize()-1];
  for (int i = sw.start_index; i < sw.end_index; i++)
  {
    csTriangle& tri = k.tri_buffer[k.tri_indices[i]];
    for (int j = 0; j < 3; j++)
    {
      Edge e(tri[j], tri[(j+1)%3]);
      MeshVerification mv;
      mv.e = e;
      size_t i = mvs.Find(mv);
      if (i == csArrayItemNotFound)
      {
        mv.num_t = 1;
        mvs.Push(mv);
      }
      else
      {
        mvs[i].num_t++;
      }
    }
  }
  for (unsigned int i = 0; i < mvs.GetSize(); i++)
  {
    assert(mvs[i].num_t == 2);
  }
}

void LodGen::GenerateLODs()
{
  k.incident_tris.SetSize(num_vertices);
  for (int i = 0; i < num_triangles; i++)
  {
    const csTriangle& tri = triangles[i];
    k.tri_buffer.Push(tri);
    AddTriangle(k, i);
    for (int j = 0; j < 3; j++)
    {
      Edge e(tri[j], tri[(j+1)%3]);
      k.edges.PushSmart(e);
    }
  }
  
  SlidingWindow sw;
  sw.start_index = 0;
  sw.end_index = num_triangles;
  top_limit = sw.end_index;
  k.sliding_windows.Push(sw);
  int collapse_counter = 0;
  int min_num_triangles = 150;
  //int min_num_triangles = 4;
  window_shift = 0;
  
  while (1)
  {
    unsigned int min_size = 0 /*edges.GetSize() / 2*/;
    
    while (k.edges.GetSize() > min_size)
    {
      float min_d = FLT_MAX;
      int min_v0, min_v1;
      
      for (unsigned int i = 0; i < k.edges.GetSize(); i++)
      {
        int v0 = k.edges[i].v0;
        int v1 = k.edges[i].v1;
        WorkMesh k_prime = k;
        bool result = Collapse(k_prime, v0, v1);
        if (result)
        {
          //VerifyMesh(k_prime);
          float d = SumOfSquareDist(k_prime);
          if (d < min_d)
          {
            min_d = d;
            min_v0 = v0;
            min_v1 = v1;
          }
        }
        k_prime = k;
        result = Collapse(k_prime, v1, v0);
        if (result)
        {
          //VerifyMesh(k_prime);
          float d = SumOfSquareDist(k_prime);
          if (d < min_d)
          {
            min_d = d;
            min_v0 = v1;
            min_v1 = v0;
          }
        }
        if (min_d == 0.0)
          break;
      }
      if (min_d == FLT_MAX)
        break;
      cout << k.edges.GetSize() << ": " << min_d << " - " << min_v0 << ", " << min_v1 << endl;
      bool result = Collapse(k, min_v0, min_v1);
      assert(result);
      VerifyMesh(k);
      collapse_counter++;
      window_shift = 0;
      cout << "T: ";
      for (unsigned int i = 0; i < k.tri_indices.GetSize(); i++)
        cout << i << "=" << k.tri_indices[i] << " ";
      cout << endl << "W: ";
      for (unsigned int i = 0; i < k.sliding_windows.GetSize(); i++)
        cout << k.sliding_windows[i].start_index << "-" << k.sliding_windows[i].end_index << " ";
      cout << endl << "Top limit = " << top_limit << endl;
    }
    SlidingWindow& sw = k.sliding_windows[k.sliding_windows.GetSize()-1];
    int curr_num_triangles = sw.end_index - sw.start_index;
    if (curr_num_triangles < min_num_triangles)
      break;
    
    // Replicate index buffer
    cout << "Replicating: " << curr_num_triangles << endl;
    k.edges.SetSize(0);
    sw.start_index += curr_num_triangles;
    sw.end_index += curr_num_triangles;
    top_limit = sw.end_index;
    for (int i = sw.start_index; i < sw.end_index; i++)
    {
      int itri = k.tri_indices[i-curr_num_triangles];
      k.tri_indices.Push(itri);
      const csTriangle& tri = k.tri_buffer[itri];
      for (int j = 0; j < 3; j++)
      {
        Edge e(tri[j], tri[(j+1)%3]);
        k.edges.PushSmart(e);
      }
    }
    VerifyMesh(k);    
  }
  for (unsigned int i = 0; i < k.tri_indices.GetSize(); i++)
    ordered_tris.Push(k.tri_buffer[k.tri_indices[i]]);
  for (unsigned int i = 0; i < ordered_tris.GetSize(); i++)
    for (unsigned int j = 0; j < 3; j++)
      assert(ordered_tris[i][j] >= 0 && ordered_tris[i][j] < (int)vertices.GetSize());
  cout << "End" << endl;
}

