/*  $Id: convert_cds_with_gaps_to_misc_feat.cpp 41973 2018-11-26 17:21:37Z kachalos $
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

////@begin includes
////@end includes
#include <objects/seq/Seq_ext.hpp>
#include <objects/seq/Delta_ext.hpp>
#include <objects/seq/Delta_seq.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>
#include <objtools/edit/text_object_description.hpp>
#include <objtools/format/flat_file_config.hpp>
#include <objtools/format/flat_file_generator.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/seq_map.hpp>
#include <objmgr/seq_map_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objtools/validator/utilities.hpp>
#include <gui/packages/pkg_sequence_edit/split_cds_with_too_many_n.hpp>
#include <gui/packages/pkg_sequence_edit/convert_cds_with_gaps_to_misc_feat.hpp>

#include <wx/stattext.h>
#include <wx/msgdlg.h> 



////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CConvertCdsWithGapsToMiscFeat type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CConvertCdsWithGapsToMiscFeat, CBulkCmdDlg )


/*!
 * CConvertCdsWithGapsToMiscFeat event table definition
 */

BEGIN_EVENT_TABLE( CConvertCdsWithGapsToMiscFeat, CBulkCmdDlg )

////@begin CConvertCdsWithGapsToMiscFeat event table entries
////@end CConvertCdsWithGapsToMiscFeat event table entries
END_EVENT_TABLE()


/*!
 * CConvertCdsWithGapsToMiscFeat constructors
 */

CConvertCdsWithGapsToMiscFeat::CConvertCdsWithGapsToMiscFeat()
{
    Init();
}

CConvertCdsWithGapsToMiscFeat::CConvertCdsWithGapsToMiscFeat( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : CBulkCmdDlg(wb), m_Converter(NULL)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CConvertCdsWithGapsToMiscFeat creator
 */

bool CConvertCdsWithGapsToMiscFeat::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CConvertCdsWithGapsToMiscFeat creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CConvertCdsWithGapsToMiscFeat creation
    return true;
}


/*!
 * CConvertCdsWithGapsToMiscFeat destructor
 */

CConvertCdsWithGapsToMiscFeat::~CConvertCdsWithGapsToMiscFeat()
{
////@begin CConvertCdsWithGapsToMiscFeat destruction
////@end CConvertCdsWithGapsToMiscFeat destruction
}


/*!
 * Member initialisation
 */

void CConvertCdsWithGapsToMiscFeat::Init()
{
////@begin CConvertCdsWithGapsToMiscFeat member initialisation
    m_StringConstraintPanel = NULL;
    m_OkCancel = NULL;
    m_RadioButton = NULL;
////@end CConvertCdsWithGapsToMiscFeat member initialisation
    m_Converter.Reset(new CConvertCDSToMiscFeat()); 

    CRef<CConversionOption> opt = m_Converter->FindRemoveTranscriptId();
    if (opt) {
        opt->SetBool()->SetVal(true);
    }

}


/*!
 * Control creation for CConvertCdsWithGapsToMiscFeat
 */

void CConvertCdsWithGapsToMiscFeat::CreateControls()
{    
////@begin CConvertCdsWithGapsToMiscFeat content construction
    CConvertCdsWithGapsToMiscFeat* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    m_RadioButton = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("Convert only when internal gap covers 50% or more of the coding region"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_RadioButton->SetValue(true);
    itemBoxSizer2->Add(m_RadioButton, 0, wxALIGN_LEFT|wxALL, 5);

    wxRadioButton* itemRadioButton9 = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("Convert all coding regions with gaps (both terminal and internal)"));
    itemRadioButton9->SetValue(false);
    itemBoxSizer2->Add(itemRadioButton9, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText1 = new wxStaticText( itemCBulkCmdDlg1, wxID_STATIC, _("Where feature text"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText1, 0, wxALIGN_LEFT|wxALL, 5);

    m_StringConstraintPanel = new CStringConstraintPanel( itemCBulkCmdDlg1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_StringConstraintPanel, 0, wxALIGN_LEFT|wxALL|wxFIXED_MINSIZE, 0); 

    m_OkCancel = new COkCancelPanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CConvertCdsWithGapsToMiscFeat content construction
}


/*!
 * Should we show tooltips?
 */

bool CConvertCdsWithGapsToMiscFeat::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CConvertCdsWithGapsToMiscFeat::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CConvertCdsWithGapsToMiscFeat bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CConvertCdsWithGapsToMiscFeat bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CConvertCdsWithGapsToMiscFeat::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CConvertCdsWithGapsToMiscFeat icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CConvertCdsWithGapsToMiscFeat icon retrieval
}


CRef<CCmdComposite> CConvertCdsWithGapsToMiscFeat::GetCommand()
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Convert CDS with gaps to misc_feat"));

    CRef<edit::CStringConstraint> string_constraint(m_StringConstraintPanel->GetStringConstraint());

    
    bool any_change = false;
    bool any_present = false;

    for (objects::CFeat_CI feat_it(m_TopSeqEntry, CSeqFeatData::eSubtype_cdregion); feat_it; ++feat_it)
    {
        objects::CSeq_feat_Handle fh = feat_it->GetSeq_feat_Handle();
        const objects::CSeq_loc& feat_loc = feat_it->GetLocation();
        CRef<CSeq_loc> total_loc = sequence::Seq_loc_Merge(feat_loc, CSeq_loc::fMerge_SingleRange, &m_TopSeqEntry.GetScope()); 

        CConstRef<objects::CSeqMap> seq_map = objects::CSeqMap::GetSeqMapForSeq_loc(*total_loc, &m_TopSeqEntry.GetScope());
        
        objects::CSeqMap_CI seq_map_ci = seq_map->ResolvedRangeIterator(&m_TopSeqEntry.GetScope(),
                                                                        0,
                                                                        objects::sequence::GetLength(feat_loc,&m_TopSeqEntry.GetScope()),
                                                                        feat_loc.GetStrand(),
                                                                        size_t(-1),
                                                                        objects::CSeqMap::fFindGap);
        //objects::CSeqMap_CI seq_map_ci = seq_map->BeginResolved(&m_TopSeqEntry.GetScope(), objects::SSeqMapSelector(objects::CSeqMap::fFindGap));

        bool gap_present(false);
        for (; seq_map_ci; ++seq_map_ci)
        {
            if (seq_map_ci.GetType() == objects::CSeqMap::eSeqGap)
            {
                //TSeqPos start = seq_map_ci.GetPosition();
                //TSeqPos length = seq_map_ci.GetLength();
                gap_present = true;
            }
        }
        bool match_constraint(true);
        if (string_constraint)
        {
            CFlatFileConfig cfg;
            cfg.SetNeverTranslateCDS();
            string str =  CFlatFileGenerator::GetSeqFeatText(*feat_it, m_TopSeqEntry.GetScope(), cfg);
            match_constraint = string_constraint->DoesTextMatch(str);
        }
        
        bool too_many_x(true);
        if (m_RadioButton->GetValue())
        {
            too_many_x = CSplitCDSwithTooManyXs::TooManyXs(fh);
        }

        if (gap_present && match_constraint && too_many_x)
        {
            CRef<CCmdComposite> subcmd = m_Converter->Convert(*fh.GetOriginalSeq_feat(), false, m_TopSeqEntry.GetScope());
            if (subcmd) 
            {
                cmd->AddCommand(*subcmd);
                any_change = true;         
            }
            any_present = true;
        }
    }  
    if (!any_present)
    {
        m_ErrorMessage = "No features found!";
        cmd.Reset(NULL);
    }
    else if (!any_change) 
    {
        m_ErrorMessage = "All conversions failed.";
        cmd.Reset(NULL);
    }
    return cmd;
}



string CConvertCdsWithGapsToMiscFeat::GetErrorMessage()
{
    return m_ErrorMessage;
}



CRef<CCmdComposite> CConvertBadCdsAndRnaToMiscFeat::apply(wxWindow *parent, objects::CSeq_entry_Handle seh)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Convert bad CDS and mRNA to misc_feat"));
    CScope &scope = seh.GetScope();
    int modified = 0;
    int total = 0;
    string msg;

    CRef<CConvertCDSToMiscFeat> converter(new CConvertCDSToMiscFeat()); 
    
    CRef<CConversionOption> opt = converter->FindRemoveTranscriptId();
    if (opt) {
        opt->SetBool()->SetVal(true);
    }
    for (objects::CFeat_CI feat_it(seh); feat_it; ++feat_it)
    {
        objects::CSeq_feat_Handle fh = feat_it->GetSeq_feat_Handle();
        if (fh.GetFeatType() == CSeqFeatData::e_Cdregion || fh.GetFeatType() == CSeqFeatData::e_Rna)
        {
            total++;
            const CSeq_feat &feat = *fh.GetOriginalSeq_feat();
            CBioseq_Handle bsh = scope.GetBioseqHandle(feat.GetLocation());
            bool cds_validation = fh.GetFeatType() == CSeqFeatData::e_Cdregion && (validator::HasStopInProtein(feat, scope) ||
                                                                                   validator::HasInternalStop(feat, scope, false) ||
                                                                                   validator::HasBadStartCodon(feat, scope, false) ||
                                                                                   validator::HasNoStop(feat, &scope) ||
                                                                                   validator::HasBadProteinStart(feat,  scope));
            bool disc_short_rrna = getShortRRNA(feat, scope);
            bool disc_rna_cds_overlap = getCdsRnaOverlap(feat, bsh);
            if (cds_validation      ||
                getBacterialPartialNonextendable(feat, bsh) ||
                disc_short_rrna                   ||
                disc_rna_cds_overlap
                )
            {
                CRef<CCmdComposite> subcmd = converter->Convert(*fh.GetOriginalSeq_feat(), false, scope);
                if (subcmd) 
                {
                    cmd->AddCommand(*subcmd);
                    modified++;       
                    msg += edit::GetTextObjectDescription(feat, scope) + " converted to misc_feature\n";
                    //cout << NDiscrepancy::CReportObj::GetTextObjectDescription(feat, scope);
                    //cout << " " << validator::HasStopInProtein(feat, scope); 
                    //cout << " " << validator::HasInternalStop(feat, scope, false);
                    //cout << " " << validator::HasBadStartCodon(feat, scope, false);
                    //cout << " " << validator::HasNoStop(feat, &scope);
                    //cout << " " << validator::HasBadProteinStart(feat,  scope);
                    //cout << " " << getBacterialPartialNonextendable(feat, bsh);
                    //cout << " " << disc_short_rrna;
                    //cout << " " << disc_rna_cds_overlap << endl;
                    if (disc_short_rrna || disc_rna_cds_overlap)
                    {
                        CSeq_feat_Handle gene;
                        if (fh.GetFeatType() == CSeqFeatData::e_Cdregion)
                            gene = feature::GetBestGeneForCds(*feat_it).GetSeq_feat_Handle();
                        else
                        {
                            CConstRef<CSeq_feat> gene_feat = sequence::GetOverlappingGene(feat.GetLocation(), scope);
                            if (gene_feat)
                                gene = scope.GetSeq_featHandle(*gene_feat, CScope::eMissing_Null);
                        }
                        if (gene)
                            cmd->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(gene)));
                    }
                }
            }
        }
    }  

    if (modified == 0 || modified * 2 > total)
    {
        cmd.Reset();
        if (modified * 2 > total)
            wxMessageBox(_("More than 50% of coding regions and RNA features are bad"), wxT("Info"), wxOK | wxICON_INFORMATION);
    }
    else
    {
        CGenericReportDlg* report = new CGenericReportDlg(parent);
        report->SetTitle(wxT("Feature Conversions"));
        report->SetText(msg);
        report->Show();
    }

    return cmd;
}

// BACTERIAL_PARTIAL_NONEXTENDABLE_PROBLEMS

bool CConvertBadCdsAndRnaToMiscFeat::IsExtendableLeft(TSeqPos left, const CBioseq& seq, CScope* scope, TSeqPos& extend_len)
{
    bool rval = false;
    if (left < 3) {
        rval = true;
        extend_len = left;
    } else if (seq.IsSetInst() && seq.GetInst().IsSetRepr() &&
               seq.GetInst().GetRepr() == CSeq_inst::eRepr_delta &&
               seq.GetInst().IsSetExt() &&
               seq.GetInst().GetExt().IsDelta()) {
        TSeqPos offset = 0;
        TSeqPos last_gap_stop = 0;
        ITERATE(CDelta_ext::Tdata, it, seq.GetInst().GetExt().GetDelta().Get()) {
            if ((*it)->IsLiteral()) {
                offset += (*it)->GetLiteral().GetLength();
                if (!(*it)->GetLiteral().IsSetSeq_data()) {
                    last_gap_stop = offset;
                } else if ((*it)->GetLiteral().GetSeq_data().IsGap()) {
                    last_gap_stop = offset;
                }
            } else if ((*it)->IsLoc()) {
                offset += sequence::GetLength((*it)->GetLoc(), scope);
            }
            if (offset > left) {
                break;
            }
        }
        if (left - last_gap_stop < 3) {
            rval = true;
            extend_len = left - last_gap_stop;
        }
    }
    return rval;
}


bool CConvertBadCdsAndRnaToMiscFeat::IsExtendableRight(TSeqPos right, const CBioseq& seq, CScope* scope, TSeqPos& extend_len)
{
    bool rval = false;
    if (right > seq.GetLength() - 4) {
        rval = true;
        extend_len = seq.GetLength() - right - 1;
    } else if (seq.IsSetInst() && seq.GetInst().IsSetRepr() &&
        seq.GetInst().GetRepr() == CSeq_inst::eRepr_delta &&
        seq.GetInst().IsSetExt() &&
        seq.GetInst().GetExt().IsDelta()) {
        TSeqPos offset = 0;
        TSeqPos next_gap_start = 0;
        ITERATE(CDelta_ext::Tdata, it, seq.GetInst().GetExt().GetDelta().Get()) {
            if ((*it)->IsLiteral()) {
                if (!(*it)->GetLiteral().IsSetSeq_data()) {
                    next_gap_start = offset;
                } else if ((*it)->GetLiteral().GetSeq_data().IsGap()) {
                    next_gap_start = offset;
                }
                offset += (*it)->GetLiteral().GetLength();
            } else if ((*it)->IsLoc()) {
                offset += sequence::GetLength((*it)->GetLoc(), scope);
            }
            if (offset > right + 3) {
                break;
            }
        }
        if (next_gap_start > right && next_gap_start - right < 3) {
            rval = true;
            extend_len = next_gap_start - right;
        }
    }
    return rval;
}


bool CConvertBadCdsAndRnaToMiscFeat::IsNonExtendable(const CSeq_loc& loc, const CBioseq& seq, CScope* scope)
{
    bool rval = false;
    if (loc.IsPartialStart(eExtreme_Positional)) {
        TSeqPos start = loc.GetStart(eExtreme_Positional);
        if (start > 0) {
            TSeqPos extend_len = 0;
            if (!IsExtendableLeft(start, seq, scope, extend_len)) {
                rval = true;
            }
        }
    }
    if (!rval && loc.IsPartialStop(eExtreme_Positional)) {
        TSeqPos stop = loc.GetStop(eExtreme_Positional);
        if (stop < seq.GetLength() - 1) {
            TSeqPos extend_len = 0;
            if (!IsExtendableRight(stop, seq, scope, extend_len)) {
                rval = true;
            }
        }
    }
    return rval;
}


bool CConvertBadCdsAndRnaToMiscFeat::getBacterialPartialNonextendable(const CSeq_feat &feat, CBioseq_Handle bsh)
{
    if (bsh.IsAa())
        return false;

    for (CSeqdesc_CI source_ci(bsh, CSeqdesc::e_Source); source_ci; ++source_ci)
    {
        const CBioSource &biosource = source_ci->GetSource();
        if (biosource.IsSetLineage() && NStr::FindNoCase(biosource.GetLineage(), "Eukaryota") != string::npos)
            return false;
    }    

    //only examine coding regions
    if (!feat.IsSetData() || !feat.GetData().IsCdregion()) {
        return false;
    }
    //ignore if feature already has exception
    if (feat.IsSetExcept_text() && NStr::FindNoCase(feat.GetExcept_text(), "unextendable partial coding region") != string::npos) {
        return false;
    }
    CConstRef<CBioseq> seq = bsh.GetBioseqCore();

    return IsNonExtendable(feat.GetLocation(), *seq, &(bsh.GetScope()));
}


// DISC_SHORT_RRNA
typedef pair<size_t, bool> TRNALength;
typedef map<const char*, TRNALength > TRNALengthMap;

static const TRNALengthMap kTrnaLengthMap{
    { "16S", { 1000, false } },
    { "18S", { 1000, false } },
    { "23S", { 2000, false } },
    { "25S", { 1000, false } },
    { "26S", { 1000, false } },
    { "28S", { 1000, false } },
    { "28S", { 3300, false } },
    { "small", { 1000, false } },
    { "large", { 1000, false } },
    { "5.8S", { 130, true } },
    { "5S", { 90, true } }
};


bool CConvertBadCdsAndRnaToMiscFeat::IsShortrRNA(const CSeq_feat& f, CScope* scope) 
{
    if (f.GetData().GetSubtype() != CSeqFeatData::eSubtype_rRNA) {
        return false;
    }
 
    bool is_bad = false;

    size_t len = sequence::GetLength(f.GetLocation(), scope);

    string rrna_name = f.GetData().GetRna().GetRnaProductName();

    ITERATE (TRNALengthMap, it, kTrnaLengthMap) {
        if (NStr::FindNoCase(rrna_name, it->first) != string::npos &&
            len < it->second.first &&
            (!it->second.second || (f.IsSetPartial() && f.GetPartial())) ) {
            is_bad = true;
            break;
        }
    }

    return is_bad;
}


bool CConvertBadCdsAndRnaToMiscFeat::getShortRRNA(const CSeq_feat &feat, CScope &scope)
{
    return !feat.IsSetPartial() && IsShortrRNA(feat, &scope);
}


//  DISC_RNA_CDS_OVERLAP

bool CConvertBadCdsAndRnaToMiscFeat::getCdsOverlap(const CSeq_feat &cds, CBioseq_Handle bsh)
{
    CScope &scope = bsh.GetScope();

    bool is_eukaryotic = false;
    for (CSeqdesc_CI source_ci(bsh, CSeqdesc::e_Source); source_ci; ++source_ci)
    {
        const CBioSource &biosource = source_ci->GetSource();
        CBioSource::EGenome genome = (CBioSource::EGenome) biosource.GetGenome();
        if (genome != CBioSource::eGenome_mitochondrion
            && genome != CBioSource::eGenome_chloroplast
            && genome != CBioSource::eGenome_plastid
            && genome != CBioSource::eGenome_apicoplast
            && (biosource.IsSetLineage() && NStr::FindNoCase(biosource.GetLineage(), "Eukaryota") != string::npos) )
            is_eukaryotic = true;;
    }    


    for (objects::CFeat_CI feat_it(bsh, CSeqFeatData::e_Rna); feat_it; ++feat_it)
    {
        objects::CSeq_feat_Handle fh = feat_it->GetSeq_feat_Handle();

        const CSeq_loc& loc_i = fh.GetLocation();
        CSeqFeatData::ESubtype subtype = fh.GetFeatSubtype();
        if (subtype == CSeqFeatData::eSubtype_tRNA) 
        {

            if (is_eukaryotic) 
            {
                continue;
            }
        }
        else if (subtype == CSeqFeatData::eSubtype_mRNA || subtype == CSeqFeatData::eSubtype_ncRNA) 
        {
            continue;
        }
        else if (subtype == CSeqFeatData::eSubtype_rRNA) 
        {
            size_t len = sequence::GetLength(loc_i, &scope);
            string rrna_name = fh.GetData().GetRna().GetRnaProductName();
            bool is_bad = false;
            ITERATE (TRNALengthMap, it, kTrnaLengthMap) 
            {
                if (NStr::FindNoCase(rrna_name, it->first) != string::npos && len < it->second.first && (!it->second.second || (fh.IsSetPartial() && fh.GetPartial())) ) 
                {
                    is_bad = true;
                    break;
                }
            }
            if (is_bad) 
            {
                continue;
            }
        }
    

   
        
        const CSeq_loc& loc_j = cds.GetLocation();
        sequence::ECompare compare;
        {
            CSeq_loc::TRange r1 = loc_j.GetTotalRange();
            CSeq_loc::TRange r2 = loc_i.GetTotalRange();
            if (r1.GetFrom() >= r2.GetToOpen() || r2.GetFrom() >= r1.GetToOpen()) 
            {
                compare =  sequence::eNoOverlap;
            }
            else
            {
                compare = sequence::Compare(loc_j, loc_i, &scope, sequence::fCompareOverlapping);
            }
        }
        
        
        if (compare == sequence::eSame) 
        {
            return true;
        }
        else if (compare == sequence::eContained) 
        {
            return true; // no Fatal();
        }
        else if (compare == sequence::eContains) 
        {
            if (fh.GetFeatSubtype() == CSeqFeatData::eSubtype_tRNA) 
            {
                return true;
            }
            else 
            {
                return true;
            }
        }
        else if (compare != sequence::eNoOverlap) 
        {
            ENa_strand cds_strand = loc_j.GetStrand();
            ENa_strand rna_strand = loc_i.GetStrand();
            if (cds_strand == eNa_strand_minus && rna_strand != eNa_strand_minus) 
            {
                return true; // no Fatal();
            }
            else if (cds_strand != eNa_strand_minus && rna_strand == eNa_strand_minus) 
            {
                return true; // no Fatal();
            }
            else 
            {
                return true; // no Fatal();
            }
        }
    }
    return false;
}

bool CConvertBadCdsAndRnaToMiscFeat::getRnaOverlap(const CSeq_feat &rna, CBioseq_Handle bsh)
{
    CScope &scope = bsh.GetScope();

    bool is_eukaryotic = false;
    for (CSeqdesc_CI source_ci(bsh, CSeqdesc::e_Source); source_ci; ++source_ci)
    {
        const CBioSource &biosource = source_ci->GetSource();
        CBioSource::EGenome genome = (CBioSource::EGenome) biosource.GetGenome();
        if (genome != CBioSource::eGenome_mitochondrion
            && genome != CBioSource::eGenome_chloroplast
            && genome != CBioSource::eGenome_plastid
            && genome != CBioSource::eGenome_apicoplast
            && (biosource.IsSetLineage() && NStr::FindNoCase(biosource.GetLineage(), "Eukaryota") != string::npos) )
            is_eukaryotic = true;;
    }    


   
    const CSeq_loc& loc_i = rna.GetLocation();
    CSeqFeatData::ESubtype subtype = rna.GetData().GetSubtype();
    if (subtype == CSeqFeatData::eSubtype_tRNA) 
    {
        
        if (is_eukaryotic) 
        {
            return false;
        }
    }
    else if (subtype == CSeqFeatData::eSubtype_mRNA || subtype == CSeqFeatData::eSubtype_ncRNA) 
    {
        return false;
    }
    else if (subtype == CSeqFeatData::eSubtype_rRNA) 
    {
        size_t len = sequence::GetLength(loc_i, &scope);
        string rrna_name = rna.GetData().GetRna().GetRnaProductName();
        bool is_bad = false;
        ITERATE (TRNALengthMap, it, kTrnaLengthMap) 
        {
            if (NStr::FindNoCase(rrna_name, it->first) != string::npos && len < it->second.first && (!it->second.second || (rna.IsSetPartial() && rna.GetPartial())) ) 
            {
                is_bad = true;
                break;
            }
        }
        if (is_bad) 
        {
            return false;
        }
    }
    

    for (objects::CFeat_CI feat_it(bsh, CSeqFeatData::e_Cdregion); feat_it; ++feat_it)
    {
        objects::CSeq_feat_Handle fh = feat_it->GetSeq_feat_Handle();
        
        const CSeq_loc& loc_j = fh.GetLocation();
        sequence::ECompare compare;
        {
            CSeq_loc::TRange r1 = loc_j.GetTotalRange();
            CSeq_loc::TRange r2 = loc_i.GetTotalRange();
            if (r1.GetFrom() >= r2.GetToOpen() || r2.GetFrom() >= r1.GetToOpen()) 
            {
                compare =  sequence::eNoOverlap;
            }
            else
            {
                compare = sequence::Compare(loc_j, loc_i, &scope, sequence::fCompareOverlapping);
            }
        }
        
        
        if (compare == sequence::eSame) 
        {
            return true;
        }
        else if (compare == sequence::eContained) 
        {
            return true; // no Fatal();
        }
        else if (compare == sequence::eContains) 
        {
            if (fh.GetFeatSubtype() == CSeqFeatData::eSubtype_tRNA) 
            {
                return true;
            }
            else 
            {
                return true;
            }
        }
        else if (compare != sequence::eNoOverlap) 
        {
            ENa_strand cds_strand = loc_j.GetStrand();
            ENa_strand rna_strand = loc_i.GetStrand();
            if (cds_strand == eNa_strand_minus && rna_strand != eNa_strand_minus) 
            {
                return true; // no Fatal();
            }
            else if (cds_strand != eNa_strand_minus && rna_strand == eNa_strand_minus) 
            {
                return true; // no Fatal();
            }
            else 
            {
                return true; // no Fatal();
            }
        }
    }
    return false;
}

bool CConvertBadCdsAndRnaToMiscFeat::getCdsRnaOverlap(const CSeq_feat &feat, CBioseq_Handle bsh)
{
    if (feat.IsSetData() && feat.GetData().IsCdregion())
        return getCdsOverlap(feat, bsh);
    if (feat.IsSetData() && feat.GetData().IsRna())
        return getRnaOverlap(feat, bsh);
    return false;
}

END_NCBI_SCOPE
