/*  $Id: wrong_illegal_quals.hpp 32523 2015-03-17 18:05:29Z filippov $
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


#ifndef _WRONG_ILLEGAL_QUALS_H_
#define _WRONG_ILLEGAL_QUALS_H_

#include <corelib/ncbistd.hpp>
#include <gui/utils/command_processor.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objects/seqfeat/Seq_feat.hpp>

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

class CWrongIllegalQuals
{
public:
    static void IllegalQualsToNote( CSeq_entry_Handle seh,  ICommandProccessor* cmd_processor);
    static void RmIllegalQuals( CSeq_entry_Handle seh,  ICommandProccessor* cmd_processor);
    static void WrongQualsToNote( CSeq_entry_Handle seh,  ICommandProccessor* cmd_processor);
    static void RmWrongQuals( CSeq_entry_Handle seh,  ICommandProccessor* cmd_processor);
    static void RmWrongOrIllegalQuals( CSeq_entry_Handle seh,  ICommandProccessor* cmd_processor);
};

END_NCBI_SCOPE

#endif  // _WRONG_ILLEGAL_QUALS_H_
