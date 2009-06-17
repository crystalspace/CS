#!/usr/bin/python
from guidregistry import guid_registry as gr
import uuid
import os
import sha
import md5
import re
import sys

from lxml import etree


class Empty(object):
    """Marker class"""
    pass


class InitializeOnDef(type):
    """
    Ok. Now this is black magic. Makes every class have its own stack of
    static_kw, map_kw etc. and simplifies defining them through not having
    to use super().
    """
    def __init__(cls, name, bases, classdict):
        cls._class_name = name
        cls.init()

class WixBase(object):
    __metaclass__ = InitializeOnDef

    @classmethod
    def setup(cls):
        cls.kw_map['id'] = 'Id'

    @classmethod
    def init(cls):
        cls.kw_static = getattr(cls, 'kw_static', {}).copy() #Statically set parameters
        cls.kw_map = getattr(cls, 'kw_map', {}).copy() #Rename
        cls.kw_allowed = getattr(cls, 'kw_allowed', {}).copy()  #Additionally allowed parameters
        cls.kw_default = getattr(cls, 'kw_default', {}).copy()  #Default values of parameters
        cls.kw_default_call = getattr(cls, 'kw_default_call', {}).copy()#Default values of parameters as function
        cls.kw_convert = getattr(cls, 'kw_convert', {}).copy()
        cls.highersetup = None
        cls.setup()

    def __init__(self, **kw):
        self.param = kw.copy()
        super(WixBase, self).__init__()

        self.childs = []
        if "parent" in kw.keys():
            kw["parent"].add(self)
            kw.pop("parent")
        self.kw = kw

    def toXML(self, contain):
        param = self.kw.copy()

        #Validate we got only valid keys
        for i in self.kw.keys():
            if i not in self.kw_map.keys() + self.kw_allowed.keys():
                raise TypeError('Unknown parameter ' + i)

        for i in self.kw_default:
            if i not in param.keys():
                param[i] = self.kw_default[i]

        for i in self.kw_default_call:
            if i not in param.keys():
                param[i] = self.kw_default_call[i](self)

        param.update(self.kw_static)

        for i in param:
            if i in self.kw_convert.keys():
                param[i] = self.kw_convert[i](self, param.pop(i))

        for i in param:
            if i in self.kw_map.keys():
                #print i,self.kw_map[i], param[i]
                param[self.kw_map[i]] = param.pop(i)
        elem = etree.SubElement(contain,
                                getattr(self, "element_name", self._class_name),
                                **param)
        for child in self.childs:
            child.toXML(elem)
        return contain

    def add(self, child):
        self.childs.append(child)

def call(a):
    import pdb
    pdb.set_trace()



class RefBase(WixBase):
    pass
    #@classmethod
    #def setup(cls):
    #    import pdb
    #    pdb.set_trace()
    #    cls.kw_map['id'] = 'Id'


class InstallExecuteSequence(WixBase):
    @classmethod
    def setup(cls):
        pass

class RemoveExistingProducts(WixBase):
    @classmethod
    def setup(cls):
        cls.kw_static['After'] = "InstallFinalize"



class Base(WixBase):
    def __init__(self, *arg, **kw):
        self.filename = kw.pop('filename')
        super(Base, self).__init__(*arg, **kw)

    def save(self):
        fh = open(self.filename, 'w')
        fh.write(etree.tostring(self.toXML(), pretty_print=True))
        fh.write('\n')
        fh.close()

    def toXML(self, contain = Empty):
        elem = etree.Element('Wix', xmlns = 'http://schemas.microsoft.com/wix/2006/wi')
        for child in self.childs:
            child.toXML(elem)
        return elem

class Product(WixBase):
    @classmethod
    def setup(cls):
        cls.kw_map['name'] = 'Name'
        cls.kw_map['version'] = 'Version'
        cls.kw_map['language'] = 'Language'
        cls.kw_map['upgrade_code'] = 'UpgradeCode'
        cls.kw_map['manufacturer'] = 'Manufacturer'
        #cls.kw_map['id'] = 'Id'

        cls.kw_default['id'] = '*'
        cls.kw_default['language'] = '1033'


class Package(WixBase):
    @classmethod
    def setup(cls):
        cls.kw_static['InstallerVersion'] = '200'
        cls.kw_map['compressed'] = 'Compressed'
        cls.kw_map['manufacturer'] = 'Manufacturer'
        #cls.kw_static['Description'] = "Whatever"

class Module(WixBase):
    @classmethod
    def setup(cls):
        cls.kw_map['version'] = 'Version'
        cls.kw_map['language'] = 'Language'

class Fragment(WixBase):
    pass

class UIRef(WixBase):
    @classmethod
    def setup(cls):
        cls.kw_static['Id'] = "WixUI_FeatureTree"

class Media(WixBase):
    @classmethod
    def setup(cls):
        cls.kw_static['Id'] = '1'
        cls.kw_static['Cabinet'] = 'cs1.cab'
        cls.kw_static['EmbedCab'] = 'yes'


class Directory(WixBase):
    @classmethod
    def setup(cls):
        cls.kw_map['source_dir'] = 'FileSource'
        cls.kw_map['id'] = 'Id'
        cls.kw_map['name'] = 'Name'

class Feature(WixBase):
    @classmethod
    def setup(cls):
        cls.kw_map['id'] = 'Id'
        cls.kw_map['configurable_directory'] = 'ConfigurableDirectory'
        cls.kw_map['title'] = 'Title'
        cls.kw_map['level'] = 'Level'

        cls.kw_default['level'] = "1"
        cls.kw_default['title'] = ""

        cls.kw_convert['configurable_directory'] = lambda self, val: val.param['id']

class FeatureRef(RefBase):
    pass

class DirectoryRef(RefBase):
    pass

class Component(WixBase):
    component_genbase = uuid.UUID(gr['base'])
    @classmethod
    def setup(cls):
        cls.kw_map['id'] = 'Id'
        cls.kw_map['guid'] = 'Guid'
        cls.kw_map['directory'] = 'Directory'

        cls.kw_default_call['guid'] = lambda self: str(uuid.uuid3(self.component_genbase, self.param['id']))

class ComponentRef(RefBase):
    pass

class Property(RefBase):
    @classmethod
    def setup(cls):
        cls.kw_map['value'] = 'Value'
        cls.kw_map['secure'] = 'Secure'
        cls.kw_map['admin'] = 'Admin'

class Upgrade(WixBase):
    @classmethod
    def setup(cls):
        cls.kw_map['upgrade_code'] = 'Id'


class UpgradeVersion(WixBase):
    @classmethod
    def setup(cls):
        cls.kw_static['Property'] = 'PATCHFOUND'
        cls.kw_static['IncludeMinimum'] = 'yes'
        cls.kw_static['IncludeMaximum'] = 'yes'

        cls.kw_map['min_version'] = "Minimum"
        cls.kw_map['max_version'] = "Maximum"

        cls.kw_convert['keep_old'] = lambda self: 'yes'


class File(WixBase):
    @classmethod
    def setup(cls):
        cls.kw_map['id'] = 'Id'
        cls.kw_map['name'] = 'Name'
        cls.kw_map['source'] = 'Source'
        #cls.kw_static['Vital'] = 'yes'

class WixVariable(WixBase):
    @classmethod
    def setup(cls):
        cls.kw_map['id'] = 'Id'
        cls.kw_map['value'] = 'Value'

class RegistryValue(WixBase):
    @classmethod
    def setup(cls):
        cls.kw_map['id'] = 'Id'
        cls.kw_map['name'] = 'Name'
        cls.kw_map['key'] = 'Key'
        cls.kw_map['value'] = 'Value'
        cls.kw_map['action'] = 'Action'
        cls.kw_map['root'] = 'Root'
        cls.kw_map['type'] = 'Type'

        cls.kw_default['root'] = "HKLM"
        cls.kw_default['type'] = "string"
        cls.kw_default['action'] = "write"

class RegistrySearch(WixBase):
    @classmethod
    def setup(cls):
        cls.kw_map['id'] = 'Id'
        cls.kw_map['name'] = 'Name'
        cls.kw_map['key'] = 'Key'
        cls.kw_map['root'] = 'Root'
        cls.kw_map['type'] = 'Type'

        cls.kw_default['root'] = "HKLM"
        cls.kw_default['type'] = "directory"

class DirectorySearch(WixBase):
    @classmethod
    def setup(cls):
        cls.kw_map['id'] = 'Id'
        cls.kw_map['depth'] = 'Depth'
        cls.kw_map['path'] = 'Path'

class Merge(WixBase):
    @classmethod
    def setup(cls):
        cls.kw_map['id'] = 'Id'
        cls.kw_map['source_file'] = 'SourceFile'
        cls.kw_map['disk_id'] = 'DiskId'
        cls.kw_map['language'] = "Language"
        cls.kw_map['compressed'] = 'FileCompression'

        cls.kw_default['language'] = "1033"
        cls.kw_default['compressed'] = "yes"


class MergeRef(WixBase):
    pass



class Environment(WixBase):
    @classmethod
    def setup(cls):
        cls.kw_map['id'] = 'Id'
        cls.kw_map['action'] = 'Action'
        cls.kw_map['name'] = 'Name'
        cls.kw_map['part'] = 'Part'
        cls.kw_map['seperator'] = 'Seperator'
        cls.kw_map['value'] = 'Value'

        cls.kw_default['action'] = 'set'
        cls.kw_default['part'] = 'last'


def extract_version_information(directory):
    fh = open(directory + "/configure")
    package = 'PACKAGE_VERSION='
    len_proj = len(package)
    for line in fh.readlines():
        if len(line) > len_proj and line[:len_proj] == package:
            break
    return line[len_proj+1:-2]

def generate_directory_tree(directory, parent = Empty, prefix = Empty, create_dir = False, current = Empty, file_filter = Empty, directory_filter = Empty):
    """
    Recurses through a directory structure and subsequently adds every file and
    directory which is not negatively filtered to parent.

    parent =
    prefix = A prefix making generated ids unique
    current = Do we have a current directory, or do we need to generate it?
    directory_filter = Which directories should we visit. Argument needs to be
      a function which signals true/false after getting passed cur directry, to
      be visited directory as arguments
    file_filter = Which files should get included. Argument needs to be a
      function which signals true/false after getting passed cur directry,
      filename as arguments
    """
    if Empty in [parent, current, prefix]:
        raise TypeError('Missing paremeter')

    curdir = None
    if create_dir:
        prefix = prefix + '.' + os.path.basename(directory)
        curdir = Directory(parent = current, source_dir = directory, name = os.path.basename(directory), id = gen_wix_id_repeatable(prefix))
    else:
        curdir = Directory(parent = current, source_dir = directory, id = gen_wix_id_repeatable(prefix))
    #comp = Component(id = prefix + '.component')

    #entrycount = 0
    for entry in os.listdir(directory):
        if os.path.isfile(os.path.join(directory, entry)) and (file_filter == Empty or file_filter(directory, entry)):
            #c_id = prefix + "_" + os.path.join(directory, entry)
            comp = Component(parent = curdir, id = gen_wix_id_repeatable(prefix + '.' + entry))
            File(parent = comp, name = entry, id = gen_wix_id_repeatable(prefix + '.' + entry))
            ComponentRef(parent = parent,
                         id = gen_wix_id_repeatable(prefix + '.' + entry))

            #entrycount += 1
        elif os.path.isdir(os.path.join(directory, entry)) and (directory_filter == Empty or directory_filter(directory, entry)):
            generate_directory_tree(os.path.join(directory, entry),
                                    current = curdir,
                                    create_dir = True,
                                    parent = parent,
                                    prefix = prefix,
                                    file_filter = file_filter,
                                    directory_filter = directory_filter)

    #if entrycount:
    #    curdir.add(comp)
    #    ComponentRef(parent = parent,
    #                 id = prefix + '.component')

    return curdir

class DirectoryHelper:
    def __init__(self, parent, prefix, rootDir):
        self.parent = parent
        self.prefix = prefix
        self.rootDir = rootDir
        if not '_dirHelper_knownDirs' in parent.__dict__:
            parent.__dict__['_dirHelper_knownDirs'] = {}
    
    def GetDirectory(self, dir):
        if dir == None:
            return self.rootDir
        if dir in self.parent._dirHelper_knownDirs:
            return self.parent._dirHelper_knownDirs[dir]
        dirSplit = dir.rsplit ('/', 1)
        if len(dirSplit) == 1:
            pObj = self.rootDir
            name = dirSplit[0]
            realPrefix = self.prefix
        else:
            pObj = self.GetDirectory (dirSplit[0])
            name = dirSplit[1]
            realPrefix = self.prefix + '.' + dirSplit[0].replace('/', '.')
        newDir = Directory (parent = pObj,
                         name = name,
                         id = gen_wix_id_repeatable(realPrefix + '.' + name))
        self.parent._dirHelper_knownDirs[dir] = newDir
        return newDir

def generate_from_file_list(filelist, parent, prefix, dir):
    """
    Goes through a file list (as generated by CS' `jam filelists` command) and
    adds each file (and containing directory) to parent.
    
    filelist = List of files. Must be iterable, with the iterated items being
      lines as in a filelist file.
    parent =
    prefix = A prefix making generated ids unique
    """
    
    dirs = DirectoryHelper(parent, prefix, dir)
    
    for file in filelist:
        (sourcePath, destPath) = file.rsplit(':', 1)
        destSplit = destPath.rsplit ('/', 1)
        if len(destSplit) == 1:
            destFN = destSplit[0].strip()
            destDir = None
            realPrefix = prefix
        else:
            destDir = destSplit[0].strip()
            destFN = destSplit[1].strip()
            realPrefix = prefix + '.' + destDir.replace('/', '.')
        dirObj = dirs.GetDirectory (destDir)
        if destFN == "":
            # No file name part - create empty dir
            continue
        if os.path.isfile(sourcePath):
            wixID = gen_wix_id_repeatable(realPrefix + '.' + destFN)
            comp = Component(parent = dirObj, id = wixID)
            File(parent = comp, name = destFN, id = wixID, source = sourcePath)
            ComponentRef(parent = parent, id = wixID)
        elif os.path.isdir(sourcePath):
            # Source is actually a directory, add recursively
            generate_directory_tree(sourcePath,
                                    current = dirObj,
                                    create_dir = True,
                                    parent = parent,
                                    prefix = realPrefix)
        else:
            print "File list entry %s not found, packages may be incomplete" % sourcePath
            
def generate_from_file_list_txt(filelist_name, parent, prefix, dir):
    try:
        list = file(filelist_name, "r")
    except:
        list = None
        print "Could not open %s, packages may be incomplete" % filelist_name
    if list:
        generate_from_file_list(list,
                                parent = parent,
                                prefix = prefix,
                                dir = dir)

def wixify_string(s):
    return s.replace('-', '_').replace(' ', '_').replace(',', '_')

def wixify_string_and_shorten(s, cur):
    """
    This is quite ugly and ad hoc.
    Compress the filenames to fit into the stupid `Id` length requirement.
    A nicer solution is needed, but in most cases we now dont need to calculate
    an sha based id.
    """
    return md5.md5(s).hexdigest()
    curlen = len(cur) + 1
    s = wixify_string(s)
    if len(s) + curlen >= 72:
        #s = re.sub('(.*)_([0-9a-f]{32})(\.[a-z]*)', r'\1.\3', s)
        s = s.replace('graph', 'gr')
        s = s.replace('source', 'sr')
        s = s.replace('struct', 'st')
        s = s.replace('class', 'c')
        s = s.replace('const', 'c')
        s = s.replace('char', 'c')
        s = s.replace('inherit', 'i')
        s = s.replace('members', 'm')
        s = s.replace('.html', '.h')
        s = s.replace('crystalspace', 'cs')
        s = s.replace('lightmap', 'lm')
        s = s.replace('crystalspace', 'cs')
        s = s.replace('Plugin', 'plg')
        s = s.replace('Common', 'cmn')
        s = s.replace('Shader', 'shd')
        s = s.replace('Comparator', 'cmp')
        s = s.replace('Default', 'def')
        s = s.replace('Scanline', 'scl')
        s = s.replace('Render', 'rn')
        s = s.replace('Implementation', 'impl')
        s = s.replace('System', 'sys')
        s = s.replace('Stream', 'str')
        s = s.replace('Notification', 'notif')
        s = s.replace('Interpolate', 'intrp')
        s = s.replace('Utility', 'util')
        s = s.replace('Container', 'cont')
        s = s.replace('Upcase', 'uc')
        s = s.replace('Downcase', 'lc')
        if len(s) + curlen > 72:
            olds = s
            s = md5.md5(s).hexdigest()
            sys.stderr.write('shortening filename because it trips the limit: \n'+
                             '\t' + olds + "\n"
                             '\twith a len of ' + str(curlen + len(olds)) + "\n" +
                             '\treplacing with ' + s + "\n"
                             '\ttill now id is ' + cur + "\n")
        #s = s.replace('__', '_')
        #s = s[:20]
    return s

def gen_wix_id_repeatable(s):
    """
    Generates a Id which fits to WIX's requirements for ids. It
    creates the same id for the same input.
    """
    #return '_' + sha.sha(str(s)).hexdigest()
    return '_' + md5.md5(s).hexdigest()[:-1]

def default_directory_filter(directory, entry):
    #print entry
    if entry in [".svn", 'CVS']:
        return False
    return True

def generate_merge_module(path = [], out = '.', id = Empty, manufacturer = "", Language = 1033, guid = Empty, version = Empty, language = '1033', filename = None):
    if Empty in [id, guid, version]:
        raise TypeError('Missing parameter')
    if not filename:
        filename = out + os.path.sep + id + '.wxs'
    base = Base(filename = filename)
    module = Module(
        parent = base,
        version = version,
        language = language,
        id = id
        )
    package = Package(
        parent = module,
        id = guid,
        manufacturer = manufacturer
    )

    p = module
    for i in path:
        (c_id, c_name, c_source) = i
        param = {"parent": p,
                 "id": c_id}
        if c_source:
            param['source_dir'] = c_source
        if c_name:
            param['name'] = c_name

        p = Directory(
            **param)

    return (base, module, p)

def merge_merge_module(id = Empty, source_file = Empty, feature = Empty, directory = Empty, media = '1', guid = Empty):
    if Empty in [source_file, feature, directory, id, guid]:
        raise TypeError('Missing parameter')
    comb_id = id + '.' + guid
    Merge(parent = directory,
          id = comb_id,
          source_file = source_file,
          disk_id = media)
    MergeRef(
        parent = feature,
        id = comb_id)
