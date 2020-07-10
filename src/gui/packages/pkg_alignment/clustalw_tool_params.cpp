/*  $Id: clustalw_tool_params.cpp 43795 2019-08-30 19:22:56Z katargir $
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
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/packages/pkg_alignment/clustalw_tool_params.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

CClustalwToolParams::CClustalwToolParams()
{
    Init();
}
CClustalwToolParams::CClustalwToolParams(const CClustalwToolParams& data)
{
    Init();
    Copy(data);
}
CClustalwToolParams::~CClustalwToolParams()
{
}
void CClustalwToolParams::operator=(const CClustalwToolParams& data)
{
    Copy(data);
}
bool CClustalwToolParams::operator==(const CClustalwToolParams& data) const
{
////@begin CClustalwToolParams equality operator
     if (!(m_ClustalWPath == data.m_ClustalWPath)) return false;
     if (!(m_CommandLine == data.m_CommandLine)) return false;
     if (!(m_GenerateTree == data.m_GenerateTree)) return false;
////@end CClustalwToolParams equality operator
    return true;
}
void CClustalwToolParams::Copy(const CClustalwToolParams& data)
{
////@begin CClustalwToolParams copy function
    m_ClustalWPath = data.m_ClustalWPath;
    m_CommandLine = data.m_CommandLine;
    m_GenerateTree = data.m_GenerateTree;
////@end CClustalwToolParams copy function
    m_Objects = data.m_Objects;
}
void CClustalwToolParams::Init()
{
////@begin CClustalwToolParams member initialisation
    m_GenerateTree = false;
////@end CClustalwToolParams member initialisation
}

wxString CClustalwToolParams::GetClustalWExecutable() const
{
    return FindExeFile(m_ClustalWPath);
}

static const char
*kGenerateTree = "kGenerateTree",
*kCommandLine = "CommandLine",
*kClustalwPath = "GBENCH.Plugins.Algo.Clustalw.ClustalwPath";

void CClustalwToolParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kGenerateTree, m_GenerateTree);
        view.Set(kCommandLine, ToStdString(m_CommandLine));
        gui_reg.Set(kClustalwPath, FnToStdString(m_ClustalWPath));
    }
}

void CClustalwToolParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_GenerateTree = view.GetBool(kGenerateTree, m_GenerateTree);
        m_CommandLine = ToWxString(view.GetString(kCommandLine, ToStdString(m_CommandLine)));
        m_ClustalWPath = FnToWxString(gui_reg.GetString(kClustalwPath, ToStdString(m_ClustalWPath)));
    }
}

END_NCBI_SCOPE
