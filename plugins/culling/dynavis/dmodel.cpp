/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "csutil/scfstr.h"
#include "iutil/string.h"
#include "qint.h"
#include "qsqrt.h"
#include "csgeom/math3d.h"
#include "csgeom/pmtools.h"
#include "igeom/objmodel.h"
#include "igeom/polymesh.h"
#include "dmodel.h"

//---------------------------------------------------------------------------

csObjectModel::csObjectModel ()
{
  normals = NULL;
  num_normals = -1;
}

csObjectModel::~csObjectModel ()
{
  delete[] normals;
}

//---------------------------------------------------------------------------

csObjectModelManager::csObjectModelManager () : models (12263)
{
}

csObjectModelManager::~csObjectModelManager ()
{
  csHashIterator it (&models);
  while (it.HasNext ())
  {
    csObjectModel* model = (csObjectModel*)it.Next ();
    delete model;
  }
}

csObjectModel* csObjectModelManager::CreateObjectModel (iObjectModel* imodel)
{
  csObjectModel* model = (csObjectModel*)models.Get ((csHashKey)imodel);
  if (model)
  {
    model->ref_cnt++;
  }
  else
  {
    model = new csObjectModel ();
    model->ref_cnt = 1;
    model->imodel = imodel;
    // To make sure we will recalc we set shape_number to one less.
    model->shape_number = imodel->GetShapeNumber ()-1;
  }
  return model;
}

void csObjectModelManager::ReleaseObjectModel (csObjectModel* model)
{
  CS_ASSERT (model->ref_cnt > 0);
  if (model->ref_cnt == 1)
  {
    // We are about to delete the model.
    models.DeleteAll ((csHashKey)(model->imodel));
    delete model;
    return;
  }
  model->ref_cnt--;
}

bool csObjectModelManager::CheckObjectModel (csObjectModel* model)
{
  CS_ASSERT (model->ref_cnt > 0);
  if (model->imodel->GetShapeNumber () != model->shape_number)
  {
    model->shape_number = model->imodel->GetShapeNumber ();
    iPolygonMesh* mesh = model->imodel->GetSmallerPolygonMesh ();
    if (!mesh)
    {
      delete[] model->normals;
      model->normals = NULL;
      model->num_normals = -1;
    }
    else
    {
      if (model->num_normals != mesh->GetPolygonCount ())
      {
        delete[] model->normals;
        model->num_normals = mesh->GetPolygonCount ();
        model->normals = new csVector3 [model->num_normals];
      }
      csPolygonMeshTools::CalculateNormals (mesh, model->normals);
    }
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------

