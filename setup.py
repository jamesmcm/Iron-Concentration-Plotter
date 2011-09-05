from distutils.core import setup
import matplotlib
import matplotlib.backends.backend_gtkagg
import py2exe
import glib
import gobject
import warnings
from email.header import Header
import email
import email.message
warnings.simplefilter('ignore')

mydata=[]
for value in matplotlib.get_py2exe_datafiles():
    mydata.append(value)
otherdata=['myapp.xml', "bildregistrierung_ng.pyc","bildregistrierung_ng_ext.pyd","bildregistrierung_ng_ext.cpp", "bildregistrierung_ng.py"]
for value in otherdata:
    mydata.append(value)


setup(
    name = 'Iron Concentration Plotter',
    description = 'A tool to plot QSSPC and PL measurements of the interstitial Iron concentration in Silicon wafers',
    version = '1.0',

    windows = [
                  {
                      'script': 'myappgtk.py',
                  }
              ],

    options = {
                  'py2exe': {
                      'compressed':1,
                      'optimize':2,
                      'ascii':True,
                      'packages':'encodings',
                      'includes': ['email', 'email.header', 'email.message', 'cairo', 'pango', 'pangocairo', 'atk', 'gobject', 'gio', 'glib', 'email.mime.*'],
                      'excludes': ['_backend_gdk', '_backend_gtk', '_tkagg,' '_cocoaagg', '_fltkagg','Qt', 'PyQt4','_qt', '_gdk', '_Tkinter', '_ssl', 'Tkinter', '_tcl', 'tcl', 'doctest', "_imagingtk", "PIL._imagingtk", "ImageTk", "PIL.ImageTk", "FixTk", "pydoc"],
                      #'bundle_files':1,
                  }
              },

    data_files=mydata,
    zipfile=None
)
