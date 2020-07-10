/*  $Id: update_replaced_ec_numbers.hpp 32177 2015-01-14 22:33:07Z filippov $
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
#ifndef _UPDATE_REPLACED_EC_NUM_H_
#define _UPDATE_REPLACED_EC_NUM_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <objmgr/feat_ci.hpp>



BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CUpdateReplacedECNumbers
{
public:
    void apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title);
};

END_NCBI_SCOPE

#endif
    // _UPDATE_REPLACED_EC_NUM_H_
