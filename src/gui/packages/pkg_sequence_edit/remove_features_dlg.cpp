/*  $Id: remove_features_dlg.cpp 45097 2020-05-29 15:15:00Z asztalos $
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
#include <sstream>
#include <objmgr/scope.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seq_feat_handle.hpp>
#include <objmgr/util/feature.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/cmd_del_seq_annot.hpp>
#include <gui/objutils/cmd_del_bioseq.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <objtools/validator/utilities.hpp>
//#include <corelib/perf_log.hpp>
//#include <gui/utils/log_performance.hpp>

//#include <gui/widgets/text_widget/draw_styled_text.hpp>
#include <gui/objutils/label.hpp>
#include <objtools/format/flat_file_config.hpp>
#include <objtools/format/flat_file_generator.hpp>
#include <gui/packages/pkg_sequence_edit/remove_sequences.hpp>
#include <gui/packages/pkg_sequence_edit/remove_features_dlg.hpp>


#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/msgdlg.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CRemoveFeaturesDlg, CBulkCmdDlg )


BEGIN_EVENT_TABLE( CRemoveFeaturesDlg, CBulkCmdDlg )


END_EVENT_TABLE()

CRemoveFeaturesDlg::CRemoveFeaturesDlg()
{
    Init();
}

CRemoveFeaturesDlg::CRemoveFeaturesDlg( wxWindow* parent, IWorkbench *wb, 
                                        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CRemoveFeaturesDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    SetSize(wxSize(280, 750));
    return true;
}


CRemoveFeaturesDlg::~CRemoveFeaturesDlg()
{
}


/*!
 * Member initialisation
 */

void CRemoveFeaturesDlg::Init()
{
    set<string> existing;
    m_Types.clear();
    m_Descriptions.clear();

    GetTopLevelSeqEntryAndProcessor();
    vector<const CFeatListItem *> feat_list = GetSortedFeatList(m_TopSeqEntry);
    ITERATE(vector<const CFeatListItem *>, ft_it, feat_list) {
        const CFeatListItem& item = **ft_it;
        string desc = item.GetDescription();
        int feat_type = item.GetType();
        int feat_subtype = item.GetSubtype();
        m_Types[pair<int,int>(feat_type,feat_subtype)] = desc;
        if (existing.find(desc) == existing.end())
        {
            existing.insert(desc);
            m_Descriptions.push_back(desc);
            if (feat_subtype ==  CSeqFeatData::eSubtype_any)
                m_SubtypeAnyFlag.push_back(true);
            else
                m_SubtypeAnyFlag.push_back(false);
        }
    }
        
    m_ListCtrl = NULL;
    m_TopSeqEntry.Reset();
    m_ErrorMessage = "No feature found";
}




void CRemoveFeaturesDlg::CreateControls()
{    
    CRemoveFeaturesDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ListCtrl = new wxListCtrl( itemDialog1, ID_LISTCTRL_FTR, wxDefaultPosition, wxSize(250, 200), wxLC_REPORT );
    itemBoxSizer3->Add(m_ListCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ListCtrl->InsertColumn(0,"    Feature",wxLIST_FORMAT_LEFT,237);
  
    for (unsigned int i=0; i<m_Descriptions.size(); i++)
        m_ListCtrl->InsertItem(i,ToWxString(m_Descriptions[i]));

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer5->Add(itemBoxSizer6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Optional String Constraint"));
    wxStaticBoxSizer* itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxVERTICAL);
    itemBoxSizer6->Add(itemStaticBoxSizer7, 0, wxALIGN_LEFT|wxALL, 5);

    m_RadioButton = new wxRadioButton( itemDialog1, ID_REMOVE_FEATURES_RADIOBUTTON, _("Remove When Text Is Present"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_RadioButton->SetValue(true);
    itemStaticBoxSizer7->Add(m_RadioButton, 0, wxALIGN_LEFT|wxALL, 5);

    wxRadioButton* itemRadioButton9 = new wxRadioButton( itemDialog1, ID_REMOVE_FEATURES_RADIOBUTTON1, _("Remove When Text Is Not Present"));
    itemRadioButton9->SetValue(false);
    itemStaticBoxSizer7->Add(itemRadioButton9, 0, wxALIGN_LEFT|wxALL, 5);

    m_TextCtrl = new wxTextCtrl( itemDialog1, ID_REMOVE_FEATURES_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(237, -1), 0 );
    itemStaticBoxSizer7->Add(m_TextCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_CheckBox = new wxCheckBox( itemDialog1, ID_REMOVE_FEATURES_CHECKBOX, _("Case Insensitive"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CheckBox->SetValue(false);
    itemStaticBoxSizer7->Add(m_CheckBox, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer15Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Optional Seq-id Constraint"));
    wxStaticBoxSizer* itemStaticBoxSizer15 = new wxStaticBoxSizer(itemStaticBoxSizer15Static, wxVERTICAL);
    itemBoxSizer6->Add(itemStaticBoxSizer15, 0, wxALIGN_LEFT|wxALL, 5);

    m_RadioButton_id = new wxRadioButton( itemDialog1, ID_REMOVE_FEATURES_RADIOBUTTON2, _("Remove When Text Is Present"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_RadioButton_id->SetValue(true);
    itemStaticBoxSizer15->Add(m_RadioButton_id, 0, wxALIGN_LEFT|wxALL, 5);

    wxRadioButton* itemRadioButton16 = new wxRadioButton( itemDialog1, ID_REMOVE_FEATURES_RADIOBUTTON3, _("Remove When Text Is Not Present"));
    itemRadioButton16->SetValue(false);
    itemStaticBoxSizer15->Add(itemRadioButton16, 0, wxALIGN_LEFT|wxALL, 5);

    m_TextCtrl_id = new wxTextCtrl( itemDialog1, ID_REMOVE_FEATURES_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(237, -1), 0 );
    itemStaticBoxSizer15->Add(m_TextCtrl_id, 0, wxALIGN_LEFT|wxALL, 5);

    m_CheckBox_id = new wxCheckBox( itemDialog1, ID_REMOVE_FEATURES_CHECKBOX1, _("Case Insensitive"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CheckBox->SetValue(false);
    itemStaticBoxSizer15->Add(m_CheckBox_id, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer17Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Optional Location Constraint"));
    wxStaticBoxSizer* itemStaticBoxSizer17 = new wxStaticBoxSizer(itemStaticBoxSizer17Static, wxVERTICAL);
    itemBoxSizer6->Add(itemStaticBoxSizer17, 0, wxALIGN_LEFT|wxALL, 5);

    wxArrayString strands,types;
    strands.Add(wxT("Any Strand"));
    strands.Add(wxT("Plus Strand"));
    strands.Add(wxT("Minus Strand"));
    types.Add(wxT("Nucleotide and protein sequences"));
    types.Add(wxT("Nucleotide sequences only"));
    types.Add(wxT("Protein sequences only"));

    m_ChoiceStrand = new wxChoice(itemDialog1, ID_REMOVE_FEATURES_CHOICE1, wxDefaultPosition, wxDefaultSize, strands,0,wxDefaultValidator,"Strand");
    m_ChoiceStrand->SetSelection(0);
    itemStaticBoxSizer17->Add(m_ChoiceStrand, 0, wxALIGN_LEFT|wxALL, 5);
    m_ChoiceType = new wxChoice(itemDialog1, ID_REMOVE_FEATURES_CHOICE2, wxDefaultPosition, wxDefaultSize, types,0,wxDefaultValidator,"Type");
    m_ChoiceType->SetSelection(0);
    itemStaticBoxSizer17->Add(m_ChoiceType, 0, wxALIGN_LEFT|wxALL, 5);

    COkCancelPanel *OkCancel = new COkCancelPanel( itemDialog1, ID_REMOVE_FEATURES_OKCANCEL, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
}


/*!
 * Should we show tooltips?
 */

bool CRemoveFeaturesDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRemoveFeaturesDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CRemoveFeaturesDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CCmdComposite> CRemoveFeaturesDlg::GetCommand()
{
    long item = -1;
    CRef<CCmdComposite> cmd( new CCmdComposite("Remove Features") );
    bool modified = false;
    if (m_ListCtrl && m_TopSeqEntry)
    {
        m_CountProteins = 0;
	map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle> > product_to_cds;
	GetProductToCDSMap(m_TopSeqEntry.GetScope(), product_to_cds);
       

        m_CollectInfoAboutProteins = false;
        bool remove_orphans = true;
	m_RemoveProteins = true;

        for ( ;; )
        {
            item = m_ListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if ( item == -1 )  break;
            string name = ToStdString(m_ListCtrl->GetItemText(item));
            bool subtypeAnyFlag = m_SubtypeAnyFlag[item];
            ApplyToCSeq_entry (m_TopSeqEntry, cmd, name, subtypeAnyFlag, modified, product_to_cds);
        }

        if (remove_orphans)
        {
            CRef<CCmdComposite> cmd_remove_orphans = RemoveOrphansRenormalizeNucProtSets(m_TopSeqEntry);
            cmd->AddCommand(*cmd_remove_orphans);
            modified = true;
        }
    }

    if (modified)
        return cmd;
    else
    {
    CRef<CCmdComposite> empty(NULL);
        return empty;
    }
}

static void s_RenderAccList(
    stringstream& ostream, const CUser_field::TData::TStrs& item_data, const string& title)
{
    string s(title);
    if ( !s.empty() ) {
        s += ": ";
    }

    list<string> refseq;
    list<string> non_refseq;
    ITERATE (CUser_field::TData::TStrs, it, item_data) {
        CSeq_id::EAccessionInfo info = CSeq_id::IdentifyAccession(*it);
        if ((info & CSeq_id::eAcc_type_mask) == CSeq_id::e_Other) {
            refseq.push_back(*it);
        } else {
            non_refseq.push_back(*it);
        }
    }

    string accs;
    if (refseq.size()) {
        accs += NStr::Join(refseq, ", ");
    }

    if (non_refseq.size()) {
        if ( !accs.empty() ) {
            accs += ", ";
        }
        accs += NStr::Join(non_refseq, ", ");
    }

    if ( !accs.empty() ) {
        s += accs;
    } else {
        s += "<unknown>";
    }

    list<string> lines;
    NStr::Wrap(s, 78, lines);
    if ( !lines.empty() ) {
        ITERATE (list<string>, iter, lines) {
            if (iter != lines.begin()) {
                ostream << "    " << *iter << endl;
            } else {
                ostream << *iter << endl;
            }
        }
    }
}

static void s_Render_Feat_Evidence(
    stringstream& ostream, const CSeq_feat& feat, CScope& /*scope*/)
{
    if ( !feat.IsSetExt()  &&  !feat.IsSetExts()) {
        return;
    }

    list< CConstRef<CUser_object> > objs;
    if (feat.IsSetExt()) {
        CConstRef<CUser_object> o(&feat.GetExt());
        if (o->IsSetType()  &&  o->GetType().IsStr()  && o->GetType().GetStr() == "CombinedFeatureUserObjects") {
            ITERATE (CUser_object::TData, it, o->GetData()) {
                const CUser_field& field = **it;
                CConstRef<CUser_object> sub_o(&field.GetData().GetObject());
                objs.push_back(sub_o);
            }
        } else {
            objs.push_back(o);
        }
    }
    if (feat.IsSetExts()) {
        copy(feat.GetExts().begin(), feat.GetExts().end(), back_inserter(objs));
    }

    ITERATE (list< CConstRef<CUser_object> >, obj_it, objs) {
        const CUser_object& obj = **obj_it;
        if ( !obj.IsSetType()  ||
             !obj.GetType().IsStr()  ||
             obj.GetType().GetStr() != "ModelEvidence") {
            continue;
        }

        string s;
        CConstRef<CUser_field> f;

        ostream << endl;

        ///
        /// generation method
        string method;
        ostream << "Model Generation Method: ";
        f = obj.GetFieldRef("Method");
        if (f) {
            method = f->GetData().GetStr();
            ostream << method;
        } else {
            ostream << "<unknown>";
        }

        ostream << endl;

        ///
        /// list of support
        if (method == "Chainer"  ||  method == "Gnomon") {

            ///
            /// scan for multiple 'Core' entries
            f = obj.GetFieldRef("Support");
            if (f) {
                string core;
                ITERATE (CUser_field::TData::TFields, it, f->GetData().GetFields()) {
                    const CUser_field& this_f = **it;
                    if (this_f.GetLabel().GetStr() == "Core") {
                        if ( !core.empty() ) {
                            core += ", ";
                        }
                        core += this_f.GetData().GetStr();
                    }
                }
                if ( !core.empty() ) {
                    s = "Model Based On: ";
                    s += core;
                    ostream << s << endl;
                }
            }

            ///
            /// supporting proteins
            f = obj.GetFieldRef("Support.Proteins");
            if (f) {
                s_RenderAccList(ostream, f->GetData().GetStrs(), "Source Proteins");
            }

            ///
            /// supporting proteins
            f = obj.GetFieldRef("Support.mRNAs");
            if (f) {
                s_RenderAccList(ostream, f->GetData().GetStrs(), "Source mRNAs");
            }
        }
    }
}

static void s_Render_SeqFeat(
    stringstream& ostream, const objects::CSeq_feat& feat, objects::CScope& scope)
{
    try {
        string text;
        /// start with the description text
        CLabel::GetLabel( feat, &text, CLabel::eDescription, &scope );
        ostream << text << endl;

        /// add information about the feature's location
        TSeqRange range = feat.GetLocation().GetTotalRange();
        ostream << "Total Range:  ";
        switch( sequence::GetStrand(feat.GetLocation(), &scope) ){
        case eNa_strand_plus:
            ostream << "(+) ";
            break;
        case eNa_strand_minus:
            ostream << "(-) ";
            break;
        default:
            break;
        }

        ostream << NStr::IntToString(range.GetFrom() + 1, NStr::fWithCommas)
                << " - "
                << NStr::IntToString(range.GetTo() + 1, NStr::fWithCommas)
                << endl;

        ostream << "Total Length:  "
                << NStr::IntToString(range.GetLength(), NStr::fWithCommas)
                << endl;

        ostream << "Processed Length:  "
                << NStr::IntToString(sequence::GetLength(feat.GetLocation(), &scope),
                                  NStr::fWithCommas)
                << endl;

        if (feat.IsSetProduct()) {
            try {
                string prod_len_str;
                CBioseq_Handle h = scope.GetBioseqHandle(feat.GetProduct());
                if (h) {
                    ostream << "Product Length: "
                            << NStr::IntToString(h.GetBioseqLength(),
                                              NStr::fWithCommas)
                            << endl;
                }
            }
            catch (CException&) {
            }
        }
        ostream << endl;

        /// next, include the flat file text for this feature
        CFlatFileConfig cfg;
        cfg.SetNeverTranslateCDS();

        /// hack: search for this feature using CFeat_CI
        /// it'd be better to use CMappedFeat instead of CSeq_feat
        SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
        sel.IncludeFeatSubtype(feat.GetData().GetSubtype());

        CFeat_CI feat_it(scope, feat.GetLocation(), sel);
        for (; feat_it ; ++feat_it) {
            if (&feat_it->GetOriginalFeature() != &feat){
                continue;
            }
            string s =
                CFlatFileGenerator::GetSeqFeatText(*feat_it, scope, cfg);
            string::size_type pos = s.find_first_not_of(" \n\r\t");
            if( pos != string::npos ){
                s.erase(0, pos);
            }

            string::size_type lpos = string::npos;
            do {
                pos = lpos +1;
                lpos = s.find_first_of("\n\r", pos );

                if( lpos == string::npos ){
                    text = s.substr( pos );
                } else {
                    text = s.substr( pos, lpos - pos );
                }
                ostream << text << endl;

            } while( lpos != string::npos );
        } // for

        /// lastly, add items relating to model evidence
        s_Render_Feat_Evidence(ostream, feat, scope);
    }
    catch (CException&) {
    }
}

void CRemoveFeaturesDlg::ApplyToCSeq_entry (objects::CSeq_entry_Handle tse, CCmdComposite* composite, string name, bool subtypeAnyFlag, bool &modified, 
					    map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle> > &product_to_cds)  // see SeqEntryConstrainedFeaturesCallback
{
    map< CConstRef<CSeq_feat>, CIRef<IEditCommand> > stored_commands;   
    set<CSeq_id_Handle> set_of_proteins;
    CFeat_CI feat (tse); 
    while (feat) 
    { 
        bool filter = false;
      if (NStr::EqualNocase(name, "All"))
      {
          filter = true;
      }
      else  if (subtypeAnyFlag)
      {
          int type = feat->GetOriginalFeature().GetData().Which();
          int subtype = CSeqFeatData::eSubtype_any;
          string desc = m_Types[pair<int,int>(type,subtype)];
          if (desc == name)
              filter = true;
      }
      else if (feat->GetOriginalFeature().CanGetData())
      {
          int type = feat->GetOriginalFeature().GetData().Which();
          int subtype = feat->GetOriginalFeature().GetData().GetSubtype(); 
          string desc = m_Types[pair<int,int>(type,subtype)];
          if (desc == name)
              filter = true;
      }

        if (filter)
        {
            bool go = true;
            if (m_TextCtrl->GetLineLength(0) > 0)
            {
                go = false;
                stringstream str;
                //str << MSerial_AsnText << feat->GetOriginalFeature();
                CScope &scope = tse.GetScope();
                s_Render_SeqFeat(str, feat->GetOriginalFeature(), scope);
                string pattern = m_TextCtrl->GetLineText(0).ToStdString();
                bool match(false);
                if (m_CheckBox->GetValue())
                    match = NStr::FindNoCase(str.str(), pattern) != NPOS;
                else
                    match = NStr::Find(str.str(), pattern) != NPOS;
                if (match && m_RadioButton->GetValue()) go = true;
                else if (!match && !m_RadioButton->GetValue()) go = true;
            }
            if (go && m_TextCtrl_id->GetLineLength(0) > 0)
            {
                go = false;
                string pattern = m_TextCtrl_id->GetLineText(0).ToStdString();
                bool match = false;                
                bool no_case = m_CheckBox_id->GetValue();
                
                CBioseq_Handle bsh = tse.GetScope().GetBioseqHandle(feat->GetLocation());
                if (bsh && bsh.IsSetId()) 
                {
                    for (CBioseq_Handle::TId::const_iterator idh = bsh.GetId().begin(); idh != bsh.GetId().end(); ++idh)
                    {
                        string str;
                        try
                        {
                            CConstRef<CSeq_id>  id = idh->GetSeqId();
                            id->GetLabel(&str);
                        } catch(const CException&) {}
                        if (!str.empty())
                        {
                            if (no_case)
                                match = NStr::FindNoCase(str, pattern) != NPOS;
                            else
                                match = NStr::Find(str, pattern) != NPOS;
                            if (match)
                                break;
                        }
                    }
                }
                if (match && m_RadioButton_id->GetValue()) go = true;
                else if (!match && !m_RadioButton_id->GetValue()) go = true;
            }

            if (go && m_ChoiceStrand->GetSelection()>0)
            {
                go = false;
                int n = m_ChoiceStrand->GetSelection();
                wxString strand = m_ChoiceStrand->GetString(n);
                if (strand == wxT("Plus Strand") && feat->GetOriginalFeature().IsSetLocation() && feat->GetOriginalFeature().GetLocation().GetStrand() == eNa_strand_plus) go = true;
                if (strand == wxT("Minus Strand") && feat->GetOriginalFeature().IsSetLocation() && feat->GetOriginalFeature().GetLocation().GetStrand() == eNa_strand_minus) go = true;
            }

            if (go && m_ChoiceType->GetSelection()>0)
            {
                go = false;
                int n = m_ChoiceType->GetSelection();
                wxString type = m_ChoiceType->GetString(n);
                if (type == wxT("Nucleotide sequences only") && feat->GetOriginalFeature().IsSetLocation())
                {
                    CBioseq_Handle bsh = tse.GetScope().GetBioseqHandle(feat->GetLocation());
                    if (bsh.IsNa())
                        go = true;
                }
                if (type == wxT("Protein sequences only") && feat->GetOriginalFeature().IsSetLocation())
                {
                    CBioseq_Handle bsh = tse.GetScope().GetBioseqHandle(feat->GetLocation());
                    if (bsh.IsAa())
                        go = true;
                }
            }
            if (go)
            {
                CSeq_feat_Handle fh = feat->GetSeq_feat_Handle();
                if (fh)
                {
                    if (m_CollectInfoAboutProteins)
                    {
                        CSeq_id_Handle id = GetProductId(fh);
                        if (id)
                            set_of_proteins.insert(id);
                    }
                    else
                    {
                        CRef<CCmdComposite> cmd = GetDeleteFeatureCommand(fh, m_RemoveProteins, product_to_cds);
                        stored_commands[feat->GetSeq_feat()] = cmd;
                        composite->AddCommand(*cmd);
                        modified = true;
                    }
                }
            }
        }
        ++feat;
    }
    m_CountProteins = set_of_proteins.size();
}


CSeq_id_Handle CRemoveFeaturesDlg::GetProductId(objects::CSeq_feat_Handle fh)
{
    CSeq_id_Handle id;
    CBioseq_Handle this_seq = fh.GetScope().GetBioseqHandle(fh.GetLocationId());
    CSeq_entry_Handle top_parent = this_seq.GetSeq_entry_Handle();
    if (this_seq && top_parent && fh.IsSetProduct()) 
    {
        if (!validator::IsFarLocation(fh.GetProduct(), top_parent) && fh.GetProductId()) 
        {
            CBioseq_Handle product = fh.GetScope().GetBioseqHandle(*(fh.GetProductId().GetSeqId()));

            if (product && product.IsProtein() && product.GetSeqId()) 
            {   
                id = CSeq_id_Handle::GetHandle(*product.GetSeqId());
            }
        }
    }
    return id;
}

int CRemoveFeaturesDlg::CountProteinsBeforeRemoval(CSeq_entry_Handle seh)
{
    set<CSeq_id_Handle> set_of_proteins;
    CBioseq_CI bi(seh, objects::CSeq_inst::eMol_na);
    while (bi) 
    {
        CFeat_CI it(*bi);
        while (it) 
        {
            objects::CSeq_feat_Handle fh = it->GetSeq_feat_Handle();
            CSeq_id_Handle id = GetProductId(fh);
            if (id)
               set_of_proteins.insert(id);
            ++it;
        }
        ++bi;
    }
    return set_of_proteins.size();
}

int CRemoveFeaturesDlg::CountOrphanedProteins(objects::CSeq_entry_Handle seh)
{
    int num = 0;
    if (seh)
    {
        set<CBioseq_Handle> proteins;
        for (CFeat_CI fi(seh); fi; ++fi)
        {
            if ( fi->IsSetProduct())
            {
                CBioseq_Handle prot_bsh = fi->GetScope().GetBioseqHandle(fi->GetProduct());
                if (prot_bsh && prot_bsh.IsProtein())
                {
                    proteins.insert(prot_bsh);
                }
            }
        }
        objects::CBioseq_CI b_iter(seh, objects::CSeq_inst::eMol_aa);                
        for ( ; b_iter ; ++b_iter ) 
        {
            CBioseq_Handle bsh = *b_iter;
            if (proteins.find(bsh) == proteins.end())
            {
                num++;
            }
        }       
    }
    return num;
}

void CRemoveFeaturesDlg::AskUser(int num_proteins, int num_orphans, bool &remove_proteins, bool &remove_orphans)
{
    remove_proteins = false;
    remove_orphans = false;
    if (num_proteins + num_orphans == 0)
        return;

    if (num_orphans == 0)
    {
        remove_proteins = true;
        return;
    }
    if (num_proteins == 0)
    {
        remove_orphans = true;
        return;
    }
    remove_proteins = true;
    remove_orphans = true;
}

void CRemoveFeaturesDlg::RemoveAllFeatures(CSeq_entry_Handle seh, ICommandProccessor* processor)
{
    wxBusyCursor wait;
//    CLogPerformance perfLog("RemoveAllFeatures");
   // CStopWatch sw(CStopWatch::eStart);
    bool remove_proteins = true;
    bool remove_orphans = true;
    size_t count = 0;
    CRef<CCmdComposite> cmd = GetDeleteAllFeaturesCommand (seh, count, remove_proteins);
    if (remove_orphans)
    {
        CRef<CCmdComposite> cmd_remove_orphans = RemoveOrphansRenormalizeNucProtSets(seh);
        cmd->AddCommand(*cmd_remove_orphans);
    }
    if (count > 0) {
        processor->Execute(cmd);
    } else {
        wxMessageBox(_("No features found!"), wxT("Error"),
                 wxOK | wxICON_ERROR, NULL);
    }
//    perfLog.Post(CRequestStatus::e200_Ok);
   // sw.Stop();
   // wxMessageBox(wxString(sw.AsSmartString()));
}

void CRemoveFeaturesDlg::RemoveDuplicateFeatures (objects::CSeq_entry_Handle tse, ICommandProccessor* processor) 
{
    bool remove_proteins = true;
    bool remove_orphans = true;

    map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle> > product_to_cds;
    GetProductToCDSMap(tse.GetScope(), product_to_cds);
    CRef<CCmdComposite> cmd( new CCmdComposite("Remove Duplicate Features") );
    bool modified = false;
    set< CSeq_feat_Handle > deleted_feats;
    for (CFeat_CI feat1(tse); feat1; ++feat1)
    {
        for (CFeat_CI feat2(tse.GetScope(),feat1->GetLocation()); feat2; ++feat2)
        {
            if (feat1->GetSeq_feat_Handle() < feat2->GetSeq_feat_Handle() &&
                deleted_feats.find(feat1->GetSeq_feat_Handle()) == deleted_feats.end() &&
                deleted_feats.find(feat2->GetSeq_feat_Handle()) == deleted_feats.end()) {
                validator::EDuplicateFeatureType dup_type = validator::IsDuplicate(feat1->GetSeq_feat_Handle(), feat2->GetSeq_feat_Handle());
                if (dup_type == validator::eDuplicate_Duplicate || dup_type == validator::eDuplicate_DuplicateDifferentTable) {
                    cmd->AddCommand(*GetDeleteFeatureCommand(feat2->GetSeq_feat_Handle(), remove_proteins, product_to_cds));
                    deleted_feats.insert(feat2->GetSeq_feat_Handle());
                    modified = true;
                }
            }
        }
    }
    if (remove_orphans)
    {
        CRef<CCmdComposite> cmd_remove_orphans = RemoveOrphansRenormalizeNucProtSets(tse);
        cmd->AddCommand(*cmd_remove_orphans);
        modified = true;
    }
    if (modified)
    {
        processor->Execute(cmd);
    }
}

CRef<CCmdComposite> CRemoveFeaturesDlg::RemoveOrphansRenormalizeNucProtSets(objects::CSeq_entry_Handle seh)
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Remove Orphaned Proteins") );
    if (seh)
    {
        set<CBioseq_Handle> proteins;
        for (CFeat_CI fi(seh, CSeqFeatData::eSubtype_cdregion); fi; ++fi)
        {
	    if (fi->IsSetProduct())
	    {
		CBioseq_Handle prot_bsh = fi->GetScope().GetBioseqHandle(fi->GetProduct());
		if (prot_bsh && prot_bsh.IsProtein())
		{
		    proteins.insert(prot_bsh);
		}
	    }
        }
        objects::CBioseq_CI b_iter(seh, objects::CSeq_inst::eMol_aa);                
        for ( ; b_iter ; ++b_iter ) 
        {
            CBioseq_Handle bsh = *b_iter;
            if (proteins.find(bsh) == proteins.end())
            {
                cmd->AddCommand(*CRef<CCmdDelBioseqInst>(new CCmdDelBioseqInst(bsh)));
            }
        }       
    }
    return cmd;
}

/*
 * CAskToRemoveOrphansAndProteins type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAskToRemoveOrphansAndProteins, wxDialog )


/*
 * CAskToRemoveOrphansAndProteins event table definition
 */

BEGIN_EVENT_TABLE( CAskToRemoveOrphansAndProteins, wxDialog )

////@begin CAskToRemoveOrphansAndProteins event table entries
////@end CAskToRemoveOrphansAndProteins event table entries

END_EVENT_TABLE()


/*
 * CAskToRemoveOrphansAndProteins constructors
 */

CAskToRemoveOrphansAndProteins::CAskToRemoveOrphansAndProteins()
{
    Init();
}

CAskToRemoveOrphansAndProteins::CAskToRemoveOrphansAndProteins( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * CAskToRemoveOrphansAndProteins creator
 */

bool CAskToRemoveOrphansAndProteins::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAskToRemoveOrphansAndProteins creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAskToRemoveOrphansAndProteins creation
    return true;
}


/*
 * CAskToRemoveOrphansAndProteins destructor
 */

CAskToRemoveOrphansAndProteins::~CAskToRemoveOrphansAndProteins()
{
////@begin CAskToRemoveOrphansAndProteins destruction
////@end CAskToRemoveOrphansAndProteins destruction
}


/*
 * Member initialisation
 */

void CAskToRemoveOrphansAndProteins::Init()
{
////@begin CAskToRemoveOrphansAndProteins member initialisation
    m_Proteins = NULL;
    m_Orphans = NULL;
////@end CAskToRemoveOrphansAndProteins member initialisation
}


/*
 * Control creation for CAskToRemoveOrphansAndProteins
 */

void CAskToRemoveOrphansAndProteins::CreateControls()
{    
////@begin CAskToRemoveOrphansAndProteins content construction
    // Generated by DialogBlocks, 19/01/2016 10:45:51 (unregistered)

    CAskToRemoveOrphansAndProteins* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_Proteins = new wxCheckBox( itemDialog1, ID_RMVPROTEINS_CHECKBOX, _("Remove proteins orphaned due to deleted features?      "), wxDefaultPosition, wxDefaultSize, 0 );
    m_Proteins->SetValue(true);
    itemBoxSizer2->Add(m_Proteins, 0, wxALIGN_LEFT|wxALL, 5);

    m_Orphans = new wxCheckBox( itemDialog1, ID_RMVPROTEINS_CHECKBOX1, _("Remove previously orphaned proteins?      "), wxDefaultPosition, wxDefaultSize, 0 );
    m_Orphans->SetValue(true);
    itemBoxSizer2->Add(m_Orphans, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CAskToRemoveOrphansAndProteins content construction
}


/*
 * Should we show tooltips?
 */

bool CAskToRemoveOrphansAndProteins::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CAskToRemoveOrphansAndProteins::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAskToRemoveOrphansAndProteins bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAskToRemoveOrphansAndProteins bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CAskToRemoveOrphansAndProteins::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAskToRemoveOrphansAndProteins icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAskToRemoveOrphansAndProteins icon retrieval
}


END_NCBI_SCOPE
