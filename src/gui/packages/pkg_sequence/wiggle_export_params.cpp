/*  $Id: wiggle_export_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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
* Authors:  Vladislav Evgeniev
*
* File Description:
*
*/

#include <ncbi_pch.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/packages/pkg_sequence/wiggle_export_params.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE

CWiggleExportParams::CWiggleExportParams()
{
    Init();
}

/*!
 * Copy constructor for CWiggleExportParams
 */

CWiggleExportParams::CWiggleExportParams(const CWiggleExportParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for CWiggleExportParams
 */

CWiggleExportParams::~CWiggleExportParams()
{
}

/*!
 * Assignment operator for CWiggleExportParams
 */

void CWiggleExportParams::operator=(const CWiggleExportParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for CWiggleExportParams
 */

bool CWiggleExportParams::operator==(const CWiggleExportParams& data) const
{
////@begin CWiggleExportParams equality operator
     if (!(m_FileName == data.m_FileName)) return false;
     if (!(m_From == data.m_From)) return false;
     if (!(m_To == data.m_To)) return false;
     if (!(m_BinSize == data.m_BinSize)) return false;
////@end CWiggleExportParams equality operator
    return true;
}

/*!
 * Copy function for CWiggleExportParams
 */

void CWiggleExportParams::Copy(const CWiggleExportParams& data)
{
////@begin CWiggleExportParams copy function
    m_FileName = data.m_FileName;
    m_From = data.m_From;
    m_To = data.m_To;
    m_BinSize = data.m_BinSize;
////@end CWiggleExportParams copy function
    m_Objects = data.m_Objects;
}

/*!
 * Member initialisation for CWiggleExportParams
 */

void CWiggleExportParams::Init()
{
////@begin CWiggleExportParams member initialisation
////@end CWiggleExportParams member initialisation
}

static const char* kFileNameUTF8 = "FileNameUTF8";
static const char* kWiggleStep = "Step";

/// IRegSettings
void CWiggleExportParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kFileNameUTF8, m_FileName.ToUTF8());
        view.Set(kWiggleStep, m_BinSize);
    }
}

void CWiggleExportParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_FileName = wxString::FromUTF8(view.GetString(kFileNameUTF8, string(m_FileName.ToUTF8())).c_str());
        m_BinSize = view.GetInt(kWiggleStep, 1000);
    }
}

void CWiggleExportParams::UpdateFileName(const wxString& fname)
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
