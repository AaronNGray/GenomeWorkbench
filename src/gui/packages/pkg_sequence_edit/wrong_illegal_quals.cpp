/*  $Id: wrong_illegal_quals.cpp 39111 2017-08-01 15:00:06Z filippov $
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
#include <gui/objutils/cmd_composite.hpp>
#include <objmgr/feat_ci.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/wrong_illegal_quals.hpp>

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

void CWrongIllegalQuals::IllegalQualsToNote( CSeq_entry_Handle seh,  ICommandProccessor* cmd_processor)
{
   CRef<CCmdComposite> cmd(new CCmdComposite("Illegal Quals To Note"));
    bool modified = false;
    for (CFeat_CI feat_ci(seh); feat_ci; ++feat_ci)
    {
        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(orig);
        bool changed = false;
        if (new_feat->IsSetQual())
        {
            CSeq_feat::TQual::iterator qual_it = new_feat->SetQual().begin();
            while (qual_it != new_feat->SetQual().end())
            {
                bool to_delete = false;
                if ((*qual_it)->IsSetQual())
                {
                    CSeqFeatData::EQualifier gbqual = CSeqFeatData::GetQualifierType((*qual_it)->GetQual());
                    if (gbqual == CSeqFeatData::eQual_bad)
                    {
                        if ((*qual_it)->IsSetVal())
                        {
                            string content = "["+(*qual_it)->GetQual()+"="+(*qual_it)->GetVal()+"]";
                            string prev_value;
                            if (new_feat->IsSetComment())
                            {
                                prev_value = new_feat->GetComment();
                            }
                            if (!prev_value.empty())
                                prev_value += "; ";
                            new_feat->SetComment(prev_value + content);
                        }
                        to_delete = true;
                        changed = true;
                    }
                }
                if (to_delete)
                    qual_it = new_feat->SetQual().erase(qual_it);
                else
                    ++qual_it;
            }
            if (new_feat->SetQual().empty())
                new_feat->ResetQual();
        }

        if (changed)
        {
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
            modified = true;
        }
    }

    if (modified)
    {
        cmd_processor->Execute(cmd);
    }
}


bool s_SpecialRemove(CSeqFeatData::ESubtype subtype, CSeqFeatData::EQualifier qual, const string &value)
{
    if (subtype == CSeqFeatData::eSubtype_cdregion) {
        if (qual == CSeqFeatData::eQual_codon_start
            || qual == CSeqFeatData::eQual_codon
            || qual == CSeqFeatData::eQual_EC_number
            || qual == CSeqFeatData::eQual_gdb_xref
            || qual == CSeqFeatData::eQual_number
            || qual == CSeqFeatData::eQual_protein_id
            || qual == CSeqFeatData::eQual_transl_except
            || qual == CSeqFeatData::eQual_transl_table
            || qual == CSeqFeatData::eQual_translation
            || qual == CSeqFeatData::eQual_allele
            || qual == CSeqFeatData::eQual_function
            || qual == CSeqFeatData::eQual_old_locus_tag) {
            return true;
        }           
    }
    if (subtype == CSeqFeatData::eSubtype_tRNA )
    {
        if (qual == CSeqFeatData::eQual_anticodon)
            return true;
        if (qual == CSeqFeatData::eQual_product 
            && !NStr::EqualNocase (value, "tRNA-fMet")
            && !NStr::EqualNocase (value, "tRNA-iMet"))
            return true;
    }
    return false;
}

void CWrongIllegalQuals::RmIllegalQuals( CSeq_entry_Handle seh,  ICommandProccessor* cmd_processor)
{
   CRef<CCmdComposite> cmd(new CCmdComposite("Remove Illegal Quals"));
    bool modified = false;
    for (CFeat_CI feat_ci(seh); feat_ci; ++feat_ci)
    {
        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(orig);
        bool changed = false;
        if (new_feat->IsSetQual())
        {
            CSeq_feat::TQual::iterator qual_it = new_feat->SetQual().begin();
            while (qual_it != new_feat->SetQual().end())
            {
                bool to_delete = false;
                string value;
                if ((*qual_it)->IsSetVal())
                    value = (*qual_it)->GetVal();

                if ((*qual_it)->IsSetQual())
                {
                    CSeqFeatData::EQualifier gbqual = CSeqFeatData::GetQualifierType((*qual_it)->GetQual());
                    if (gbqual == CSeqFeatData::eQual_bad || gbqual == CSeqFeatData::eQual_transl_except ||
                        !CSeqFeatData::IsLegalQualifier(new_feat->GetData().GetSubtype(), gbqual) ||
                        s_SpecialRemove(new_feat->GetData().GetSubtype(), gbqual, value))
                    {                        
                        to_delete = true;
                        changed = true;
                    }
                }
                if (to_delete)
                    qual_it = new_feat->SetQual().erase(qual_it);
                else
                    ++qual_it;
            }
            if (new_feat->SetQual().empty())
                new_feat->ResetQual();
        }

        if (changed)
        {
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
            modified = true;
        }
    }

    if (modified)
    {
        cmd_processor->Execute(cmd);
    }
}

void CWrongIllegalQuals::WrongQualsToNote( CSeq_entry_Handle seh,  ICommandProccessor* cmd_processor)
{
   CRef<CCmdComposite> cmd(new CCmdComposite("Wrong Quals To Note"));
    bool modified = false;
    for (CFeat_CI feat_ci(seh); feat_ci; ++feat_ci)
    {
        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(orig);
        bool changed = false;
        if (new_feat->IsSetQual())
        {
            CSeq_feat::TQual::iterator qual_it = new_feat->SetQual().begin();
            while (qual_it != new_feat->SetQual().end())
            {
                bool to_delete = false;
                if ((*qual_it)->IsSetQual())
                {
                    CSeqFeatData::EQualifier gbqual = CSeqFeatData::GetQualifierType((*qual_it)->GetQual());
                    if (gbqual != CSeqFeatData::eQual_bad && !CSeqFeatData::IsLegalQualifier(new_feat->GetData().GetSubtype(), gbqual))
                    {
                        if ((*qual_it)->IsSetVal())
                        {
                            string content = "["+(*qual_it)->GetQual()+"="+(*qual_it)->GetVal()+"]";
                            string prev_value;
                            if (new_feat->IsSetComment())
                            {
                                prev_value = new_feat->GetComment();
                            }
                            if (!prev_value.empty())
                                prev_value += "; ";
                            new_feat->SetComment(prev_value + content);
                        }
                        to_delete = true;
                        changed = true;
                    }
                }
                if (to_delete)
                    qual_it = new_feat->SetQual().erase(qual_it);
                else
                    ++qual_it;
            }
            if (new_feat->SetQual().empty())
                new_feat->ResetQual();
        }

        if (changed)
        {
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
            modified = true;
        }
    }

    if (modified)
    {
        cmd_processor->Execute(cmd);
    }
}

void CWrongIllegalQuals::RmWrongQuals( CSeq_entry_Handle seh,  ICommandProccessor* cmd_processor)
{
   CRef<CCmdComposite> cmd(new CCmdComposite("Remove Wrong Quals"));
    bool modified = false;
    for (CFeat_CI feat_ci(seh); feat_ci; ++feat_ci)
    {
        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(orig);
        bool changed = false;
        if (new_feat->IsSetQual())
        {
            CSeq_feat::TQual::iterator qual_it = new_feat->SetQual().begin();
            while (qual_it != new_feat->SetQual().end())
            {
                bool to_delete = false;
                if ((*qual_it)->IsSetQual())
                {
                    CSeqFeatData::EQualifier gbqual = CSeqFeatData::GetQualifierType((*qual_it)->GetQual());
                    if (gbqual != CSeqFeatData::eQual_bad && !CSeqFeatData::IsLegalQualifier(new_feat->GetData().GetSubtype(), gbqual))
                    {                       
                        to_delete = true;
                        changed = true;
                    }
                }
                if (to_delete)
                    qual_it = new_feat->SetQual().erase(qual_it);
                else
                    ++qual_it;
            }
            if (new_feat->SetQual().empty())
                new_feat->ResetQual();
        }

        if (changed)
        {
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
            modified = true;
        }
    }

    if (modified)
    {
        cmd_processor->Execute(cmd);
    }
}


void CWrongIllegalQuals::RmWrongOrIllegalQuals( CSeq_entry_Handle seh,  ICommandProccessor* cmd_processor)
{
   CRef<CCmdComposite> cmd(new CCmdComposite("Remove Wrong or Illegal Quals"));
    bool modified = false;
    for (CFeat_CI feat_ci(seh); feat_ci; ++feat_ci)
    {
        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(orig);
        bool changed = false;
        if (new_feat->IsSetQual())
        {
            CSeq_feat::TQual::iterator qual_it = new_feat->SetQual().begin();
            while (qual_it != new_feat->SetQual().end())
            {
                bool to_delete = false;
                if ((*qual_it)->IsSetQual())
                {
                    CSeqFeatData::EQualifier gbqual = CSeqFeatData::GetQualifierType((*qual_it)->GetQual());
                    if ((gbqual != CSeqFeatData::eQual_bad && !CSeqFeatData::IsLegalQualifier(new_feat->GetData().GetSubtype(), gbqual))
                        || (gbqual == CSeqFeatData::eQual_bad) )
                    {                       
                        to_delete = true;
                        changed = true;
                    }
                }
                if (to_delete)
                    qual_it = new_feat->SetQual().erase(qual_it);
                else
                    ++qual_it;
            }
            if (new_feat->SetQual().empty())
                new_feat->ResetQual();
        }

        if (changed)
        {
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
            modified = true;
        }
    }

    if (modified)
    {
        cmd_processor->Execute(cmd);
    }
}

END_NCBI_SCOPE
