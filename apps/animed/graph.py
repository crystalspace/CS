from PyQt4 import QtGui as qt, QtCore as qtcore
import math
import random

class Node(qt.QGraphicsItem):
  def __init__(self, graph):
    qt.QGraphicsItem.__init__(self)
    self.setFlag(qt.QGraphicsItem.ItemIsMovable)
    self.setZValue(1)
    # edges
    self.edges = []
    self.graph = graph
    self.size = (500, 25)
    # testing
    self.properties = {"fuckc" : "asdf", "array" : "directions", "kllfjflk" : "poe", "trala" : ["jl", 0, False]}
  def addEdge(self, edge):
    self.edges.append(edge)
    edge.adjust()
  def boundingRect(self):
    adjust = 2
    return qtcore.QRectF(-10 - adjust, -10 - adjust, self.size[0] + adjust, self.size[1] + adjust)
  def paint(self, painter, option, widget):
    painter.setPen(qtcore.Qt.NoPen)
    painter.setBrush(qtcore.Qt.darkGray)
    painter.drawRect(-7, -7, self.size[0], self.size[1])

    gradient = qt.QLinearGradient(0, 0, self.size[0] * 2, 0);
    gradient.setColorAt(0, qt.QColor(qtcore.Qt.yellow).light(120))
    gradient.setColorAt(1, qt.QColor(qtcore.Qt.darkYellow).light(120))
    painter.setBrush(qt.QBrush(gradient))
    painter.setPen(qt.QPen(qtcore.Qt.black, 0))
    painter.drawRect(-10, -10, self.size[0], self.size[1])
    # text
    textrect = qtcore.QRectF(0,-10, self.size[0], self.size[1])
    message = "SkelControlSingleBone : AimCtrlSpine"
    font = painter.font()
    font.setBold(True)
    font.setPointSize(14)
    painter.setFont(font)
    painter.setPen(qtcore.Qt.lightGray)
    painter.drawText(textrect.translated(2, 2), message)
    painter.setPen(qtcore.Qt.black)
    painter.drawText(textrect, message)
  def itemChange(self, change, value):
    if change == qt.QGraphicsItem.ItemPositionChange:
      for edge in self.edges:
        edge.adjust()
    return qt.QGraphicsItem.itemChange(self, change, value)
  def mousePressEvent(self, event):
    self.update()
    self.graph.updateProperties(self)
    qt.QGraphicsItem.mousePressEvent(self, event)
  def mouseReleaseEvent(self, event):
    self.update()
    qt.QGraphicsItem.mouseReleaseEvent(self, event)

class DynamicEdge(qt.QGraphicsItem):
  def __init__(self, start, end):
    qt.QGraphicsItem.__init__(self)
    self.setAcceptedMouseButtons(qtcore.Qt.NoButton)
    self.start = start
    self.end = end
    self.adjust()
  def adjust(self):
    line = qtcore.QLineF(self.start, self.end)
    length = line.length()
    if length <= 0:
      length = 0.1  #!!
    edgeoffset = qtcore.QPointF((line.dx() * 10) / length, (line.dy() * 10) / length)

    self.prepareGeometryChange()
    self.startpoint = line.p1() + edgeoffset;
    self.endpoint = line.p2() - edgeoffset;
  def boundingRect(self):
    size = qtcore.QSizeF(self.end.x() - self.start.x(), self.end.y() - self.start.y())
    rect = qtcore.QRectF(self.start, size)
    return rect.normalized()
  def paint(self, painter, options, widget):
    line = qtcore.QLineF(self.start, self.end)
    # draw the line itself
    painter.setPen(qt.QPen(qtcore.Qt.black, 1, qtcore.Qt.SolidLine, qtcore.Qt.FlatCap, qtcore.Qt.MiterJoin))
    painter.drawLine(line)

class Edge(qt.QGraphicsItem):
  def __init__(self, sourcenode, destnode):
    qt.QGraphicsItem.__init__(self)
    self.arrowsize = 10
    self.setAcceptedMouseButtons(qtcore.Qt.NoButton)
    self.source = sourcenode
    self.dest = destnode
    self.source.addEdge(self)
    self.dest.addEdge(self)
    self.adjust()
  def adjust(self):
    line = qtcore.QLineF(self.source.pos(), self.dest.pos())
    length = line.length()
    if length <= 0:
      length = 0.1  #!!
    edgeoffset = qtcore.QPointF((line.dx() * 10) / length, (line.dy() * 10) / length)

    self.prepareGeometryChange()
    self.sourcepoint = line.p1() + edgeoffset;
    self.destpoint = line.p2() - edgeoffset;
  def boundingRect(self):
    penwidth = 1
    extra = (penwidth + self.arrowsize) / 2.0
    size = qtcore.QSizeF(self.destpoint.x() - self.sourcepoint.x(), self.destpoint.y() - self.sourcepoint.y())
    rect = qtcore.QRectF(self.sourcepoint, size)
    return rect.normalized().adjusted(-extra, -extra, extra, extra)
  def paint(self, painter, options, widget):
    # draw the line itself
    painter.setPen(qt.QPen(qtcore.Qt.black, 1, qtcore.Qt.SolidLine, qtcore.Qt.RoundCap, qtcore.Qt.RoundJoin))
    path = qt.QPainterPath()
    path.moveTo(self.sourcepoint)
    dxy = (self.destpoint - self.sourcepoint) * 0.5
    # Qt Bezier class is a fucking piece of shit!
    dxy.setY(0)
    path.cubicTo(self.sourcepoint + dxy, self.destpoint - dxy, self.destpoint)
    painter.drawPath(path)

    # draw the arrows if there's enough room
    line = qtcore.QLineF(self.sourcepoint, self.destpoint)
    length = line.length()
    if length <= 0:
      length = 0.01
    angle = math.acos(line.dx() / length)
    if line.dy() >= 0:
      angle = 2 * math.pi - angle

    destarrowp1 = self.destpoint + qtcore.QPointF(math.sin(angle - math.pi / 3) * self.arrowsize, math.cos(angle - math.pi / 3) * self.arrowsize)
    destarrowp2 = self.destpoint + qtcore.QPointF(math.sin(angle - math.pi + math.pi / 3) * self.arrowsize, math.cos(angle - math.pi + math.pi / 3) * self.arrowsize);

    painter.setBrush(qtcore.Qt.black);
    poly = qt.QPolygonF()
    poly.append(self.destpoint)
    poly.append(destarrowp1)
    poly.append(destarrowp2)
    painter.drawPolygon(poly)

class Graph(qt.QGraphicsView):
  def __init__(self, parent, propedit):
    self.propedit = propedit
    qt.QGraphicsView.__init__(self, parent)
    scene = qt.QGraphicsScene(self)
    scene.setItemIndexMethod(qt.QGraphicsScene.NoIndex)
    scene.setSceneRect(-200, -200, 400, 400)
    self.setScene(scene)
    self.setCacheMode(qt.QGraphicsView.CacheBackground)
    self.setRenderHint(qt.QPainter.Antialiasing)
    self.setTransformationAnchor(qt.QGraphicsView.AnchorUnderMouse)
    self.setResizeAnchor(qt.QGraphicsView.AnchorViewCenter)

    nodes = self.nodes = []
    for i in xrange(9):
      self.nodes.append(Node(self))
      scene.addItem(self.nodes[i])
    self.edges = []
    self.edges.append(Edge(nodes[0], nodes[1]))
    self.edges.append(Edge(nodes[1], nodes[2]))
    self.edges.append(Edge(nodes[1], nodes[4]))
    self.edges.append(Edge(nodes[2], nodes[5]))
    #self.edges.append(Edge(nodes[3], nodes[0]))
    #self.edges.append(Edge(nodes[3], nodes[4]))
    #self.edges.append(Edge(nodes[4], nodes[5]))
    self.edges.append(Edge(nodes[4], nodes[7]))
    self.edges.append(Edge(nodes[5], nodes[8]))
    self.edges.append(Edge(nodes[6], nodes[3]))
    self.edges.append(Edge(nodes[7], nodes[6]))
    #self.edges.append(Edge(nodes[8], nodes[7]))
    nodes[0].setPos(-400, -400)
    nodes[1].setPos(-300, -200)
    nodes[2].setPos(-300, -150)
    nodes[3].setPos(-400, 300)
    nodes[4].setPos(-400, -100)
    nodes[5].setPos(-200, 100)
    nodes[6].setPos(-400, 250)
    nodes[7].setPos(-400, 200)
    nodes[8].setPos(150, 150)
    for i in self.edges:
      scene.addItem(i)
      i.adjust()
    # are we in the process of connecting 2 nodes?
    self.connectingnodes = False
    self.dragview = False

    self.scale(0.8, 0.8)
    self.setMinimumSize(400, 400)
    self.setWindowTitle("Elastic Nodes")
  def keyPressEvent(self, event):
    k = event.key()
    if k == qtcore.Qt.Key_Plus:
      self.scaleView(1.2)
    elif k == qtcore.Qt.Key_Minus:
      self.scaleView(1 / 1.2)
    elif k == qtcore.Qt.Key_Space or k == qtcore.Qt.Key_Enter:
      for i in self.nodes:
        i.setPos(random.randint(-150, 150), random.randint(-150, 150))
    else:
      qt.QGraphicsView.keyPressEvent(self, event)
  def wheelEvent(self, event):
    self.scaleView(pow(2.0, event.delta() / 240.0))
  def drawBackground(self, painter, rect):
    scenerect = self.sceneRect()
    # fill
    gradient = qt.QLinearGradient(rect.topLeft(), rect.bottomRight())
    gradient.setColorAt(0, qtcore.Qt.white)
    gradient.setColorAt(1, qtcore.Qt.lightGray)
    painter.fillRect(rect, qt.QBrush(gradient))
    painter.setBrush(qtcore.Qt.NoBrush)
    painter.drawRect(rect)
  def scaleView(self, scalefactor):
    factor = self.matrix().scale(scalefactor, scalefactor).mapRect(qtcore.QRectF(0, 0, 1, 1)).width()
    if factor < 0.07 or factor > 100:
      return
    self.scale(scalefactor, scalefactor)
  def mousePressEvent(self, event):
    if event.button() == qtcore.Qt.RightButton:
      item = self.itemAt(event.pos())
      if item:
        pos = self.mapToScene(event.pos())
        self.connectingnodes = [DynamicEdge(pos, pos), item]
        self.scene().addItem(self.connectingnodes[0])
    elif event.button() == qtcore.Qt.MidButton:
      self.dragview = self.mapToScene(event.pos())
    qt.QGraphicsView.mousePressEvent(self, event)
  def mouseMoveEvent(self, event):
    if self.connectingnodes:
      self.connectingnodes[0].end = qtcore.QPointF(self.mapToScene(event.pos()))
      # redraw the background
      self.resetCachedContent()
      self.connectingnodes[0].update()
    elif self.dragview:
      move = self.mapToScene(event.pos()) - self.dragview
      sr = self.scene().sceneRect()
      sr.translate(-move)
      self.scene().setSceneRect(sr)
      # fucking shit Qt bug!
      #self.update()
    qt.QGraphicsView.mouseMoveEvent(self, event)
  def mouseReleaseEvent(self, event):
    if event.button() == qtcore.Qt.RightButton and self.connectingnodes:
      self.scene().removeItem(self.connectingnodes[0])
      item = self.itemAt(event.pos())
      if item and item in self.nodes:
        newedge = Edge(self.connectingnodes[1], item)
        self.edges.append(newedge)
        self.scene().addItem(newedge)
      self.connectingnodes = None
    elif event.button() == qtcore.Qt.MidButton:
      self.dragview = None
    qt.QGraphicsView.mouseReleaseEvent(self, event)
  def updateProperties(self, node):
    self.propedit.clear()
    for key, val in node.properties.iteritems():
      if type(val) == list:
        arr = self.propedit.addParameter("array", key)
        for i in range(len(val)):
          print str(i), str(val[i]), arr
          self.propedit.createItem(str(i), str(val[i]), arr)
      else:
        self.propedit.addParameter(key, str(val))
