/*  $Id: merge_biosources.cpp 42255 2019-01-22 15:35:14Z asztalos $
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
#include <objects/seqfeat/BioSource.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objtools/cleanup/cleanup.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/packages/pkg_sequence_edit/merge_biosources.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

CRef<CCmdComposite> CMergeBiosources::apply(CSeq_entry_Handle tse)
{
    if (!tse) 
        return CRef<CCmdComposite>(NULL);
    bool modified = false;
    CRef<CCmdComposite> composite( new CCmdComposite("Merge BioSources") );   

    for (CBioseq_CI bit(tse); bit; ++bit) 
    {
        CBioseq_Handle bsh = *bit;
        vector<CSeqdesc_CI> descs;
        vector<const CBioSource*> biosources;
        vector<CSeq_feat_Handle> feats;
        for (CSeqdesc_CI source_ci(bsh, CSeqdesc::e_Source); source_ci; ++source_ci)
        {
            const CBioSource* biosource = &source_ci->GetSource();
            biosources.push_back(biosource);
            descs.push_back(source_ci);
            CSeq_feat_Handle fh;
            feats.push_back(fh);
        }
        for (CFeat_CI feat_ci(bsh, CSeqFeatData::e_Biosrc); feat_ci; ++feat_ci)
        {
            const CBioSource* biosource = &feat_ci->GetData().GetBiosrc();
            biosources.push_back(biosource);
            descs.push_back(CSeqdesc_CI());
            feats.push_back(feat_ci->GetSeq_feat_Handle());
        }
        vector<CRef<CBioSource> > new_biosources(biosources.size(), CRef<CBioSource>(NULL));
        for (size_t i = 0; i < biosources.size(); i++)
            if (biosources[i] != NULL)
            {
                for (size_t j = i + 1; j < biosources.size(); j++)
                    if (biosources[j] != NULL)
                    {
                        if (CCleanup::AreBioSourcesMergeable(*biosources[i], *biosources[j]))
                        {
                            CRef<CBioSource> new_biosource(new CBioSource);
                            if (new_biosources[i])
                                new_biosource->Assign(*new_biosources[i]);
                            else
                                new_biosource->Assign(*biosources[i]);
                            CCleanup::MergeDupBioSources(*new_biosource, *biosources[j]);
                            CRef<CSeq_feat> new_feat(new CSeq_feat);
                            new_feat->SetData().SetBiosrc(*new_biosource);
                            new_biosources[i] = CCleanup::BioSrcFromFeat(*new_feat);
                            if (descs[j])
                            {
                                CSeq_entry_Handle seh = descs[j].GetSeq_entry_Handle();
                                CIRef<IEditCommand> cmdDelDesc(new CCmdDelDesc(seh, *descs[j]));
                                composite->AddCommand(*cmdDelDesc);
                                descs[j] = CSeqdesc_CI();
                                biosources[j] = NULL;
                            }
                            else if (feats[j])
                            {
                                composite->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(feats[j])));    
                                feats[j].Reset();
                                biosources[j] = NULL;
                            }
                        }
                    }
            }

        for (size_t i = 0; i < new_biosources.size(); i++)
            if (new_biosources[i])
            {
                modified = true;
                if (descs[i])
                {                    
                    CRef<CSeqdesc> new_desc(new CSeqdesc);
                    new_desc->SetSource(*new_biosources[i]);
                    CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(descs[i].GetSeq_entry_Handle(), *descs[i], *new_desc));
                    composite->AddCommand(*cmd);
                }
                else if (feats[i])
                {
                    CRef<CSeq_feat> new_feat(new CSeq_feat);
                    new_feat->Assign(*feats[i].GetOriginalSeq_feat());
                    new_feat->SetData().SetBiosrc(*new_biosources[i]);
                    composite->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feats[i], *new_feat)));
                }
            }
    }
    if (!modified)
        composite.Reset();
    return composite;
}



END_NCBI_SCOPE
