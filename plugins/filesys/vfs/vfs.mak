#------------------------------------------------------------------------------
# Virtual File System plug-in
#------------------------------------------------------------------------------
DESCRIPTION.vfs = Virtual File System plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make vfs          Make the $(DESCRIPTION.vfs)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: vfs vfsclean
all plugins: vfs

vfs:
	$(MAKE_TARGET) MAKE_DLL=yes
vfsclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/filesys/vfs

ifeq ($(USE_PLUGINS),yes)
  VFS = $(OUTDLL)vfs$(DLL)
  LIB.VFS = $(foreach d,$(DEP.VFS),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(VFS)
else
  VFS = $(OUT)$(LIB_PREFIX)vfs$(LIB)
  DEP.EXE += $(VFS)
  SCF.STATIC += vfs
  TO_INSTALL.STATIC_LIBS += $(VFS)
endif

INC.VFS = $(wildcard plugins/filesys/vfs/*.h)
SRC.VFS = $(wildcard plugins/filesys/vfs/*.cpp)
OBJ.VFS = $(addprefix $(OUT),$(notdir $(SRC.VFS:.cpp=$O)))
DEP.VFS = CSSYS CSUTIL CSSYS
CFG.VFS = vfs.cfg

TO_INSTALL.ROOT += $(CFG.VFS)

MSVC.DSP += VFS
DSP.VFS.NAME = vfs
DSP.VFS.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: vfs vfsclean
clean: vfsclean

vfs: $(OUTDIRS) $(VFS)

$(VFS): $(OBJ.VFS) $(LIB.VFS)
	$(DO.PLUGIN)

vfsclean:
	$(RM) $(VFS) $(OBJ.VFS)

ifdef DO_DEPEND
dep: $(OUTOS)vfs.dep
$(OUTOS)vfs.dep: $(SRC.VFS)
	$(DO.DEP)
else
-include $(OUTOS)vfs.dep
endif

endif # ifeq ($(MAKESECTION),targets)
