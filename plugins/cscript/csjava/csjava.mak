# Plug-in module only valid if module is listed in PLUGINS.
ifneq (,$(findstring csjava,$(PLUGINS)))

# Plugin description
DESCRIPTION.csjava = Crystal Script Java plug-in
DESCRIPTION.csjavaswig = Crystal Script Java SWIG interface

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plugin-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make csjava        Make the $(DESCRIPTION.csjava)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csjava csjavaclean csjavaswig

all plugins: csjava

csjava:
	$(MAKE_TARGET) MAKE_DLL=yes
csjavaclean:
	$(MAKE_CLEAN)
csjavaswig: 
	$(MAKE_TARGET) MAKE_DLL=yes	

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

#TCLTK=-ltk8.0 -ltcl8.0 -L/usr/X11R6/lib -lX11
#PTHREAD=-lpthread

CFLAGS.JAVA += $(CFLAGS.I)$(JAVA_INC)
LIB.CSJAVA.SYSTEM += $(LFLAGS.l)native $(LFLAGS.l)kaffevm \
  $(LFLAGS.L)$(JAVA_LIB) $(TCLTK) $(PTHREAD)

ifeq ($(USE_PLUGINS),yes)
  CSJAVA = $(OUTDLL)csjava$(DLL)
  LIB.CSJAVA = $(foreach d,$(DEP.CSJAVA),$($d.LIB))
  LIB.CSJAVA.LOCAL = $(LIB.CSJAVA.SYSTEM)
# TO_INSTALL.DYNAMIC_LIBS += $(CSJAVA)
else
  CSJAVA = $(OUT)$(LIB_PREFIX)csja$(LIB)
  DEP.EXE += $(CSJAVA)
  LIBS.EXE += $(LIB.CSJAVA.SYSTEM)
  SCF.STATIC += csjava
# TO_INSTALL.STATIC_LIBS += $(CSJAVA)
endif

SWIG.CSJAVA = plugins/cscript/csjava/cs_java.cpp
SWIG.INTERFACE = plugins/cscript/common/cs.i

INC.CSJAVA = $(wildcard plugins/cscript/csjava/*.h)
SRC.CSJAVA = $(wildcard plugins/cscript/csjava/*.cpp) $(SWIG.CSJAVA)
OBJ.CSJAVA = $(addprefix $(OUT),$(notdir $(SRC.CSJAVA:.cpp=$O)))
DEP.CSJAVA = CSGEOM CSSYS CSUTIL CSSYS

#MSVC.DSP += CSJAVA
#DSP.CSJAVA.NAME = csjava
#DSP.CSJAVA.TYPE = plugin
#DSP.CSJAVA.RESOURCES = $(SWIG.INTERFACE)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csjava csjavaclean csjavaswig csjavaswigclean

all: $(CSJAVA.LIB)
csjava: $(OUTDIRS) $(CSJAVA)
clean: csjavaclean

$(OUT)%$O: plugins/cscript/csjava/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.JAVA)

$(OUT)%$O: plugins/cscript/csjava/%.c
	$(DO.COMPILE.C) $(CFLAGS.JAVA)

$(SWIG.CSJAVA): $(SWIG.INTERFACE)
	swig -java -c++ -shadow -o $(SWIG.CSJAVA) $(SWIG.INTERFACE)
	mv *.java scripts/java/
	javac scripts/java/*.java
	mv *.class scripts/java/

$(CSJAVA): $(OBJ.CSJAVA) $(LIB.CSJAVA)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.CSJAVA.LOCAL) \
	$(DO.PLUGIN.POSTAMBLE)

csjavaclean:
	-$(RM) $(CSJAVA) $(OBJ.CSJAVA)

csjavaswig: csjavaswigclean csjava

csjavaswigclean:
	-$(RM) $(CSJAVA) $(SWIG.CSJAVA)

ifdef DO_DEPEND
dep: $(OUTOS)csjava.dep
$(OUTOS)csjava.dep: $(SRC.CSJAVA)
	$(DO.DEP)
else
-include $(OUTOS)csjava.dep
endif

endif # ifeq ($(MAKESECTION),targets)
endif # ifneq (,$(findstring csjava,$(PLUGINS)))
