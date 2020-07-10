# $Id: Makefile.timeout.app 554150 2017-12-28 18:33:44Z ucko $

APP = db100_timeout
SRC = timeout common

CPPFLAGS = -DHAVE_CONFIG_H=1 -DNEED_FREETDS_SRCDIR $(FTDS100_INCLUDE) \
           $(ORIG_CPPFLAGS)
LIB      = sybdb_ftds100$(STATIC) tds_ftds100$(STATIC)
LIBS     = $(FTDS100_CTLIB_LIBS) $(NETWORK_LIBS) $(RT_LIBS) $(C_LIBS)
LINK     = $(C_LINK)

CHECK_CMD  = test-db100 db100_timeout
CHECK_COPY = timeout.sql

CHECK_REQUIRES = in-house-resources

WATCHERS = ucko satskyse
