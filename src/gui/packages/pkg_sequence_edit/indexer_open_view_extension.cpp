/*  $Id: indexer_open_view_extension.cpp 41060 2018-05-16 18:11:00Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include "indexer_open_view_extension.hpp"
#include "smart_file_text_toolbar.hpp"

#include <serial/iterator.hpp>

#include <objects/submit/Seq_submit.hpp>

#include <gui/core/project_service.hpp>
#include <gui/core/document.hpp>

#include <objects/gbproj/ProjectDescr.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/file_extensions.hpp>

#include <gui/widgets/seq/text_panel.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>

#include <gui/widgets/grid_widget/grid_event.hpp>

#include <gui/widgets/edit/table_data_validate_job.hpp>
#include <gui/widgets/edit/table_data_validate.hpp>
#include <gui/objutils/execute_lock_guard.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/objutils/project_item_extra.hpp>

#include <wx/aui/aui.h>
#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h> 
#include <wx/filename.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static CConstRef<CObject> sGetValidateObject(const CObject& object, CScope& scope)
{
    CConstRef<CObject> validateObj;

    const CSeq_submit* seqsubmit = dynamic_cast<const CSeq_submit*>(&object);
    const CSeq_entry* seqEntry = dynamic_cast<const CSeq_entry*>(&object);
    const CSeq_id* seqid = dynamic_cast<const CSeq_id*>(&object);
    const CBioseq_set* seqset = dynamic_cast<const CBioseq_set*>(&object);
    
    CSeq_entry_Handle seh;
    if (seqsubmit) {
        validateObj.Reset(seqsubmit);
    }
    else if (seqEntry) {
        seh = scope.GetSeq_entryHandle(*seqEntry);
    }
    else if (seqid) {
        CBioseq_Handle bsh = scope.GetBioseqHandle(*seqid);
        if (bsh) 
            seh = bsh.GetSeq_entry_Handle();
    }
    else if (seqset) {
            CBioseq_set_Handle bssh = scope.GetBioseq_setHandle(*seqset);
            seh = bssh.GetParentEntry();
    }
    
    if (seh) {
        CConstRef<CSeq_entry> seq_entry = seh.GetCompleteObject();
        validateObj.Reset(seq_entry.GetPointer());
    }
    

    return validateObj;
}

void CIndexerOpenViewExtension::OnViewOpen(IProjectView& view, IServiceLocator* serviceLocator)
{
    if (!serviceLocator) return;
    if (view.GetTypeDescriptor().GetLabel() != "Text View")
        return;

    CProjectService* prjSrv = serviceLocator->GetServiceByType<CProjectService>();
    if (!prjSrv) return;

    CRef<CGBWorkspace> ws = prjSrv->GetGBWorkspace();
    if (!ws) return;

    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromId(view.GetProjectId()));
    if (!doc) return;

    CTextPanel* textPanel = dynamic_cast<CTextPanel*>(view.GetWindow());
    if (!textPanel) return;

    const CObject* object = textPanel->GetOrigObject();
    if (!object) return;

    const CProjectItem* projectItem = 0;
    for (CTypeConstIterator<CProjectItem> it(doc->GetData()); it; ++it) {
        if (it->GetObject() == object) {
            projectItem = &*it;
            break;
        }
    }
    if (!projectItem) return;

    string smartFile;
    CProjectItemExtra::GetStr(*projectItem, "SmartFile", smartFile);
    if (smartFile.empty()) return;

    wxString file = wxString::FromUTF8(smartFile.c_str());
    textPanel->SetWorkDir(wxFileName(file).GetPath());

    SConstScopedObject validateObj;
    CScope* scope = textPanel->GetScope();
    if (object && scope) {
        validateObj.object = sGetValidateObject(*object, *scope);
        validateObj.scope.Reset(scope);
    }

    wxAuiToolBar* toolbar = new CSmartFileTextToolbar(textPanel, view, serviceLocator, validateObj, file);
    toolbar->Realize();

    textPanel->AddExtensionToolbar(*toolbar, wxT("INDEXER tool bar"));
}

///////////////////////////////////////////////////////////////////////////////
/// class CIndexerOpenViewExtension

string CIndexerOpenViewExtension::GetExtensionIdentifier() const
{
    static string sid("indexer_open_view_extension");
    return sid;
}

string CIndexerOpenViewExtension::GetExtensionLabel() const
{
    static string slabel("Genome Workbench INDEXER mode Open view extension");
    return slabel;
}

END_NCBI_SCOPE

