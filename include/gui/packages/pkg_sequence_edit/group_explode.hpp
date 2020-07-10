/*  $Id: group_explode.hpp 37200 2016-12-14 20:51:20Z filippov $
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


#ifndef _GROUP_EXPLODE_H_
#define _GROUP_EXPLODE_H_

#include <corelib/ncbistd.hpp>
#include <gui/utils/command_processor.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <gui/framework/workbench_impl.hpp>

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

class CGroupExplode
{
public:
    static void GetSeqFeats( IWorkbench* wb, vector<const objects::CSeq_feat*> &result );
    static void apply( IWorkbench* wb,  ICommandProccessor* cmd_processor, CScope &scope);
};

class CExplodeRNAFeats
{
public:
    static CRef<CCmdComposite> apply(objects::CSeq_entry_Handle tse);
private:
    static void RNAWordsFromString(string name, vector<string> &names);
};

END_NCBI_SCOPE

#endif  // _GROUP_EXPLODE_H_
