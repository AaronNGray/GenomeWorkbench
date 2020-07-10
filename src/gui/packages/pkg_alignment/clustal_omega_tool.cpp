/*  $Id: clustal_omega_tool.cpp 38402 2017-05-04 17:24:36Z evgeniev $
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
* Authors:  Roman Katargin, Vladislav Evgeniev
*
*/

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_alignment/clustal_omega_tool.hpp>
#include <gui/packages/pkg_alignment/clustal_omega_tool_job.hpp>
#include "clustal_omega_tool_panel.hpp"

#include <gui/widgets/wx/message_box.hpp>

#include <serial/typeinfo.hpp>

#include <gui/objutils/user_type.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

///////////////////////////////////////////////////////////////////////////////
/// CClustalOmegaTool
CClustalOmegaTool::CClustalOmegaTool()
:   CAlgoToolManagerBase("Clustal Omega",
                         "",
                         "Create an alignment using Clustal Omega",
                         "Fast and scalable multiple sequence alignment tool. Provides greater accuracy due to new HMM alignment engine.",
                         "MSATools",
                         "Multiple Sequence Aligners"),
                         m_Panel()
{
}

string CClustalOmegaTool::GetExtensionIdentifier() const
{
    return "clustal_omega_tool";
}


string CClustalOmegaTool::GetExtensionLabel() const
{
    return "Clustal Omega Tool";
}

void CClustalOmegaTool::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_Panel = NULL;
}

void CClustalOmegaTool::CleanUI()
{
    m_Panel = NULL;
    CAlgoToolManagerBase::CleanUI();
}

void CClustalOmegaTool::x_CreateParamsPanelIfNeeded()
{
    if (m_Panel == NULL) {
        x_SelectCompatibleInputObjects();

        m_Panel = new CClustalOmegaToolPanel(m_ParentWindow, wxID_ANY,
                                   wxDefaultPosition, wxSize(0, 0), SYMBOL_CCLUSTALOMEGATOOLPANEL_STYLE, false);
        m_Panel->SetData(m_Params);
        m_Panel->SetObjects(&m_Objects);

        m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_Panel->LoadSettings();
    }
}

bool CClustalOmegaTool::x_ValidateParams()
{
    return true;
}

void CClustalOmegaTool::x_SelectCompatibleInputObjects()
{
    x_ConvertInputObjects(CSeq_loc::GetTypeInfo(), m_Objects);
}

wxPanel* CClustalOmegaTool::GetMaxPanel()
{
    CClustalOmegaToolPanel* panel = new CClustalOmegaToolPanel();
    panel->Create(m_ParentWindow);
    return panel;
}

CAlgoToolManagerParamsPanel* CClustalOmegaTool::x_GetParamsPanel()
{
    return m_Panel;
}

IRegSettings* CClustalOmegaTool::x_GetParamsAsRegSetting()
{
    return &m_Params;
}

CDataLoadingAppJob* CClustalOmegaTool::x_CreateLoadingJob()
{
    if (m_Panel) m_Params = m_Panel->GetData();
    CClustalOmegaToolJob* job = new CClustalOmegaToolJob(m_Params);
    return job;
}

END_NCBI_SCOPE
