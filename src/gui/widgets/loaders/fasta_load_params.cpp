/*  $Id: fasta_load_params.cpp 44796 2020-03-17 22:37:42Z evgeniev $
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

#include <gui/widgets/loaders/fasta_load_params.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CFastaLoadParams type definition
 */

/*!
 * Default constructor for CFastaLoadParams
 */

CFastaLoadParams::CFastaLoadParams()
{
    Init();
}

/*!
 * Copy constructor for CFastaLoadParams
 */

CFastaLoadParams::CFastaLoadParams(const CFastaLoadParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for CFastaLoadParams
 */

CFastaLoadParams::~CFastaLoadParams()
{
}

/*!
 * Assignment operator for CFastaLoadParams
 */

void CFastaLoadParams::operator=(const CFastaLoadParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for CFastaLoadParams
 */

bool CFastaLoadParams::operator==(const CFastaLoadParams& data) const
{
////@begin CFastaLoadParams equality operator
     if (!(m_ForceLocalIDs == data.m_ForceLocalIDs)) return false;
     if (!(m_IgnoreGaps == data.m_IgnoreGaps)) return false;
     if (!(m_LowercaseOption == data.m_LowercaseOption)) return false;
     if (!(m_MakeDelta == data.m_MakeDelta)) return false;
     if (!(m_NoSplit == data.m_NoSplit)) return false;
     if (!(m_ReadFirst == data.m_ReadFirst)) return false;
     if (!(m_SeqType == data.m_SeqType)) return false;
     if (!(m_SkipInvalid == data.m_SkipInvalid)) return false;
////@end CFastaLoadParams equality operator
    return true;
}

/*!
 * Copy function for CFastaLoadParams
 */

void CFastaLoadParams::Copy(const CFastaLoadParams& data)
{
////@begin CFastaLoadParams copy function
    m_ForceLocalIDs = data.m_ForceLocalIDs;
    m_IgnoreGaps = data.m_IgnoreGaps;
    m_LowercaseOption = data.m_LowercaseOption;
    m_MakeDelta = data.m_MakeDelta;
    m_NoSplit = data.m_NoSplit;
    m_ReadFirst = data.m_ReadFirst;
    m_SeqType = data.m_SeqType;
    m_SkipInvalid = data.m_SkipInvalid;
////@end CFastaLoadParams copy function
}

/*!
 * Member initialisation for CFastaLoadParams
 */

void CFastaLoadParams::Init()
{
////@begin CFastaLoadParams member initialisation
    m_ForceLocalIDs = false;
    m_IgnoreGaps = false;
    m_LowercaseOption = 0;
    m_MakeDelta = false;
    m_NoSplit = false;
    m_ReadFirst = false;
    m_SeqType = 0;
    m_SkipInvalid = true;
////@end CFastaLoadParams member initialisation
}

static const char
*kSeqTypeTag         = "SeqType",
*kLowercaseOptionTag = "LowercaseOption",
*kForceLocalIDsTag   = "ForceLocalIDs",
*kMakeDeltaTag       = "MakeDelta",
*kIgnoreGaps         = "IgnoreGaps",
*kReadFirstTag       = "ReadFirst",
*kParseAllIDsTag     = "ParseAllIDs",
*kSkipInvalidTag     = "SkipInvalid",
*kNoSplit            = "NoSplit";

/// IRegSettings
void CFastaLoadParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kSeqTypeTag, m_SeqType);
        view.Set(kLowercaseOptionTag, m_LowercaseOption);

        view.Set(kForceLocalIDsTag, m_ForceLocalIDs);
        view.Set(kMakeDeltaTag, m_MakeDelta);
        view.Set(kIgnoreGaps, m_IgnoreGaps);
        view.Set(kReadFirstTag, m_ReadFirst);
        view.Set(kSkipInvalidTag, m_SkipInvalid);
        view.Set(kNoSplit, m_NoSplit);
    }
}

void CFastaLoadParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_SeqType = view.GetInt(kSeqTypeTag, m_SeqType);
        m_LowercaseOption = view.GetInt(kLowercaseOptionTag, m_LowercaseOption);

        m_ForceLocalIDs = view.GetBool(kForceLocalIDsTag, m_ForceLocalIDs);
        m_MakeDelta = view.GetBool(kMakeDeltaTag, m_MakeDelta);
        m_IgnoreGaps = view.GetBool(kIgnoreGaps, m_IgnoreGaps);
        m_ReadFirst = view.GetBool(kReadFirstTag, m_ReadFirst);
        m_SkipInvalid = view.GetBool(kSkipInvalidTag, m_SkipInvalid);
        m_NoSplit = view.GetBool(kNoSplit, m_NoSplit);
    }
}

END_NCBI_SCOPE
