# $Id: Makefile.test_resize_iter.app 173093 2009-10-14 16:24:46Z vakatov $
# Author:  Aaron Ucko (ucko@ncbi.nlm.nih.gov)

# Build resizing iterator test application "test_resize_iter"
#################################

APP = test_resize_iter
SRC = test_resize_iter
LIB = xutil xncbi

CHECK_CMD = test_resize_iter "test" /CHECK_NAME=test_resize_iter

WATCHERS = ucko
