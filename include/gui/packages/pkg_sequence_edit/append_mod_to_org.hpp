/*  $Id: append_mod_to_org.hpp 42073 2018-12-12 21:23:04Z asztalos $
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
#ifndef _APPEND_MOD_TO_ORG_H_
#define _APPEND_MOD_TO_ORG_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objmgr/feat_ci.hpp>

BEGIN_NCBI_SCOPE

class CAppendModToOrg
{
public:
    CAppendModToOrg(bool isOrgMod, int subtype, string abbr, bool use_abbr, bool only_sp, bool only_cf, bool only_aff, bool only_nr, bool no_taxid, bool EndsWith = false,
                    CRef<CEditingActionConstraint> constraint = CRef<CEditingActionConstraint>(NULL)) :
        m_IsOrgMod(isOrgMod), m_Subtype(subtype), m_Abbreviation(abbr), m_Use_Abbreviation(use_abbr), 
        m_Only_sp(only_sp), m_Only_cf(only_cf), m_Only_aff(only_aff), m_Only_nr(only_nr), m_No_taxid(no_taxid), m_EndsWith(EndsWith), m_constraint(constraint) {};

    CRef<CCmdComposite> GetCommand(objects::CSeq_entry_Handle tse);

private:
    void apply_to_seq_and_feat(objects::CSeq_entry_Handle tse, CCmdComposite* composite);
    void x_ApplyToDescriptors(const objects::CSeq_entry& se, CCmdComposite* composite);
    bool x_ApplyToBioSource(objects::CBioSource& biosource);

    bool m_IsOrgMod;
    int m_Subtype;
    string m_Abbreviation;
    bool m_Use_Abbreviation;
    bool m_Only_sp, m_Only_cf, m_Only_aff, m_Only_nr, m_No_taxid;
    bool m_EndsWith;
    CRef<CEditingActionConstraint> m_constraint;
    CRef<objects::CScope> m_Scope;
};



END_NCBI_SCOPE

#endif
    // _APPEND_MOD_TO_ORG_H_
