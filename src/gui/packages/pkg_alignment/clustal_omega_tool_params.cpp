/*  $Id: clustal_omega_tool_params.cpp 43796 2019-08-30 19:31:56Z katargir $
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
 * Authors: Vladislav Evgeniev
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/packages/pkg_alignment/clustal_omega_tool_params.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE


////@begin includes
////@end includes

CClustalOmegaToolParams::CClustalOmegaToolParams()
{
    Init();
}

CClustalOmegaToolParams::CClustalOmegaToolParams(const CClustalOmegaToolParams& data)
{
    Init();
    Copy(data);
}

CClustalOmegaToolParams::~CClustalOmegaToolParams()
{
}

void CClustalOmegaToolParams::operator=(const CClustalOmegaToolParams& data)
{
    Copy(data);
}

bool CClustalOmegaToolParams::operator==(const CClustalOmegaToolParams& data) const
{
////@begin CClustalOmegaToolParams equality operator
     if (!(m_ClustalOmegaPath == data.m_ClustalOmegaPath)) return false;
     if (!(m_CommandLine == data.m_CommandLine)) return false;
     if (!(m_GenerateTree == data.m_GenerateTree)) return false;
////@end CClustalOmegaToolParams equality operator
    return true;
}

void CClustalOmegaToolParams::Copy(const CClustalOmegaToolParams& data)
{
////@begin CClustalOmegaToolParams copy function
    m_ClustalOmegaPath = data.m_ClustalOmegaPath;
    m_CommandLine = data.m_CommandLine;
    m_GenerateTree = data.m_GenerateTree;
////@end CClustalOmegaToolParams copy function
    m_Objects = data.m_Objects;
}

void CClustalOmegaToolParams::Init()
{
////@begin CClustalOmegaToolParams member initialisation
    m_GenerateTree = false;
////@end CClustalOmegaToolParams member initialisation
}

wxString CClustalOmegaToolParams::GetClustalOmegaExecutable() const
{
    return FindExeFile(m_ClustalOmegaPath);
}

static const char
*kGenerateTree = "kGenerateTree",
*kCommandLine = "CommandLine",
*kClustalwPath = "GBENCH.Plugins.Algo.ClustalOmega.ClustalOmegaPath";

void CClustalOmegaToolParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kGenerateTree, m_GenerateTree);
        view.Set(kCommandLine, ToStdString(m_CommandLine));
        gui_reg.Set(kClustalwPath, FnToStdString(m_ClustalOmegaPath));
    }
}

void CClustalOmegaToolParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_GenerateTree = view.GetBool(kGenerateTree, m_GenerateTree);
        m_CommandLine = ToWxString(view.GetString(kCommandLine, ToStdString(m_CommandLine)));
        m_ClustalOmegaPath = FnToWxString(gui_reg.GetString(kClustalwPath, ToStdString(m_ClustalOmegaPath)));
    }
}

END_NCBI_SCOPE
