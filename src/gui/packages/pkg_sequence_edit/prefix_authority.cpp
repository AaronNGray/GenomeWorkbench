/* $Id: prefix_authority.cpp 28832 2013-09-04 16:47:31Z chenj $
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
 * File:  for Special:Organism->Prefix Authority with Organism
 *  
 */

#include <ncbi_pch.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>

#include "gui/packages/pkg_sequence_edit/citem_util.hpp"

using namespace ncbi;
using namespace objects;

void CPrefixAuthority :: apply_to(const CSeq_entry_Handle& tse, CRef <CCmdComposite> composite)
{
   set <const CSeqdesc*> psds;
   for (CBioseq_CI bit(tse); bit; ++bit) {
      for (CSeqdesc_CI it(*bit, CSeqdesc::e_Source); it; ++it) {
        if (psds.find(&*it) != psds.end()) continue;
        psds.insert(&*it);
        if (it->GetSource().IsSetOrgMod()) {
           ITERATE (list <CRef < COrgMod > >, mit, it->GetSource().GetOrgname().GetMod()) {
             if ( (*mit)->GetSubtype() == COrgMod::eSubtype_authority) {
                if (it->GetSource().IsSetTaxname()) {
                   const string& taxname = it->GetSource().GetTaxname();
                   const string authority = (*mit)->GetSubname();
                   if (!NStr::EqualNocase(authority, 0, taxname.size(), taxname)) {
                      CRef <COrgMod> new_mod (new COrgMod);
                      new_mod->Assign(**mit);
                      new_mod->SetSubname(taxname + " " + authority);
                      add_to_cmd <COrgMod> ( (CObject*)((*mit).GetPointer()), 
                                                 (CObject*)(new_mod.GetPointer()), composite);
                      break;
                   }
                }
             }
           }
        }
      }
   }
};
