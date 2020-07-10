# $Id: Makefile.unit_test_id_mapper.app 603209 2020-03-09 14:58:12Z ivanov $

REQUIRES = -Cygwin

APP = unit_test_id_mapper

SRC = unit_test_id_mapper

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = xid_mapper gencoll_client sqlitewrapp $(OBJREAD_LIBS) $(XFORMAT_LIBS) \
      xalnmgr xobjutil tables xregexp $(PCRE_LIB) test_boost $(OBJMGR_LIBS)

LIBS = $(SQLITE3_LIBS) $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) \
       $(ORIG_LIBS)

REQUIRES = Boost.Test.Included SQLITE3

CHECK_CMD  = unit_test_id_mapper
CHECK_COPY = unit_test_id_mapper.ini
CHECK_TIMEOUT = 1800

WATCHERS = boukn meric
