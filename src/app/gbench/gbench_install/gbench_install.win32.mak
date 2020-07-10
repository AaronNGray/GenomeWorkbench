# $Id: gbench_install.win32.mak 43856 2019-09-11 17:48:22Z katargir $
#################################################################
#
# Main Installation makefile for Genome Workbench
#
# Author: Mike DiCuccio
#
#################################################################


#################################################################
#
# Overridable variables
# These can be modified on the NMAKE command line
#

#
# This is the main path for installation.  It can be overridden
# on the command-line
INSTALL       = ..\..\..\bin

#
# What kind of copy to use
#
COPY = copy
#COPY = xcopy


#################################################################
#
# Do not override any of the variables below on the NMAKE command
# line
#

!IF "$(INTDIR)" == ".\DebugDLL"
INTDIR = DebugDLL
!ELSEIF "$(INTDIR)" == ".\ReleaseDLL"
INTDIR = ReleaseDLL
!ENDIF

#
# Alias for the bin directory
#
DLLBIN        = $(INSTALL)\$(INTDIR)

#
# Alias for the genome workbench path
#
GBENCH        = $(DLLBIN)\gbench

#
# Third-party DLLs' installation path and rules
#
INSTALL_BINPATH          = $(GBENCH)\bin
THIRDPARTY_MAKEFILES_DIR = ..\..\..
THIRDPARTY_MAKEFILE      = $(THIRDPARTY_MAKEFILES_DIR)\..\third_party_install.meta.mk
STAMP_SUFFIX             = _gbench
!if exist($(THIRDPARTY_MAKEFILE))
!include $(THIRDPARTY_MAKEFILE)
!endif

INI2REG		= cmd /C "set PATH=%PATH%;$(GBENCH)\bin&$(DLLBIN)ini2reg

#
# Alias for the source tree
#
SRCDIR        = ..\..\..\..\..\..\src

#
# Alias for the local installation of third party libs
#
THIRDPARTYLIB = ..\..\..\..\..\..\..\lib\$(INTDIR)

#
# Alias for interaction with replace locations JScript
#
THISFILE = gbench_install.win32.mak
THISDIR = $(SRCDIR)\app\gbench\gbench_install
REPLACE = replace_location.js

#
# Core Libraries
#
CORELIBS = \
        $(GBENCH)\bin\gui_config.dll            \
        $(GBENCH)\bin\gui_objects.dll           \
        $(GBENCH)\bin\gui_core.dll              \
        $(GBENCH)\bin\gui_graph.dll             \
        $(GBENCH)\bin\gui_utils.dll             \
        $(GBENCH)\bin\gui_widgets.dll           \
        $(GBENCH)\bin\gui_widgets_aln.dll       \
        $(GBENCH)\bin\gui_widgets_misc.dll      \
        $(GBENCH)\bin\gui_widgets_seq.dll       \
        $(GBENCH)\bin\gui_view_align.dll        \
        $(GBENCH)\bin\ncbi_algo.dll             \
        $(GBENCH)\bin\ncbi_align_format.dll     \
#        $(GBENCH)\bin\ncbi_bdb.dll              \
        $(GBENCH)\bin\ncbi_blastinput.dll       \
        $(GBENCH)\bin\ncbi_core.dll             \
        $(GBENCH)\bin\ncbi_dbapi.dll            \
        $(GBENCH)\bin\ncbi_dbapi_driver.dll     \
        $(GBENCH)\bin\ncbi_general.dll          \
        $(GBENCH)\bin\ncbi_image.dll            \
        $(GBENCH)\bin\ncbi_lds2.dll              \
        $(GBENCH)\bin\ncbi_misc.dll             \
        $(GBENCH)\bin\ncbi_pub.dll              \
        $(GBENCH)\bin\ncbi_seq.dll              \
        $(GBENCH)\bin\ncbi_seqext.dll           \
		$(GBENCH)\bin\ncbi_trackmgr.dll       \
        $(GBENCH)\bin\ncbi_mmdb.dll           \
        $(GBENCH)\bin\ncbi_validator.dll        \
        $(GBENCH)\bin\ncbi_web.dll              \
#        $(GBENCH)\bin\ncbi_xcache_bdb.dll       \
        $(GBENCH)\bin\ncbi_xcache_netcache.dll  \
        $(GBENCH)\bin\ncbi_xcache_sqlite3.dll   \
#        $(GBENCH)\bin\ncbi_xdbapi_ctlib.dll     \
#        $(GBENCH)\bin\ncbi_xdbapi_dblib.dll     \
        $(GBENCH)\bin\ncbi_xdbapi_ftds.dll      \
#        $(GBENCH)\bin\ncbi_xdbapi_mysql.dll     \
        $(GBENCH)\bin\ncbi_xdbapi_odbc.dll      \
        $(GBENCH)\bin\ncbi_xdiscrepancy.dll \
#        $(GBENCH)\bin\ncbi_xdiscrepancy_report.dll \
        $(GBENCH)\bin\ncbi_xloader_blastdb.dll  \
        $(GBENCH)\bin\ncbi_xloader_genbank.dll  \
        $(GBENCH)\bin\ncbi_xloader_lds2.dll      \
#        $(GBENCH)\bin\ncbi_xobjsimple.dll       \
        $(GBENCH)\bin\ncbi_xreader.dll          \
        $(GBENCH)\bin\ncbi_xreader_cache.dll    \
        $(GBENCH)\bin\ncbi_xreader_id1.dll      \
        $(GBENCH)\bin\ncbi_xreader_id2.dll      \
#        $(GBENCH)\bin\ncbi_eutils.dll           \
        $(GBENCH)\bin\ncbi_xloader_bam.dll      \
        $(GBENCH)\bin\bamread.dll               \
        $(GBENCH)\bin\ncbi_xloader_csra.dll     \
        $(GBENCH)\bin\ncbi_xloader_vdbgraph.dll     \
        $(GBENCH)\bin\ncbi_xloader_wgs.dll     \
        $(GBENCH)\bin\ncbi_xloader_snp.dll     \
        $(GBENCH)\bin\sraread.dll               \
#        $(GBENCH)\bin\srapath.dll               \
#        $(GBENCH)\bin\sra-path.dll              \
#        $(GBENCH)\bin\sra-schema.dll           \
#        $(GBENCH)\bin\ncbi-vdb-read.dll
   
        
#
# Standalone utilities
#
STALLONE_UTILS = \
        $(GBENCH)\bin\windowmasker.exe \
        $(GBENCH)\bin\blastn.exe \
        $(GBENCH)\bin\blastp.exe \
        $(GBENCH)\bin\blastx.exe \
        $(GBENCH)\bin\tblastn.exe \
        $(GBENCH)\bin\tblastx.exe \
        $(GBENCH)\bin\gbenchmacro.exe \
        $(GBENCH)\bin\macroflow.exe \
        $(GBENCH)\bin\demo_widgets_gl_wx
        
        
OPTIONAL_LIBS = \
        $(GBENCH)\bin\ncbi_xreader_pubseqos.dll

PACKAGES = \
        sequence \
        sequence_edit \
        alignment \
        snp

#
# Resource files
#
RESOURCES = \
        share\gbench\*.png                  \
        share\gbench\*.gif                  \
        share\gbench\*.ico                  \
        share\gbench\*.xpm                  \
        share\gbench\fonts\*.ttf            \
        \
        etc\algo_urls                       \
        etc\def_window_layout.asn           \
        etc\gbench-objmgr.ini               \
        etc\gbench.ini                      \
		etc\gbenchmacro.ini                 \
		etc\macroflow.ini                   \
        etc\news.ini                        \
        etc\web_pages.ini                   \
        \
        etc\blastdb-spec\blast-db-tree.asn  \
        etc\remote-only.kfg                 \
        etc\certs.kfg                       \
        etc\align_scores\aa-rasmol-colors   \
        etc\align_scores\aa-shapely-colors  \
        etc\align_scores\blosum45           \
        etc\align_scores\blosum62           \
        etc\align_scores\blosum80           \
        etc\align_scores\hydropathy         \
        etc\align_scores\membrane           \
        etc\align_scores\na-colors          \
        etc\align_scores\signal             \
        etc\align_scores\size               \
        etc\taxlist.txt                     \
        etc\macro_scripts\list_of_macros.mql         \
        etc\macro_scripts\autofix_gb.mql             \
        etc\macro_scripts\autofix_tsa.mql            \
        etc\macro_scripts\autofix_wgs.mql            \
        etc\macro_scripts\add16S.mql                 \
        etc\macro_scripts\altitude_fix.mql           \
        etc\macro_scripts\apply_gdna.mql             \
        etc\macro_scripts\apply_genbank_to_wgs_set.mql             \
        etc\macro_scripts\apply_mito.mql             \
        etc\macro_scripts\chloroprod.mql             \
        etc\macro_scripts\comcirc.mql                \
        etc\macro_scripts\complete_phage.mql         \
        etc\macro_scripts\cytb_macro.mql             \
        etc\macro_scripts\extendCDSgene_nearEnds.mql \
        etc\macro_scripts\fix_allauthors_affil.mql   \
        etc\macro_scripts\fixRNAITSpartials.mql      \
        etc\macro_scripts\fixsource_underscores.mql  \
        etc\macro_scripts\flip_host.mql              \
        etc\macro_scripts\flu_macro.mql              \
        etc\macro_scripts\gbmito_macro.mql           \
        etc\macro_scripts\gpipe_cultured_macro.mql   \
        etc\macro_scripts\gpipe_ITS_macro.mql        \
        etc\macro_scripts\gpipe_uncult_macro.mql     \
        etc\macro_scripts\isolate_to_strain.mql      \
        etc\macro_scripts\default_library.mql        \
        etc\macro_scripts\join_short_trna.mql        \
        etc\macro_scripts\localid_to_clone.mql       \
        etc\macro_scripts\localid_to_isolate.mql     \
        etc\macro_scripts\localid_to_strain.mql      \
        etc\macro_scripts\lowercase_isolationsrc.mql \
        etc\macro_scripts\measles_sourceH.mql        \
        etc\macro_scripts\measles_sourceN.mql        \
        etc\macro_scripts\parse_ATCC.mql             \
        etc\macro_scripts\parse_bla_allele.mql       \
        etc\macro_scripts\parse2strain.mql           \
        etc\macro_scripts\PB1_genexref.mql           \
        etc\macro_scripts\personal_Susan.mql         \
        etc\macro_scripts\remove_all_add16S.mql      \
        etc\macro_scripts\remove_assembly_name.mql   \
        etc\macro_scripts\remove_fulllength.mql      \
        etc\macro_scripts\remove_gene.mql            \
        etc\macro_scripts\remove_identifiedby.mql    \
        etc\macro_scripts\remove_intronlessgene.mql  \
        etc\macro_scripts\remove_location.mql        \
        etc\macro_scripts\remove_location_wotaxlookup.mql \
        etc\macro_scripts\remove_molcomplete.mql     \
        etc\macro_scripts\remove_xtrageneinfo.mql    \
        etc\macro_scripts\rmvorg_from_isolate.mql    \
        etc\macro_scripts\rmvorg_from_strain.mql     \
        etc\macro_scripts\rRNA_macro.mql             \
        etc\macro_scripts\rubella_sourceSH.mql       \
        etc\macro_scripts\set_gene_partial.mql       \
        etc\macro_scripts\set_partial_extend.mql     \
        etc\macro_scripts\trimgenelocus2.mql         \
        etc\macro_scripts\trimgenelocus4.mql         \
        etc\macro_scripts\tRNA_codon.mql             \
        etc\macro_scripts\tRNA_macro.mql             \
        etc\macro_scripts\TSAnote.mql                \
        etc\macro_scripts\uncultured_taxtool.mql     \
        etc\synonyms.txt                             \
        etc\macro_scripts\Updates\add16S.mql         \
        etc\macro_scripts\Updates\remove_all_add16S.mql 


#
# Third-Party Libraries
# These may depend on the build mode
#
THIRD_PARTY_LIBS =          \
        install_berkeleydb  \
        install_openssl     \
        install_wxwidgets   \
        install_sqlite3     \
        install_libxml      \
        install_libxslt     \
        install_glew        \
        install_msvc        \
        install_gnutls      \
        install_vdb         \
        install_nghttp2     \
        install_uv

#
# Registry files
#
REGISTRIES = \
		etc\gbench.asn			\
		etc\plugin_config.asn
		
#
# Pattern Files
#
PATTERNS = \
        $(GBENCH)\etc\patterns\kozak.ini

#
#
# Main Targets
#
all : dirs              \
    $(CORELIBS)         \
    $(OPTIONAL_LIBS)    \
    $(THIRD_PARTY_LIBS) \
    $(STALLONE_UTILS)   \
    $(PACKAGES)         \
    $(PLUGINS)          \
    $(INTERNAL_PLUGINS) \
    $(RESOURCES)        \
    EXTRA_RESOURCES     \
	$(REGISTRIES)		\
    $(PATTERNS)			\
	macro_xrc

clean :
    @echo Removing Genome Workbench Installation...
    -@rmdir /S /Q $(GBENCH)


###############################################################
#
# Target: Create our directory structure
#
dirs :
    @echo Creating directory structure...  $(GBENCH)
    @if not exist $(GBENCH) mkdir $(GBENCH)
    @if not exist $(GBENCH)\bin mkdir $(GBENCH)\bin
    @if not exist $(GBENCH)\etc mkdir $(GBENCH)\etc
    @if not exist $(GBENCH)\plugins mkdir $(GBENCH)\plugins
    @if not exist $(GBENCH)\etc\patterns mkdir $(GBENCH)\etc\patterns
	@if not exist $(GBENCH)\etc\macro_xrc mkdir $(GBENCH)\etc\macro_xrc
    @if not exist $(GBENCH)\share mkdir $(GBENCH)\share
    @if not exist $(GBENCH)\packages mkdir $(GBENCH)\packages
    

###############################################################
#
# Target: Copy the core libraries
#
$(CORELIBS) : $(DLLBIN)\$(*B).dll
    @if exist $** echo Updating $(*B).dll...
    @if exist $(DLLBIN)\$(*B).pdb  $(COPY) $(DLLBIN)\$(*B).pdb $(GBENCH)\bin > NUL
    @if exist $** $(COPY) $** $(GBENCH)\bin > NUL

###############################################################
#
# Target: Copy the optional libraries
#
$(OPTIONAL_LIBS) :
    @echo Checking optional $(DLLBIN)\$(*B).dll...
    @if exist $(DLLBIN)\$(*B).dll echo Updating optional $(DLLBIN)\$(*B).dll...
    @if exist $(DLLBIN)\$(*B).pdb $(COPY) $(DLLBIN)\$(*B).pdb $(GBENCH)\bin > NUL
    @if exist $(DLLBIN)\$(*B).dll $(COPY) $(DLLBIN)\$(*B).dll $(GBENCH)\bin > NUL

###############################################################
#
# Target: Copy the standalone utilities
#
$(STALLONE_UTILS) : $(DLLBIN)\$(*B).exe
    @if exist $** echo Updating $(*B).exe...
    @if exist $** $(COPY) $** $(GBENCH)\bin > NUL
    @if exist $**.manifest $(COPY) $**.manifest $(GBENCH)\bin > NUL


###############################################################
#
# Target: Copy the packages
#
$(PACKAGES) : $(DLLBIN)\pkg_$@.dll $(THISDIR)\$(REPLACE)
    @if exist $(DLLBIN)\pkg_$@.dll echo Updating packages\$@ package...
    @if not exist $(GBENCH)\packages\$@ mkdir $(GBENCH)\packages\$@
    @if exist $(SRCDIR)\gui\packages\pkg_$@\package.manifest $(COPY) $(SRCDIR)\gui\packages\pkg_$@\package.manifest $(GBENCH)\packages\$@ > NUL
    @if exist $(DLLBIN)\pkg_$@.dll $(COPY) $(DLLBIN)\pkg_$@.dll $(GBENCH)\packages\$@ > NUL
    @if exist $(DLLBIN)\pkg_$@.pdb $(COPY) $(DLLBIN)\pkg_$@.pdb $(GBENCH)\packages\$@ > NUL
    @if exist $(DLLBIN)\$@.dll $(COPY) $(DLLBIN)\$@.dll $(GBENCH)\packages\$@ > NUL
    @$(COPY) $(GBENCH)\packages\$@\*.dll $(GBENCH)\bin\ > NUL
#    @for /r $(GBENCH)\packages\$@ %%f in (*.dll) do @$(COPY) $(GBENCH)\packages\$@\%%~nxf $(GBENCH)\bin\%%~nxf > NUL
#    @for /r $(GBENCH)\bin %%f in (Microsoft.*.manifest msvc*.dll) do $(COPY) %%f $(GBENCH)\packages\$@\%%~nxf
#    @for /r $(GBENCH)\bin %%f in (Microsoft.*.manifest) do @cscript //nologo //e:jscript $(THISDIR)\$(REPLACE) %%f $(GBENCH)\packages\$@\%%~nxf

###############################################################
#
# Target: Copy the resources
#
$(RESOURCES) : $(SRCDIR)\gui\res\$@
    @if not exist $(GBENCH)\$(*D) mkdir $(GBENCH)\$(*D)
    @if exist $(SRCDIR)\gui\res\$@ echo Updating $@...
    @if exist $(SRCDIR)\gui\res\$@ $(COPY) $(SRCDIR)\gui\res\$@ $(GBENCH)\$@ > NUL

EXTRA_RESOURCES : $(SRCDIR)\objects\seqloc\accguide.txt
    @if not exist $(GBENCH)\etc mkdir $(GBENCH)\etc 
    @$(COPY) $(SRCDIR)\objects\seqloc\accguide.txt $(GBENCH)\etc > NUL
	
###############################################################
#
# Target: Generate registry files
#
$(REGISTRIES) : $(SRCDIR)\gui\res\$@.ini
    @if not exist $(GBENCH)\$(*D) mkdir $(GBENCH)\$(*D)
    @if exist $(SRCDIR)\gui\res\$@.ini echo Generating $@ ...
    @if exist $(SRCDIR)\gui\res\$@.ini $(INI2REG) -ini $(SRCDIR)\gui\res\$@.ini -reg $(GBENCH)\$@" 


###############################################################
#
# Target: Copy the pattern files
#
$(PATTERNS) : $(SRCDIR)\gui\res\etc\patterns\$(*B).ini
    @if exist $** echo Updating $(*B).ini...
    @if exist $** $(COPY) $** $(GBENCH)\etc\patterns > NUL


###############################################################
#
# Target: Copy the XRC files
#
macro_xrc : 
	@echo Copying macro editor's XRC files...
    @$(COPY) $(SRCDIR)\gui\res\etc\macro_xrc\*.xrc $(GBENCH)\etc\macro_xrc\ > NUL


###############################################################
#
# Target: Make replace JScript file
#
$(THISDIR)\$(REPLACE) : $(THISDIR)\$(THISFILE)
    @echo Creating <<$(THISDIR)\$(REPLACE) file...
var args = WScript.Arguments;
var FSO = WScript.CreateObject( "Scripting.FileSystemObject" );

var stdin = ( args.length > 0 ) ? FSO.OpenTextFile( args(0), 1 ) : WScript.StdIn;
var stdout = ( args.length > 1 ) ? FSO.OpenTextFile( args(1), 2, true ) : WScript.StdOut;

var re = new RegExp( "(name=\")([^\"\s]*)(msvc)", "g" );

while( !stdin.AtEndOfStream ){

    var str = stdin.ReadLine();

    var result = re.exec(str);
    stdout.WriteLine( str.replace(re, "$$1..\\..\\bin\\$$3") );
}

stdout.Close();
stdin.Close();
<<KEEP
