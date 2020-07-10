/*  $Id: update_replaced_ec_numbers.cpp 32177 2015-01-14 22:33:07Z filippov $
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
#include <wx/msgdlg.h> 
#include <gui/packages/pkg_sequence_edit/update_replaced_ec_numbers.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CUpdateReplacedECNumbers::apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title)
{
    if (tse) 
    {
        CRef<CCmdComposite> cmd(new CCmdComposite(title));
        bool ask_about_invalid = false;
        bool remove_invalid = false;
        bool ask_about_unknown = false;
        bool remove_unknown = false;
        for (CFeat_CI feat_ci(tse, SAnnotSelector(CSeqFeatData::e_Prot)); feat_ci; ++feat_ci)
        {
            const CSeq_feat& orig = feat_ci->GetOriginalFeature();       

            if (orig.IsSetData() && orig.GetData().IsProt())
            {
                if (!orig.GetData().GetProt().IsSetEc()) 
                    continue;
            
                CRef<CSeq_feat> new_feat(new CSeq_feat());
                new_feat->Assign(orig);
                
                bool modified = false;
                CProt_ref::TEc::iterator it = new_feat->SetData().SetProt().SetEc().begin();
                while (it != new_feat->SetData().SetProt().SetEc().end()) 
                {
                    bool is_valid = CProt_ref::IsValidECNumberFormat(*it);
                    string ec = *it;
                    CProt_ref::EECNumberStatus status = CProt_ref::GetECNumberStatus(ec);
                    if (!is_valid)
                    {
                        if (!ask_about_invalid)
                        {
                            int answer = wxMessageBox (_("Remove invalid EC numbers?"), _("Delete Invalid"), wxYES_NO |  wxICON_QUESTION);
                            remove_invalid = (answer == wxYES);
                            ask_about_invalid = true;
                        }                   
                    }
                    else 
                    {
                        while (status == CProt_ref::eEC_replaced)
                        {
                            ec = CProt_ref::GetECNumberReplacement(ec);   
                            status = CProt_ref::GetECNumberStatus(ec);
                        }
                    
                        if (status == CProt_ref::eEC_unknown)
                        {
                            if (!ask_about_unknown)
                            {
                                int answer = wxMessageBox (_("Remove unknown EC numbers?"), _("Delete Unknown"), wxYES_NO |  wxICON_QUESTION);
                                remove_unknown = (answer == wxYES);
                                ask_about_unknown = true;
                            }
                        }
                    }
                    
                    if ((!is_valid && ask_about_invalid && remove_invalid) || (is_valid && ask_about_unknown && remove_unknown && status == CProt_ref::eEC_unknown) || (is_valid && status == CProt_ref::eEC_deleted))
                    {
                        it = new_feat->SetData().SetProt().SetEc().erase(it);
                        modified = true;
                    }
                    else
                    {
                        if (ec != *it)
                        {
                            *it = ec;
                            modified = true;
                        }
                        ++it;
                    }                
                }
                if (new_feat->SetData().SetProt().SetEc().empty()) 
                {
                    new_feat->SetData().SetProt().ResetEc();
                }

                if (modified)
                    cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat))); 
            }      
        }
        cmdProcessor->Execute(cmd);   
    }
}


END_NCBI_SCOPE
