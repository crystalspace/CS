############################################
# Definitions for Netwide Assemler (common for all x86 platforms)
############################################

# NASM works only on x86
ifneq ($(PROC),INTEL)
override NASM.INSTALLED = no
endif

# If we don't want assembly, disable NASM
ifeq ($(DO_ASM),no)
override NASM.INSTALLED = no
endif

ifeq ($(NASM.INSTALLED),yes)

# The executable name
NASM.BIN = nasm

# NASM flags (well, PROC= is not quite useful (for now?))
NASMFLAGS = -DOS=$(OS) -DCOMP=$(COMP) -DPROC=$(PROC) $(NASMFLAGS.SYSTEM)

# If shared libraries use position-independent code, tell NASM about that
ifneq ($(substr pic,$(CFLAGS.DLL))$(substr PIC,$(CFLAGS.DLL)),)
NASMFLAGS += -DPIC
endif

NASM.@ = -o $@

endif
