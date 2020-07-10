/*  $Id: ngalign_tool.cpp 38720 2017-06-12 20:49:12Z evgeniev $
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

#include <objects/seqloc/Seq_id.hpp>
#include <serial/serial.hpp>
#include <serial/typeinfo.hpp>

#include <gui/packages/pkg_alignment/ngalign_tool.hpp>
#include <gui/packages/pkg_alignment/ngalign_job.hpp>
#include "ngalign_panel.hpp"
#include "ngalign_blast_panel.hpp"

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <objmgr/object_manager.hpp>

#include <gui/objutils/label.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/gui_seq_info.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

///////////////////////////////////////////////////////////////////////////////
/// CNGAlignTool
CNGAlignTool::CNGAlignTool()
:   CAlgoToolManagerBase("Genomic Aligner (NG Aligner)",
                         "",
                         "NCBI Genomic Aligner",
                         "The Genomic Aligner, also known as the NG aligner, attempts to find polished, consistent alignments of short-to-medium length genomic sequences (up to 1 megabase) aligned to long genomic sequences such as chromosomes.",
                         "NGALIGN",
                         "Alignment Creation"),
    m_Panel(), m_BLASTPanel(), m_CurrentPage(0)
{
}

string CNGAlignTool::GetExtensionIdentifier() const
{
    return "ngalign_tool";
}


string CNGAlignTool::GetExtensionLabel() const
{
    return "NGAlign Tool";
}

void CNGAlignTool::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_Panel = NULL;
    m_BLASTPanel = NULL;
}

void CNGAlignTool::CleanUI()
{
    m_Panel = NULL;
    m_BLASTPanel = NULL;
    CAlgoToolManagerBase::CleanUI();
}

bool CNGAlignTool::IsFinalState()
{
    return UsesSingleOMScope() ? (m_State == eParams && 1 == m_CurrentPage) : m_State == eSelectProject;
}


bool CNGAlignTool::DoTransition(EAction action)
{
    bool next = (action == eNext);

    if (m_State == eInvalid) {
        if (next) {
            /// initial state - display first page
            m_State = eParams;
            return x_GoFirstPage();
        }
    }
    else if (m_State == eParams) {
        if (x_IsFirstPage() && m_Panel)
            m_Panel->SaveSettings();

        if (next) {
            wxPanel* panel = x_GetCurrentPage();
            _ASSERT(panel);
            if (panel->Validate() && panel->TransferDataFromWindow()) {
                if (x_IsLastPage()) {
                    x_CreateProjectPanelIfNeeded();
                    // The new code begins here.
                    if (m_ProjectSelPanel) {
                        m_ProjectSelPanel->TransferDataToWindow();
                        m_State = eSelectProject;
                    } else {
                        m_State = eCompleted;
                    }
                    // The new code ends here.
                    return true;
                }
                return x_GoNextPage();
            }
            return false;
        }
        else {
            if (x_IsFirstPage()) {
                m_State = eInvalid;
                return true;
            }
            return x_GoPrevPage();
        }
    }
    else if (m_State == eSelectProject) {
        if (next) {
            if(m_ProjectSelPanel->TransferDataFromWindow()) {
                m_State = eCompleted;
                return true;
            }
            return false;
        }
        else {
            m_State = eParams;
            return x_GoLastPage();
        }
    }

    _ASSERT(false);
    return false;
}

wxPanel* CNGAlignTool::GetCurrentPanel()
{
    switch(m_State) {
    case eParams:
        return x_GetCurrentPage();
    case eSelectProject:
        return m_ProjectSelPanel;
    default:
        return NULL;
    }
}

bool CNGAlignTool::x_GoFirstPage()
{
    m_CurrentPage = 0;
    return x_LoadPage();
}

bool CNGAlignTool::x_GoNextPage()
{
    _ASSERT(m_CurrentPage == 0);
    ++m_CurrentPage;
    return x_LoadPage();
}

bool CNGAlignTool::x_GoPrevPage()
{
    _ASSERT(m_CurrentPage == 1);
    --m_CurrentPage;
    return x_LoadPage();
}

bool CNGAlignTool::x_GoLastPage()
{
    m_CurrentPage = 1;
    return x_LoadPage();
}

bool CNGAlignTool::x_LoadPage()
{
    if (m_CurrentPage == 0)
        x_CreateParamsPanelIfNeeded();
    else
        x_CreateBALSTPanelIfNeeded();

    wxPanel* panel = x_GetCurrentPage();
    _ASSERT(panel);
    if (panel) {
        panel->TransferDataToWindow();
        return true;
    }
    NcbiErrorBox("Failed to create parameters page");
    return false;
}


wxPanel* CNGAlignTool::x_GetCurrentPage()
{
    if (m_CurrentPage == 0)
        return m_Panel;
    else if (m_CurrentPage == 1)
        return m_BLASTPanel;

    return NULL;
}

bool CNGAlignTool::x_IsFirstPage() const
{
    return (m_CurrentPage == 0);
}

bool CNGAlignTool::x_IsLastPage() const
{
    return (m_CurrentPage == 1);
}

void CNGAlignTool::x_CreateParamsPanelIfNeeded()
{
    if (m_Panel == NULL) {

        x_SelectCompatibleInputObjects();

        m_Panel = new CNGAlignPanel(m_ParentWindow);
        m_Panel->SetObjects(&m_SeqIds);
        m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_Panel->LoadSettings();
    }
}

void CNGAlignTool::x_CreateBALSTPanelIfNeeded()
{
    if (m_BLASTPanel == NULL) {
        m_BLASTPanel = new CNGAlignBLASTPanel(m_ParentWindow);
        m_BLASTPanel->SetRegistryPath(m_RegPath + ".BLASTPanel");
        m_BLASTPanel->LoadSettings();
    }
}

bool CNGAlignTool::x_ValidateParams()
{
    return true;
}

void CNGAlignTool::x_SelectCompatibleInputObjects()
{
    TConstScopedObjects objects;
    x_ConvertInputObjects(CSeq_loc::GetTypeInfo(), objects);

    m_SeqIds.clear();
    NON_CONST_ITERATE(TConstScopedObjects, it, objects) {
        CIRef<IGuiSeqInfo> gui_seq_info(CreateObjectInterface<IGuiSeqInfo>(*it, NULL));
        if( !gui_seq_info.IsNull() && gui_seq_info->IsDNA() ){
            m_SeqIds.push_back(*it);
        }
    }
}

CAlgoToolManagerParamsPanel* CNGAlignTool::x_GetParamsPanel()
{
    return m_Panel;
}

IRegSettings* CNGAlignTool::x_GetParamsAsRegSetting()
{
    return &m_Params;
}

CDataLoadingAppJob* CNGAlignTool::x_CreateLoadingJob()
{
    if (m_BLASTPanel) m_Params = m_BLASTPanel->GetData();
    if (m_Panel) {
        m_Params.SetSubject() = m_Panel->GetSubject();
        m_Params.SetQueries() = m_Panel->GetQueries();
    }

    CNGAlignJob* job = new CNGAlignJob(m_Params);
    return job;
}

END_NCBI_SCOPE
