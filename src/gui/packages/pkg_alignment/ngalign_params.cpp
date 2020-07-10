/*  $Id: ngalign_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/packages/pkg_alignment/ngalign_params.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <objects/seqloc/Seq_id.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CNGAlignParams::CNGAlignParams()
{
    Init();
}

CNGAlignParams::CNGAlignParams(const CNGAlignParams& data)
{
    Init();
    Copy(data);
}

CNGAlignParams::~CNGAlignParams()
{
}

void CNGAlignParams::operator=(const CNGAlignParams& data)
{
    Copy(data);
}

bool CNGAlignParams::operator==(const CNGAlignParams& data) const
{
    ////@begin CNGAlignParams equality operator
     if (!(m_FilterMode == data.m_FilterMode)) return false;
     if (!(m_WordSize == data.m_WordSize)) return false;
     if (!(m_eValue == data.m_eValue)) return false;
     if (!(m_BestHit == data.m_BestHit)) return false;
     if (!(m_JobTitle == data.m_JobTitle)) return false;
     if (!(m_AdvParams == data.m_AdvParams)) return false;
     if (!(m_WM_TaxId == data.m_WM_TaxId)) return false;
     if (!(m_WM_Dir == data.m_WM_Dir)) return false;
    ////@end CNGAlignParams equality operator
    return true;
}

void CNGAlignParams::Copy(const CNGAlignParams& data)
{
    ////@begin CNGAlignParams copy function
    m_FilterMode = data.m_FilterMode;
    m_WordSize = data.m_WordSize;
    m_eValue = data.m_eValue;
    m_BestHit = data.m_BestHit;
    m_JobTitle = data.m_JobTitle;
    m_AdvParams = data.m_AdvParams;
    m_WM_TaxId = data.m_WM_TaxId;
    m_WM_Dir = data.m_WM_Dir;
    ////@end CNGAlignParams copy function
    m_Subject = data.m_Subject;
    m_Queries = data.m_Queries;
}

void CNGAlignParams::Init()
{
    ////@begin CNGAlignParams member initialisation
    m_FilterMode = 0;
    m_WordSize = 100;
    m_eValue = 0.0001;
    m_BestHit = true;
    m_WM_TaxId = 0;
    ////@end CNGAlignParams member initialisation
}

static const char
*kFilterMode = "FilterMode",
*kWordSize = "WordSize",
*keValue = "eValue",
*kJobTitle = "JobTitle",
*kAdvParams = "AdvParams",
*kBestHit = "BestHit",
*kWM_TaxId = "WM_TaxId";

void CNGAlignParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kFilterMode, m_FilterMode);
        view.Set(kWordSize, (int)m_WordSize);
        view.Set(keValue, m_eValue);
        view.Set(kJobTitle, ToStdString(m_JobTitle));
        view.Set(kAdvParams, ToStdString(m_AdvParams));
        view.Set(kBestHit, m_BestHit);
        view.Set(kWM_TaxId, m_WM_TaxId);
    }
}

void CNGAlignParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_FilterMode = view.GetInt(kFilterMode, m_FilterMode);
        m_WordSize = view.GetInt(kWordSize, (int)m_WordSize);
        m_eValue = view.GetReal(keValue, m_eValue);
        m_JobTitle = ToWxString(view.GetString(kJobTitle, ToStdString(m_JobTitle)));
        m_AdvParams = ToWxString(view.GetString(kAdvParams, ToStdString(m_AdvParams)));
        m_BestHit = view.GetBool(kBestHit, m_BestHit);
        m_WM_TaxId = view.GetInt(kWM_TaxId, m_WM_TaxId);
    }
}

void CNGAlignParams::LogDump()
{
    LOG_POST(Info << "CNGAlignParams");

    string subject_seq, query_seqs;
    const CSeq_id* seq_id = dynamic_cast<const CSeq_id*>(m_Subject.object.GetPointer());
    if (seq_id != NULL) {
        seq_id->GetLabel(&subject_seq, CSeq_id::eContent);
    }

    LOG_POST(Info << "    Subject: " << subject_seq);

    NON_CONST_ITERATE(TConstScopedObjects, iter, m_Queries) {
        seq_id = dynamic_cast<const CSeq_id*>(iter->object.GetPointer());
        if (seq_id != NULL) {
            string seq;
            seq_id->GetLabel(&seq, CSeq_id::eContent);
            if (!query_seqs.empty())
                query_seqs += ", ";
            query_seqs += seq;
        }
    }
    LOG_POST(Info << "    Query: " << query_seqs);
    LOG_POST(Info << "    Word size: " << m_WordSize);
    LOG_POST(Info << "    e-value: " << m_eValue);
    LOG_POST(Info << "    Use BLAST best hit algorithm: " << (m_BestHit ? "true" : "false"));

    string tune_for;
    switch (m_FilterMode) {
    case 0:
        tune_for = "Phase 1 HTG Sequences";
        break;
    case 1:
        tune_for = "Finished clones";
        break;
    case 2:
        tune_for = "Curated genomic sequences(NGs)";
        break;
    default:
        tune_for = "Invalid value";
        break;
    }

    LOG_POST(Info << "    Tune search for: " << tune_for);
    if (m_WM_TaxId != 0)
        LOG_POST(Info << "    Windowmasket TaxId: " << m_WM_TaxId);
    if (!m_AdvParams.empty())
        LOG_POST(Info << "    Advanced BLAST params: " << m_AdvParams);
}

END_NCBI_SCOPE
