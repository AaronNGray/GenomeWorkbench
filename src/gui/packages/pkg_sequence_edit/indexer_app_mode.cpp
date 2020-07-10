/*  $Id: indexer_app_mode.cpp 39528 2017-10-05 15:27:37Z katargir $
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

#include "indexer_app_mode.hpp"
#include "indexer_open_view_extension.hpp"

#include <gui/widgets/loaders/asn_object_loader.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/utils/extension.hpp>
#include <gui/utils/extension_impl.hpp>

#include <gui/framework/app_task_service.hpp>
#include <gui/framework/workbench_impl.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/object_loading_task.hpp>
#include <gui/core/project_item_extension.hpp>
#include <gui/core/commands.hpp>
#include <gui/core/document.hpp>
#include <gui/core/ws_auto_saver.hpp>

#include <gui/objects/WorkspaceFolder.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/project_item_extra.hpp>

#include <objects/submit/Seq_submit.hpp>

#include <serial/iterator.hpp>

#include <wx/filedlg.h>
#include <wx/filefn.h> 
#include <wx/msgdlg.h>
#include <wx/filename.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
/// class CIndexerProjectItemExtension

class  CIndexerProjectItemExtension :
    public CObject,
    public IProjectItemExtension,
    public IExtension
{
public:
    CIndexerProjectItemExtension() {}

    /// @name IProjectItemExtension interface implementation
    /// @{
    virtual void ProjectItemCreated(objects::CProjectItem& item, IServiceLocator* service_locator);
    virtual void ProjectItemAttached(objects::CProjectItem& /*item*/, CGBDocument& /*doc*/, IServiceLocator* /*service_locator*/) {}
    virtual void ProjectItemDetached(objects::CProjectItem& /*item*/, CGBDocument& /*doc*/, IServiceLocator* /*service_locator*/) {}
    /// @}

    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}
};

void CIndexerProjectItemExtension::ProjectItemCreated(objects::CProjectItem& item, IServiceLocator* service_locator)
{
    // If project item is a seqsubmit, use the first seqentry to set the label
    if (item.CanGetItem()) {
        const CProjectItem::TItem& citem = item.GetItem();
        if (citem.IsSubmit()) {
            const CSeq_submit* seqSubmit = &citem.GetSubmit();
            const CObject* obj = seqSubmit;
            if (seqSubmit->IsEntrys() && seqSubmit->CanGetData()) {
                const CSeq_submit::TData& data = seqSubmit->GetData();
                if (data.IsEntrys()) {
                    const CSeq_submit::TData::TEntrys& entrys = data.GetEntrys();
                    if (!entrys.empty()) {
                        obj = entrys.front().GetPointerOrNull();
                    }
                }
            }

            string sLabel;
            if (obj) {
                CLabel::GetLabel( *obj, &sLabel, CLabel::eDefault, NULL );
            }
            item.SetLabel(sLabel);
        }
    }

    item.AddTag("open_discrepancy_dlg");
    item.AddTag("open_editing_buttons");
    item.AddTag("open_text_view");
}

string CIndexerProjectItemExtension::GetExtensionIdentifier() const
{
    static string sid("single_project_mode_item_extension");
    return sid;
}

string CIndexerProjectItemExtension::GetExtensionLabel() const
{
    static string slabel("Genome Workbench Single Project mode item extension");
    return slabel;
}

///////////////////////////////////////////////////////////////////////////////
/// class CIndexerAppModeExtension

CIndexerAppModeExtension::CIndexerAppModeExtension() : m_Workbench()
{
}

string CIndexerAppModeExtension::GetExtensionIdentifier() const
{
    static string sid("indexer_app_mode_extension");
    return sid;
}

string CIndexerAppModeExtension::GetExtensionLabel() const
{
    static string slabel("Indexer application mode extension");
    return slabel;
}

string CIndexerAppModeExtension::GetModeName() const
{
    return "indexer";
}

void CIndexerAppModeExtension::SetWorkbench(CWorkbench* wb)
{
    if (wb) {
        m_Workbench = wb;
        CObjectLoadingTask::SetSaveFilePath(true);
        m_Workbench->Bind(wxEVT_MENU, &CIndexerAppModeExtension::OnFileOpen, this, wxID_OPEN);
        m_Workbench->Bind(wxEVT_MENU, &CIndexerAppModeExtension::OnCloseWorkspace, this, eCmdCloseWorkspace);

        CExtensionDeclaration(EXT_POINT__PROJECT_ITEM_EXTENSION, new CIndexerProjectItemExtension());
        CExtensionDeclaration(EXT_POINT__OPEN_PROJECT_VIEW_EXTENSION, new CIndexerOpenViewExtension());
    }
    else if (m_Workbench) {
        m_Workbench->Unbind(wxEVT_MENU, &CIndexerAppModeExtension::OnFileOpen, this, wxID_OPEN);
        m_Workbench = 0;
    }
}

void CIndexerAppModeExtension::OnFileOpen(wxCommandEvent& event)
{
    if (!m_Workbench) return;

    long style = wxFD_OPEN | wxFD_FILE_MUST_EXIST;
    wxString filter = CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
        CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
    wxFileDialog dlg(NULL, "Select ASN.1 File", wxGetCwd(), wxEmptyString, filter, style);

    if (dlg.ShowModal() != wxID_OK)
        return;

    vector<wxString> fileNames = { dlg.GetPath() };
    CIRef<IObjectLoader> loader(new CAsnObjectLoader(fileNames));

    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();

    if (!srv->HasWorkspace())
        srv->CreateNewWorkspace();

    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws) return;

    CSelectProjectOptions options;
    options.Set_CreateNewProject();

    CRef<CObjectLoadingTask> task(new CObjectLoadingTask(srv, *loader, options));
    task->AddProjectItemExtra("SmartFile", string(fileNames.front().ToUTF8()));
    m_Workbench->GetAppTaskService()->AddTask(*task);
}

void CIndexerAppModeExtension::OnCloseWorkspace(wxCommandEvent&)
{
    if (!m_Workbench) return;

    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();

    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws) return;

    vector<CGBDocument*> docs;
    for (CTypeIterator<CGBProjectHandle> it(ws->SetWorkspace()); it; ++it) {
        CGBDocument* doc = dynamic_cast<CGBDocument*>(&*it);
        if (doc && doc->GetDataModified())
            docs.push_back(doc);
    }

    if (docs.empty()) {
        srv->ResetWorkspace();
        CWorkspaceAutoSaver::CleanUp();
        return;
    }

    map<const CSerialObject*, string> files;
    for (auto doc : docs) {
        for (CTypeConstIterator<CProjectItem> it(doc->GetData()); it; ++it) {
            const CSerialObject* so = dynamic_cast<const CSerialObject*>(it->GetObject());
            if (!so) return;

            string smartFile;
            CProjectItemExtra::GetStr(*it, "SmartFile", smartFile);
            if (smartFile.empty()) continue;
            files[so] = smartFile;
        }

        if (!files.empty()) {
            wxString fileNames, msg = wxT("Want to save changes to "), delim;
            for (auto& f : files) {
                wxFileName fname(wxString::FromUTF8(f.second.c_str()));
                msg += delim + fname.GetFullName();
                delim = wxT(",");
            }
            msg += wxT("?");

            wxMessageDialog msgBox(nullptr, msg, wxT("Gemome Workbech"), wxYES_NO | wxCANCEL | wxICON_QUESTION);
            msgBox.SetYesNoLabels(wxID_SAVE, _("Do&n't save"));

            int result = msgBox.ShowModal();
            if (result == wxID_CANCEL)
                return;

            if (result == wxID_YES) {
                for (auto f : files) {
                    string errMsg;
                    wxString fileName = wxString::FromUTF8(f.second.c_str());
                    try {
                        CNcbiOfstream os(fileName.fn_str());
                        os << MSerial_AsnText << *f.first;
                    }
                    catch (const CException& e) {
                        errMsg = e.GetMsg();
                    }
                    catch (const exception& e) {
                        errMsg = e.what();
                    }

                    if (!errMsg.empty()) {
                        wxMessageBox(wxT("File: ") + fileName + wxT("\nError: ") + ToWxString(errMsg),
                            wxT("Failed to save file"), wxOK | wxICON_ERROR);
                    }
                }
            }
        }
    }

    srv->ResetWorkspace();
    CWorkspaceAutoSaver::CleanUp();
}


END_NCBI_SCOPE
