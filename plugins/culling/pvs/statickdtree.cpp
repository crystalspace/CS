/*
    Copyright (C) 2006 by Benjamin Stover

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
#include "statickdtree.h"

csStaticKDTree::csStaticKDTree(csArray<csStaticKDTreeObject*>& items) {
    // Base case:  if there are less than a certain threshold, stop recursing
    if (items.Length() < 10) {
        objects = new csArray<csStaticKDTreeObject*>(items);
        csArray<csStaticKDTreeObject*>::Iterator it = items.GetIterator();
        while (it.HasNext())
            it.Next()->leafs.Push(this);
    }
    // Otherwise find the axis with the longest min-max distance and use the midpoint as a splitting axis.
    else {
        csArray<csStaticKDTreeObject*>::Iterator it = items.GetIterator();
        float xmin = 0, ymin = 0, zmin = 0;
        float xmax = 0, ymax = 0, zmax = 0;
        while (it.HasNext()) {
            csStaticKDTreeObject& obj = *it.Next();
            xmin = fmin(obj.box.MinX(), xmin);
            ymin = fmin(obj.box.MinY(), ymin);
            zmin = fmin(obj.box.MinZ(), zmin);
            xmax = fmin(obj.box.MaxX(), xmax);
            ymax = fmin(obj.box.MaxY(), ymax);
            zmax = fmin(obj.box.MaxZ(), zmax);
        }

        float xdist = xmax - xmin;
        float ydist = ymax - ymin;
        float zdist = zmax - zmin;
        float maxdist = fmax(xdist, fmax(ydist, zdist));

        if (xdist == maxdist) {
            splitLocation = (xmax + xmin) / 2;
            axis = CS_XAXIS;
        }
        else if (ydist == maxdist) {
            splitLocation = (ymax + ymin) / 2;
            axis = CS_YAXIS;
        }
        else {
            splitLocation = (zmax + zmin) / 2;
            axis = CS_ZAXIS;
        }

        it = items.GetIterator();
        csArray<csStaticKDTreeObject*> left;
        csArray<csStaticKDTreeObject*> right;

        while (it.HasNext()) {
            csStaticKDTreeObject* obj = it.Next();

            float min = getMin(axis, obj->box), max = getMax(axis, obj->box);
            if (splitLocation < min)
                left.Push(obj);
            else if (splitLocation > max)
                right.Push(obj);
            else {
                left.Push(obj);
                right.Push(obj);
            }
        }

        child1 = new csStaticKDTree(left);
        child2 = new csStaticKDTree(right);
    }
}

csStaticKDTree::~csStaticKDTree() {
    if (isLeafNode()) {
        CS_ASSERT(!child2);
        CS_ASSERT(objects);

        csArray<csStaticKDTreeObject*>::Iterator it = objects.GetIterator();
        while (it.HasNext())
            it.Next()->leafs.Remove(this);
        if (items.IsEmpty())
            delete objects;
    }
    else {
        CS_ASSERT(child2);
        CS_ASSERT(!objects);
        delete child1;
        delete child2;
    }
}

csStaticKDTreeObject* csStaticKDTree::AddObject(const csBox3& bbox, void* userdata) {
    csStaticKDTreeObject* ref = new csStaticKDTreeObject(bbox, userdata);
    AddObject(ref);
    return ref;
}

void csStaticKDTree::AddObject(csStaticKDTreeObject* object) {
    if (isLeafNode()) {
        objects->Push(object);
        object->leafs.Push(this);
    }
    else {
        float min = getMin(axis, object->box), max = getMax(axis, object->box);
        if (splitLocation < min)
            child1->AddObject(object);
        else if (splitLocation > max)
            child2->AddObject(object);
        else {
            child1->AddObject(object);
            child2->AddObject(object);
        }
    }
}

void csStaticKDTree::UnlinkObject(csStaticKDTreeObject* object) {
    csArray<csStaticKDTree*>::Iterator it = object->leafs.GetIterator();
    while (it.HasNext()) {
        csStaticKDTree& t = *it.Next();
        t.objects->DeleteFast(object);
    }
    object->leafs.DeleteAll();
}

void csStaticKDTree::MoveObject(const csBox3& bbox_new, csStaticKDTreeObject* object) {
    // TODO:  optimization would be to check if new box is in same node
    UnlinkObject(object);
    object->box = bbox_new;
    AddObject(object);
}

