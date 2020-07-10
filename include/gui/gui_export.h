#ifndef GUI___GUI_EXPORT__H
#define GUI___GUI_EXPORT__H

/*  $Id: gui_export.h 41257 2018-06-25 18:52:22Z katargir $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Author:  Mike DiCuccio
 *
 */

/**
 * @file gui_export.h
 *
 * Defines to provide correct exporting from DLLs in Windows.
 * These are necessary to compile DLLs with Visual C++ - exports must be
 * explicitly labeled as such.
 *
 */

#include <common/ncbi_export.h>


/** @addtogroup WinDLL
 *
 * @{
 */

#if defined(NCBI_OS_MSWIN)  &&  defined(NCBI_DLL_BUILD)

#ifndef _MSC_VER
#  error "This toolkit is not buildable with a compiler other than MSVC."
#endif


/*
 * Dumping ground for Windows-specific stuff
 */
#pragma warning (disable : 4191)


/*
 * -------------------------------------------------
 * DLL clusters
 */


/*
 * Definitions for GUI_UTILS.DLL
 */
#ifdef NCBI_GUIUTILS_EXPORTS
#  define NCBI_GUIOBJUTILS_EXPORTS
#  define NCBI_GUIOBJCOORDS_EXPORTS
#  define NCBI_GUIOPENGL_EXPORTS
#  define NCBI_GUIPRINT_EXPORTS
#  define NCBI_GUIMATH_EXPORTS
#endif


/*
 * Definitions for GUI_WIDGETS.DLL
 */
#ifdef NCBI_GUIWIDGETS_EXPORTS
#  define NCBI_GUIWIDGETS_WX_EXPORTS
#  define NCBI_GUIWIDGETS_GL_EXPORTS
#  define NCBI_GUIWIDGETS_CONFIG_EXPORTS
#  define NCBI_GUIWIDGETS_DATA_EXPORTS
#  define NCBI_GUIWIDGETS_LOADERS_EXPORTS
#  define NCBI_GUIWIDGETS_ALNSCORE_EXPORTS
#  define NCBI_GUIWIDGETS_FEEDBACK_EXPORTS
#endif


/*
 * Definitions for GUI_WIDGETS_ALN.DLL
 */
#ifdef NCBI_GUIWIDGETS_ALN_EXPORTS
#  define NCBI_GUIWIDGETS_ALNCROSSALN_EXPORTS
#  define NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORTS
#  define NCBI_GUIWIDGETS_ALNTABLE_EXPORTS
#  define NCBI_GUIWIDGETS_HIT_MATRIX_EXPORTS
#endif

/*
 * Definitions for GUI_WIDGETS_MISC.DLL
 */
#ifdef NCBI_GUIWIDGETS_MISC_EXPORTS
#  define NCBI_GUIWIDGETS_PHYLO_TREE_EXPORTS
#  define NCBI_GUIWIDGETS_OBJECT_LIST_EXPORTS
#  define NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORTS
#  define NCBI_GUIWIDGETS_GRID_WIDGET_EXPORTS
#  define NCBI_GUIWIDGETS_MACRO_EDIT_EXPORTS
#endif

/*
 * Definitions for GUI_WIDGETS_SEQ.DLL
 */
#ifdef NCBI_GUIWIDGETS_SEQ_EXPORTS
#  define NCBI_GUIWIDGETS_EDIT_EXPORTS
#  define NCBI_GUIWIDGETS_FEATCOMPARE_EXPORTS
#  define NCBI_GUIWIDGETS_FEATTABLE_EXPORTS
#  define NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORTS
#  define NCBI_GUIWIDGETS_SEQTEXT_EXPORTS
#  define NCBI_GUIWIDGETS_TAXTREE_EXPORTS
#  define NCBI_GUIWIDGETS_DESKTOP_EXPORTS
#endif

/*
 * Definitions for GUI_CORE.DLL
 */
#ifdef NCBI_GUICORE_EXPORTS
#  define NCBI_GUIFRAMEWORK_EXPORTS
#endif



/* ------------------------------------------------- */
/*
 * Individual Library Definitions
 * Please keep alphabetized!
 */

/*
 * Export specifier for library w_config
 */
#ifdef NCBI_GUIWIDGETS_CONFIG_EXPORTS
#  define NCBI_GUIWIDGETS_CONFIG_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_CONFIG_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library gui_config
 */
#ifdef NCBI_GUICONFIG_EXPORTS
#  define NCBI_GUICONFIG_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUICONFIG_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library gui_core
 */
#ifdef NCBI_GUICORE_EXPORTS
#  define NCBI_GUICORE_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUICORE_EXPORT __declspec(dllimport)
#endif


/*
 * Export specifier for library gui_objects
 */
#ifdef NCBI_GUIOBJECTS_EXPORTS
#  define NCBI_GUIOBJECTS_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIOBJECTS_EXPORT __declspec(dllimport)
#endif


/*
 * Export specifier for library gui_graph
 */
#ifdef NCBI_GUIGRAPH_EXPORTS
#  define NCBI_GUIGRAPH_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIGRAPH_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library gui_objutils
 */
#ifdef NCBI_GUIOBJUTILS_EXPORTS
#  define NCBI_GUIOBJUTILS_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIOBJUTILS_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library gui_objutils
 */
#ifdef NCBI_GUIOBJCOORDS_EXPORTS
#  define NCBI_GUIOBJCOORDS_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIOBJCOORDS_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library gui_opengl
 */
#ifdef NCBI_GUIOPENGL_EXPORTS
#  define NCBI_GUIOPENGL_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIOPENGL_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library gui_print
 */
#ifdef NCBI_GUIPRINT_EXPORTS
#  define NCBI_GUIPRINT_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIPRINT_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library gui_utils
 */
#ifdef NCBI_GUIUTILS_EXPORTS
#  define NCBI_GUIUTILS_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIUTILS_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_seq
 */
#ifdef NCBI_GUIWIDGETS_SEQ_EXPORTS
#  define NCBI_GUIWIDGETS_SEQ_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_SEQ_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_seqtext
 */
#ifdef NCBI_GUIWIDGETS_SEQTEXT_EXPORTS
#  define NCBI_GUIWIDGETS_SEQTEXT_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_SEQTEXT_EXPORT __declspec(dllimport)
#endif

/*
* Export specifier for library w_seq_desktop
*/
#ifdef NCBI_GUIWIDGETS_DESKTOP_EXPORTS
#  define NCBI_GUIWIDGETS_DESKTOP_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_DESKTOP_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_feat_compare
 */
#ifdef NCBI_GUIWIDGETS_FEATCOMPARE_EXPORTS
#  define NCBI_GUIWIDGETS_FEATCOMPARE_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_FEATCOMPARE_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_feattable
 */
#ifdef NCBI_GUIWIDGETS_FEATTABLE_EXPORTS
#  define NCBI_GUIWIDGETS_FEATTABLE_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_FEATTABLE_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_data
 */
#ifdef NCBI_GUIWIDGETS_DATA_EXPORTS
#  define NCBI_GUIWIDGETS_DATA_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_DATA_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_loaders
 */
#ifdef NCBI_GUIWIDGETS_LOADERS_EXPORTS
#  define NCBI_GUIWIDGETS_LOADERS_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_LOADERS_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_alncrossaln
 */
#ifdef NCBI_GUIWIDGETS_ALNCROSSALN_EXPORTS
#  define NCBI_GUIWIDGETS_ALNCROSSALN_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_ALNCROSSALN_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_alnmultiple
 */
#ifdef NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORTS
#  define NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_alnscore
 */
#ifdef NCBI_GUIWIDGETS_ALNSCORE_EXPORTS
#  define NCBI_GUIWIDGETS_ALNSCORE_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_ALNSCORE_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_alntable
 */
#ifdef NCBI_GUIWIDGETS_ALNTABLE_EXPORTS
#  define NCBI_GUIWIDGETS_ALNTABLE_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_ALNTABLE_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_wx
 */
#ifdef NCBI_GUIWIDGETS_WX_EXPORTS
#  define NCBI_GUIWIDGETS_WX_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_WX_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_gl
 */
#ifdef NCBI_GUIWIDGETS_GL_EXPORTS
#  define NCBI_GUIWIDGETS_GL_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_GL_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_hitmatrix
 */
#ifdef NCBI_GUIWIDGETS_HIT_MATRIX_EXPORTS
#  define NCBI_GUIWIDGETS_HIT_MATRIX_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_HIT_MATRIX_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_phylo_tree
 */
#ifdef NCBI_GUIWIDGETS_PHYLO_TREE_EXPORTS
#  define NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_seqgraphic
 */
#ifdef NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORTS
#  define NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_seq
 */
#ifdef NCBI_GUIWIDGETS_SEQ_EXPORTS
#  define NCBI_GUIWIDGETS_SEQ_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_SEQ_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_snp
 */
#ifdef NCBI_GUIWIDGETS_SNP_EXPORTS
#  define NCBI_GUIWIDGETS_SNP_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_SNP_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_taxtree
 */
#ifdef NCBI_GUIWIDGETS_TAXTREE_EXPORTS
#  define NCBI_GUIWIDGETS_TAXTREE_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_TAXTREE_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_object_list
 */
#ifdef NCBI_GUIWIDGETS_OBJECT_LIST_EXPORTS
#  define NCBI_GUIWIDGETS_OBJECT_LIST_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_OBJECT_LIST_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_text_widget
 */
#ifdef NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORTS
#  define NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_grid_widget
 */
#ifdef NCBI_GUIWIDGETS_GRID_WIDGET_EXPORTS
#  define NCBI_GUIWIDGETS_GRID_WIDGET_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_GRID_WIDGET_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_macro_edit
 */
#ifdef NCBI_GUIWIDGETS_MACRO_EDIT_EXPORTS
#  define NCBI_GUIWIDGETS_MACRO_EDIT_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_MACRO_EDIT_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library w_edit
 */
#ifdef NCBI_GUIWIDGETS_EDIT_EXPORTS
#  define NCBI_GUIWIDGETS_EDIT_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_EDIT_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library gui_pkg_sequence_edit
 */ 

#ifdef NCBI_GUIPKG_SEQUENCE_EDIT_EXPORTS
#	define NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT __declspec(dllexport)
#else
#	define NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT __declspec(dllimport)
#endif


/*
 * Export specifier for library gui_services
 */
#ifdef NCBI_GUISERVICES_EXPORTS
#  define NCBI_GUISERVICES_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUISERVICES_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library gui_framework
 */
#ifdef NCBI_GUIFRAMEWORK_EXPORTS
#  define NCBI_GUIFRAMEWORK_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIFRAMEWORK_EXPORT __declspec(dllimport)
#endif

/*
 * Export specifier for library gui_view_align
 */
#ifdef NCBI_GUIVIEW_ALIGN_EXPORTS
#  define NCBI_GUIVIEW_ALIGN_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIVIEW_ALIGN_EXPORT __declspec(dllimport)
#endif

/*
* Export specifier for library w_feedback
*/
#ifdef NCBI_GUIWIDGETS_FEEDBACK_EXPORTS
#  define NCBI_GUIWIDGETS_FEEDBACK_EXPORT __declspec(dllexport)
#else
#  define NCBI_GUIWIDGETS_FEEDBACK_EXPORT __declspec(dllimport)
#endif

#else  /*  !defined(NCBI_OS_MSWIN)  ||  !defined(NCBI_DLL_BUILD)  */

/*
 * NULL operations for other cases
 */

#  define NCBI_GUICONFIG_EXPORT
#  define NCBI_GUICORE_EXPORT
#  define NCBI_GUIFRAMEWORK_EXPORT
#  define NCBI_GUIGRAPH_EXPORT
#  define NCBI_GUIOBJECTS_EXPORT
#  define NCBI_GUIOBJUTILS_EXPORT
#  define NCBI_GUIOBJCOORDS_EXPORT
#  define NCBI_GUIOPENGL_EXPORT
#  define NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT
#  define NCBI_GUIPRINT_EXPORT
#  define NCBI_GUISERVICES_EXPORT
#  define NCBI_GUIUTILS_EXPORT
#  define NCBI_GUIWIDGETS_ALNCROSSALN_EXPORT
#  define NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT
#  define NCBI_GUIWIDGETS_ALNSCORE_EXPORT
#  define NCBI_GUIWIDGETS_ALNTABLE_EXPORT
#  define NCBI_GUIWIDGETS_CONFIG_EXPORT
#  define NCBI_GUIWIDGETS_DATA_EXPORT
#  define NCBI_GUIWIDGETS_LOADERS_EXPORT
#  define NCBI_GUIWIDGETS_EDIT_EXPORT
#  define NCBI_GUIWIDGETS_FEATCOMPARE_EXPORT
#  define NCBI_GUIWIDGETS_FEATTABLE_EXPORT
#  define NCBI_GUIWIDGETS_GL_EXPORT
#  define NCBI_GUIWIDGETS_HIT_MATRIX_EXPORT
#  define NCBI_GUIWIDGETS_OBJECT_LIST_EXPORT
#  define NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT
#  define NCBI_GUIWIDGETS_GRID_WIDGET_EXPORT
#  define NCBI_GUIWIDGETS_MACRO_EDIT_EXPORT
#  define NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT
#  define NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT
#  define NCBI_GUIWIDGETS_SEQTREE_EXPORT
#  define NCBI_GUIWIDGETS_SEQTEXT_EXPORT
#  define NCBI_GUIWIDGETS_SEQ_EXPORT
#  define NCBI_GUIWIDGETS_SNP_EXPORT
#  define NCBI_GUIWIDGETS_TAXTREE_EXPORT
#  define NCBI_GUIWIDGETS_DESKTOP_EXPORT
#  define NCBI_GUIWIDGETS_WX_EXPORT
#  define NCBI_GUIVIEW_ALIGN_EXPORT
#  define NCBI_GUIWIDGETS_FEEDBACK_EXPORT

#endif


/* @} */

#endif  /*  GUI___GUI_EXPORT__H  */
