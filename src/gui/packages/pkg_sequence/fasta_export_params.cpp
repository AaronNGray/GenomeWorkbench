/*  $Id: fasta_export_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/packages/pkg_sequence/fasta_export_params.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CFastaExportParams type definition
 */

/*!
 * Default constructor for CFastaExportParams
 */

CFastaExportParams::CFastaExportParams()
{
    Init();
}

/*!
 * Copy constructor for CFastaExportParams
 */

CFastaExportParams::CFastaExportParams(const CFastaExportParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for CFastaExportParams
 */

CFastaExportParams::~CFastaExportParams()
{
}

/*!
 * Assignment operator for CFastaExportParams
 */

void CFastaExportParams::operator=(const CFastaExportParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for CFastaExportParams
 */

bool CFastaExportParams::operator==(const CFastaExportParams& data) const
{
////@begin CFastaExportParams equality operator
     if (!(m_FileName == data.m_FileName)) return false;
     if (!(m_SaveMasterRecord == data.m_SaveMasterRecord)) return false;
     if (!(m_ExportType == data.m_ExportType)) return false;
////@end CFastaExportParams equality operator
    return true;
}

/*!
 * Copy function for CFastaExportParams
 */

void CFastaExportParams::Copy(const CFastaExportParams& data)
{
////@begin CFastaExportParams copy function
    m_FileName = data.m_FileName;
    m_SaveMasterRecord = data.m_SaveMasterRecord;
    m_ExportType = data.m_ExportType;
////@end CFastaExportParams copy function
    m_Objects = data.m_Objects;
}

/*!
 * Member initialisation for CFastaExportParams
 */

void CFastaExportParams::Init()
{
////@begin CFastaExportParams member initialisation
    m_SaveMasterRecord = false;
    m_ExportType = 0;
////@end CFastaExportParams member initialisation
}


static const char* kSaveMasterRecord = "SaveMasterRecord";
static const char* kFileName = "FileName";

/// IRegSettings
void CFastaExportParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kSaveMasterRecord, m_SaveMasterRecord);
        view.Set(kFileName, FnToStdString( m_FileName ));
    }
}

void CFastaExportParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_SaveMasterRecord = view.GetBool(kSaveMasterRecord, m_SaveMasterRecord);
        m_FileName =
            FnToWxString(view.GetString(kFileName, FnToStdString(m_FileName)));
    }
}

END_NCBI_SCOPE
