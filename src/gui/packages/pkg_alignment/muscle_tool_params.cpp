/*  $Id: muscle_tool_params.cpp 44360 2019-12-04 21:44:01Z katargir $
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

#include <gui/packages/pkg_alignment/muscle_tool_params.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

CMuscleToolParams::CMuscleToolParams()
{
    Init();
}

CMuscleToolParams::CMuscleToolParams(const CMuscleToolParams& data)
{
    Init();
    Copy(data);
}

CMuscleToolParams::~CMuscleToolParams()
{
}

void CMuscleToolParams::operator=(const CMuscleToolParams& data)
{
    Copy(data);
}

bool CMuscleToolParams::operator==(const CMuscleToolParams& data) const
{
////@begin CMuscleToolParams equality operator
     if (!(m_CommandLine == data.m_CommandLine)) return false;
     if (!(m_GenerateTree == data.m_GenerateTree)) return false;
     if (!(m_MusclePath == data.m_MusclePath)) return false;
     if (!(m_ScoringMethod == data.m_ScoringMethod)) return false;
////@end CMuscleToolParams equality operator
    return true;
}

void CMuscleToolParams::Copy(const CMuscleToolParams& data)
{
////@begin CMuscleToolParams copy function
    m_CommandLine = data.m_CommandLine;
    m_GenerateTree = data.m_GenerateTree;
    m_MusclePath = data.m_MusclePath;
    m_ScoringMethod = data.m_ScoringMethod;
////@end CMuscleToolParams copy function
    m_Objects = data.m_Objects;
}

void CMuscleToolParams::Init()
{
////@begin CMuscleToolParams member initialisation
    m_GenerateTree = false;
    m_ScoringMethod = 0;
////@end CMuscleToolParams member initialisation
}

wxString CMuscleToolParams::GetMuscleExecutable() const
{
    return FindExeFile(m_MusclePath);
}

static const char
*kScoringMethod = "ScoringMethod",
*kGenerateTree = "kGenerateTree",
*kCommandLine = "CommandLine",
*kMusclePath = "GBENCH.Plugins.Algo.Muscle.MusclePath";

void CMuscleToolParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kScoringMethod, m_ScoringMethod);
        view.Set(kGenerateTree, m_GenerateTree);
        view.Set(kCommandLine, ToStdString(m_CommandLine));
        gui_reg.Set(kMusclePath, m_MusclePath.ToUTF8());
    }
}

void CMuscleToolParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_ScoringMethod = view.GetInt(kScoringMethod, m_ScoringMethod);
        m_GenerateTree = view.GetBool(kGenerateTree, m_GenerateTree);
        m_CommandLine = ToWxString(view.GetString(kCommandLine, ToStdString(m_CommandLine)));
        m_MusclePath = wxString::FromUTF8(gui_reg.GetString(kMusclePath, string(m_MusclePath.ToUTF8())).c_str());
    }
}

END_NCBI_SCOPE
