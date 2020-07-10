# $Id: Makefile.t0016.app 554151 2017-12-28 18:34:57Z ucko $

APP = db100_t0016
SRC = t0016 common

CPPFLAGS = -DHAVE_CONFIG_H=1 -DNEED_FREETDS_SRCDIR $(FTDS100_INCLUDE) \
           $(ORIG_CPPFLAGS)
LIB      = sybdb_ftds100$(STATIC) tds_ftds100$(STATIC)
LIBS     = $(FTDS100_CTLIB_LIBS) $(NETWORK_LIBS) $(RT_LIBS) $(C_LIBS)
LINK     = $(C_LINK)

CHECK_CMD  = test-db100 db100_t0016
CHECK_COPY = t0016.sql t0016.in t0016_1.sql t0016_1.in t0016_2.sql t0016_2.in t0016_3.sql t0016_3.in t0016_4.sql t0016_4.in t0016_5.sql t0016_5.in t0016_6.sql t0016_6.in t0016_7.sql t0016_7.in t0016_8.sql t0016_8.in t0016_9.sql t0016_9.in t0016_10.sql t0016_10.in t0016_11.sql t0016_11.in

CHECK_REQUIRES = in-house-resources

WATCHERS = ucko satskyse
