# $Id: GNUmakefile,v 1.2 2000/10/19 12:22:10 stanaka Exp $
# --------------------------------------------------------------
# GNUmakefile for examples module.  Gabriele Cosmo, 06/04/98.
# --------------------------------------------------------------

name := RCSim
G4TARGET := $(name)
G4EXLIB := true

G4WORKDIR := .

.PHONY: all
all: lib bin

include $(G4INSTALL)/config/binmake.gmk

ifdef ROOTSYS
  ifeq ($(G4SYSTEM), Linux-g++)
    #  for Linux
    CPPFLAGS += -I$(shell $(ROOTSYS)/bin/root-config --incdir)
    LDLIBS   += -Wl,-rpath,$(shell $(ROOTSYS)/bin/root-config --libdir) \
                $(shell $(ROOTSYS)/bin/root-config --libs)
  else
    CPPFLAGS += -I$(shell $(ROOTSYS)/bin/root-config --incdir)
    LDLIBS   += $(shell $(ROOTSYS)/bin/root-config --libs)
  endif
endif

