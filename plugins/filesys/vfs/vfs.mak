#-----------------------------
# Virtual File System plug-in
#-----------------------------

# Driver description
DESCRIPTION.vfs = Virtual File System plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
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

ifeq ($(USE_SHARED_PLUGINS),yes)
  VFS=$(OUTDLL)vfs$(DLL)
  DEP.VFS=$(CSSYS.LIB) $(CSUTIL.LIB)
  TO_INSTALL.DYNAMIC_LIBS+=$(VFS)
else
  VFS=$(OUT)$(LIB_PREFIX)vfs$(LIB)
  DEP.EXE+=$(VFS)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_VFS
  TO_INSTALL.STATIC_LIBS+=$(VFS)
endif
DESCRIPTION.$(VFS) = $(DESCRIPTION.vfs)
SRC.VFS = $(wildcard plugins/filesys/vfs/*.cpp)
OBJ.VFS = $(addprefix $(OUT),$(notdir $(SRC.VFS:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: vfs vfsclean

# Chain rules
clean: vfsclean

vfs: $(OUTDIRS) $(VFS)

$(VFS): $(OBJ.VFS) $(DEP.VFS)
	$(DO.PLUGIN)

vfsclean:
	$(RM) $(VFS) $(OBJ.VFS) $(OUTOS)vfs.dep

ifdef DO_DEPEND
dep: $(OUTOS)vfs.dep
$(OUTOS)vfs.dep: $(SRC.VFS)
	$(DO.DEP)
else
-include $(OUTOS)vfs.dep
endif

endif # ifeq ($(MAKESECTION),targets)
