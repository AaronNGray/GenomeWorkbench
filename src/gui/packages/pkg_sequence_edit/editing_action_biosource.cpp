/*  $Id: editing_action_biosource.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
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
#include <objects/seq/Seq_data.hpp>
#include <objmgr/seq_vector.hpp>
#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_biosource.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


IEditingActionBiosource::IEditingActionBiosource(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature, const string &name)
    : IEditingAction(seh, name), m_is_descriptor(is_descriptor), m_is_feature(is_feature), m_EditedBiosource(NULL), m_Desc(NULL)
{
  
}

void IEditingActionBiosource::SetFeat(CSeq_feat_Handle fh)
{
    m_EditedFeat.Reset(new CSeq_feat);
    m_EditedFeat->Assign(*fh.GetOriginalSeq_feat());
    m_EditedBiosource = &m_EditedFeat->SetData().SetBiosrc();
}

void IEditingActionBiosource::SetDesc(const CSeqdesc* desc)
{
    m_EditedDesc.Reset(new CSeqdesc);
    m_EditedDesc->Assign(*desc);
    m_EditedBiosource = &m_EditedDesc->SetSource();
}
 
void IEditingActionBiosource::Find(EActionType action)
{
    if (!m_TopSeqEntry)
        return;
    size_t count = 0;
    for (CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na); b_iter; ++b_iter)
    {
        m_EditedBiosource = NULL;
        m_EditedFeat.Reset();
        m_EditedDesc.Reset();
        CBioseq_Handle bsh = *b_iter; 
        m_CurrentSeqEntry = bsh.GetSeq_entry_Handle();
        if (m_constraint->Match(bsh))
        {     
            if (m_is_descriptor)
            {
                bool found = false;
                CSeq_entry_Handle found_seh;
                for (CSeqdesc_CI desc_ci( bsh, CSeqdesc::e_Source); desc_ci; ++desc_ci) 
                {
                    found = true;
                    m_EditedDesc.Reset();
                    m_Desc = &*desc_ci;
                    //m_CurrentSeqEntry = desc_ci.GetSeq_entry_Handle();
                    CSeq_entry_Handle seh = desc_ci.GetSeq_entry_Handle();
                    if (IsFrom(action) && found_seh && seh != found_seh)
                        break;
                    if (m_constraint->Match(m_Desc))
                    {
                        if (m_ChangedDescriptors.find(m_Desc) != m_ChangedDescriptors.end())
                            m_EditedDesc = m_ChangedDescriptors[m_Desc];
                        else
                        {
                            m_EditedDesc.Reset(new CSeqdesc);
                            m_EditedDesc->Assign(*m_Desc);
                        }
                        m_EditedBiosource = &m_EditedDesc->SetSource();
                        m_DescContext = seh;
                        Modify(action);                       
                        found_seh = seh;
                    }
                }
                if (!found && !IsNOOP(action))
                {
                    m_EditedDesc.Reset(new CSeqdesc);
                    m_Desc = m_EditedDesc.GetPointer();
                    CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
                    if (m_constraint->Match(m_Desc))
                    {
                        m_CreatedDescriptors[m_Desc] = seh;
                        m_EditedBiosource = &m_EditedDesc->SetSource();
                        Modify(action);
                    }
                }
            }

            m_EditedDesc.Reset();
            if (m_is_feature)
            {
                for (CFeat_CI feat_ci(bsh, CSeqFeatData::e_Biosrc); feat_ci; ++feat_ci) 
                {
                    m_EditedFeat.Reset();
                    m_Feat = feat_ci->GetSeq_feat_Handle();
                    if (m_constraint->Match(m_Feat))
                    {
                        if (m_ChangedFeatures.find(m_Feat) != m_ChangedFeatures.end())
                            m_EditedFeat = m_ChangedFeatures[m_Feat];
                        else
                        {
                            m_EditedFeat.Reset(new CSeq_feat);
                            m_EditedFeat->Assign(*m_Feat.GetOriginalSeq_feat());
                        }
                        m_EditedBiosource = &m_EditedFeat->SetData().SetBiosrc();
                        Modify(action);                       
                    }
                }
            }
        }
        ++count;
        if (count >= m_max_records)
            break;
    }
// TODO create if none found?
}

void IEditingActionBiosource::FindRelated(EActionType action)
{
    Find(action); 
}

void IEditingActionBiosource::Modify(EActionType action)
{
    Action(action);
    if (m_modified)
    {
        if (m_EditedDesc) {
            m_ChangedDescriptors[m_Desc] = m_EditedDesc;
            m_ContextForDescriptors[m_Desc] = m_DescContext;
        }
        if (m_EditedFeat)
            m_ChangedFeatures[m_Feat] = m_EditedFeat;
    }
}

bool IEditingActionBiosource::SameObject()
{  
    IEditingActionBiosource *other = dynamic_cast<IEditingActionBiosource*>(m_Other);
    if (other)
        return true;
    return false;
}

void IEditingActionBiosource::SwapContext(IEditingAction* source)
{
    IEditingActionBiosource *src_source = dynamic_cast<IEditingActionBiosource*>(source);
    if (src_source)
    {
        swap(m_EditedDesc, src_source->m_EditedDesc);
        swap(m_EditedFeat, src_source->m_EditedFeat);
        swap(m_EditedBiosource, src_source->m_EditedBiosource);
        swap(m_Desc, src_source->m_Desc);
        swap(m_Feat, src_source->m_Feat);
        swap(m_DescContext, src_source->m_DescContext);
    }
    IEditingAction::SwapContext(source);
}

CEditingActionBiosourceTaxname::CEditingActionBiosourceTaxname(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature)
    : IEditingActionBiosource(seh, is_descriptor, is_feature, "CEditingActionBiosourceTaxname")
{
}

bool CEditingActionBiosourceTaxname::IsSetValue()
{
    return m_EditedBiosource && m_EditedBiosource->IsSetOrg() && m_EditedBiosource->GetOrg().IsSetTaxname();
}

void CEditingActionBiosourceTaxname::SetValue(const string &value)
{
    m_EditedBiosource->SetOrg().SetTaxname(value);
}

string CEditingActionBiosourceTaxname::GetValue()
{
    return m_EditedBiosource->GetOrg().GetTaxname();
}

void CEditingActionBiosourceTaxname::ResetValue()
{
    m_EditedBiosource->SetOrg().ResetTaxname();
}

CEditingActionBiosourceTaxnameAfterBinomial::CEditingActionBiosourceTaxnameAfterBinomial(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature)
    : IEditingActionBiosource(seh, is_descriptor, is_feature, "CEditingActionBiosourceTaxnameAfterBinomial")
{
}

static const char* nomial_keywords[] = {
"f. sp. ",
"var",
"pv.",
"bv.",
"serovar",
"subsp.",
NULL
};

string CEditingActionBiosourceTaxnameAfterBinomial::x_GetTextAfterNomial(const string& taxname) const
{
    if (NStr::IsBlank(taxname)) {
        return kEmptyStr;
    }

    SIZE_TYPE pos = NStr::FindNoCase(taxname, " ");
    vector<string> names;
    if (pos != NPOS) {
        NStr::Split(taxname, " ", names, NStr::fSplit_Tokenize);
    }

    if (names.empty()) {
        return kEmptyStr;
    }

    pos = 0;
    if (NStr::EqualNocase(names[0], "uncultured")) {
        pos = 2; // skipping the first three words
    } else {
        pos = 1; // skipping the first two words
    }

    if (names.size() <= pos + 1) {
        return kEmptyStr;
    }
    pos++;
    bool found_keyword = true;
    while (pos < names.size() && found_keyword) {
        /* if the next word is a nomial keyword, skip that plus the first word that follows it */
        found_keyword = false;
        if (NStr::EqualCase(names[pos], "f.") && (pos+1 < names.size() && NStr::EqualCase(names[pos+1],"sp."))) {
            pos = pos + 3;
            found_keyword = true;
        } else {
            size_t n = 1;
            while (nomial_keywords[n] && !found_keyword) {
                if (NStr::EqualCase(nomial_keywords[n], names[pos])) {
                    pos = pos + 2;
                    found_keyword = true;
                }
                ++n;
            }
        }
    }
    string taxname_after_binomial = kEmptyStr;
    while (pos < names.size()) {
        taxname_after_binomial += names[pos] + " ";
        pos++;
    }
    NStr::TruncateSpacesInPlace(taxname_after_binomial, NStr::eTrunc_End);
    return taxname_after_binomial;
}

bool CEditingActionBiosourceTaxnameAfterBinomial::IsSetValue()
{
    string taxname;
    if (m_EditedBiosource && m_EditedBiosource->IsSetOrg() && m_EditedBiosource->GetOrg().IsSetTaxname())
        taxname = x_GetTextAfterNomial(m_EditedBiosource->GetOrg().GetTaxname());
    return !taxname.empty();
}

void CEditingActionBiosourceTaxnameAfterBinomial::SetValue(const string &value)
{
    if (IsSetValue())
    {
        string taxname = GetValue();
        NStr::ReplaceInPlace(m_EditedBiosource->SetOrg().SetTaxname(), taxname, value);
    }
    else
    {
        m_EditedBiosource->SetOrg().SetTaxname() += value;
    }
}

string CEditingActionBiosourceTaxnameAfterBinomial::GetValue()
{
    return x_GetTextAfterNomial(m_EditedBiosource->GetOrg().GetTaxname());
}

void CEditingActionBiosourceTaxnameAfterBinomial::ResetValue()
{
    string taxname = GetValue();
    NStr::ReplaceInPlace(m_EditedBiosource->SetOrg().SetTaxname(), taxname, kEmptyStr);
}

CEditingActionBiosourceCommonName::CEditingActionBiosourceCommonName(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature)
    : IEditingActionBiosource(seh, is_descriptor, is_feature, "CEditingActionBiosourceCommonName")
{
}

bool CEditingActionBiosourceCommonName::IsSetValue()
{
    return m_EditedBiosource  && m_EditedBiosource->IsSetOrg() && m_EditedBiosource->GetOrg().IsSetCommon();
}

void CEditingActionBiosourceCommonName::SetValue(const string &value)
{
    m_EditedBiosource->SetOrg().SetCommon(value);
}

string CEditingActionBiosourceCommonName::GetValue()
{
    return m_EditedBiosource->GetOrg().GetCommon();
}

void CEditingActionBiosourceCommonName::ResetValue()
{
    m_EditedBiosource->SetOrg().ResetCommon();
}

CEditingActionBiosourceDivision::CEditingActionBiosourceDivision(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature)
    : IEditingActionBiosource(seh, is_descriptor, is_feature, "CEditingActionBiosourceDivision")
{
}

bool CEditingActionBiosourceDivision::IsSetValue()
{
    return m_EditedBiosource  && m_EditedBiosource->IsSetOrg() && m_EditedBiosource->GetOrg().IsSetOrgname()
        && m_EditedBiosource->GetOrg().GetOrgname().IsSetDiv();
}

void CEditingActionBiosourceDivision::SetValue(const string &value)
{
    m_EditedBiosource->SetOrg().SetOrgname().SetDiv(value);
}

string CEditingActionBiosourceDivision::GetValue()
{
    return m_EditedBiosource->GetOrg().GetOrgname().GetDiv();
}

void CEditingActionBiosourceDivision::ResetValue()
{
    m_EditedBiosource->SetOrg().SetOrgname().ResetDiv();
}

CEditingActionBiosourceLineage::CEditingActionBiosourceLineage(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature)
    : IEditingActionBiosource(seh, is_descriptor, is_feature, "CEditingActionBiosourceLineage")
{
}

bool CEditingActionBiosourceLineage::IsSetValue()
{
    return m_EditedBiosource  && m_EditedBiosource->IsSetOrg() && m_EditedBiosource->GetOrg().IsSetOrgname()
        && m_EditedBiosource->GetOrg().GetOrgname().IsSetLineage();
}

void CEditingActionBiosourceLineage::SetValue(const string &value)
{
    m_EditedBiosource->SetOrg().SetOrgname().SetLineage(value);
}

string CEditingActionBiosourceLineage::GetValue()
{
    return m_EditedBiosource->GetOrg().GetOrgname().GetLineage();
}

void CEditingActionBiosourceLineage::ResetValue()
{
    m_EditedBiosource->SetOrg().SetOrgname().ResetLineage();
}


CEditingActionBiosourceLocation::CEditingActionBiosourceLocation(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature)
    : IEditingActionBiosource(seh, is_descriptor, is_feature, "CEditingActionBiosourceLocation")
{
}

bool CEditingActionBiosourceLocation::IsSetValue()
{
    return m_EditedBiosource  && m_EditedBiosource->IsSetGenome();
}

void CEditingActionBiosourceLocation::SetValue(const string &value)
{
    if (CBioSource::ENUM_METHOD_NAME(EGenome)()->IsValidName(value))
    {
        m_EditedBiosource->SetGenome(static_cast<CBioSource::EGenome>(CBioSource::ENUM_METHOD_NAME(EGenome)()->FindValue(value)));
    }
}

string CEditingActionBiosourceLocation::GetValue()
{
    return CBioSource::ENUM_METHOD_NAME(EGenome)()->FindName(m_EditedBiosource->GetGenome(), true);
}

void CEditingActionBiosourceLocation::ResetValue()
{
    m_EditedBiosource->ResetGenome();
}


CEditingActionBiosourceOrigin::CEditingActionBiosourceOrigin(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature)
    : IEditingActionBiosource(seh, is_descriptor, is_feature, "CEditingActionBiosourceOrigin")
{
}

bool CEditingActionBiosourceOrigin::IsSetValue()
{
    return m_EditedBiosource  && m_EditedBiosource->IsSetOrigin();
}

void CEditingActionBiosourceOrigin::SetValue(const string &value)
{
    if (CBioSource::ENUM_METHOD_NAME(EOrigin)()->IsValidName(value))
    {
        m_EditedBiosource->SetOrigin(static_cast<CBioSource::EOrigin>(CBioSource::ENUM_METHOD_NAME(EOrigin)()->FindValue(value)));
    }
}

string CEditingActionBiosourceOrigin::GetValue()
{
    return CBioSource::ENUM_METHOD_NAME(EOrigin)()->FindName(m_EditedBiosource->GetOrigin(), true);
}

void CEditingActionBiosourceOrigin::ResetValue()
{
    m_EditedBiosource->ResetOrigin();
}

CEditingActionBiosourcePrimers::CEditingActionBiosourcePrimers(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature, bool is_fwd, bool is_rev, bool is_name, bool is_seq)
    : IEditingActionBiosource(seh, is_descriptor, is_feature, "CEditingActionBiosourcePrimers"), m_is_fwd(is_fwd), m_is_rev(is_rev), m_is_name(is_name), m_is_seq(is_seq), m_val(NULL)
{
}

void CEditingActionBiosourcePrimers::Modify(EActionType action)
{
    bool found = false;
    if (m_EditedBiosource->IsSetPcr_primers())
    {
        EDIT_EACH_PCRREACTION_IN_PCRREACTIONSET(qual_it, m_EditedBiosource->SetPcr_primers())
        {
            if (!IsCreateNew(action))
            {
                m_reaction.Reset();
                if (m_is_fwd && (*qual_it)->IsSetForward())
                {
                    EDIT_EACH_PCRPRIMER_IN_PCRPRIMERSET(primer_it, (*qual_it)->SetForward())
                    {
			m_primer.Reset();
                        if (m_is_name && (*primer_it)->IsSetName())
                        {
                            found = true;
                            m_val = &(*primer_it)->SetName().Set();
                            IEditingActionBiosource::Modify(action);
                        }
                        if (m_is_seq && (*primer_it)->IsSetSeq())
                        {
                            found = true;
                            m_val = &(*primer_it)->SetSeq().Set();
                            IEditingActionBiosource::Modify(action);
                        }
			if (!found)
			{
			    found = true;
			    m_val = NULL;
			    m_primer = *primer_it;
			    IEditingActionBiosource::Modify(action);
			}
                        if ( (!(*primer_it)->IsSetName() || (*primer_it)->GetName().Get().empty()) &&
                             (!(*primer_it)->IsSetSeq() || (*primer_it)->GetSeq().Get().empty()) )
                            ERASE_PCRPRIMER_IN_PCRPRIMERSET(primer_it, (*qual_it)->SetForward());
                    }
                }
                if (m_is_rev && (*qual_it)->IsSetReverse())
                {
                    EDIT_EACH_PCRPRIMER_IN_PCRPRIMERSET(primer_it, (*qual_it)->SetReverse())
                    {
			m_primer.Reset();
                        if (m_is_name && (*primer_it)->IsSetName())
                        {
                            found = true;
                            m_val = &(*primer_it)->SetName().Set();
                            IEditingActionBiosource::Modify(action);
                        }
                        if (m_is_seq && (*primer_it)->IsSetSeq())
                        {
                            found = true;
                            m_val = &(*primer_it)->SetSeq().Set();
                            IEditingActionBiosource::Modify(action);
                        }
			if (!found)
			{
			    found = true;
			    m_val = NULL;
			    m_primer = *primer_it;
			    IEditingActionBiosource::Modify(action);
			}
                        if ( (!(*primer_it)->IsSetName() || (*primer_it)->GetName().Get().empty()) &&
                             (!(*primer_it)->IsSetSeq() || (*primer_it)->GetSeq().Get().empty()) )
                            ERASE_PCRPRIMER_IN_PCRPRIMERSET(primer_it, (*qual_it)->SetReverse());
                    }
                }
            }
            if (!found)
            {
                found = true;
                m_val = NULL;
		m_primer.Reset();
                m_reaction = *qual_it;
                IEditingActionBiosource::Modify(action);
            }
            if ( (!(*qual_it)->IsSetForward() || !(*qual_it)->GetForward().IsSet() || (*qual_it)->GetForward().Get().empty()) &&
                 (!(*qual_it)->IsSetReverse() || !(*qual_it)->GetReverse().IsSet() || (*qual_it)->GetReverse().Get().empty()) )
                ERASE_PCRREACTION_IN_PCRREACTIONSET(qual_it, m_EditedBiosource->SetPcr_primers());
        }
    }
    if (!found)
    {
        m_val = NULL;
	m_primer.Reset();
        m_reaction.Reset();
        IEditingActionBiosource::Modify(action);
    }
    if (m_EditedBiosource->IsSetPcr_primers() &&  m_EditedBiosource->GetPcr_primers().IsSet() &&  m_EditedBiosource->GetPcr_primers().Get().empty())
	m_EditedBiosource->ResetPcr_primers();
}

bool CEditingActionBiosourcePrimers::IsSetValue()
{
    return m_val != NULL;
}

void CEditingActionBiosourcePrimers::SetValue(const string &value)
{
    if (m_val)
    {
        *m_val = value;
    }
    else if ( m_primer)
    {
	if (m_is_name)
        {
            m_primer->SetName(CPCRPrimerName(value));
        }
        if (m_is_seq)
        {
            m_primer->SetSeq(CPCRPrimerSeq(value));
        }
    }
    else if (m_reaction)
    {
        CRef< CPCRPrimer > primer(new CPCRPrimer);
        if (m_is_name)
        {
            primer->SetName(CPCRPrimerName(value));
        }
        if (m_is_seq)
        {
            primer->SetSeq(CPCRPrimerSeq(value));
        }
        if (m_is_fwd)
        {
            m_reaction->SetForward().Set().push_back(primer);
        }
        if (m_is_rev)
        {
            m_reaction->SetReverse().Set().push_back(primer);
        }
    }
    else 
    {
        CRef< CPCRReaction > reaction(new CPCRReaction);
        CRef< CPCRPrimer > primer(new CPCRPrimer);
        if (m_is_name)
        {
            primer->SetName(CPCRPrimerName(value));
        }
        if (m_is_seq)
        {
            primer->SetSeq(CPCRPrimerSeq(value));
        }
        if (m_is_fwd)
        {
            reaction->SetForward().Set().push_back(primer);
        }
        if (m_is_rev)
        {
            reaction->SetReverse().Set().push_back(primer);
        }
        m_EditedBiosource->SetPcr_primers().Set().push_back(reaction);
    }
}

string CEditingActionBiosourcePrimers::GetValue()
{
    if (m_val)
        return *m_val;
    return kEmptyStr;
}

void CEditingActionBiosourcePrimers::ResetValue()
{
    m_val->clear();
}



CEditingActionBiosourceOrgMod::CEditingActionBiosourceOrgMod(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature, const COrgMod::ESubtype subtype, const string &name)
    : IEditingActionBiosource(seh, is_descriptor, is_feature, name), m_qual(subtype), m_erase(false)
{
}

void CEditingActionBiosourceOrgMod::Modify(EActionType action)
{
    bool found = false;
    if (!IsCreateNew(action))
    {
        EDIT_EACH_ORGMOD_ON_BIOSOURCE(qual_it, *m_EditedBiosource)
        {
            if ((*qual_it)->IsSetSubtype() && (*qual_it)->GetSubtype() == m_qual)
            {
                found = true;
                m_erase = false;
                m_OrgMod = *qual_it;
                IEditingActionBiosource::Modify(action);
                if (m_erase)
                    ERASE_ORGMOD_ON_BIOSOURCE(qual_it, *m_EditedBiosource);
            }
        }
    }
    if (!found)
    {
        m_OrgMod.Reset();
        m_erase = false;
        IEditingActionBiosource::Modify(action);
    }
}

bool CEditingActionBiosourceOrgMod::IsSetValue()
{
    return m_OrgMod && m_OrgMod->IsSetSubname();
}

void CEditingActionBiosourceOrgMod::SetValue(const string &value)
{
    if (m_OrgMod)
        m_OrgMod->SetSubname(value);
    else
    {
        CRef<COrgMod> new_mod(new COrgMod);
        new_mod->SetSubtype(m_qual);
        new_mod->SetSubname(value);
        m_EditedBiosource->SetOrg().SetOrgname().SetMod().push_back(new_mod);
    }
}

string CEditingActionBiosourceOrgMod::GetValue()
{
    return m_OrgMod->GetSubname();
}

void CEditingActionBiosourceOrgMod::ResetValue()
{
    m_erase = true;
}

CEditingActionBiosourceSubSource::CEditingActionBiosourceSubSource(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature, const CSubSource::ESubtype subtype, const string &name)
    : IEditingActionBiosource(seh, is_descriptor, is_feature, name), m_qual(subtype), m_erase(false)
{
}

void CEditingActionBiosourceSubSource::Modify(EActionType action)
{
    bool found = false;
    if (!IsCreateNew(action))
    {
        EDIT_EACH_SUBSOURCE_ON_BIOSOURCE(qual_it, *m_EditedBiosource)
        {
            if ((*qual_it)->IsSetSubtype() && (*qual_it)->GetSubtype() == m_qual)
            {
                found = true;
                m_erase = false;
                m_SubSource = *qual_it;
                IEditingActionBiosource::Modify(action);
                if (m_erase)
                    ERASE_SUBSOURCE_ON_BIOSOURCE(qual_it, *m_EditedBiosource);
            }
        }
    }
    if (!found)
    {
        m_SubSource.Reset();
        m_erase = false;
        IEditingActionBiosource::Modify(action);
    }
}

bool CEditingActionBiosourceSubSource::IsSetValue()
{
    return m_SubSource && m_SubSource->IsSetName();
}

void CEditingActionBiosourceSubSource::SetValue(const string &value)
{
    if (m_SubSource)
        m_SubSource->SetName(value);
    else
    {
        CRef<CSubSource> new_mod(new CSubSource);
        new_mod->SetSubtype(m_qual);
        new_mod->SetName(value);
        m_EditedBiosource->SetSubtype().push_back(new_mod);
    }
}

string CEditingActionBiosourceSubSource::GetValue()
{
    return m_SubSource->GetName();
}

void CEditingActionBiosourceSubSource::ResetValue()
{
    m_erase = true;
}


CEditingActionBiosourceAllNotes::CEditingActionBiosourceAllNotes(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature)
    : IEditingActionBiosource(seh, is_descriptor, is_feature, "CEditingActionBiosourceAllNotes"),  m_erase(false)
{
}

void CEditingActionBiosourceAllNotes::Modify(EActionType action)
{
    bool found = false;
    EDIT_EACH_ORGMOD_ON_BIOSOURCE(qual_it, *m_EditedBiosource)
    {
        if ((*qual_it)->IsSetSubtype() && (*qual_it)->GetSubtype() == COrgMod::eSubtype_other)
        {
            m_erase = false;
            m_OrgMod = *qual_it;
            IEditingActionBiosource::Modify(action);
            if (m_erase)
                ERASE_ORGMOD_ON_BIOSOURCE(qual_it, *m_EditedBiosource);
        }
    }
    m_OrgMod.Reset();
    EDIT_EACH_SUBSOURCE_ON_BIOSOURCE(qual_it, *m_EditedBiosource)
    {
        if ((*qual_it)->IsSetSubtype() && (*qual_it)->GetSubtype() == CSubSource::eSubtype_other)
        {
            m_erase = false;
            m_SubSource = *qual_it;
            IEditingActionBiosource::Modify(action);
            if (m_erase)
                ERASE_SUBSOURCE_ON_BIOSOURCE(qual_it, *m_EditedBiosource);
        }
    }   
}

bool CEditingActionBiosourceAllNotes::IsSetValue()
{
    return (m_OrgMod && m_OrgMod->IsSetSubname()) || (m_SubSource && m_SubSource->IsSetName());
}

void CEditingActionBiosourceAllNotes::SetValue(const string &value)
{
    if (m_OrgMod)
        m_OrgMod->SetSubname(value);
    if (m_SubSource)
        m_SubSource->SetName(value);
}

string CEditingActionBiosourceAllNotes::GetValue()
{
    if (m_OrgMod)
        return m_OrgMod->GetSubname();
    if (m_SubSource)
        return m_SubSource->GetName();
    return kEmptyStr;
}

void CEditingActionBiosourceAllNotes::ResetValue()
{
    m_erase = true;
}


CEditingActionBiosourceStructuredVoucher::CEditingActionBiosourceStructuredVoucher(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature, const COrgMod::ESubtype subtype, const int voucher_type)
    : CEditingActionBiosourceOrgMod(seh, is_descriptor, is_feature, subtype, "CEditingActionBiosourceStructuredVoucher"), m_voucher_type(voucher_type)
{
}

bool CEditingActionBiosourceStructuredVoucher::IsSetValue()
{
    bool res = false;
    if (m_OrgMod && m_OrgMod->IsSetSubname())
    {
        string str = m_OrgMod->GetSubname();
        string inst, coll, id;
        Parse(str, inst, coll, id);
        switch (m_voucher_type)
        {
        case eVoucher_inst :  res = !inst.empty(); break;
        case eVoucher_coll : res = !coll.empty(); break;
        case eVoucher_id : res = !id.empty(); break;
        default: break;
        }
    }
    return res;
}

void CEditingActionBiosourceStructuredVoucher::Parse(const string &str, string &inst, string &coll, string &id)
{
    vector<string> values;
    NStr::Split(str, ":", values);
    if (values.size() == 3)
    {
        inst = values[0];
        coll = values[1];
        id = values[2];
    }
    else if (values.size() == 2)
    {
        inst = values[0];
        id = values[1];
    }
    else if (values.size() == 1)
    {
        id = values[0];
    }
}

void CEditingActionBiosourceStructuredVoucher::SetValue(const string &value)
{
    if (m_OrgMod)
    {
        string inst, coll, id;
        if (m_OrgMod->IsSetSubname())
        {
            string str = m_OrgMod->GetSubname();
            Parse(str, inst, coll, id);
        }
        switch (m_voucher_type)
        {
        case eVoucher_inst :  inst = value; break;
        case eVoucher_coll : coll = value; break;
        case eVoucher_id : id = value; break;
        default: break;
        }
        string str = COrgMod::MakeStructuredVoucher(inst, coll, id);
        m_OrgMod->SetSubname(str);
    }
    else
    {
        CRef<COrgMod> new_mod(new COrgMod);
        new_mod->SetSubtype(m_qual);
        string inst, coll, id;
        switch (m_voucher_type)
        {
        case eVoucher_inst :  inst = value; break;
        case eVoucher_coll : coll = value; break;
        case eVoucher_id : id = value; break;
        default: break;
        }
        string str = COrgMod::MakeStructuredVoucher(inst, coll, id);

        new_mod->SetSubname(str);
        m_EditedBiosource->SetOrg().SetOrgname().SetMod().push_back(new_mod);
    }
}

string CEditingActionBiosourceStructuredVoucher::GetValue()
{
    string res;
    string str = m_OrgMod->GetSubname();
    string inst, coll, id;
    Parse(str, inst, coll, id);
    switch (m_voucher_type)
    {
    case eVoucher_inst :  res = inst; break;
    case eVoucher_coll : res = coll; break;
    case eVoucher_id : res = id; break;
    default: break;
    }
    return res;
}

void CEditingActionBiosourceStructuredVoucher::ResetValue()
{
    if (m_OrgMod)
    {
        string inst, coll, id;
        if (m_OrgMod->IsSetSubname())
        {
            string str = m_OrgMod->GetSubname();
            Parse(str, inst, coll, id);
        }
        switch (m_voucher_type)
        {
        case eVoucher_inst :  inst.clear(); break;
        case eVoucher_coll : coll.clear(); break;
        case eVoucher_id : id.clear(); break;
        default: break;
        }
        string str = COrgMod::MakeStructuredVoucher(inst, coll, id);
        m_OrgMod->SetSubname(str);
    }  
}

CEditingActionBiosourceDbxref::CEditingActionBiosourceDbxref(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature)
    : IEditingActionBiosource(seh, is_descriptor, is_feature, "CEditingActionBiosourceDbxrefs"), m_erase(false)
{
}


void CEditingActionBiosourceDbxref::Modify(EActionType action)
{
    bool found = false;
    if (IsFrom(action) && 
        m_EditedBiosource->IsSetOrg() && m_EditedBiosource->GetOrg().IsSetDb() && !m_EditedBiosource->GetOrg().GetDb().empty() 
        && m_EditedBiosource->GetOrg().GetDb().capacity() < 2 * m_EditedBiosource->GetOrg().GetDb().size())
    {
        m_EditedBiosource->SetOrg().SetDb().reserve(2 * m_EditedBiosource->GetOrg().GetDb().size());
    }
    if (!IsCreateNew(action))
    {
        EDIT_EACH_DBXREF_ON_ORGREF(dbxref_it, m_EditedBiosource->SetOrg())
        {
            found = true;
            m_erase = false;
            m_Dbtag = *dbxref_it;
            IEditingActionBiosource::Modify(action);
            if (m_erase)
                ERASE_DBXREF_ON_ORGREF(dbxref_it, m_EditedBiosource->SetOrg());
        }
    }

    if (!found)
    {
        m_Dbtag.Reset();
        m_erase = false;
        IEditingActionBiosource::Modify(action);
    }
    if (IsFrom(action) && m_EditedBiosource->IsSetOrg() && m_EditedBiosource->GetOrg().IsSetDb())
    {
        m_EditedBiosource->SetOrg().SetDb().shrink_to_fit();
    }
    if (m_EditedBiosource->IsSetOrg() && m_EditedBiosource->GetOrg().IsSetDb() && m_EditedBiosource->GetOrg().GetDb().empty())
    {
        m_EditedBiosource->SetOrg().ResetDb();
    }
}

bool CEditingActionBiosourceDbxref::IsSetValue()
{
    return m_Dbtag && m_Dbtag->IsSetDb() && m_Dbtag->IsSetTag();
}

void CEditingActionBiosourceDbxref::SetValue(const string &value)
{
    string db, tag;
    NStr::SplitInTwo(value, ":", db, tag);
    int id = NStr::StringToInt(value, NStr::fConvErr_NoThrow);

    if (!db.empty() && !tag.empty())
    {
        if (m_Dbtag)
        {
            m_Dbtag->SetDb(db);
            m_Dbtag->ResetTag();
            if (id != 0)
                m_Dbtag->SetTag().SetId(id);
            else
                m_Dbtag->SetTag().SetStr(tag);
        }
        else
        {
            CRef<CDbtag> dbtag(new CDbtag);
            dbtag->SetDb(db);
            if (id != 0)
                dbtag->SetTag().SetId(id);
            else
                dbtag->SetTag().SetStr(tag);
            m_EditedBiosource->SetOrg().SetDb().push_back(dbtag);
        }
    }
}

string CEditingActionBiosourceDbxref::GetValue()
{
    string db = m_Dbtag->GetDb();
    string tag;
    if (m_Dbtag->GetTag().IsStr())
        tag = m_Dbtag->GetTag().GetStr();
    if (m_Dbtag->GetTag().IsId())
        tag = NStr::IntToString(m_Dbtag->GetTag().GetId());
    return db + ":" + tag;
}

void CEditingActionBiosourceDbxref::ResetValue()
{
    m_erase = true;
}


IEditingActionBiosource* CreateActionBiosource(CSeq_entry_Handle seh, string field)
{
    bool is_descriptor = true;
    bool is_feature = true;
    if (NStr::EndsWith(field, " descriptor"))
    {
        is_feature = false;
        NStr::ReplaceInPlace(field, " descriptor", kEmptyStr);
    }
    else  if (NStr::EndsWith(field, " feature"))
    {
        is_descriptor = false;
        NStr::ReplaceInPlace(field, " feature", kEmptyStr);
    }
    if (NStr::EqualNocase(field, "taxname") || field == "organism name" || field == "org")
        return new CEditingActionBiosourceTaxname(seh, is_descriptor, is_feature);
    if (field == "Taxname after Binomial")
        return new CEditingActionBiosourceTaxnameAfterBinomial(seh, is_descriptor, is_feature);
    if (NStr::EqualNocase(field, "Dbxref"))
        return new CEditingActionBiosourceDbxref(seh, is_descriptor, is_feature);
    if (field == "common name" || field == "common-name")
        return new CEditingActionBiosourceCommonName(seh, is_descriptor, is_feature);
    if (field == "division")
        return new CEditingActionBiosourceDivision(seh, is_descriptor, is_feature);
    if (field == "lineage")
        return new CEditingActionBiosourceLineage(seh, is_descriptor, is_feature);
    if (field == "genome")
        return new CEditingActionBiosourceLocation(seh, is_descriptor, is_feature);
    if (field == "origin")
        return new CEditingActionBiosourceOrigin(seh, is_descriptor, is_feature);
    if (field == "fwd-primer-name")
        return new CEditingActionBiosourcePrimers(seh, is_descriptor, is_feature, true, false, true, false);
    if (field == "fwd-primer-seq")
        return new CEditingActionBiosourcePrimers(seh, is_descriptor, is_feature, true, false, false, true);
    if (field == "rev-primer-name")
        return new CEditingActionBiosourcePrimers(seh, is_descriptor, is_feature, false, true, true, false);
    if (field == "rev-primer-seq")
        return new CEditingActionBiosourcePrimers(seh, is_descriptor, is_feature, false, true, false, true);
    if (field == "host")
        return new CEditingActionBiosourceOrgMod(seh, is_descriptor, is_feature, COrgMod::eSubtype_nat_host);   
    if (field == "note-orgmod" || field == "orgmod-note")
        return new CEditingActionBiosourceOrgMod(seh, is_descriptor, is_feature, COrgMod::eSubtype_other);   
    if (field == "note-subsource" || field == "subsource-note" || field == "note-subsrc" || field == "subsrc-note")
        return new CEditingActionBiosourceSubSource(seh, is_descriptor, is_feature, CSubSource::eSubtype_other);
    if (field  == "All primers")
        return new CEditingActionBiosourcePrimers(seh, is_descriptor, is_feature, true, true, true, true);
    if (field == "All notes" || field == "all-notes" || field == "note")
        return new CEditingActionBiosourceAllNotes(seh, is_descriptor, is_feature);
    if (field == "bio-material-coll")
        return new CEditingActionBiosourceStructuredVoucher(seh, is_descriptor, is_feature, COrgMod::eSubtype_bio_material, CEditingActionBiosourceStructuredVoucher::eVoucher_coll);
    if (field == "bio-material-inst")
        return new CEditingActionBiosourceStructuredVoucher(seh, is_descriptor, is_feature, COrgMod::eSubtype_bio_material, CEditingActionBiosourceStructuredVoucher::eVoucher_inst);
    if (field == "bio-material-specid")
        return new CEditingActionBiosourceStructuredVoucher(seh, is_descriptor, is_feature, COrgMod::eSubtype_bio_material, CEditingActionBiosourceStructuredVoucher::eVoucher_id);
    if (field == "culture-collection-coll")
        return new CEditingActionBiosourceStructuredVoucher(seh, is_descriptor, is_feature, COrgMod::eSubtype_culture_collection, CEditingActionBiosourceStructuredVoucher::eVoucher_coll);
    if (field == "culture-collection-inst")
        return new CEditingActionBiosourceStructuredVoucher(seh, is_descriptor, is_feature, COrgMod::eSubtype_culture_collection, CEditingActionBiosourceStructuredVoucher::eVoucher_inst);
    if (field == "culture-collection-specid")
        return new CEditingActionBiosourceStructuredVoucher(seh, is_descriptor, is_feature, COrgMod::eSubtype_culture_collection, CEditingActionBiosourceStructuredVoucher::eVoucher_id);
    if (field == "specimen-voucher-coll")
        return new CEditingActionBiosourceStructuredVoucher(seh, is_descriptor, is_feature, COrgMod::eSubtype_specimen_voucher, CEditingActionBiosourceStructuredVoucher::eVoucher_coll);
    if (field == "specimen-voucher-inst")
        return new CEditingActionBiosourceStructuredVoucher(seh, is_descriptor, is_feature, COrgMod::eSubtype_specimen_voucher, CEditingActionBiosourceStructuredVoucher::eVoucher_inst);
    if (field == "specimen-voucher-specid")
        return new CEditingActionBiosourceStructuredVoucher(seh, is_descriptor, is_feature, COrgMod::eSubtype_specimen_voucher, CEditingActionBiosourceStructuredVoucher::eVoucher_id);
    if (COrgMod::IsValidSubtypeName(field))
        return new CEditingActionBiosourceOrgMod(seh, is_descriptor, is_feature, static_cast<COrgMod::ESubtype>(COrgMod::GetSubtypeValue(field)));   
    if (CSubSource::IsValidSubtypeName(field))
        return new CEditingActionBiosourceSubSource(seh, is_descriptor, is_feature, static_cast<CSubSource::ESubtype>(CSubSource::GetSubtypeValue(field)));   
    return NULL;
}



END_NCBI_SCOPE

