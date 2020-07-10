#ifndef PKG_SEQUENCE___BAM_UTILS__HPP
#define PKG_SEQUENCE___BAM_UTILS__HPP

/*  
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
 * Authors:  Andrei Shkeda
 *
 * File Description:
 *
 */

#include <objects/seq/Seq_annot.hpp>

BEGIN_NCBI_SCOPE

class ICanceled;

class CBamUtils
{
public:
    typedef vector<CConstRef<objects::CSeq_id> > TSeqIdsVector;

    static CRef<objects::CSeq_annot>
        CreateCoverageGraphs(const string& bam_file,
                             const string& index_file,
                             const string& annot_name,
                             const string& assembly,
                             int bin_size = 1000,
                             TSeqIdsVector* ids = 0, 
                             bool is_esitmated = false,
                             ICanceled* canceled = 0);
    static bool GetCoverageGraphInfo(const objects::CSeq_annot& seq_annot, string& bam_data, string& bam_index, string& assembly);
    static void UpdateCoverageGraphInfo(objects::CSeq_annot& seq_annot, const string& bam_data, const string& bam_index, const string& assembly);

};


END_NCBI_SCOPE;

#endif  // PKG_SEQUENCE___BAM_UTILS__HPP
