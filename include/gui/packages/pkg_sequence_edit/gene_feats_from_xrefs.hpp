/*  $Id: gene_feats_from_xrefs.hpp 37141 2016-12-07 19:33:54Z filippov $
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
 * Authors:  Igor Filippov
 */


#ifndef _GENE_FEATS_FROM_XREFS_H_
#define _GENE_FEATS_FROM_XREFS_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objects/seqfeat/Seq_feat.hpp>

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);


class CGeneFeatFromXrefs 
{
public:
    static CRef<CCmdComposite> GetCommand(CSeq_entry_Handle seh);
};

class CGeneXrefsFromFeats
{
public:
    static CRef<CCmdComposite> GetCommand(CSeq_entry_Handle seh);
};

END_NCBI_SCOPE

#endif  // _GENE_FEATS_FROM_XREFS_H_
