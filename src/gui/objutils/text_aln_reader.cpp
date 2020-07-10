/*  $Id: text_aln_reader.cpp 44740 2020-03-04 20:47:30Z evgeniev $
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
* Authors:  Victor Joukov, Vladislav Evgeniev
*
* File Description:
*
*/

#include <ncbi_pch.hpp>

#include <gui/objutils/text_aln_reader.hpp>

#include <objmgr/scope.hpp>
#include <objtools/readers/idmapper.hpp>
#include <objtools/readers/fasta.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CRef<objects::CSeq_id> CTextAlnReader::GenerateID(const string& fasta_defline, const TSeqPos& line_number, TFastaFlags fasta_flags)
{
    TSeqPos range_start = 0, range_end = 0;
    bool has_range = false;
    SDeflineParseInfo parse_info;
    parse_info.fBaseFlags = 0;
    parse_info.fFastaFlags = fasta_flags | objects::CFastaReader::fParseRawID;
    parse_info.maxIdLength = kMax_UI4;
    parse_info.lineNumber = line_number;

    TIgnoredProblems ignored_errors;
    TSeqTitles seq_titles;
    list<CRef<CSeq_id>> ids;
    try {
        CFastaDeflineReader::ParseDefline(fasta_defline,
            parse_info,
            ignored_errors,
            ids,
            has_range,
            range_start,
            range_end,
            seq_titles,
            0);
    }
    catch (const exception&) {}

    CRef<CSeq_id> result;
    const bool unique_id = (fasta_flags & objects::CFastaReader::fUniqueIDs);

    if (has_range) {

        string seq_id_text;
        CRef<CSeq_id> bestId;
        if (ids.empty() || unique_id) {
            seq_id_text = "lcl|" + NStr::IntToString(line_number);
        }
        else {
            bestId = FindBestChoice(ids, CSeq_id::BestRank);
            seq_id_text = "lcl|rng_" + bestId->GetSeqIdString(true);
        }

        seq_id_text += "-" + NStr::NumericToString(range_start + 1) + "-" + NStr::NumericToString(range_end + 1);
        result.Reset(new CSeq_id(seq_id_text));
        if (bestId.NotEmpty() && !bestId->IsLocal())
            m_LocalIds[result] = bestId;
        result = Ref(new CSeq_id(seq_id_text));
    }
    else {
        if (unique_id) {
            result = Ref(new CSeq_id("lcl|" + NStr::IntToString(line_number)));
        }
        else {
            result = CAlnReader::GenerateID(fasta_defline, line_number, fasta_flags);
        }
        if (!ids.empty()) {
            CRef<CSeq_id> bestId = FindBestChoice(ids, CSeq_id::BestRank);
            CRef<CSeq_id> non_local_id = fasta_utils::IdentifyLocalId(bestId->GetSeqIdString(true));
            if (non_local_id.NotEmpty())
                m_LocalIds[result] = non_local_id;
        }
    }

    return result;
}

CRef<CSeq_entry> CTextAlnReader::GetFilteredSeqEntry(TFastaFlags fasta_flags)
{
    CRef<CSeq_entry> entries = CAlnReader::GetSeqEntry(fasta_flags);

    entries.Reset(fasta_utils::ReplaceWellknownSeqs(*entries, m_Scope, nullptr, &m_LocalIds));

    if (m_LocalIds.empty())
        return entries;
        
    fasta_utils::UpdateOrgInformation(*entries, m_Scope, m_LocalIds);
    
    return entries;
}

END_NCBI_SCOPE
