/*  $Id: modify_structured_comment.hpp 42191 2019-01-10 16:36:16Z asztalos $
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
#ifndef _MODIFY_STRUCTURED_COMMENT_H_
#define _MODIFY_STRUCTURED_COMMENT_H_
#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CUser_object;
    class CComment_set;
    class CComment_rule;
    class CSeq_entry;
END_SCOPE(objects);

class CModifyStructuredComment
{
public:
    bool ApplyToUserObject(objects::CUser_object& user);
    void ApplyToCSeq_entry (objects::CSeq_entry_Handle tse, const objects::CSeq_entry& se, CCmdComposite* composite);
    void apply_to_seq_and_feat(objects::CSeq_entry_Handle tse, CCmdComposite* composite);
    bool apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, const string& title);
private:
    string m_Field;
    wxString m_FindStr;
};

class CReorderStructuredComment
{
public:
    bool apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, const string& title);
    void ApplyToCSeq_entry_user (objects::CSeq_entry_Handle tse, const objects::CSeq_entry& se, CCmdComposite* composite);
    void reorder_comment_by_rule(const objects::CSeqdesc& user_desc, objects::CSeq_entry_Handle seh, const objects::CComment_rule* rule, CCmdComposite* composite);
private:
    CConstRef<objects::CComment_set> m_Rules;
};


END_NCBI_SCOPE

#endif
    // _MODIFY_STRUCTURED_COMMENT_H_
