/*  $Id: editing_action_constraint.hpp 45101 2020-05-29 20:53:24Z asztalos $
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

#ifndef _EDITING_ACTION_CONSTRAINT_H_
#define _EDITING_ACTION_CONSTRAINT_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/RNA_qual.hpp>
#include <objects/macro/String_constraint.hpp>
#include <objects/macro/Location_constraint.hpp>
#include <objects/macro/Location_pos_constraint.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/seq_feat_handle.hpp>
#include <gui/objutils/macro_fn_do.hpp>
#include <objtools/cleanup/capitalization_string.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>
#include <gui/widgets/edit/pub_fieldtype.hpp>
#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_features.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_biosource.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_desc.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CConstraintMatcher : public CObject
{
public:
    virtual ~CConstraintMatcher() {}
    virtual bool GetNegation(void) {return false;}
    virtual void SetNegation(bool val) {}
    virtual bool DoesTextMatch(const string& val) {return false;}
    virtual bool IsEmpty(void) const {return true;}
    virtual void SetFeat(CSeq_feat_Handle fh) {}
    virtual void SetDesc(const CSeqdesc* desc) {}
};

class CSimpleConstraintMatcher : public CConstraintMatcher
{
public :
    explicit CSimpleConstraintMatcher(CRef<edit::CStringConstraint> constraint) : m_constraint(constraint) {}
    virtual bool GetNegation() {return m_constraint->GetNegation();}
    virtual void SetNegation(bool val) {m_constraint->SetNegation(val);}
    virtual bool DoesTextMatch(const string& val) {return m_constraint->DoesTextMatch(val);}
    virtual bool IsEmpty(void) const { return m_constraint.Empty() || m_constraint->GetMatchText().empty();}
private:
    mutable CRef<edit::CStringConstraint> m_constraint;
};

class CAdvancedConstraintMatcher : public CConstraintMatcher
{
public :
    explicit CAdvancedConstraintMatcher(CRef<CString_constraint> constraint) : m_constraint(constraint) {}
    virtual bool GetNegation() {return m_constraint->IsSetNot_present() && m_constraint->GetNot_present();}
    virtual void SetNegation(bool val) {m_constraint->SetNot_present(val);}
    virtual bool DoesTextMatch(const string& val) {return m_constraint->Match(val);}
    virtual bool IsEmpty(void) const { return m_constraint.Empty() || m_constraint->Empty();}
private:
    CRef<CString_constraint> m_constraint;
};


class CPresentConstraintMatcher : public CConstraintMatcher
{
public :
    explicit CPresentConstraintMatcher() : m_negate(false) {}
    virtual bool GetNegation() {return m_negate;}
    virtual void SetNegation(bool val) {m_negate = val;}
    virtual bool DoesTextMatch(const string& val) 
        {            
            if (m_negate)
                return val.empty();
            else
                return !val.empty();
        }
    virtual bool IsEmpty(void) const {return false;}
private:
    bool m_negate;
};

class CSameConstraintMatcher : public CConstraintMatcher
{
public :
    explicit CSameConstraintMatcher(const string &field, CFieldNamePanel::EFieldType field_type, int subtype);
    virtual bool GetNegation() {return m_negate;}
    virtual void SetNegation(bool val) {m_negate = val;}
    virtual bool DoesTextMatch(const string& val) 
        {     
            if (m_negate)
                return m_values.find(val) == m_values.end();
            else
                return m_values.find(val) != m_values.end();
        }
    virtual bool IsEmpty(void) const {return false;}
    virtual void SetFeat(CSeq_feat_Handle fh);
    virtual void SetDesc(const CSeqdesc* desc);
private:
    bool m_negate;
    set<string> m_values;
    CIRef<IEditingAction> m_action;
    IEditingActionFeat* m_feat_action;
    IEditingActionBiosource* m_src_action;
    IEditingActionDesc* m_desc_action;
};

class CLocationConstraintMatcher : public CConstraintMatcher
{
public :
    explicit CLocationConstraintMatcher(CRef<objects::CLocation_constraint> lc) : m_lc(lc) {}
    virtual bool Match(CSeq_feat_Handle fh); 
    virtual bool IsEmpty(void) const;
private:
    CRef<objects::CLocation_constraint> m_lc;
};

class CPubStatusConstraintMatcher : public CConstraintMatcher
{
public :
    explicit CPubStatusConstraintMatcher(CPubFieldType::EPubFieldStatus status) : m_status(status) {}
    explicit CPubStatusConstraintMatcher(const string &status) {m_status = CPubFieldType::GetStatusFromString(status);}
    virtual bool Match(const CSeqdesc* obj); 
    virtual bool IsEmpty(void) const {return m_status == CPubFieldType::ePubFieldStatus_Any;}
private:
    CPubFieldType::EPubFieldStatus m_status;
};

class CEditingActionConstraint : public CObject
{
public:
    CEditingActionConstraint() {}
    virtual ~CEditingActionConstraint() {}
    virtual bool Match(const string &value);
    virtual bool Match(CSeq_feat_Handle fh);
    virtual bool Match(const CSeqdesc* desc);
    virtual bool Match(CBioseq_Handle bsh);
    virtual bool Match(void);
    CEditingActionConstraint& operator+=(CRef<CEditingActionConstraint> next);
    virtual void SetFeat(CSeq_feat_Handle fh) {}
    virtual void SetDesc(const CSeqdesc* desc) {}
protected:
    CRef<CEditingActionConstraint> m_Next; 
};

class CEditingActionStringConstraint : public CEditingActionConstraint
{
public:
    CEditingActionStringConstraint(CRef<CConstraintMatcher>matcher);
    virtual bool Match(const string &value);
    virtual bool Match(CSeq_feat_Handle fh);
    virtual bool Match(const CSeqdesc* desc);
    virtual bool Match(void) {return false;}  
    bool GetNegation() {return m_matcher->GetNegation();}
    void SetNegation(bool val) {m_matcher->SetNegation(val);}
    virtual void SetFeat(CSeq_feat_Handle fh) {m_matcher->SetFeat(fh);}
    virtual void SetDesc(const CSeqdesc* desc) {m_matcher->SetDesc(desc);}
private:
    CRef<CConstraintMatcher>  m_matcher;
};

class CEditingActionSummaryConstraint : public CEditingActionConstraint
{
public:
    CEditingActionSummaryConstraint(const string &const_field, CFieldNamePanel::EFieldType const_field_type, int const_subtype, CRef<CConstraintMatcher>constraint);
protected:
    CRef<CEditingActionConstraint> m_string_constraint;
    CIRef<IEditingAction> m_action;
    bool m_revert_constraint;
    IEditingActionFeat* m_feat_action;
    IEditingActionBiosource* m_src_action;
    IEditingActionDesc* m_desc_action;
};

class CEditingActionFeatureConstraint : public CEditingActionSummaryConstraint
{
public:
    CEditingActionFeatureConstraint(const string &const_field, CFieldNamePanel::EFieldType const_field_type, int const_subtype, CRef<CConstraintMatcher>constraint);
    virtual bool Match(CSeq_feat_Handle fh);
};

class CEditingActionFeatureLocationConstraint : public CEditingActionConstraint
{
public:
    CEditingActionFeatureLocationConstraint(CRef<CLocationConstraintMatcher> loc_matcher);
    virtual bool Match(CSeq_feat_Handle fh);
private:
    CRef<CLocationConstraintMatcher>  m_matcher;
};

class CEditingActionRelatedFeatureConstraint : public CEditingActionSummaryConstraint
{
public:
    CEditingActionRelatedFeatureConstraint(const string &const_field, CFieldNamePanel::EFieldType const_field_type, int const_subtype, CRef<CConstraintMatcher>constraint);
    virtual bool Match(CSeq_feat_Handle fh);
};


class CEditingActionDescriptorConstraint : public CEditingActionSummaryConstraint
{
public:
    CEditingActionDescriptorConstraint(const string &const_field, CFieldNamePanel::EFieldType const_field_type, int const_subtype, CRef<CConstraintMatcher>constraint);
    virtual bool Match(const CSeqdesc* desc);
};

class CEditingActionBiosourceConstraint : public CEditingActionSummaryConstraint
{
public:
    CEditingActionBiosourceConstraint(const string &const_field, CFieldNamePanel::EFieldType const_field_type, int const_subtype, CRef<CConstraintMatcher>constraint);
    virtual bool Match(const CSeqdesc* desc);
    virtual bool Match(CSeq_feat_Handle fh);
};

class CEditingActionBioseqConstraint : public CEditingActionSummaryConstraint
{
public:
    CEditingActionBioseqConstraint(const string &const_field, CFieldNamePanel::EFieldType const_field_type, int const_subtype, CRef<CConstraintMatcher>constraint);
    virtual bool Match(CBioseq_Handle bsh);
};

class CEditingActionPubStatusConstraint : public CEditingActionConstraint
{
public:
    CEditingActionPubStatusConstraint(CRef<CPubStatusConstraintMatcher> pub_matcher);
    virtual bool Match(const CSeqdesc* obj);
private:
    CRef<CPubStatusConstraintMatcher>  m_matcher;
};

CRef<CEditingActionConstraint> CreateEditingActionConstraint(const string &field, CFieldNamePanel::EFieldType field_type, int subtype, 
                                                             const string &const_field, CFieldNamePanel::EFieldType const_field_type, int const_subtype, 
                                                             CRef<CConstraintMatcher>constraint);
END_NCBI_SCOPE

#endif
    // _EDITING_ACTION_CONSTRAINT_H_
