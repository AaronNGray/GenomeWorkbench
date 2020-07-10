/*  $Id: editing_action_misc.hpp 45101 2020-05-29 20:53:24Z asztalos $
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

#ifndef _EDITING_ACTION_MISC_H_
#define _EDITING_ACTION_MISC_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/macro_fn_do.hpp>
#include <objtools/cleanup/capitalization_string.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CEditingActionConstraint;

class CEditingActionFlatFile : public IEditingAction 
{
public:
    CEditingActionFlatFile(CSeq_entry_Handle seh, const string &name = "CEditingActionFlatFile");
protected:
    virtual void Find(EActionType action);
    virtual void FindRelated(EActionType action);
    virtual void Modify(EActionType action);
    virtual void SwapContext(IEditingAction* source);
    virtual bool SameObject();
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);

private:
    string m_flat_file;
};

IEditingAction* CreateActionMisc(CSeq_entry_Handle seh, const string &field);

END_NCBI_SCOPE

#endif
    // _EDITING_ACTION_MISC_H_
