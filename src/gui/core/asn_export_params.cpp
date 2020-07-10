/*  $Id: asn_export_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/core/asn_export_params.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE

CAsnExportParams::CAsnExportParams()
{
    Init();
}
CAsnExportParams::CAsnExportParams(const CAsnExportParams& data)
{
    Init();
    Copy(data);
}
CAsnExportParams::~CAsnExportParams()
{
}
void CAsnExportParams::operator=(const CAsnExportParams& data)
{
    Copy(data);
}
bool CAsnExportParams::operator==(const CAsnExportParams& data) const
{
////@begin CAsnExportParams equality operator
     if (!(m_AsnType == data.m_AsnType)) return false;
     if (!(m_FileName == data.m_FileName)) return false;
////@end CAsnExportParams equality operator
    return true;
}
void CAsnExportParams::Copy(const CAsnExportParams& data)
{
////@begin CAsnExportParams copy function
    m_AsnType = data.m_AsnType;
    m_FileName = data.m_FileName;
////@end CAsnExportParams copy function

    m_Objects = data.m_Objects;
}
void CAsnExportParams::Init()
{
////@begin CAsnExportParams member initialisation
    m_AsnType = 0;
////@end CAsnExportParams member initialisation
}

static const char* kAsnType = "AsnType";
static const char* kFileNameUTF8 = "FileNameUTF8";

/// IRegSettings
void CAsnExportParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kAsnType, m_AsnType);
        view.Set(kFileNameUTF8, m_FileName.ToUTF8());
    }
}

void CAsnExportParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_AsnType = view.GetInt(kAsnType, m_AsnType);
        m_FileName = wxString::FromUTF8(view.GetString(kFileNameUTF8, string(m_FileName.ToUTF8())).c_str());
    }
}

void CAsnExportParams::UpdateFileName(const wxString& fname)
{
    if (fname.empty())
        return;

    wxFileName fileName(fname);
    if (!m_FileName.empty()) {
        wxFileName origFileName(m_FileName);
        origFileName.SetFullName(fileName.GetFullName());
        m_FileName = origFileName.GetFullPath();
    }
    else {
        m_FileName = fileName.GetFullPath();
    }
}

END_NCBI_SCOPE
