#ifndef GUI_UTILS___TEXT_ALN_READER__HPP
#define GUI_UTILS___TEXT_ALN_READER__HPP

/*  $Id: text_aln_reader.hpp 38881 2017-06-28 20:57:40Z evgeniev $
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
* File Description: Defines text-based alignments reader class to be used by the GUI code
*
*/

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <objects/seqloc/Seq_id.hpp>
#include <objtools/readers/aln_reader.hpp>
#include <objtools/readers/fasta.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <gui/objutils/fasta_utils.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
class CScope;
END_SCOPE(objects)

/// Text-based alignments reader class that does post-processing:
/// * Overrides the local ids;
/// * Removes the well-known sequences from the set;
/// * Updates the organism information for the Seq-entry object by copying the information from a well-known sequence to the local sequence.
class NCBI_GUIOBJUTILS_EXPORT CTextAlnReader : public CAlnReader
{
public:
    CTextAlnReader(objects::CScope &scope, CNcbiIstream &is)
        : CAlnReader(is)
        , m_Scope(scope)
    {}

    CRef<objects::CSeq_entry> GetFilteredSeqEntry(TFastaFlags fasta_flags = 0);

protected:
    virtual CRef<objects::CSeq_id> GenerateID(const string& fasta_defline, const TSeqPos& line_number, TFastaFlags fasta_flags) override;

private:
    objects::CScope         &m_Scope;
    fasta_utils::TSeq_idMap m_LocalIds;

};

END_NCBI_SCOPE

#endif  // GUI_UTILS___TEXT_ALN_READER__HPP
