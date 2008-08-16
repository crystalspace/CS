#!/usr/bin/python
from wixlib import *

version = extract_version_information('/home/andres/Projects/CS')
version_stringed = version.replace('.', '_')
version_full = version + '.3'
upgrade_code = '933473B2-62E5-11DC-99EE-0015582877AD'
outdir = '/mnt/vm_1/work'

def gen_cel_doc_msm():
    (base, merge, dir) = generate_merge_module(id = 'cel.doc',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('CELINSTALLDIR', 'Cel', None),
                                                       ('CELVERSION' + version, version, 'cel')
                                                       ],
                                               out = outdir,
                                               manufacturer = "The Crystal Space Project",
                                               guid = gr['cel.doc.msm'],
                                               version = version_full)
    f = Fragment(
        parent = base,
        id = 'cel.doc')

    ref = DirectoryRef(
        parent = f,
        id = dir.kw['id'])

    doc_d = Directory(
        parent = ref,
        id = 'cel.doc',
        name = "docs")

    man_d = Directory(
        parent = doc_d,
        id = 'cel.doc.man_d',
        name = "manual")

    generate_directory_tree("docs/html/manual",
                            parent = merge,
                            current = man_d,
                            prefix = "cel.doc.manual",
                            directory_filter = default_directory_filter,
                            file_filter = lambda d, f: f != 'Jamfile')

    api_d = Directory(
        parent = doc_d,
        id = 'cel.doc.api_d',
        name = "api")


    generate_directory_tree("docs/html/api",
                            parent = merge,
                            current = api_d,
                            prefix = "cel.doc.api",
                            directory_filter = default_directory_filter,
                            file_filter = lambda d, f: f != 'Jamfile')

    base.save()



def gen_cel_vfs_msm():
    (base, merge, dir) = generate_merge_module(id = 'cel.vfs',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('CELINSTALLDIR', 'Cel', None),
                                                       ('CELVERSION' + version, version, 'cel'),
                                                    ],
                                               out = outdir,
                                               manufacturer = "The Crystal Space Project",
                                               guid = gr['cel.vfs.msm'],
                                               version = version_full)
    vfs_c = Component(
        parent = dir,
        id = "cel.vfs.comp")

    File(
        parent = vfs_c,
        id = "cel.vfs.vfs.cfg",
        name="vfs.cfg")

    base.save()

def gen_cel_register_msm():
    (base, merge, dir) = generate_merge_module(id = 'cel.register',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('CELINSTALLDIR', 'Cel', None),
                                                       ('CELVERSION' + version, version, 'cel'),
                                                    ],
                                               out = outdir,
                                               manufacturer = "The Crystal Space Project",
                                               guid = gr['cel.register.msm'],
                                               version = version_full)


    register_c = Component(
        parent = dir,
        id = 'cel.register.c')

    RegistryValue(
        parent = register_c,
        id = "cel.reggister.r",
        key = "SOFTWARE\\Cel\\"+ version,
        name = 'InstallPath',
        type='string',
        value="[CELVERSION" + version +']')

    Environment(
        parent = register_c,
        id = 'cel.register.e',
        name = 'CEL_' + version_stringed,
        value='[CELVERSION' + version + ']'
        )

    base.save()


def gen_cel_data_msm():
    (base, merge, dir) = generate_merge_module(id = 'cel.data',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('CELINSTALLDIR', 'Cel', None),
                                                       ('CELVERSION' + version, version, 'cel'),
                                                       ],
                                               out = outdir,
                                               manufacturer = "The Crystal Space Project",
                                               guid = gr['cel.data.msm'],
                                               version = version_full)

    frag = Fragment(
        parent = base,
        id = 'cel.data')

    ref = DirectoryRef(
        parent = frag,
        id = dir.kw['id'])

    generate_directory_tree("data",
                            create_dir = True,
                            current = ref,
                            prefix = 'cel.data',
                            parent = merge,
                            directory_filter = default_directory_filter,
                            file_filter = lambda d, f: f != 'Jamfile')

    base.save()


def gen_cel_include_msm():
    (base, merge, dir) = generate_merge_module(id = 'cel.include',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('CELINSTALLDIR', 'Cel', None),
                                                       ('CELVERSION' + version, version, 'cel'),
                                                       ],
                                               out = outdir,
                                               manufacturer = "The Crystal Space Project",
                                               guid = gr['cel.include.msm'],
                                               version = version_full)

    frag = Fragment(
        parent = base,
        id = 'cel.include')

    ref = DirectoryRef(
        parent = frag,
        id = dir.kw['id'])


    generate_directory_tree("include",
                            current = ref,
                            prefix = "cel.include",
                            create_dir = True,
                            parent = merge,
                            directory_filter = default_directory_filter,
                            file_filter = lambda d, f: f[-2:] == '.h' and not (d == "include" and f == "celconfig.h"))

    base.save()

def gen_cel_arch_msms(arch_name, arch_dir):
    gen_cel_arch_include_msm(arch_name, arch_dir)
    gen_cel_arch_executable_msm(arch_name, arch_dir)
    gen_cel_arch_lib_msm(arch_name, arch_dir)

def gen_cel_arch_include_msm(arch_name, arch_dir):
    (base, merge, dir) = generate_merge_module(id = 'cel.arch.' + arch_name + '.include',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('CELINSTALLDIR', 'Cel', None),
                                                       ('CELVERSION' + version, version, 'cel'),
                                                       ('cel.arch.' + arch_name, arch_name, arch_dir),
                                                       ('cel.arch.' + arch_name + '.include', 'include', 'include')
                                                       ],
                                               out = outdir,
                                               manufacturer = "The Crystal Space Project",
                                               guid = gr['cel.arch.'+arch_name + '.include.msm'],
                                               version = version_full)

    c_incl_comp = Component(
        parent = dir,
        id = 'cel.arch.' + arch_name + '.i')
    File(
        parent = c_incl_comp,
        id = 'cel.arch.' + arch_name + '.i.' + 'cfg.h',
        name = "celconfig.h")
    base.save()


def gen_cel_arch_lib_msm(arch_name, arch_dir):
    (base, merge, dir) = generate_merge_module(id = 'cel.arch.' + arch_name + '.lib',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('CELINSTALLDIR', 'Cel', None),
                                                       ('CELVERSION' + version, version, 'cel'),
                                                       ('cel.arch.' + arch_name, arch_name, arch_dir),
                                                       ('cel.arch.' + arch_name + '.lib', 'bin', '.')
                                                       ],
                                               out = outdir,
                                               manufacturer = "The Crystal Space Project",
                                               guid = gr['cel.arch.'+arch_name + '.lib.msm'],
                                               version = version_full)

    frag = Fragment(
        parent = base,
        id = 'cel.arch.' + arch_name + '.lib')

    ref = DirectoryRef(
        parent = frag,
        id = dir.kw['id'])

    register_c = Component(
        parent = dir,
        id = 'cel.arch.' + arch_name + '.register.c')

    Environment(
        parent = register_c,
        id = 'cel.arch.' + arch_name + '.register.e',
        name = 'PATH',
        value='[cel.arch.' + arch_name + '.lib]'
        )

    generate_directory_tree(arch_dir,
                            parent = merge,
                            prefix = 'cel.arch.' + arch_name + '.lib',
                            current = ref,
                            directory_filter = lambda d, f: False,
                            file_filter = lambda d, f: f[-4:] in ['.dll', '.pyd'])

    base.save()

def gen_cel_arch_executable_msm(arch_name, arch_dir):
    (base, merge, dir) = generate_merge_module(id = 'cel.arch.' + arch_name + '.executable',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('CELINSTALLDIR', 'Cel', None),
                                                       ('CELVERSION' + version, version, 'cel'),
                                                       ('cel.arch.' + arch_name, arch_name, arch_dir),
                                                       ('cel.arch.' + arch_name + '.executable', 'bin', '.')
                                                       ],
                                               out = outdir,
                                               manufacturer = "The Crystal Space Project",
                                               guid = gr['cel.arch.'+arch_name + '.executable.msm'],
                                               version = version_full)

    frag = Fragment(
        parent = base,
        id = 'cel.arch.' + arch_name + '.executable')

    ref = DirectoryRef(
        parent = frag,
        id = dir.kw['id'])

    generate_directory_tree(arch_dir,
                            parent = merge,
                            prefix = 'cel.arch.' + arch_name + '.bin',
                            current = ref,
                            directory_filter = lambda d, f: False,
                            file_filter = lambda d, f: f[-4:] in ['.exe', 'py', 'pyd'] or f == 'cs-config-' + version)

    base.save()



def gen_cel_msi():
    base = Base(filename = outdir + os.path.sep + 'cel.wxs')
    product= Product(
        parent = base,
        version = version_full,
        name = 'Crystal Entity Layer ' + version,
        upgrade_code = gr['cel.upgrade_code'],
        manufacturer = "The Crystal Space Project"
    )
    package = Package(parent = product)
    Media(parent = product)
    UIRef(parent = product)
    WixVariable(
        id = "WixUILicenseRtf",
        value = "lgpl.rtf",
        parent = product)

    upgrade = Upgrade(
        parent = product,
        upgrade_code = gr['cel.upgrade_code'])

    UpgradeVersion(
        parent = upgrade,
        min_version = '1.3.0',
        max_version = version_full
    )

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
        key = "SOFTWARE\\Cel\\" + version, name = "InstallPath")

    #cs_old_installdir_prop = Property(
    #    parent = product,
    #    id = "CELVERSION" + version)

    #RegistrySearch(
    #    parent = cs_old_installdir_prop,
    #    id = "CS.old_installdir_prop",
    #    key = "SOFTWARE\\CrystalSpace\\" + version, name = "InstallPath")


    gen_cel_feature_tree(directory = celversiondir, base = product)

    base.save()

def gen_cel_feature_tree(directory = Empty, base = Empty):
    cs = Feature(
        parent = base,
        id = "cel",
        title = "Crystal Entity Layer",
        configurable_directory = DirectoryRef(id  = 'CELVERSION' + version)
        )

    merge_merge_module(id = 'cel.register',
                       guid = gr['cel.register.msm'],
                       source_file = 'cel.register.msm',
                       feature = cs,
                       directory = directory)

    merge_merge_module(id = 'cel.vfs',
                       guid = gr['cel.vfs.msm'],
                       source_file = 'cel.vfs.msm',
                       feature = cs,
                       directory = directory)

    include = Feature(
        parent = cs,
        id = 'cel.include',
        title = "Headers")

    merge_merge_module(id = 'cel.include',
                       guid = gr['cel.include.msm'],
                       source_file = 'cel.include.msm',
                       feature = include,
                       directory = directory)

    data = Feature(
        parent = cs,
        id = 'cel.data',
        title = "Data")

    merge_merge_module(id = 'cel.data',
                       guid = gr['cel.data.msm'],
                       source_file = 'cel.data.msm',
                       feature = data,
                       directory = directory)
    doc = Feature(
        parent = cs,
        id = 'cel.doc',
        title = "Documentation")

    merge_merge_module(id = 'cel.doc',
                       guid = gr['cel.doc.msm'],
                       source_file = 'cel.doc.msm',
                       feature = doc,
                       directory = directory)

    arch_all = Feature(
        parent = cs,
        id = 'cel.arch',
        title = "Binaries")

    for arch in ['GCC']:
        cur_arch = Feature(
            parent = arch_all,
            id = 'cel.arch.'+arch,
            title = "Binaries compiled with " + arch)

        lib = Feature(
            parent = cur_arch,
            id = 'cel.arch.'+arch + '.lib',
            title = "Libraries")

        merge_merge_module(id = 'cel.arch.' + arch + '.lib',
                           guid = gr['cel.arch.'+ arch + '.lib.msm'],
                           source_file = 'cel.arch.'+ arch + '.lib.msm',
                           feature = lib,
                           directory = directory)

        include = Feature(
            parent = cur_arch,
            id = 'cel.arch.'+arch + '.include',
            title = "Headers")

        merge_merge_module(id = 'cel.arch.' + arch + '.include',
                           guid = gr['cel.arch.'+ arch + '.include.msm'],
                           source_file = 'cel.arch.'+ arch + '.include.msm',
                           feature = include,
                           directory = directory)

        executable = Feature(
            parent = cur_arch,
            id = 'cel.arch.'+arch + '.executable',
            title = "Application")

        merge_merge_module(id = 'cel.arch.' + arch + '.executable',
                           guid = gr['cel.arch.'+ arch + '.executable.msm'],
                           source_file = 'cel.arch.'+ arch + '.executable.msm',
                           feature = executable,
                           directory = directory)

def gen_cel_cel_msi():
    pass
def gen_cel_cel_b2cs_msi():
    pass

if __name__ == '__main__':
    gen_cel_doc_msm()
    gen_cel_vfs_msm()
    gen_cel_data_msm()
    gen_cel_include_msm()
    gen_cel_register_msm()
    gen_cel_arch_msms('GCC', 'GCC')

    gen_cel_msi()

