/*  $Id: remove_unnecessary_exceptions.hpp 39517 2017-10-04 15:23:59Z filippov $
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
 *
 * Authors:  Igor Filippov
 */
#ifndef _REMOVE_UNNECESSARY_EXCEPTIONS_H_
#define _REMOVE_UNNECESSARY_EXCEPTIONS_H_
#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;

class CRemoveUnnecessaryExceptions
{
public: 
    static void apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor);
};

END_NCBI_SCOPE

#endif
    // _REMOVE_UNNECESSARY_EXCEPTIONS_H_
