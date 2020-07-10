# $Id: Makefile.sdbapi_unit_test.app 573535 2018-10-29 18:54:02Z ucko $

APP = sdbapi_unit_test
SRC = sdbapi_unit_test sdbapi_unit_test_object sdbapi_unit_test_bcp \
      sdbapi_unit_test_proc sdbapi_unit_test_stmt \
      sdbapi_unit_test_connection sdbapi_unit_test_common \
      sdbapi_unit_test_lob sdbapi_unit_test_xact_abort

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = test_boost $(SDBAPI_LIB) xconnect xutil xncbi

LIBS = $(SDBAPI_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_REQUIRES = connext in-house-resources

CHECK_COPY = sdbapi_unit_test.ini

CHECK_TIMEOUT = 600

CHECK_CMD = sdbapi_unit_test -dr ftds95 -S MsSql
CHECK_CMD = sdbapi_unit_test -dr ftds95 -S MsSql -V 7.2
CHECK_CMD = sdbapi_unit_test -dr ftds100 -S MsSql
CHECK_CMD = sdbapi_unit_test -dr ftds100 -S MsSql -V 7.4
CHECK_CMD = sdbapi_unit_test -dr ftds95 -S Sybase
CHECK_CMD = sdbapi_unit_test -dr ftds100 -S Sybase
CHECK_CMD = sdbapi_unit_test -dr ftds95 -S DBAPI_DEV16_16K -T Sybase
CHECK_CMD = sdbapi_unit_test -dr ftds100 -S DBAPI_DEV16_16K -T Sybase

WATCHERS = ucko satskyse
