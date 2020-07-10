/*  $Id: debug_macro_tool.cpp 38592 2017-06-01 15:36:57Z evgeniev $
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

#include <gui/packages/pkg_sequence_edit/debug_macro_tool.hpp>
#include <gui/packages/pkg_sequence_edit/debug_macro_tool_job.hpp>
#include "debug_macro_tool_panel.hpp"

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

///////////////////////////////////////////////////////////////////////////////
/// CDebugMacroTool
CDebugMacroTool::CDebugMacroTool()
:   CAlgoToolManagerBase("Debug Macro Tool",
                         "",
                         "Debug Macro Tool",
                         "Debug Macro Tool",
                         "Debug_Macro",
                         "Macro testing"),
                         m_Panel()
{
    m_Descriptor.SetLogEvent("tools");
}

string CDebugMacroTool::GetExtensionIdentifier() const
{
    return "debug_macro_tool";
}


string CDebugMacroTool::GetExtensionLabel() const
{
    return "Debug Macro Tool";
}

void CDebugMacroTool::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_Panel = NULL;
}

void CDebugMacroTool::CleanUI()
{
    m_Panel = NULL;
    CAlgoToolManagerBase::CleanUI();
}

void CDebugMacroTool::x_CreateParamsPanelIfNeeded()
{
    if (m_Panel == NULL) {
        x_SelectCompatibleInputObjects();
        x_SelectMacro();

        m_Panel = new CDebugMacroToolPanel(m_ParentWindow, wxID_ANY,
                                   wxDefaultPosition, wxSize(0, 0), SYMBOL_CDEBUGMACROTOOLPANEL_STYLE);
        m_Panel->SetData(m_Params);
        m_Panel->SetObjects(&m_Objects);

        m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_Panel->LoadSettings();
    }
}

bool CDebugMacroTool::x_ValidateParams()
{
    return true;
}

void CDebugMacroTool::x_SelectCompatibleInputObjects()
{
    x_ConvertInputObjects(CSeq_entry::GetTypeInfo(), m_Objects);
}

void CDebugMacroTool::x_SelectMacro()
{   
    char *sDebugMacro = "MACRO First_M \"for debugging\""               "\n"
                        "FOR EACH SeqFeat"                              "\n"
                        "WHERE feature.note = \"[intronless gene]\""    "\n"
                        "DO"                                            "\n"
                        "   RemoveComment ()"                           "\n"
                        "DONE"                                          "\n";

    m_Params.SetText(ToWxString(sDebugMacro));  
}


wxPanel* CDebugMacroTool::GetMaxPanel()
{
    CDebugMacroToolPanel* panel = new CDebugMacroToolPanel();
    panel->Create(m_ParentWindow);
    return panel;
}

CAlgoToolManagerParamsPanel* CDebugMacroTool::x_GetParamsPanel()
{
    return m_Panel;
}

IRegSettings* CDebugMacroTool::x_GetParamsAsRegSetting()
{
    return &m_Params;
}

CDataLoadingAppJob* CDebugMacroTool::x_CreateLoadingJob()
{
    if (m_Panel) m_Params = m_Panel->GetData();
    CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
    CDebugMacroToolJob* job = new CDebugMacroToolJob(m_Params);
    return job;
}

END_NCBI_SCOPE
