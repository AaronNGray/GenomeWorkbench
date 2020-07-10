/*  $Id: convert_desc_to_feat.cpp 37346 2016-12-27 18:27:53Z filippov $
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
#include <objtools/validator/validatorp.hpp>

#include <wx/textdlg.h>

#include <gui/packages/pkg_sequence_edit/convert_desc_to_feat.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CRef<CCmdComposite> CConvertDescToFeatComment::apply(CSeq_entry_Handle tse)
{
    CRef<CCmdComposite> cmd;
    if (!tse)
        return cmd;
    cmd.Reset(new CCmdComposite("Convert Desc To Feat Comment"));
    bool modified = false;
    set<pair<CSeq_entry_Handle, const CSeqdesc*> > visited;
    for (CBioseq_CI b_iter(tse, CSeq_inst::eMol_na); b_iter; ++b_iter)
    {         
        CBioseq_Handle bsh = *b_iter;
        for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_Comment); desc_it; ++desc_it)
        {
            CRef<CSeq_feat> new_feat(new CSeq_feat);
            new_feat->SetData().SetComment();
            new_feat->SetComment(desc_it->GetComment());
            CRef<objects::CSeq_id> seq_id(new objects::CSeq_id());
            seq_id->Assign(*(bsh.GetSeqId()));
            CRef<objects::CSeq_loc> loc(new objects::CSeq_loc(*seq_id, 0, bsh.GetBioseqLength() - 1));
            new_feat->SetLocation(*loc);
            CSeq_entry_Handle feat_seh = bsh.GetSeq_entry_Handle();
            cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(feat_seh, *new_feat)));
            pair<CSeq_entry_Handle, const CSeqdesc*> p = pair<CSeq_entry_Handle, const CSeqdesc*>(desc_it.GetSeq_entry_Handle(), &*desc_it);
            if (visited.find(p) == visited.end())
            {
                cmd->AddCommand(*CRef<CCmdDelDesc>(new CCmdDelDesc(desc_it.GetSeq_entry_Handle(), *desc_it)));
                visited.insert(p);
            }
            modified = true;
        }               
    }
    if (!modified)
        cmd.Reset();
    return cmd;
}

CRef<CCmdComposite> CConvertDescToFeatSource::apply(CSeq_entry_Handle tse)
{
    CRef<CCmdComposite> cmd;
    if (!tse)
        return cmd;
    cmd.Reset(new CCmdComposite("Convert Desc To Feat Source"));
    bool modified = false;
    set<pair<CSeq_entry_Handle, const CSeqdesc*> > visited;
    for (CBioseq_CI b_iter(tse, CSeq_inst::eMol_na); b_iter; ++b_iter)
    {         
        CBioseq_Handle bsh = *b_iter;
        for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_Source); desc_it; ++desc_it)
        {
            CRef<CBioSource> new_src(new CBioSource);
            new_src->Assign(desc_it->GetSource());
            CRef<CSeq_feat> new_feat(new CSeq_feat);
            new_feat->SetData().SetBiosrc(*new_src);
            CRef<objects::CSeq_id> seq_id(new objects::CSeq_id());
            seq_id->Assign(*(bsh.GetSeqId()));
            CRef<objects::CSeq_loc> loc(new objects::CSeq_loc(*seq_id, 0, bsh.GetBioseqLength() - 1));
            new_feat->SetLocation(*loc);
            CSeq_entry_Handle feat_seh = bsh.GetSeq_entry_Handle();
            cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(feat_seh, *new_feat)));
            pair<CSeq_entry_Handle, const CSeqdesc*> p = pair<CSeq_entry_Handle, const CSeqdesc*>(desc_it.GetSeq_entry_Handle(), &*desc_it);
            if (visited.find(p) == visited.end())
            {
                cmd->AddCommand(*CRef<CCmdDelDesc>(new CCmdDelDesc(desc_it.GetSeq_entry_Handle(), *desc_it)));
                visited.insert(p);
            }
            modified = true;
        }               
    }
    if (!modified)
        cmd.Reset();
    return cmd;
}

CRef<CCmdComposite> CConvertDescToFeatPub::apply(CSeq_entry_Handle tse)
{
    CRef<CCmdComposite> cmd;
    if (!tse)
        return cmd;
    cmd.Reset(new CCmdComposite("Convert Desc To Feat Pub"));
    bool modified = false;
    set<pair<CSeq_entry_Handle, const CSeqdesc*> > visited;
    for (CBioseq_CI b_iter(tse, CSeq_inst::eMol_na); b_iter; ++b_iter)
    {         
        CBioseq_Handle bsh = *b_iter;
        for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_Pub); desc_it; ++desc_it)
        {
            CRef<CPubdesc> new_pub(new CPubdesc);
            new_pub->Assign(desc_it->GetPub());
            CRef<CSeq_feat> new_feat(new CSeq_feat);
            new_feat->SetData().SetPub(*new_pub);
            CRef<objects::CSeq_id> seq_id(new objects::CSeq_id());
            seq_id->Assign(*(bsh.GetSeqId()));
            CRef<objects::CSeq_loc> loc(new objects::CSeq_loc(*seq_id, 0, bsh.GetBioseqLength() - 1));
            new_feat->SetLocation(*loc);
            CSeq_entry_Handle feat_seh = bsh.GetSeq_entry_Handle();
            cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(feat_seh, *new_feat)));
            pair<CSeq_entry_Handle, const CSeqdesc*> p = pair<CSeq_entry_Handle, const CSeqdesc*>(desc_it.GetSeq_entry_Handle(), &*desc_it);
            if (visited.find(p) == visited.end())
            {
                cmd->AddCommand(*CRef<CCmdDelDesc>(new CCmdDelDesc(desc_it.GetSeq_entry_Handle(), *desc_it)));
                visited.insert(p);
            }
            modified = true;
        }               
    }
    if (!modified)
        cmd.Reset();
    return cmd;
}

CRef<CCmdComposite> CConvertDescToFeatPubConstraint::apply(CSeq_entry_Handle tse)
{
    CRef<CCmdComposite> cmd;
    if (!tse)
        return cmd;
    string constraint;
    wxTextEntryDialog dlg(NULL, _("Optional String Constraint"), _("Convert Pub Descriptors"), wxEmptyString);
    if (dlg.ShowModal() == wxID_OK) 
    {
        constraint = dlg.GetValue().ToStdString();
    }
    if (constraint.empty())
        return cmd;

    cmd.Reset(new CCmdComposite("Convert Desc To Feat Pub"));
    bool modified = false;
    set<pair<CSeq_entry_Handle, const CSeqdesc*> > visited;
    for (CBioseq_CI b_iter(tse, CSeq_inst::eMol_na); b_iter; ++b_iter)
    {         
        CBioseq_Handle bsh = *b_iter;
        for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_Pub); desc_it; ++desc_it)
        {
            CNcbiOstrstream str;
            str << MSerial_AsnText << desc_it->GetPub();
            string val = CNcbiOstrstreamToString(str);
            if (NStr::FindNoCase(val, constraint) == NPOS)
                continue;
            CRef<CPubdesc> new_pub(new CPubdesc);
            new_pub->Assign(desc_it->GetPub());
            CRef<CSeq_feat> new_feat(new CSeq_feat);
            new_feat->SetData().SetPub(*new_pub);
            CRef<objects::CSeq_id> seq_id(new objects::CSeq_id());
            seq_id->Assign(*(bsh.GetSeqId()));
            CRef<objects::CSeq_loc> loc(new objects::CSeq_loc(*seq_id, 0, bsh.GetBioseqLength() - 1));
            new_feat->SetLocation(*loc);
            CSeq_entry_Handle feat_seh = bsh.GetSeq_entry_Handle();
            cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(feat_seh, *new_feat)));
            pair<CSeq_entry_Handle, const CSeqdesc*> p = pair<CSeq_entry_Handle, const CSeqdesc*>(desc_it.GetSeq_entry_Handle(), &*desc_it);
            if (visited.find(p) == visited.end())
            {
                cmd->AddCommand(*CRef<CCmdDelDesc>(new CCmdDelDesc(desc_it.GetSeq_entry_Handle(), *desc_it)));
                visited.insert(p);
            }
            modified = true;
        }               
    }
    if (!modified)
        cmd.Reset();
    return cmd;
}

CRef<CCmdComposite> CConvertFeatToDescComment::apply(CSeq_entry_Handle tse)
{
    CRef<CCmdComposite> cmd;
    if (!tse)
        return cmd;
    CScope &scope = tse.GetScope();
    cmd.Reset(new CCmdComposite("Convert Feat to Desc Comment"));
    bool modified = false;
    for (CFeat_CI fi(tse, CSeqFeatData::e_Comment); fi; ++fi)
    {         
        const CSeq_loc& loc = fi->GetLocation();
        CBioseq_Handle bsh = scope.GetBioseqHandle(loc);
        if (loc.IsInt() && loc.GetInt().GetStart(eExtreme_Positional) == 0 && loc.GetInt().GetStop(eExtreme_Positional) ==  bsh.GetBioseqLength() - 1 &&
            fi->GetOriginalFeature().IsSetComment())
        {
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->SetComment(fi->GetOriginalFeature().GetComment());
            cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(bsh.GetParentEntry(), *new_desc)) );
            cmd->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(fi->GetSeq_feat_Handle())));         
            modified = true;
        }               
    }
    if (!modified)
        cmd.Reset();
    return cmd;
}

CRef<CCmdComposite> CConvertFeatToDescSource::apply(CSeq_entry_Handle tse)
{
    CRef<CCmdComposite> cmd;
    if (!tse)
        return cmd;
    CScope &scope = tse.GetScope();
    cmd.Reset(new CCmdComposite("Convert Feat to Desc Source"));
    bool modified = false;
    for (CFeat_CI fi(tse, CSeqFeatData::e_Biosrc); fi; ++fi)
    {         
        const CSeq_loc& loc = fi->GetLocation();
        CBioseq_Handle bsh = scope.GetBioseqHandle(loc);
        if (loc.IsInt() && loc.GetInt().GetStart(eExtreme_Positional) == 0 && loc.GetInt().GetStop(eExtreme_Positional) ==  bsh.GetBioseqLength() - 1)
        {
            CRef<CBioSource> new_src(new CBioSource);
            new_src->Assign(fi->GetOriginalFeature().GetData().GetBiosrc());
            if (fi->GetOriginalFeature().IsSetComment())
            {
                CRef< CSubSource > new_sub(new CSubSource(CSubSource::eSubtype_other, fi->GetOriginalFeature().GetComment()));                
                new_src->SetSubtype().push_back(new_sub);
            }
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->SetSource(*new_src);
            cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(bsh.GetParentEntry(), *new_desc)) );
            cmd->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(fi->GetSeq_feat_Handle())));         
            modified = true;
        }               
    }
    if (!modified)
        cmd.Reset();
    return cmd;
}

CRef<CCmdComposite> CConvertFeatToDescPub::apply(CSeq_entry_Handle tse)
{
    CRef<CCmdComposite> cmd;
    if (!tse)
        return cmd;
    CScope &scope = tse.GetScope();
    cmd.Reset(new CCmdComposite("Convert Feat to Desc Pub"));
    bool modified = false;
    for (CFeat_CI fi(tse, CSeqFeatData::e_Pub); fi; ++fi)
    {         
        const CSeq_loc& loc = fi->GetLocation();
        CBioseq_Handle bsh = scope.GetBioseqHandle(loc);
        if (loc.IsInt() && loc.GetInt().GetStart(eExtreme_Positional) == 0 && loc.GetInt().GetStop(eExtreme_Positional) ==  bsh.GetBioseqLength() - 1)
        {
            CRef<CPubdesc> new_pub(new CPubdesc);
            new_pub->Assign(fi->GetOriginalFeature().GetData().GetPub());
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->SetPub(*new_pub);
            cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(bsh.GetParentEntry(), *new_desc)) );
            cmd->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(fi->GetSeq_feat_Handle())));         
            modified = true;
        }               
    }
    if (!modified)
        cmd.Reset();
    return cmd;
}

END_NCBI_SCOPE
