/* $Id: package_features_on_parts.cpp 40545 2018-03-06 18:45:25Z filippov $
 * ===========================================================================
 *
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
 * Authors: Igor Filippov
 * File:  Package Features on Parts
 *  
 */

#include <ncbi_pch.hpp>
#include <objmgr/objmgr_exception.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_add_cds.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>

#include <gui/packages/pkg_sequence_edit/package_features_on_parts.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CRef<CCmdComposite>  CPackageFeaturesOnParts::apply(CSeq_entry_Handle seh)
{
    CRef<CCmdComposite> composite(new CCmdComposite("Package Features on Parts"));
   if (seh) 
   {
       for(CFeat_CI feat_it(seh); feat_it; ++feat_it)
       {
           const CSeq_annot_Handle& annot_handle = feat_it->GetAnnot();
           CSeq_entry_Handle old_seh = annot_handle.GetParentEntry();
	   CBioseq_Handle bsh;
	   try 
	   {
	       bsh = seh.GetScope().GetBioseqHandle(feat_it->GetLocation());
	   } catch (const CObjMgrException&) {}
	   if (!bsh)
	       continue;
           CSeq_entry_Handle new_seh = bsh.GetSeq_entry_Handle();
           if ( feat_it->GetSeq_feat_Handle().GetFeatType() == CSeqFeatData::e_Cdregion)
           {
               CSeq_entry_Handle parent_seh = new_seh.GetParentEntry();
               if ( parent_seh && parent_seh.IsSet() && parent_seh.GetSet().IsSetClass() 
                    && parent_seh.GetSet().GetClass() ==  CBioseq_set::eClass_nuc_prot )
               {
                   new_seh = parent_seh;
               }
           }
           if (old_seh != new_seh)
           {
               CRef<CSeq_feat> new_feat(new CSeq_feat());
               new_feat->Assign(feat_it->GetOriginalFeature());    
               composite->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(feat_it->GetSeq_feat_Handle())));
               composite->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(new_seh, *new_feat)));
           }
       }
    
   }

   return composite;
}          

END_NCBI_SCOPE
