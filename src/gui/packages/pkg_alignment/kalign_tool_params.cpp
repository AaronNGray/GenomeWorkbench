/*  $Id: kalign_tool_params.cpp 44360 2019-12-04 21:44:01Z katargir $
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

#include <gui/packages/pkg_alignment/kalign_tool_params.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

CKalignToolParams::CKalignToolParams()
{
    Init();
}

CKalignToolParams::CKalignToolParams(const CKalignToolParams& data)
{
    Init();
    Copy(data);
}

CKalignToolParams::~CKalignToolParams()
{
}

void CKalignToolParams::operator=(const CKalignToolParams& data)
{
    Copy(data);
}

bool CKalignToolParams::operator==(const CKalignToolParams& data) const
{
////@begin CKalignToolParams equality operator
     if (!(m_CommandLine == data.m_CommandLine)) return false;
     if (!(m_KalignPath == data.m_KalignPath)) return false;
     if (!(m_QuietFlag == data.m_QuietFlag)) return false;
////@end CKalignToolParams equality operator
    return true;
}

void CKalignToolParams::Copy(const CKalignToolParams& data)
{
////@begin CKalignToolParams copy function
    m_CommandLine = data.m_CommandLine;
    m_KalignPath = data.m_KalignPath;
    m_QuietFlag = data.m_QuietFlag;
////@end CKalignToolParams copy function
    m_Objects = data.m_Objects;
}

void CKalignToolParams::Init()
{
////@begin CKalignToolParams member initialisation
    m_QuietFlag = true;
////@end CKalignToolParams member initialisation
}

wxString CKalignToolParams::GetKalignExecutable() const
{
    return FindExeFile(m_KalignPath);
}

static const char
*kQuietMode = "QuietMode",
*kCommandLine = "CommandLine",
*kAlignPath = "GBENCH.Plugins.Algo.Kalign.KalignPath";

void CKalignToolParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kCommandLine, ToStdString(m_CommandLine));
        view.Set(kQuietMode, m_QuietFlag);
        gui_reg.Set(kAlignPath, m_KalignPath.ToUTF8());
    }
}

void CKalignToolParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_CommandLine = ToWxString(view.GetString(kCommandLine, ToStdString(m_CommandLine)));
        m_QuietFlag = view.GetBool(kQuietMode, true);
        m_KalignPath = wxString::FromUTF8(gui_reg.GetString(kAlignPath, string(m_KalignPath.ToUTF8())).c_str());
    }
}

END_NCBI_SCOPE
