/*  $Id: flatfile_export_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/packages/pkg_sequence/flatfile_export_params.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE


/*!
 * CFlatFileExportParams type definition
 */

/*!
 * Default constructor for CFlatFileExportParams
 */

CFlatFileExportParams::CFlatFileExportParams()
{
    Init();
}

/*!
 * Copy constructor for CFlatFileExportParams
 */

CFlatFileExportParams::CFlatFileExportParams(const CFlatFileExportParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for CFlatFileExportParams
 */

CFlatFileExportParams::~CFlatFileExportParams()
{
}

/*!
 * Assignment operator for CFlatFileExportParams
 */

void CFlatFileExportParams::operator=(const CFlatFileExportParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for CFlatFileExportParams
 */

bool CFlatFileExportParams::operator==(const CFlatFileExportParams& data) const
{
////@begin CFlatFileExportParams equality operator
     if (!(m_FileName == data.m_FileName)) return false;
     if (!(m_Mode == data.m_Mode)) return false;
////@end CFlatFileExportParams equality operator
    return true;
}

/*!
 * Copy function for CFlatFileExportParams
 */

void CFlatFileExportParams::Copy(const CFlatFileExportParams& data)
{
////@begin CFlatFileExportParams copy function
    m_FileName = data.m_FileName;
    m_Mode = data.m_Mode;
////@end CFlatFileExportParams copy function
    m_SeqLoc = data.m_SeqLoc;
}

/*!
 * Member initialisation for CFlatFileExportParams
 */

void CFlatFileExportParams::Init()
{
////@begin CFlatFileExportParams member initialisation
    m_Mode = 0;
////@end CFlatFileExportParams member initialisation
}

static const char* kMode = "Mode";
static const char* kFileName = "FileName";

/// IRegSettings
void CFlatFileExportParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kMode, m_Mode);
        view.Set(kFileName, FnToStdString( m_FileName ));
    }
}

void CFlatFileExportParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_Mode = view.GetInt(kMode, m_Mode);
        m_FileName = FnToWxString(view.GetString(kFileName,
                                               FnToStdString(m_FileName)));
    }
}

END_NCBI_SCOPE
