# This is a subinclude file used to define the rules needed
# to build the OS/2 OpenGL 2D driver

# Driver description
DESCRIPTION.glos2 = Crystal Space OS/2 OpenGL 2D driver

include libs/cs2d/openglcommon/glcommon2d.mak

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make glos2        Make the $(DESCRIPTION.glos2)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glos2

ifeq ($(USE_SHARED_PLUGINS),yes)
all drivers drivers2d: glos2
endif

glos2:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# The OpenGL library
LIBS.GLOS2+=-lopengl

# Resource file for OS/2 OpenGL driver
GLOS2.RES=$(OUTOS)libGL.res

# The 2D OS/2 OpenGL driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  GLOS2=glos2$(DLL)
  LIBS.LOCAL.GLOS2=$(LIBS.GLOS2)
  DEP.GLOS2=$(GLOS2.RES) $(CSCOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  GLOS2=$(OUT)$(LIB_PREFIX)glos2$(LIB)
  DEP.EXE+=$(GLOS2.RES) $(GLOS2) $(CSOS2.LIB)
  LIBS.EXE+=$(LIBS.GLOS2)
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_GL2DOS2
endif
DESCRIPTION.$(GLOS2)=$(DESCRIPTION.glos2)
SRC.GLOS2 = $(wildcard libs/cs2d/openglos2/*.cpp \
  libs/cs2d/openglcommon/*.cpp \
  libs/cs3d/opengl/ogl_*cache.cpp libs/cs3d/opengl/ogl_txtmgr.cpp \
  libs/cs3d/opengl/itexture.cpp \
  libs/cs3d/common/txtmgr.cpp libs/cs3d/common/memheap.cpp \
  libs/cs3d/common/inv_cmap.cpp libs/cs3d/common/imgtools.cpp\
  $(SRC.COMMON.DRV2D))
OBJ.GLOS2 = $(addprefix $(OUT),$(notdir $(SRC.GLOS2:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp libs/cs2d/openglos2

.PHONY: glos2 os2clean os2cleanlib

# Chain rules
clean: gl2dclean
cleanlib: gl2dcleanlib

glos2: $(OUTDIRS) $(GLOS2)

$(GLOS2): $(OBJ.GLOS2) $(DEP.GLOS2)
	$(DO.PLUGIN) $(LIBS.LOCAL.GLOS2)

$(GLOS2.RES): libs/cs2d/openglos2/libGL.rc
	$(RC) $(RCFLAGS) $< $@

gl2dclean:
	$(RM) $(GLOS2)

gl2dcleanlib:
	$(RM) $(OBJ.GLOS2) $(GLOS2)

ifdef DO_DEPEND
depend: $(OUTOS)glos2.dep
$(OUTOS)glos2.dep: $(SRC.GLOS2)
	$(DO.DEP)
else
-include $(OUTOS)glos2.dep
endif

endif # ifeq ($(MAKESECTION),targets)
