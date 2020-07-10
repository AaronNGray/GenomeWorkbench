/*  $Id: split_cds_with_too_many_n.hpp 42167 2019-01-08 17:17:20Z filippov $
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
 * Authors: Igor Filippov
 */
#ifndef _SPLIT_CDS_WITH_TOO_MANY_N_H_
#define _SPLIT_CDS_WITH_TOO_MANY_N_H_


#include <corelib/ncbistd.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>

BEGIN_NCBI_SCOPE

class CSplitCDSwithTooManyXs
{
public:
    static CRef<CCmdComposite> apply(objects::CSeq_entry_Handle tse);
    static bool TooManyXs(objects::CSeq_feat_Handle fh);

private:
    static void AdjustSingleCDS(const objects::CMappedFeat& feat, TSeqPos seq_start, TSeqPos seq_length, objects::CScope &scope, const vector < pair<int,int> > &gaps, CRef<CCmdComposite> composite, bool create_general_only);
    static void FindNewLocations(const objects::CSeq_loc &feat_loc, const vector < pair<int,int> > &gaps, TSeqPos seq_start, TSeqPos seq_length, vector<CRef<objects::CSeq_loc> > &locations, bool &comment);
    static void GapLocationsFromNs(objects::CBioseq_Handle bsh, vector < pair<int,int> > &start_length_locs);
};

END_NCBI_SCOPE

#endif
// _SPLIT_CDS_WITH_TOO_MANY_N_H_
