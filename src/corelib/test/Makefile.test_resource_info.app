# $Id: Makefile.test_resource_info.app 460347 2015-02-26 16:04:00Z grichenk $

APP = test_resource_info
SRC = test_resource_info
LIB = xncbi

CHECK_COPY = resinfo_plain.txt .ncbi_test_key .ncbi_keys.domain test_resource_info.ini
CHECK_CMD = test_resource_info

WATCHERS = grichenk
