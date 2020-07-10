/*  $Id: cleanup_tool.cpp 38477 2017-05-15 21:10:59Z evgeniev $
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

#include <gui/packages/pkg_sequence/cleanup_tool.hpp>
#include <gui/packages/pkg_sequence/cleanup_panel.hpp>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/submit/Seq_submit.hpp>

#include <gui/widgets/wx/message_box.hpp>

#include <serial/typeinfo.hpp>

#include <objmgr/object_manager.hpp>

#include <gui/objutils/user_type.hpp>
#include <gui/objutils/label.hpp>

#include <objtools/cleanup/cleanup.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

///////////////////////////////////////////////////////////////////////////////
/// CCleanupTool
CCleanupTool::CCleanupTool()
:   CAlgoToolManagerBase("Basic/Extended Cleanup",
                         "",
                         "Basic/Extended Cleanup",
                         "Basic record cleanup - performs some trivial "
                         "data rearrangements and clarifications",
                         "CLEANUP",
                         "Edit"),
                         m_Panel()
{
}

string CCleanupTool::GetExtensionIdentifier() const
{
    return "cleanup_tool";
}


string CCleanupTool::GetExtensionLabel() const
{
    return "Cleanup Tool";
}

void CCleanupTool::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_Panel = NULL;
}

void CCleanupTool::CleanUI()
{
    m_Panel = NULL;
    CAlgoToolManagerBase::CleanUI();
}

bool CCleanupTool::IsFinalState()
{
    return m_State == eParams;
}

bool CCleanupTool::DoTransition(EAction action)
{
    if (m_State != eParams || action != eNext)
        return CAlgoToolManagerBase::DoTransition(action);

    if (x_GetParamsPanel()->TransferDataFromWindow()) {
        m_State = eCompleted;
        return true;
    }

    return false;
}

IAppTask* CCleanupTool::GetTask()
{
    CRef<CDataLoadingAppJob> job(x_CreateLoadingJob());
    if (!job)
        return 0;

    CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
    CSelectProjectOptions options;
    CRef<CDataLoadingAppTask> task(new CDataLoadingAppTask(srv, options, *job));
    return task.Release();
}

void CCleanupTool::x_CreateParamsPanelIfNeeded()
{
    if (m_Panel == NULL) {
        x_SelectCompatibleInputObjects();

        m_Panel = new CCleanupPanel();
        m_Panel->Hide(); // to reduce flicker
        m_Panel->Create(m_ParentWindow);
        m_Panel->SetData(m_Params);
        m_Panel->SetObjects(&m_Objects);

        m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_Panel->LoadSettings();
    }
}

bool CCleanupTool::x_ValidateParams()
{
    return true;
}

void CCleanupTool::x_SelectCompatibleInputObjects()
{
    m_Objects.clear();

    NON_CONST_ITERATE(vector<TConstScopedObjects>, it, m_InputObjects) {
        NON_CONST_ITERATE(TConstScopedObjects, it2, *it) {
            const CObject* ptr = it2->object.GetPointer();
            if (dynamic_cast<const CSeq_entry*>(ptr) ||
                dynamic_cast<const CSeq_annot*>(ptr) ||
                dynamic_cast<const CSeq_submit*>(ptr) ||
                dynamic_cast<const CSeq_id*>(ptr))
                m_Objects.push_back(*it2);
        }
    }
}

CAlgoToolManagerParamsPanel* CCleanupTool::x_GetParamsPanel()
{
    return m_Panel;
}

IRegSettings* CCleanupTool::x_GetParamsAsRegSetting()
{
    return &m_Params;
}

///////////////////////////////////////////////////////////////////////////////
/// CCleanupJob
class  CCleanupJob  : public CDataLoadingAppJob
{
public:
    CCleanupJob (const CCleanupParams& params);

protected:
    virtual void    x_CreateProjectItems(); // overriding virtual function

protected:
    CCleanupParams m_Params;
};

CDataLoadingAppJob* CCleanupTool::x_CreateLoadingJob()
{
    LOG_POST(Info << "CCleanupTool CleanupMode=" << m_Params.GetCleanupMode());

    if (m_Panel) m_Params = m_Panel->GetData();
    CCleanupJob* job = new CCleanupJob(m_Params);
    return job;
}

CCleanupJob::CCleanupJob(const CCleanupParams& params)
:   m_Params(params)
{
    CFastMutexGuard lock(m_Mutex);

    m_Descr = "Cleanup Job"; //TODO
}

// This is a utility function for converting a list of changes to a single string
// with carriage returns between each change.

static void s_ListChanges(CConstRef<CCleanupChange> changes, string& changelist)
{
    vector<string> changes_str = changes->GetAllDescriptions();
    ITERATE(vector<string>, vit, changes_str) {
        if (!NStr::IsBlank(changelist)) {
            changelist += "\n";
        }
        changelist += *vit;
    }
}

void CCleanupJob::x_CreateProjectItems()
{
    bool error_occurred = false;
    string errmsg = "";
    string changelist = "";

    ITERATE (TConstScopedObjects, it, m_Params.GetObjects()) {
        CCleanup cleanup;
        CRef<CScope> scope(it->scope);
        cleanup.SetScope(scope);

        const CObject* ptr = it->object.GetPointer();

        /// CSeq_entry
        const CSeq_entry* seqEntry = dynamic_cast<const CSeq_entry*>(ptr);
        if (seqEntry) {
            try {
                CSeq_entry& obj_ref = const_cast<CSeq_entry&>(*seqEntry);
                CConstRef<CCleanupChange> changes =
                    (m_Params.GetCleanupMode() == 0) ?
                        cleanup.BasicCleanup(obj_ref) : cleanup.ExtendedCleanup(obj_ref);
                s_ListChanges(changes, changelist);
            }
            catch (CException& e) {
                error_occurred = true;
                errmsg += "error in basic cleanup: ";
                errmsg += e.GetMsg();
                errmsg += "\n";
            }
            continue;
        }

        /// CSeq_annot
        const CSeq_annot* seqAnnot = dynamic_cast<const CSeq_annot*>(ptr);
        if (seqAnnot) {
            try {
                CSeq_annot& obj_ref = const_cast<CSeq_annot&>(*seqAnnot);
                CConstRef<CCleanupChange> changes =
                    (m_Params.GetCleanupMode() == 0) ?
                    cleanup.BasicCleanup(obj_ref) : cleanup.ExtendedCleanup(obj_ref);
                s_ListChanges(changes, changelist);
            }
            catch (CException& e) {
                error_occurred = true;
                errmsg += "error in basic cleanup: ";
                errmsg += e.GetMsg();
                errmsg += "\n";
            }
            continue;
        }

        /// CSeq_annot
        const CSeq_submit* seqSubmit = dynamic_cast<const CSeq_submit*>(ptr);
        if (seqSubmit) {
            try {
                CSeq_submit& obj_ref = const_cast<CSeq_submit&>(*seqSubmit);
                CConstRef<CCleanupChange> changes =
                    (m_Params.GetCleanupMode() == 0) ?
                    cleanup.BasicCleanup(obj_ref) : cleanup.ExtendedCleanup(obj_ref);
                s_ListChanges(changes, changelist);
            }
            catch (CException& e) {
                error_occurred = true;
                errmsg += "error in basic cleanup: ";
                errmsg += e.GetMsg();
                errmsg += "\n";
            }
            continue;
        }

        /// CSeq_id
        const CSeq_id* seqId = dynamic_cast<const CSeq_id*>(ptr);
        if (seqId) {
            try {
                CSeq_id& obj_ref = const_cast<CSeq_id&>(*seqId);
                CBioseq_Handle bh = scope->GetBioseqHandle(obj_ref);
                CConstRef<CCleanupChange> changes;

                if (m_Params.GetCleanupMode() == 0)
                    changes = cleanup.BasicCleanup(bh);
                else
                    changes = cleanup.ExtendedCleanup(const_cast <CSeq_entry&>
                                    (*(bh.GetSeq_entry_Handle().GetCompleteSeq_entry())));

                s_ListChanges(changes, changelist);
            }
            catch (CException& e) {
                error_occurred = true;
                errmsg += "error in basic cleanup: ";
                errmsg += e.GetMsg();
                errmsg += "\n";
            }
            continue;
        }
    }
}


END_NCBI_SCOPE
