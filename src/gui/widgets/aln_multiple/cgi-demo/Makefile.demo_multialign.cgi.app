# $Id: Makefile.demo_multialign.cgi.app 41919 2018-11-13 23:35:47Z rudnev $

APP = aln_multi.cgi

PROJ_TAG = demo

SRC = aln_multi_cgi

LIB = w_aln_multi w_data gui_glmesa w_gl gui_graph gui_opengl gui_print\
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
      gui_utils_fl gui_utils xalnmgr ximage xcgi xhtml xconnect $(SOBJMGR_LIBS)

LIBS = $(OSMESA_LIBS) $(VDB_LIBS) $(CMPRS_LIBS) \
       $(DL_LIBS) $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)

