/*
    Copyright (C) 2001 by W.C.A. Wijngaards

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

#ifndef __GENMAZE_H__
#define __GENMAZE_H__

///
/// node for csGenMaze
///
class csGenMazeNode {
public:
  /// visited
  bool visited;
  /// openings to n,e,s,w?
  bool opening[4];
};


///
/// Class to generate gridlike mazes.
///
class csGenMaze {
  /// maze size
  int width, height;

  /// maze contents
  csGenMazeNode *maze;

  /// straightness
  float straightness;

  /// cyclicalness
  float cyclicalness;

public:
  /// give width and height of maze, initialises maze.
  csGenMaze(int w, int h);
  ///
  ~csGenMaze();
  /// init maze to fully walled in.
  void InitMaze();

  /// set the straightness of the corridors in the maze
  void SetStraightness(float n) {straightness = n;}
  /// get the straightness of the corridors in the maze
  float GetStraightness() const {return straightness;}
  /// set the cyclicalness of the corridors in the maze
  void SetCyclicalness(float n) {cyclicalness = n;}
  /// get the cyclicalness of the corridors in the maze
  float GetCyclicalness() const {return cyclicalness;}

  /// Set a position at an edge to be an 'access point' to the maze.
  /// there will be an opening to the outside there.
  /// (corner points will make double accesses)
  void MakeAccess(int x, int y);

  /// generate the maze from a given starting point
  void GenerateMaze(int x, int y);

  /// Prints ascii Maze - amazing :-)
  void PrintMaze();

  /// Return actual height - allows for thickness of walls
  int ActualHeight();

  /// Return actual width - allows for thickness of walls
  int ActualWidth();

  /// Returns true if x,y is solid - note this function adjusts
  /// for tickness of walls
  bool ActualSolid(int x,int y);

  /// get a maze node
  csGenMazeNode& GetNode(int x, int y)
  {
    CS_ASSERT(x>=0 && x<width);
    CS_ASSERT(y>=0 && y<height);
    return maze[y*width+x];
  }

  /// visit a node, used to gen maze.
  void VisitNode(int x, int y, int direction);

  /// in gen maze: generate an order with preference
  void GenOrder(int order[4], int direction);

  /// get neightboring position in direction
  void GetNeighbor(int dir, int x, int y, int& nx, int& ny);

  /// make an opening between two nodes
  void MakeOpening(int x1, int y1, int x2, int y2);

  /// is there an opening between two nodes?
  bool Opening(int x1, int y1, int x2, int y2);

};


#endif // __GENMAZE_H__
