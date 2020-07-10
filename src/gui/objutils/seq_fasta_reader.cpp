/*  $Id: seq_fasta_reader.cpp 38900 2017-06-30 20:50:04Z evgeniev $
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

#include <gui/objutils/seq_fasta_reader.hpp>

#include <corelib/ncbiutil.hpp>
#include <objmgr/scope.hpp>
#include <objtools/readers/idmapper.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CSeqFastaReader::GenerateID() 
{
    CTempString defline = m_LineReader->GetCurrentLine();

    TSeqPos range_start = 0, range_end = 0;
    bool has_range = false;
    SDefLineParseInfo parseInfo;
    parseInfo.fBaseFlags = m_iFlags;
    parseInfo.fFastaFlags = GetFlags() | fParseRawID;
    parseInfo.maxIdLength = m_MaxIDLength;
    parseInfo.lineNumber = LineNumber();

    list<CRef<CSeq_id> > ids;
    try {
        ParseDefLine(defline,
            parseInfo,
            m_ignorable,
            ids,
            has_range,
            range_start,
            range_end,
            m_CurrentSeqTitles,
            0);
    }
    catch (const exception&) {}

    for (auto it = ids.begin(); it != ids.end();) {
        const CSeq_id& id = **it;
        switch (id.Which()) {
        case CSeq_id::e_not_set:
        case CSeq_id::e_Local:
            it = ids.erase(it);
            break;
        default:
            ++it;
            break;
        }
    }

    if (!ids.empty()) {
        CRef<CSeq_id> bestId = FindBestChoice(ids, CSeq_id::BestRank);
        string seq_id_text = "lcl|";
        if (has_range)
            seq_id_text += "rng_";
        seq_id_text += bestId->GetSeqIdString(true);
        if (has_range)
            seq_id_text += "-" + NStr::NumericToString(range_start + 1) + "-" + NStr::NumericToString(range_end + 1);
        CRef<CSeq_id> seq_id(new CSeq_id(seq_id_text));
        SetIDs().push_back(seq_id);
        m_LocalIds[seq_id] = bestId;
    }
    else {
        CFastaReader::GenerateID();
    }
}

void CSeqFastaReader::PostProcessIDs(const CBioseq::TId& defline_ids, const string& defline, bool has_range, TSeqPos range_start, TSeqPos range_end)
{
    CFastaReader::PostProcessIDs(defline_ids, defline, has_range, range_start, range_end);

    if (defline_ids.empty())
        return;

    CRef<CSeq_id> bestId = FindBestChoice(defline_ids, CSeq_id::BestRank);
    if (!bestId->IsLocal())
        return;
    
    CRef<CSeq_id> non_local_id = fasta_utils::IdentifyLocalId(bestId->GetSeqIdString(true));
    if (non_local_id.Empty())
        return;
    
    m_LocalIds[bestId] = non_local_id;
}

CRef<CSeq_entry> CSeqFastaReader::ReadSequences(vector<CConstRef<CSeq_id>> *wellknown_ids, int max_seqs, ILineErrorListener * pMessageListener)
{
    CRef<CSeq_entry> entries = CFastaReader::ReadSet(max_seqs, pMessageListener);

    entries.Reset(fasta_utils::ReplaceWellknownSeqs(*entries, m_Scope, wellknown_ids, &m_LocalIds));

    if (m_LocalIds.empty())
        return entries;
        
    fasta_utils::UpdateOrgInformation(*entries, m_Scope, m_LocalIds);
    
    return entries;
}



END_NCBI_SCOPE
