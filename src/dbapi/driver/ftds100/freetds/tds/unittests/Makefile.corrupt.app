# $Id: Makefile.corrupt.app 554148 2017-12-28 18:32:41Z ucko $

APP = tds100_corrupt
SRC = corrupt common

CPPFLAGS = -DHAVE_CONFIG_H=1 $(FTDS100_INCLUDE) $(ORIG_CPPFLAGS)
LIB      = tds_ftds100$(STATIC)
LIBS     = $(FTDS100_CTLIB_LIBS) $(NETWORK_LIBS) $(RT_LIBS) $(C_LIBS)
LINK     = $(C_LINK)

CHECK_CMD  = test-tds100 tds100_corrupt

CHECK_REQUIRES = in-house-resources

WATCHERS = ucko satskyse
