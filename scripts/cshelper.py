import sys, pytocs;

class PrintOut:
  def write(self, data):
    pytocs.printout(data);

class PrintErr:
  def write(self, data):
    pytocs.printerr(data);

sys.stdout=PrintOut();
sys.stderr=PrintErr();
