/* $Id: mrna_cds.cpp 38664 2017-06-07 19:25:40Z filippov $
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
 * Authors:  Jie Chen & Igor Filippov
 * File:  create mRNA for CDS
 *  
 */

#include <ncbi_pch.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/mrna_cds.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;

void CMRnaForCDS :: x_ChkAndMakeMRna(const CSeq_feat* cd, CRef <CCmdComposite> composite, CScope& scope)
{
   if (!cd) 
       return;
   if (IsOrganelle(cd, scope))
       return;
   CRef<CSeq_feat> new_mrna = edit::MakemRNAforCDS(*cd, scope);
   if (new_mrna) {
       CBioseq_Handle bsh = scope.GetBioseqHandle(cd->GetLocation());
       CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
       composite->AddCommand(*(CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_mrna))));
   }
} 

bool CMRnaForCDS::IsOrganelle(const CSeq_feat* cd, CScope& scope)
{
    if (!cd->IsSetLocation())
        return false;
    CBioseq_Handle bsh = scope.GetBioseqHandle(cd->GetLocation());
    if (!bsh)
        return false;
    CSeqdesc_CI source_ci(bsh, CSeqdesc::e_Source);
    if (!source_ci)
        return false;
    if (!source_ci->GetSource().IsSetGenome())
        return false;
    int genome = source_ci->GetSource().GetGenome();
    bool rval = false;
    switch (genome) {
    case CBioSource::eGenome_chloroplast:
    case CBioSource::eGenome_chromoplast:
    case CBioSource::eGenome_kinetoplast:
    case CBioSource::eGenome_mitochondrion:
    case CBioSource::eGenome_cyanelle:
    case CBioSource::eGenome_nucleomorph:
    case CBioSource::eGenome_apicoplast:
    case CBioSource::eGenome_leucoplast:
    case CBioSource::eGenome_proplastid:
    case CBioSource::eGenome_hydrogenosome:
    case CBioSource::eGenome_chromatophore:
    case CBioSource::eGenome_plastid:
        rval = true;
        break;
    default:
        rval = false;
        break;
    }
    return rval;
}
    
bool CMRnaForCDS :: DoesMrnaExist(CSeq_entry_Handle seh) // fci->GetAnnot().GetParentEntry()
{
    bool exists = false;
    if (seh.IsSet())
    {
        for (CSeq_entry_CI seq(seh.GetSet()); seq; ++seq)
            for (CSeqdesc_CI desc(*seq,CSeqdesc::e_Molinfo); desc; ++desc)
            {                
                if (desc->GetMolinfo().IsSetBiomol() && desc->GetMolinfo().GetBiomol() == CMolInfo::eBiomol_mRNA)
                    exists = true;
            }
    }
    return exists;
}

void CMRnaForCDS :: apply_to(CRef <CCmdComposite> composite,  TConstScopedObjects& SelObjs, CScope& scope)
{
   NON_CONST_ITERATE (TConstScopedObjects, it, SelObjs) {
      const CObject* ptr = it->object.GetPointer();
                 
      // selected objects
      const CSeq_feat* seqfeat = dynamic_cast<const CSeq_feat*>(ptr);
      const CBioseq* bioseq = dynamic_cast<const CBioseq*>(ptr);
      const CSeq_entry* seq_entry = dynamic_cast<const CSeq_entry*>(ptr);
 
      if (seqfeat && seqfeat->GetData().IsCdregion()) {          
          if (!DoesMrnaExist(it->scope->GetSeq_featHandle(*seqfeat).GetAnnot().GetParentEntry()))                 
              x_ChkAndMakeMRna(seqfeat, composite,scope); 
          break;
      }
      else if (bioseq) {
          for (CFeat_CI fci(it->scope->GetBioseqHandle(*bioseq), CSeqFeatData::e_Cdregion); fci; ++fci) 
          {
              if (!DoesMrnaExist(fci->GetAnnot().GetParentEntry()))     
                  x_ChkAndMakeMRna(fci->GetSeq_feat().GetPointer(), composite,scope);
          }          
      }   
      else if (seq_entry) {        
          for (CFeat_CI fci(it->scope->GetSeq_entryHandle(*seq_entry), CSeqFeatData::e_Cdregion); fci; ++fci) 
          {
              if (!DoesMrnaExist(fci->GetAnnot().GetParentEntry()))     
                  x_ChkAndMakeMRna(fci->GetSeq_feat().GetPointer(), composite,scope);
          }          
      }   
   }
}        

bool CMRnaForCDS :: apply(IWorkbench* wb, ICommandProccessor* cmdProcessor, const string& title, CScope& scope)
{
   if (wb) 
   {
    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = wb->GetServiceByType<CSelectionService>();
    if (!sel_srv) return false;
    sel_srv->GetActiveObjects(objects);
    if (objects.empty()) 
    {
        GetViewObjects(wb,objects);
    }
    if (objects.empty())
        return false;

    CRef<CCmdComposite> composite(new CCmdComposite(title));
    apply_to(composite, objects,scope);
    cmdProcessor->Execute(composite.GetPointer());
    return true;
   }
    else return false;
}          

END_NCBI_SCOPE
