# $Id: Makefile.demo_seqgraphic.cgi.app 42475 2019-03-05 20:47:36Z shkeda $

SRC = demo_seqgraphic.cgi

PROJ_TAG = demo


APP = seqtest.cgi

LIB = w_seq_graphic w_data gui_glmesa w_gl \
      gui_opengl gui_config gui_utils_fl gui_utils xobjutil ximage id1cli \
      splines ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
      xcgi xhtml $(OBJMGR_LIBS)

LIBS = $(OSMESA_LIBS) $(CMPRS_LIBS) $(VDB_LIBS) \
       $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)


