/*  $Id: append_mod_to_org.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
#include <objtools/edit/source_edit.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>

#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>
#include <gui/packages/pkg_sequence_edit/append_mod_to_org.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CRef<CCmdComposite> CAppendModToOrg::GetCommand(CSeq_entry_Handle tse)
{
    if (!tse)
        return CRef<CCmdComposite>();

    m_Scope.Reset(&(tse.GetScope()));
    CRef<CCmdComposite> composite(new CCmdComposite("Strain to sp."));
    apply_to_seq_and_feat(tse, composite);
    return composite;
}

void CAppendModToOrg::apply_to_seq_and_feat(CSeq_entry_Handle tse, CCmdComposite* composite)
{
    x_ApplyToDescriptors(*(tse.GetCompleteSeq_entry()), composite);

    for (CFeat_CI feat_it(tse, SAnnotSelector(CSeqFeatData::e_Biosrc)); feat_it; ++feat_it) {
        if (m_constraint->Match(*feat_it)) {
            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(feat_it->GetOriginalFeature());
            if (x_ApplyToBioSource(new_feat->SetData().SetBiosrc())) {
                CRef<CCmdChangeSeq_feat> cmd(new CCmdChangeSeq_feat(*feat_it, *new_feat));
                composite->AddCommand(*cmd);
            }
        }
    }
}

void CAppendModToOrg::x_ApplyToDescriptors(const CSeq_entry& se, CCmdComposite* composite)
{
    FOR_EACH_SEQDESC_ON_SEQENTRY(it, se) {
        if ((*it)->IsSource()) {
            if (m_constraint->Match(it->GetPointer())) {
                const CSeqdesc& orig_desc = **it;
                CRef<CSeqdesc> new_desc(new CSeqdesc);
                new_desc->Assign(orig_desc);
                if (x_ApplyToBioSource(new_desc->SetSource())) {
                    CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(m_Scope->GetSeq_entryHandle(se), orig_desc, *new_desc));
                    composite->AddCommand(*cmd);
                }
            }
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET(it, se.GetSet()) {
            x_ApplyToDescriptors(**it, composite);
        }
    }
}

bool CAppendModToOrg::x_ApplyToBioSource (CBioSource& biosource)
{
    bool modified = false;

    bool ok_to_add = false;
    if (!m_Only_sp && !m_Only_cf && !m_Only_aff && !m_Only_nr)
        ok_to_add = true;
  
    if (biosource.IsSetTaxname()) {
        if (m_EndsWith) {
            if (m_Only_sp && NStr::EndsWith(biosource.GetTaxname()," sp."))
                ok_to_add = true;
        
            if (m_Only_cf  && NStr::EndsWith(biosource.GetTaxname()," cf."))
                ok_to_add = true;
        
            if (m_Only_aff && NStr::EndsWith(biosource.GetTaxname()," aff."))
                ok_to_add = true;

            if (m_Only_nr && NStr::EndsWith(biosource.GetTaxname()," nr."))
                ok_to_add = true;
        } else {
            if (m_Only_sp && biosource.GetTaxname().find(" sp.") != string::npos)
                ok_to_add = true;
  
            if (m_Only_cf  && biosource.GetTaxname().find(" cf.") !=  string::npos)
                ok_to_add = true;
  
            if (m_Only_aff && biosource.GetTaxname().find(" aff.") != string::npos)
                ok_to_add = true;

            if (m_Only_nr && biosource.GetTaxname().find(" nr.") != string::npos)
                ok_to_add = true;
        }
    }
    
    if (m_No_taxid)
    {
        if  (biosource.IsSetOrg() && biosource.GetOrg().IsSetDb())       
            {
                COrg_ref::TDb new_db;
                FOR_EACH_DBXREF_ON_ORGREF(db,biosource.SetOrg())
                    if ((*db)->IsSetDb() && NStr::Equal((*db)->GetDb(),"taxon",NStr::eNocase))
                        ok_to_add = false;
            }
    }

    if (!ok_to_add)
        return modified;
  


    string str_to_add;
    if (m_IsOrgMod)
    {
        if (biosource.IsSetOrg() && biosource.GetOrg().IsSetOrgname() && biosource.GetOrg().GetOrgname().IsSetMod())
        {
            FOR_EACH_ORGMOD_ON_BIOSOURCE(orgmod,biosource)
                if ((*orgmod)->IsSetSubtype() && (*orgmod)->IsSetSubname() && (*orgmod)->GetSubtype() == m_Subtype)
                {
                    str_to_add = (*orgmod)->GetSubname();
                }
        }
    }
    else
    {
        if (biosource.IsSetSubtype())
        {
            FOR_EACH_SUBSOURCE_ON_BIOSOURCE(subsource,biosource)
                if ((*subsource)->IsSetSubtype() && (*subsource)->IsSetName() && (*subsource)->GetSubtype() == m_Subtype)
                {
                    str_to_add = (*subsource)->GetName();
                }
        }
    }
    if (!str_to_add.empty())
    {
       
        if (m_IsOrgMod && (m_Subtype == COrgMod::eSubtype_bio_material || m_Subtype == COrgMod::eSubtype_culture_collection || m_Subtype == COrgMod::eSubtype_specimen_voucher))
        {
            NStr::ReplaceInPlace(str_to_add,":"," ");
        }
        
        bool influenza = false;
        if (biosource.IsSetOrg() && biosource.GetOrg().IsSetTaxname())
        {
            string& taxname = biosource.SetOrg().SetTaxname();
            if (NStr::Equal(taxname,"Influenza A virus",NStr::eNocase) || NStr::Equal(taxname,"Influenza B virus",NStr::eNocase))
                influenza = true;
        }

        string str;
        if (biosource.IsSetOrg() && biosource.GetOrg().IsSetTaxname())
        {
            string& taxname = biosource.SetOrg().SetTaxname();
            str = taxname + " ";
        }
        if (!m_Abbreviation.empty() && m_Use_Abbreviation)
            str += m_Abbreviation + " ";
        if (influenza)
            str += "(";
        str += str_to_add;
        if (influenza)
        {
            str += ")";
            if (NStr::StartsWith(str, "Influenza A virus", NStr::eNocase))             //str = FixInfluenzaVirusName(str);
            {
                NStr::ReplaceInPlace (str, "  ", " ");
                NStr::ReplaceInPlace (str, "( ", "(");
                NStr::ReplaceInPlace (str, " )", ")");
                NStr::TruncateSpacesInPlace (str);
            }
        }

        // SetTaxNameAndRemoveTaxRef (biop->org, str);
        bool remove_taxref = false;
        bool is_set_taxname =  biosource.IsSetOrg() && biosource.GetOrg().IsSetTaxname();
        if (!is_set_taxname || str.empty() || str !=  biosource.GetOrg().GetTaxname())
            remove_taxref = true;
        if (biosource.IsSetOrg())
        {
            biosource.SetOrg().SetTaxname() = str;
            modified = true;
        }
        if (remove_taxref) {
            edit::CleanupForTaxnameChange(biosource);
        }
    }
    return modified;
}

END_NCBI_SCOPE
