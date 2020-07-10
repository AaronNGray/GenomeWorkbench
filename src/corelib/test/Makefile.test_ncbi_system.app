#$Id: Makefile.test_ncbi_system.app 350009 2012-01-17 20:06:03Z vakatov $

APP = test_ncbi_system
SRC = test_ncbi_system
LIB = xncbi

CHECK_CMD  = test_ncbi_system.sh
CHECK_COPY = test_ncbi_system.sh

CHECK_REQUIRES = -Valgrind

WATCHERS = ivanov
