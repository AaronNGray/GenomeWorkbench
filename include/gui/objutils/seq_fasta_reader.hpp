#ifndef GUI_UTILS___SEQ_FASTA_READER__HPP
#define GUI_UTILS___SEQ_FASTA_READER__HPP

/*  $Id: seq_fasta_reader.hpp 38900 2017-06-30 20:50:04Z evgeniev $
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
* Authors:  Victor Joukov, Vladislav Evgeniev
*
* File Description: Defines FASTA sequences reader class to be used by the GUI code
*
*/

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objtools/readers/fasta.hpp>
#include <gui/objutils/fasta_utils.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
class CScope;
END_SCOPE(objects)

/// FASTA sequences reader class that does post-processing:
/// * Overrides the local ids;
/// * Removes the well-known sequences from the set and optionally returns their ids;
/// * Updates the organism information for the Seq-entry object by copying the information from a well-known sequence to the local sequence.
class NCBI_GUIOBJUTILS_EXPORT CSeqFastaReader : public objects::CFastaReader
{
public:
    CSeqFastaReader(objects::CScope& scope, ILineReader& reader, objects::CFastaReader::TFlags flags = objects::CFastaReader::fParseRawID)
        : objects::CFastaReader(reader, flags)
        , m_Scope(scope)
    {}
    CSeqFastaReader(objects::CScope& scope, CNcbiIstream& in, TFlags flags = objects::CFastaReader::fParseRawID)
        : objects::CFastaReader(in, flags)
        , m_Scope(scope)
    {}
    virtual void GenerateID() override;

    virtual void PostProcessIDs(const objects::CBioseq::TId& defline_ids,
        const string& defline,
        bool has_range = false,
        TSeqPos range_start = kInvalidSeqPos,
        TSeqPos range_end = kInvalidSeqPos) override;
    
    /// Read multiple sequences and remove the well-known sequences from the entry
    CRef<objects::CSeq_entry> ReadSequences(vector<CConstRef<objects::CSeq_id>> *wellknown_ids = nullptr, int max_seqs = kMax_Int, objects::ILineErrorListener * pMessageListener = 0);

private:
    objects::CScope         &m_Scope;
    fasta_utils::TSeq_idMap m_LocalIds;
};

END_NCBI_SCOPE

#endif  // GUI_UTILS___SEQ_FASTA_READER__HPP
