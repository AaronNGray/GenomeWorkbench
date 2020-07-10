/*  $Id: convert_second_prot_name.cpp 37143 2016-12-07 21:02:59Z filippov $
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
#include <gui/packages/pkg_sequence_edit/convert_second_prot_name.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CRef<CCmdComposite> CConvertSecondProtNameToDesc::apply(objects::CSeq_entry_Handle tse)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Convert second protein name to description"));      
    bool modified = false;

    for (CFeat_CI feat_ci(tse, SAnnotSelector(CSeqFeatData::e_Prot)); feat_ci; ++feat_ci)
    {
        const CSeq_feat& orig = feat_ci->GetOriginalFeature();       
        
        if (!orig.IsSetData() || !orig.GetData().IsProt() || !orig.GetData().GetProt().IsSetName() || orig.GetData().GetProt().GetName().size() < 2) 
            continue;
            
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(orig);
        CProt_ref &prot = new_feat->SetData().SetProt();

        CProt_ref::TName::iterator it = prot.SetName().begin();
        ++it;
        string name = *it;
        it = prot.SetName().erase(it);
        modified = true;
        if (prot.IsSetDesc() && !prot.GetDesc().empty())
            name = prot.GetDesc() + "; " + name;
        prot.SetDesc(name);
        cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat))); 
    }      
    
    if (!modified)
        cmd.Reset();
    return cmd;
}

CRef<CCmdComposite> CConvertDescToSecondProtName::apply(objects::CSeq_entry_Handle tse)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Convert description to second protein name"));      
    bool modified = false;

    for (CFeat_CI feat_ci(tse, SAnnotSelector(CSeqFeatData::e_Prot)); feat_ci; ++feat_ci)
    {
        const CSeq_feat& orig = feat_ci->GetOriginalFeature();       
        
        if (!orig.IsSetData() || !orig.GetData().IsProt() || !orig.GetData().GetProt().IsSetDesc() || orig.GetData().GetProt().GetDesc().empty() ) 
            continue;
            
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(orig);
        CProt_ref &prot = new_feat->SetData().SetProt();

        string desc = prot.GetDesc();
        prot.SetName().push_back(desc);
        prot.ResetDesc();
        modified = true;
        cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat))); 
    }      
    
    if (!modified)
        cmd.Reset();
    return cmd;
}


END_NCBI_SCOPE
