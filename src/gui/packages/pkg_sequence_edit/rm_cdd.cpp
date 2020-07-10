/* $Id: rm_cdd.cpp 40679 2018-03-28 21:45:29Z filippov $
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
 * Authors:  Jie Chen 
 * File:  Remove all cdd features and Dbxrefs.
 *  
 */

#include <ncbi_pch.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <gui/objutils/util_cmds.hpp>

#include "gui/packages/pkg_sequence_edit/citem_util.hpp"

using namespace ncbi;
using namespace objects;

void CRmvCddFeatDbxref ::apply_to(const CSeq_entry_Handle& tse, CRef <CCmdComposite> composite)
{
    map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle> > product_to_cds;
    GetProductToCDSMap(tse.GetScope(), product_to_cds);
   string end_str = "ScoreData", ext_str;
   for (CFeat_CI fci(tse); fci; ++fci) {
      const CSeq_feat& sf = fci->GetOriginalFeature();     
      if (sf.CanGetExt() && sf.GetExt().GetType().IsStr()) {    // remove cdd feature
         ext_str = CTempString(sf.GetExt().GetType().GetStr());
         if (ext_str.size() > end_str.size()
                && ext_str.substr(0, 3) == "cdd"
                && ext_str.substr(ext_str.size() - end_str.size()) == end_str) {
             CRef <CCmdComposite> cmd = GetDeleteFeatureCommand(fci->GetSeq_feat_Handle(), true, product_to_cds);
             composite->AddCommand(*cmd);
         }
      }
      if (sf.CanGetDbxref()) {   // remove cd dbxrefs
         if (sf.GetNamedDbxref("CDD").NotEmpty()) {
             CRef <CSeq_feat> new_sf (new CSeq_feat);
             new_sf->Assign(sf);
             new_sf->ResetDbxref();
             ITERATE (vector <CRef <CDbtag> >, it, sf.GetDbxref()) { 
                if ( (*it)->GetDb() != "CDD") new_sf->SetDbxref().push_back(*it);
             }
             add_to_cmd<CSeq_feat>((CObject*)(&sf),(CObject*)(new_sf.GetPointer()), composite);
         }
      }
   }
};
