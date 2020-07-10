#################################
# $Id: Makefile.test_checksum.app 571831 2018-10-03 14:47:24Z ivanov $

APP = test_checksum
SRC = test_checksum
LIB = xutil xncbi

CHECK_COPY = test_data

CHECK_CMD = test_checksum -selftest /CHECK_NAME=test_checksum

WATCHERS = vasilche ivanov
