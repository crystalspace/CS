# Plug-in module only valid if module is listed in PLUGINS.
ifneq (,$(findstring cslua,$(PLUGINS)))

# Plugin description
DESCRIPTION.cslua = Crystal Script Lua plug-in
DESCRIPTION.csluaswig = Crystal Script Lua SWIG interface

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plugin-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make cslua     Make the $(DESCRIPTION.cslua)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cslua csluaclean csluaswig
all plugins: cslua

cslua:
	$(MAKE_TARGET) MAKE_DLL=yes
csluaclean:
	$(MAKE_CLEAN)
csluaswig: 
	$(MAKE_TARGET) MAKE_DLL=yes	

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

#CFLAGS.LUA += $(CFLAGS.I)$(LUA_INC)
CFLAGS.LUA += $(CFLAGS.D)LUA_MS

# LUA_LIB points at the Lua library directory (often /usr/lib/lua1.5).
# The actual static link library usually resides in a "config" subdirectory of
# the library directory.  The library's name is the same as the directory,
# thus in this example, the library would be called "liblua1.5.a".
LIB.CSLUA.SYSTEM +=  \
  $(LFLAGS.l)lua -llualib

ifeq ($(USE_PLUGINS),yes)
  CSLUA = $(OUTDLL)cslua$(DLL)
  LIB.CSLUA = $(foreach d,$(DEP.CSLUA),$($d.LIB))
  LIB.CSLUA.LOCAL = $(LIB.CSLUA.SYSTEM)
# TO_INSTALL.DYNAMIC_LIBS += $(CSLUA)
else
  CSLUA = $(OUT)$(LIB_PREFIX)cspy$(LIB)
  DEP.EXE += $(CSLUA)
  LIBS.EXE += $(LIB.CSLUA.SYSTEM)
  SCF.STATIC += cslua
# TO_INSTALL.STATIC_LIBS += $(CSLUA)
endif

SWIG.INTERFACE = plugins/cscript/common/cs.i
SWIG.CSLUA = plugins/cscript/cslua/cs_lua.cpp
SWIG.CSLUA.OBJ = $(addprefix $(OUT),$(notdir $(SWIG.CSLUA:.cpp=$O)))

INC.CSLUA = $(wildcard plugins/cscript/cslua/*.h)
SRC.CSLUA = \
  $(sort $(wildcard plugins/cscript/cslua/*.cpp) $(SWIG.CSLUA))
OBJ.CSLUA = $(addprefix $(OUT),$(notdir $(SRC.CSLUA:.cpp=$O)))
DEP.CSLUA = CSGEOM CSSYS CSUTIL CSSYS

MSVC.DSP += CSLUA
DSP.CSLUA.NAME = cslua
DSP.CSLUA.TYPE = plugin
DSP.CSLUA.RESOURCES = $(SWIG.INTERFACE)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cslua csluaclean csluaswig csluaswigclean

all: $(CSLUA.LIB)
cslua: $(OUTDIRS) $(CSLUA)
clean: csluaclean

$(SWIG.CSLUA.OBJ): $(SWIG.CSLUA)
	$(filter-out -W -Wunused -Wall,$(DO.COMPILE.CPP) $(CFLAGS.LUA))

$(OUT)%$O: plugins/cscript/cslua/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.LUA)

$(OUT)%$O: plugins/cscript/cslua/%.c
	$(DO.COMPILE.C) $(CFLAGS.LUA)

$(SWIG.CSLUA): $(SWIG.INTERFACE)
	swiglua -multistate -c++ -o $(SWIG.CSLUA) $(SWIG.INTERFACE)

$(CSLUA): $(OBJ.CSLUA) $(LIB.CSLUA)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.CSLUA.LOCAL) \
	$(DO.PLUGIN.POSTAMBLE)

csluaclean:
	-$(RM) $(CSLUA) $(OBJ.CSLUA)

csluaswig: csluaswigclean cslua

csluaswigclean:
	-$(RM) $(CSLUA) $(SWIG.CSLUA) $(OUT)cs_lua.cpp

ifdef DO_DEPEND
dep: $(OUTOS)cslua.dep
$(OUTOS)cslua.dep: $(SRC.CSLUA)
	$(DO.DEP1) $(CFLAGS.LUA) $(DO.DEP2)
else
-include $(OUTOS)cslua.dep
endif

endif # ifeq ($(MAKESECTION),targets)
endif # ifneq (,$(findstring cslua,$(PLUGINS)))
