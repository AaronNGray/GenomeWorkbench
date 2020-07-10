/*  $Id: suppress_genes.cpp 42430 2019-02-22 13:19:16Z filippov $
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
#include <sstream>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <objects/general/Object_id.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <objtools/cleanup/cleanup.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <objtools/validator/utilities.hpp>
#include <gui/objutils/label.hpp>
#include <objtools/format/flat_file_config.hpp>
#include <objtools/format/flat_file_generator.hpp>
#include <objmgr/util/feature.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/packages/pkg_sequence_edit/suppress_genes.hpp>

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);



IMPLEMENT_DYNAMIC_CLASS( CSuppressGenes, CBulkCmdDlg )


BEGIN_EVENT_TABLE( CSuppressGenes, CBulkCmdDlg )

END_EVENT_TABLE()

CSuppressGenes::CSuppressGenes()
{
    Init();
}

CSuppressGenes::CSuppressGenes( wxWindow* parent, IWorkbench *wb, 
                                        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CSuppressGenes::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}


CSuppressGenes::~CSuppressGenes()
{
}


/*!
 * Member initialisation
 */

void CSuppressGenes::Init()
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
    m_ErrorMessage = "No feature found";
}




void CSuppressGenes::CreateControls()
{    

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer1);

    wxPanel *itemDialog1 = new wxPanel(this);
    itemBoxSizer1->Add(itemDialog1, 1, wxGROW, 0);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_ListCtrl = new wxListCtrl( itemDialog1, ID_SUPPRESS_GENES_LISTCTRL, wxDefaultPosition, wxSize(250, 200), wxLC_REPORT );
    itemBoxSizer2->Add(m_ListCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ListCtrl->InsertColumn(0,"    Feature",wxLIST_FORMAT_LEFT,237);
  
    for (unsigned int i=0; i<m_Descriptions.size(); i++)
        m_ListCtrl->InsertItem(i,ToWxString(m_Descriptions[i]));

        
    wxStaticBox* itemStaticBoxSizer17Static = new wxStaticBox(itemDialog1, wxID_ANY, wxEmptyString);
    wxStaticBoxSizer* itemStaticBoxSizer17 = new wxStaticBoxSizer(itemStaticBoxSizer17Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer17, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Where feature text"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer17->Add(itemStaticText4, 0, wxALIGN_TOP|wxALL, 5);

    m_StringConstraintPanel = new CStringConstraintPanel( itemDialog1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer17->Add(m_StringConstraintPanel, 0, wxALIGN_TOP|wxALL|wxFIXED_MINSIZE, 0);
    m_StringConstraintPanel->SetStringSelection(_("Contains"));


    COkCancelPanel *OkCancel = new COkCancelPanel( itemDialog1, ID_SUPPRESS_GENES_OKCANCEL, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
}


/*!
 * Should we show tooltips?
 */

bool CSuppressGenes::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSuppressGenes::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CSuppressGenes::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

bool CSuppressGenes::CreateSuppressingGeneXrefs(CSeq_feat& f)
{
    bool modified = false;
    bool found = false;
    
    if (f.IsSetXref()) 
    {
        EDIT_EACH_SEQFEATXREF_ON_SEQFEAT(xit, f)
        {
            if ((*xit)->IsSetData() && (*xit)->GetData().IsGene()) 
            {
                found = true;
                if (!(*xit)->GetData().GetGene().IsSuppressed())
                {
                    (*xit)->SetData().SetGene().Reset();
                    modified = true;
                }
                break;
            }
        }
    }
    if (!found)
    {
        CRef< CSeqFeatXref >  gene_ref(new CSeqFeatXref);
        gene_ref->SetData().SetGene();
        f.SetXref().push_back(gene_ref);
        modified = true;
    }
    return modified;
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

CRef<CCmdComposite> CSuppressGenes::GetCommand()
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Suppress Genes") );
    CRef<edit::CStringConstraint> constraint(m_StringConstraintPanel->GetStringConstraint());

    long item = -1;
    if (m_ListCtrl && m_TopSeqEntry)
    {
        for ( ;; )
        {
            item = m_ListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if ( item == -1 )  break;
            string name = ToStdString(m_ListCtrl->GetItemText(item));
            bool subtypeAnyFlag = m_SubtypeAnyFlag[item];
            for (CFeat_CI feat (m_TopSeqEntry); feat; ++feat)
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
                    if (constraint)
                    {
                        stringstream str;
                        CScope &scope = m_TopSeqEntry.GetScope();
                        s_Render_SeqFeat(str, feat->GetOriginalFeature(), scope);
                        go = constraint->DoesTextMatch(str.str());
                    }
                    
                    if (go)
                    {
                        const CSeq_feat& orig = feat->GetOriginalFeature();
                        CRef<CSeq_feat> new_feat(new CSeq_feat);
                        new_feat->Assign(orig);
                        bool modified = CreateSuppressingGeneXrefs(*new_feat);
                        
                        if (modified)
                        {
                            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat->GetSeq_feat_Handle(), *new_feat)));
                        }
                    }
                }
            }
        }
    }   
    return cmd;
}


END_NCBI_SCOPE
