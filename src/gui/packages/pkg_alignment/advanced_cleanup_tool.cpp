/*  $Id: advanced_cleanup_tool.cpp 44061 2019-10-17 18:21:00Z katargir $
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

#include <gui/objutils/label.hpp>

#include "advanced_cleanup_tool.hpp"
#include "advanced_cleanup_params_panel.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

///////////////////////////////////////////////////////////////////////////////
/// CAdvancedCleanupTool
CAdvancedCleanupTool::CAdvancedCleanupTool()
:   CAlgoToolManagerBase("Advanced Cleanup",
                         "",
                         "Remove redundant elements from alignments",
                         "Remove redundant elements from alignments",
                         "CLEAN_ALIGNMENTS",
                         "Alignment Creation"),
                         m_Panel()
{
}

string CAdvancedCleanupTool::GetExtensionIdentifier() const
{
    return "advanced_cleanup_tool";
}


string CAdvancedCleanupTool::GetExtensionLabel() const
{
    return "Advanced Cleanup Tool";
}

void CAdvancedCleanupTool::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_Panel = NULL;
}

void CAdvancedCleanupTool::CleanUI()
{
    m_Panel = NULL;
    CAlgoToolManagerBase::CleanUI();
}

void CAdvancedCleanupTool::x_CreateParamsPanelIfNeeded()
{
    if (m_Panel == NULL) {
        x_SelectCompatibleInputObjects();

        m_Panel = new CAdvancedCleanupParamsPanel(m_ParentWindow, wxID_ANY,
            wxDefaultPosition, wxSize(0, 0), SYMBOL_CADVANCEDCLEANUPPARAMSPANEL_STYLE);
        m_Panel->SetData(m_Params);
        m_Panel->SetObjects(&m_Alignments);

        m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_Panel->LoadSettings();
    }
}

bool CAdvancedCleanupTool::x_ValidateParams()
{
    return true;
}

void CAdvancedCleanupTool::x_SelectCompatibleInputObjects()
{
    m_Params.m_Objects.clear();
    m_Alignments.clear();
    ITERATE(vector<TConstScopedObjects>, it, m_InputObjects) {
        ITERATE(TConstScopedObjects, it2, *it) {
            const CSeq_align* align = dynamic_cast<const CSeq_align*>(it2->object.GetPointerOrNull());
            if (align) m_Alignments.push_back(*it2);
        }
    }

    if (m_Alignments.empty())
        x_ConvertInputObjects(CSeq_align::GetTypeInfo(), m_Alignments);
}

wxPanel* CAdvancedCleanupTool::GetMaxPanel()
{
    CAdvancedCleanupParamsPanel* panel = new CAdvancedCleanupParamsPanel();
    panel->Create(m_ParentWindow);
    return panel;
}

CAlgoToolManagerParamsPanel* CAdvancedCleanupTool::x_GetParamsPanel()
{
    return m_Panel;
}

IRegSettings* CAdvancedCleanupTool::x_GetParamsAsRegSetting()
{
    return &m_Params;
}

///////////////////////////////////////////////////////////////////////////////
/// CAdvancedCleanupJob
class  CAdvancedCleanupJob : public CDataLoadingAppJob
{
public:
    CAdvancedCleanupJob(const CAdvancedCleanupParams& params);

protected:
    virtual void    x_CreateProjectItems();

protected:
    CAdvancedCleanupParams  m_Params;
};

CDataLoadingAppJob* CAdvancedCleanupTool::x_CreateLoadingJob()
{
    if (m_Panel) m_Params = m_Panel->GetData();
    return new CAdvancedCleanupJob(m_Params);
}

CAdvancedCleanupJob::CAdvancedCleanupJob(const CAdvancedCleanupParams& params)
    : m_Params(params)
{
    m_Descr = "Advanced Cleanup Job";
}

void CAdvancedCleanupJob::x_CreateProjectItems()
{
    list< CRef< CSeq_align > > input_aligns, cleaned_aligns;

    CRef<CScope> scope;
    for (auto& o : m_Params.GetObjects()) {
        const CSeq_align* seq_align = dynamic_cast<const CSeq_align*>(o.object.GetPointer());
        if (seq_align) {
            if (!scope)
                scope = o.scope;
            input_aligns.emplace_back(const_cast<CSeq_align*>(seq_align));
        }
    }

    if (input_aligns.empty()) {
        LOG_POST(Error << "Advanced Cleanup. Input alignments list is empty");
        return;
    }

    CAdvancedAlignCleanup cleanup;

    unique_ptr<CArgDescriptions> arg_desc(new CArgDescriptions());
    CAdvancedAlignCleanup::SetupArgDescriptions(*arg_desc);
    CNcbiArguments argv(0, nullptr);
    unique_ptr<CArgs> args(arg_desc->CreateArgs(argv));
    cleanup.SetParams(*args);

    cleanup.SetScope(scope);

    CAdvancedAlignCleanup::EQueryType queryType = CAdvancedAlignCleanup::eInfer;
    switch (m_Params.GetQueryTpe()) {
    case CAdvancedAlignCleanup::eGenomic:
        queryType = CAdvancedAlignCleanup::eGenomic;
        break;
    case CAdvancedAlignCleanup::eRna:
        queryType = CAdvancedAlignCleanup::eRna;
        break;
    case CAdvancedAlignCleanup::eProtein:
        queryType = CAdvancedAlignCleanup::eProtein;
        break;
    default:
        queryType = CAdvancedAlignCleanup::eInfer;
        break;
    };

    CAdvancedAlignCleanup::ESplignDirRun splignDir = CAdvancedAlignCleanup::eDirBoth;
    switch (m_Params.GetSplignDirection()) {
    case CAdvancedAlignCleanup::eDirSense :
        splignDir = CAdvancedAlignCleanup::eDirSense;
        break;
    case CAdvancedAlignCleanup::eDirAntisense :
        splignDir = CAdvancedAlignCleanup::eDirAntisense;
        break;
    default :
        splignDir = CAdvancedAlignCleanup::eDirBoth;
        break;
    };

    cleanup.Cleanup(input_aligns, cleaned_aligns, queryType, m_Params.GetInvokeBest(), false, splignDir);

    if (cleaned_aligns.empty())
        return;

    CRef<CSeq_annot> annot(new CSeq_annot());
    auto& seqannot_align = annot->SetData().SetAlign();
    for (auto& a : cleaned_aligns)
        seqannot_align.push_back(a);
    annot->SetCreateDate(CTime(CTime::eCurrent));

    string name("Cleaned Alignment: ");
    CLabel::GetLabel(*annot, &name, CLabel::eDefault, &*scope);
    if (!name.empty()) {
        annot->SetNameDesc(name);
        annot->SetTitleDesc(name);
    }

    CRef<CProjectItem> pitem(new CProjectItem());

    pitem->SetItem().SetAnnot(*annot);
    pitem->SetLabel(name);

    AddProjectItem(*pitem);
}

END_NCBI_SCOPE
