/*  $Id: locationl_list_ctrl.cpp 42869 2019-04-23 12:24:11Z filippov $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>
#include <set>

#include <objects/seqloc/Na_strand.hpp>
#include <objects/general/Object_id.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/util/sequence.hpp>

#include <util/xregexp/regexp.hpp>

#include <gui/widgets/edit/locationl_list_ctrl.hpp>
#include <gui/widgets/edit/large_spin_ctrl.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/objutils/label.hpp>

#include <wx/hyperlink.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/scrolwin.h>
#include <wx/stattext.h>
#include <wx/radiobox.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CLocationListCtrl type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CLocationListCtrl, wxPanel )


/*!
 * CLocationListCtrl event table definition
 */

BEGIN_EVENT_TABLE( CLocationListCtrl, wxPanel )

    EVT_BUTTON( ID_BUTTON1, CLocationListCtrl::OnSortClick )
    EVT_HYPERLINK(wxID_ANY, CLocationListCtrl::OnDelete)
    EVT_COMMAND(wxID_ANY, wxEVT_LARGE_SPIN_CTRL_EVENT, CLocationListCtrl::OnSpinCtrl)
    EVT_CHOICE(wxID_ANY, CLocationListCtrl::OnStrandChange)
END_EVENT_TABLE()


/*!
 * CLocationListCtrl constructors
 */

CLocationListCtrl::CLocationListCtrl()
{
    Init();
}

CLocationListCtrl::CLocationListCtrl( wxWindow* parent, CRef<objects::CScope> scope, bool allow_nuc, bool allow_prot, bool is_aa,
                                      wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_AllowNuc(allow_nuc), m_AllowProt(allow_prot), m_Scope(scope), m_is_aa(is_aa)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CLocationListCtrl creator
 */

bool CLocationListCtrl::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CLocationListCtrl creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CLocationListCtrl creation
    return true;
}


/*!
 * CLocationListCtrl destructor
 */

CLocationListCtrl::~CLocationListCtrl()
{
}


/*!
 * Member initialisation
 */

void CLocationListCtrl::Init()
{
////@begin CLocationListCtrl member initialisation
    m_LocType = 0;
    m_ScrolledWindow = NULL;
    m_Partial5 = NULL;
    m_Partial3 = NULL;
////@end CLocationListCtrl member initialisation
    m_Sizer = NULL;
    m_LastFrom = NULL;
    m_LastTo = NULL;
    m_LastStrand = NULL;
    m_LastId = NULL;
    m_LastIdSelection = wxNOT_FOUND;
    m_LastStrandSelection = wxNOT_FOUND;
    m_CtrlColNum = 5;
    if (m_is_aa)
        m_CtrlColNum = 4;
}


/*!
 * Control creation for CLocationListCtrl
 */

void CLocationListCtrl::CreateControls()
{    
////@begin CLocationListCtrl content construction
    CLocationListCtrl* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer14Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Partialness"));
    wxStaticBoxSizer* itemStaticBoxSizer14 = new wxStaticBoxSizer(itemStaticBoxSizer14Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer14, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    
    wxString start_label = _("5'");
    if (m_is_aa)
        start_label = _("NH2");
    m_Partial5 = new wxCheckBox( itemPanel1, ID_CHECKBOX1, start_label, wxDefaultPosition, wxDefaultSize, 0 );
    m_Partial5->SetValue(false);
    itemStaticBoxSizer14->Add(m_Partial5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxString stop_label = _("3'");
    if (m_is_aa)
        stop_label = _("CO2H");
    m_Partial3 = new wxCheckBox( itemPanel1, ID_CHECKBOX2, stop_label, wxDefaultPosition, wxDefaultSize, 0 );
    m_Partial3->SetValue(false);
    itemStaticBoxSizer14->Add(m_Partial3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);


    wxPanel* itemPanel3 = new wxPanel( itemPanel1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(itemPanel3, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemPanel3->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer5, 0, wxALIGN_LEFT|wxALL, 0);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel3, wxID_STATIC, _("From"), wxDefaultPosition, wxSize(itemPanel3->ConvertDialogToPixels(wxSize(35, -1)).x, -1), wxALIGN_CENTRE );
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel3, wxID_STATIC, _("To"), wxDefaultPosition, wxSize(itemPanel3->ConvertDialogToPixels(wxSize(35, -1)).x, -1), wxALIGN_CENTRE );
    itemBoxSizer5->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    if (!m_is_aa)
    {
        wxStaticText* itemStaticText8 = new wxStaticText( itemPanel3, wxID_STATIC, _("Strand"), wxDefaultPosition, wxSize(itemPanel3->ConvertDialogToPixels(wxSize(45, -1)).x, -1), wxALIGN_CENTRE );
        itemBoxSizer5->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);
    }

    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel3, wxID_STATIC, _("SeqID"), wxDefaultPosition, wxSize(itemPanel3->ConvertDialogToPixels(wxSize(65, -1)).x, -1), wxALIGN_CENTRE );
    itemBoxSizer5->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    itemBoxSizer5->Add(82, 8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticLine* itemStaticLine11 = new wxStaticLine( itemPanel3, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer4->Add(itemStaticLine11, 0, wxGROW|wxALL, 1);

    m_ScrolledWindow = new wxScrolledWindow( itemPanel3, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxSize(-1, 100), wxTAB_TRAVERSAL );
    itemBoxSizer4->Add(m_ScrolledWindow, 1, wxGROW|wxTOP|wxBOTTOM, 0);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);
   
    wxArrayString itemRadioBox17Strings;
    itemRadioBox17Strings.Add(_("&join()"));
    itemRadioBox17Strings.Add(_("&order()"));
    wxRadioBox* itemRadioBox17 = new wxRadioBox( itemPanel1, ID_RADIOBOX6, _("Save As"), wxDefaultPosition, wxDefaultSize, itemRadioBox17Strings, 1, wxRA_SPECIFY_ROWS );
    itemRadioBox17->SetSelection(0);
    if (CLocationListCtrl::ShowToolTips())
        itemRadioBox17->SetToolTip(_("If each interval is separate, and should not be joined with the others to describe the feature, choose order (for example, when annotating multiple primer binding sites)."));
    itemBoxSizer13->Add(itemRadioBox17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxButton* itemButton18 = new wxButton( itemPanel1, ID_BUTTON1, _("Sort"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemButton18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemRadioBox17->SetValidator( wxGenericValidator(& m_LocType) );
////@end CLocationListCtrl content construction

    m_Sizer = new wxFlexGridSizer(0, m_CtrlColNum, 0, 0);
    m_ScrolledWindow->SetSizer(m_Sizer);
    m_ScrolledWindow->FitInside();

    //m_AddIntHyperlink->SetVisitedColour(m_AddIntHyperlink->GetNormalColour());
    m_ScrolledWindow->SetScrollRate(0, 5);
}

void CLocationListCtrl::AddRow(int from, int to, int strand, const  wxString& seqID, const bool fuzz)
{
    int comboIndex = 0;
    switch(strand) {
    default :
        break;
    case eNa_strand_plus :
        comboIndex = 0;
        break;
    case eNa_strand_minus :
        comboIndex = 1;
        break;
    case eNa_strand_both :
        comboIndex = 2;
        break;
    case eNa_strand_other :
        comboIndex = 3;
        break;
    }   



    SIntData intData;
    intData.m_From   = from;
    intData.m_To     = to;
    intData.m_Strand = comboIndex;
    intData.m_SeqId  = seqID;
    intData.m_Empty  = false;
    intData.m_Fuzz = fuzz;
    m_Data.push_back(intData);
}

void CLocationListCtrl::SetPartials(bool partial5, bool partial3)
{
    m_Partial5->SetValue(partial5);    
    m_Partial3->SetValue(partial3);
}

void CLocationListCtrl::AddEmptyRow()
{
    SIntData intData;
    intData.m_Empty = true;
    m_Data.push_back(intData);
}

void CLocationListCtrl::ClearRows()
{
    m_Data.clear();
    m_Sizer->Clear(true);
}

void s_FinishLoc(CSeq_loc& loc, bool partial5, bool partial3)
{
    if (!loc.IsNull()) {
        loc.SetPartialStart(partial5, eExtreme_Biological);
        loc.SetPartialStop(partial3, eExtreme_Biological);
    }    
}


CRef<CSeq_loc> CLocationListCtrl::GetSeq_loc()
{
    CRef<CSeq_loc> loc(new CSeq_loc());
    if (m_Data.empty()) {
        loc->CSeq_loc_Base::SetNull();
        return loc;
    }

    bool partial5 = m_Partial5->GetValue();
    bool partial3 = m_Partial3->GetValue();

    if (m_Data.size() == 1 || (m_Data.size() == 2 && m_Data.back().m_Empty)) {
        if (m_Data[0].m_Empty) {
            loc->CSeq_loc_Base::SetNull();
        } else {
            loc.Reset();
            loc = x_GetInterval(0, false);
            s_FinishLoc(*loc, partial5, partial3);
        }
        return loc;
    }

    if (m_LocType == 0) {
        CRef<CPacked_seqint> packed(new CPacked_seqint());
        for (size_t i = 0; i < m_Data.size(); ++i) {
            CRef<CSeq_loc> seq_int = x_GetInterval(i, true);
            if (seq_int && seq_int->IsInt()) {
                CRef<CSeq_interval> new_int(new CSeq_interval);
                new_int->Assign(seq_int->GetInt());
                packed->Set().push_back(new_int);
            }
        }
        if (packed->Set().size() == 1) {
            loc->SetInt().Assign(*(packed->Set().front()));
        } else if (packed->Set().size() == 0) {
            loc->SetNull();
        } else {
            loc->CSeq_loc_Base::SetPacked_int(*packed);
        }
    }
    else {
        CRef<CSeq_loc_mix> mix(new CSeq_loc_mix());

        for (size_t i = 0; i < m_Data.size(); ++i) {
            CRef<CSeq_loc> seq_int = x_GetInterval(i, false);
            if (seq_int) {                
                mix->Set().push_back(seq_int);
                CRef<CSeq_loc> null_loc(new CSeq_loc());
                null_loc->CSeq_loc_Base::SetNull();
                mix->Set().push_back(null_loc);
            }
        }
        if (mix->Set().back()->IsNull()) {
            mix->Set().pop_back();
        }
        if (mix->Set().size() == 0) {
            loc->SetNull();
        } else if (mix->Set().size() == 1) {
            loc->SetInt().Assign(*(mix->Set().front()));
        } else {
            loc->CSeq_loc_Base::SetMix(*mix);
        }
    }
    s_FinishLoc(*loc, partial5, partial3);
    return loc;
}

ENa_strand CLocationListCtrl::GetStrandFromForm(size_t index)
{
    ENa_strand strand = x_GetStrandFromForm(index);
    if (strand == eNa_strand_unknown)
    {
        for (size_t i = 0; i < m_Data.size(); i++)
        {
            ENa_strand tmp = x_GetStrandFromForm(i);
            if (tmp != eNa_strand_unknown)
            {
                strand = tmp;
                break;
            }
        }
    }
    return strand;
}

ENa_strand CLocationListCtrl::x_GetStrandFromForm(size_t index)
{
    ENa_strand strand = eNa_strand_unknown;
    switch(m_Data[index].m_Strand) {
    case 0 :
        strand = eNa_strand_plus;
        break;
    case 1 :
        strand = eNa_strand_minus;
        break;
    case 2 :
        strand = eNa_strand_both;
        break;
    case 3 :
        strand = eNa_strand_other;
        break;
    default:
        strand = eNa_strand_unknown; 
        break;
    }

    return strand;
}

CRef<CSeq_id> CLocationListCtrl::GetIdFromForm(size_t index)
{
    CRef<CSeq_id> seq_id(new CSeq_id());
    
    string id_str = ToStdString(m_Data[index].m_SeqId);
    if (id_str.empty())
    {
        for (size_t i = 0; i < m_Data.size(); i++)
        {
            if (!(m_Data[i].m_SeqId.IsEmpty()))
            {
                id_str = ToStdString(m_Data[i].m_SeqId);
                break;
            }
        }
    }
    
    if (id_str.empty())
    {
        for (wxArrayString::iterator it = m_SeqIds.begin(); it != m_SeqIds.end(); ++it)
        {
            if (!it->IsEmpty())
            {
                id_str = it->ToStdString();
                break;
            }
        }
    }  

    if (m_str_to_id.find(id_str) != m_str_to_id.end())
    {
        seq_id->Assign(*m_str_to_id[id_str]);
        return seq_id;
    }

    try {
        seq_id->Set(id_str);
    } catch (CSeqIdException&) {
        // make a local ID
        seq_id->SetLocal().SetStr(id_str);
    }   
    if (seq_id->IsGi()) {
        // does a sequence with this GI exist in the scope?
        if (!m_Scope || !m_Scope->GetBioseqHandle(*seq_id)) {
            seq_id->SetLocal().SetGi(seq_id->GetGi());
        }
    }
    else if (m_Scope) {
        try
        {
            TGi gi = sequence::GetGiForId(*seq_id, *m_Scope);
            if (gi > ZERO_GI) {
                seq_id->SetGi(gi);
            }
        }
        catch(CLoaderException&) {}
    }

    return seq_id;
}

CRef<CSeq_loc> CLocationListCtrl::x_GetInterval(size_t index, bool int_only) 
{
    if (m_Data[index].m_Empty) {
        return CRef<CSeq_loc>();
    }
    CRef<CSeq_loc> seq_loc(new CSeq_loc);
    if (m_Data[index].m_From == m_Data[index].m_To && !int_only)
    {
        CRef<CSeq_point> seq_pnt(new CSeq_point);
        seq_pnt->SetPoint(m_Data[index].m_From - 1);
        if (!m_is_aa)
        {
            ENa_strand strand = GetStrandFromForm(index);    
            seq_pnt->SetStrand(strand);
        }
        CRef<CSeq_id> seq_id = GetIdFromForm(index);
        seq_pnt->SetId(*seq_id);
        seq_loc->SetPnt(*seq_pnt);
    }
    else if (m_Data[index].m_Fuzz && abs(m_Data[index].m_To - m_Data[index].m_From) <= 1  && !int_only)
    {
        CRef<CSeq_point> seq_pnt(new CSeq_point);
        seq_pnt->SetPoint(m_Data[index].m_From - 1);
        if (!m_is_aa)
        {
            ENa_strand strand = GetStrandFromForm(index);    
            seq_pnt->SetStrand(strand);
        }
        CRef<CSeq_id> seq_id = GetIdFromForm(index);
        seq_pnt->SetId(*seq_id);
        seq_pnt->SetFuzz().SetLim(CInt_fuzz::eLim_tr);
        seq_loc->SetPnt(*seq_pnt);
    }
    else
    {
        CRef<CSeq_interval> seq_int(new CSeq_interval());
        seq_int->SetFrom(m_Data[index].m_From - 1);
        seq_int->SetTo(m_Data[index].m_To - 1);
        if (!m_is_aa)
        {
            ENa_strand strand = GetStrandFromForm(index);    
            seq_int->SetStrand(strand);
        }
        CRef<CSeq_id> seq_id = GetIdFromForm(index);
        seq_int->SetId(*seq_id);
        seq_loc->SetInt(*seq_int);
    }
    return seq_loc;
}

void CLocationListCtrl::DeleteRow(wxSizerItemList::iterator row, wxSizerItemList& itemList)
{
    wxSizerItemList::iterator node = row;
    for (int i = 0; node != itemList.end() && i < m_CtrlColNum; ++i) {
        wxSizer* sizer = (**node).GetSizer();
        
        if (m_LastFrom && 
            ((sizer && sizer->GetItem(m_LastFrom)) 
             || (**node).GetWindow() == m_LastFrom) )
        {
            m_LastFrom = NULL;
            m_LastTo = NULL;
            m_LastStrand = NULL;
            m_LastId = NULL;
        }
        (**node).DeleteWindows();
        node = itemList.erase(node);
    }
}

wxSizerItemList::iterator CLocationListCtrl::x_FindRow(wxWindow* wnd, wxSizerItemList& itemList)
{
    if (NULL == wnd)
        return itemList.end();

    wxSizerItemList::iterator row, it = itemList.begin();

    for(int index = 0; it != itemList.end(); ++it, --index) {
        if (index == 0) {
            row = it;
            index = m_CtrlColNum;
        }

        wxSizer* sizer = (**it).GetSizer();
        if (sizer && sizer->GetItem(wnd))
            return row;
        else if ((**it).GetWindow() == wnd)
            return row;
    }

    return it;
}

wxSizerItemList::iterator CLocationListCtrl::x_NextRow(wxSizerItemList::iterator row, wxSizerItemList& itemList)
{
    wxSizerItemList::iterator it = row;
    for (int i = 0; i < m_CtrlColNum; ++i, ++it) {
        if (it == itemList.end())
            return itemList.end();
    }
    return it;
}

wxSizerItemList::iterator CLocationListCtrl::x_PrevRow(wxSizerItemList::iterator row, wxSizerItemList& itemList)
{
    wxSizerItemList::iterator it = row;
    for (int i = 0; i < m_CtrlColNum; ++i, --it) {
        if (it == itemList.begin())
            return itemList.end();
    }
    return it;
}

static void s_SwapRows(wxSizerItemList::iterator row1,
                       wxSizerItemList::iterator row2,
                       wxSizerItemList& itemList)
{
    for (int i = 0; i < 4; ++i) {
        if (row1 == itemList.end() || row2 == itemList.end())
            break;
        swap(*row1++, *row2++);
    }
}

void CLocationListCtrl::x_GetRowData(wxSizerItemList::iterator row, wxSizerItemList& itemList,
                                   int& from, int& to, int& strand, wxString& seqID, bool &fuzz)
{
    wxSizerItemList::iterator node = row;

    if (node == itemList.end())return;

    fuzz = false;

    wxTextCtrl* text = (wxTextCtrl*)(**node).GetWindow();
    fuzz |= NStr::StartsWith(text->GetValue().ToStdString(),"^") || NStr::EndsWith(text->GetValue().ToStdString(),"^");
    from = wxAtoi(text->GetValue());
    if (++node == itemList.end()) return;

    text = (wxTextCtrl*)(**node).GetWindow();
    fuzz |= NStr::StartsWith(text->GetValue().ToStdString(),"^") || NStr::EndsWith(text->GetValue().ToStdString(),"^");
    to = wxAtoi(text->GetValue());
    if (++node == itemList.end()) return;

    strand = 0;
    if (!m_is_aa)
    {
        wxChoice* strand_ctrl = dynamic_cast<wxChoice*>((**node).GetWindow());
        strand = strand_ctrl->GetSelection();
        if (++node == itemList.end()) return;
    }

    if (strand == 1) // eNa_strand_minus
        swap(from,to);
    wxComboBox *combo = (wxComboBox*)(**node).GetWindow();
    seqID = combo->GetValue();
}

void CLocationListCtrl::OnStrandChange(wxCommandEvent& event)
{
    wxSizerItemList& itemList = m_Sizer->GetChildren();
    wxSizerItemList::iterator row = x_FindRow((wxWindow*)event.GetEventObject(), itemList);
    if (row == itemList.end())
        return;
    wxSizerItemList::iterator node = row;

    wxTextCtrl* from_ctrl = (wxTextCtrl*)(**node).GetWindow();
    int from = wxAtoi(from_ctrl->GetValue());
    if (++node == itemList.end()) return;

    wxTextCtrl* to_ctrl = (wxTextCtrl*)(**node).GetWindow();
    int to = wxAtoi(to_ctrl->GetValue());
    if (++node == itemList.end()) return;

    int strand = 0;

    if (!m_is_aa)
    {
        wxChoice* strand_ctrl = dynamic_cast<wxChoice*>((**node).GetWindow());
        strand = strand_ctrl->GetSelection();
    }

    if ((strand == 1 && from < to) || (strand != 1 && from > to) ) // eNa_strand_minus
    {
        from_ctrl->SetValue(wxEmptyString);
        to_ctrl->SetValue(wxEmptyString);
        *from_ctrl << to;
        *to_ctrl << from;
    }
}

void CLocationListCtrl::OnDelete (wxHyperlinkEvent& event)
{
    wxSizerItemList& itemList = m_Sizer->GetChildren();
    wxSizerItemList::iterator row = x_FindRow((wxWindow*)event.GetEventObject(), itemList);
    DeleteRow(row, itemList);
    if (m_Sizer->GetChildren().empty()) {
        m_LastFrom = NULL;
        m_LastTo = NULL;
        m_LastStrand = NULL;
        m_LastId = NULL;
        x_AddEmptyRow();
    }
    m_ScrolledWindow->FitInside();
}

void CLocationListCtrl::OnSpinCtrl ( wxCommandEvent& evt )
{
    wxWindow* spnCtrl = (wxWindow*)evt.GetEventObject();

    wxSizerItemList& itemList = m_Sizer->GetChildren();
    wxSizerItemList::iterator row = x_FindRow(spnCtrl, itemList);
    if (row == itemList.end())
        return;

    wxSizerItemList::iterator row2;

    switch (evt.GetId()) {
        case CLargeSpinControl::kBtnUp:
            row2 = x_PrevRow(row, itemList);
            s_SwapRows(row, row2, itemList);
            m_Sizer->Layout();
            m_ScrolledWindow->Refresh();
            break;
        case CLargeSpinControl::kBtnDn:
            row2 = x_NextRow(row, itemList);
            s_SwapRows(row, row2, itemList);
            m_Sizer->Layout();
            m_ScrolledWindow->Refresh();
            break;
        case 2 + CLargeSpinControl::kBtnUp:
            {{
                size_t rowPos = itemList.IndexOf(*row);

                int from = 1, to = 1, strand = 0;
                wxString seqID;
                bool fuzz = false;
                x_GetRowData(row, itemList, from, to, strand, seqID, fuzz);
                if (from > 1)
                    --from;
#if 1
                x_AddEmptyRow(rowPos);
#else
                x_AddRow(from, from, strand, seqID, fuzz, rowPos);
#endif
                m_Sizer->Layout();
                m_ScrolledWindow->FitInside();
                m_ScrolledWindow->Refresh();
            }}
            break;
        case 2 + CLargeSpinControl::kBtnDn:
            {{
                int from = 1, to = 1, strand = 0;
                wxString seqID;
                bool fuzz = false;
                x_GetRowData(row, itemList, from, to, strand, seqID, fuzz);

                row = x_NextRow(row, itemList);
                size_t rowPos = (row == itemList.end()) ? (size_t)-1 : itemList.IndexOf(*row);
                ++to;
#if 1
                x_AddEmptyRow(rowPos);
#else
                x_AddRow(to, to, strand, seqID, fuzz, rowPos);
#endif
                m_Sizer->Layout();
                m_ScrolledWindow->FitInside();
                m_ScrolledWindow->Refresh();
            }}
            break;
    }
}

void CLocationListCtrl::x_AddRow(int from, int to, int strand, const wxString& seqID, const bool fuzz, size_t rowPos)
{
    if (strand == 1) //eNa_strand_minus
        swap(from,to);
    wxTextCtrl *textFrom;
    textFrom = new wxTextCtrl(m_ScrolledWindow, wxID_ANY, wxT(""), wxDefaultPosition,
                          wxSize(m_ScrolledWindow->ConvertDialogToPixels(wxSize(35, -1)).x, -1),
                          wxTE_RIGHT);
    *textFrom << from;
    if (fuzz)
        *textFrom << "^";
    if (rowPos == (size_t)-1)
        m_Sizer->Add(textFrom, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    else 
        m_Sizer->Insert(rowPos++, textFrom, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxTextCtrl *text;
    text = new wxTextCtrl(m_ScrolledWindow, wxID_ANY, wxT(""), wxDefaultPosition,
                          wxSize(m_ScrolledWindow->ConvertDialogToPixels(wxSize(35, -1)).x, -1),
                          wxTE_RIGHT);
    *text << to;
    if (rowPos == (size_t)-1)
        m_Sizer->Add(text, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    else
        m_Sizer->Insert(rowPos++, text, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    if (!m_is_aa)
    {
        wxArrayString itemComboBoxStrings;
        itemComboBoxStrings.Add(wxT("Plus"));
        itemComboBoxStrings.Add(wxT("Minus"));
        itemComboBoxStrings.Add(wxT("Both"));
        itemComboBoxStrings.Add(wxT("Other"));
        
        wxChoice* strand_ctrl = new CNoTabChoice(m_ScrolledWindow, wxID_ANY,
                                                 wxDefaultPosition,
                                                 wxSize(m_ScrolledWindow->ConvertDialogToPixels(wxSize(45, -1)).x, -1),
                                                 itemComboBoxStrings);
        strand_ctrl->SetSelection(strand);

        if (rowPos == (size_t)-1)
            m_Sizer->Add(strand_ctrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
        else
            m_Sizer->Insert(rowPos++, strand_ctrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    }

    CNoTabComboBox* combo = new CNoTabComboBox(m_ScrolledWindow, wxID_ANY, seqID,
                           wxDefaultPosition,
                           wxSize(m_ScrolledWindow->ConvertDialogToPixels(wxSize(65, -1)).x, -1),
                           m_SeqIds);
    if (rowPos == (size_t)-1)
        m_Sizer->Add(combo, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    else
        m_Sizer->Insert(rowPos++, combo, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxBoxSizer* controlsSizer = new wxBoxSizer(wxHORIZONTAL);
    if (rowPos == (size_t)-1)
        m_Sizer->Add(controlsSizer);
    else
        m_Sizer->Insert(rowPos++, controlsSizer);      

    CLargeNoTabSpinControl* spinCtrl1 = new CLargeNoTabSpinControl(m_ScrolledWindow, wxID_ANY, wxDefaultPosition);
    spinCtrl1->SetToolTip(_("Use insert controls to insert blank location intervals"));
    spinCtrl1->UseImageSet(1);
    controlsSizer->Add(spinCtrl1, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    CLargeNoTabSpinControl* spinCtrl2 = new CLargeNoTabSpinControl(m_ScrolledWindow, wxID_ANY, wxDefaultPosition);
    spinCtrl2->SetToolTip(_("Use arrow controls to reorder location intervals"));
    controlsSizer->Add(spinCtrl2, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    CNoTabHyperlinkCtrl* itemHyperLink = new CNoTabHyperlinkCtrl(m_ScrolledWindow, wxID_ANY, wxT("Delete"), wxT(""));
    itemHyperLink->SetVisitedColour(itemHyperLink->GetNormalColour());
    controlsSizer->Add(itemHyperLink, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    textFrom->SetFocus();
    textFrom->SetSelection(-1, -1);
    m_LastIdSelection = combo->GetSelection();
    m_LastStrandSelection = strand;
    m_LastIdValue = combo->GetValue();
}

void CLocationListCtrl::x_AddEmptyRow(size_t rowPos)
{
    if (m_LastFrom)
        m_LastFrom->Unbind(wxEVT_TEXT_ENTER, &CLocationListCtrl::OnNewText, this);
    if (m_LastTo)
        m_LastTo->Unbind(wxEVT_TEXT_ENTER, &CLocationListCtrl::OnNewText, this);
   
   
    m_LastFrom = new wxTextCtrl(m_ScrolledWindow, wxID_ANY, wxT(""), wxDefaultPosition,
                                wxSize(m_ScrolledWindow->ConvertDialogToPixels(wxSize(35, -1)).x, -1),
                                wxTE_RIGHT|wxTE_PROCESS_ENTER);
    if (rowPos == (size_t)-1)
        m_Sizer->Add(m_LastFrom, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    else 
        m_Sizer->Insert(rowPos++, m_LastFrom, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  
    m_LastTo = new wxTextCtrl(m_ScrolledWindow, wxID_ANY, wxT(""), wxDefaultPosition,
                              wxSize(m_ScrolledWindow->ConvertDialogToPixels(wxSize(35, -1)).x, -1),
                              wxTE_RIGHT|wxTE_PROCESS_ENTER);
    if (rowPos == (size_t)-1)
        m_Sizer->Add(m_LastTo, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    else
        m_Sizer->Insert(rowPos++, m_LastTo, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxArrayString itemComboBoxStrings;
    itemComboBoxStrings.Add(wxT("Plus"));
    itemComboBoxStrings.Add(wxT("Minus"));
    itemComboBoxStrings.Add(wxT("Both"));
    itemComboBoxStrings.Add(wxT("Other"));
    if (!m_is_aa)
    {
        m_LastStrand = new CNoTabChoice(m_ScrolledWindow, wxID_ANY,
                                        wxDefaultPosition,
                                        wxSize(m_ScrolledWindow->ConvertDialogToPixels(wxSize(45, -1)).x, -1),
                                        itemComboBoxStrings);
        m_LastStrand->SetSelection(m_LastStrandSelection);

        if (rowPos == (size_t)-1)
            m_Sizer->Add(m_LastStrand, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
        else
            m_Sizer->Insert(rowPos++, m_LastStrand, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    }

    m_LastId = new CNoTabComboBox(m_ScrolledWindow, wxID_ANY, wxT(""),
                              wxDefaultPosition,
                              wxSize(m_ScrolledWindow->ConvertDialogToPixels(wxSize(65, -1)).x, -1),
                              m_SeqIds);
    m_LastId->SetValue(m_LastIdValue);
    if (rowPos == (size_t)-1)
        m_Sizer->Add(m_LastId, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    else
        m_Sizer->Insert(rowPos++, m_LastId, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxBoxSizer* controlsSizer = new wxBoxSizer(wxHORIZONTAL);
    if (rowPos == (size_t)-1)
        m_Sizer->Add(controlsSizer);
    else
        m_Sizer->Insert(rowPos++, controlsSizer);       

    CLargeSpinControl* spinCtrl1 = new CLargeNoTabSpinControl(m_ScrolledWindow, wxID_ANY, wxDefaultPosition);
    spinCtrl1->UseImageSet(1);
    controlsSizer->Add(spinCtrl1, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    CLargeSpinControl* spinCtrl2 = new CLargeNoTabSpinControl(m_ScrolledWindow, wxID_ANY, wxDefaultPosition);
    controlsSizer->Add(spinCtrl2, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    wxHyperlinkCtrl* itemHyperLink = new CNoTabHyperlinkCtrl(m_ScrolledWindow, wxID_ANY, wxT("Delete"), wxT(""));
    itemHyperLink->SetVisitedColour(itemHyperLink->GetNormalColour());
    controlsSizer->Add(itemHyperLink, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    m_LastFrom->SetFocus();
    m_LastFrom->SetSelection(-1, -1);

    m_LastFrom->Bind(wxEVT_TEXT_ENTER, &CLocationListCtrl::OnNewText, this);
    m_LastTo->Bind(wxEVT_TEXT_ENTER, &CLocationListCtrl::OnNewText, this);   
}

void CLocationListCtrl::OnSortClick( wxCommandEvent& WXUNUSED(event) )
{
    vector<SIntData> intVec;

    SIntData intData;
    wxSizerItemList& children = m_Sizer->GetChildren();
    wxSizerItemList::iterator node = children.begin();

    for (;  node != children.end(); ) {
        wxTextCtrl* text = (wxTextCtrl*)(**node).GetWindow();
        if (++node == children.end()) break;
        wxString from = text->GetValue();

        text = (wxTextCtrl*)(**node).GetWindow();
        if (++node == children.end()) break;
        wxString to = text->GetValue();

        wxChoice* strand_combo = NULL;
        if (!m_is_aa)
        {
            strand_combo = dynamic_cast<wxChoice*>((**node).GetWindow());
            if (++node == children.end()) break;
        }

        wxComboBox* id_combo = (wxComboBox*)(**node).GetWindow();
        if (++node == children.end()) break;

        ++node;  // Controls sizer

        if (from.IsEmpty() && to.IsEmpty()) {
            intData.m_Empty = true;
        } else {
            intData.m_Fuzz = NStr::StartsWith(from.ToStdString(),"^") || NStr::EndsWith(from.ToStdString(),"^") || NStr::StartsWith(to.ToStdString(),"^") || NStr::EndsWith(to.ToStdString(),"^"); 
            intData.m_From = wxAtoi(from);
            intData.m_To = wxAtoi(to);
            intData.m_Empty = false;
        }

        intData.m_Strand = 0;
        if (strand_combo)
            intData.m_Strand = strand_combo->GetSelection();
        intData.m_SeqId = id_combo->GetValue();
        if (intData.m_Strand == 1) // eNa_strand_minus
            swap(intData.m_From, intData.m_To);

        intVec.push_back(intData);
    }

    if (intVec.empty())
        return;

    sort(intVec.begin(), intVec.end());

    for (node = children.begin();  node != children.end();  ++node)
    {

        (**node).DeleteWindows();
    }

    children.clear();

    vector<SIntData>::const_iterator it;
    int num_added = 0;
    for (it = intVec.begin(); it != intVec.end(); ++it) {
        if (!it->m_Empty) {
            x_AddRow(it->m_From, it->m_To, it->m_Strand, it->m_SeqId, it->m_Fuzz);
            num_added++;
        }
    }
    m_LastFrom = NULL;
    m_LastTo = NULL;
    m_LastStrand = NULL;
    m_LastId = NULL;
    x_AddEmptyRow();
    

    m_ScrolledWindow->FitInside();
}


void CLocationListCtrl::PopulateIdListFromScope()
{
    vector<SIntData>::const_iterator it;
    set<wxString> idsSet;
    for (it = m_Data.begin(); it != m_Data.end(); ++it)
        idsSet.insert(it->m_SeqId);

    m_SeqIds.Empty();
    m_str_to_id.clear();
    if (m_Scope && (m_AllowNuc || m_AllowProt)) {
        CScope::TTSE_Handles tses;
        m_Scope->GetAllTSEs(tses, CScope::eAllTSEs);
        if (m_AllowNuc && m_AllowProt) {
            ITERATE (CScope::TTSE_Handles, handle, tses) {
                for (CBioseq_CI bioseq_it(*handle);  bioseq_it;  ++bioseq_it) {
                    try {
                        string str;
                        CLabel::GetLabel(*(bioseq_it->GetSeqId()), &str, CLabel::eUserTypeAndContent, m_Scope);
                        m_str_to_id[str] = bioseq_it->GetSeqId();
                        m_SeqIds.Add(ToWxString(str));
                    }
                    catch (const CException&) {}
                }
            }
        } else if (m_AllowNuc) {
            ITERATE (CScope::TTSE_Handles, handle, tses) {
                for (CBioseq_CI bioseq_it(*handle, CSeq_inst::eMol_na);  bioseq_it;  ++bioseq_it) {
                    string str;
                    CLabel::GetLabel(*(bioseq_it->GetSeqId()), &str, CLabel::eUserTypeAndContent, m_Scope);
                    m_str_to_id[str] = bioseq_it->GetSeqId();
                    m_SeqIds.Add(ToWxString (str));
                }
            }
        } else {
            ITERATE (CScope::TTSE_Handles, handle, tses) {
                for (CBioseq_CI bioseq_it(*handle, CSeq_inst::eMol_aa);  bioseq_it;  ++bioseq_it) {
                    string str;
                    CLabel::GetLabel(*(bioseq_it->GetSeqId()), &str, CLabel::eUserTypeAndContent, m_Scope);
                    m_str_to_id[str] = bioseq_it->GetSeqId();
                    m_SeqIds.Add(ToWxString (str));
                }
            }
        }
    } else {
        set<wxString>::const_iterator it2;
        for (it2 = idsSet.begin(); it2 != idsSet.end(); ++it2)
            m_SeqIds.Add(*it2);
    }
}


bool CLocationListCtrl::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    wxSizerItemList& children = m_Sizer->GetChildren();
    wxSizerItemList::iterator node = children.begin();
    for (;  node != children.end();  ++node)
    {
        (**node).DeleteWindows();
    }
    children.clear();
    m_LastFrom = NULL;
    m_LastTo = NULL;
    m_LastStrand = NULL;
    m_LastId = NULL;

	if (m_SeqIds.IsEmpty()) {
		PopulateIdListFromScope();
	}
    bool last_empty = false;
    vector<SIntData>::const_iterator it;
    for (it = m_Data.begin(); it != m_Data.end(); ++it) {
        if (it->m_Empty) {
            x_AddEmptyRow ();
            last_empty = true;
        } else {
            x_AddRow(it->m_From, it->m_To, it->m_Strand, it->m_SeqId, it->m_Fuzz);
            last_empty = false;
        }
    }

    if (!last_empty)
    {
        x_AddEmptyRow ();
    }
    m_ScrolledWindow->FitInside();

    return true;
}

bool CLocationListCtrl::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    m_Data.clear();

    SIntData intData;
    wxSizerItemList& children = m_Sizer->GetChildren();
    wxSizerItemList::iterator node = children.begin();

    for (;  node != children.end(); ) {
        wxTextCtrl* text = (wxTextCtrl*)(**node).GetWindow();
        if (++node == children.end()) break;
        wxString from = text->GetValue();

        text = (wxTextCtrl*)(**node).GetWindow();
        if (++node == children.end()) break;
        wxString to = text->GetValue();

        wxChoice* strand_combo = NULL;
        if (!m_is_aa)
        {
            strand_combo = dynamic_cast<wxChoice*>((**node).GetWindow());
            if (++node == children.end()) break;
        }

        wxComboBox* id_combo = (wxComboBox*)(**node).GetWindow();
        if (++node == children.end()) break;

        ++node;  // "Delete" Hyperlink

        if (from.IsEmpty() && to.IsEmpty()) {
            intData.m_Empty = true;
        } else {
            if (from.IsEmpty()) {
                from = to;
            } else if (to.IsEmpty()) {
                to = from;
            }

            string from_str = from.ToStdString();
            string to_str = to.ToStdString();
            intData.m_Fuzz = NStr::StartsWith(from_str,"^") || NStr::EndsWith(from_str,"^") || NStr::StartsWith(to_str,"^") || NStr::EndsWith(to_str,"^");
            if (intData.m_Fuzz) {
                NStr::ReplaceInPlace(from_str, "^", "");
                NStr::ReplaceInPlace(to_str, "^", "");
            }
            intData.m_From = NStr::StringToInt(from_str);
            intData.m_To = NStr::StringToInt(to_str);
            intData.m_Strand = 0;
            if (strand_combo)
                intData.m_Strand = strand_combo->GetSelection();
            intData.m_SeqId = id_combo->GetValue();
            intData.m_Empty = false;
            if (intData.m_Strand == 1) // eNa_strand_minus
                swap(intData.m_From, intData.m_To);
        }
        m_Data.push_back(intData);
    }

    return true;
}

void CLocationListCtrl::OnNewText(wxCommandEvent& event)
{
    if (m_LastFrom && m_LastTo && !m_LastFrom->GetValue().IsEmpty() && !m_LastTo->GetValue().IsEmpty())
    {
        if (m_LastId->GetValue().IsEmpty())
        {
            if (m_LastIdSelection != wxNOT_FOUND)
                m_LastId->SetSelection(m_LastIdSelection);
            else
                m_LastId->ChangeValue(m_LastIdValue);
        }
        else
        {
            m_LastIdSelection = m_LastId->GetSelection();
            m_LastIdValue = m_LastId->GetValue();
        }
        if (m_LastStrand)
        {
            if (m_LastStrand->GetSelection() == wxNOT_FOUND)
            {
                m_LastStrand->SetSelection(m_LastStrandSelection);
            }
            else
            {
                m_LastStrandSelection = m_LastStrand->GetSelection();
            }
        }
        x_AddEmptyRow();
        m_ScrolledWindow->FitInside();
    }
}

bool CLocationListCtrl::IsInIdList(wxString str)
{
    wxArrayString::iterator it = m_SeqIds.begin();
    while (it != m_SeqIds.end()) {
        if (str == *it) {
            return true;
        }
        it++;
    }
    return false;
}


wxString CLocationListCtrl::GetListIdStrForSeqId(const CSeq_id& seq_id)
{
    if (m_SeqIds.IsEmpty()) {
        PopulateIdListFromScope();
    }
    string str;
    if (m_Scope)
        CLabel::GetLabel(seq_id, &str, CLabel::eUserTypeAndContent, m_Scope);
    else
        seq_id.GetLabel(&str, CSeq_id::eContent);
    wxString seqId = ToWxString(str);
    if (!IsInIdList(seqId) && m_Scope) {
        // if this seqID is not in the list for the location editor,
        // try to find a different ID from the same BioSeq that is
        CBioseq_Handle bsh = m_Scope->GetBioseqHandle(seq_id);
        if (bsh) {
            ITERATE(CBioseq::TId, id_it, bsh.GetCompleteBioseq()->GetId()) {
                string check;
                CLabel::GetLabel(**id_it, &check, CLabel::eUserTypeAndContent, m_Scope);
                wxString wxchk = ToWxString(check);
                if (IsInIdList(wxchk)) {
                    seqId = wxchk;
                    break;
                }
            }
        }
    }
    return seqId;
}


/*!
 * Should we show tooltips?
 */

bool CLocationListCtrl::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CLocationListCtrl::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CLocationListCtrl bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CLocationListCtrl bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CLocationListCtrl::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CLocationListCtrl icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CLocationListCtrl icon retrieval
}

END_NCBI_SCOPE


