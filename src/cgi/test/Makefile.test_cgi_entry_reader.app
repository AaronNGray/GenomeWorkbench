# $Id: Makefile.test_cgi_entry_reader.app 505858 2016-06-29 16:55:21Z elisovdn $

APP = test_cgi_entry_reader
SRC = test_cgi_entry_reader
LIB = xcgi xutil xncbi

CHECK_CMD = test_cgi_entry_reader.sh test_cgi_entry_reader.dat /CHECK_NAME=test_cgi_entry_reader.sh
CHECK_COPY = test_cgi_entry_reader.sh test_cgi_entry_reader.dat

WATCHERS = ucko
