/*  $Id: editing_action_constraint.cpp 45100 2020-05-29 20:38:10Z asztalos $
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

#include <ncbi_pch.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <objmgr/object_manager.hpp>
#include <gui/widgets/edit/bioseq_editor.hpp>
#include <objtools/edit/string_constraint.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/packages/pkg_sequence_edit/pub_field.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CRef<CEditingActionConstraint> CreateEditingActionConstraint(const string &field, CFieldNamePanel::EFieldType field_type, int subtype, 
                                                             const string &const_field, CFieldNamePanel::EFieldType const_field_type, int const_subtype, 
                                                             CRef<CConstraintMatcher>constraint)
{
    if (!constraint || constraint->IsEmpty())
	return CRef<CEditingActionConstraint>(new CEditingActionConstraint());

    CLocationConstraintMatcher *loc_matcher = dynamic_cast<CLocationConstraintMatcher*>(constraint.GetPointer());
    CPubStatusConstraintMatcher *pub_status_matcher = dynamic_cast<CPubStatusConstraintMatcher*>(constraint.GetPointer());
    if (loc_matcher)
    {
	return CRef<CEditingActionConstraint>(new CEditingActionFeatureLocationConstraint(CRef<CLocationConstraintMatcher>(loc_matcher))); 
    }
    else if (pub_status_matcher)
    {
	return CRef<CEditingActionConstraint>(new CEditingActionPubStatusConstraint(CRef<CPubStatusConstraintMatcher>(pub_status_matcher))); 
    }
    else if (const_field_type != CFieldNamePanel::eFieldType_Unknown)
    {
        
        if (const_subtype == -1 && const_field_type == CFieldNamePanel::eFieldType_RNA &&  // RNA subtype "any" should match any RNAs
            (subtype == CSeqFeatData::eSubtype_preRNA || subtype == CSeqFeatData::eSubtype_mRNA || subtype == CSeqFeatData::eSubtype_tRNA || 
             subtype == CSeqFeatData::eSubtype_rRNA || subtype == CSeqFeatData::eSubtype_snRNA || subtype == CSeqFeatData::eSubtype_scRNA || 
             subtype == CSeqFeatData::eSubtype_snoRNA || subtype == CSeqFeatData::eSubtype_ncRNA || subtype == CSeqFeatData::eSubtype_tmRNA || 
             subtype == CSeqFeatData::eSubtype_otherRNA ) )
            const_subtype = subtype;
            
        if ((subtype == const_subtype  && field == const_field && field_type == const_field_type) ||
            (subtype >= 0 && subtype == const_subtype  && field == const_field) )
        {
            // constraint on value
            return CRef<CEditingActionConstraint>(new CEditingActionStringConstraint(constraint));            
        }
        else if (field != const_field && field_type == const_field_type && (field_type == CFieldNamePanel::eFieldType_Source || field_type == CFieldNamePanel::eFieldType_Taxname))
        {
            // constraint on biosource
            return CRef<CEditingActionConstraint>(new CEditingActionBiosourceConstraint(const_field, const_field_type, const_subtype, constraint)); 
        }
        else if ( subtype >= 0 && subtype == const_subtype && field != const_field)
        {
            // constraint on seq-feat
            return CRef<CEditingActionConstraint>(new CEditingActionFeatureConstraint(const_field, const_field_type, const_subtype, constraint)); 
        }
        else if ( subtype >= 0 && const_subtype >= 0 && subtype != const_subtype)
        {
            // constraint on related seq-feat
            return CRef<CEditingActionConstraint>(new CEditingActionRelatedFeatureConstraint(const_field, const_field_type, const_subtype, constraint)); 
        }
        else if ( subtype == -1 && subtype == const_subtype  && field != const_field && field_type == const_field_type 
                  && (field_type == CFieldNamePanel::eFieldType_Misc || field_type == CFieldNamePanel::eFieldType_StructuredComment || field_type == CFieldNamePanel::eFieldType_DBLink
                      || field_type == CFieldNamePanel::eFieldType_MolInfo || field_type == CFieldNamePanel::eFieldType_Pub))
        {
            // constraint on seqdesc
            return CRef<CEditingActionConstraint>(new CEditingActionDescriptorConstraint(const_field, const_field_type, const_subtype, constraint)); 
        }
        else 
        {
            // constraint on bioseq   
            return CRef<CEditingActionConstraint>(new CEditingActionBioseqConstraint(const_field, const_field_type, const_subtype, constraint)); 
        }
    }
  
    return CRef<CEditingActionConstraint>(new CEditingActionConstraint());
}

bool CEditingActionConstraint::Match(const string &value)
{
    if (m_Next)
        return m_Next->Match(value);
    return true;
}

bool CEditingActionConstraint::Match(CSeq_feat_Handle fh)
{
    if (m_Next)
        return m_Next->Match(fh);
    return true;
}

bool CEditingActionConstraint::Match(const CSeqdesc* desc)
{
    if (m_Next)
        return m_Next->Match(desc);
    return true;
}

bool CEditingActionConstraint::Match(CBioseq_Handle bsh)
{
    if (m_Next)
        return m_Next->Match(bsh);
    return true;
}

bool CEditingActionConstraint::Match(void)
{
    if (m_Next)
        return m_Next->Match();
    return true;
}

CEditingActionConstraint& CEditingActionConstraint::operator+=(CRef<CEditingActionConstraint> next)
{
    if (!next)
        return *this;
    if (next.GetPointer() == this)
        return *this;
    if (m_Next)
        *m_Next += next;
    else
        m_Next = next;
    return *this;
}

static CSeq_feat_Handle ChangeFeat(IEditingActionFeat* feat_action, CSeq_feat_Handle fh)
{
    if (fh.GetFeatSubtype() != feat_action->GetActionFeatSubtype()) 
    {           
        if (fh.GetFeatSubtype() == CSeqFeatData::eSubtype_prot)
        {
            const CSeq_loc& prot_loc = fh.GetLocation();
            CBioseq_Handle prot_bsh = fh.GetScope().GetBioseqHandle(prot_loc);
            if (prot_bsh)
            {
                CMappedFeat mapped_cds = sequence::GetMappedCDSForProduct(prot_bsh);
                if (mapped_cds)
                {
                    CSeq_feat_Handle cds_fh = mapped_cds.GetSeq_feat_Handle();
                    if (cds_fh)
                    {
                        fh = cds_fh;
                        if (feat_action->GetActionFeatSubtype() != CSeqFeatData::eSubtype_cdregion)
                        {
                            CSeq_feat_Handle gene_fh = feat_action->x_FindGeneForFeature(fh.GetLocation(), fh.GetScope(), feat_action->GetActionFeatSubtype());                  
                            if (gene_fh)
                            {
                                fh = gene_fh;
                            }
                        }
                    }
                }
            }
        }
        else if (feat_action->GetActionFeatSubtype() == CSeqFeatData::eSubtype_prot)
        {
            CSeq_feat_Handle cds_fh = fh;
            if (cds_fh.GetFeatSubtype() != CSeqFeatData::eSubtype_cdregion)
            {
                cds_fh = feat_action->x_FindGeneForFeature(fh.GetLocation(), fh.GetScope(), CSeqFeatData::eSubtype_cdregion);  
            }
            if (cds_fh && cds_fh.IsSetProduct())
            {
                const CSeq_loc& prot_loc = cds_fh.GetProduct();
                CBioseq_Handle prot_bsh = cds_fh.GetScope().GetBioseqHandle(prot_loc);
                if (prot_bsh) 
                {
                    CFeat_CI prot_feat_ci(prot_bsh, SAnnotSelector(CSeqFeatData::eSubtype_prot));
                    if( prot_feat_ci)
                    {
                        CSeq_feat_Handle prot_fh = prot_feat_ci->GetSeq_feat_Handle();
                        if (prot_fh)
                        {
                            fh = prot_fh;
                        }
               
                    }
                }
            }
        }
        else
        {
            CSeq_feat_Handle gene_fh = feat_action->x_FindGeneForFeature(fh.GetLocation(), fh.GetScope(), feat_action->GetActionFeatSubtype());                  
            if (gene_fh)
                fh = gene_fh;
        }
    }
    return fh;
}

CEditingActionStringConstraint::CEditingActionStringConstraint(CRef<CConstraintMatcher>matcher) 
    : m_matcher(matcher)
{
}

bool CEditingActionStringConstraint::Match(const string &value)
{
    if (m_matcher && !m_matcher->IsEmpty())
        return m_matcher->DoesTextMatch(value) && CEditingActionConstraint::Match(value);
    return false;
}

bool CEditingActionStringConstraint::Match(CSeq_feat_Handle fh)
{
    SetFeat(fh);
    return CEditingActionConstraint::Match(fh);
}

bool CEditingActionStringConstraint::Match(const CSeqdesc* desc)
{
    SetDesc(desc);
    return CEditingActionConstraint::Match(desc);
}

CEditingActionSummaryConstraint::CEditingActionSummaryConstraint(const string &const_field, CFieldNamePanel::EFieldType const_field_type, int const_subtype, CRef<CConstraintMatcher>constraint)
{
    CSeq_entry_Handle seh; // bogus value
    m_action = CreateAction(seh, const_field, const_field_type, const_subtype);
    m_feat_action = dynamic_cast<IEditingActionFeat*>(m_action.GetPointer());
    m_src_action = dynamic_cast<IEditingActionBiosource*>(m_action.GetPointer());
    m_desc_action = dynamic_cast<IEditingActionDesc*>(m_action.GetPointer()); 
    m_string_constraint.Reset(new CEditingActionStringConstraint(constraint));
    CEditingActionStringConstraint &str_const = dynamic_cast<CEditingActionStringConstraint&>(*m_string_constraint);
    m_revert_constraint = str_const.GetNegation();
    str_const.SetNegation(false);
}

CEditingActionFeatureConstraint::CEditingActionFeatureConstraint(const string &const_field, CFieldNamePanel::EFieldType const_field_type, int const_subtype, CRef<CConstraintMatcher>constraint) 
    : CEditingActionSummaryConstraint(const_field, const_field_type, const_subtype, constraint) 
{  
}

bool CEditingActionFeatureConstraint::Match(CSeq_feat_Handle fh)
{    
    if (m_feat_action)
    {
        m_string_constraint->SetFeat(fh);
        m_feat_action->SetConstraint(m_string_constraint);
        m_feat_action->SetFeat(fh);
        bool result = m_feat_action->CheckValues();
        if (m_revert_constraint)
            result = !result;
        return result && CEditingActionConstraint::Match(fh);
    }   
    return false;
}

CEditingActionFeatureLocationConstraint::CEditingActionFeatureLocationConstraint(CRef<CLocationConstraintMatcher> loc_matcher) 
    : m_matcher(loc_matcher)
{  
}

bool CEditingActionFeatureLocationConstraint::Match(CSeq_feat_Handle fh)
{    
    bool result = m_matcher->Match(fh);
    return result && CEditingActionConstraint::Match(fh);
}

CEditingActionRelatedFeatureConstraint::CEditingActionRelatedFeatureConstraint(const string &const_field, CFieldNamePanel::EFieldType const_field_type, int const_subtype, CRef<CConstraintMatcher>constraint) 
    : CEditingActionSummaryConstraint(const_field, const_field_type, const_subtype, constraint)
{
}

bool CEditingActionRelatedFeatureConstraint::Match(CSeq_feat_Handle fh)
{    
    if (m_feat_action)
    {
        fh = ChangeFeat(m_feat_action, fh);
        m_string_constraint->SetFeat(fh);
        m_feat_action->SetConstraint(m_string_constraint);
        m_feat_action->SetFeat(fh);
        bool result = m_feat_action->CheckValues();
        if (m_revert_constraint)
            result = !result;
        return result && CEditingActionConstraint::Match(fh);
    }  
    return false;
}

CEditingActionDescriptorConstraint::CEditingActionDescriptorConstraint(const string &const_field, CFieldNamePanel::EFieldType const_field_type, int const_subtype, CRef<CConstraintMatcher>constraint) 
    : CEditingActionSummaryConstraint(const_field, const_field_type, const_subtype, constraint) 
{
}

bool CEditingActionDescriptorConstraint::Match(const CSeqdesc* desc)
{
    if (m_desc_action)
    {
        m_string_constraint->SetDesc(desc);
        m_desc_action->SetConstraint(m_string_constraint);
        m_desc_action->SetDesc(desc);
        bool result =  m_desc_action->CheckValues();
        if (m_revert_constraint)
            result = !result;
        return result && CEditingActionConstraint::Match(desc);
    }  
    return false;
}


CEditingActionBiosourceConstraint::CEditingActionBiosourceConstraint(const string &const_field, CFieldNamePanel::EFieldType const_field_type, int const_subtype, CRef<CConstraintMatcher>constraint) 
    : CEditingActionSummaryConstraint(const_field, const_field_type, const_subtype, constraint) 
{
}

bool CEditingActionBiosourceConstraint::Match(const CSeqdesc* desc)
{
  if (m_src_action)
    {
        m_string_constraint->SetDesc(desc);
        m_src_action->SetConstraint(m_string_constraint);
        m_src_action->SetDesc(desc);
        bool result =  m_src_action->CheckValues();
        if (m_revert_constraint)
            result = !result;
        return result && CEditingActionConstraint::Match(desc);
    }
    return false;
}

bool CEditingActionBiosourceConstraint::Match(CSeq_feat_Handle fh)
{    
   if (m_src_action)
    {
        m_string_constraint->SetFeat(fh);
        m_src_action->SetConstraint(m_string_constraint);
        m_src_action->SetFeat(fh);
        bool result = m_src_action->CheckValues();
        if (m_revert_constraint)
            result = !result;
        return result && CEditingActionConstraint::Match(fh);
    }
    return false;
}

CEditingActionBioseqConstraint::CEditingActionBioseqConstraint(const string &const_field, CFieldNamePanel::EFieldType const_field_type, int const_subtype, CRef<CConstraintMatcher>constraint)
    : CEditingActionSummaryConstraint(const_field, const_field_type, const_subtype, constraint) 
{
}

bool CEditingActionBioseqConstraint::Match(CBioseq_Handle bsh)
{
    if (m_action)
    {
        CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
        m_action->SetTopSeqEntry(seh);
        m_action->SetConstraint(m_string_constraint); 
        m_action->ResetChangedValues();
        m_action->NOOP();
        bool result = !m_action->GetChangedValues().empty();
        if (m_revert_constraint)
            result = !result;
        return result && CEditingActionConstraint::Match(bsh);
    }
    
        
    return false;
}

CSameConstraintMatcher::CSameConstraintMatcher(const string &field, CFieldNamePanel::EFieldType field_type, int subtype) 
: m_negate(false) 
{
    CSeq_entry_Handle seh; // bogus value
    m_action = CreateAction(seh, field, field_type, subtype);
    m_feat_action = dynamic_cast<IEditingActionFeat*>(m_action.GetPointer());
    m_src_action = dynamic_cast<IEditingActionBiosource*>(m_action.GetPointer());
    m_desc_action = dynamic_cast<IEditingActionDesc*>(m_action.GetPointer()); 
}

void CSameConstraintMatcher::SetFeat(CSeq_feat_Handle fh)
{
    m_values.clear();
    if (m_feat_action)
    {
        fh = ChangeFeat(m_feat_action, fh);
        CSeq_entry_Handle seh = fh.GetScope().GetBioseqHandle(fh.GetLocation()).GetSeq_entry_Handle();
        m_feat_action->SetTopSeqEntry(seh);
        m_feat_action->SetFeat(fh);
        const vector<string>& values = m_feat_action->GetValues();
        m_values.insert(values.begin(), values.end());
    }
    if (m_src_action)
    {
        CSeq_entry_Handle seh = fh.GetScope().GetBioseqHandle(fh.GetLocation()).GetSeq_entry_Handle();
        m_src_action->SetTopSeqEntry(seh);
        m_src_action->SetFeat(fh);
        const vector<string>& values = m_src_action->GetValues();
        m_values.insert(values.begin(), values.end());
    }
}

void CSameConstraintMatcher::SetDesc(const CSeqdesc* desc)
{
    m_values.clear();
    if (m_desc_action)
    {
        m_desc_action->SetDesc(desc);
        const vector<string>& values = m_desc_action->GetValues();
        m_values.insert(values.begin(), values.end());
    }
    if (m_src_action)
    {
        m_src_action->SetDesc(desc);
        const vector<string>& values = m_src_action->GetValues();
        m_values.insert(values.begin(), values.end());
    }
}

bool CLocationConstraintMatcher::IsEmpty() const
{
    return m_lc->IsEmpty();
}

bool CLocationConstraintMatcher::Match(CSeq_feat_Handle fh) 
{       
    bool result = true;
    if (!IsEmpty())
    {
        result = false;
        if (fh)
        {
            CScope &scope = fh.GetScope();
            CBioseq_Handle bsh = scope.GetBioseqHandle(fh.GetLocation());
            if (bsh)
            {
                CConstRef <CSeq_feat> feat_to;
                if (bsh.IsAa())
                    feat_to.Reset(sequence::GetCDSForProduct(bsh));
                result = m_lc->Match(*fh.GetOriginalSeq_feat(), feat_to, bsh.GetCompleteBioseq());
            }
        }
    }
    return result;
}

bool CPubStatusConstraintMatcher::Match(const CSeqdesc* obj) 
{       
    if (!IsEmpty())
    {
	return(CPubFieldType::GetStatusFromString(CPubField::GetStatus(*obj)) == m_status);
	
    }
    return true;
}

CEditingActionPubStatusConstraint::CEditingActionPubStatusConstraint(CRef<CPubStatusConstraintMatcher> matcher) 
    : m_matcher(matcher)
{  
}

bool CEditingActionPubStatusConstraint::Match(const CSeqdesc* obj)
{    
    bool result = m_matcher->Match(obj);
    return result && CEditingActionConstraint::Match(obj);
}

END_NCBI_SCOPE
