#ifndef _GUI_PACKAGES_SEQUPD_HPP_
#define _GUI_PACKAGES_SEQUPD_HPP_

/*  $Id: sequpd.hpp 43730 2019-08-23 20:05:22Z asztalos $
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
 * Authors:  Andrea Asztalos
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui.hpp>

#include <util/icanceled.hpp>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <objects/seq/Seq_inst.hpp>

#include <objects/seqalign/Seq_align.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
class CBioseq;
class CBioseq_Handle;
class CSeq_entry_Handle;
END_SCOPE(objects)

class NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT sequpd
{
public:
    typedef map<objects::CSeq_id_Handle, objects::CSeq_id_Handle> TSeqIdHMap;
    typedef vector<objects::CSeq_id_Handle> TSeqIdHVector;

    static objects::CSeq_id_Handle GetGoodSeqIdHandle(const objects::CBioseq_Handle& bsh);

    static bool HaveIdenticalResidues(const objects::CBioseq_Handle& bsh1, const objects::CBioseq_Handle& bsh2);
    static void FixCollidingIDs_Bioseq(objects::CBioseq& bseq, const objects::CBioseq::TId& seq_ids);

    // matches update sequences with old ones
    // identifies identical pairs
    // if no error occurs, update sequences are added to the scope
    static void FindMatches(
        objects::CSeq_inst::EMol type,
        objects::CSeq_entry_Handle& oldSeq,
        objects::CSeq_entry& updEntry,
        TSeqIdHMap& matches,
        TSeqIdHVector& unmatched);

    static vector<CConstRef<objects::CSeq_align> > RunBlast2Seq(
        const objects::CBioseq_Handle& subject,
        const objects::CBioseq_Handle& query,
        bool accept_atleast_one, ICanceled* canceled = nullptr);

    /// Returns true if the first alignment is 'better' than the second one
    /// Compares the Blast scores and in the case of a tie, picks the longer alignment
    static bool CompareAlignments(const objects::CSeq_align& align_first, const objects::CSeq_align& align_sec);
};

END_NCBI_SCOPE

#endif // _GUI_PACKAGES_SEQUPD_HPP_
