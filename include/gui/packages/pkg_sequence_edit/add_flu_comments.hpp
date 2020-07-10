/*  $Id: add_flu_comments.hpp 31479 2014-10-15 19:10:36Z filippov $
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
#ifndef _ADD_FLU_COMMENTS_H_
#define _ADD_FLU_COMMENTS_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <objects/seqfeat/BioSource.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;

class CAddFluComments
{    

public:
    void apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title);
private:
    struct SCompareStr
    {
        bool operator ()(const string& a, const string& b)
            {
                long num_a = NStr::StringToULong(a, NStr::fAllowLeadingSymbols | NStr::fConvErr_NoThrow);
                long num_b = NStr::StringToULong(b, NStr::fAllowLeadingSymbols | NStr::fConvErr_NoThrow);
                if (num_a !=0 && num_b != 0)
                    return num_a < num_b;
                return a < b;
            }
    };
    typedef set<string, SCompareStr> TNumStrSet;
  
    void ApplyToCSeq_entry (objects::CSeq_entry_Handle tse, CCmdComposite* composite);
    CRef<objects::CSeqdesc> CreateNewComment(const TNumStrSet &ids, const string &taxname);
    void AddBioSource(const CBioSource& biosource, string &taxname);
    void GetDesc(const CSeq_entry& se, string & taxname);
    void FindBioSource(objects::CSeq_entry_Handle tse, string &taxname);
    string GetBestLabel(const CBioseq &seq);
    string ConcatIds(const TNumStrSet &ids);
};

END_NCBI_SCOPE

#endif
    // _ADD_FLU_COMMENTS_H_
