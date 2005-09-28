/*
    Copyright (C) 2005 by Christopher Nelson

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
#include "fvg.h"

namespace aws
{
  namespace fvg
  {

    fvg_parser::fvg_parser()
    {
     
    }

    fvg_parser::~fvg_parser()
    {

    }

    // Parses a float and returns the number of characters consumed by the parse.
    uint fvg_parser::ParseFloat(const char *base, float &rv)
    {
      char *end;
      rv = (float)strtod(base, &end);

      return end-base;
    }

    void fvg_parser::ParsePath(object *vo, shape_attr &attr, csString &path)
    {
      float lx = 0, ly = 0;
      const char *pos = path.GetData();

      polygon *poly=0;

      while(pos<path.GetData() + path.Length())
      {      
	float x, y;

	// Skip whitespace
	if (isspace(*pos) || *pos == ',') { ++pos; continue; }

	switch(*pos)
	{
	// Absolute move to 
	//  If there's no poly, then we start a new one.
	//  If there IS a poly, we add it to the shape stack and start a new one.
	//  In all cases, a MoveTo starts a new shape.
	case 'M':        
	  pos+= ParseFloat(pos, x);
	  pos+= ParseFloat(pos, y);

	  if (poly) vo->AddShape(poly);	    	  
  	
	  poly = new polygon;
	  poly->SetAttr(attr);
	  poly->AddVertex(csVector2(x,y));
	  break;

	// Relative move to
	//  If there's no poly, then we start a new one, the coords are considered absolute, NOT relative.
	//  If there IS a poly, we add it to the shape stack and start a new one.
	//  In all cases, a MoveTo starts a new shape.
	case 'm':
	  pos+= ParseFloat(pos, x);
	  pos+= ParseFloat(pos, y);  	  	  

	  if (poly) vo->AddShape(poly);	  
	  else
	  {
	    x+=lx;	  
	    y+=ly;
	  }	  

	  poly = new polygon;
	  poly->SetAttr(attr);
	  poly->AddVertex(csVector2(x,y));  
	  break;

	// Absolute line to
	case 'L':
	  pos+= ParseFloat(pos, x);
	  pos+= ParseFloat(pos, y);
	  
	  lx=x; ly=y;
	  if (poly) poly->AddVertex(csVector2(x,y));
	  break;

	// Relative line to
	case 'l':
	  pos+= ParseFloat(pos, x);
	  pos+= ParseFloat(pos, y);
	  
	  x+=lx;
	  y+=ly;

	  lx=x; ly=y;
	  if (poly) poly->AddVertex(csVector2(x,y));
	  break;

	// Absolute horizontal line to
	case 'H':
	  pos+= ParseFloat(pos, x);	
	  if (poly) poly->AddVertex(csVector2(x,y));
	  break;

	// Relative horizontal line to
	case 'h':
	  pos+= ParseFloat(pos, x);
	  if (poly) poly->AddVertex(csVector2(x,y));
	  break;

	// Absolute vertical line to
	case 'V':
	  pos+= ParseFloat(pos, y);	
	  if (poly) poly->AddVertex(csVector2(x,y));
	  break;

	// Relative vertical line to
	case 'v':
	  pos+= ParseFloat(pos, y);
	  if (poly) poly->AddVertex(csVector2(x,y));
	  break;

	// Close path.
	case 'Z':
	case 'z':
	  if (poly)
	  { 
	    poly->Close();
	    vo->AddShape(poly);
	  }

	  poly=0;
	  break;

	default: // Anytime we don't understand what is in the path, we just exit.
	  return;
	} // end switch

	lx=x; ly=y;

      } // end for each char in path

      if (poly) vo->AddShape(poly);
    }

    csColor4 fvg_parser::ParseColor(const char *val)
    {      
      csColor4 c;
      size_t len = strlen(val);
      
      if (len<7) return c;

      if (val[0] == '#')
      {
	char digits[3];

	digits[2]=0;

	digits[0] = val[1];
	digits[1] = val[2];

	long tmp = strtol(digits, 0, 16);

	c.red = (float)tmp / 255.0;        	  

	digits[0] = val[3];
	digits[1] = val[4];

	tmp = strtol(digits, 0, 16);

	c.green = (float)tmp / 255.0;

	digits[0] = val[5];
	digits[1] = val[6];

	tmp = strtol(digits, 0, 16);

	c.blue = (float)tmp / 255.0;

	// Get alpha
	if (len>=9)
	{
	  digits[0] = val[7];
	  digits[1] = val[8];

	  tmp = strtol(digits, 0, 16);

	  c.alpha = (float)tmp / 255.0;
	}
	else 
	  c.alpha=1;
      }      

      return c;
    }

    void fvg_parser::FillAttribute(shape_attr &attr, csRef<iDocumentNode> &pos)
    {
      const char *tmp = pos->GetAttributeValue("fill");
      
      if (tmp)
      {
	attr.filled=true;
	attr.fill_color = ParseColor(tmp);
      }

      tmp = pos->GetAttributeValue("stroke");

      if (tmp)
      {
	attr.stroked=true;
	attr.stroke_color = ParseColor(tmp);
      }

      tmp = pos->GetAttributeValue("stroke-opacity");

      if (tmp)      
	attr.stroke_color.alpha = pos->GetAttributeValueAsFloat("stroke-opacity");
      

      tmp = pos->GetAttributeValue("fill-opacity");

      if (tmp)      
	attr.fill_color.alpha = pos->GetAttributeValueAsFloat("fill-opacity");

      tmp = pos->GetAttributeValue("transform");

      if (tmp)
      {
	// float tx, ty, ra, sx, sy;
	// parse the transform stack...	
      }
      
    }
    
    void fvg_parser::ParseNode(object *vo, csRef<iDocumentNodeIterator> &pos)
    {
      // Walk through all of the nodes and
      while(pos->HasNext())
      {
	csRef<iDocumentNode> child = pos->Next ();

	// Don't process comments.
	if (child->GetType()==CS_NODE_COMMENT) continue;

	csString name(child->GetValue());

	// This is the definition of a graphics object, a shape.
	if (name == "g")
	{
	  csRef<iDocumentNodeIterator> new_pos = child->GetNodes();

	  // Create a new graphics object.
	  object *_vo = new object();

	  // Create an attribute object.
	  shape_attr attr;
    
	  // Fill the attribute structure.
	  FillAttribute(attr, child);	  	  

	  // Insert it into the map.
	  fvg_shapes[child->GetAttributeValue("id")] = _vo;
	  
	  // Parse it.
	  ParseNode(_vo, new_pos);
	}      
	else if (vo!=0)
	{
	   // Create an attribute object.
	  shape_attr attr;
    
	  // Fill the attribute structure.
	  FillAttribute(attr, child);	

	  // If we have a graphics object, start interpreting graphics commands.
	  if (name=="path")
	  {
	    csString v(child->GetAttributeValue("d"));
            ParsePath(vo, attr, v);
	  }
	  else if(name=="rect")
	  {
	    float x,y,width,height,roundness,miter;

            x = child->GetAttributeValueAsFloat("x");
	    y = child->GetAttributeValueAsFloat("y");
	    width = child->GetAttributeValueAsFloat("width");
	    height = child->GetAttributeValueAsFloat("height");

	    roundness = child->GetAttributeValueAsFloat("roundness");
	    miter = child->GetAttributeValueAsFloat("miter");

	    if (roundness>0)	    
	      vo->AddShape(new rect(RECT_ROUNDED, csVector2(x,y), csVector2(x+width, y+height), roundness));
	    else if (miter>0)
	      vo->AddShape(new rect(RECT_MITERED, csVector2(x,y), csVector2(x+width, y+height), miter));
	    else
	      vo->AddShape(new rect(RECT_NORMAL, csVector2(x,y), csVector2(x+width, y+height), 0));	    
	  }
	  else if(name=="circle")
	  {
	    float x,y,r;

            x = child->GetAttributeValueAsFloat("cx");
	    y = child->GetAttributeValueAsFloat("cy");
	    r = child->GetAttributeValueAsFloat("r");

	    csVector2 e1(x-r,y-r), e2(x+r, y+r);
	    vo->AddShape(new ellipse(e1, e2));
	  }
	  else if(name=="ellipse")
	  {
	    float x,y,rx,ry;

            x = child->GetAttributeValueAsFloat("cx");
	    y = child->GetAttributeValueAsFloat("cy");
	    rx = child->GetAttributeValueAsFloat("rx");
	    ry = child->GetAttributeValueAsFloat("ry");

	    csVector2 e1(x-rx,y-ry), e2(x+rx, y+ry);
	    vo->AddShape(new ellipse(e1, e2));
	  }
          else if(name=="line")
	  {
	    float x1,y1,x2,y2;

            x1 = child->GetAttributeValueAsFloat("x1");
	    y1 = child->GetAttributeValueAsFloat("y1");
	    x2 = child->GetAttributeValueAsFloat("x2");
	    y2 = child->GetAttributeValueAsFloat("y2");

	    csVector2 s1(x1,y1), s2(x2, y2);
	    vo->AddShape(new line(s1, s2));
	  }
	}

      } // end while more nodes left.
    }

    bool fvg_parser::Parse (const scfString &txt, autom::scope &sc)
    {
      csRef<iDocumentSystem> xml;
      xml.AttachNew (new csTinyDocumentSystem ());
      csRef<iDocument> doc = xml->CreateDocument ();

      doc->Parse(txt.GetData(), true);

      return true;
    }

    bool fvg_parser::Draw(const csString &name, iPen *pen)
    {
      shape_map_type::iterator pos = fvg_shapes.find(name);

      if (pos==fvg_shapes.end()) return false;
      else pos->second->Draw(pen);

      return true;
    }

    object *fvg_parser::Find(const csString &name)
    {
      shape_map_type::iterator pos = fvg_shapes.find(name);

      if (pos==fvg_shapes.end()) return 0;
      else return pos->second;
    }

  } // end fvg namespace
} // end aws namespace
