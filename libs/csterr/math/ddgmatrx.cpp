/*
    Copyright (C) 1997, 1998, 1999, 2000 by Alex Pfaffe
	(Digital Dawn Graphics Inc)
  
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
#include "math/ddgmatrx.h"

ddgMatrix4& ddgMatrix4::operator *= ( const ddgMatrix4& x )
{	ddgMatrix4 c(*this);

	m[0].v[0] = c.m[0].v[0] * x.m[0].v[0] + c.m[1].v[0] * x.m[0].v[1] + c.m[2].v[0] * x.m[0].v[2] + c.m[3].v[0] * x.m[0].v[3];
    m[0].v[1] = c.m[0].v[1] * x.m[0].v[0] + c.m[1].v[1] * x.m[0].v[1] + c.m[2].v[1] * x.m[0].v[2] + c.m[3].v[1] * x.m[0].v[3];
	m[0].v[2] = c.m[0].v[2] * x.m[0].v[0] + c.m[1].v[2] * x.m[0].v[1] + c.m[2].v[2] * x.m[0].v[2] + c.m[3].v[2] * x.m[0].v[3];
	m[0].v[3] = c.m[0].v[3] * x.m[0].v[0] + c.m[1].v[3] * x.m[0].v[1] + c.m[2].v[3] * x.m[0].v[2] + c.m[3].v[3] * x.m[0].v[3];
  
	m[1].v[0] = c.m[0].v[0] * x.m[1].v[0] + c.m[1].v[0] * x.m[1].v[1] + c.m[2].v[0] * x.m[1].v[2] + c.m[3].v[0] * x.m[1].v[3];
	m[1].v[1] = c.m[0].v[1] * x.m[1].v[0] + c.m[1].v[1] * x.m[1].v[1] + c.m[2].v[1] * x.m[1].v[2] + c.m[3].v[1] * x.m[1].v[3];
	m[1].v[2] = c.m[0].v[2] * x.m[1].v[0] + c.m[1].v[2] * x.m[1].v[1] + c.m[2].v[2] * x.m[1].v[2] + c.m[3].v[2] * x.m[1].v[3];
	m[1].v[3] = c.m[0].v[3] * x.m[1].v[0] + c.m[1].v[3] * x.m[1].v[1] + c.m[2].v[3] * x.m[1].v[2] + c.m[3].v[3] * x.m[1].v[3];
  
	m[2].v[0] = c.m[0].v[0] * x.m[2].v[0] + c.m[1].v[0] * x.m[2].v[1] + c.m[2].v[0] * x.m[2].v[2] + c.m[3].v[0] * x.m[2].v[3];
	m[2].v[1] = c.m[0].v[1] * x.m[2].v[0] + c.m[1].v[1] * x.m[2].v[1] + c.m[2].v[1] * x.m[2].v[2] + c.m[3].v[1] * x.m[2].v[3];
	m[2].v[2] = c.m[0].v[2] * x.m[2].v[0] + c.m[1].v[2] * x.m[2].v[1] + c.m[2].v[2] * x.m[2].v[2] + c.m[3].v[2] * x.m[2].v[3];
	m[2].v[3] = c.m[0].v[3] * x.m[2].v[0] + c.m[1].v[3] * x.m[2].v[1] + c.m[2].v[3] * x.m[2].v[2] + c.m[3].v[3] * x.m[2].v[3];
  
	m[3].v[0] = c.m[0].v[0] * x.m[3].v[0] + c.m[1].v[0] * x.m[3].v[1] + c.m[2].v[0] * x.m[3].v[2] + c.m[3].v[0] * x.m[3].v[3];
	m[3].v[1] = c.m[0].v[1] * x.m[3].v[0] + c.m[1].v[1] * x.m[3].v[1] + c.m[2].v[1] * x.m[3].v[2] + c.m[3].v[1] * x.m[3].v[3];
	m[3].v[2] = c.m[0].v[2] * x.m[3].v[0] + c.m[1].v[2] * x.m[3].v[1] + c.m[2].v[2] * x.m[3].v[2] + c.m[3].v[2] * x.m[3].v[3];
	m[3].v[3] = c.m[0].v[3] * x.m[3].v[0] + c.m[1].v[3] * x.m[3].v[1] + c.m[2].v[3] * x.m[3].v[2] + c.m[3].v[3] * x.m[3].v[3];                                                                  

	return *this;

}

void ddgMatrix4::transpose()
{
	ddgMatrix4 a(*this);

	/*---------------------------------------------------
	La transpuesta es la que cambia filas por columnas
	o sea m[i].v[j] = m[j].v[i] (la diagonal no cambia)
	para una matriz de rotación u ortogonal, la trans-
	puesta es la inversa.
	----------------------------------------------------*/

	m[0].v[1] = a.m[1].v[0];
	m[0].v[2] = a.m[2].v[0];
	m[0].v[3] = a.m[3].v[0];
	m[1].v[2] = a.m[2].v[1];
	m[1].v[3] = a.m[3].v[1];
	m[2].v[3] = a.m[3].v[2];

	m[1].v[0] = a.m[0].v[1];
	m[2].v[0] = a.m[0].v[2];
	m[3].v[0] = a.m[0].v[3];
	m[2].v[1] = a.m[1].v[2];
	m[3].v[1] = a.m[1].v[3];
	m[3].v[2] = a.m[2].v[3];
}

/*-------------------------------------------------------------
Saca la inversa de la matriz. NO ES LA TRANSPUESTA.
siendo A' la inversa de A => A*A' = A'*A = I (mat identidad).
La transformación de la inversa es exactamente contraria a 
la de la matriz. Si tenemos en cuenta que al multiplicar ma-
trices se acumulan sus efectos es fácil ver porque la matriz 
por la inversa da la identidad (transformo y destransformo lo 
que da la identidad que no hace nada).
En este caso se supone una parte rotacional ortogonal, por lo 
que la parte de rotación solo la trasponemos y además que el
último vector columna es (0,0,0,1).
--------------------------------------------------------------*/
void ddgMatrix4::invert()
{
	ddgMatrix4 b = *this;

	b.m[0].v[0] = m[0].v[0];
  	b.m[0].v[1] = m[1].v[0];
	b.m[0].v[2] = m[2].v[0];
	b.m[1].v[0] = m[0].v[1];
	b.m[1].v[1] = m[1].v[1];
	b.m[1].v[2] = m[2].v[1];
	b.m[2].v[0] = m[0].v[2];
	b.m[2].v[1] = m[1].v[2];
	b.m[2].v[2] = m[2].v[2];
  
	/* el nuevo vector desplazamiento es:  d' = -(R^-1) * d */
	b.m[3].v[0] = -( m[3].v[0] * b.m[0].v[0] + m[3].v[1] * b.m[1].v[0] + m[3].v[2] * b.m[2].v[0] );
	b.m[3].v[1] = -( m[3].v[0] * b.m[0].v[1] + m[3].v[1] * b.m[1].v[1] + m[3].v[2] * b.m[2].v[1] );
	b.m[3].v[2] = -( m[3].v[0] * b.m[0].v[2] + m[3].v[1] * b.m[1].v[2] + m[3].v[2] * b.m[2].v[2] );
  
	/* el resto queda igual */
	b.m[0].v[3] = b.m[1].v[3] = b.m[2].v[3] = 0.0f; 
	b.m[3].v[3] = 1.0f;

	*this = b;
}
