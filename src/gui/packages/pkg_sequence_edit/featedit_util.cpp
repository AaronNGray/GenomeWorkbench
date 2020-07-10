/*  $Id: featedit_util.cpp 43676 2019-08-14 14:28:05Z asztalos $
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
 * Authors:  Colleen Bollin, J. Chen
 */


#include <ncbi_pch.hpp>

#include <objects/general/Object_id.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/seqfeat/Feat_id.hpp>
#include <objects/seqfeat/Genetic_code_table.hpp>
#include <objects/seqfeat/Gene_ref.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/RNA_gen.hpp>
#include <objects/seqfeat/Trna_ext.hpp>
#include <objects/seqfeat/SeqFeatXref.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/RNA_qual.hpp>
#include <objects/seqfeat/RNA_qual_set.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objects/seqset/seqset_macros.hpp>
#include <objects/seqloc/Na_strand.hpp>
#include <objects/seqloc/Seq_interval.hpp>

#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seq_feat_handle.hpp>
#include <objmgr/util/sequence.hpp>

#include <objtools/edit/cds_fix.hpp>
#include <objtools/edit/gene_utils.hpp>

#include <util/sequtil/sequtil_convert.hpp>

#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>

#include <gui/packages/pkg_sequence_edit/featedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <gui/packages/pkg_sequence_edit/seq_table_grid.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);
USING_SCOPE(edit);

static const string kIgnoreColumns[] = {kStartColLabel, kStopColLabel, kFeatureID, kProblems };
static const int kNumIgnoreColumns = sizeof (kIgnoreColumns) / sizeof (string);


vector<CConstRef<CObject> > CFeatureSeqTableColumnBase::GetObjects(CBioseq_Handle bsh)
{
    vector<CConstRef<CObject> > objs;
    if (CSeqFeatData::GetTypeFromSubtype(m_Subtype) == CSeqFeatData::e_Prot && !bsh.IsAa()) {
        CFeat_CI f(bsh, SAnnotSelector(CSeqFeatData::e_Cdregion));
        while (f) {
            if (f->IsSetProduct()) {
                CBioseq_Handle p_bsh = bsh.GetScope().GetBioseqHandle(f->GetProduct());
                if (p_bsh) {
                    CFeat_CI p(p_bsh, SAnnotSelector(m_Subtype));
                    while (p) {
                        CConstRef<CObject> object;
                        object.Reset(p->GetOriginalSeq_feat());
                        objs.push_back(object);
                        ++p;
                    }
                }
            } else {
                CConstRef<CObject> object;
                object.Reset(f->GetOriginalSeq_feat());
                objs.push_back(object);
            }
            ++f;
        }
    } else {
        CFeat_CI f(bsh, SAnnotSelector(m_Subtype));
        while (f) {
            CConstRef<CObject> object;
            object.Reset(f->GetOriginalSeq_feat());
            objs.push_back(object);
            ++f;
        }

        if (objs.empty() && m_MajorType != CSeqFeatData::e_not_set) {
            CFeat_CI f(bsh, SAnnotSelector(m_MajorType));
            while (f) {
                CConstRef<CObject> object;
                object.Reset(f->GetOriginalSeq_feat());
                objs.push_back(object);
                ++f;
            }
        }

    }
    return objs;
}


vector<CRef<CApplyObject> > CFeatureSeqTableColumnBase::GetApplyObjects(CBioseq_Handle bsh)
{
    vector<CRef<CApplyObject> > objs;
    if (m_MajorType == CSeqFeatData::e_not_set || m_Subtype == CSeqFeatData::eSubtype_bad) {
        vector<CConstRef<CObject> > feats = GetObjects(bsh);
        ITERATE(vector<CConstRef<CObject> >, it, feats) {
            const CSeq_feat* f = dynamic_cast<const CSeq_feat* >(it->GetPointer());
            if (f) {
                objs.push_back(CRef<CApplyObject>(new CApplyObject(bsh, *f)));
            }
        }
    } else {
        objs = GetRelatedGeneApplyObjects(bsh);
    }
    return objs;
}


vector<CRef<CApplyObject> > CFeatureSeqTableColumnBase::GetRelatedGeneApplyObjects(CBioseq_Handle bsh)
{
    vector<CRef<CApplyObject> > objs;
    vector<CConstRef<CObject> > feats = GetObjects(bsh);
    ITERATE(vector<CConstRef<CObject> >, it, feats) {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat* >(it->GetPointer());
        if (f) {
            CConstRef <CSeq_feat> gene_for_feat = edit::GetGeneForFeature(*f, bsh.GetScope());
            if (gene_for_feat) {
                objs.push_back(CRef<CApplyObject>(new CApplyObject(bsh, *gene_for_feat)));
            } else {
                // create new gene for feature
                CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
                CRef<CSeq_feat> new_feat(new CSeq_feat());
                new_feat->SetLocation().Assign(*(f->GetLocation().Merge(CSeq_loc::fMerge_SingleRange, NULL)));
                new_feat->SetData().SetGene();
                CRef<CObject> new_obj(new_feat.GetPointer());
                objs.push_back(CRef<CApplyObject>(new CApplyObject(seh, CConstRef<CObject>(NULL), new_obj)));
            }
        }
    }
    return objs;
}


// J. Chen
string CDbXrefColumn :: GetFromFeat(const CSeq_feat& feat)
{
   string rval(kEmptyStr);
   if (feat.CanGetDbxref()) {
     ITERATE (vector <CRef <CDbtag> >, it, feat.GetDbxref()) {
       string label(kEmptyStr);
       (*it)->GetLabel(&label);
       if (!label.empty()) {
          rval += label + ";";
       }
     }
     rval = CTempString(rval).substr(0, rval.size()-1);
   }
   else {
     rval = CGbQualColumn :: GetFromFeat(feat);
   }
   return rval; 
};

void CDbXrefColumn :: ClearInFeature(CSeq_feat& feat)
{
    if (feat.IsSetDbxref()) {
        feat.ResetDbxref();
    }
    else {
        CGbQualColumn :: ClearInFeature(feat);
    }
};


string CEvidenceColumn :: GetFromFeat(const CSeq_feat& feat)
{
    if (feat.CanGetExp_ev()) {
        return ((feat.GetExp_ev() == CSeq_feat::eExp_ev_experimental) ?
                "experimental": "non-experimental");
    } else {
        return "";
    }
}


string CRegionColumn :: GetFromFeat(const CSeq_feat& feat)
{
   if (feat.GetData().IsRegion()) {
      return (feat.GetData().GetRegion());
   }
   else return CGbQualColumn :: GetFromFeat(feat);
};

string CLocationColumn :: GetFromFeat(const CSeq_feat& feat)
{
   return "";
   //return (SeqLocPrintUseBestID(feat->GetLocation()));
};

string CCodonsRecColumn :: GetFromFeat(const CSeq_feat& feat)
{
   string rval(kEmptyStr);
   if (feat.GetData().IsRna() 
          && feat.GetData().GetRna().CanGetExt() 
          && feat.GetData().GetRna().GetExt().IsTRNA()
          && feat.GetData().GetRna().GetExt().GetTRNA().CanGetCodon() ) {
      ITERATE (list <int>, it, 
                  feat.GetData().GetRna().GetExt().GetTRNA().GetCodon()) {  
         // should have only 6
         if ( (*it) < 64) {
            rval += CGen_code_table :: IndexToCodon(*it) + ", ";
         }
      }
      rval = CTempString(rval).substr(0, rval.size()-2);
   }
   else {
     rval = CGbQualColumn :: GetFromFeat(feat);
   }
   return rval;
};

string GetIntervalString(const CSeq_interval& seq_int)
{
  bool partial5 = seq_int.IsPartialStart(eExtreme_Biological);
  bool partial3 = seq_int.IsPartialStop(eExtreme_Biological);
  string rval(kEmptyStr);

  if (seq_int.CanGetStrand() && seq_int.GetStrand() == eNa_strand_minus) {
    rval = (string)"complement(" + (partial3 ? "<" : "")
             + NStr::UIntToString(seq_int.GetStart(eExtreme_Positional))
             + ".." + (partial5 ? ">" : "")
             + NStr::UIntToString(seq_int.GetStop(eExtreme_Positional)) + ")";
  } else {
    rval = (partial5 ? "<" : "")
             + NStr::UIntToString(seq_int.GetStart(eExtreme_Positional))
             + ".." + (partial3 ? ">" : "")
             + NStr::UIntToString(seq_int.GetStop(eExtreme_Positional));
  }
  return rval;
};

string CAntiCodonColumn :: GetFromFeat(const CSeq_feat& feat)
{
 string rval(kEmptyStr);
 if (feat.GetData().IsRna()
          && feat.GetData().GetRna().CanGetExt()
          && feat.GetData().GetRna().GetExt().IsTRNA()
          && feat.GetData().GetRna().GetExt().GetTRNA().CanGetAnticodon() ) {
   const CTrna_ext& trna = feat.GetData().GetRna().GetExt().GetTRNA();
   if (trna.GetAnticodon().IsInt()) {
         rval = GetIntervalString(trna.GetAnticodon().GetInt());
   }
   else if (trna.GetAnticodon().IsMix()) {
     ITERATE (list <CRef <CSeq_loc> >, it, trna.GetAnticodon().GetMix().Get()) {
       if ( (*it)->IsInt()) {
           rval += GetIntervalString( (*it)->GetInt() ) + ", ";
       }
       else return( "complex location" );
     }
     rval = CTempString(rval).substr(0, rval.size() - 2);
   }
 }
 else {
    rval = CGbQualColumn :: GetFromFeat(feat);
 }
 return rval;

};

string CTagPeptideColumn :: GetFromFeat(const CSeq_feat& feat) 
{
  string rval;
  if (feat.GetData().IsRna()
         && feat.GetData().GetRna().CanGetExt()
         && feat.GetData().GetRna().GetExt().IsGen()
         && feat.GetData().GetRna().GetExt().GetGen().CanGetQuals()) {
     ITERATE (list <CRef <CRNA_qual> >, it,
                    feat.GetData().GetRna().GetExt().GetGen().GetQuals().Get()){
        if ((*it)->GetQual() == "tag_peptide" && !((*it)->GetVal()).empty()) {
           rval += (*it)->GetVal() + ";";
        }
     }
     rval = CTempString(rval).substr(0, rval.size()-1);
  }
  else {
    rval = CGbQualColumn :: GetFromFeat(feat);
  }
  return rval;
};

string CCitationColumn :: GetFromFeat(const CSeq_feat& feat)
{
  return kEmptyStr;
};


// J. Chen end

CRef<CFeatureSeqTableColumnBase>
CFeatureSeqTableColumnBaseFactory::Create(const string& sTitle, CSeqFeatData::ESubtype subtype)
{
    string member_name = sTitle;
    size_t pos = NStr::Find(member_name, "\n");
    if (pos != string::npos) {
        member_name = member_name.substr(0, pos);
    }
    
    CSeqFeatData::E_Choice major_type = CSeqFeatData::e_not_set;

    if (NStr::StartsWith(member_name, "gene ") 
           || NStr::StartsWith(member_name, "gene-")) {
        member_name = member_name.substr(5);
        major_type = CSeqFeatData::GetTypeFromSubtype(subtype);
        subtype = CSeqFeatData::eSubtype_gene;
    } else if (NStr::StartsWith(member_name, "mRNA ", NStr::eNocase)) {
        member_name = member_name.substr(5);
    }
    CRef<CFeatureSeqTableColumnBase> rval(NULL);

    if (QualifierNamesAreEquivalent(member_name, "Comment") 
            || NStr::EqualNocase(sTitle, "Note")) {
        rval = CRef<CFeatureSeqTableColumnBase>( new CCommentColumn());
    } else if (QualifierNamesAreEquivalent(member_name, "Exception")) {
        rval = CRef<CFeatureSeqTableColumnBase>( new CExceptionColumn());
    } else if (QualifierNamesAreEquivalent(member_name, kPartialStart)) {
        rval = CRef<CFeatureSeqTableColumnBase>( new CPartialStartColumn());
    } else if (QualifierNamesAreEquivalent(member_name, kPartialStop)) {
        rval = CRef<CFeatureSeqTableColumnBase>( new CPartialStopColumn());
    } else if (QualifierNamesAreEquivalent(member_name, "rpt_unit_seq")) {
        rval = CRef<CFeatureSeqTableColumnBase>( new CRptUnitSeqColumn());
    } 
    else if (NStr::EqualNocase(member_name, "db-xref") || NStr::EqualNocase(member_name, "db_xref")) {
       rval = CRef<CFeatureSeqTableColumnBase>( new CDbXrefColumn());
    }
    else if (NStr::EqualNocase(member_name, "evidence")) {
       rval = CRef<CFeatureSeqTableColumnBase>( new CEvidenceColumn());
    }
    else if (NStr::EqualNocase(member_name, "citation")) {
       rval = CRef<CFeatureSeqTableColumnBase>( new CCitationColumn());
        // don't know how 
    }
    else if (NStr::EqualNocase(member_name, "inference")) {
        rval = CRef<CFeatureSeqTableColumnBase>( new CInferenceColumn());
    }
    else if (NStr::EqualNocase(member_name, "location")) {
       rval = CRef<CFeatureSeqTableColumnBase>( new CLocationColumn());
         // move SeqLocPrintUseBestId to seq_loc_util
    }
    else if (NStr::EqualNocase(member_name, "pseudo")) {
       rval =CRef<CFeatureSeqTableColumnBase>( new CGbQualColumn("pseudogene"));
    }
    else if (NStr::EqualNocase(member_name, "codons-recognized")) {
       rval = CRef<CFeatureSeqTableColumnBase>( new CCodonsRecColumn());
    }
    else if (NStr::EqualNocase(member_name, "anticodon")) {
       rval = CRef<CFeatureSeqTableColumnBase>( new CAntiCodonColumn());
    }
    else if (NStr::EqualNocase(member_name, "tag-peptide")) {
       rval = CRef<CFeatureSeqTableColumnBase>( new CTagPeptideColumn());
    }
    else if (NStr::EqualNocase(member_name, "region")) {
       rval = CRef<CFeatureSeqTableColumnBase>( new CRegionColumn());
    }
    else if (QualifierNamesAreEquivalent(member_name, "codon-start")) {
        rval = CRef<CFeatureSeqTableColumnBase>( new CCodonStartColumn());
    }
    else if (QualifierNamesAreEquivalent(member_name, "transl-table")) {
        rval = CRef<CFeatureSeqTableColumnBase>( new CGeneticCodeColumn());
    }
    else if (NStr::StartsWith(member_name, "satellite")) {
        rval = CRef<CFeatureSeqTableColumnBase>(new CSatelliteColumn(member_name));
    }
    else {
        for (int i = 0; i < kNumIgnoreColumns; i++) {
            if (QualifierNamesAreEquivalent(member_name, kIgnoreColumns[i])) {
                rval = CRef<CFeatureSeqTableColumnBase>( NULL);
                break;
            }
        }
        if (!rval) {    
            // is this a product field?
            bool is_protein = QualifierNamesAreEquivalent(member_name, "protein name") || NStr::EqualNocase(member_name, "name");
            if (NStr::EndsWith(member_name, "product") || is_protein) {
                rval = CRef<CFeatureSeqTableColumnBase>( new CProductColumn());
                if (subtype == CSeqFeatData::eSubtype_cdregion || 
                    (is_protein && 
                     CSeqFeatData::GetTypeFromSubtype(subtype) != CSeqFeatData::e_Prot)) {
                    subtype = CSeqFeatData::eSubtype_prot;
                }
            }
        }

        if (!rval) {
            // is this a description? could be protein or gene
            if (NStr::EndsWith(member_name, "description") || NStr::EndsWith(sTitle, "desc")) {
                rval = CRef<CFeatureSeqTableColumnBase>( new CDescriptionColumn());
            }
        }

        if (!rval) {
            if (QualifierNamesAreEquivalent(member_name, "ncRNA class")) {
                rval = CRef<CFeatureSeqTableColumnBase>( new CNcrnaClassColumn());
            }
        }

        if (!rval) {
            // gene fields
            if (CGeneRefColumn::IsGeneRefField(member_name)) {
                rval = CRef<CFeatureSeqTableColumnBase>( new CGeneRefColumn(member_name));
            }
        }

        if (!rval) {
            // protein fields
            if (CProtRefColumn::IsProtRefField(member_name)) {
                rval = CRef<CFeatureSeqTableColumnBase>( new CProtRefColumn(member_name));
                if (CSeqFeatData::GetTypeFromSubtype(subtype) != CSeqFeatData::e_Prot) {
                    subtype = CSeqFeatData::eSubtype_prot;
                }
            }
        }

        if (!rval) {
            // if nothing else is found, create a GBQual
            rval = CRef<CFeatureSeqTableColumnBase>( new CGbQualColumn(member_name) );
        }
    }
    if (rval) {
        rval->SetSubtype(subtype);
        if (major_type != CSeqFeatData::e_not_set) {
            rval->SetMajorType(major_type);
        }
    }
    return rval;
}


bool CPartialStartColumn::AddToFeature( CSeq_feat & in_out_feat, const string & newValue,
    EExistingText existing_text) 
{
    if (NStr::IsBlank(newValue)) {
        in_out_feat.SetLocation().SetPartialStart(false, eExtreme_Biological);
    } else {
        in_out_feat.SetLocation().SetPartialStart(true, eExtreme_Biological);
    }
    if (in_out_feat.GetLocation().IsPartialStart(eExtreme_Biological)
        || in_out_feat.GetLocation().IsPartialStop(eExtreme_Biological)) {
        in_out_feat.SetPartial(true);
    } else {
        in_out_feat.ResetPartial();
    }
    return true;
}
  

void CPartialStartColumn::ClearInFeature( CSeq_feat & in_out_feat ) 
{
    in_out_feat.SetLocation().SetPartialStart(false, eExtreme_Biological);
    if (in_out_feat.GetLocation().IsPartialStart(eExtreme_Biological)
        || in_out_feat.GetLocation().IsPartialStop(eExtreme_Biological)) {
        in_out_feat.SetPartial(true);
    } else {
        in_out_feat.ResetPartial();
    }
}


string CPartialStartColumn::GetFromFeat( const CSeq_feat & in_out_feat ) 
{
    if (in_out_feat.GetLocation().IsPartialStart(eExtreme_Biological)) {
        return "true";
    } else {
        return "";
    }
}


bool CPartialStopColumn::AddToFeature( CSeq_feat & in_out_feat, const string & newValue,
    EExistingText existing_text) 
{
    if (NStr::IsBlank(newValue)) {
        in_out_feat.SetLocation().SetPartialStop(false, eExtreme_Biological);
    } else {
        in_out_feat.SetLocation().SetPartialStop(true, eExtreme_Biological);
    }
    if (in_out_feat.GetLocation().IsPartialStart(eExtreme_Biological)
        || in_out_feat.GetLocation().IsPartialStop(eExtreme_Biological)) {
        in_out_feat.SetPartial(true);
    } else {
        in_out_feat.ResetPartial();
    }
    return true;
}
   

void CPartialStopColumn::ClearInFeature( CSeq_feat & in_out_feat ) 
{
    in_out_feat.SetLocation().SetPartialStop(false, eExtreme_Biological);
    if (in_out_feat.GetLocation().IsPartialStart(eExtreme_Biological)
        || in_out_feat.GetLocation().IsPartialStop(eExtreme_Biological)) {
        in_out_feat.SetPartial(true);
    } else {
        in_out_feat.ResetPartial();
    }
}


string CPartialStopColumn::GetFromFeat( const CSeq_feat & in_out_feat ) 
{
    if (in_out_feat.GetLocation().IsPartialStop(eExtreme_Biological)) {
        return "true";
    } else {
        return "";
    }
}


bool CExceptionColumn::AddToFeature( CSeq_feat & in_out_feat, const string & newValue,
    EExistingText existing_text) 
{
    if (NStr::IsBlank (newValue)) {
        return false;
    } else {
        in_out_feat.SetExcept(true);
        string current = kEmptyStr;
        if (in_out_feat.IsSetExcept_text()) {
            current = in_out_feat.GetExcept_text();
        }
        if (AddValueToString(current, newValue, existing_text)) {
            in_out_feat.SetExcept_text(current);
            return true;
        }
    }
    return false;
}


void CExceptionColumn::ClearInFeature( CSeq_feat & in_out_feat )
{
    in_out_feat.ResetExcept();
    in_out_feat.ResetExcept_text(); 
}


string CExceptionColumn::GetFromFeat( const CSeq_feat & in_out_feat )
{
    if (in_out_feat.IsSetExcept() && in_out_feat.GetExcept() && in_out_feat.IsSetExcept_text()) {
        return in_out_feat.GetExcept_text();
    } else {
        return "";
    }
}


bool CCommentColumn::AddToFeature( CSeq_feat & in_out_feat, const string & newValue,
    EExistingText existing_text) 
{ 
    if (NStr::IsBlank(newValue)) {
        return false;
    } else {
        string current = kEmptyStr;
        if (in_out_feat.IsSetComment()) {
            current = in_out_feat.GetComment();
        }
        if (AddValueToString(current, newValue, existing_text)) {
            in_out_feat.SetComment(current); 
            return true;
        }
    }
    return false;
}

void CCommentColumn::ClearInFeature( CSeq_feat & in_out_feat ) 
{ 
    in_out_feat.ResetComment(); 
}

string CCommentColumn::GetFromFeat( const CSeq_feat & in_out_feat ) 
{ 
    if (in_out_feat.IsSetComment()) {
        return in_out_feat.GetComment();
    } else {
        return "";
    }
}


bool CGbQualColumn::AddToFeature(CSeq_feat & in_out_feat, const string & newValue,
    EExistingText existing_text)
{	
    if (NStr::IsBlank(newValue)) {
        return false;
    }

    bool rval = false;
    string current = kEmptyStr;
    NON_CONST_ITERATE (CSeq_feat::TQual, iter, in_out_feat.SetQual()) {
        if (NStr::EqualNocase((*iter)->GetQual(), m_QualName) && (*iter)->IsSetVal()) {
            current = (*iter)->GetVal();
            if (AddValueToString(current, newValue, existing_text)) {
                (*iter)->SetVal(current);
                rval = true;
            }
        }
    }
    if (!rval) {
        CRef<CGb_qual> qual(new CGb_qual(m_QualName, newValue));
        in_out_feat.SetQual().push_back(qual);
        rval = true;
    }
    return rval;
    
}
    


void CGbQualColumn::ClearInFeature (CSeq_feat & in_out_feat )
{
    if (in_out_feat.IsSetQual()) {
        CSeq_feat::TQual::iterator it = in_out_feat.SetQual().begin();
        while (it != in_out_feat.SetQual().end()) {
            if ((*it)->IsSetQual() && NStr::EqualNocase((*it)->GetQual(), m_QualName)) {
                it = in_out_feat.SetQual().erase(it);
            } else {
                it++;
            }
        }
    }
}


string CGbQualColumn::GetFromFeat(const CSeq_feat & in_out_feat )
{
    string val = "";
    if (in_out_feat.IsSetQual()) {
        ITERATE (CSeq_feat::TQual, it, in_out_feat.GetQual()) {
            if ((*it)->IsSetQual() && NStr::EqualNocase((*it)->GetQual(), m_QualName)
                && (*it)->IsSetVal()) {
                val += (*it)->GetVal() + ";";
            }
        }
        if (!NStr::IsBlank(val)) {
            val = val.substr(0, val.length() - 1);
        }
    }
    return val;            
}


CRptUnitRangeColumn::CRptUnitRangeColumn(const vector<size_t> lens) 
  : CGbQualColumn("rpt_unit_range"), m_Lens(lens)
{
}


vector<string> CRptUnitRangeColumn::IsValid(const vector<string>& values)
{
    vector<string> problems;
    bool any_errors = false;
    size_t pos = 0;
    ITERATE(vector<string>, it, values) {
        string error = "";
        if (NStr::IsBlank(*it)) {
            // blank is always allowed
            problems.push_back("");
            continue;
        }
        size_t div = NStr::Find(*it, "..");
        if (div == string::npos) {
            error = "rpt_unit_range has invalid format";
        } else {
            string start_str = (*it).substr(0, div);
            string end_str = (*it).substr(div + 2);
            int range_start = 1;
            int range_end = m_Lens[pos];
            try {
                range_start = NStr::StringToInt(start_str);
                range_end = NStr::StringToInt(end_str);
            } catch (CException&) {
                error = "rpt_unit_range has invalid format";
            }
            if (pos < m_Lens.size()) {
                if (range_start < 1 || range_end < 1) {
                    error = "rpt_unit_range start and end values must be between 1 and sequence length";
                } else if ((size_t)range_start > m_Lens[pos]) {
                    error = "rpt_unit_range start exceeds sequence length";
                } else if ((size_t)range_end > m_Lens[pos]) {
                    error = "rpt_unit_range end exceeds sequence length";
                }
            }
        }
        pos++;
        if (!NStr::IsBlank(error)) {
            any_errors = true;
        }
        problems.push_back(error);
    }

    if (!any_errors) {
        problems.clear();
    }
    return problems;
}


CRptUnitSeqColumn::CRptUnitSeqColumn() : CGbQualColumn("rpt_unit_seq")
{
}


static bool IsAllRptUnitSeqChars(string seq)
{
    ITERATE(string, it, seq) {
        if (!isdigit(*it) && !isalpha(*it) && *it != '(' && *it != ')' && *it != ',' && *it != ';') {
            return false;
        }
    }
    return true;
}


vector<string> CRptUnitSeqColumn::IsValid(const vector<string>& values)
{
    vector<string> problems;
    bool any_errors = false;
    ITERATE(vector<string>, it, values) {
        if (!IsAllRptUnitSeqChars(*it)) {
            problems.push_back("rpt_unit_seq value contains invalid characters");
            any_errors = true;
        } else {
            problems.push_back("");
        }
    }
    if (!any_errors) {
        problems.clear();
    }
    return problems;
}

const string kSatellite = "satellite";

CSatelliteColumn::CSatelliteColumn(const string& satellite_field)
    : CGbQualColumn(kSatellite), m_Field(eSatellite)
{
    if (QualifierNamesAreEquivalent(satellite_field, "satellite-type")) {
        m_Field = eSatelliteType;
    }
    else if (QualifierNamesAreEquivalent(satellite_field, "satellite-name")) {
        m_Field = eSatelliteName;
    }
}

bool CSatelliteColumn::AddToFeature(CSeq_feat & in_out_feat, const string & newValue, EExistingText existing_text)
{
    if (NStr::IsBlank(newValue)) {
        return false;
    }

    switch (m_Field) {
    case (eSatellite) : {
        const string prefixed_value = x_AddSatellitePrefix(newValue, existing_text);
        return CGbQualColumn::AddToFeature(in_out_feat, prefixed_value, existing_text);
    }
    case (eSatelliteType) :
        return x_AddSatelliteType(in_out_feat, newValue, existing_text);
    case (eSatelliteName):
        return x_AddSatelliteName(in_out_feat, newValue, existing_text);
    }
    return false;
}

string CSatelliteColumn::x_AddSatellitePrefix(const string& value, EExistingText existing_text) const
{
    string newValue(value);
    if (existing_text == eExistingText_replace_old) {
        bool type_added(false);
        for (size_t i = 0; i < kNumSatelliteTypes; i++) {
            if (NStr::StartsWith(value, kSatelliteTypes[i], NStr::eNocase)) {
                type_added = true;
            }
        }
        if (!type_added) {
            newValue = kSatellite + ":" + value;
        }
    }
    return newValue;
}

bool CSatelliteColumn::x_AddSatelliteType(CSeq_feat & in_out_feat, const string & newValue, EExistingText existing_text)
{
    // allow adding only satellite types defined in kSatelliteTypes array
    string value(newValue);
    NStr::TruncateSpacesInPlace(value);
    bool proper_type(false);
    for (size_t i = 0; i < kNumSatelliteTypes; i++) {
        if (NStr::EqualNocase(value, kSatelliteTypes[i])) {
            proper_type = true;
            break;
        }
    }
    if (!proper_type) {
        return false;
    }
    
    bool rval = false;
    string current_type, current_name;
    NON_CONST_ITERATE(CSeq_feat::TQual, iter, in_out_feat.SetQual()) {
        if (NStr::EqualNocase((*iter)->GetQual(), kSatellite) && (*iter)->IsSetVal()) {
            current_type = x_GetSatelliteType((*iter)->GetVal());
            current_name = x_GetSatelliteName((*iter)->GetVal());
            if (AddValueToString(current_type, value, existing_text)) {
                (*iter)->SetVal(current_type + ":" + current_name);
                rval = true;
            }
        }
    }
    if (!rval) {
        CRef<CGb_qual> qual(new CGb_qual(kSatellite, value));
        in_out_feat.SetQual().push_back(qual);
        rval = true;
    }
    return rval;
}

bool CSatelliteColumn::x_AddSatelliteName(CSeq_feat & in_out_feat, const string & newValue, EExistingText existing_text)
{
    bool rval = false;
    string current_type, current_name;
    NON_CONST_ITERATE(CSeq_feat::TQual, iter, in_out_feat.SetQual()) {
        if (NStr::EqualNocase((*iter)->GetQual(), kSatellite) && (*iter)->IsSetVal()) {
            current_type = x_GetSatelliteType((*iter)->GetVal());
            current_name = x_GetSatelliteName((*iter)->GetVal());
            if (AddValueToString(current_name, newValue, existing_text)) {
                (*iter)->SetVal(current_type + ":" + current_name);
                rval = true;
            }
        }
    }
    if (!rval) {
        string prefixed_value = x_AddSatellitePrefix(newValue, eExistingText_replace_old);
        CRef<CGb_qual> qual(new CGb_qual(kSatellite, prefixed_value));
        in_out_feat.SetQual().push_back(qual);
        rval = true;
    }
    return rval;
}

void CSatelliteColumn::ClearInFeature(CSeq_feat & in_out_feat)
{
    if (!in_out_feat.IsSetQual()) {
        return;
    }
    if (m_Field == eSatellite) {
        return CGbQualColumn::ClearInFeature(in_out_feat);
    }
    x_ClearSatelliteSubFields(in_out_feat);
}

void CSatelliteColumn::x_ClearSatelliteSubFields(CSeq_feat & in_out_feat) const
{
    CSeq_feat::TQual::iterator it = in_out_feat.SetQual().begin();
    while (it != in_out_feat.SetQual().end()) {
        if ((*it)->IsSetQual() && NStr::EqualNocase((*it)->GetQual(), kSatellite)) {
            if (!(*it)->IsSetVal()) {
                it = in_out_feat.SetQual().erase(it);
            } else {
                string newValue;
                if (m_Field == eSatelliteType) {
                    newValue = x_GetSatelliteName((*it)->GetVal());
                } else if (m_Field == eSatelliteName) {
                    newValue = x_GetSatelliteType((*it)->GetVal());
                } 
                if (newValue.empty()) {
                    it = in_out_feat.SetQual().erase(it);
                } else {
                    (*it)->SetVal(newValue);
                    ++it;
                }
            }
        }
        else {
            ++it;
        }
    }
}

string CSatelliteColumn::GetFromFeat(const CSeq_feat & in_out_feat)
{
    string field = CGbQualColumn::GetFromFeat(in_out_feat);
    switch (m_Field) {
    case (eSatellite) :
        return field;
    case (eSatelliteType) :
        return x_GetSatelliteType(field);
    case (eSatelliteName) :
        return x_GetSatelliteName(field);
    }

    return kEmptyStr;
}

string CSatelliteColumn::x_GetSatelliteType(const string& field) const
{
    SIZE_TYPE pos = NStr::FindNoCase(field, ":");
    if (pos != NPOS) {
        return field.substr(0, pos);
    }

    for (size_t i = 0; i < kNumSatelliteTypes; i++) {
        if (NStr::EqualNocase(field, kSatelliteTypes[i])) {
            return field;
        }
    }
    return kEmptyStr;
}

string CSatelliteColumn::x_GetSatelliteName(const string& field) const
{
    SIZE_TYPE pos = NStr::FindNoCase(field, ":");
    if (pos != NPOS) {
        return field.substr(pos + 1, NPOS);
    }

    for (size_t i = 0; i < kNumSatelliteTypes; i++) {
        if (NStr::EqualNocase(field, kSatelliteTypes[i])) {
            return kEmptyStr;
        }
    }
    return field;
}


bool CSatelliteTypeNameColumn::AddToFeature(CSeq_feat & in_out_feat, const string & newValue, EExistingText existing_text)
{    
    string val = m_SatelliteType;
    if (!NStr::IsBlank(newValue)) {
        val += ": " + newValue;
    }
    CRef<CGb_qual> qual(new CGb_qual(kSatellite,  val));
    in_out_feat.SetQual().push_back(qual);
    return true;
}

bool CSatelliteTypeNameColumn::x_IsQualAMatch (const CGb_qual& qual) const
{
    if (qual.IsSetQual() && NStr::EqualNocase(qual.GetQual(), kSatellite)
        && qual.IsSetVal() && NStr::StartsWith(qual.GetVal(), m_SatelliteType, NStr::eNocase)) {
        return true;
    } else {
        return false;
    }
}

        
void CSatelliteTypeNameColumn::ClearInFeature (CSeq_feat & in_out_feat )
{
    if (in_out_feat.IsSetQual()) {
        CSeq_feat::TQual::iterator it = in_out_feat.SetQual().begin();
        while (it != in_out_feat.SetQual().end()) {
            if (x_IsQualAMatch(**it)) {
                it = in_out_feat.SetQual().erase(it);
            } else {
                it++;
            }
        }
    }
}


string CSatelliteTypeNameColumn::GetFromFeat(const CSeq_feat & in_out_feat )
{
    string val = "";
    if (in_out_feat.IsSetQual()) {
        ITERATE (CSeq_feat::TQual, it, in_out_feat.GetQual()) {
            if (x_IsQualAMatch(**it)) {
                val += (*it)->GetVal() + ";";
            }
        }
        if (!NStr::IsBlank(val)) {
            val = val.substr(0, val.length() - 1);
        }
    }
    return val;            
}


bool CSatelliteTypeNameColumn::RelaxedMatch(const CSeq_feat & f1, const CSeq_feat &f2) const
{
    if (!f1.IsSetQual() || !f2.IsSetQual()) {
        return false;
    }
    bool yes1 = false;
    ITERATE (CSeq_feat::TQual, it, f1.GetQual()) {
        if (x_IsQualAMatch(**it)) {
            yes1 = true;
            break;
        }
    }
    if (!yes1) {
        return false;
    }
    bool yes2 = false;
    ITERATE (CSeq_feat::TQual, it, f2.GetQual()) {
        if (x_IsQualAMatch(**it)) {
            yes2 = true;
            break;
        }
    }
    return yes2;
}


vector<string> FindNonUniqueStrings(const vector<string>& values)
{    
    vector<string> sorted;
    sorted.assign(values.begin(), values.end());
    stable_sort(sorted.begin(), sorted.end());

    if (sorted.size() < 2) {
        sorted.clear();
        return sorted;
    }

    vector<string>::iterator it_p = sorted.begin();
    vector<string>::iterator it_n = it_p;
    int match_count = 0;
    it_n++;
    while (it_n != sorted.end()) {
        if (NStr::Equal(*it_p, *it_n)) {
            it_n = sorted.erase(it_n);
            match_count++;
        } else {
            if (match_count == 0) {
                it_p = sorted.erase(it_p);
            } else {
                it_p++;
            }
            match_count = 0;
            it_n = it_p;
            it_n++;
        }
    }
    if (match_count == 0) {
        // last item had no duplicates
        sorted.pop_back();
    }
    return sorted;
}


vector<string> SortUniqueStrings(const vector<string>& values)
{    
    vector<string> sorted;
    sorted.assign(values.begin(), values.end());
    stable_sort(sorted.begin(), sorted.end());

    if (sorted.size() < 2) {
        return sorted;
    }

    vector<string>::iterator it_p = sorted.begin();
    vector<string>::iterator it_n = it_p;
    it_n++;
    while (it_n != sorted.end()) {
        if (NStr::Equal(*it_p, *it_n)) {
            it_n = sorted.erase(it_n);
        } else {
            it_p++;
            it_n = it_p;
            it_n++;
        }
    }
    return sorted;
}


vector<string> CSatelliteTypeNameColumn::IsValid(const vector<string>& values)
{
    vector<string> problems;
    if (!m_RequireUniqueness) {
        return problems;
    }
    bool any_errors = false;
    vector<string> sorted = FindNonUniqueStrings(values);

    ITERATE(vector<string>, it, values) {
        if (NStr::IsBlank(*it)) {
            problems.push_back("missing " + GetLabel());
            any_errors = true;
        } else {
            bool match = false;
            ITERATE(vector<string>, sit, sorted) {
                if (NStr::Equal(*it, *sit)) {
                    match = true;
                    break;
                }
            }
            if (match) {
                problems.push_back("duplicate " + GetLabel());
                any_errors = true;
            } else {
                problems.push_back("");
            }
        }
    }
    if (!any_errors) {
        problems.clear();
    }
    return problems;
}


void SetRnaProductName( CRNA_ref & rna, const string& product, 
    string& remainder, EExistingText existing_text )
{
    if (NStr::IsBlank(product)) {
        rna.SetRnaProductName(product, remainder);
    } else {
        string current = rna.GetRnaProductName();
        if (AddValueToString(current, product, existing_text)) {
            rna.SetRnaProductName(current, remainder);
        }
    }
}


vector<CRef<CApplyObject> > CProductColumn::GetApplyObjects(CBioseq_Handle bsh)
{
    vector<CRef<CApplyObject> > objs;
    if (CSeqFeatData::GetTypeFromSubtype(m_Subtype) == CSeqFeatData::e_Prot) {
        if (bsh.IsAa()) {
            vector<CConstRef<CObject> > feats = GetObjects(bsh);
            ITERATE(vector<CConstRef<CObject> >, it, feats) {
                const CSeq_feat* f = dynamic_cast<const CSeq_feat* >(it->GetPointer());
                if (f) {
                    objs.push_back(CRef<CApplyObject>(new CApplyObject(bsh, *f)));
                }
            }
            if (objs.empty() && CSeqFeatData::GetTypeFromSubtype(m_Subtype) == CSeqFeatData::e_Prot) {
                if (bsh.IsAa()) {
                    CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
                    CRef<CSeq_feat> new_feat(new CSeq_feat());
                    CRef<CSeq_id> id(new CSeq_id());
                    id->Assign(*(bsh.GetId().front().GetSeqId()));
                    new_feat->SetLocation().SetInt().SetId(*id);
                    new_feat->SetLocation().SetInt().SetFrom(0);
                    new_feat->SetLocation().SetInt().SetTo(bsh.GetInst_Length() - 1);
                    new_feat->SetData().SetProt();
                    const CSeq_feat* cds = sequence::GetCDSForProduct(bsh);
                    if (cds) {
                        edit::AdjustProteinFeaturePartialsToMatchCDS(*new_feat, *cds);
                    }
                    CRef<CObject> new_obj(new_feat.GetPointer());
                    objs.push_back(CRef<CApplyObject>(new CApplyObject(seh, CConstRef<CObject>(NULL), new_obj)));
                }
            }
        } else {
            CFeat_CI f(bsh, SAnnotSelector(CSeqFeatData::e_Cdregion));
            while (f) {
                if (f->IsSetProduct()) {
                    CBioseq_Handle p_bsh = bsh.GetScope().GetBioseqHandle(f->GetProduct());
                    if (p_bsh) {
                        vector<CRef<CApplyObject> > p_objs = GetApplyObjects(p_bsh);
                        objs.insert(objs.end(), p_objs.begin(), p_objs.end());
                    }
                } else {
                    objs.push_back(CRef<CApplyObject>(new CApplyObject(bsh, *(f->GetOriginalSeq_feat()))));
                }
                ++f;
            }
        }
    } else {
        // not a protein feature
        vector<CConstRef<CObject> > feats = GetObjects(bsh);
        ITERATE(vector<CConstRef<CObject> >, it, feats) {
            const CSeq_feat* f = dynamic_cast<const CSeq_feat* >(it->GetPointer());
            if (f) {
                objs.push_back(CRef<CApplyObject>(new CApplyObject(bsh, *f)));
            }
        }
    }  
    return objs;
}


bool CProductColumn::AddToFeature( CSeq_feat & in_out_feat, const string & newValue,
    EExistingText existing_text) 
{
    if (in_out_feat.GetData().IsRna()) {
        string remainder;
        SetRnaProductName(in_out_feat.SetData().SetRna(), newValue, remainder, existing_text);
        if (!NStr::IsBlank(remainder)) {
            if (in_out_feat.IsSetComment() && !NStr::IsBlank(in_out_feat.GetComment())) {
                in_out_feat.SetComment(in_out_feat.GetComment() + "; " + remainder);
            } else {
                in_out_feat.SetComment(remainder);
            }
        }
    } else {
        if (in_out_feat.GetData().Which() == CSeqFeatData::e_Prot) {
            if (in_out_feat.GetData().GetProt().IsSetName()) {
                if (in_out_feat.GetData().GetProt().GetName().size() > 0) {
                    string current = in_out_feat.GetData().GetProt().GetName().front();
                    if (AddValueToString(current, newValue, existing_text)){
                        in_out_feat.SetData().SetProt().SetName().front() = current;
                    }
                } else {
                    in_out_feat.SetData().SetProt().SetName().push_back(newValue);
                }
            } else {
                if (!NStr::IsBlank(newValue)) {
                    in_out_feat.SetData().SetProt().SetName().push_back(newValue);
                }
            }
        } else {
            return CGbQualColumn::AddToFeature (in_out_feat, newValue, existing_text);
        }
    }
    return true;
}


void CProductColumn::ClearInFeature( CSeq_feat & in_out_feat ) 
{
    if (in_out_feat.GetData().IsRna()) {
        string remainder;
        SetRnaProductName(in_out_feat.SetData().SetRna(), "", remainder, eExistingText_replace_old);
    } else {
        if (in_out_feat.GetData().GetProt().IsSetName()) {
            in_out_feat.SetData().SetProt().ResetName();
        } else {
            CGbQualColumn::ClearInFeature (in_out_feat);
        }
    }
}

string CProductColumn::GetFromFeat( const CSeq_feat & in_out_feat ) 
{
    string val = "";
    if (in_out_feat.GetData().IsRna()) {
        val = in_out_feat.GetData().GetRna().GetRnaProductName();
    } else  if (in_out_feat.GetData().IsProt()) {
        if (in_out_feat.GetData().GetProt().IsSetName() && !in_out_feat.GetData().GetProt().GetName().empty() ) {
            val = in_out_feat.GetData().GetProt().GetName().front();
        }
    } else {
        val = CGbQualColumn::GetFromFeat (in_out_feat);
    }
    return val;
}


vector<CRef<CApplyObject> > CCodonStartColumn::GetApplyObjects(CBioseq_Handle bsh)
{
    m_Scope.Reset(&(bsh.GetScope()));
    vector<CRef<CApplyObject> > objs;
    if (CSeqFeatData::GetTypeFromSubtype(m_Subtype) == CSeqFeatData::e_Prot) {
        const CSeq_feat* cds = sequence::GetCDSForProduct(bsh);
        if (cds) {
            CBioseq_Handle n_bsh = bsh.GetScope().GetBioseqHandle(cds->GetLocation());
            if (n_bsh) {
                objs.push_back(CRef<CApplyObject>(new CApplyObject(n_bsh, *cds)));
            }
        }
    } else {
        objs = CFeatureSeqTableColumnBase::GetApplyObjects(bsh);
    }  
    return objs;
}


bool CCodonStartColumn::AddToFeature( CSeq_feat & in_out_feat, const string & newValue,
    EExistingText existing_text) 
{
    bool rval = false;
    if (in_out_feat.GetData().IsCdregion() && m_Scope) {
        edit::ApplyCDSFrame::ECdsFrame f = edit::ApplyCDSFrame::s_GetFrameFromName(newValue);
        if (f != edit::ApplyCDSFrame::eNotSet) {
            edit::ApplyCDSFrame::s_SetCDSFrame(in_out_feat, f, *m_Scope);
            rval = true;
        }
    } 
    if (!rval) {
        rval = CGbQualColumn::AddToFeature (in_out_feat, newValue, existing_text);
    }
    return rval;
}


void CCodonStartColumn::ClearInFeature( CSeq_feat & in_out_feat ) 
{
    if (in_out_feat.GetData().IsCdregion()) {
        in_out_feat.SetData().SetCdregion().ResetFrame();
    } else {
        CGbQualColumn::ClearInFeature (in_out_feat);
    }
}


string CCodonStartColumn::GetFromFeat( const CSeq_feat & in_out_feat )
{
    string val = "";
    if (in_out_feat.GetData().IsCdregion()) {
        if (in_out_feat.GetData().GetCdregion().IsSetFrame()) {
            switch (in_out_feat.GetData().GetCdregion().GetFrame()) {
                case CCdregion::eFrame_one:
                case CCdregion::eFrame_not_set:
                    val = "1";
                    break;
                case CCdregion::eFrame_two:
                    val = "2";
                    break;
                case CCdregion::eFrame_three:
                    val = "3";
                    break;
            }
        } else {
            val = "1";
        }
    } else {
        val = CGbQualColumn::GetFromFeat(in_out_feat);
    }
    return val;
}

bool CGeneticCodeColumn::AddToFeature(CSeq_feat & in_out_feat, const string & newValue,
    EExistingText existing_text)
{
    try {
        int gc = NStr::StringToInt(newValue, NStr::fConvErr_NoThrow | NStr::fAllowLeadingSpaces | NStr::fAllowTrailingSpaces);
        if (gc > 0 && in_out_feat.GetData().IsCdregion()) {
            in_out_feat.SetData().SetCdregion().SetCode().SetId(gc);
            return true;
        }
    }
    catch(const CException&)
    {
        return CGbQualColumn::AddToFeature(in_out_feat, newValue, existing_text);
    }
    catch (const exception&)
    {
        return CGbQualColumn::AddToFeature(in_out_feat, newValue, existing_text);
    }
    return false;
}

void CGeneticCodeColumn::ClearInFeature(CSeq_feat & in_out_feat)
{
    if (in_out_feat.GetData().IsCdregion()) {
        in_out_feat.SetData().SetCdregion().ResetCode();
    }
    else {
        CGbQualColumn::ClearInFeature(in_out_feat);
    }
}

string CGeneticCodeColumn::GetFromFeat(const CSeq_feat & in_out_feat)
{
    if (!in_out_feat.GetData().IsCdregion())
        return kEmptyStr;

    const CCdregion& cdregion = in_out_feat.GetData().GetCdregion();
    if (cdregion.IsSetCode()) {
        int gc = cdregion.GetCode().GetId();
        return NStr::IntToString(gc);
    }

    return kEmptyStr;
}

bool CDescriptionColumn::AddToFeature(CSeq_feat & in_out_feat, const string & newValue,
    EExistingText existing_text)
{
    bool rval = false;
    if (in_out_feat.GetData().IsGene()) {
        string current = kEmptyStr;
        if (in_out_feat.GetData().GetGene().IsSetDesc()) {
            current = in_out_feat.GetData().GetGene().GetDesc();
        }
        if (AddValueToString(current, newValue, existing_text)) {
            in_out_feat.SetData().SetGene().SetDesc(current);
            rval = true;
        }
    } else if (in_out_feat.GetData().IsProt()) {
        string current = kEmptyStr;
        if (in_out_feat.GetData().GetProt().IsSetDesc()) {
            current = in_out_feat.GetData().GetProt().GetDesc();
        }
        if (AddValueToString(current, newValue, existing_text)) {
            in_out_feat.SetData().SetProt().SetDesc(current);
            rval = true;
        }
    } else {
        rval = CGbQualColumn::AddToFeature(in_out_feat, newValue, existing_text);
    }
    return rval;
}

void CDescriptionColumn::ClearInFeature( CSeq_feat & in_out_feat )
{
    if (in_out_feat.GetData().IsGene()) {
        in_out_feat.SetData().SetGene().ResetDesc();
    } else if (in_out_feat.GetData().IsProt()) {
        in_out_feat.SetData().SetProt().ResetDesc();
    } else {
        CGbQualColumn::ClearInFeature(in_out_feat);
    }
}


string CDescriptionColumn::GetFromFeat( const CSeq_feat & in_out_feat )
{
    string val = "";
    if (in_out_feat.GetData().IsGene()) {
        if (in_out_feat.GetData().GetGene().IsSetDesc()) {
            val = in_out_feat.GetData().GetGene().GetDesc();
        }
    } else if (in_out_feat.GetData().IsProt()) {
        if (in_out_feat.GetData().GetProt().IsSetDesc()) {
            val = in_out_feat.GetData().GetProt().GetDesc();
        }
    } else {
        val = CGbQualColumn::GetFromFeat(in_out_feat);
    }
    return val;
}


CGeneRefColumn::CGeneRefColumn(string qual) 
  : CGbQualColumn(qual)
{
    m_FieldType = x_GetFieldTypeFromString(qual);
}


CGeneRefColumn::EFieldType CGeneRefColumn::x_GetFieldTypeFromString(string qual)
{
    EFieldType field_type = eFieldType_other;

    if (QualifierNamesAreEquivalent(qual, "locus")) {
        field_type = eFieldType_locus;
    } else if (QualifierNamesAreEquivalent(qual, "locus_tag")) {
        field_type = eFieldType_locus_tag;
    } else if (QualifierNamesAreEquivalent(qual, "maploc")) {
        field_type = eFieldType_maploc;
    } else if (QualifierNamesAreEquivalent(qual, "synonym")) {
        field_type = eFieldType_synonym;
    } else if (QualifierNamesAreEquivalent(qual, "allele")) {
        field_type = eFieldType_allele;
    }
    return field_type;
    
}


bool CGeneRefColumn::IsGeneRefField(string qual)
{
    EFieldType field_type = x_GetFieldTypeFromString(qual);
    if (field_type == eFieldType_other) {
        return false;
    } else {
        return true;
    }
}


bool CGeneRefColumn::AddToFeature(CSeq_feat & in_out_feat, const string & newValue,
    EExistingText existing_text)
{
    bool rval = false;
    string current = kEmptyStr;
    if (in_out_feat.GetData().IsGene()) {
        CConstRef<CGene_ref> gene(&(in_out_feat.GetData().GetGene()));
        switch (m_FieldType) {
            case eFieldType_locus:
                if (gene->IsSetLocus()) {
                    current = gene->GetLocus();
                }
                if (AddValueToString(current, newValue, existing_text)) {
                    in_out_feat.SetData().SetGene().SetLocus(current);
                    rval = true;
                }
                break;
            case eFieldType_locus_tag:
                if (gene->IsSetLocus_tag()) {
                    current = gene->GetLocus_tag();
                }
                if (AddValueToString(current, newValue, existing_text)) {
                    in_out_feat.SetData().SetGene().SetLocus_tag(current);
                    rval = true;
                }
                break;
            case eFieldType_maploc:
                if (gene->IsSetMaploc()) {
                    current = gene->GetMaploc();
                }
                if (AddValueToString(current, newValue, existing_text)) {
                    in_out_feat.SetData().SetGene().SetMaploc(current);
                    rval = true;
                }
                break;
            case eFieldType_synonym:
                if (gene->IsSetSyn()) {
                    in_out_feat.SetData().SetGene().SetSyn().front() = newValue;
                } else {
                    in_out_feat.SetData().SetGene().SetSyn().push_back(newValue);
                }
                break;
            case eFieldType_allele:
                if (gene->IsSetAllele()) {
                    current = gene->GetAllele();
                }
                if (AddValueToString(current, newValue, existing_text)) {
                    in_out_feat.SetData().SetGene().SetAllele(current);
                    rval = true;
                }
                break;
            default:
                rval = CGbQualColumn::AddToFeature(in_out_feat, newValue, existing_text);
                break;
        }
    } else {
        const CGene_ref* gene = in_out_feat.GetGeneXref();
        if (gene) {
            switch (m_FieldType) {
                case eFieldType_locus:
                    if (gene->IsSetLocus()) {
                        current = gene->GetLocus();
                        if (AddValueToString(current, newValue, existing_text)) {
                            in_out_feat.SetGeneXref().SetLocus(current);
                            rval = true;
                        }
                    } else {
                        in_out_feat.SetGeneXref().SetLocus(newValue);
                        rval = true;
                    }
                    break;
                case eFieldType_locus_tag:
                    if (gene->IsSetLocus_tag()) {
                        current = gene->GetLocus_tag();
                        if (AddValueToString(current, newValue, existing_text)) {
                            in_out_feat.SetGeneXref().SetLocus_tag(current);
                            rval = true;
                        }
                    } else {
                        in_out_feat.SetGeneXref().SetLocus_tag(newValue);
                        rval = true;
                    }
                    break;
                case eFieldType_maploc:
                    if (gene->IsSetMaploc()) {
                        current = gene->GetMaploc();
                        if (AddValueToString(current, newValue, existing_text)) {
                            in_out_feat.SetGeneXref().SetMaploc(current);
                            rval = true;
                        }
                    } else {
                        in_out_feat.SetGeneXref().SetMaploc(newValue);
                        rval = true;
                    }
                    break;
                case eFieldType_synonym:
                    in_out_feat.SetGeneXref().SetSyn().front() = newValue;
                    break;
                case eFieldType_allele:
                    if (gene->IsSetAllele()) {
                        current = gene->GetAllele();
                        if (AddValueToString(current, newValue, existing_text)) {
                            in_out_feat.SetGeneXref().SetAllele(current);
                            rval = true;
                        }
                    } else {
                        in_out_feat.SetGeneXref().SetAllele(newValue);
                        rval = true;
                    }
                    break;
                default:
                    break;
            }
        } else {
            rval = CGbQualColumn::AddToFeature(in_out_feat, newValue, existing_text);
        }
    }    
    return rval;
}

void CGeneRefColumn::ClearInFeature( CSeq_feat & in_out_feat )
{
    if (in_out_feat.GetData().IsGene()) {
        switch (m_FieldType) {
            case eFieldType_locus:
                in_out_feat.SetData().SetGene().ResetLocus();
                break;
            case eFieldType_locus_tag:
                in_out_feat.SetData().SetGene().ResetLocus_tag();
                break;
            case eFieldType_maploc:
                in_out_feat.SetData().SetGene().ResetMaploc();
                break;
            case eFieldType_synonym:
                in_out_feat.SetData().SetGene().ResetSyn();
                break;
            case eFieldType_allele:
                in_out_feat.SetData().SetGene().ResetAllele();
                break;
            default:
                CGbQualColumn::ClearInFeature(in_out_feat);
                break;
        }
    } else if ( in_out_feat.IsSetXref()) {
        switch (m_FieldType) {
            case eFieldType_locus:
                in_out_feat.SetGeneXref().ResetLocus();
                break;
            case eFieldType_locus_tag:
                in_out_feat.SetGeneXref().ResetLocus_tag();
                break;
            case eFieldType_maploc:
                in_out_feat.SetGeneXref().ResetMaploc();
                break;
            case eFieldType_synonym:
                in_out_feat.SetGeneXref().ResetSyn();
                break;
            case eFieldType_allele:
                in_out_feat.SetGeneXref().ResetAllele();
                break;
            default:
                CGbQualColumn::ClearInFeature(in_out_feat);
                break;
        }
    } 
    CGbQualColumn::ClearInFeature(in_out_feat);
}


string CGeneRefColumn::GetFromFeat( const CSeq_feat & in_out_feat )
{
    string val;
    if (in_out_feat.GetData().IsGene()) {
        switch (m_FieldType) {
            case eFieldType_locus:
                if (in_out_feat.GetData().GetGene().IsSetLocus()) {
                    val = in_out_feat.GetData().GetGene().GetLocus();
                }
                break;
            case eFieldType_locus_tag:
                if (in_out_feat.GetData().GetGene().IsSetLocus_tag()) {
                    val = in_out_feat.GetData().GetGene().GetLocus_tag();
                }
                break;
            case eFieldType_maploc:
                if (in_out_feat.GetData().GetGene().IsSetMaploc()) {
                    val = in_out_feat.GetData().GetGene().GetMaploc();
                }
                break;
            case eFieldType_synonym:
                if (in_out_feat.GetData().GetGene().IsSetSyn() 
                    && in_out_feat.GetData().GetGene().GetSyn().size() > 0) {
                    val = in_out_feat.GetData().GetGene().GetSyn().front();
                }
                break;
            case eFieldType_allele:
                if (in_out_feat.GetData().GetGene().IsSetAllele()) {
                    val = in_out_feat.GetData().GetGene().GetAllele();
                }
                break;
            default:
                val = CGbQualColumn::GetFromFeat(in_out_feat);
                break;
        }
    } else {
        CConstRef<CGene_ref> gene_ref;
        if (in_out_feat.IsSetXref()) {
            ITERATE(CSeq_feat::TXref, it, in_out_feat.GetXref()) {
                if ((*it)->IsSetData() && (*it)->GetData().IsGene()) {
                    gene_ref = &((*it)->GetData().GetGene());
                    break;
                }
            }
        }
        if (gene_ref) {
            switch (m_FieldType) {
                case eFieldType_locus:
                    if (gene_ref->IsSetLocus()) {
                        val = gene_ref->GetLocus();
                    }
                    break;
                case eFieldType_locus_tag:
                    if (gene_ref->IsSetLocus_tag()) {
                        val = gene_ref->GetLocus_tag();
                    }
                    break;
                case eFieldType_maploc:
                    if (gene_ref->IsSetMaploc()) {
                        val = gene_ref->GetMaploc();
                    }
                    break;
                case eFieldType_synonym:
                    if (gene_ref->IsSetSyn() && gene_ref->GetSyn().size() > 0) {
                        val = gene_ref->GetSyn().front();
                    }
                    break;
                case eFieldType_allele:
                    if (gene_ref->IsSetAllele()) {
                        val = gene_ref->GetAllele();
                    }
                    break;
                default:
                    break;
            }
        } else {
            val = CGbQualColumn::GetFromFeat(in_out_feat);
        }
    }        
    if (val.empty())
    {
        val = CGbQualColumn::GetFromFeat(in_out_feat);
    }
    return val;
}

vector<CRef<CApplyObject> > CGeneRefColumn::GetApplyObjects(CBioseq_Handle bsh)
{
    vector<CRef<CApplyObject> > objs;
    if (m_MajorType == CSeqFeatData::e_not_set) {
        objs = CFeatureSeqTableColumnBase::GetApplyObjects(bsh);
    } else {
        objs = GetRelatedGeneApplyObjects(bsh);
    }
    return objs;
}


CProtRefColumn::CProtRefColumn(string qual) 
  : CGbQualColumn(qual)
{
    m_FieldType = x_GetFieldTypeFromString(qual);
}


CProtRefColumn::EFieldType CProtRefColumn::x_GetFieldTypeFromString(string qual)
{
    EFieldType field_type = eFieldType_other;

    if (QualifierNamesAreEquivalent(qual, "name")) {
        field_type = eFieldType_name;
    } else if (QualifierNamesAreEquivalent(qual, "ec_number")) {
        field_type = eFieldType_ec_number;
    } else if (QualifierNamesAreEquivalent(qual, "activity")) {
        field_type = eFieldType_activity;
    }
    return field_type;
    
}


bool CProtRefColumn::IsProtRefField(string qual)
{
    EFieldType field_type = x_GetFieldTypeFromString(qual);
    if (field_type == eFieldType_other) {
        return false;
    } else {
        return true;
    }
}

bool CProtRefColumn::AddToFeature(CSeq_feat & in_out_feat, const string & newValue,
    EExistingText existing_text)
{
    bool rval = false;
    if (in_out_feat.GetData().IsProt()) {
        switch (m_FieldType) {
            case eFieldType_name: // only the first one is updated
                if (in_out_feat.GetData().GetProt().IsSetName() &&
                    in_out_feat.GetData().GetProt().GetName().size() > 0) {
                    string current = in_out_feat.GetData().GetProt().GetName().front();
                    if (AddValueToString(current, newValue, existing_text)) {
                        in_out_feat.SetData().SetProt().SetName().front() = current;
                        rval = true;
                    }
                } else if (!NStr::IsBlank(newValue)){
                    in_out_feat.SetData().SetProt().SetName().push_back(newValue);
                    rval = true;
                }
                break;
            case eFieldType_ec_number: // all of them needs to be updated
                if (in_out_feat.GetData().GetProt().IsSetEc()) {
                    NON_CONST_ITERATE(CProt_ref::TEc, it, in_out_feat.SetData().SetProt().SetEc()) {
                        string current = *it;
                        if (AddValueToString(current, newValue, existing_text)) {
                            *it = current;
                            rval = true;
                        }
                    }
                } else if (!NStr::IsBlank(newValue)) {
                    in_out_feat.SetData().SetProt().SetEc().push_back(newValue);
                    rval = true;
                }
                break;
            case eFieldType_activity: // all of them needs to be updated
                if (in_out_feat.GetData().GetProt().IsSetActivity()) {
                    NON_CONST_ITERATE(CProt_ref::TActivity, it, in_out_feat.SetData().SetProt().SetActivity()) {
                        string current = *it;
                        if (AddValueToString(current, newValue, existing_text)) {
                            *it = current;
                            rval = true;
                        }
                    }
                } else if (!NStr::IsBlank(newValue)) {
                    in_out_feat.SetData().SetProt().SetActivity().push_back(newValue);
                    rval = true;
                }
                break;
            default:
                rval = CGbQualColumn::AddToFeature(in_out_feat, newValue, existing_text);
                break;
        }
    } else {
        rval = CGbQualColumn::AddToFeature(in_out_feat, newValue, existing_text);
    }
    return rval;
}


void CProtRefColumn::ClearInFeature( CSeq_feat & in_out_feat )
{
    if (in_out_feat.GetData().IsProt()) {
        switch (m_FieldType) {
            case eFieldType_name:
                in_out_feat.SetData().SetProt().ResetName();
                break;
            case eFieldType_ec_number:
                in_out_feat.SetData().SetProt().ResetEc();
                break;
            case eFieldType_activity:
                in_out_feat.SetData().SetProt().ResetActivity();
                break;
            default:
                CGbQualColumn::ClearInFeature(in_out_feat);
                break;
        }
    } else {
        CGbQualColumn::ClearInFeature(in_out_feat);
    }
}


string CProtRefColumn::GetFromFeat( const CSeq_feat & in_out_feat )
{
    string val = "";
    if (in_out_feat.GetData().IsProt()) {
        switch (m_FieldType) {
            case eFieldType_name:
                if (in_out_feat.GetData().GetProt().IsSetName() 
                    && in_out_feat.GetData().GetProt().GetName().size() > 0) {
                    val = in_out_feat.GetData().GetProt().GetName().front();
                }
                break;
            case eFieldType_ec_number:
                if (in_out_feat.GetData().GetProt().IsSetEc() 
                    && in_out_feat.GetData().GetProt().GetEc().size() > 0) {
                    val = in_out_feat.GetData().GetProt().GetEc().front();
                }
                break;
            case eFieldType_activity:
                if (in_out_feat.GetData().GetProt().IsSetActivity() 
                    && in_out_feat.GetData().GetProt().GetActivity().size() > 0) {
                    val = in_out_feat.GetData().GetProt().GetActivity().front();
                }
                break;
            default:
                val = CGbQualColumn::GetFromFeat(in_out_feat);
                break;
        }
    } else {
        val = CGbQualColumn::GetFromFeat(in_out_feat);
    }        

    return val;
}


bool CNcrnaClassColumn::AddToFeature(CSeq_feat & in_out_feat, const string & newValue,
    EExistingText existing_text)
{
    bool rval = false;
    if (in_out_feat.GetData().GetSubtype() == CSeqFeatData::eSubtype_ncRNA) {
        in_out_feat.SetData().SetRna().SetExt().SetGen().SetClass(newValue);
        rval = true;
    } else {
        rval = CGbQualColumn::AddToFeature(in_out_feat, newValue, existing_text);
    }
    return rval;
}   

void CNcrnaClassColumn::ClearInFeature( CSeq_feat & in_out_feat )
{
    if (in_out_feat.GetData().GetSubtype() == CSeqFeatData::eSubtype_ncRNA) {
        in_out_feat.SetData().SetRna().SetExt().SetGen().ResetClass();
    } else {
        CGbQualColumn::ClearInFeature(in_out_feat);
    }
}


string CNcrnaClassColumn::GetFromFeat( const CSeq_feat & in_out_feat )
{
    string val = "";
    if (in_out_feat.GetData().GetSubtype() == CSeqFeatData::eSubtype_ncRNA) {
        if (in_out_feat.GetData().GetRna().IsSetExt() 
            && in_out_feat.GetData().GetRna().GetExt().IsGen()
            && in_out_feat.GetData().GetRna().GetExt().GetGen().IsSetClass()) {
            val = in_out_feat.GetData().GetRna().GetExt().GetGen().GetClass();
        }
    } else {
        val = CGbQualColumn::GetFromFeat(in_out_feat);
    }
    return val;
}


int FindHighestFeatureId(const CSeq_entry_Handle& entry)
{
    int feat_id = 0;
    for (CFeat_CI feat_it(entry); feat_it; ++feat_it) {
        if (feat_it->IsSetId()) {
            const CFeat_id &id = feat_it->GetId();
            if (id.IsLocal() && id.GetLocal().IsId() && id.GetLocal().GetId() > feat_id) {
                feat_id = id.GetLocal().GetId();
            }
        }
    }
    return feat_id;
}


void AddToFeatureIdColumn(CSeq_feat_Handle feat, CRef<CSeqTable_column> column, size_t row, int& top_id)
{
    if (!feat.IsSetId()) {
        CSeq_feat_EditHandle fh(feat);
        fh.SetFeatId(top_id + 1);
        top_id++;
    }
    const CFeat_id &fid = feat.GetId();        
    if (fid.IsLocal() && fid.GetLocal().IsId()) {
        AddValueToColumn(column, NStr::NumericToString(fid.GetLocal().GetId()), row);
    } else {
        AddValueToColumn(column, "non-local int ID", row);
    }
}


CSeq_feat_Handle GetFeatureFromTableId(const string& id_string, CSeq_entry_Handle entry)
{
    CSeq_feat_Handle orig_feat;

    try {
        int id = NStr::StringToInt(id_string);                
        orig_feat = entry.GetTopLevelEntry().GetTSE_Handle().GetFeatureWithId(CSeqFeatData::e_not_set,
                                                  id);
    } catch(const CException& e) {
        LOG_POST(Error << "GetFeatureFromTableId() failed: " << e.GetMsg());
    } catch (const exception& e) {
        LOG_POST(Error << "GetFeatureFromTableId() failed: " << e.what());
    } 

    return orig_feat;
}


CRef<CSeq_table> BuildFeatureSeqTableFromSeqEntry (CSeq_entry_Handle entry, const SAnnotSelector& sel)
{
    CRef<CSeq_table> table(new CSeq_table());

    int top_id = FindHighestFeatureId(entry);

    // first column is sequence ID
    CRef<CSeqTable_column> id_col(new CSeqTable_column());
    id_col->SetHeader().SetField_id(CSeqTable_column_info::eField_id_location_id);
    id_col->SetHeader().SetTitle(kSequenceIdColLabel);
    table->SetColumns().push_back(id_col);
    // second column is feature ID
    AddStringColumnToTable(table, kFeatureID);

    size_t row = 0;
    CFeat_CI feat (entry, sel);
    while (feat) {
        CBioseq_Handle bsh = entry.GetBioseqHandle(*(feat->GetLocation().GetId()));
        CRef<CSeq_id> id(new CSeq_id());
        id->Assign(*(bsh.GetSeqId()));
        table->SetColumns()[0]->SetData().SetId().push_back(id);

        AddToFeatureIdColumn(feat->GetSeq_feat_Handle(), table->SetColumns()[1], row, top_id);

        ++feat;
        ++row;
    }

    table->SetNum_rows(row);
    FillShortColumns(table);

    return table;
}


void AddColumnsToFeatureTable(const vector<string> & fields, CRef<CSeq_table> table, CSeq_entry_Handle entry)
{
    TFeatureSeqTableColumnList field_list;
    vector<CRef<CSeqTable_column> > column_list;
    bool need_gene = false;
    bool need_mrna = false;
    bool need_prot = false;
    ITERATE (vector<string>, sit, fields) {    
        CRef<CFeatureSeqTableColumnBase> featcol = CFeatureSeqTableColumnBaseFactory::Create(*sit);
        field_list.push_back(featcol);
        column_list.push_back(AddStringColumnToTable(table, *sit));        
        if (NStr::StartsWith(*sit, "gene ")) {
            need_gene = true;
        } else if (NStr::StartsWith(*sit, "mrna ", NStr::eNocase)) {
            need_mrna = true;
        } else if (NStr::StartsWith(*sit, "protein ")) {
            need_prot = true;
        }
    }

    for (int row = 0; row < table->GetNum_rows(); row++) {
        string id_string = table->GetColumns()[1]->GetData().GetString()[row];
        CSeq_feat_Handle orig_feat = GetFeatureFromTableId(id_string, entry);
        if (orig_feat) {
            CConstRef<CSeq_feat> gene;
            if (need_gene) {
                gene = sequence::GetOverlappingGene(orig_feat.GetLocation(), entry.GetScope());
            }
            CConstRef<CSeq_feat> mrna;
            CConstRef<CSeq_feat> prot;
            if (orig_feat.GetData().IsCdregion() && (need_mrna || need_prot)) {
                if (need_mrna) {
                    mrna = sequence::GetOverlappingmRNA(orig_feat.GetLocation(), entry.GetScope());
                }
                if (orig_feat.IsSetProduct() && need_prot) {
                    CBioseq_Handle prot_bsh = entry.GetScope().GetBioseqHandle(*(orig_feat.GetProduct().GetId()));
                    if (prot_bsh) {
                        CFeat_CI f(prot_bsh, SAnnotSelector(CSeqFeatData::eSubtype_prot));
                        if (f) {
                            prot = f->GetOriginalSeq_feat();
                        }
                    }
                }
            }
            for (size_t i = 0; i < field_list.size(); i++) {
                string val = "";
                if (NStr::StartsWith(fields[i], "gene ") && !orig_feat.GetData().IsGene()) {
                    // from gene feature
                    if (gene) {
                        val = field_list[i]->GetFromFeat(*gene);
                    }
                } else if (NStr::StartsWith(fields[i], "mRNA ", NStr::eNocase) && orig_feat.GetData().GetSubtype() != CSeqFeatData::eSubtype_mRNA) {
                    // from mRNA feature
                    if (mrna) {
                        val = field_list[i]->GetFromFeat(*mrna);
                    }
                } else if (NStr::StartsWith(fields[i], "protein ") && !orig_feat.GetData().IsProt()) {
                    if (prot) {
                        val = field_list[i]->GetFromFeat(*prot);
                    }
                } else {
                    val = field_list[i]->GetFromFeat(*(orig_feat.GetSeq_feat()));
                }
                AddValueToColumn(column_list[i], val, row);
            }
        }
    }          
    FillShortColumns(table);
}


CRef<CCmdComposite> ApplyFeatureSeqTableToSeqEntry(CRef<CSeq_table> table, CSeq_entry_Handle entry)
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Edit Features") );
    bool any_change = false;

    bool need_gene = false;
    bool need_mrna = false;
    bool need_prot = false;

    TFeatureSeqTableColumnList field_list;
    for (size_t col = 2; col < table->GetColumns().size(); col++) {
        string title = table->GetColumns()[col]->GetHeader().GetTitle();
        CRef<CFeatureSeqTableColumnBase> featcol = CFeatureSeqTableColumnBaseFactory::Create(title);
        field_list.push_back(featcol);
        if (NStr::StartsWith(title, "gene ")) {
            need_gene = true;
        } else if (NStr::StartsWith(title, "mrna ", NStr::eNocase)) {
            need_mrna = true;
        } else if (NStr::StartsWith(title, "protein ")) {
            need_prot = true;
        }
    }

    for (size_t row = 0; row < table->GetNum_rows(); row++) {
        string id_string = table->GetColumns()[1]->GetData().GetString()[row];
        CSeq_feat_Handle orig_feat = GetFeatureFromTableId(id_string, entry);
        if (orig_feat) {
            CConstRef<CSeq_feat> gene;
            CConstRef<CSeq_feat> mrna;
            CConstRef<CSeq_feat> prot;
            if (need_gene) {
                gene = sequence::GetOverlappingGene(orig_feat.GetLocation(), entry.GetScope());
            }
            if (orig_feat.GetData().IsCdregion() && (need_mrna || need_prot)) {
                if (need_mrna) {
                    mrna = sequence::GetOverlappingmRNA(orig_feat.GetLocation(), entry.GetScope());
                }
                if (orig_feat.IsSetProduct() && need_prot) {
                    CBioseq_Handle prot_bsh = entry.GetScope().GetBioseqHandle(*(orig_feat.GetProduct().GetId()));
                    if (prot_bsh) {
                        CFeat_CI f(prot_bsh, SAnnotSelector(CSeqFeatData::eSubtype_prot));
                        if (f) {
                            prot = f->GetOriginalSeq_feat();
                        }
                    }
                }
            }
            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(*(orig_feat.GetSeq_feat()));
            CRef<CSeq_feat> new_gene(new CSeq_feat());
            CRef<CSeq_feat> new_mrna(new CSeq_feat());
            CRef<CSeq_feat> new_prot(new CSeq_feat());

            if (gene) {
                new_gene->Assign(*gene);
            } else {
                new_gene->SetData().SetGene();
                new_gene->SetLocation().Assign(new_feat->GetLocation());
            }
            if (mrna) {
                new_mrna->Assign(*mrna);
            } else {
                new_mrna->SetData().SetRna().SetType(CRNA_ref::eType_mRNA);
                new_mrna->SetLocation().Assign(new_feat->GetLocation());
            }
            if (prot) {
                new_prot->Assign(*prot);
            }

            bool changed_feat = false;
            bool changed_gene = false;
            bool changed_mrna = false;
            bool changed_prot = false;
            for (size_t col = 2; col < table->GetColumns().size(); col++) {                
                string title = table->GetColumns()[col]->GetHeader().GetTitle();                
                string new_val = table->GetColumns()[col]->GetData().GetString()[row];
                if (NStr::StartsWith(title, "gene ") && !orig_feat.GetData().IsGene()) {
                    string orig_val = "";
                    if (gene) {
                        orig_val = field_list[col - 2]->GetFromFeat(*gene);
                    }
                    if (!NStr::Equal(new_val, orig_val)) {
                        if (NStr::IsBlank(new_val)) {
                            field_list[col - 2]->ClearInFeature(*new_gene);
                        } else {
                            field_list[col - 2]->AddToFeature(*new_gene, new_val, eExistingText_replace_old);
                        }
                        changed_gene = true;
                    }  
                } else if (NStr::StartsWith(title, "mrna ", NStr::eNocase) && orig_feat.GetData().GetSubtype() != CSeqFeatData::eSubtype_mRNA) {
                    string orig_val = "";
                    if (mrna) {
                        orig_val = field_list[col - 2]->GetFromFeat(*mrna);
                    }
                    if (!NStr::Equal(new_val, orig_val)) {
                        if (NStr::IsBlank(new_val)) {
                            field_list[col - 2]->ClearInFeature(*new_mrna);
                        } else {
                            field_list[col - 2]->AddToFeature(*new_mrna, new_val, eExistingText_replace_old);
                        }
                        changed_mrna = true;
                    } 
                } else if (NStr::StartsWith(title, "protein ", NStr::eNocase) && orig_feat.GetData().GetSubtype() != CSeqFeatData::eSubtype_prot && prot) {
                    string orig_val = "";
                    orig_val = field_list[col - 2]->GetFromFeat(*prot);
                    if (!NStr::Equal(new_val, orig_val)) {
                        if (NStr::IsBlank(new_val)) {
                            field_list[col - 2]->ClearInFeature(*new_prot);
                        } else {
                            field_list[col - 2]->AddToFeature(*new_prot, new_val, eExistingText_replace_old);
                        }
                        changed_prot = true;
                    } 
                } else {
                    string orig_val = field_list[col - 2]->GetFromFeat(*(orig_feat.GetSeq_feat()));
                    if (!NStr::Equal(new_val, orig_val)) {
                        if (NStr::IsBlank(new_val)) {
                            field_list[col - 2]->ClearInFeature(*new_feat);
                        } else {
                            field_list[col - 2]->AddToFeature(*new_feat, new_val, eExistingText_replace_old);
                        }
                        changed_feat = true;
                    }                    
                }
            }
            if (changed_gene) {
                if (gene) {
                    cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(entry.GetScope().GetSeq_featHandle(*gene), *new_gene)));
                } else {
                    CBioseq_Handle bsh = entry.GetScope().GetBioseqHandle(*(new_feat->GetLocation().GetId()));
                    CSeq_entry_Handle gene_seh = bsh.GetSeq_entry_Handle();
                    cmd->AddCommand(*(CRef<CCmdCreateFeat>(new CCmdCreateFeat(gene_seh, *new_gene))));
                }
                any_change = true;
            }
            if (changed_mrna) {
                if (mrna) {
                    cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(entry.GetScope().GetSeq_featHandle(*mrna), *new_mrna)));
                } else {
                    CBioseq_Handle bsh = entry.GetScope().GetBioseqHandle(*(new_feat->GetLocation().GetId()));
                    CSeq_entry_Handle mrna_seh = bsh.GetSeq_entry_Handle();
                    cmd->AddCommand(*(CRef<CCmdCreateFeat>(new CCmdCreateFeat(mrna_seh, *new_mrna))));
                }
                any_change = true;
            }
            if (changed_prot) {
                cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(entry.GetScope().GetSeq_featHandle(*prot), *new_prot)));
            }
            if (changed_feat) {
                cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(orig_feat, *new_feat)));
                any_change = true;
            }
        }
    }
    if (!any_change) {
        cmd.Reset(NULL);
    }
    return cmd;
}


CRef<CSeq_loc> CreateFeatLocation(const CBioseq_Handle& bsh, int from, int to, bool ispartial5, bool ispartial3)
{
    CRef<CSeq_loc> loc(new CSeq_loc());
    loc->SetInt().SetFrom(from);
    loc->SetInt().SetTo(to);
    loc->SetInt().SetStrand(eNa_strand_plus);
    CConstRef<CBioseq> bioseq = bsh.GetCompleteBioseq();
    loc->SetInt().SetId().Assign(*bioseq->GetId().front()); 
    if (ispartial5)
        loc->SetPartialStart(true, eExtreme_Biological);
    if (ispartial3)
        loc->SetPartialStop(true, eExtreme_Biological);
        
    return loc;
}


CRef<CSeq_feat> AddMiscRNA(const CBioseq_Handle& bsh, string name, int from, int to, bool ispartial5, bool ispartial3)
{
    CRef<CRNA_ref> rna_ref(new CRNA_ref());
    rna_ref->SetType(CRNA_ref::eType_miscRNA);
    rna_ref->SetExt().SetGen().SetProduct(name);

    CRef<CSeq_loc> loc = CreateFeatLocation(bsh, from, to, ispartial5, ispartial3);
    CRef<CSeq_feat> rna(new CSeq_feat());
    rna->SetData().SetRna().Assign(*rna_ref);
    rna->SetLocation().Assign(*loc);
    if (ispartial5 || ispartial3)
        rna->SetPartial(true);

    return rna;
}


CRef<CSeq_feat> AddRibosomalRNA(const CBioseq_Handle& bsh, string name, int from, int to, bool ispartial5, bool ispartial3)
{	
    CRef<CRNA_ref> rna_ref(new CRNA_ref());
    rna_ref->SetType(CRNA_ref::eType_rRNA);
    rna_ref->SetExt().SetName(name);
    
    CRef<CSeq_loc> loc = CreateFeatLocation(bsh, from, to, ispartial5, ispartial3);	
    
    CRef<CSeq_feat> rna(new CSeq_feat());
    rna->SetData().SetRna().Assign(*rna_ref);
    rna->SetLocation().Assign(*loc);
    if (ispartial5 || ispartial3)
        rna->SetPartial(true);
    
    return rna;
}

CRef<CCmdComposite>  AddNamedrRna(CSeq_entry_Handle entry, string rna_name, bool ispartial5, bool ispartial3)
{
    if (NStr::IsBlank(rna_name) || !entry){
        CRef<CCmdComposite> empty(NULL);
        return empty;
    }
    
    CRef<CCmdComposite> cmd (new CCmdComposite("Adding named rRNA"));
    CBioseq_CI b_iter(entry, CSeq_inst::eMol_na); // only for those bioseqs that contain nucleotide sequences
    for ( ; b_iter ; ++b_iter){
        int from = 0;
        int to = b_iter->GetBioseqLength() - 1;
        CRef<CSeq_feat> new_feat = AddRibosomalRNA(*b_iter, rna_name, from, to, ispartial5, ispartial3);
        CSeq_entry_Handle seh = b_iter->GetSeq_entry_Handle();
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat)));
    }	
    return cmd; 
}

CRef<CCmdComposite> CreateControlRegion(CSeq_entry_Handle entry)
{
    if (!entry){
        CRef<CCmdComposite> empty(NULL);
        return empty;
    }
    
    CRef<CCmdComposite> cmd (new CCmdComposite("Creating control region"));
    CBioseq_CI b_iter(entry, CSeq_inst::eMol_na); // only for those bioseqs that contain nucleotide sequences
    bool partial5 = true;
    bool partial3 = true;
    for ( ; b_iter ; ++b_iter){
        int from = 0;
        int to = b_iter->GetBioseqLength() - 1;
        CRef<CSeq_loc> loc = CreateFeatLocation(*b_iter, from, to, partial5, partial3);
        
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->SetData().SetImp().SetKey("misc_feature");
        new_feat->SetComment("control region");
        new_feat->SetLocation().Assign(*loc);
        new_feat->SetPartial(true);
        
        CSeq_entry_Handle seh = b_iter->GetSeq_entry_Handle();
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat)));
    }
    return cmd;
}

CRef<CCmdComposite> CreateMicrosatellite(CSeq_entry_Handle entry)
{
    if (!entry){
        CRef<CCmdComposite> empty(NULL);
        return empty;
    }

    CRef<CCmdComposite> cmd (new CCmdComposite("Creating microsatellite common feature"));
    CBioseq_CI b_iter(entry, CSeq_inst::eMol_na); // only for those bioseqs that contain nucleotide sequences
    bool partial5 = false;
    bool partial3 = false; 
    for ( ; b_iter ; ++b_iter){
        int from = 0;
        int to = b_iter->GetBioseqLength() - 1; 
        CRef<CSeq_loc> loc = CreateFeatLocation(*b_iter, from, to, partial5, partial3);
        
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->SetData().SetImp().SetKey("repeat_region");
        new_feat->SetLocation().Assign(*loc);
        
        CRef<CGb_qual> qual1(new CGb_qual("rpt_type", "tandem"));
        CRef<CGb_qual> qual2(new CGb_qual("satellite", "microsatellite"));
        new_feat->SetQual().push_back(qual1);
        new_feat->SetQual().push_back(qual2);
        
        CSeq_entry_Handle seh = b_iter->GetSeq_entry_Handle();
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat)));
    }
    return cmd;
}


END_NCBI_SCOPE
