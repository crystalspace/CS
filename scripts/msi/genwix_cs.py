#!/usr/bin/python
from wixlib import *

from genwix_cel import gen_cel_feature_tree
#from genwix_b2cs import gen_cel_feature_tree

import time
import datetime

from csguidregistry import guid_registry as gr
version = extract_version_information('/home/andres/Projects/CS')
version_stringed = version.replace('.', '_')
version_full = version + '.3'
outdir = '/mnt/vm_1/work'
#outdir = "/tmp"

def gen_cs_doc_msm():
    (base, merge, dir) = generate_merge_module(id = 'CS.doc',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('CSINSTALLDIR', 'CrystalSpace', None),
                                                       ('CSVERSION' + version, version, 'CS')
                                                       ],
                                               out = outdir,
                                               manufacturer = "The Crystal Space Project",
                                               guid = gr['CS.doc.msm'],
                                               version = version_full)
    f = Fragment(
        parent = base,
        id = 'CS.doc')

    ref = DirectoryRef(
        parent = f,
        id = dir.kw['id'])

    doc_d = Directory(
        parent = ref,
        id = 'CS.doc',
        name = "docs")

    man_d = Directory(
        parent = doc_d,
        id = 'CS.doc.man_d',
        name = "manual")

    generate_directory_tree("docs/html/manual",
                            parent = merge,
                            current = man_d,
                            prefix = "CS.doc.manual",
                            directory_filter = default_directory_filter,
                            file_filter = lambda d, f: f != 'Jamfile')

    api_d = Directory(
        parent = doc_d,
        id = 'CS.doc.api_d',
        name = "api")


    generate_directory_tree("docs/html/api",
                            parent = merge,
                            current = api_d,
                            prefix = "CS.doc.api",
                            directory_filter = default_directory_filter,
                            file_filter = lambda d, f: f != 'Jamfile')

    base.save()



def gen_cs_vfs_msm():
    (base, merge, dir) = generate_merge_module(id = 'CS.vfs',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('CSINSTALLDIR', 'CrystalSpace', None),
                                                       ('CSVERSION' + version, version, 'CS')
                                                    ],
                                               out = outdir,
                                               manufacturer = "The Crystal Space Project",
                                               guid = gr['CS.vfs.msm'],
                                               version = version_full)

    vfs_c = Component(
        parent = dir,
        id = "CS.vfs.comp")

    File(
        parent = vfs_c,
        id = "CS.vfs.vfs.cfg",
        name="vfs.cfg")

    base.save()


def gen_cs_register_msm():
    (base, merge, dir) = generate_merge_module(id = 'CS.register',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('CSINSTALLDIR', 'CrystalSpace', None),
                                                       ('CSVERSION' + version, version, 'CS')
                                                    ],
                                               out = outdir,
                                               manufacturer = "The Crystal Space Project",
                                               guid = gr['CS.register.msm'],
                                               version = version_full)

    register_c = Component(
        parent = dir,
        id = 'CS.register.')

    RegistryValue(
        parent = register_c,
        id = "CS.register.reg",
        key = "SOFTWARE\\CrystalSpace\\"+ version,
        name = 'InstallPath',
        type='string',
        value="[CSVERSION" + version +']')

    Environment(
        parent = register_c,
        id = "CS.register.env",
        name = 'CRYSTAL_' + version_stringed,
        value="[CSVERSION" + version +']'
    )
    base.save()

def gen_cs_data_msm():
    (base, merge, dir) = generate_merge_module(id = 'CS.data',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('CSINSTALLDIR', 'CrystalSpace', None),
                                                       ('CSVERSION' + version, version, 'CS')
                                                       ],
                                               out = outdir,
                                               manufacturer = "The Crystal Space Project",
                                               guid = gr['CS.data.msm'],
                                               version = version_full)

    frag = Fragment(
        parent = base,
        id = 'CS.data')

    ref = DirectoryRef(
        parent = frag,
        id = dir.kw['id'])

    generate_directory_tree("data",
                            create_dir = True,
                            current = ref,
                            prefix = 'CS.data',
                            parent = merge,
                            directory_filter = default_directory_filter,
                            file_filter = lambda d, f: f != 'Jamfile')

    base.save()


def gen_cs_include_msm():
    (base, merge, dir) = generate_merge_module(id = 'CS.include',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('CSINSTALLDIR', 'CrystalSpace', None),
                                                       ('CSVERSION' + version, version, 'CS')
                                                       ],
                                               out = outdir,
                                               manufacturer = "The Crystal Space Project",
                                               guid = gr['CS.include.msm'],
                                               version = version_full)

    frag = Fragment(
        parent = base,
        id = 'CS.include')

    ref = DirectoryRef(
        parent = frag,
        id = dir.kw['id'])


    generate_directory_tree("include",
                            current = ref,
                            prefix = "CS.include",
                            create_dir = True,
                            parent = merge,
                            directory_filter = default_directory_filter,
                            file_filter = lambda d, f: f[-2:] == '.h' and not (d == "include" and f == "csconfig.h"))

    base.save()

def gen_cs_arch_msms(arch_name, arch_dir):
    gen_cs_arch_include_msm(arch_name, arch_dir)
    gen_cs_arch_executable_msm(arch_name, arch_dir)
    gen_cs_arch_lib_msm(arch_name, arch_dir)
    gen_cs_arch_support_msm(arch_name, arch_dir)

def gen_cs_arch_include_msm(arch_name, arch_dir):
    (base, merge, dir) = generate_merge_module(id = 'CS.arch.' + arch_name + '.include',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('CSINSTALLDIR', 'CrystalSpace', None),
                                                       ('CSVERSION' + version, version, None),
                                                       ('CS.arch.' + arch_name, arch_name, arch_dir),
                                                       ('CS.arch.' + arch_name + '.include', 'include', 'include')
                                                       ],
                                               out = outdir,
                                               manufacturer = "The Crystal Space Project",
                                               guid = gr['CS.arch.'+arch_name + '.include.msm'],
                                               version = version_full)

    c_incl_comp = Component(
        parent = dir,
        id = 'CS.arch.' + arch_name + '.include')
    File(
        parent = c_incl_comp,
        id = 'CS.arch.' + arch_name + '.include.' + 'csconfig.h',
        name = "csconfig.h")
    base.save()


def gen_cs_arch_lib_msm(arch_name, arch_dir):
    (base, merge, dir) = generate_merge_module(id = 'CS.arch.' + arch_name + '.lib',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('CSINSTALLDIR', 'CrystalSpace', None),
                                                       ('CSVERSION' + version, version, None),
                                                       ('CS.arch.' + arch_name, arch_name, arch_dir),
                                                       ('CS.arch.' + arch_name + '.lib', 'bin', arch_dir)
                                                       ],
                                               out = outdir,
                                               manufacturer = "The Crystal Space Project",
                                               guid = gr['CS.arch.'+arch_name + '.lib.msm'],
                                               version = version_full)
    frag = Fragment(
        parent = base,
        id = 'CS.arch.' + arch_name + '.lib')

    ref = DirectoryRef(
        parent = frag,
        id = dir.kw['id'])

    register_c = Component(
        parent = dir,
        id = 'CS.arch.' + arch_name + '.register.c')

    Environment(
        parent = register_c,
        id = 'CS.arch.' + arch_name + '.register.path',
        name = 'PATH',
        value='[CS.arch.' + arch_name + '.lib]'
        )
    #Environment(
    #    parent = register_c,
    #    id = 'CS.arch.' + arch_name + '.register.crystal',
    #    name = 'CRYSTAL_' + version_stringed,
    #    value='[CS.arch.' + arch_name + '.lib]'
    #    )

    #File(
    #    parent = register_c,
    #    id = "CS.arch." + arch_name + ".vfs.cfg",
    #    name = "vfs.cfg",
    #    )


    generate_directory_tree(arch_dir,
                            parent = merge,
                            prefix = 'CS.arch.' + arch_name + '.lib',
                            current = ref,
                            directory_filter = lambda d, f: False,
                            file_filter = lambda d, f: f[-4:] in ['.dll', '.pyd'])

    base.save()

def gen_cs_arch_executable_msm(arch_name, arch_dir):
    (base, merge, dir) = generate_merge_module(id = 'CS.arch.' + arch_name + '.executable',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('CSINSTALLDIR', 'CrystalSpace', None),
                                                       ('CSVERSION' + version, version, None),
                                                       ('CS.arch.' + arch_name, arch_name, arch_dir),
                                                       ('CS.arch.' + arch_name + '.executable', 'bin', '.')
                                                       ],
                                               out = outdir,
                                               manufacturer = "The Crystal Space Project",
                                               guid = gr['CS.arch.'+arch_name + '.executable.msm'],
                                               version = version_full)

    frag = Fragment(
        parent = base,
        id = 'CS.arch.' + arch_name + '.executable')

    ref = DirectoryRef(
        parent = frag,
        id = dir.kw['id'])

    generate_directory_tree(arch_dir,
                            parent = merge,
                            prefix = 'CS.arch.' + arch_name + '.bin',
                            current = ref,
                            directory_filter = lambda d, f: False,
                            file_filter = lambda d, f: f[-4:] in ['.exe', 'py', 'pyd'] or f == 'cs-config-' + version)

    base.save()


def gen_cs_arch_support_msm(arch_name, arch_dir):
    (base, merge, dir) = generate_merge_module(id = 'CS.arch.' + arch_name + '.support',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('CSINSTALLDIR', 'CrystalSpace', None),
                                                       ('CSVERSION' + version, version, None),
                                                       ('CS.arch.' + arch_name, arch_name, arch_dir),
                                                       ('CS.arch.' + arch_name + '.support', 'bin', '.')
                                                       ],
                                               out = outdir,
                                               manufacturer = "The Crystal Space Project",
                                               guid = gr['CS.arch.'+arch_name + '.support.msm'],
                                               version = version_full)

    frag = Fragment(
        parent = base,
        id = 'CS.arch.' + arch_name + '.support')

    ref = DirectoryRef(
        parent = frag,
        id = dir.kw['id'])

    generate_directory_tree(arch_dir + '/lib',
                            parent = merge,
                            prefix = 'CS.arch.' + arch_name + '.support',
                            current = ref,
                            directory_filter = lambda d, f: False,
                            file_filter = lambda d, f: f[-4:] in ['.dll', 'fest'])

    base.save()


def gen_cs_feature_tree(directory = Empty, base = Empty):
    cs = Feature(
        parent = base,
        id = "CS",
        title = "Crystal Space",
        configurable_directory = DirectoryRef(id  = 'CSVERSION' + version)
        )

    merge_merge_module(id = 'CS.register',
                       guid = gr['CS.register.msm'],
                       source_file = 'CS.register.msm',
                       feature = cs,
                       directory = directory)

    merge_merge_module(id = 'CS.vfs',
                       guid = gr['CS.vfs.msm'],
                       source_file = 'CS.vfs.msm',
                       feature = cs,
                       directory = directory)

    include = Feature(
        parent = cs,
        id = 'CS.include',
        title = "Headers")

    merge_merge_module(id = 'CS.include',
                       guid = gr['CS.include.msm'],
                       source_file = 'CS.include.msm',
                       feature = include,
                       directory = directory)

    data = Feature(
        parent = cs,
        id = 'CS.data',
        title = "Data")

    merge_merge_module(id = 'CS.data',
                       guid = gr['CS.data.msm'],
                       source_file = 'CS.data.msm',
                       feature = data,
                       directory = directory)
    doc = Feature(
        parent = cs,
        id = 'CS.doc',
        title = "Documentation")

    merge_merge_module(id = 'CS.doc',
                       guid = gr['CS.doc.msm'],
                       source_file = 'CS.doc.msm',
                       feature = doc,
                       directory = directory)

    arch_all = Feature(
        parent = cs,
        id = 'CS.arch',
        title = "Binaries")

    for arch in ['GCC']:
        cur_arch = Feature(
            parent = arch_all,
            id = 'CS.arch.'+arch,
            title = "Binaries compiled with " + arch)

        lib = Feature(
            parent = cur_arch,
            id = 'CS.arch.'+arch + '.lib',
            title = "Libraries")

        merge_merge_module(id = 'CS.arch.' + arch + '.lib',
                           guid = gr['CS.arch.'+ arch + '.lib.msm'],
                           source_file = 'CS.arch.'+ arch + '.lib.msm',
                           feature = lib,
                           directory = directory)

        include = Feature(
            parent = cur_arch,
            id = 'CS.arch.'+arch + '.include',
            title = "Headers")

        merge_merge_module(id = 'CS.arch.' + arch + '.include',
                           guid = gr['CS.arch.'+ arch + '.include.msm'],
                           source_file = 'CS.arch.'+ arch + '.include.msm',
                           feature = include,
                           directory = directory)

        executable = Feature(
            parent = cur_arch,
            id = 'CS.arch.'+arch + '.executable',
            title = "Application")

        merge_merge_module(id = 'CS.arch.' + arch + '.executable',
                           guid = gr['CS.arch.'+ arch + '.executable.msm'],
                           source_file = 'CS.arch.'+ arch + '.executable.msm',
                           feature = executable,
                           directory = directory)

        support = Feature(
            parent = cur_arch,
            id = 'CS.arch.'+arch + '.support',
            title = "Support Libraries")

        merge_merge_module(id = 'CS.arch.' + arch + '.support',
                           guid = gr['CS.arch.'+ arch + '.support.msm'],
                           source_file = 'CS.arch.'+ arch + '.support.msm',
                           feature = support,
                           directory = directory)


def gen_cs_msi_base(fname = Empty):
    base = Base(filename = outdir + os.path.sep + fname)
    product= Product(
        parent = base,
        version = version_full,
        name = 'Crystal Space ' + version,
        upgrade_code = gr['CS.upgrade_code'],
        manufacturer = "The Crystal Space Project"
    )
    package = Package(parent = product)
    Media(parent = product)
    UIRef(parent = product)
    WixVariable(
        id = "WixUILicenseRtf",
        value = "lgpl.rtf",
        parent = product)

    install_execute_sequence = InstallExecuteSequence(
        parent = product)

    RemoveExistingProducts(
        parent = install_execute_sequence)


    targetdir =  Directory(
        parent = product,
        id = "TARGETDIR",
        name = "SourceDir")

    pfiles =  Directory(
        parent = DirectoryRef(parent = product,
                              id = targetdir.kw['id']),
        id = "ProgramFilesFolder",
        name = "PFiles")

    csinstalldir =  Directory(
        parent = DirectoryRef(parent = product,
                              id = pfiles.kw['id']),
        id = "CSINSTALLDIR",
        name = "CrystalSpace")


    csversiondir = Directory(
        parent = DirectoryRef(parent = product,
                              id = csinstalldir.kw['id']),
        id = "CSVERSION" + version,
        name = version)

    old_installdir_prop = Property(
        parent = product,
        id = "CSVERSION" + version)

    RegistrySearch(
        parent = old_installdir_prop,
        id = "CS.old_installdir_prop",
        key = "SOFTWARE\\CrystalSpace\\" + version, name = "InstallPath")

    upgrade_cs = Upgrade(
        parent = product,
        upgrade_code = gr['CS.upgrade_code'])

    UpgradeVersion(
        parent = upgrade_cs,
        min_version = '1.9.0',
        max_version = version_full
    )

    gen_cs_feature_tree(directory = csversiondir, base = product)

    return (base, product, pfiles, csversiondir)


def gen_cs_msi():
    (base, product, pfiles, csversiondir) = gen_cs_msi_base(fname = 'CS.wxs')
    base.save()


def gen_cs_cel_msi_base(fname = Empty):
    (base, product, pfiles, csversiondir) = gen_cs_msi_base(fname = fname)

    celinstalldir =  Directory(
        parent = DirectoryRef(parent = product,
                              id = pfiles.kw['id']),
        id = "CELINSTALLDIR",
        name = "Cel")


    celversiondir = Directory(
        parent = DirectoryRef(parent = product,
                              id = celinstalldir.kw['id']),
        id = "CELVERSION" + version,
        name = version)


    old_installdir_prop = Property(
        parent = product,
        id = "CELVERSION" + version)

    RegistrySearch(
        parent = old_installdir_prop,
        id = "cel.old_installdir_prop",
        key = "SOFTWARE\\Cel\\" + version,
        name = "InstallPath")

    upgrade_cel = Upgrade(
        parent = product,
        upgrade_code = gr['cel.upgrade_code'])

    UpgradeVersion(
        parent = upgrade_cel,
        min_version = '1.9.0',
        max_version = version_full
    )

    gen_cel_feature_tree(directory = celversiondir, base = product)

    return (base, product, pfiles, csversiondir)

def gen_cs_cel_msi():
    (base, product, pfiles, csversiondir) = gen_cs_cel_msi_base(fname = 'CS_cel.wxs')
    base.save()

def gen_cs_cel_b2cs_msi_base(fname = Empty):
    (base, product, pfiles, csversiondir) = gen_cs_cel_msi_base(fname = fname)


    b2cs_installdir_prop = Property(
        parent = product,
        id = "BLENDERINSTALLDIR." + gr['b2cs.blender.plugin.msm'].replace('-','_'),
        admin = "yes"
        )

    blenderfound =  Directory(
        parent = DirectoryRef(parent = product,
                              id = pfiles.kw['id']),
        id = "BLENDERFOUND." + gr['b2cs.blender.plugin.msm'].replace('-','_'),
        name = "Blender Foundation")

    blenderinstalldir = Directory(
        parent = blenderfound,
        id = "BLENDERINSTALLDIR." + gr['b2cs.blender.plugin.msm'].replace('-','_'),
        name = "Blender")

    blenderhomedir = Directory(
        parent = blenderinstalldir,
        id = "BLENDERHOMEDIR." + gr['b2cs.blender.plugin.msm'].replace('-','_'),
        name = ".blender")

    blenderscripts = Directory(
        parent = blenderhomedir,
        id = "BLENDERSCRIPTS." + gr['b2cs.blender.plugin.msm'].replace('-','_'),
        name = 'scripts')

    b2cs = Feature(
        parent = product,
        id = "b2cs",
        title = "blender2crystal",
        configurable_directory = DirectoryRef(id = 'BLENDERSCRIPTS.' + gr['b2cs.blender.plugin.msm'].replace('-','_'))
        )

    b2cs_blender = Feature(
        parent = b2cs,
        id = "b2cs.blender.plugin",
        title = "Blender Plugin",
        )

    upgrade_b2cs = Upgrade(
        parent = product,
        upgrade_code = gr['b2cs.upgrade_code'])

    UpgradeVersion(
        parent = upgrade_b2cs,
        min_version = '1.9.0',
        max_version = version_full
    )

    merge_merge_module(id = 'b2cs.blender.plugin',
                       guid = gr['b2cs.blender.plugin.msm'],
                       source_file = 'b2cs.blender.plugin.msm',
                       feature = b2cs_blender,
                       directory = DirectoryRef(parent = product,
                                                id  = 'BLENDERSCRIPTS.' + gr['b2cs.blender.plugin.msm'].replace('-','_')))

    return (base, product, pfiles, csversiondir)

def gen_cs_cel_b2cs_msi():
    (base, product, pfiles, csversiondir) = gen_cs_cel_b2cs_msi_base(fname = 'CS_cel_b2cs.wxs')
    base.save()

def gen_cs_cel_b2cs_python_msi_base(fname = Empty):
    (base, product, pfiles, csversiondir) = gen_cs_cel_b2cs_msi_base(fname = fname)

    python = Feature(
        parent = product,
        id = "python",
        title = "Python 2.5",
        configurable_directory = DirectoryRef(id  = 'TARGETDIR')
        )

    merge_merge_module(id = 'python',
                       guid = '6b976adf-8ae8-434e-b282-a06c7f624d2f',
                       source_file = 'python_new.msm',
                       feature = python,
                       directory = DirectoryRef(parent = product,
                                                id  = 'TARGETDIR'))

    return (base, product, pfiles, csversiondir)

def gen_cs_cel_b2cs_python_msi():
    (base, product, pfiles, csversiondir) = gen_cs_cel_b2cs_python_msi_base(fname = 'CS_cel_b2cs_python.wxs')
    base.save()

if __name__ == '__main__':
    gen_cs_doc_msm()
    gen_cs_vfs_msm()
    gen_cs_data_msm()
    gen_cs_include_msm()
    gen_cs_register_msm()
    gen_cs_arch_msms('GCC', 'GCC')

    gen_cs_msi()
    gen_cs_cel_msi()
    gen_cs_cel_b2cs_msi()
    gen_cs_cel_b2cs_python_msi()
