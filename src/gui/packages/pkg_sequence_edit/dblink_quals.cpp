/* $Id: dblink_quals.cpp 36594 2016-10-12 20:17:36Z evgeniev $
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
 * File:  spilt dblink quals at commas
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

static string strtmp;
static vector <string> arr;

void CSplitDblinkQuals :: x_ConvertStrToStrs(const string& dt_str, CRef <CUser_field> new_fld)
{
   arr.clear();
   arr = NStr::Split(dt_str, ",", arr);
   CRef <CUser_field::C_Data> new_dt(new CUser_field::C_Data);
   unsigned cnt = 0;
   new_dt->SetStrs().reserve(arr.size());
   ITERATE (vector <string>, it, arr) {
      if (!NStr::TruncateSpaces(*it).empty()) {
           cnt++;
           new_dt->SetStrs().push_back(
               CUtf8::AsUTF8(CTempString(NStr::TruncateSpaces(*it)), eEncoding_UTF8));
      }
   }
   new_fld->SetData(*new_dt);
   new_fld->SetNum(cnt);
};
 
void CSplitDblinkQuals ::x_ConvertStrsToStrs(CRef <CUser_field> fld, CRef <CCmdComposite> composite)
{
   CRef <CUser_field> new_fld(new CUser_field);
   new_fld->SetLabel(fld->SetLabel());
   strtmp = kEmptyStr;
   ITERATE (vector <CStringUTF8>, sit, fld->GetData().GetStrs())
       strtmp += CUtf8::AsSingleByteString(*sit, eEncoding_Ascii) + ",";
   strtmp = strtmp.substr(0, strtmp.size()-1);
   x_ConvertStrToStrs(strtmp, new_fld);
   add_to_cmd<objects::CUser_field> ( 
                    (CObject*)(fld.GetPointer()), (CObject*)(new_fld.GetPointer()), composite);
};

void CSplitDblinkQuals :: x_SplitQuals(const CUser_object& user_obj, CRef <CCmdComposite> composite)
{
   ITERATE (vector <CRef <CUser_field> >, it, user_obj.GetData()) {
      if ((*it)->GetData().IsStr()) {
         strtmp = CUtf8::AsSingleByteString( (*it)->GetData().GetStr(), eEncoding_Ascii);
         if (strtmp.find(",") != string::npos) {
             CRef <CUser_field> new_fld(new CUser_field);
             new_fld->Assign(**it);
             x_ConvertStrToStrs(strtmp, new_fld);
             add_to_cmd<objects::CUser_field> ( 
                 (CObject*)((*it).GetPointer()), (CObject*)(new_fld.GetPointer()), composite);
         }
      }
      else if ( (*it)->GetData().IsStrs()) {
         ITERATE (vector <CStringUTF8>, sit, (*it)->GetData().GetStrs()) {
            strtmp = CUtf8::AsSingleByteString(*sit, eEncoding_Ascii);
            if (strtmp.find(",") != string::npos) {
                  x_ConvertStrsToStrs(*it, composite);
                  break;
            }
         }
      }
   }
};

bool CSplitDblinkQuals :: x_IsDblinkObj(const CUser_object& uobj)
{
   if (uobj.GetType().IsStr() && uobj.GetType().GetStr() == "DBLink") return true;
   else return false;
};

void CSplitDblinkQuals :: apply_to(const CSeq_entry_Handle& tse, CRef <CCmdComposite> composite)
{
  set <const CSeqdesc*> psds;
  for (CBioseq_CI bit(tse); bit; ++bit) {
    for (CSeqdesc_CI it(*bit, CSeqdesc::e_User); it; ++it) {
       if (psds.find(&(*it)) != psds.end()) continue;
       if (x_IsDblinkObj(it->GetUser())) {
            psds.insert(&(*it));
            x_SplitQuals(it->GetUser(), composite);
       }
    }
    for (CFeat_CI it (*bit,  CSeqFeatData::e_User); it; ++it) {
       if (x_IsDblinkObj(it->GetOriginalFeature().GetData().GetUser())) 
            x_SplitQuals(it->GetOriginalFeature().GetData().GetUser(), composite);
    }
  }
};
