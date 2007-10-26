import PyQt4.QtGui as qt, PyQt4.QtCore as qtcore

class PropertyEditor(qt.QTreeWidget):
  def __init__(self, parent):
    qt.QTreeWidget.__init__(self, parent)
    self.setRootIsDecorated(False)
    self.setItemsExpandable(True)
    self.setAnimated(False)
    self.setColumnCount(2)
    self.setObjectName("treewidget")

    self.addParameter("unset", "unset")
    self.addParameter("array", "directions")
  def addParameter(self, name, value):
    return self.createItem(name, value, self)
  def createItem(self, name, value, parent):
    item = qt.QTreeWidgetItem(parent)
    item.setText(0, name)
    item.setText(1, value)
    item.setFlags(item.flags() | qtcore.Qt.ItemIsEditable)
    item.setExpanded(True)
    return item
  def mousePressEvent(self, event):
    qt.QTreeWidget.mousePressEvent(self, event)
    if event.button() == qtcore.Qt.RightButton:
      selitem = self.selectedItems()
      if len(selitem):
        selitem = selitem[0]
        if "array" == selitem.text(0):
          self.createItem(str(selitem.childCount()), "unset", selitem)
