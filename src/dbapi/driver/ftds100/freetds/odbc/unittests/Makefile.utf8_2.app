# $Id: Makefile.utf8_2.app 554130 2017-12-28 17:20:38Z ucko $

APP = odbc100_utf8_2
SRC = utf8_2 common

CPPFLAGS = -DHAVE_CONFIG_H=1 $(FTDS100_INCLUDE) $(ODBC_INCLUDE) $(ORIG_CPPFLAGS)
LIB      = odbc_ftds100$(STATIC) tds_ftds100$(STATIC) odbc_ftds100$(STATIC)
LIBS     = $(FTDS100_CTLIB_LIBS) $(NETWORK_LIBS) $(RT_LIBS) $(C_LIBS)
LINK     = $(C_LINK)

CHECK_CMD  = test-odbc100 odbc100_utf8_2
CHECK_COPY = odbc.ini

# This test fails with FreeTDS's replacement iconv, which doesn't
# support any legacy encodings apart from ISO-8859-1 and ASCII.
# Solaris's native iconv has no problem on this test, but fails on
# tds100_utf8_2, so doesn't count, and ptb.ini's claim of iconv on
# Windows is a fiction to help ensure Xcode builds pick libiconv up.
CHECK_REQUIRES = in-house-resources Iconv -Solaris -MSWin

WATCHERS = ucko satskyse
