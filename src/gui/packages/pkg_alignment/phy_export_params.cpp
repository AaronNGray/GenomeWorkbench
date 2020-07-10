/*  $Id: phy_export_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  Vladimir Tereshkov
 */


#include <ncbi_pch.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/packages/pkg_alignment/phy_export_params.hpp>

#include <wx/string.h>

BEGIN_NCBI_SCOPE

/*!
 * CPhyExportParams type definition
 */

/*!
 * Default constructor for CPhyExportParams
 */

CPhyExportParams::CPhyExportParams()
{
    Init();
}

/*!
 * Copy constructor for CPhyExportParams
 */

CPhyExportParams::CPhyExportParams(const CPhyExportParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for CPhyExportParams
 */

CPhyExportParams::~CPhyExportParams()
{
}

/*!
 * Assignment operator for CPhyExportParams
 */

void CPhyExportParams::operator=(const CPhyExportParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for CPhyExportParams
 */

bool CPhyExportParams::operator==(const CPhyExportParams& data) const
{
////@begin CPhyExportParams equality operator
     if (!(m_FileName == data.m_FileName)) return false;
     if (!(m_FileFormat == data.m_FileFormat)) return false;
//     if (!(m_pBioTree == data.m_pBioTree)) return false; // that's not good
////@end CPhyExportParams equality operator
    return true;
}

/*!
 * Copy function for CPhyExportParams
 */

void CPhyExportParams::Copy(const CPhyExportParams& data)
{
////@begin CPhyExportParams copy function
    m_FileName = data.m_FileName;
    m_FileFormat = data.m_FileFormat;
    m_LabelFormat = data.m_LabelFormat;
    m_pBioTree = data.m_pBioTree;
////@end CPhyExportParams copy function
}

/*!
 * Member initialisation for CPhyExportParams
 */

void CPhyExportParams::Init()
{
////@begin CPhyExportParams member initialisation
////@end CPhyExportParams member initialisation
    m_FileName      = wxT("");
    m_FileFormat    = wxT("Newick");
    m_LabelFormat   = wxT("");
    m_pBioTree      = NULL;
}

static const char* kFileName    = "ParseIDs";
static const char* kFileFormat  = "SetGapInfo";

/// IRegSettings
void CPhyExportParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kFileName,     FnToStdString( m_FileName ));
        view.Set(kFileFormat,   ToStdString( m_FileFormat ));
    }
}

void CPhyExportParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_FileName   =
            FnToWxString(view.GetString(kFileName, FnToStdString(m_FileName)));
        m_FileFormat =
            ToWxString(view.GetString(kFileFormat, ToStdString(m_FileFormat)));
    }
}

END_NCBI_SCOPE
