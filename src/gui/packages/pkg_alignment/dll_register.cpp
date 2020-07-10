/*  $Id: dll_register.cpp 44436 2019-12-18 17:13:59Z katargir $
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
 * Authors:  Roman Katargin, Andrey Yazhuk
 *
 * File Description:
 *    Defines "Alignment" package for GBENCH.
 */

#include <ncbi_pch.hpp>

#include <gui/framework/gui_package.hpp>
#include <gui/utils/extension_impl.hpp>

#include <gui/packages/pkg_alignment/align_span_view.hpp>
#include <gui/packages/pkg_alignment/dot_matrix_view.hpp>
#include <gui/packages/pkg_alignment/cross_align_view.hpp>
#include <gui/packages/pkg_alignment/mp_cross_align_view.hpp>
#include <gui/packages/pkg_alignment/multi_align_view.hpp>
#include <gui/packages/pkg_alignment/phy_tree_view.hpp>
#include <gui/packages/pkg_alignment/annot_compare_view.hpp>
#include <gui/packages/pkg_alignment/aln_summary_view.hpp>

#include <gui/packages/pkg_alignment/net_blast_ui_data_source.hpp>
#include <gui/packages/pkg_alignment/blast_tool_manager.hpp>
#include <gui/packages/pkg_alignment/lblast_ui_data_source.hpp>
#include <gui/packages/pkg_alignment/merge_alignments_tool_manager.hpp>
#include <gui/packages/pkg_alignment/group_alignments_tool_manager.hpp>
#include <gui/packages/pkg_alignment/cleanup_alignments_tool_manager.hpp>
#include <gui/packages/pkg_alignment/create_gene_model_tool_manager.hpp>
#include <gui/packages/pkg_alignment/find_overlap_tool.hpp>
#include <gui/packages/pkg_alignment/splign_tool.hpp>
#include <gui/packages/pkg_alignment/prosplign_tool.hpp>
#include <gui/packages/pkg_alignment/tree_builder_tool.hpp>
#include <gui/packages/pkg_alignment/muscle_tool.hpp>
#include <gui/packages/pkg_alignment/kalign_tool.hpp>
#include <gui/packages/pkg_alignment/mafft_tool.hpp>
#include <gui/packages/pkg_alignment/clustalw_tool.hpp>
#include <gui/packages/pkg_alignment/clustal_omega_tool.hpp>
#include <gui/packages/pkg_alignment/taxtree_tool.hpp>
#include <gui/packages/pkg_alignment/ngalign_tool.hpp>
#include "create_needleman_wunsch_tool_manager.hpp"
#include "phy_loader.hpp"
#include "phy_exporter.hpp"
#include "align_tab_exporter.hpp"
#include "fasta_align_exporter.hpp"
#include "rm_load_manager.hpp"
#include "text_align_load_manager.hpp"
#include "align_project_item_extension.hpp"
#include "advanced_cleanup_tool.hpp"
#include "psl_load_manager.hpp"

#include <gui/widgets/wx/ui_command.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// Declare the Package Object

class CAlignPackage 
    : public IGuiPackage
    , public IMenuContributor
{
public:
    virtual string GetName() const
    {
        return "Alignment";
    }
    virtual void GetVersion(size_t& verMajor, size_t& verMinor, size_t& verPatch) const;

    virtual bool Init();
    virtual void Shut() {}

protected:
    void    x_RegisterCommands();

protected:
    // IMenuContributor implementation
    virtual const SwxMenuItemRec* GetMenuDef() const;
};


void CAlignPackage::GetVersion(size_t& verMajor, size_t& verMinor, size_t& verPatch) const
{
    verMajor = 1;
    verMinor = 0;
    verPatch = 0;
}


bool CAlignPackage::Init()
{
    // Register Views
    CExtensionDeclaration("view_manager_service::view_factory",
                          new CDotMatrixViewFactory());
    CExtensionDeclaration("view_manager_service::view_factory",
                          new CAlignSpanViewFactory());
    CExtensionDeclaration("view_manager_service::view_factory",
                          new CCrossAlignViewFactory());
    CExtensionDeclaration("view_manager_service::view_factory",
                          new CMPCrossAlignViewFactory());
    CExtensionDeclaration("view_manager_service::view_factory",
                          new CMultiAlignViewFactory());    
    CExtensionDeclaration("view_manager_service::view_factory",
                          new CPhyTreeViewFactory());
    CExtensionDeclaration("view_manager_service::view_factory",
                          new CAlnSummaryViewFactory());
    // TODO: not yet ready
    // CExtensionDeclaration("view_manager_service::view_factory",
    //                      new CAnnotCompareViewFactory());


    // Register Net BLAST components
    CExtensionDeclaration("ui_data_source_type", new CNetBLASTUIDataSourceType());
    CExtensionDeclaration("ui_algo_tool_manager", new CBLASTToolManager());

    CExtensionDeclaration("ui_data_source_type", new CLBLASTUIDataSourceType());

    CExtensionDeclaration("ui_algo_tool_manager", new CMergeAlignmentsToolManager());
    CExtensionDeclaration("ui_algo_tool_manager", new CGroupAlignmentsToolManager());
    CExtensionDeclaration("ui_algo_tool_manager", new CCleanupAlignmentsToolManager());
    CExtensionDeclaration("ui_algo_tool_manager", new CCreateGeneModelToolManager());
    CExtensionDeclaration("ui_algo_tool_manager", new CCreateNeedlemanWunschToolManager());

    CExtensionDeclaration("ui_algo_tool_manager", new CFindOverlapTool());
    CExtensionDeclaration("ui_algo_tool_manager", new CSplignTool());
    CExtensionDeclaration("ui_algo_tool_manager", new CProSplignTool());
    CExtensionDeclaration("ui_algo_tool_manager", new CTreeBuilderTool());
    CExtensionDeclaration("ui_algo_tool_manager", new CKalignTool());
    CExtensionDeclaration("ui_algo_tool_manager", new CMafftTool());
    CExtensionDeclaration("ui_algo_tool_manager", new CMuscleTool());
    CExtensionDeclaration("ui_algo_tool_manager", new CClustalwTool());
    CExtensionDeclaration("ui_algo_tool_manager", new CClustalOmegaTool());
    CExtensionDeclaration("ui_algo_tool_manager", new CCommonTaxTreeTool());
    CExtensionDeclaration("ui_algo_tool_manager", new CNGAlignTool());

    if (RunningInsideNCBI()) {
        CExtensionDeclaration("ui_algo_tool_manager", new CAdvancedCleanupTool());
    }

    // newick tree format reader
    CExtensionDeclaration ("file_format_loader_manager",
                    new CPhyLoader());

    // RepeatMasker output reader
    CExtensionDeclaration ("file_format_loader_manager",
                    new CRMLoader());

    CExtensionDeclaration ("file_format_loader_manager",
                          new CTextAlignLoaderManager());

    CExtensionDeclaration ("file_format_loader_manager",
                          new CPslLoadManager());

// IFileLoadPanelClient
    CExtensionDeclaration ("file_load_panel_client", new CFileLoadPanelClientFactory<CTextAlignLoaderManager>());
    CExtensionDeclaration ("file_load_panel_client", new CFileLoadPanelClientFactory<CRMLoader>());
    CExtensionDeclaration ("file_load_panel_client", new CFileLoadPanelClientFactory<CPhyLoader>());

    // ... writer
    CExtensionDeclaration (EXT_POINT__UI_EXPORTER_FACTORY, 
                    new CPhyExporterFactory());
    CExtensionDeclaration (EXT_POINT__UI_EXPORTER_FACTORY, 
                    new CAlignTabExporterFactory());

    CExtensionDeclaration(EXT_POINT__UI_EXPORTER_FACTORY,
                    new CFastaAlignExporterFactory());

    CExtensionDeclaration (EXT_POINT__PROJECT_ITEM_EXTENSION, new CAlignProjectItemExtension());

    x_RegisterCommands();
    return true;
}


// register commands defined in the package
void CAlignPackage::x_RegisterCommands()
{
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();

    cmd_reg.RegisterCommand(eCmdRetriveBlastJobs, "Retrieve", "Retrieve",
                            "", "Retrieve BLAST Job results from Net BLAST server");
    cmd_reg.RegisterCommand(eCmdMonitorBlastJobs, "Monitor", "Monitor",
                            "", "Monitor BLAST Jobs on Net BLAST server");
    cmd_reg.RegisterCommand(eCmdDeleteBlastJobs, "Delete", "Delete",
                            "", "Delete BLAST Jobs from GBENCH");
    cmd_reg.RegisterCommand(eCmdExploreNetBLAST, "Explore", "Explore",
                            "", "View NCBI Net BLAST jobs");
    cmd_reg.RegisterCommand(eCmdLoadRIDs, "Load RIDs", "Load RIDs",
                            "", "Load RIDs from NCBI Net BLAST Server");
    cmd_reg.RegisterCommand(eCmdLoadDB, "Load BLAST DB", "Load BLAST DB",
                            "", "Load Local BLAST DB");
}

WX_DEFINE_MENU(kPkgMenu)
    WX_SUBMENU("&File")
        WX_MENU_ITEM(eCmdLoadRIDs)
    WX_END_SUBMENU()
    WX_MENU_SEPARATOR_L("Contribs")
    WX_SUBMENU("&MultiAlign")
        WX_MENU_ITEM(eCmdMonitorBlastJobs)
        WX_MENU_ITEM(eCmdExploreNetBLAST)
    WX_END_SUBMENU()
WX_END_MENU()

const SwxMenuItemRec* CAlignPackage::GetMenuDef() const 
{
    //return kPkgMenu;
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
/// Declare Package Entry Point
extern "C" {
    NCBI_PACKAGEENTRYPOINT_EXPORT IGuiPackage* NCBIGBenchGetPackage()
    {
        return new CAlignPackage();
    }
}

END_NCBI_SCOPE
