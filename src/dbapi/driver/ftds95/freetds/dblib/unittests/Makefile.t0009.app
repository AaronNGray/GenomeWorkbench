# $Id: Makefile.t0009.app 553128 2017-12-13 16:54:04Z satskyse $

APP = db95_t0009
SRC = t0009 common

CPPFLAGS = -DHAVE_CONFIG_H=1 -DNEED_FREETDS_SRCDIR $(FTDS95_INCLUDE) \
           $(ORIG_CPPFLAGS)
LIB      = sybdb_ftds95$(STATIC) tds_ftds95$(STATIC)
LIBS     = $(FTDS95_CTLIB_LIBS) $(NETWORK_LIBS) $(RT_LIBS) $(C_LIBS)
LINK     = $(C_LINK)

CHECK_CMD  = test-db95 db95_t0009
CHECK_COPY = t0009.sql

CHECK_REQUIRES = in-house-resources

WATCHERS = ucko satskyse
