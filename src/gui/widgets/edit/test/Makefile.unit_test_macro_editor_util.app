# $Id: Makefile.unit_test_macro_editor_util.app 44736 2020-03-03 19:44:00Z asztalos $

APP = unit_test_macro_editor_util
SRC = unit_test_macro_editor_util
CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE) $(WXWIDGETS_INCLUDE)

LIB  = w_edit w_seq w_wx w_data w_loaders prosplign test_boost $(OBJMGR_LIBS)

LIBS = $(DL_LIBS) $(ORIG_LIBS) $(WXWIDGETS_LIBS)

REQUIRES = Boost.Test.Included wxWidgets WinMain

CHECK_CMD = unit_test_macro_editor_util
CHECK_TIMEOUT = 3000

WATCHERS = asztalos evgeniev
