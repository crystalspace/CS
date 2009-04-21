# This is an adjunct of the Python cspace module. It is loaded automatically
# when the Crystal Space cspython plugin is used by C++ programs when the
# command-line option --python-enable-reporter is specified. It redirects
# Python's sys.stdout and sys.stderr to a loaded iReporter plugin.  You never
# need to load this file manually.

import sys, pytocs;

class PrintOut:
  def write(self, data):
    pytocs.printout(data);

class PrintErr:
  def write(self, data):
    pytocs.printerr(data);

sys.stdout=PrintOut();
sys.stderr=PrintErr();
