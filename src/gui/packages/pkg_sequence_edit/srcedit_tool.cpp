/*  $Id: srcedit_tool.cpp 38592 2017-06-01 15:36:57Z evgeniev $
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
* Authors:  Colleen Bollin, based on a file by Roman Katargin
*
*/

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_sequence_edit/srcedit_tool.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_panel.hpp>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/submit/Submit_block.hpp>
#include <objects/submit/Contact_info.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/biblio/Affil.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>
#include <objects/biblio/Auth_list.hpp>

#include <gui/widgets/wx/message_box.hpp>

#include <serial/typeinfo.hpp>

#include <objmgr/object_manager.hpp>

#include <gui/objutils/user_type.hpp>
#include <gui/objutils/label.hpp>

#include <objtools/cleanup/cleanup.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

///////////////////////////////////////////////////////////////////////////////
/// CSrcEditTool
CSrcEditTool::CSrcEditTool()
:   CAlgoToolManagerBase("Bulk Source Editing",
                         "",
                         "Bulk Source Editing",
                         "Assistant for preparing sequences "
                         "for submission to GenBank",
                         "SRCEDIT",
                         "Bulk Source Editing"),
                         m_Panel()
{
    m_Descriptor.SetLogEvent("tools");
}

string CSrcEditTool::GetExtensionIdentifier() const
{
    return "srcedit_tool";
}


string CSrcEditTool::GetExtensionLabel() const
{
    return "Submission Preparation Tool";
}

void CSrcEditTool::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_Panel = NULL;
}

void CSrcEditTool::CleanUI()
{
    m_Panel = NULL;
    CAlgoToolManagerBase::CleanUI();
}

bool CSrcEditTool::IsFinalState()
{
    return m_State == eParams;
}

bool CSrcEditTool::DoTransition(EAction action)
{
    if (m_State != eParams || action != eNext)
        return CAlgoToolManagerBase::DoTransition(action);

    if (x_GetParamsPanel()->TransferDataFromWindow()) {
        m_State = eCompleted;
        return true;
    }

    return false;
}

IAppTask* CSrcEditTool::GetTask()
{
    CRef<CDataLoadingAppJob> job(x_CreateLoadingJob());
    if (!job)
        return 0;

    CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
    CSelectProjectOptions options;
    options.Set_CreateNewProject();
    CRef<CDataLoadingAppTask> task(new CDataLoadingAppTask(srv, options, *job));
    return task.Release();
}

void CSrcEditTool::x_CreateParamsPanelIfNeeded()
{
    if (m_Panel == NULL) {
        x_SelectCompatibleInputObjects();

        m_Panel = new CSrcEditPanel();
        m_Panel->Hide(); // to reduce flicker
        m_Panel->Create(m_ParentWindow);
        m_Panel->SetData(m_Params);
        m_Panel->SetObjects(&m_Objects);

        m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_Panel->LoadSettings();
    }
}

bool CSrcEditTool::x_ValidateParams()
{
    return true;
}

void CSrcEditTool::x_SelectCompatibleInputObjects()
{
    m_Objects.clear();

    ITERATE(vector<TConstScopedObjects>, it, m_InputObjects) {
        ITERATE(TConstScopedObjects, it2, *it) {
            const CObject* ptr = it2->object.GetPointer();
            if (dynamic_cast<const CSeq_entry*>(ptr) ||
                dynamic_cast<const CSeq_annot*>(ptr) ||
                dynamic_cast<const CSeq_submit*>(ptr) ||
                dynamic_cast<const CSeq_id*>(ptr))
                m_Objects.push_back(*it2);
        }
    }
}

CAlgoToolManagerParamsPanel* CSrcEditTool::x_GetParamsPanel()
{
    return m_Panel;
}

IRegSettings* CSrcEditTool::x_GetParamsAsRegSetting()
{
    return &m_Params;
}

///////////////////////////////////////////////////////////////////////////////
/// CCleanupJob
class  CSrcEditJob  : public CDataLoadingAppJob
{
public:
    CSrcEditJob (const CSrcEditParams& params);

protected:
    virtual void    x_CreateProjectItems(); // overriding virtual function

protected:
    CSrcEditParams m_Params;
};

CDataLoadingAppJob* CSrcEditTool::x_CreateLoadingJob()
{
    if (m_Panel) m_Params = m_Panel->GetData();
    CSrcEditJob* job = new CSrcEditJob(m_Params);
    return job;
}

CSrcEditJob::CSrcEditJob(const CSrcEditParams& params)
:   m_Params(params)
{
    CFastMutexGuard lock(m_Mutex);

    m_Descr = "Submission Preparation Job"; //TODO
}

void CSrcEditJob::x_CreateProjectItems()
{
    //bool error_occurred = false;
    string errmsg = "";
    string changelist = "";
    CRef<objects::CSeq_submit> submit;
    vector<CRef<objects::CSeq_entry> > entries;

    ITERATE (TConstScopedObjects, it, m_Params.m_Objects) {
        const CObject* ptr = it->object.GetPointer();

        /// CSeq_entry
        const objects::CSeq_entry* seqEntry = dynamic_cast<const objects::CSeq_entry*>(ptr);
        if (seqEntry) {
            CRef<objects::CSeq_entry> e(new objects::CSeq_entry());
            e->Assign(*seqEntry);
            entries.push_back(e);
        } else {
            const objects::CSeq_submit* seqsubmit = dynamic_cast<const objects::CSeq_submit*>(ptr);
            if (seqsubmit) {
                submit.Reset(new objects::CSeq_submit());
                submit->Assign(*submit);
            }
        }
    }
    if (!submit) {
        submit = new objects::CSeq_submit();
        submit->SetSub().SetContact().SetFirst_name("");
        submit->SetSub().SetContact().SetLast_name("");
        CRef<CAuthor> new_auth(new CAuthor());
        new_auth->SetName().SetName().SetLast("");
        new_auth->SetName().SetName().SetFirst("");
        submit->SetSub().SetCit().SetAuthors().SetNames().SetStd().push_back(new_auth);
    }
    ITERATE (vector<CRef<objects::CSeq_entry> >, it, entries) {
        submit->SetData().SetEntrys().push_back (*it);
    }
    CRef<CProjectItem> item(new CProjectItem());
    item->SetItem().SetSubmit(*submit);

	CLabel::SetLabelByData(*item, m_Scope.GetPointer());

    AddProjectItem(*item);

}


END_NCBI_SCOPE
