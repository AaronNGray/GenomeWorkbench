/* $Id: src_focus.cpp 28832 2013-09-04 16:47:31Z chenj $
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
 * File:  for BioSource is_focus set & clear
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

void CSrcFocusSet :: apply_to(const CSeq_entry_Handle& tse, CRef <CCmdComposite> composite)
{
   for (CBioseq_CI bit(tse); bit; ++bit) {
     CSeqdesc_CI s_ci(*bit, CSeqdesc::e_Source);
     if ( s_ci && CFeat_CI(*bit, CSeqFeatData::e_Biosrc) 
               && !s_ci->GetSource().CanGetIs_focus()) {
         CRef <CBioSource> new_obj(new CBioSource);
         new_obj->Assign(s_ci->GetSource());
         if (!(new_obj->CanGetIs_focus())) new_obj->SetIs_focus(); 
         add_to_cmd<objects::CBioSource>(
             (CObject*)(&(s_ci->GetSource())), (CObject*)(new_obj.GetPointer()), composite); 
     }
   }
};

void CSrcFocusClear :: apply_to(const CSeq_entry_Handle& tse, CRef <CCmdComposite> composite)
{
   for (CBioseq_CI bit(tse); bit; ++bit) {
     for (CSeqdesc_CI s_ci(*bit, CSeqdesc::e_Source); s_ci; ++s_ci) { // read original file
         CRef <CBioSource> new_obj(new CBioSource);
         new_obj->Assign(s_ci->GetSource());
         new_obj->ResetIs_focus();
         add_to_cmd<objects::CBioSource> (
             (CObject*)(&(s_ci->GetSource())), (CObject*)(new_obj.GetPointer()), composite); 
     }
   }
};
