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
LIBS.CSJAVA += $(LFLAGS.l)native $(LFLAGS.l)kaffevm \
  $(LFLAGS.L)$(JAVA_LIB) $(TCLTK) $(PTHREAD)

ifeq ($(USE_SHARED_PLUGINS),yes)
  CSJAVA = $(OUTDLL)csjava$(DLL)
  LIBS.LOCAL.CSJAVA = $(LIBS.CSJAVA)
  DEP.CSJAVA = $(CSGEOM.LIB) $(CSSYS.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  CSJAVA = $(OUT)$(LIB_PREFIX)csja$(LIB)
  DEP.EXE += $(CSJAVA)
  LIBS.EXE += $(LIBS.CSJAVA)
  CFLAGS.STATIC_SCF += $(CFLAGS.D)SCL_JAVA
endif
DESCRIPTION.$(CSJAVA) = $(DESCRIPTION.csjava)

SWIG.CSJAVA = plugins/cscript/csjava/cs_java.cpp
SWIG.INTERFACE = plugins/cscript/common/cs.i

SRC.CSJAVA = $(wildcard plugins/cscript/csjava/*.cpp) $(SWIG.CSJAVA)
OBJ.CSJAVA = $(addprefix $(OUT),$(notdir $(SRC.CSJAVA:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csjava csjavaclean csjavaswig

all: $(CSJAVA.LIB)
csjava: $(OUTDIRS) $(CSJAVA)
clean: csjavaclean

$(OUT)%$O: plugins/csjava/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.JAVA)

$(SWIG.CSJAVA): $(SWIG.INTERFACE)
	swig -java -c++ -shadow -o $(SWIG.CSJAVA) $(SWIG.INTERFACE)
	mv *.java scripts/java/
	javac scripts/java/*.java
	mv *.class scripts/java/

$(OUT)%$O: plugins/cscript/csjava/%.cpp
	$(DO.COMPILE.C) -w $(CFLAGS.JAVA)

$(CSJAVA): $(OBJ.CSJAVA) $(DEP.CSJAVA)
	$(DO.PLUGIN) $(LIBS.LOCAL.CSJAVA)

csjavaclean:
	-$(RM) $(CSJAVA) $(OBJ.CSJAVA) $(OUTOS)csjava.dep

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
