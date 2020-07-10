/*  $Id: mafft_tool_params.cpp 44360 2019-12-04 21:44:01Z katargir $
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
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/packages/pkg_alignment/mafft_tool_params.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

CMafftToolParams::CMafftToolParams()
{
    Init();
}

CMafftToolParams::CMafftToolParams(const CMafftToolParams& data)
{
    Init();
    Copy(data);
}

CMafftToolParams::~CMafftToolParams()
{
}

void CMafftToolParams::operator=(const CMafftToolParams& data)
{
    Copy(data);
}

bool CMafftToolParams::operator==(const CMafftToolParams& data) const
{
////@begin CMafftToolParams equality operator
     if (!(m_AutoFlag == data.m_AutoFlag)) return false;
     if (!(m_CommandLine == data.m_CommandLine)) return false;
     if (!(m_MafftPath == data.m_MafftPath)) return false;
     if (!(m_QuietFlag == data.m_QuietFlag)) return false;
////@end CMafftToolParams equality operator
    return true;
}

void CMafftToolParams::Copy(const CMafftToolParams& data)
{
////@begin CMafftToolParams copy function
    m_AutoFlag = data.m_AutoFlag;
    m_CommandLine = data.m_CommandLine;
    m_MafftPath = data.m_MafftPath;
    m_QuietFlag = data.m_QuietFlag;
////@end CMafftToolParams copy function
    m_Objects = data.m_Objects;
}

void CMafftToolParams::Init()
{
////@begin CMafftToolParams member initialisation
    m_AutoFlag = false;
    m_QuietFlag = true;
////@end CMafftToolParams member initialisation
}

wxString CMafftToolParams::GetMafftExecutable() const
{
    return FindExeFile(m_MafftPath);
}

static const char
*kAutoFlag = "AutoFlag",
*kQuietMode = "QuietMode",
*kCommandLine = "CommandLine",
*kMafftPath = "GBENCH.Plugins.Algo.MAFFT.MafftPath";

void CMafftToolParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kCommandLine, ToStdString(m_CommandLine));
        view.Set(kQuietMode, m_QuietFlag);
        view.Set(kAutoFlag, m_AutoFlag);
        gui_reg.Set(kMafftPath, m_MafftPath.ToUTF8());
    }
}

void CMafftToolParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_CommandLine = ToWxString(view.GetString(kCommandLine, ToStdString(m_CommandLine)));
        m_QuietFlag = view.GetBool(kQuietMode, true);
        m_AutoFlag = view.GetBool(kAutoFlag, false);
        m_MafftPath = wxString::FromUTF8(gui_reg.GetString(kMafftPath, string(m_MafftPath.ToUTF8())).c_str());
    }
}

END_NCBI_SCOPE
