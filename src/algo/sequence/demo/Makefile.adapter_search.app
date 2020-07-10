# $Id: Makefile.adapter_search.app 420799 2013-12-05 16:00:37Z astashya $

WATCHERS = astashya

SRC = adapter_search
APP = adapter_search

CPPFLAGS = $(ORIG_CPPFLAGS)
CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)

LIB  = xalgoseq taxon1 xalnmgr xobjutil xconnect tables xregexp $(PCRE_LIB) $(SOBJMGR_LIBS)
LIBS = $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

