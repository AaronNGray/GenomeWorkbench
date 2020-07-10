/*  $Id: taxtree_tool.cpp 30812 2014-07-25 18:57:09Z katargir $
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

#include <gui/packages/pkg_alignment/taxtree_tool.hpp>
#include <gui/packages/pkg_alignment/taxtree_tool_job.hpp>
#include "taxtree_tool_panel.hpp"

#include <gui/widgets/wx/message_box.hpp>

#include <serial/typeinfo.hpp>

#include <gui/objutils/user_type.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

///////////////////////////////////////////////////////////////////////////////
/// CCommonTaxTreeTool
CCommonTaxTreeTool::CCommonTaxTreeTool()
:   CAlgoToolManagerBase("Common taxonomic tree for sequences",
                         "",
                         "Common taxonomic tree for sequences",
                         "Create a bio-tree object of common taxonomic tree",
                         "CommonTaxTree",
                         "Alignment Creation"),
                         m_Panel()
{
}

string CCommonTaxTreeTool::GetExtensionIdentifier() const
{
    return "common_taxtree_tool";
}


string CCommonTaxTreeTool::GetExtensionLabel() const
{
    return "Common TaxTree Tool";
}

void CCommonTaxTreeTool::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_Panel = NULL;
}

void CCommonTaxTreeTool::CleanUI()
{
    m_Panel = NULL;
    CAlgoToolManagerBase::CleanUI();
}

void CCommonTaxTreeTool::x_CreateParamsPanelIfNeeded()
{
    if (m_Panel == NULL) {
        x_SelectCompatibleInputObjects();

        m_Panel = new CTaxTreeToolPanel(m_ParentWindow, wxID_ANY,
                    wxDefaultPosition, wxSize(0, 0), SYMBOL_CTAXTREETOOLPANEL_STYLE, false);
        m_Panel->SetData(m_Params);
        m_Panel->SetObjects(&m_Objects);

        m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_Panel->LoadSettings();
    }
}

bool CCommonTaxTreeTool::x_ValidateParams()
{
    return true;
}

void CCommonTaxTreeTool::x_SelectCompatibleInputObjects()
{
    x_ConvertInputObjects(CSeq_id::GetTypeInfo(), m_Objects);
}

wxPanel* CCommonTaxTreeTool::GetMaxPanel()
{
    CTaxTreeToolPanel* panel = new CTaxTreeToolPanel();
    panel->Create(m_ParentWindow);
    return panel;
}

CAlgoToolManagerParamsPanel* CCommonTaxTreeTool::x_GetParamsPanel()
{
    return m_Panel;
}

IRegSettings* CCommonTaxTreeTool::x_GetParamsAsRegSetting()
{
    return &m_Params;
}

CDataLoadingAppJob* CCommonTaxTreeTool::x_CreateLoadingJob()
{
    if (m_Panel) m_Params = m_Panel->GetData();
    CTaxTreeToolJob* job = new CTaxTreeToolJob(m_Params);
    return job;
}

END_NCBI_SCOPE
