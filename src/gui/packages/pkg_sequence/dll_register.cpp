/*  $Id: dll_register.cpp 37306 2016-12-22 16:04:16Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *      Defines "Sequence" package for GBENCH.
 */

#include <ncbi_pch.hpp>

#include <gui/core/ui_data_source_service.hpp>
#include <gui/core/dm_search_tool.hpp>

#include <gui/framework/gui_package.hpp>

#include <gui/widgets/wx/ui_command.hpp>

#include <gui/utils/extension_impl.hpp>

#include "text_view.hpp"
#include <gui/packages/pkg_sequence/seq_text_view.hpp>
#include <gui/packages/pkg_sequence/view_graphic.hpp>
#include <gui/packages/pkg_sequence/view_feattable.hpp>
#include <gui/packages/pkg_sequence/gb_ui_data_source.hpp>
#include <gui/packages/pkg_sequence/cleanup_tool.hpp>
#include <gui/packages/pkg_sequence/merge_tool.hpp>
#include <gui/packages/pkg_sequence/windowmasker_tool.hpp>
#include <gui/packages/pkg_sequence/bam_ui_data_source.hpp>
#include <gui/widgets/seq_graphic/track_config_manager.hpp>

#include "fasta_format_load_manager.hpp"
#include "entrez_search_tool.hpp"
#include "component_search_tool.hpp"
#include "feature_search_tool.hpp"
#include "sequence_search_tool.hpp"
#include "orf_search_tool.hpp"
#include "cpg_search_tool.hpp"
#include <gui/packages/pkg_sequence/snp_search_tool.hpp>

#include "agp_exporter.hpp"
#include "agp_loader.hpp"
#include "gff_exporter.hpp"
#include "gtf_exporter.hpp"
#include "5col_exporter.hpp"
#include "fasta_exporter.hpp"
#include "flatfile_exporter.hpp"
#include "wiggle_exporter.hpp"
#include "gff_load_manager.hpp"
#include "wig_load_manager.hpp"
#include "bed_load_manager.hpp"
#include "vcf_load_manager.hpp"
#include "5col_load_manager.hpp"

#include "text_view_project_item_extension.hpp"

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// Declare the Package Object
class CSeqPackage : public IGuiPackage
{
public:
    virtual string  GetName() const { return "Sequence"; }
    virtual void    GetVersion(size_t& verMajor, size_t& verMinor, size_t& verPatch) const
    {
        verMajor = 1;
        verMinor = 0;
        verPatch = 0;
    }
    virtual bool    Init();
    virtual void    Shut() {}

protected:
    virtual void    x_RegisterCommands();
};


bool CSeqPackage::Init()
{
    CExtensionDeclaration("view_manager_service::view_factory",
                          new CTextViewFactory());
    CExtensionDeclaration("view_manager_service::view_factory",
                          new CSeqTextViewFactory());
    CExtensionDeclaration("view_manager_service::view_factory",
                          new CViewGraphicFactory());
    CExtensionDeclaration("view_manager_service::view_factory",
                          new CFeatTableViewFactory());

    CExtensionDeclaration("ui_data_source_type",
                          new CGenBankUIDataSourceType());
    CExtensionDeclaration ("file_format_loader_manager",
                          new CFASTAFormatLoadManager());
    CExtensionDeclaration ("file_format_loader_manager",
                          new CGffLoadManager());
    CExtensionDeclaration ("file_format_loader_manager",
                           new CAgpLoader());
    CExtensionDeclaration ("file_format_loader_manager",
                           new CWigLoadManager());
    CExtensionDeclaration ("file_format_loader_manager",
                           new CBedLoadManager());
    CExtensionDeclaration ("file_format_loader_manager",
                           new CVCFLoaderManager());
    CExtensionDeclaration ("file_format_loader_manager",
                          new C5ColLoadManager());

// IFileLoadPanelClient
    CExtensionDeclaration ("file_load_panel_client", new CFileLoadPanelClientFactory<CFASTAFormatLoadManager>());
    CExtensionDeclaration ("file_load_panel_client", new CFileLoadPanelClientFactory<CGffLoadManager>());
    CExtensionDeclaration ("file_load_panel_client", new CFileLoadPanelClientFactory<CAgpLoader>());
    CExtensionDeclaration ("file_load_panel_client", new CFileLoadPanelClientFactory<CWigLoadManager>());
    CExtensionDeclaration ("file_load_panel_client", new CFileLoadPanelClientFactory<CBedLoadManager>());
    CExtensionDeclaration ("file_load_panel_client", new CFileLoadPanelClientFactory<CVCFLoaderManager>());
    CExtensionDeclaration ("file_load_panel_client", new CFileLoadPanelClientFactory<C5ColLoadManager>());


    CExtensionDeclaration (EXT_POINT__UI_EXPORTER_FACTORY, new CGffExporterFactory());
    CExtensionDeclaration (EXT_POINT__UI_EXPORTER_FACTORY, new CGtfExporterFactory());
    CExtensionDeclaration (EXT_POINT__UI_EXPORTER_FACTORY, new C5ColExporterFactory());
    CExtensionDeclaration (EXT_POINT__UI_EXPORTER_FACTORY, new CFastaExporterFactory());
    CExtensionDeclaration (EXT_POINT__UI_EXPORTER_FACTORY, new CAgpExporterFactory());
    CExtensionDeclaration (EXT_POINT__UI_EXPORTER_FACTORY, new CFlatFileExporterFactory());
    CExtensionDeclaration(EXT_POINT__UI_EXPORTER_FACTORY, new CWiggleExporterFactory());

    
    // datamining tools 
    CExtensionDeclaration("data_mining_tool", new CEntrezSearchTool());
    CExtensionDeclaration("data_mining_tool", new CFeatureSearchTool());
    CExtensionDeclaration("data_mining_tool", new CSequenceSearchTool());   
    CExtensionDeclaration("data_mining_tool", new CComponentSearchTool());
    CExtensionDeclaration("data_mining_tool", new COrfSearchTool());
    CExtensionDeclaration("data_mining_tool", new CCpgSearchTool());
    CExtensionDeclaration("data_mining_tool", new CSNPSearchTool());

    CExtensionDeclaration("ui_data_source_type", new CBamUIDataSourceType());

    // Edit tools
    CExtensionDeclaration("ui_algo_tool_manager", new CCleanupTool());
    CExtensionDeclaration("ui_algo_tool_manager", new CMergeTool());
    CExtensionDeclaration("ui_algo_tool_manager", new CWindowMaskerTool());

    CExtensionDeclaration(EXT_POINT__PROJECT_ITEM_EXTENSION, new CTextViewProjectItemExtension());

    CTrackConfigManager::RegisterSGTracks();

    x_RegisterCommands();

    return true;
}


// register commands defined in the package
void CSeqPackage::x_RegisterCommands()
{
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();

    cmd_reg.RegisterCommand(eCmdLoadFromGenBank, "Load Sequences", "Load Sequences",
                            "", "Load Sequences from GenBank");

    cmd_reg.RegisterCommand(eCmdLoadBamFile, "Load BAM Files", "Load BAM Files",
                            "", "Load Local BAM Files");
}


///////////////////////////////////////////////////////////////////////////////
/// Declare Package Entry Point
extern "C" {
    NCBI_PACKAGEENTRYPOINT_EXPORT IGuiPackage* NCBIGBenchGetPackage()
    {
        return new CSeqPackage();
    }
}


END_NCBI_SCOPE
