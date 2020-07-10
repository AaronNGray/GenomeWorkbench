/*  $Id: convert_feat.cpp 43474 2019-07-11 19:20:45Z filippov $
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
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>

#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/RNA_gen.hpp>
#include <objects/seqfeat/SeqFeatXref.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objects/seqfeat/Prot_ref.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_bond.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>

#include <objmgr/feat_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/seq_loc_mapper.hpp>
#include <objtools/edit/field_handler.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/objutils/convert_feat.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/cmd_add_seqentry.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CConvertFeatureBase::CConvertFeatureBase(CSeqFeatData::ESubtype feat_to, CSeqFeatData::ESubtype feat_from) 
        : m_To(feat_to) , m_From(feat_from) 
{ 
    m_Options.clear(); 
    if (m_From == CSeqFeatData::eSubtype_cdregion) {
        x_AddCDSSourceConversionOptions();
    }
    if (CSeqFeatData::GetTypeFromSubtype(m_To) == CSeqFeatData::e_Rna) {
        x_AddRnaDestinationConversionOptions();
    } else if (m_To == CSeqFeatData::eSubtype_site) {
        x_AddSiteDestinationConversionOptions();
    } else if (m_To == CSeqFeatData::eSubtype_bond) {
        x_AddBondDestinationConversionOptions();
    } else if (m_To == CSeqFeatData::eSubtype_region && !x_IsProtSubtype(feat_from)) {
        x_AddRegionDestionationConversionOptions();
    } else if (m_To == CSeqFeatData::eSubtype_cdregion) {
        x_AddCDSDestinationConversionOptions();
    }
}


// often a class converts to a single subtype, but this can be overridden
// for example, converting to different subtypes of RNAs or import features
bool CConvertFeatureBase::CanConvertTo(CSeqFeatData::ESubtype subtype)
{
    if (subtype == m_To) {
        return true;
    } else {
        return false;
    }
}


// often a class converts to a single subtype, but this can be overridden
// for example, converting to different subtypes of RNAs or import features
bool CConvertFeatureBase::CanConvertFrom(CSeqFeatData::ESubtype subtype)
{
    if (subtype == m_From) {
        return true;
    } else {
        return false;
    }
}


CRef<CCmdComposite> CConvertFeatureBase::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CProt_ref::EProcessed GetProtProcessedFromFeatureSubtype(CSeqFeatData::ESubtype subtype)
{
    CProt_ref::EProcessed processed = CProt_ref::eProcessed_not_set;
    switch (subtype) {
        case CSeqFeatData::eSubtype_preprotein:
            processed = CProt_ref::eProcessed_preprotein;
            break;
        case CSeqFeatData::eSubtype_mat_peptide:
        case CSeqFeatData::eSubtype_mat_peptide_aa:
            processed = CProt_ref::eProcessed_mature;
            break;
        case CSeqFeatData::eSubtype_sig_peptide:
        case CSeqFeatData::eSubtype_sig_peptide_aa:
            processed = CProt_ref::eProcessed_signal_peptide;
            break;
        case CSeqFeatData::eSubtype_transit_peptide:
        case CSeqFeatData::eSubtype_transit_peptide_aa:
            processed = CProt_ref::eProcessed_transit_peptide;
            break;
        default:
            break;
    }
    return processed;
}


CRNA_ref::EType GetRnaTypeFromFeatureSubtype(CSeqFeatData::ESubtype subtype)
{
    CRNA_ref::EType rna_type = CRNA_ref::eType_unknown;

    switch (subtype) {
        case CSeqFeatData::eSubtype_preRNA:
        case CSeqFeatData::eSubtype_precursor_RNA:
            rna_type = CRNA_ref::eType_premsg;
            break;
        case CSeqFeatData::eSubtype_mRNA:
            rna_type = CRNA_ref::eType_mRNA;
            break;
        case CSeqFeatData::eSubtype_tRNA:
            rna_type = CRNA_ref::eType_tRNA;
            break;
        case CSeqFeatData::eSubtype_rRNA:
            rna_type = CRNA_ref::eType_rRNA;
            break;
        case CSeqFeatData::eSubtype_snRNA:
            rna_type = CRNA_ref::eType_snRNA;
            break;
        case CSeqFeatData::eSubtype_scRNA:
            rna_type = CRNA_ref::eType_scRNA;
            break;
        case CSeqFeatData::eSubtype_snoRNA:
            rna_type = CRNA_ref::eType_snoRNA;
            break;
        case CSeqFeatData::eSubtype_ncRNA:
            rna_type = CRNA_ref::eType_ncRNA;
            break;
        case CSeqFeatData::eSubtype_tmRNA:
            rna_type = CRNA_ref::eType_tmRNA;
            break;
        case CSeqFeatData::eSubtype_otherRNA:
            rna_type = CRNA_ref::eType_miscRNA;
            break;
        case CSeqFeatData::eSubtype_misc_RNA:
            rna_type = CRNA_ref::eType_other;
            break;
        default:
            // leave as unknown;
            break;
    }
    return rna_type;
}


CRef<CCmdComposite> CConvertFeatureBase::x_SimpleConversion(const CSeq_feat& orig, CRef<CSeq_feat> new_feat, bool keep_orig, CScope& scope)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Convert Feature"));

    if (x_IsProtSubtype(m_To)) {
        new_feat->SetData().SetProt().SetProcessed(GetProtProcessedFromFeatureSubtype(m_To));   
    } else if (x_IsRnaSubtype(m_To)) {
        new_feat->SetData().SetRna().SetType(GetRnaTypeFromFeatureSubtype(m_To));
    } else if (x_IsImportSubtype(m_To)) {
        new_feat->SetData().SetImp().SetKey(CSeqFeatData::SubtypeValueToName(m_To));
    }

    if (new_feat->IsSetProduct()) {
        CRef<CConversionOption> remove_transcript = FindRemoveTranscriptId();    
        if (!remove_transcript || (remove_transcript && remove_transcript->IsBool() && remove_transcript->GetBool()->GetVal())) {
            new_feat->ResetProduct();
        }
    }
    if (m_To == CSeqFeatData::eSubtype_ncRNA) {
        CRef<CConversionOption> ncrna_class = x_FindNcrnaClass();
        if (ncrna_class && ncrna_class->IsString() && !NStr::IsBlank(ncrna_class->GetString()->GetVal())) {
            new_feat->SetData().SetRna().SetExt().SetGen().SetClass(ncrna_class->GetString()->GetVal());
        }
    }
    if (m_To == CSeqFeatData::eSubtype_site) {
        CRef<CConversionOption> site_type = x_FindSiteType();
        if (site_type && site_type->IsString() && !NStr::IsBlank(site_type->GetString()->GetVal())) {
            new_feat->SetData().SetSite(m_SiteList.GetSiteType(site_type->GetString()->GetVal()));
        }
    }
    if (m_To == CSeqFeatData::eSubtype_bond) {
        CRef<CConversionOption> bond_type = x_FindBondType();
        if (bond_type && bond_type->IsString() && !NStr::IsBlank(bond_type->GetString()->GetVal())) {
            new_feat->SetData().SetBond(m_BondList.GetBondType(bond_type->GetString()->GetVal()));
        }
    }
    if (m_To == CSeqFeatData::eSubtype_region) {
        CRef<CConversionOption> on_prot = x_FindPlaceOnProt();
        if (on_prot && on_prot->IsBool() && on_prot->GetBool()->GetVal()) {
            if (!x_MoveFeatureToProtein(orig, new_feat, keep_orig, scope)) {
                return CRef<CCmdComposite>(NULL);
            }
        }
    }


    if (keep_orig) {
        CBioseq_Handle bsh = scope.GetBioseqHandle(new_feat->GetLocation());
        CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat)));
    } else  {
        // create new feature
        CBioseq_Handle bsh = scope.GetBioseqHandle(new_feat->GetLocation());
        CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat)));
        // delete original feature (will also remove product and renormalize nuc-prot set
        cmd->AddCommand(*GetDeleteFeatureCommand(scope.GetSeq_featHandle(orig)));
    }

    CRef<CConversionOption> remove_gene = x_FindRemoveGene();
    if (remove_gene && remove_gene->IsBool() && remove_gene->GetBool()->GetVal() && !(orig.IsSetData() && orig.GetData().IsGene())) {
        CConstRef<CSeq_feat> gene = sequence::GetOverlappingGene(orig.GetLocation(), scope);
        if (gene) {
            cmd->AddCommand(*GetDeleteFeatureCommand(scope.GetSeq_featHandle(*gene)));
        }
    }
    
    CRef<CConversionOption> remove_mrna = x_FindRemovemRNA();
    if (remove_mrna && remove_mrna->IsBool() && remove_mrna->GetBool()->GetVal() && !(orig.IsSetData() && orig.GetData().GetSubtype() == CSeqFeatData::eSubtype_mRNA)) {
        CConstRef<CSeq_feat> mrna = sequence::GetOverlappingmRNA(orig.GetLocation(), scope);
        if (mrna) {
            cmd->AddCommand(*GetDeleteFeatureCommand(scope.GetSeq_featHandle(*mrna)));
        }
    }

    return cmd;
}


CRef<CCmdComposite> CConvertFeatureBase::x_ConvertToCDS
    (const CSeq_feat& orig, 
     CRef<CSeq_feat> new_feat, 
     bool keep_orig, 
     CScope& scope, 
     vector<string> products)
{
    bool make_pseudo = false;
    CRef<CConversionOption> is_pseudo = x_FindPseudo();
    if (is_pseudo && is_pseudo->IsBool() && is_pseudo->GetBool()->GetVal()) {
        make_pseudo = true;
    }
    CRef<CCmdComposite> cmd(NULL);
    if (make_pseudo) {
        new_feat->SetPseudo(true);
        ITERATE(vector<string>, it, products) {
            x_AddToComment(*it, new_feat);
        }
        cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);
    } else {
        // translate new coding region
        CBioseq_Handle bsh = scope.GetBioseqHandle(new_feat->GetLocation());
        CRef<CSeq_entry> protein = CreateTranslatedProteinSequence (new_feat, bsh, m_create_general_only, m_offset);
        CRef<CSeq_feat> prot = AddProteinFeatureToProtein (protein,
                              new_feat->GetLocation().IsPartialStart(eExtreme_Biological), 
                              new_feat->GetLocation().IsPartialStop(eExtreme_Biological)); 
        ITERATE(vector<string>, it, products) {
            prot->SetData().SetProt().SetName().push_back(*it);
        }

        CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
        cmd.Reset(new CCmdComposite("Convert to coding region"));
        cmd->AddCommand(*CRef<CCmdAddSeqEntry>(new CCmdAddSeqEntry(protein, seh)));
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat)));
        if (!keep_orig) {
            cmd->AddCommand(*GetDeleteFeatureCommand(scope.GetSeq_featHandle(orig)));
        }
    }

    return cmd;
}


string CConvertFeatureBase::x_GetCDSProduct(const CSeq_feat& orig, CScope& scope)
{
    string product = "";
    if (orig.IsSetProduct()) {
        CBioseq_Handle bsh = scope.GetBioseqHandle(orig.GetProduct());
        if (bsh) {
            CFeat_CI f(bsh, SAnnotSelector(CSeqFeatData::eSubtype_prot));
            if (f && f->IsSetData() && f->GetData().IsProt()
                && f->GetData().GetProt().IsSetName()
                && f->GetData().GetProt().GetName().size() > 0) {
                product = f->GetData().GetProt().GetName().front();
            }
        }
    } else if (orig.IsSetXref()) {
        ITERATE(CSeq_feat::TXref, it, orig.GetXref()) {
            if ((*it)->IsSetData() && (*it)->GetData().IsProt() 
                && (*it)->GetData().GetProt().IsSetName()
                && (*it)->GetData().GetProt().GetName().size() > 0) {
                product = (*it)->GetData().GetProt().GetName().front();
            }
        }
    }
    return product;
}

static void AdjustMappedStop(CRef<CSeq_loc> loc, const CSeq_loc &product, CScope& scope)
{
    CBioseq_Handle protein = scope.GetBioseqHandle(product);
    if (protein)
    {
        TSeqPos length = protein.GetBioseqLength();        
        if ( loc && loc->GetId() && loc->GetStop(eExtreme_Positional) >= length)
        {
            CRef<CSeq_id> prot_id(new CSeq_id);
            prot_id->Assign(*loc->GetId());
            bool partial_stop =  loc->IsPartialStop(eExtreme_Positional);
            CSeq_loc sub(*prot_id, length, loc->GetStop(eExtreme_Positional), loc->GetStrand());
            CRef<CSeq_loc> new_loc = sequence::Seq_loc_Subtract(*loc, sub, CSeq_loc::fMerge_All|CSeq_loc::fSort, &scope);
            new_loc->SetPartialStop(partial_stop, eExtreme_Positional);
            loc->Assign(*new_loc);
        }
    }
}

bool CConvertFeatureBase::x_MoveFeatureToProtein(const CSeq_feat& orig, CRef<CSeq_feat> new_feat, bool keep_orig, CScope& scope)
{
    if (orig.GetData().IsCdregion() && keep_orig) {
        if (!orig.IsSetProduct()) {
            return false;
        }
        CSeq_loc_Mapper mapper(orig, CSeq_loc_Mapper::eLocationToProduct, &scope);
        mapper.SetMergeAbutting();
        CRef<CSeq_loc> loc = mapper.Map(orig.GetLocation());
        if ( !loc || loc->Which() == CSeq_loc::e_Null) {
            return false;
        }
        AdjustMappedStop(loc, orig.GetProduct(), scope);
        new_feat->SetLocation().Assign(*loc);
    } else {
        CConstRef<CSeq_feat> overlapping_cds = sequence::GetBestOverlappingFeat(orig.GetLocation(), CSeqFeatData::eSubtype_cdregion,
                                  sequence::eOverlap_Contained, scope, sequence::fBestFeat_Defaults | sequence::fBestFeat_FavorLonger);
        if (!overlapping_cds || !overlapping_cds->IsSetProduct() || overlapping_cds.GetPointer() == &orig) {
            return false;
        } else {
            CSeq_loc_Mapper mapper(*overlapping_cds, CSeq_loc_Mapper::eLocationToProduct, &scope);
            mapper.SetMergeAbutting();
            CRef<CSeq_loc> loc = mapper.Map(orig.GetLocation());         
            if ( !loc  || loc->Which() == CSeq_loc::e_Null) {
                return false;
            }
            AdjustMappedStop(loc, overlapping_cds->GetProduct(), scope);
            new_feat->SetLocation().Assign(*loc);
        }
    }
    return true;
}


vector<string> CConvertFeatureBase::x_ExtractQuals(CRef<CSeq_feat> new_feat, string qual_name)
{
    vector<string> vals;

    if (new_feat->IsSetQual()) {
        CSeq_feat::TQual::iterator it = new_feat->SetQual().begin();
        while (it != new_feat->SetQual().end()) {
            if ((*it)->IsSetQual() && NStr::EqualNocase((*it)->GetQual(), qual_name)) {
                if ((*it)->IsSetVal() && !NStr::IsBlank((*it)->GetVal())) {
                    vals.push_back((*it)->GetVal());
                }
                it = new_feat->SetQual().erase(it);
            } else {
                ++it;
            }
        }
        if (new_feat->GetQual().size() == 0) {
            new_feat->ResetQual();
        }
    }
    return vals;
}


vector<string> CConvertFeatureBase::x_ExtractProductQuals(CRef<CSeq_feat> new_feat)
{
    return x_ExtractQuals(new_feat, "product");
}

void CConvertFeatureBase::x_AddToComment(string comment, CRef<CSeq_feat> new_feat, edit::EExistingText existing_text)
{
    if (NStr::IsBlank(comment)) {
        return;
    }
    
    if (!new_feat->IsSetComment() || NStr::IsBlank(new_feat->GetComment())) {
        new_feat->SetComment(comment);
    } else {
        string orig_val = new_feat->GetComment();
        if (edit::AddValueToString(orig_val, comment, existing_text)) {
            new_feat->SetComment(orig_val);
        }
    }
}

void CConvertFeatureBase::x_ClearGenericFields(CRef<CSeq_feat> new_feat)
{
    new_feat->ResetId();
    new_feat->SetLocation().SetPartialStart(false, eExtreme_Biological);
    new_feat->SetLocation().SetPartialStop(false, eExtreme_Biological);
    new_feat->ResetExcept();
    new_feat->ResetExcept_text();
    new_feat->ResetComment();
    new_feat->ResetProduct();
    new_feat->ResetQual();
    new_feat->ResetTitle();
    new_feat->ResetCit();
    new_feat->ResetDbxref();
    new_feat->ResetPseudo();
}

const string CConvertFeatureBase::s_RemovemRNA("Remove overlapping mRNA");
const string CConvertFeatureBase::s_RemoveGene("Remove overlapping gene");
const string CConvertFeatureBase::s_RemoveTranscriptID("Remove transcript ID");
const string CConvertFeatureBase::s_NcrnaClass("ncRNA class");
const string CConvertFeatureBase::s_SiteType("Site type");
const string CConvertFeatureBase::s_BondType("Bond type");
const string CConvertFeatureBase::s_PlaceOnProt("Create region features on protein sequence of overlapping coding region");
const string CConvertFeatureBase::s_IsPseudo("Pseudo");

void CConvertFeatureBase::x_AddCDSSourceConversionOptions()
{
    CRef<CConversionOption> remove_mrna(new CConversionOption(s_RemovemRNA));
    remove_mrna->SetBool();
    m_Options.push_back(remove_mrna);
    CRef<CConversionOption> remove_gene(new CConversionOption(s_RemoveGene));
    remove_gene->SetBool();
    m_Options.push_back(remove_gene);
    CRef<CConversionOption> remove_transcript(new CConversionOption(s_RemoveTranscriptID));
    remove_transcript->SetBool();
    m_Options.push_back(remove_transcript);
}


void CConvertFeatureBase::x_AddCDSDestinationConversionOptions()
{
    CRef<CConversionOption> is_pseudo(new CConversionOption(s_IsPseudo));
    is_pseudo->SetBool();
    m_Options.push_back(is_pseudo);
}


void CConvertFeatureBase::x_AddRnaDestinationConversionOptions()
{
    if (m_To == CSeqFeatData::eSubtype_ncRNA) {
        CRef<CConversionOption> ncrna_class(new CConversionOption(s_NcrnaClass));
        CRef<CConversionOptionString> val = ncrna_class->SetString();
        vector<string> class_vals = CRNA_gen::GetncRNAClassList();
        ITERATE(vector<string>, it, class_vals) {
            val->AddSuggestedValue(*it);
        }
        val->SetOnlySuggested(false);

        m_Options.push_back(ncrna_class);
    }
}


void CConvertFeatureBase::x_AddSiteDestinationConversionOptions()
{
    CRef<CConversionOption> site_type(new CConversionOption(s_SiteType));
    CRef<CConversionOptionString> val = site_type->SetString();
    CSiteList::const_iterator it = m_SiteList.begin();
    while (it != m_SiteList.end()) {
        val->AddSuggestedValue(it->first);
        ++it;
    }
    val->SetDefaultVal("active");
    val->SetOnlySuggested(true);
    m_Options.push_back(site_type);
}


void CConvertFeatureBase::x_AddBondDestinationConversionOptions()
{
    CRef<CConversionOption> bond_type(new CConversionOption(s_BondType));
    CRef<CConversionOptionString> val = bond_type->SetString();
    CBondList::const_iterator it = m_BondList.begin();
    while (it != m_BondList.end()) {
        val->AddSuggestedValue(it->first);
        ++it;
    }
    val->SetDefaultVal("disulfide");
    val->SetOnlySuggested(true);
    m_Options.push_back(bond_type);
}


void CConvertFeatureBase::x_AddRegionDestionationConversionOptions()
{
    CRef<CConversionOption> on_prot(new CConversionOption(s_PlaceOnProt));
    CRef<CConversionOptionBool> val = on_prot->SetBool();
    m_Options.push_back(on_prot);
}


CRef<CConversionOption> CConvertFeatureBase::x_FindOption(const string& label)
{
    ITERATE(TOptions, it, m_Options) {
        if (NStr::Equal((*it)->GetLabel(), label)) {
            return *it;
        }
    }
    return CRef<CConversionOption>(NULL);
}


CRef<CConversionOption> CConvertFeatureBase::x_FindRemoveGene()
{
    return x_FindOption(s_RemoveGene);
}


CRef<CConversionOption> CConvertFeatureBase::x_FindRemovemRNA()
{
    return x_FindOption(s_RemovemRNA);
}


CRef<CConversionOption> CConvertFeatureBase::FindRemoveTranscriptId()
{
    return x_FindOption(s_RemoveTranscriptID);
}


CRef<CConversionOption> CConvertFeatureBase::x_FindNcrnaClass()
{
    return x_FindOption(s_NcrnaClass);
}


CRef<CConversionOption> CConvertFeatureBase::x_FindSiteType()
{
    return x_FindOption(s_SiteType);
}


CRef<CConversionOption> CConvertFeatureBase::x_FindBondType()
{
    return x_FindOption(s_BondType);
}


CRef<CConversionOption> CConvertFeatureBase::x_FindPlaceOnProt()
{
    return x_FindOption(s_PlaceOnProt);
}


CRef<CConversionOption> CConvertFeatureBase::x_FindPseudo()
{
    return x_FindOption(s_IsPseudo);
}


bool CConvertFeatureBase::x_IsImportSubtype(CSeqFeatData::ESubtype subtype)
{
    if (CSeqFeatData::GetTypeFromSubtype(subtype) == CSeqFeatData::e_Imp) {
        return true;
    } else {
        return false;
    }
}


bool CConvertFeatureBase::x_IsProtSubtype(CSeqFeatData::ESubtype subtype)
{
    if (CSeqFeatData::GetTypeFromSubtype(subtype) == CSeqFeatData::e_Prot) {
        return true;
    } else {
        return false;
    }
}


bool CConvertFeatureBase::x_IsRnaSubtype(CSeqFeatData::ESubtype subtype)
{
    if (CSeqFeatData::GetTypeFromSubtype(subtype) == CSeqFeatData::e_Rna) {
        return true;
    } else {
        return false;
    }
}


string CConvertFeatureBase::GetSiteName(CSeqFeatData::TSite site_type) 
{
    CSiteList::const_iterator it = m_SiteList.begin();
    while (it != m_SiteList.end() && site_type != it->second) {
        ++it;
    }
    if (it) {
        return it->first;
    } else {
        return "";
    }
}


CRef<CCmdComposite> CConvertMiscFeatToGene::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{    
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetGene();
    if (orig.IsSetComment()) {
        new_feat->SetData().SetGene().SetLocus(orig.GetComment());
        size_t pos = NStr::Find(new_feat->GetComment(), ";");
        if (pos == string::npos) {
            new_feat->ResetComment();
        } else {
            new_feat->SetData().SetGene().SetLocus(new_feat->GetData().GetGene().GetLocus().substr(0, pos));
            new_feat->SetComment(new_feat->GetComment().substr(pos));
        }
    }

    return x_SimpleConversion(orig, new_feat, keep_orig, scope);
}


CRef<CCmdComposite> CConvertImpToImp::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{    
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);

    return x_SimpleConversion(orig, new_feat, keep_orig, scope);
}


CRef<CCmdComposite> CConvertCDSToRNA::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CCmdComposite> cmd(NULL);
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetRna().SetType(GetRnaTypeFromFeatureSubtype(m_To));

    // get protein name
    string product = x_GetCDSProduct(orig, scope);
    if (!NStr::IsBlank(product)) {
        string remainder = "";
        new_feat->SetData().SetRna().SetRnaProductName(product, remainder);
        x_AddToComment(remainder, new_feat);
    }

    cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


void CConvertGeneToRNA::x_AddOneGeneField(string& product, string& comment, const string& field)
{
    if (NStr::IsBlank(field)) {
        return;
    }
    if (NStr::IsBlank(product)) {
        product = field;
    } else if (NStr::Find(product, field) == string::npos && NStr::Find(comment, field) == string::npos) {
        comment = comment + "; " + field;
    }
}


void CConvertGeneToRNA::x_AddGeneFieldsToComment(const CGene_ref& orig, CRef<CSeq_feat> new_feat)
{
    string product = "";
    string comment = "";
    if (new_feat->IsSetComment()) {
        comment = new_feat->GetComment();
    }
    
    if (orig.IsSetLocus()) {
        x_AddOneGeneField(product, comment, orig.GetLocus());
    }
    if (orig.IsSetAllele()) {
        x_AddOneGeneField(product, comment, orig.GetAllele());
    }
    if (orig.IsSetDesc()) {
        x_AddOneGeneField(product, comment, orig.GetDesc());
    }
    if (orig.IsSetMaploc()) {
        x_AddOneGeneField(product, comment, orig.GetMaploc());
    }
    if (orig.IsSetLocus_tag()) {
        x_AddOneGeneField(product, comment, orig.GetLocus_tag());
    }

    string remainder = "";
    new_feat->SetData().SetRna().SetRnaProductName(product, remainder);
    if (!NStr::IsBlank(remainder)) {
        comment = comment + "; " + remainder;
    }
    if (NStr::StartsWith(comment, ";")) {
        comment = comment.substr(1);
    }
    NStr::TruncateSpacesInPlace(comment);
    if (!NStr::IsBlank(comment)) {
        new_feat->SetComment(comment);
    }
}


CRef<CCmdComposite> CConvertGeneToRNA::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetRna().SetType(GetRnaTypeFromFeatureSubtype(m_To));
    x_AddGeneFieldsToComment(orig.GetData().GetGene(), new_feat);
    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CRef<CCmdComposite> CConvertBioSrcToRptRegion::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetImp().SetKey("repeat_region");

    bool is_transposon = false;
    bool is_insertion = false;
    string transposon_name = "";
    string insertion_name = "";
    string comment = "";
    if (orig.IsSetComment()) {
        comment = orig.GetComment();
    }

    if (orig.IsSetData() && orig.GetData().IsBiosrc()) {
        if (orig.GetData().GetBiosrc().IsSetSubtype()) {
            ITERATE(CBioSource::TSubtype, it, orig.GetData().GetBiosrc().GetSubtype()) {
                if ((*it)->IsSetSubtype()) {
                    if ((*it)->GetSubtype() == CSubSource::eSubtype_transposon_name) {
                        is_transposon = true;
                        if ((*it)->IsSetName() && !NStr::IsBlank((*it)->GetName())) {
                            if (NStr::IsBlank(transposon_name)) {
                                transposon_name = (*it)->GetName();
                            } else {
                                transposon_name = transposon_name + ";" + (*it)->GetName();
                            }
                        }
                    } else if ((*it)->GetSubtype() == CSubSource::eSubtype_insertion_seq_name) {
                        is_insertion = true;
                        if ((*it)->IsSetName() && !NStr::IsBlank((*it)->GetName())) {
                            if (NStr::IsBlank(insertion_name)) {
                                insertion_name = (*it)->GetName();
                            } else {
                                insertion_name = insertion_name + ";" + (*it)->GetName();
                            }
                        }
                    } else if ((*it)->GetSubtype() == CSubSource::eSubtype_other
                               && (*it)->IsSetName()
                               && !NStr::IsBlank((*it)->GetName())) {
                        if (NStr::IsBlank(comment)) {
                            comment = (*it)->GetName();
                        } else {
                            comment = comment + ";" + (*it)->GetName();
                        }
                    }
                }
            }
        }
        if (orig.GetData().GetBiosrc().IsSetOrg()
            && orig.GetData().GetBiosrc().GetOrg().IsSetOrgname()
            && orig.GetData().GetBiosrc().GetOrg().GetOrgname().IsSetMod()) {
            ITERATE (COrgName::TMod, it, orig.GetData().GetBiosrc().GetOrg().GetOrgname().GetMod()) {
                if ((*it)->IsSetSubtype() && (*it)->GetSubtype() == COrgMod::eSubtype_other
                    && (*it)->IsSetSubname() && !NStr::IsBlank((*it)->GetSubname())) {
                    if (NStr::IsBlank(comment)) {
                        comment = (*it)->GetSubname();
                    } else {
                        comment = comment + ";" + (*it)->GetSubname();
                    }
                }
            }
        }
    }
    if (is_transposon) {
        CRef<CGb_qual> q(new CGb_qual("mobile_element", "transposon:" + transposon_name));
        new_feat->SetQual().push_back(q);
    }

    if (is_insertion) {
        CRef<CGb_qual> q(new CGb_qual("mobile_element", "insertion sequence:" + insertion_name));
        new_feat->SetQual().push_back(q);
    }

    if (!NStr::IsBlank(comment)) {
        new_feat->SetComment(comment);
    }


    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CRef<CCmdComposite> CConvertCDSToMiscFeat::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetImp().SetKey("misc_feature");
    new_feat->ResetProduct();

    EDIT_EACH_GBQUAL_ON_SEQFEAT(qual, *new_feat) 
    {
        const string& qual_str = (*qual)->GetQual();
        if ( NStr::Equal (qual_str, "gsdb_id")) 
            continue;
        CSeqFeatData::EQualifier gbqual = CSeqFeatData::GetQualifierType(qual_str);
        if ( gbqual != CSeqFeatData::eQual_bad && !CSeqFeatData::IsLegalQualifier(CSeqFeatData::eSubtype_misc_feature, gbqual) ) 
        {
            ERASE_GBQUAL_ON_SEQFEAT(qual, *new_feat);
        }
    }
    if (new_feat->IsSetQual() && new_feat->GetQual().empty())
        new_feat->ResetQual();
    
    bool cross_origin = false;
    const CSeq_loc& cds_loc = orig.GetLocation();
    if (cds_loc.GetStrand() == eNa_strand_plus) {
        cross_origin = cds_loc.GetStart(eExtreme_Positional) >= cds_loc.GetStop(eExtreme_Positional);
    }
    else if (cds_loc.GetStrand() == eNa_strand_minus) {
        cross_origin = cds_loc.GetStop(eExtreme_Positional) >= cds_loc.GetStart(eExtreme_Positional);
    }

    if (cross_origin) {
        // copy the location as is when it crosses the origin
        CRef<CSeq_loc> new_loc(new CSeq_loc);
        new_loc->Assign(orig.GetLocation());
        new_feat->SetLocation(*new_loc);
    }
    else {
        // otherwise, merge location into single interval
        CSeq_loc::TRange range = orig.GetLocation().GetTotalRange();
        ENa_strand strand = orig.GetLocation().GetStrand();
        bool part5 = orig.GetLocation().IsPartialStart(eExtreme_Positional);
        bool part3 = orig.GetLocation().IsPartialStop(eExtreme_Positional);

        CRef<CSeq_loc> new_loc(new CSeq_loc);
        CRef<CSeq_id> id(new CSeq_id);
        id->Assign(*orig.GetLocation().GetId());

        CRef<CSeq_interval> new_int(new CSeq_interval(*id, range.GetFrom(), range.GetTo(), strand));
        new_int->SetPartialStart(part5, eExtreme_Positional);
        new_int->SetPartialStop(part3, eExtreme_Positional);
        new_loc->SetInt(*new_int);
        new_feat->SetLocation(*new_loc);
    }

    string product = x_GetCDSProduct(orig, scope);
    if (!product.empty())
    {
        string comment = product;
        if (m_Similar)
        {
            comment = "similar to "+product;
        }
        else if (m_Nonfunctional)
        {
            comment = "nonfunctional "+product+" due to mutation";
        }
        x_AddToComment(comment, new_feat);
    }

    const string& prot_desc = x_GetProteinDesc(orig, scope);
    x_AddToComment(prot_desc, new_feat, edit::eExistingText_append_semi);

    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}

string CConvertCDSToMiscFeat::x_GetProteinDesc(const CSeq_feat& cds, CScope& scope)
{
    if (!cds.IsSetProduct()) {
        return kEmptyStr;
    }

    string desc(kEmptyStr);
    CBioseq_Handle bsh = scope.GetBioseqHandle(cds.GetProduct());
    if (bsh) {
        CFeat_CI f(bsh, SAnnotSelector(CSeqFeatData::eSubtype_prot));
        if (f && f->IsSetData() && f->GetData().IsProt()
            && f->GetData().GetProt().IsSetDesc()) {
            desc = f->GetData().GetProt().GetDesc();
        }
    }
    return desc;
}

CRef<CCmdComposite> CConvertImpToProt::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetProt().SetProcessed(GetProtProcessedFromFeatureSubtype(m_To));    
     
    if (!x_MoveFeatureToProtein(orig, new_feat, keep_orig, scope)) {
        return CRef<CCmdComposite>(NULL);
    }

    CConstRef<CSeq_feat> overlapping_cds = sequence::GetBestOverlappingFeat(orig.GetLocation(), CSeqFeatData::eSubtype_cdregion,
                                                                            sequence::eOverlap_Contained, scope, sequence::fBestFeat_Defaults | sequence::fBestFeat_FavorLonger);
    string product = x_GetCDSProduct(*overlapping_cds, scope);
    new_feat->SetData().SetProt().SetName().push_back(product);

    if (new_feat->IsSetQual()) {
        CSeq_feat::TQual::iterator it = new_feat->SetQual().begin();
        while (it != new_feat->SetQual().end()) {
            string val = "";
            if ((*it)->IsSetVal()) {
                val = (*it)->GetVal();
            }
            if (!(*it)->IsSetQual()) {
                it++;
            } else if (NStr::EqualNocase((*it)->GetQual(), "product")) {
                // product qualifier
                if (!NStr::IsBlank(val)) {
                    new_feat->SetData().SetProt().SetName().push_back((*it)->GetVal());
                }
                it = new_feat->SetQual().erase(it);
            } else if (NStr::EqualNocase((*it)->GetQual(), "EC_number")) {
                // EC number qualifiers
                if (!NStr::IsBlank(val)) {
                    new_feat->SetData().SetProt().SetEc().push_back((*it)->GetVal());
                }
                it = new_feat->SetQual().erase(it);
            } else if (NStr::EqualNocase((*it)->GetQual(), "function")) {
                // activity
                if (!NStr::IsBlank(val)) {
                    new_feat->SetData().SetProt().SetActivity().push_back((*it)->GetVal());
                }
                it = new_feat->SetQual().erase(it);
            } else if (NStr::EqualNocase((*it)->GetQual(), "dbxref")) {
                // dbxrefs
                if (!NStr::IsBlank(val)) {
                    CRef<CDbtag> db(new CDbtag());
                    size_t pos = NStr::Find(val, ":");
                    if (pos == string::npos) {
                        db->SetDb(val);
                    } else {
                        db->SetDb(val.substr(0, pos));
                        db->SetTag().SetStr(val.substr(pos));
                    }
                    new_feat->SetData().SetProt().SetDb().push_back(db);
                }
                it = new_feat->SetQual().erase(it);
            } else {
                it++;
            }
        }

    }

    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CRef<CCmdComposite> CConvertProtToImp::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CBioseq_Handle bsh = scope.GetBioseqHandle(orig.GetLocation());
    if (!bsh) {
        return CRef<CCmdComposite>(NULL);
    }
    const CSeq_feat * overlapping_cds = sequence::GetCDSForProduct(bsh);
    if (!overlapping_cds) {
        return CRef<CCmdComposite>(NULL);
    }

    // map to corresponding location on protein sequence
    CSeq_loc_Mapper mapper(*overlapping_cds, CSeq_loc_Mapper::eProductToLocation);
    mapper.SetMergeAbutting();
    CRef<CSeq_loc> loc = mapper.Map(orig.GetLocation());
    if ( !loc ) {
        return CRef<CCmdComposite>(NULL);
    }

    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetImp().SetKey(CSeqFeatData::SubtypeValueToName(m_To));
     
    new_feat->SetLocation().Assign(*loc);

    // product qualifier
    const CProt_ref& prot = orig.GetData().GetProt();
    if (prot.IsSetName() 
        && prot.GetName().size() > 0
        && !NStr::IsBlank (prot.GetName().front())) {
        CRef<CGb_qual> q(new CGb_qual("product", prot.GetName().front()));
        new_feat->SetQual().push_back(q);
    } else if (prot.IsSetDesc() 
        && !NStr::IsBlank(prot.GetDesc())) {
        CRef<CGb_qual> q(new CGb_qual("product", prot.GetDesc()));
        new_feat->SetQual().push_back(q);
    }

    // EC number qualifiers
    if (prot.IsSetEc()) {
        ITERATE(CProt_ref::TEc, it, prot.GetEc()) {
            CRef<CGb_qual> q(new CGb_qual("EC_number", (*it)));
            new_feat->SetQual().push_back(q);
        }
    }

    // activity
    if (prot.IsSetActivity()) {
        ITERATE(CProt_ref::TActivity, it, prot.GetActivity()) {
            CRef<CGb_qual> q(new CGb_qual("function", (*it)));
            new_feat->SetQual().push_back(q);
        }
    }

    // dbxrefs
    if (prot.IsSetDb()) {
        ITERATE(CProt_ref::TDb, it, prot.GetDb()) {
            string val = "";
            (*it)->GetLabel(&val);
            CRef<CGb_qual> q(new CGb_qual("dbxref", val));
            new_feat->SetQual().push_back(q);
        }
    }


    CRef<CCmdComposite> cmd(new CCmdComposite("Convert Feature"));

    // create new feature
    CBioseq_Handle nuc_bsh = scope.GetBioseqHandle(new_feat->GetLocation());
    CSeq_entry_Handle seh = nuc_bsh.GetSeq_entry_Handle();
    cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat)));
    // delete original coding region (will also remove product and renormalize nuc-prot set
    if (!keep_orig)
    {
        cmd->AddCommand(*GetDeleteFeatureCommand(scope.GetSeq_featHandle(orig)));
    } 

    return cmd;
}


CRef<CCmdComposite> CConvertImpToRNA::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetRna().SetType(GetRnaTypeFromFeatureSubtype(m_To));

    vector<string> products = x_ExtractProductQuals(new_feat);
    if (products.size() > 0) {
        string product = products[0];
        for(size_t i = 1; i < products.size(); i++) {
            product += "; " + products[i];
        }
        string remainder = "";
        new_feat->SetData().SetRna().SetRnaProductName(product, remainder);
        x_AddToComment(remainder, new_feat);
    }

    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CRef<CCmdComposite> CConvertImpToCDS::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetCdregion();
    vector<string> products = x_ExtractProductQuals(new_feat);
    if (products.size() == 0 && new_feat->IsSetComment() && !NStr::IsBlank(new_feat->GetComment())) {
        products.push_back(new_feat->GetComment());
        new_feat->ResetComment();
    }

    return x_ConvertToCDS(orig, new_feat, keep_orig, scope, products);
}


CRef<CCmdComposite> CConvertImpToGene::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetGene();

    vector<string> gene_vals = x_ExtractQuals(new_feat, "gene");
    if (gene_vals.size() == 0 && new_feat->IsSetComment() && !NStr::IsBlank(new_feat->GetComment())) {
        gene_vals.push_back(new_feat->GetComment());
        new_feat->ResetComment();
    }

    if (gene_vals.size() > 0) {
        string gene = gene_vals[0];
        for(size_t i = 1; i < gene_vals.size(); i++) {
            gene += "; " + gene_vals[i];
        }
        new_feat->SetData().SetGene().SetLocus(gene);
    }

    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CRef<CCmdComposite> CConvertRegionToImp::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetImp().SetKey(CSeqFeatData::SubtypeValueToName(m_To));
    x_AddToComment(orig.GetData().GetRegion(), new_feat);

    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CRef<CCmdComposite> CConvertRegionToRna::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetRna().SetType(GetRnaTypeFromFeatureSubtype(m_To));

    string product = orig.GetData().GetRegion();
    string remainder = "";
    new_feat->SetData().SetRna().SetRnaProductName(product, remainder);
    x_AddToComment(remainder, new_feat);

    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CRef<CCmdComposite> CConvertCommentToImp::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetImp().SetKey(CSeqFeatData::SubtypeValueToName(m_To));

    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CRef<CCmdComposite> CConvertGeneToImp::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetImp().SetKey(CSeqFeatData::SubtypeValueToName(m_To));
    string orig_comment = "";
    if (new_feat->IsSetComment()) {
        orig_comment = new_feat->GetComment();
    }
    new_feat->ResetComment();
    const CGene_ref& gene = orig.GetData().GetGene();
    if (gene.IsSetLocus() 
        && (m_To != CSeqFeatData::eSubtype_D_loop || !edit::CFieldHandler::QualifierNamesAreEquivalent(gene.GetLocus(), "D-Loop")) ) {
        x_AddToComment(gene.GetLocus(), new_feat);
    }
    if (gene.IsSetDesc()
        && (m_To != CSeqFeatData::eSubtype_D_loop || !edit::CFieldHandler::QualifierNamesAreEquivalent(gene.GetLocus(), "D-Loop"))) {
        x_AddToComment(gene.GetDesc(), new_feat);
    }
    x_AddToComment(orig_comment, new_feat);

    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CRef<CCmdComposite> CConvertRnaToImp::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetImp().SetKey(CSeqFeatData::SubtypeValueToName(m_To));
    string product = orig.GetData().GetRna().GetRnaProductName();
    x_AddToComment(product, new_feat);

    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CRef<CCmdComposite> CConvertmRNAToCDS::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetCdregion();

    vector<string> products;
    products.push_back(orig.GetData().GetRna().GetRnaProductName());

    return x_ConvertToCDS(orig, new_feat, keep_orig, scope, products);
}


CRef<CCmdComposite> CConverttRNAToGene::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);

    string product = orig.GetData().GetRna().GetRnaProductName();
    new_feat->SetData().SetGene().SetDesc(product);

    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CRef<CCmdComposite> CConvertSiteToImp::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetImp().SetKey(CSeqFeatData::SubtypeValueToName(m_To));
    string site_name = GetSiteName(orig.GetData().GetSite());
    x_AddToComment(site_name, new_feat);

    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CRef<CCmdComposite> CConvertRegionToProt::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    if (!x_MoveFeatureToProtein(orig, new_feat, keep_orig, scope)) {
        return CRef<CCmdComposite>(NULL);
    }
    new_feat->SetData().SetProt().SetName().push_back(orig.GetData().GetRegion());

    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CRef<CCmdComposite> CConvertRNAToRNA::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetRna().SetType(GetRnaTypeFromFeatureSubtype(m_To));
    string product = orig.GetData().GetRna().GetRnaProductName();
    string remainder = "";
    new_feat->SetData().SetRna().SetRnaProductName(product, remainder);
    x_AddToComment(remainder, new_feat);

    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CRef<CCmdComposite> CConvertToBond::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    x_ClearGenericFields(new_feat);
	if (orig.IsSetComment() && !NStr::IsBlank(orig.GetComment())) {
		new_feat->SetComment(orig.GetComment());
	}
    CSeq_loc_CI ci(orig.GetLocation());
    new_feat->SetLocation().SetBond().SetA().SetId().Assign(ci.GetSeq_id());
    new_feat->SetLocation().SetBond().SetA().SetPoint(orig.GetLocation().GetStart(eExtreme_Biological));
    CSeq_loc_CI ci_next = ci;
    ++ci_next;
    while (ci_next) {
        ++ci;
        ++ci_next;
    }
    new_feat->SetLocation().SetBond().SetB().SetId().Assign(ci.GetSeq_id());
    new_feat->SetLocation().SetBond().SetB().SetPoint(orig.GetLocation().GetStop(eExtreme_Biological));

    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CRef<CCmdComposite> CConvertToSite::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    x_ClearGenericFields(new_feat);
    if (orig.IsSetComment() && !NStr::IsBlank(orig.GetComment())) {
        new_feat->SetComment(orig.GetComment());
    }
    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


string CConvertToRegion::GetDescription() 
{
    if (x_IsProtSubtype(m_From)) {
        return "Create region feature with the protein name as the region name";
    } else {
        return "Create Region feature on nucleotide sequence or protein product sequence of overlapping coding region as specified.  Use comment on feature for region name.\n\
All feature ID, partialness, except, comment, product, location, genbank qualifiers, title, citation, experimental evidence, gene xrefs, db xrefs, and pseudo-ness information is discarded.";
    }
}


CRef<CCmdComposite> CConvertToRegion::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetRegion();
    x_ClearGenericFields(new_feat);
    if (orig.GetData().IsProt()) {
        if (orig.GetData().GetProt().IsSetName() && orig.GetData().GetProt().GetName().size() > 0) {
            new_feat->SetData().SetRegion(orig.GetData().GetProt().GetName().front());
        } 
    } else if (orig.IsSetComment()) {
        new_feat->SetData().SetRegion(orig.GetComment());
    }
    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


string CConvertCDSToProt::GetDescription()
{
    string subtype_name = CSeqFeatData::SubtypeValueToName(m_To);
    string desc = "If coding region is overlapped by another coding region, convert the coding region to a "
                  + subtype_name 
                  + " on the overlapping coding region's protein sequence, otherwise if you have checked \"Leave Original Feature\" it will create a "
                  + subtype_name + " with the same protein names and description on the protein sequence for the coding region.";
    return desc;
}


CRef<CCmdComposite> CConvertCDSToProt::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    if (!x_MoveFeatureToProtein(orig, new_feat, keep_orig, scope)) {
        return CRef<CCmdComposite>(NULL);
    }
    string product = x_GetCDSProduct(orig, scope);
    new_feat->SetData().SetProt().SetName().push_back(product);
    if (m_To == CSeqFeatData::eSubtype_mat_peptide_aa || m_To == CSeqFeatData::eSubtype_mat_peptide)
    {
        new_feat->ResetProduct();
        if (orig.IsSetProduct())
        {
            CBioseq_Handle prot_bsh = scope.GetBioseqHandle(orig.GetProduct());
            if (prot_bsh && prot_bsh.IsProtein())   
            {
                CFeat_CI prot_feat_ci(prot_bsh, CSeqFeatData::e_Prot);
                if (prot_feat_ci && prot_feat_ci->GetOriginalFeature().IsSetData() && prot_feat_ci->GetOriginalFeature().GetData().IsProt() 
                    && prot_feat_ci->GetOriginalFeature().GetData().GetProt().IsSetDesc())
                {
                    new_feat->SetData().SetProt().SetDesc(prot_feat_ci->GetOriginalFeature().GetData().GetProt().GetDesc());
                }
            }
        }
    }
    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CRef<CCmdComposite> CConvertCDSToGene::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetGene();
    
    if (orig.IsSetProduct()) {
        CBioseq_Handle bsh = scope.GetBioseqHandle(orig.GetProduct());
        if (bsh) {
            CFeat_CI f(bsh, SAnnotSelector(CSeqFeatData::eSubtype_prot));
            if (f && f->IsSetData() && f->GetData().IsProt()) {
                const CProt_ref& prot = f->GetData().GetProt();
                if (prot.IsSetName()) {
                    ITERATE(CProt_ref::TName, it, prot.GetName()) {
                        x_AddToComment(*it, new_feat);
                    }
                }
                if (prot.IsSetDesc()) {
                    x_AddToComment(prot.GetDesc(), new_feat);
                }
                if (prot.IsSetActivity()) {
                    ITERATE(CProt_ref::TActivity, it, prot.GetActivity()) {
                        x_AddToComment(*it, new_feat);
                    }
                }
                if (prot.IsSetDb()) {
                    ITERATE(CProt_ref::TDb, it, prot.GetDb()) {
                        string val = "";
                        (*it)->GetLabel(&val);
                        x_AddToComment(val, new_feat);
                    }
                }
                if (prot.IsSetEc()) {
                    ITERATE(CProt_ref::TEc, it, prot.GetEc()) {
                        x_AddToComment(*it, new_feat);
                    }
                }
            }
        }
    }
    new_feat->ResetProduct();
    CRef<CCmdComposite> cmd = x_SimpleConversion(orig, new_feat, keep_orig, scope);

    return cmd;
}


CRef<CConvertFeatureBase>
CConvertFeatureBaseFactory::Create(CSeqFeatData::ESubtype subtype_from, CSeqFeatData::ESubtype subtype_to)
{
    // choose appropriate converter
    
    CRef<CConvertFeatureBase> rval(NULL);

    if (subtype_from != subtype_to) {
        rval = new CConvertCDSToRNA(subtype_to);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        }
        rval = new CConvertGeneToRNA(subtype_to);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        }
        rval = new CConvertMiscFeatToGene();
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        }
        rval = new CConvertImpToImp(subtype_to, subtype_from);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        } 
        rval = new CConvertBioSrcToRptRegion();
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        } 
        rval = new CConvertCDSToMiscFeat();
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        } 
        rval = new CConvertImpToProt(subtype_to, subtype_from);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        } 
        rval = new CConvertImpToRNA(subtype_to, subtype_from);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        } 
        rval = new CConvertImpToCDS(subtype_from);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        } 
        rval = new CConvertImpToGene(subtype_from);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        } 
        rval = new CConvertRegionToImp(subtype_to);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        } 
        rval = new CConvertRegionToRna(subtype_to);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        } 
        rval = new CConvertCommentToImp(subtype_to);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        }
        rval = new CConvertGeneToImp(subtype_to);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        }
        rval = new CConvertRnaToImp(subtype_to, subtype_from);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        }
        rval = new CConvertmRNAToCDS(subtype_to, subtype_from);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        }
        rval = new CConverttRNAToGene(subtype_to, subtype_from);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        }
        rval = new CConvertSiteToImp(subtype_to);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        }
        rval = new CConvertRNAToRNA(subtype_to, subtype_from);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        }
        rval = new CConvertRegionToProt(subtype_to);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        }
        rval = new CConvertToBond(subtype_from);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        }
        rval = new CConvertToSite(subtype_from);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        }
        rval = new CConvertToRegion(subtype_from);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        }
        rval = new CConvertProtToProt(subtype_to, subtype_from);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        }
        rval = new CConvertCDSToProt(subtype_to);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        }
        rval = new CConvertCDSToGene();
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        }
        rval = new CConvertProtToImp(subtype_to, subtype_from);
        if (rval->CanConvertFrom(subtype_from) && rval->CanConvertTo(subtype_to)) {
            return rval;
        } 
       
    }

    // return "no conversion found" if no appropriate one found
    rval = new CConvertFeatureBase();
    return rval;
}


CRef<CConversionOptionBool> CConversionOption::SetBool()
{
    if (m_Choice != e_Bool) {
        CRef<CConversionOptionBool> new_obj(new CConversionOptionBool());
        m_Object = new_obj;
    }
    CRef<CConversionOptionBool> obj(dynamic_cast<CConversionOptionBool *>(m_Object.GetPointer()));
    m_Choice = e_Bool;
    return obj;
}


CConstRef<CConversionOptionBool> CConversionOption::GetBool() const
{
    if (m_Choice != e_Bool) {
        return CConstRef<CConversionOptionBool>(NULL);
    } else {
        return CConstRef<CConversionOptionBool>(dynamic_cast<const CConversionOptionBool *>(m_Object.GetPointer()));
    }
}


CRef<CConversionOptionString> CConversionOption::SetString()
{
    if (m_Choice != e_String) {
        CRef<CConversionOptionString> new_obj(new CConversionOptionString("", ""));
        m_Object = new_obj;
    }
    CRef<CConversionOptionString> obj(dynamic_cast<CConversionOptionString *>(m_Object.GetPointer()));
    m_Choice = e_String;
    return obj;
}


CConstRef<CConversionOptionString> CConversionOption::GetString() const
{
    if (m_Choice != e_String) {
        return CConstRef<CConversionOptionString>(NULL);
    } else {
        return CConstRef<CConversionOptionString>(dynamic_cast<const CConversionOptionString *>(m_Object.GetPointer()));
    }
}

CConvertCDSToPseudoGene::CConvertCDSToPseudoGene() 
{
    m_To = CSeqFeatData::eSubtype_gene;
    m_From = CSeqFeatData::eSubtype_cdregion;
    m_Options.clear(); 
    //x_AddCDSSourceConversionOptions();
    //CRef<CConversionOption> is_pseudo(new CConversionOption(kIsPseudo));
    //is_pseudo->SetBool();
    //m_Options.push_back(is_pseudo);
}

CRef<CCmdComposite> CConvertCDSToPseudoGene::Convert(const CSeq_feat& orig, bool keep_orig, CScope& scope)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Convert Feature"));

    CConstRef<CSeq_feat> gene = sequence::GetOverlappingGene(orig.GetLocation(), scope);

    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig);
    new_feat->SetData().SetGene();
    if (gene)
    {
        CRef<CSeq_feat> copy_gene(new CSeq_feat());
        copy_gene->Assign(*gene);
        new_feat->SetData().SetGene(copy_gene->SetData().SetGene());
        cmd->AddCommand(*GetDeleteFeatureCommand(scope.GetSeq_featHandle(*gene)));
    }
    
    CRef<CGene_ref> gene_ref(&new_feat->SetData().SetGene());
    if (orig.IsSetProduct()) {
        CBioseq_Handle bsh = scope.GetBioseqHandle(orig.GetProduct());
        if (bsh) {
            CFeat_CI f(bsh, SAnnotSelector(CSeqFeatData::eSubtype_prot));
            if (f && f->IsSetData() && f->GetData().IsProt()) {
                const CProt_ref& prot = f->GetData().GetProt();

                if (f->IsSetComment()) {
                    x_AddToComment(f->GetComment(), new_feat);
                }
                if (prot.IsSetDesc()) {
                    x_AddToComment(prot.GetDesc(), new_feat);
                }

                if (!gene_ref->IsSetLocus()) {
                    // use protein product names for gene locus if gene does not already have one
                    if (prot.IsSetName()) {
                        ITERATE (CProt_ref::TName, it, prot.GetName()) {
                            x_AddToGeneLocus(*it, gene_ref, edit::eExistingText_append_semi);

                        }
                    }
                } else if (!gene_ref->IsSetDesc()) {
                    // use protein product names for gene description if gene does not already have one
                    if (prot.IsSetName()) {
                        ITERATE (CProt_ref::TName, it, prot.GetName()) {
                            x_AddToGeneDescr(*it, gene_ref, edit::eExistingText_append_semi);
                        }
                    }
                } else {
                    // prepend list of protein product names to gene comment
                    if (prot.IsSetName()) {
                        string protein_names;
                        ITERATE (CProt_ref::TName, it, prot.GetName()) {
                            edit::AddValueToString(protein_names, *it, edit::eExistingText_append_semi);
                        }
                        x_AddToComment(protein_names, new_feat);
                    }
                }
            }
        }
    }
    
    new_feat->SetPseudo(true);
    new_feat->ResetProduct();

    if (keep_orig) {
        CBioseq_Handle bsh = scope.GetBioseqHandle(new_feat->GetLocation());
        CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat)));
    } else if (orig.IsSetData() && orig.GetData().IsCdregion()) {
        // create new feature
        CBioseq_Handle bsh = scope.GetBioseqHandle(new_feat->GetLocation());
        CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat)));
        // delete original coding region (will also remove product and renormalize nuc-prot set
        cmd->AddCommand(*GetDeleteFeatureCommand(scope.GetSeq_featHandle(orig)));
    } else {
        cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(scope.GetSeq_featHandle(orig), *new_feat)));
    }

    return cmd;
}

void CConvertCDSToPseudoGene::x_AddToGeneLocus(string text, CRef<CGene_ref> gene, edit::EExistingText existing_text)
{
    if (NStr::IsBlank(text)) {
        return;
    }
    
    if (!gene->IsSetLocus() || NStr::IsBlank(gene->GetLocus())) {
        gene->SetLocus(text);
    } else {
        string orig_val = gene->GetLocus();
        if (edit::AddValueToString(orig_val, text, existing_text)) {
            gene->SetLocus(orig_val);
        }
    }
}

void CConvertCDSToPseudoGene::x_AddToGeneDescr(string text, CRef<CGene_ref> gene, edit::EExistingText existing_text)
{
    if (NStr::IsBlank(text)) {
        return;
    }
    
    if (!gene->IsSetDesc() || NStr::IsBlank(gene->GetDesc())) {
        gene->SetDesc(text);
    } else {
        string orig_val = gene->GetDesc();
        if (edit::AddValueToString(orig_val, text, existing_text)) {
            gene->SetDesc(orig_val);
        }
    }
}


END_NCBI_SCOPE
