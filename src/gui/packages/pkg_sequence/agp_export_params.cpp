/*  $Id: agp_export_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/packages/pkg_sequence/agp_export_params.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CAgpExportParams type definition
 */

/*!
 * Default constructor for CAgpExportParams
 */

CAgpExportParams::CAgpExportParams()
{
    Init();
}

/*!
 * Copy constructor for CAgpExportParams
 */

CAgpExportParams::CAgpExportParams(const CAgpExportParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for CAgpExportParams
 */

CAgpExportParams::~CAgpExportParams()
{
}

/*!
 * Assignment operator for CAgpExportParams
 */

void CAgpExportParams::operator=(const CAgpExportParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for CAgpExportParams
 */

bool CAgpExportParams::operator==(const CAgpExportParams& data) const
{
////@begin CAgpExportParams equality operator
     if (!(m_GapType == data.m_GapType)) return false;
     if (!(m_AltObjId == data.m_AltObjId)) return false;
     if (!(m_AssertEvidence == data.m_AssertEvidence)) return false;
     if (!(m_FileName == data.m_FileName)) return false;
////@end CAgpExportParams equality operator
    return true;
}

/*!
 * Copy function for CAgpExportParams
 */

void CAgpExportParams::Copy(const CAgpExportParams& data)
{
////@begin CAgpExportParams copy function
    m_GapType = data.m_GapType;
    m_AltObjId = data.m_AltObjId;
    m_AssertEvidence = data.m_AssertEvidence;
    m_FileName = data.m_FileName;
////@end CAgpExportParams copy function
    m_SeqLoc = data.m_SeqLoc;
}

/*!
 * Member initialisation for CAgpExportParams
 */

void CAgpExportParams::Init()
{
////@begin CAgpExportParams member initialisation
    m_AssertEvidence = false;
////@end CAgpExportParams member initialisation
}

static const char* kGapType = "GapType";
static const char* kAltObjId = "AltObjId";
static const char* kAssertEvidence = "AssertEvidence";
static const char* kFileName = "FileName";

/// IRegSettings
void CAgpExportParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kGapType, m_GapType);
        view.Set(kAltObjId, ToStdString( m_AltObjId ));
        view.Set(kAssertEvidence, m_AssertEvidence);
        view.Set(kFileName, FnToStdString( m_FileName ));
    }
}

void CAgpExportParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_GapType = view.GetString(kGapType, m_GapType);
        m_AltObjId = ToWxString(view.GetString(kAltObjId,
                                               ToStdString(m_AltObjId)));
        m_AssertEvidence = view.GetBool(kAssertEvidence, m_AssertEvidence);
        m_FileName = FnToWxString(view.GetString(kFileName,
                                               FnToStdString(m_FileName)));
    }
}

END_NCBI_SCOPE
