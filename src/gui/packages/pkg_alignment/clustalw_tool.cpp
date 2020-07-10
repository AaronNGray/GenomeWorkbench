/*  $Id: clustalw_tool.cpp 38402 2017-05-04 17:24:36Z evgeniev $
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

#include <gui/packages/pkg_alignment/clustalw_tool.hpp>
#include <gui/packages/pkg_alignment/clustalw_tool_job.hpp>
#include "clustalw_tool_panel.hpp"

#include <gui/widgets/wx/message_box.hpp>

#include <serial/typeinfo.hpp>

#include <gui/objutils/user_type.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

///////////////////////////////////////////////////////////////////////////////
/// CClustalwTool
CClustalwTool::CClustalwTool()
:   CAlgoToolManagerBase("ClustalW2",
                         "",
                         "Create an alignment using ClustalW2",
                         "ClustalW is a general purpose DNA or protein multiple sequence alignment program",
                         "MSATools",
                         "Multiple Sequence Aligners"),
                         m_Panel()
{
}

string CClustalwTool::GetExtensionIdentifier() const
{
    return "clustalw_tool";
}


string CClustalwTool::GetExtensionLabel() const
{
    return "ClustalW Tool";
}

void CClustalwTool::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_Panel = NULL;
}

void CClustalwTool::CleanUI()
{
    m_Panel = NULL;
    CAlgoToolManagerBase::CleanUI();
}

void CClustalwTool::x_CreateParamsPanelIfNeeded()
{
    if (m_Panel == NULL) {
        x_SelectCompatibleInputObjects();

        m_Panel = new CClustalwToolPanel(m_ParentWindow, wxID_ANY,
                                   wxDefaultPosition, wxSize(0, 0), SYMBOL_CCLUSTALWTOOLPANEL_STYLE, false);
        m_Panel->SetData(m_Params);
        m_Panel->SetObjects(&m_Objects);

        m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_Panel->LoadSettings();
    }
}

bool CClustalwTool::x_ValidateParams()
{
    return true;
}

void CClustalwTool::x_SelectCompatibleInputObjects()
{
    x_ConvertInputObjects(CSeq_loc::GetTypeInfo(), m_Objects);
}

wxPanel* CClustalwTool::GetMaxPanel()
{
    CClustalwToolPanel* panel = new CClustalwToolPanel();
    panel->Create(m_ParentWindow);
    return panel;
}

CAlgoToolManagerParamsPanel* CClustalwTool::x_GetParamsPanel()
{
    return m_Panel;
}

IRegSettings* CClustalwTool::x_GetParamsAsRegSetting()
{
    return &m_Params;
}

CDataLoadingAppJob* CClustalwTool::x_CreateLoadingJob()
{
    if (m_Panel) m_Params = m_Panel->GetData();
    CClustalwToolJob* job = new CClustalwToolJob(m_Params);
    return job;
}

END_NCBI_SCOPE
