#!/usr/bin/env python
import uuid
import os
import sha
import md5
import re
import sys
import tempfile
import subprocess


from lxml import etree

config = {}
config['msidb'] = 'msidb'
config['dark'] = 'dark'
config['msiexec'] = 'msiexec'
config['light'] = 'light'
config['candle'] = 'candle'

config['python.msi'] = sys.argv[1]

temp = ""
wxs = "a"
data = ""

def changeparentguid(f):
    parent = f.xpath('ancestor-or-self::wxs:*[@Guid]', namespaces = namespacemap)
    #print f, len(parent), f.attrib['Id']
    assert(len(parent) == 1)
    parent = parent[0]
    parent.attrib['Guid'] = str(uuid.UUID(parent.attrib['Guid'])).upper()

def tag_or_action(f):
    schemalen = len(root[0].nsmap[None])+2
    name = ""
    #print f.tag[schemalen:]
    if f.tag[schemalen:] == 'Custom':
        name = f.attrib['Action']
    else:
        name = f.tag[schemalen:]
    return name

def make_relative(root):
    schemalen = len(root[0].nsmap[None])+2
    for f in root.xpath('wxs:*[@Sequence]', namespaces = namespacemap):
        cur_sequence = f.attrib['Sequence']

        #Search relatively earlier Sequence.
        earlier = list(root.xpath('wxs:*[@Sequence < %s]' % cur_sequence, namespaces = namespacemap))
        earlier.sort(key = lambda x: x.attrib['Sequence'])
        if len(earlier):
            f.attrib['After'] = tag_or_action(earlier[-1])
        else:
            later = list(root.xpath('wxs:*[@Sequence > %s]' % cur_sequence, namespaces = namespacemap))
            later.sort(key = lambda x: x.attrib['Sequence'])
            f.attrib['Before'] = tag_or_action(later[0])

    for f in root.xpath('wxs:*[@Sequence]', namespaces = namespacemap):
        f.attrib.pop('Sequence')


def prepare_work():
    #temp = r'c:\dokume~1\admini~1\lokale~1\temp\tmproz3fzcrystal'
    #wxs = r'C:\DOKUME~1\ADMINI~1\LOKALE~1\Temp\tmproz3fzcrystal\python.wxs'
    #data = r'C:\DOKUME~1\ADMINI~1\LOKALE~1\Temp\tmproz3fzcrystal\data'

    #return (temp, wxs, data)

    temp = tempfile.mkdtemp("crystal")

    print "Working in Directory: " + temp

    wxs = temp + os.path.sep + "python.wxs"
    data = temp + os.path.sep + 'data'
    os.mkdir(data)

    subprocess.call([config['dark'],
                     "-o", wxs,
                     '-x', data + os.path.sep + "SourceDir",
                     "-nologo",
                     config['python.msi']]
                    )

    #msiexec only read files in current directory
    curpath = os.getcwd()
    os.chdir(os.path.dirname(config['python.msi']))
    print os.path.dirname(config['python.msi'])
    subprocess.call(['cmd', '/c',
                     'start', '/wait',
                     config['msiexec'],
                     '/qn',
                     "/a", config['python.msi'],
                     'TARGETDIR=' + data + os.path.sep + 'Dup']
                    )
    os.chdir(curpath)

    return (temp, wxs, data)

namespacemap = {'wxs': "http://schemas.microsoft.com/wix/2006/wi"}

if __name__ == '__main__':
    (temp, wxs, data) = prepare_work()

    et = etree.parse(wxs)
    root = et.getroot()

    ###
    # Remove Errors in Wix file.
    ###

    #Remove deprecated ShortName attributes
    for f in root.xpath('//wxs:File', namespaces = namespacemap):
        f.attrib.pop('ShortName', None)

    for f in root.xpath('//wxs:Directory', namespaces = namespacemap):
        f.attrib.pop('ShortName', None)

    for f in root.xpath('//wxs:Component[@Id="REGISTRY.tcl"]', namespaces = namespacemap):
        f.getparent().remove(f)

    #File Id's are not case sensitive on the filesystem, but are in the cab file... Not very nice.
    for f in root.xpath('//wxs:Component[@Id="TARGETDIR"]/wxs:File[@Id="README.TXT"]', namespaces = namespacemap):
        f.attrib['Id'] = r"TARGETDIR_README.TXT"
        f.attrib['Source'] = r"SourceDir\Dup\README.TXT"
        changeparentguid(f)

    for f in root.xpath('//wxs:Component[@Id="test"]/wxs:File[@Id="readme.txt"]', namespaces = namespacemap):
        f.attrib['Id'] = r"test_readme.txt"
        f.attrib['Source'] = r"SourceDir\Dup\Lib\test\readme.txt"
        changeparentguid(f)

    for f in root.xpath('//wxs:Component[@Id="idlelib"]/wxs:File[@Id="FileList.py"]', namespaces = namespacemap):
        f.attrib['Id'] = r"idlelib_FileList.py"
        f.attrib['Source'] = r"SourceDir\Dup\Lib\idlelib\FileList.py"
        changeparentguid(f)

    for f in root.xpath('//wxs:Component[@Id="distutils"]/wxs:File[@Id="filelist.py"]', namespaces = namespacemap):
        f.attrib['Id'] = r"distutils_filelist.py"
        f.attrib['Source'] = r"SourceDir\Dup\Lib\distutils\FileList.py"
        changeparentguid(f)

    for f in root.xpath('//wxs:Component[@Id="tk8.4"]/wxs:File[@Id="console.tcl"]', namespaces = namespacemap):
        f.attrib['Id'] = r"tk8.4_console.tcl"
        f.attrib['Source'] = r"SourceDir\Dup\tcl\tk8.4\console.tcl"
        changeparentguid(f)

    for f in root.xpath('//wxs:Component[@Id="tix8.4"]/wxs:File[@Id="Console.tcl"]', namespaces = namespacemap):
        f.attrib['Id'] = r"tix8.4_Console.tcl"
        f.attrib['Source'] = r"SourceDir\Dup\tcl\tix8.4\console.tcl"
        changeparentguid(f)

    for f in root.xpath('//wxs:Component[@Id="demos"]/wxs:File[@Id="filebox.tcl"]', namespaces = namespacemap):
        f.attrib['Id'] = r"demos_filebox.tcl"
        f.attrib['Source'] = r"SourceDir\Dup\tcl\tk8.4\demos\filebox.tcl"
        changeparentguid(f)

    for f in root.xpath('//wxs:Component[@Id="tix8.4"]/wxs:File[@Id="FileBox.tcl"]', namespaces = namespacemap):
        f.attrib['Id'] = r"tix8.4_FileBox.tcl"
        f.attrib['Source'] = r"SourceDir\Dup\tcl\tix8.4\FileBox.tcl"
        changeparentguid(f)

    for f in root.xpath('//wxs:Component[@Id="tk8.4"]/wxs:File[@Id="optMenu.tcl"]', namespaces = namespacemap):
        f.attrib['Id'] = r"tk8.4_optMenu.tcl"
        f.attrib['Source'] = r"SourceDir\Dup\tcl\tk8.4\optMenu.tcl"
        changeparentguid(f)

    for f in root.xpath('//wxs:Component[@Id="tix8.4"]/wxs:File[@Id="OptMenu.tcl"]', namespaces = namespacemap):
        f.attrib['Id'] = r"tix8.4_OptMenu.tcl"
        f.attrib['Source'] = r"SourceDir\Dup\tcl\tix8.4\OptMenu.tcl"
        changeparentguid(f)

    for f in root.xpath('//wxs:Component[@Id="samples"]/wxs:File[@Id="ListNBK.tcl"]', namespaces = namespacemap):
        f.attrib['Id'] = r"samples_ListNBK.tcl"
        f.attrib['Source'] = r"SourceDir\Dup\tcl\tix8.4\demos\samples\ListNBK.tcl"
        changeparentguid(f)

    for f in root.xpath('//wxs:Component[@Id="tix8.4"]/wxs:File[@Id="ListNBk.tcl"]', namespaces = namespacemap):
        f.attrib['Id'] = r"tix8.4_ListNBk.tcl"
        f.attrib['Source'] = r"SourceDir\Dup\tcl\tix8.4\ListNBk.tcl"
        changeparentguid(f)

    for f in root.xpath('//wxs:Component[@Id="tix8.4"]/wxs:File[@Id="Init.tcl"]', namespaces = namespacemap):
        f.attrib['Id'] = r"tix8.4_Init.tcl"
        f.attrib['Source'] = r"SourceDir\Dup\tcl\tix8.4\Init.tcl"
        changeparentguid(f)

    for f in root.xpath('//wxs:Component[@Id="tk8.4"]/wxs:File[@Id="init.tcl"]', namespaces = namespacemap):
        f.attrib['Id'] = r"tk8.4_init.tcl"
        f.attrib['Source'] = r"SourceDir\Dup\tcl\tk8.4\init.tcl"
        changeparentguid(f)


    #Advertised Shortcuts need real keypath -> ie python.exe itself
    elem_from = root.xpath('//wxs:Shortcut[@Id="Uninstall"]', namespaces = namespacemap)[0]
    changeparentguid(elem_from)
    elem_from.getparent().remove(elem_from)

    elem_to = root.xpath('//wxs:Component[@Id = "python.exe"]', namespaces = namespacemap)[0]
    elem_to.append(elem_from)
    changeparentguid(elem_to)

    #Wrong KeyPath error again.
    elem_from = root.xpath('//wxs:Shortcut[@Id = "Manual"]', namespaces = namespacemap)[0]
    changeparentguid(elem_from)
    elem_from.getparent().remove(elem_from)
    elem_from.attrib.pop('Target')
    elem_from.attrib['Advertise'] = 'yes'

    elem_to = root.xpath('//wxs:File[@Id = "Python25.chm"]', namespaces = namespacemap)[0]
    elem_to.append(elem_from)
    changeparentguid(elem_to)

    #Reduces warnings due to wrong KeyPath
    elem_from = root.xpath('//wxs:Component[@Id = "REGISTRY.def"]', namespaces = namespacemap)[0]
    elem_from.attrib.pop('KeyPath')
    changeparentguid(elem_from)

    elem_to = elem_from.xpath('./wxs:RegistryValue[@Id = "AppPaths"]', namespaces = namespacemap)[0]
    elem_to.attrib['KeyPath'] = 'yes'
    changeparentguid(elem_to)

    ###
    # Convert to merge module
    ###
    module = root.xpath('/wxs:Wix/wxs:Product', namespaces = namespacemap)[0]
    module.tag = 'Module'
    module.attrib.pop('Manufacturer')
    module.attrib.pop('Name')
    module.attrib.pop('UpgradeCode')
    product_id = module.attrib['Id']
    module.attrib['Id']= 'python'

    package = module.xpath('./wxs:Package', namespaces = namespacemap)[0]
    package.attrib.pop('Compressed')
    package.attrib['Id'] = str(uuid.UUID(product_id))
    print "merge module id: %s" % package.attrib['Id']

    for f in root.xpath('//wxs:*[@DiskId]', namespaces = namespacemap):
        f.attrib.pop("DiskId")

    for f in root.xpath('//wxs:Feature', namespaces = namespacemap):
        f.getparent().remove(f)

    for f in root.xpath('//wxs:Media', namespaces = namespacemap):
        f.getparent().remove(f)

    for f in root.xpath('//wxs:Upgrade', namespaces = namespacemap):
        f.getparent().remove(f)


    make_relative(root.xpath('//wxs:InstallExecuteSequence', namespaces = namespacemap)[0])
    make_relative(root.xpath('//wxs:InstallUISequence', namespaces = namespacemap)[0])
    make_relative(root.xpath('//wxs:AdminExecuteSequence', namespaces = namespacemap)[0])
    make_relative(root.xpath('//wxs:AdminUISequence', namespaces = namespacemap)[0])
    #make_relative(root.xpath('//wxs:AdvertiseExecuteSequence', namespaces = namespacemap)[0])

    for f in root.xpath('//wxs:InstallExecuteSequence/wxs:*[not(@Action)]', namespaces = namespacemap):
        f.getparent().remove(f)

    for f in root.xpath('//wxs:InstallUISequence/wxs:*[not(@Action)]', namespaces = namespacemap):
        f.getparent().remove(f)

    for f in root.xpath('//wxs:AdminExecuteSequence/wxs:*[not(@Action)]', namespaces = namespacemap):
        f.getparent().remove(f)

    for f in root.xpath('//wxs:AdminUISequence/wxs:*[not(@Action)]', namespaces = namespacemap):
        f.getparent().remove(f)

    for f in root.xpath('//wxs:AdvertiseExecuteSequence/wxs:*[not(@Action)]', namespaces = namespacemap):
        f.getparent().remove(f)

    for f in root.xpath('//wxs:UI', namespaces = namespacemap):
        f.getparent().remove(f)

    for f in root.xpath('//wxs:Property[@Id = "ErrorDialog" or @Id="Progress1" or @Id="Progress2" or @Id="ProductLine" or @Id="DefaultUIFont"]', namespaces = namespacemap):
        f.getparent().remove(f)

    ##TODO: UpdateEditIDLE uses REGISTRY.tcl
    for f in root.xpath('//wxs:Custom[@Action = "UpdateEditIDLE"]', namespaces = namespacemap):
        f.getparent().remove(f)

    newwxsname = temp + os.path.sep + "python_new.wxs"
    newwxs = open(newwxsname, "w")
    newwxs.write(etree.tostring(root, pretty_print = True))
    newwxs.close()

    subprocess.call([config['candle'],
                     "-o", "out/msi/temp/python.wixobj",
                     "-nologo",
                     newwxsname]
                    )
    subprocess.call([config['light'],
                     "-o", "out/msi/python.msm",
                     "-nologo",
                     "-b", data,
                     "python.wixobj"]
                    )
