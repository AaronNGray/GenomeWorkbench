/*  $Id: remove_structuredcomment_keyword.hpp 32695 2015-04-13 16:18:43Z filippov $
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
#ifndef _REMOVE_STRUCTUREDCOMMENT_KEYWORD_H_
#define _REMOVE_STRUCTUREDCOMMENT_KEYWORD_H_
#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objects/seqblock/GB_block.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <objects/misc/sequence_macros.hpp>


BEGIN_NCBI_SCOPE
using namespace objects;

class CChangeStructuredCommentKeyword
{
public: 
    typedef  CChangeUnindexedObjectCommand<objects::CGB_block> CChangeGBblockCommand;
    CChangeStructuredCommentKeyword();
    virtual ~CChangeStructuredCommentKeyword() {}
    virtual void ApplyToCSeq_entry_genbank (objects::CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite) = 0;
    virtual void ApplyToCSeq_entry_user (CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite);
    void apply_to_seq_and_feat(CSeq_entry_Handle tse, CCmdComposite* composite);
    bool apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title);
    string PrefixToKeyword(const string &prefix);
    set<string> m_Keywords;
    set<string> m_ValidKeywords;
    map<string,string> m_PrefixToKeyword;
};

class CRemoveStructuredCommentKeyword : public CChangeStructuredCommentKeyword
{
public: 
    virtual void ApplyToCSeq_entry_genbank (objects::CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite);
};

class CAddStructuredCommentKeyword : public CChangeStructuredCommentKeyword
{
public: 
    virtual void ApplyToCSeq_entry_genbank (objects::CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite);
    void add_to_existing_genbank (objects::CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite, string keyword, bool &modified);
};

class CAddConstKeyword : public CAddStructuredCommentKeyword
{
public:
    virtual void ApplyToCSeq_entry_user (CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite) {}
    void SetKeyword(const string &keyword) {m_ValidKeywords.insert(keyword);}
};

END_NCBI_SCOPE

#endif
    // _REMOVE_STRUCTUREDCOMMENT_KEYWORD_H_
