/*
    Copyright (C) 2003 by Rene Jager
    (renej@frog.nl, renej.frog@yucom.be, renej_frog@sourceforge.net)

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

package com.crystalspace;

import com.crystalspace.csPluginRequest;

public class CS extends cspace
{
    public static String CSJAVA_LIB = "csjava";

    static 
    { 
        try
        {
            System.out.println("Loading CrystalSpace library");
            System.loadLibrary(CSJAVA_LIB);
        }
        catch (UnsatisfiedLinkError e)
        {
            System.out.println("Fatal Error: unresolved symbol in '" +
			       CSJAVA_LIB + "'.");
            System.out.println(e.toString());
            System.exit(-1);
        }
        catch (SecurityException e)
        {
            System.out.println("Fatal Error: no permission to open '" +
			       CSJAVA_LIB + "'.");
            System.out.println(e.toString());
            System.exit(-1);
        }
    }

    // handy
    protected static boolean requestPlugin (String name, String intf)
    {
        int intfVersion = 0;
        long intfID = (long) iSCF.getSCF().GetInterfaceID(intf);

	csPluginRequestArray arr = new csPluginRequestArray();
        arr.Push(new csPluginRequest(
	    new csString(name), new csString(intf), intfID, intfVersion));

	boolean result =
	    csInitializer._RequestPlugins(getTheObjectRegistry(), arr); 
        System.out.println("Loading " + name + "? " + result);
        return result;
    }

    // CS_VEC_* constants
    public static csVector3 CS_VEC_FORWARD = new csVector3(0,0,1);
    public static csVector3 CS_VEC_BACKWARD = new csVector3(0,0,-1);
    public static csVector3 CS_VEC_RIGHT = new csVector3(1,0,0);
    public static csVector3 CS_VEC_LEFT = new csVector3(-1,0,0);
    public static csVector3 CS_VEC_UP = new csVector3(0,1,0);
    public static csVector3 CS_VEC_DOWN = new csVector3(0,-1,0);
    public static csVector3 CS_VEC_ROT_RIGHT = new csVector3(0,1,0);
    public static csVector3 CS_VEC_ROT_LEFT = new csVector3(0,-1,0);
    public static csVector3 CS_VEC_TILT_RIGHT = new csVector3(0,0,-1);
    public static csVector3 CS_VEC_TILT_LEFT = new csVector3(0,0,1);
    public static csVector3 CS_VEC_TILT_UP = new csVector3(-1,0,0);
    public static csVector3 CS_VEC_TILT_DOWN = new csVector3(1,0,0);

};
