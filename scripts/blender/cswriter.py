# File: cswriter.py
# Created: Rene Dudfield, June 22.
# 
# Purpose: A general class to write geometry data into crystal space format.


class csSpriteVertex:
  def __init__(self,x,y,z,u,v):
    self.x = x
    self.y = y
    self.z = z
    self.u = u
    self.v = v

  def __str__(self):
    return ("V ("+ str(self.x)+"," +str(self.y)+"," +str(self.z)+":" +
            str(self.u)+"," + str(self.v) + ")")


class csSpriteFrame:
  def __init__(self, frameName, vertexList):
    """ ("stand1", [vert1, vert2, vert3, vert4]) """
    self.frameName = frameName
    self.vertexList = vertexList
    self.indenting = 3
    
  def __str__(self):
    returnString =  (" " * self.indenting) + "FRAME '" + self.frameName + "' (\n"
    for x in self.vertexList:
      returnString = returnString + (" " * 2 * self.indenting) +str(x) + "\n"
    returnString = returnString + (" " * self.indenting) +")\n"
  
    return returnString


class csAction:
  def __init__(self, actionName, frameList):
    """act1 = csAction("stand1",[["stand1",30], ["stand1", 100]])"""

    self.frameList = frameList
    self.actionName = actionName
    self.indenting = 3

  def __str__(self):
    returnString = "ACTION " + "'" + self.actionName + "'(\n"
    f = self.frameList

    for x in f:
      returnString = returnString + (" " * self.indenting*2) + ( "F (" + x[0] + ", " + 
				      str(x[1]) + ")\n" )
    
    returnString = returnString + (" " * self.indenting) + ")"
    return returnString


class csTriangle:
  """ takes an index to some vertexes. """
  def __init__(self, indexList):
    self.indexList = indexList
    
  def __str__(self):
    l = self.indexList
    return "TRIANGLE (" + str(l[0]) +"," + str(l[1]) + "," + str(l[2]) + ")"


class csSprite3d:
  """ not implemented """
  
  def __init__(self,frameList, triangleList, actionList,
                     textureName, spriteName):
    """([spriteFrame, spriteFrame], [csTriangle, csTriangle],
	[csAction, csAction], "sydney.gif", "sydney")
    """
    
    self.indenting = 3
    
    self.frameList = frameList
    self.triangleList = triangleList
    self.actionList = actionList
    self.textureName = textureName
    self.spriteName = spriteName
    
    
  def __str__(self):
    
    frameList = self.frameList
    triangleList = self.triangleList
    actionList = self.actionList
    textureName = self.textureName
    spriteName = self.spriteName
    indenting = self.indenting

    
    returnString = ("SPRITE '" + spriteName + "'(\n"+ (" " * indenting) +
		   "TEXNR('" + textureName + "')\n")
    
    for x in frameList:
      returnString = returnString + str(x)
    
    for x in triangleList:
      returnString = returnString + (" " * indenting) + str(x) + "\n"
    
    for x in actionList:
      returnString = returnString + "\n" + (" " * indenting) + str(x) + "\n"
    
    returnString = returnString + ")"
    
    return returnString
  



class csWriter:

  def vertexListToCsOut(self,n,spacing):
    """ vertexListToCsOut([0.1, 1.222, 2.3], 3) returns a string in the 
    cs format.  The spacing variable is how far the output should be padded 
    from the left."""
    
    tempString = ""
    for x in range(0,spacing):
       tempString = tempString + ' '
  
    tempString = tempString + 'VERTEX('
  # Add the vertexes
    for x in n:
      tempString = tempString + ' %(x)f10,' %vars()
    
  # take the last ' off the end.
    tempString = tempString[:-1]
  
    tempString = tempString + ')'
  
    return(tempString)
  
  
  
  
  def polygonListToCsOut(self,n,spacing,name):
    """ polygonListToCsOut([0.1, 1.222, 2.3], 3,'up') returns a string in 
    the cs format.  spacing is how far the output should be padded from the 
    left.  name is for the name of the POLYGON.
    """
    
    tempString = ""
    for x in range(0,spacing):
       tempString = tempString + ' '
    
    tempString = tempString +'POLYGON \'' + name + '\' (VERTICES('
  
    for x in n:
      tempString = tempString + ' %(x)d,' %vars()                               
  
  # take the last ' off the end.
    tempString = tempString[:-1]
  
    tempString = tempString + '))'
  
    return(tempString)
  
  
  
  
  def csThingWriter(self,vertexList,polygonList,thingName,lpadding,addString):
    """ 
    Format of variables: (polygonList, vertexList)
    polygonList([[1,2,3,4],[2,3,4,1]])
    vertexList([[1.0,3.0,2.0], [1.2,2.3,4.2], [0.1,0.2,0.3], [1.2,3.0,0.1]])
    csThingWriter(vertextList,polygonList,thingName,lpadding,addString)
    returns a  string with a THING in the cs format.
    addString is a string that is placed in between the vertexes and the
    polygons.
  
    NOTE: this function would be better with polygon names.  maybe a list of
    strings could also be used as input.  For now the polygons will be 
    named numbers.
    """
    
    stringToReturn = ""
    paddString = ""
  
    for x in range(0,lpadding):
      paddString = paddString + '  '
  
    stringToReturn = stringToReturn+paddString+'THING \''+thingName+'\' (\n'
    
    print "==================================="
    print self
    
    for x in vertexList:
      tempString = self.vertexListToCsOut(x,2)
      stringToReturn = stringToReturn + paddString + tempString + '\n'
  
    stringToReturn = stringToReturn + addString
  
    count = 0
    for x in polygonList:
      countString = '%(count)d,' %vars()
      tempString = self.polygonListToCsOut(x,2,countString)
      stringToReturn = stringToReturn + paddString + tempString + '\n'
      count = count + 1
  
    stringToReturn = stringToReturn + paddString + ')'
    return(stringToReturn)

  

def test_csSprite3d():
  """ This function tests out the csSprite3d python class, and all the classes
      that are used in this class.
  """
  vert1 = csSpriteVertex(0.014, 0.736, 0.068, 0.26, 0.74)
  vert2 = csSpriteVertex(0.082,0.704,0.045,0.31,0.66)
  vert3 = csSpriteVertex(0.069,0.752,0.016,0.31,0.75)
  vert4 = csSpriteVertex(-0.000,0.741,0.055,0.26,0.80)
  vert5 = csSpriteVertex(-0.104,0.650,0.019,0.82,0.61)
  vert6 = csSpriteVertex(-0.088,0.687,0.004,0.85,0.59)
  
  sf1 = csSpriteFrame("stand1", [vert1, vert2, vert3, vert4, vert5, vert6])
  
  cstri1 = csTriangle([0,1,2])
  cstri2 = csTriangle([3,4,5])
  
  act1 = csAction("stand",[["stand1",30], ["stand1", 100]])
  
  asprite = csSprite3d([sf1], [cstri1, cstri2], [act1], "sydney.gif", "sydney")
  
  print vert1
  print sf1
  print cstri1
  print act1
  print ""
  print asprite
  
#  a = csSprite3d([spriteFrame, spriteFrame], [csTriangle, csTriangle],
#	[csAction, csAction], "sydney.gif", "sydney")
 
test_csSprite3d()
