/*  $Id: agp_load_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/loaders/agp_load_params.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CAgpLoadParams type definition
 */

/*!
 * Default constructor for CAgpLoadParams
 */

CAgpLoadParams::CAgpLoadParams()
{
    Init();
}

/*!
 * Copy constructor for CAgpLoadParams
 */

CAgpLoadParams::CAgpLoadParams(const CAgpLoadParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for CAgpLoadParams
 */

CAgpLoadParams::~CAgpLoadParams()
{
}

/*!
 * Assignment operator for CAgpLoadParams
 */

void CAgpLoadParams::operator=(const CAgpLoadParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for CAgpLoadParams
 */

bool CAgpLoadParams::operator==(const CAgpLoadParams& data) const
{
////@begin CAgpLoadParams equality operator
     if (!(m_FastaFile == data.m_FastaFile)) return false;
     if (!(m_ParseIDs == data.m_ParseIDs)) return false;
     if (!(m_SetGapInfo == data.m_SetGapInfo)) return false;
////@end CAgpLoadParams equality operator
    return true;
}

/*!
 * Copy function for CAgpLoadParams
 */

void CAgpLoadParams::Copy(const CAgpLoadParams& data)
{
////@begin CAgpLoadParams copy function
    m_FastaFile = data.m_FastaFile;
    m_ParseIDs = data.m_ParseIDs;
    m_SetGapInfo = data.m_SetGapInfo;
////@end CAgpLoadParams copy function
}

/*!
 * Member initialisation for CAgpLoadParams
 */

void CAgpLoadParams::Init()
{
////@begin CAgpLoadParams member initialisation
    m_ParseIDs = 0;
    m_SetGapInfo = false;
////@end CAgpLoadParams member initialisation
}

static const char* kParseIDs = "ParseIDs";
static const char* kSetGapInfo = "SetGapInfo";
static const char* kFastaFile = "FastaFile";

/// IRegSettings
void CAgpLoadParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kParseIDs, m_ParseIDs);
        view.Set(kSetGapInfo, m_SetGapInfo);
        view.Set(kFastaFile, FnToStdString( m_FastaFile ));
    }
}

void CAgpLoadParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_ParseIDs = view.GetInt(kParseIDs, m_ParseIDs);
        m_SetGapInfo = view.GetBool(kSetGapInfo, m_SetGapInfo);
        m_FastaFile = FnToWxString(view.GetString(kFastaFile,
                                                  FnToStdString(m_FastaFile)));
    }
}

END_NCBI_SCOPE
