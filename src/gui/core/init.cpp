/*  $Id: init.cpp 39528 2017-10-05 15:27:37Z katargir $
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
* Authors: Roman Katargin
*
* File Description:
*
*/

#include <ncbi_pch.hpp>
#include <gui/core/init.hpp>

#include <gui/utils/extension_impl.hpp>

#include <gui/core/ui_export_tool.hpp>
#include <gui/widgets/loaders/asn_load_extension.hpp>
#include <gui/widgets/wx/ieditor_factory.hpp>

#include "asn_exporter.hpp"
#include <gui/core/asn_format_load_manager.hpp>
#include <gui/core/ui_project_load_manager.hpp>
#include <gui/core/table_file_load_manager.hpp>
#include <gui/core/table_view.hpp>

#include <gui/core/project_item_extension.hpp>
#include <gui/core/open_view_extension.hpp>

BEGIN_NCBI_SCOPE

extern void initCTableGBProject();
extern void init_project_converters();

bool init_gui_core()
{
    initCTableGBProject();

    init_project_converters();

    CExtensionDeclaration("view_manager_service::view_factory",
                          new CTableViewFactory());
    CExtensionDeclaration("view_manager_service::view_factory",
                          new CProjectTableViewFactory());

    CExtensionDeclaration ("file_format_loader_manager",
                           new CASNFormatLoadManager());
    CExtensionDeclaration ("file_format_loader_manager",
                           new CProjectFormatLoadManager());
    CExtensionDeclaration ("file_format_loader_manager",
                           new CTableFileLoadManager());

// IFileLoadPanelClient
	CExtensionDeclaration ("file_load_panel_client", new CFileLoadPanelClientFactory<CASNFormatLoadManager>());
	CExtensionDeclaration ("file_load_panel_client", new CFileLoadPanelClientFactory<CTableFileLoadManager>());


    CExtensionDeclaration (EXT_POINT__UI_EXPORTER_FACTORY,
                           new CAsnExporterFactory());

    return true;
}

namespace
{

static bool init_gui_core_static()
{
    CExtensionPointDeclaration (EXT_POINT__UI_EXPORTER_FACTORY,
                                "Export service - exporter factory extension");

    CExtensionPointDeclaration (EXT_POINT__ASN_LOAD_EXTENSION,
                                "Import  - register ASN types for ASN loader");

    CExtensionPointDeclaration (EXT_POINT__EDITOR_FACTORY,
                                "Editing - editor factory extension");

    CExtensionPointDeclaration (EXT_POINT__PROJECT_ITEM_EXTENSION,
                                "Project item attach/detach extension");

    CExtensionPointDeclaration (EXT_POINT__OPEN_PROJECT_VIEW_EXTENSION,
                                "Open Project view extension");
    

    return true;
}

const bool gui_core_static_registered = init_gui_core_static();

}


END_NCBI_SCOPE
