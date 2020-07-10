/*  $Id: field_handler.hpp 34511 2016-01-19 16:21:35Z asztalos $
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
 * Authors:  Colleen Bollin
 */


#ifndef _GUI_PKG_EDIT__FIELD_HANDLER_HPP_
#define _GUI_PKG_EDIT__FIELD_HANDLER_HPP_

#include <corelib/ncbistd.hpp>

#include <objmgr/scope.hpp>
#include <objtools/edit/seqid_guesser.hpp>
#include <objtools/edit/apply_object.hpp>

#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>

#include <gui/objutils/cmd_composite.hpp>

BEGIN_NCBI_SCOPE

class CFieldHandler : public CObject
{
public:
    virtual vector<CConstRef<CObject> > GetObjects(objects::CBioseq_Handle bsh) = 0;
    virtual vector<CConstRef<CObject> > GetObjects(objects::CSeq_entry_Handle seh, const string& constraint_field, CRef<objects::edit::CStringConstraint> string_constraint) = 0;
    virtual vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(objects::CBioseq_Handle bsh) = 0;
    virtual string GetVal(const CObject& object) = 0;
    virtual vector<string> GetVals(const CObject& object) = 0;
    virtual bool IsEmpty(const CObject& object) const = 0;
    virtual void ClearVal(CObject& object) = 0;
    virtual bool SetVal(CObject& object, const string& val, objects::edit::EExistingText existing_text) = 0;
    virtual string IsValid(const string& val) { return ""; };
    virtual vector<string> IsValid(const vector<string>& values) { vector<string> x; return x; };;
    virtual objects::CSeqFeatData::ESubtype GetFeatureSubtype() = 0;
    virtual objects::CSeqdesc::E_Choice GetDescriptorSubtype() = 0;
    virtual void SetConstraint(const string& field, CConstRef<objects::edit::CStringConstraint> string_constraint) = 0;
    virtual bool AllowMultipleValues() = 0;
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const CObject& object, CRef<objects::CScope> scope) = 0;
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const objects::edit::CApplyObject& object) = 0;
    void SetScope(CRef<CScope> scope) { m_Scope = scope; };
protected:
    CRef<CScope> m_Scope;
};


END_NCBI_SCOPE

#endif // _GUI_PKG_EDIT__FIELD_HANDLER_HPP_