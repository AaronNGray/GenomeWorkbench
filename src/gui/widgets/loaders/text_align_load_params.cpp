/*  $Id: text_align_load_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/loaders/text_align_load_params.hpp>

BEGIN_NCBI_SCOPE

CTextAlignParams::CTextAlignParams()
{
    Init();
}
CTextAlignParams::CTextAlignParams(const CTextAlignParams& data)
{
    Init();
    Copy(data);
}
CTextAlignParams::~CTextAlignParams()
{
}
void CTextAlignParams::operator=(const CTextAlignParams& data)
{
    Copy(data);
}
bool CTextAlignParams::operator==(const CTextAlignParams& data) const
{
////@begin CTextAlignParams equality operator
     if (!(m_Begin == data.m_Begin)) return false;
     if (!(m_End == data.m_End)) return false;
     if (!(m_Match == data.m_Match)) return false;
     if (!(m_Middle == data.m_Middle)) return false;
     if (!(m_SeqType == data.m_SeqType)) return false;
     if (!(m_Unknown == data.m_Unknown)) return false;
////@end CTextAlignParams equality operator
    return true;
}
void CTextAlignParams::Copy(const CTextAlignParams& data)
{
////@begin CTextAlignParams copy function
    m_Begin = data.m_Begin;
    m_End = data.m_End;
    m_Match = data.m_Match;
    m_Middle = data.m_Middle;
    m_SeqType = data.m_SeqType;
    m_Unknown = data.m_Unknown;
////@end CTextAlignParams copy function
}
void CTextAlignParams::Init()
{
////@begin CTextAlignParams member initialisation
    m_Begin = wxT("-.?");
    m_End = wxT("-.?");
    m_Match = wxT(".");
    m_Middle = wxT("-");
    m_SeqType = 0;
    m_Unknown = wxT("?");
////@end CTextAlignParams member initialisation
}

static const char* kUnknown = "Unknown";
static const char* kMatch = "Match";
static const char* kBegin = "Begin";
static const char* kMiddle = "Middle";
static const char* kEnd = "End";
static const char* kSeqType = "SeqType";

/// IRegSettings
void CTextAlignParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kUnknown, ToStdString(m_Unknown));
        view.Set(kMatch, ToStdString(m_Match));
        view.Set(kBegin, ToStdString(m_Begin));
        view.Set(kMiddle, ToStdString(m_Middle));
        view.Set(kEnd, ToStdString(m_End));
        view.Set(kSeqType, m_SeqType);
    }
}

void CTextAlignParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_Unknown = ToWxString(view.GetString(kUnknown, ToStdString(m_Unknown)));
        m_Match = ToWxString(view.GetString(kMatch, ToStdString(m_Match)));
        m_Begin = ToWxString(view.GetString(kBegin, ToStdString(m_Begin)));
        m_Middle = ToWxString(view.GetString(kMiddle, ToStdString(m_Middle)));
        m_End = ToWxString(view.GetString(kEnd, ToStdString(m_End)));
        m_SeqType = view.GetInt(kSeqType, m_SeqType);
    }
}

END_NCBI_SCOPE
