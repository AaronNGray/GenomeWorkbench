/*  $Id: correct_interval_order.cpp 42430 2019-02-22 13:19:16Z filippov $
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
#include <objtools/edit/loc_edit.hpp>

//#include <gui/widgets/text_widget/draw_styled_text.hpp>
#include <gui/objutils/label.hpp>
#include <objtools/format/flat_file_config.hpp>
#include <objtools/format/flat_file_generator.hpp>
#include <gui/packages/pkg_sequence_edit/correct_interval_order.hpp>


#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/msgdlg.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


IMPLEMENT_DYNAMIC_CLASS( CCorrectIntervalOrder, CBulkCmdDlg )


BEGIN_EVENT_TABLE( CCorrectIntervalOrder, CBulkCmdDlg )


END_EVENT_TABLE()

CCorrectIntervalOrder::CCorrectIntervalOrder()
{
    Init();
}

CCorrectIntervalOrder::CCorrectIntervalOrder( wxWindow* parent, IWorkbench *wb, 
                                        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CCorrectIntervalOrder::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
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


CCorrectIntervalOrder::~CCorrectIntervalOrder()
{
}


/*!
 * Member initialisation
 */

void CCorrectIntervalOrder::Init()
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




void CCorrectIntervalOrder::CreateControls()
{    
    CCorrectIntervalOrder* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ListCtrl = new wxListCtrl( itemDialog1, ID_LISTCTRL_CORRECT_INTERVAL_ORDER, wxDefaultPosition, wxSize(250, 200), wxLC_REPORT );
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

    m_RadioButton = new wxRadioButton( itemDialog1, ID_CORRECT_INTERVAL_ORDER_RADIOBUTTON, _("Remove When Text Is Present"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_RadioButton->SetValue(true);
    itemStaticBoxSizer7->Add(m_RadioButton, 0, wxALIGN_LEFT|wxALL, 5);

    wxRadioButton* itemRadioButton9 = new wxRadioButton( itemDialog1, ID_CORRECT_INTERVAL_ORDER_RADIOBUTTON1, _("Remove When Text Is Not Present"));
    itemRadioButton9->SetValue(false);
    itemStaticBoxSizer7->Add(itemRadioButton9, 0, wxALIGN_LEFT|wxALL, 5);

    m_TextCtrl = new wxTextCtrl( itemDialog1, ID_CORRECT_INTERVAL_ORDER_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(237, -1), 0 );
    itemStaticBoxSizer7->Add(m_TextCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_CheckBox = new wxCheckBox( itemDialog1, ID_CORRECT_INTERVAL_ORDER_CHECKBOX, _("Case Insensitive"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CheckBox->SetValue(false);
    itemStaticBoxSizer7->Add(m_CheckBox, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer15Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Optional Seq-id Constraint"));
    wxStaticBoxSizer* itemStaticBoxSizer15 = new wxStaticBoxSizer(itemStaticBoxSizer15Static, wxVERTICAL);
    itemBoxSizer6->Add(itemStaticBoxSizer15, 0, wxALIGN_LEFT|wxALL, 5);

    m_RadioButton_id = new wxRadioButton( itemDialog1, ID_CORRECT_INTERVAL_ORDER_RADIOBUTTON2, _("Remove When Text Is Present"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_RadioButton_id->SetValue(true);
    itemStaticBoxSizer15->Add(m_RadioButton_id, 0, wxALIGN_LEFT|wxALL, 5);

    wxRadioButton* itemRadioButton16 = new wxRadioButton( itemDialog1, ID_CORRECT_INTERVAL_ORDER_RADIOBUTTON3, _("Remove When Text Is Not Present"));
    itemRadioButton16->SetValue(false);
    itemStaticBoxSizer15->Add(itemRadioButton16, 0, wxALIGN_LEFT|wxALL, 5);

    m_TextCtrl_id = new wxTextCtrl( itemDialog1, ID_CORRECT_INTERVAL_ORDER_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(237, -1), 0 );
    itemStaticBoxSizer15->Add(m_TextCtrl_id, 0, wxALIGN_LEFT|wxALL, 5);

    m_CheckBox_id = new wxCheckBox( itemDialog1, ID_CORRECT_INTERVAL_ORDER_CHECKBOX1, _("Case Insensitive"), wxDefaultPosition, wxDefaultSize, 0 );
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

    m_ChoiceStrand = new wxChoice(itemDialog1, ID_CORRECT_INTERVAL_ORDER_CHOICE1, wxDefaultPosition, wxDefaultSize, strands,0,wxDefaultValidator,"Strand");
    m_ChoiceStrand->SetSelection(0);
    itemStaticBoxSizer17->Add(m_ChoiceStrand, 0, wxALIGN_LEFT|wxALL, 5);
    m_ChoiceType = new wxChoice(itemDialog1, ID_CORRECT_INTERVAL_ORDER_CHOICE2, wxDefaultPosition, wxDefaultSize, types,0,wxDefaultValidator,"Type");
    m_ChoiceType->SetSelection(0);
    itemStaticBoxSizer17->Add(m_ChoiceType, 0, wxALIGN_LEFT|wxALL, 5);

    COkCancelPanel *OkCancel = new COkCancelPanel( itemDialog1, ID_CORRECT_INTERVAL_ORDER_OKCANCEL, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
}


/*!
 * Should we show tooltips?
 */

bool CCorrectIntervalOrder::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCorrectIntervalOrder::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CCorrectIntervalOrder::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CCmdComposite> CCorrectIntervalOrder::GetCommand()
{
    long item = -1;
    CRef<CCmdComposite> cmd( new CCmdComposite("Correct Interval Order") );
    bool modified = false;
    if (m_ListCtrl && m_TopSeqEntry)
    {
        for ( ;; )
        {
            item = m_ListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if ( item == -1 )  break;
            string name = ToStdString(m_ListCtrl->GetItemText(item));
            bool subtypeAnyFlag = m_SubtypeAnyFlag[item];
            ApplyToCSeq_entry (m_TopSeqEntry, cmd, name, subtypeAnyFlag, modified);
        }      
    }

    if (!modified)
        cmd.Reset();
    return cmd;
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

void CCorrectIntervalOrder::ApplyToCSeq_entry (objects::CSeq_entry_Handle tse, CRef<CCmdComposite> composite, const string &name, bool subtypeAnyFlag, bool &modified)  // see SeqEntryConstrainedFeaturesCallback
{
    for (CFeat_CI feat (tse);  feat; ++feat)
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
                        } catch(const CException&) {} catch (const exception&) {}  
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
                    CRef<CSeq_feat> new_feat(new CSeq_feat());
                    new_feat->Assign(feat->GetOriginalFeature());
                    if (edit::CorrectIntervalOrder(new_feat->SetLocation()))
                    {
                        composite->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat->GetSeq_feat_Handle(), *new_feat)));
                        modified = true;                    
                    }
                }
            }
        }
    }
}



END_NCBI_SCOPE
