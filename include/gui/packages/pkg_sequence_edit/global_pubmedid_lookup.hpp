/*  $Id: global_pubmedid_lookup.hpp 37297 2016-12-21 20:03:22Z filippov $
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
#ifndef _GLOBAL_PUBMEDID_LOOKUP_H_
#define _GLOBAL_PUBMEDID_LOOKUP_H_

#include <corelib/ncbistd.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CGlobalPubmedIdLookup
{
public:
    static CRef<CCmdComposite> apply(CSeq_entry_Handle tse);
};

END_NCBI_SCOPE

#endif
    // _GLOBAL_PUBMEDID_LOOKUP_H_
