# $Id: Makefile.cs_diag.app 554130 2017-12-28 17:20:38Z ucko $

APP = ct100_cs_diag
SRC = cs_diag common

CPPFLAGS = -DHAVE_CONFIG_H=1 $(FTDS100_INCLUDE) $(ORIG_CPPFLAGS)
LIB      = $(FTDS100_CTLIB_LIB)
LIBS     = $(FTDS100_CTLIB_LIBS) $(NETWORK_LIBS) $(RT_LIBS) $(C_LIBS)
LINK     = $(C_LINK)

CHECK_CMD =

WATCHERS = ucko satskyse
