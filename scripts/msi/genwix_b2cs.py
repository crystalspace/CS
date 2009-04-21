#!/usr/bin/python
from wixlib import *

version = extract_version_information('/home/andres/Projects/CS')
version_stringed = version.replace('.', '_')
version_full = version + '.3'
upgrade_code = '933473B2-62E5-11DC-99EE-0015582877AD'
outdir = '/mnt/vm_1/work'


def gen_b2cs_blender_plugin_msm():
    (base, merge, dir) = generate_merge_module(id = 'b2cs.blender.plugin',
                                               path = [('TARGETDIR', 'SourceDir', None),
                                                       ('ProgramFilesFolder', 'PFiles', None),
                                                       ('BLENDERFOUND', 'Blender Foundation', None),
                                                       ('BLENDERINSTALLDIR', 'Blender', None),
                                                       ('BLENDERHOMEDIR', '.blender', None),
                                                       ('BLENDERSCRIPTS', 'scripts', 'scripts'),
                                                       ],
                                               out = outdir,
                                               manufacturer = "b2cs.delcorp.org",
                                               guid = gr['b2cs.blender.plugin.msm'],
                                               version = version_full)


    t_c = Component(
        parent = dir,
        id = "b2cs.blender.plugin.exec")

    b2cs_installdir_prop = Property(
        parent = merge,
        id = "BLENDERSCRIPTS")

    b2cs_r = RegistrySearch(
        parent = b2cs_installdir_prop,
        id = "b2cs.blender.installdir_prop",
        key = "SOFTWARE\\BlenderFoundation",
        name = "Install_Dir")

    b2cs_dotblender = DirectorySearch(
        parent = b2cs_r,
        id = "b2cs.blender.installdir_prop.h",
        depth = "0",
        path = ".blender")

    b2cs_scripts = DirectorySearch(
        parent = b2cs_dotblender,
        id = "b2cs.blender.installdir_prop.s",
        depth = "0",
        path = "scripts")

    File(
        parent = t_c,
        id = "b2cs.blender.plugin.cs_export.py",
        name="crystalspace_export.py")


    frag = Fragment(
        parent = base,
        id = 'b2cs.blender.plugin.lib')

    ref = DirectoryRef(
        parent = frag,
        id = dir.kw['id'])

    generate_directory_tree("scripts",
                            parent = merge,
                            current = ref,
                            prefix = "b2cs.blender.plugin.lib",
                            directory_filter = default_directory_filter,
                            file_filter = lambda d, f: f != 'Jamfile')
    base.save()


def gen_b2cs_msi():
    base = Base(filename = outdir + os.path.sep + 'b2cs.wxs')
    product= Product(
        parent = base,
        version = version_full,
        name = 'Blender to Crystal ' + version,
        upgrade_code = gr['b2cs.upgrade_code'],
        manufacturer = "b2cs.delcorp.org"
    )
    package = Package(parent = product)
    Media(parent = product)
    UIRef(parent = product)
    WixVariable(
        id = "WixUILicenseRtf",
        value = "lgpl.rtf",
        parent = product)

    upgrade_cs = Upgrade(
        parent = product,
        upgrade_code = gr['b2cs.upgrade_code'])

    UpgradeVersion(
        parent = upgrade_cs,
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

    blenderfound =  Directory(
        parent = DirectoryRef(parent = product,
                              id = pfiles.kw['id']),
        id = "BLENDERFOUND",
        name = "Blender Foundation")

    blenderinstalldir = Directory(
        parent = blenderfound,
        id = "BLENDERINSTALLDIR",
        name = "Blender")

    blenderhomedir = Directory(
        parent = blenderinstalldir,
        id = "BLENDERHOMEDIR",
        name = ".blender")

    blenderscripts = Directory(
        parent = blenderhomedir,
        id = "BLENDERSCRIPTS",
        name = 'scripts')


    #b2cs_installdir_prop = Property(
    #    parent = product,
    #    id = "BLENDERSCRIPTS")

    #RegistrySearch(
    #    parent = b2cs_installdir_prop,
    #    id = "b2cs.blender.path",
    #    key = "SOFTWARE\\BlenderFoundation", name = "Install_Dir")

    b2cs = Feature(
        parent = product,
        id = "b2cs",
        title = "blender2crystal",
#        configurable_directory = DirectoryRef(id = 'BLENDERSCRIPTS') #.' +gr['b2cs.blender.plugin.msm'].replace('-','_'))
        )

    b2cs_blender = Feature(
        parent = b2cs,
        id = "b2cs.blender.plugin",
        title = "Blender Plugin",
        )

    merge_merge_module(id = 'b2cs.blender.plugin',
                       guid = gr['b2cs.blender.plugin.msm'],
                       source_file = 'b2cs.blender.plugin.msm',
                       feature = b2cs_blender,
                       directory = DirectoryRef(parent = product,
                                                id  = 'BLENDERSCRIPTS'))


    base.save()

if __name__ == '__main__':
    gen_b2cs_blender_plugin_msm()
    gen_b2cs_msi()
