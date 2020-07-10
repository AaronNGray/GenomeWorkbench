#ifndef GUI_WIDGETS_SNP___SNP_DATA__HPP
#define GUI_WIDGETS_SNP___SNP_DATA__HPP

/*  $Id: snp_data.hpp 24909 2011-12-22 17:09:30Z dicuccio $
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
 * Authors:  Melvin Quintos
 *
 *  File Description:
 *
 */

#include <string>
#include <objtools/snputil/snp_bitfield.hpp>
#include <objects/seqloc/Seq_loc.hpp>

BEGIN_NCBI_SCOPE

struct SSnpData
{
    string        rsid;
    string        gene_id;
    string        url;
    string        studyName;
    double        studyValue;
    CSnpBitfield  bitfield;

    CConstRef<objects::CSeq_loc>  loc;
    CConstRef<objects::CSeq_feat> orig_feat;

    /*
    // data modified before being displayed (see CSnpTableData::GetValueAt)
    mutable CLinkButton::SLink rsid_link;
    mutable CLinkButton::SLink gene_link;
    mutable CLinkButton::SLink disease_link;
    mutable CLinkButton::SLink omim_link;
    mutable CLinkButton::SLink gty_link;
    mutable CLinkButton::SLink snp3d_link;
    mutable string        seq_id_str;
    */
};

typedef vector<SSnpData>                            TSnpList;
typedef TSnpList::iterator                          TSnpListIter;
typedef CObjectFor<TSnpList>                        TSnpListObject;

typedef vector<const SSnpData *>                    TSnpPtrList;
typedef TSnpPtrList::iterator                       TSnpPtrListIter;
typedef CObjectFor<TSnpPtrList>                     TSnpPtrListObject;

END_NCBI_SCOPE


#endif // GUI_WIDGETS_SNP___SNP_DATA__HPP

