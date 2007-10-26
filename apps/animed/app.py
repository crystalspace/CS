from PyQt4 import QtGui as qt, QtCore as qtcore
import sys
# uic files
import graph
import qcelstartwidget
import propedit

if __name__ == "__main__":
  app = qt.QApplication(sys.argv)
  top = qt.QSplitter()
  celstart = qcelstartwidget.QCelstartWidget(top)
  celstart.LoadWorld("shadertest.celzip")
  celstart.Run()
  #celstart = qt.QTextEdit()
  top.addWidget(celstart)
  propedit = propedit.PropertyEditor(top)
  top.addWidget(propedit)
  top.setOrientation(qtcore.Qt.Vertical)
  s = qt.QSplitter()
  s.addWidget(top)
  g = graph.Graph(s, propedit)
  s.addWidget(g)
  s.show()
  sys.exit(app.exec_())
