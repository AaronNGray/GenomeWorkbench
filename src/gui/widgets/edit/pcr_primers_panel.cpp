/*  $Id: pcr_primers_panel.cpp 43089 2019-05-13 21:05:28Z filippov $
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
 * Authors:  Igor Filippov, based on Vasuki Palanigobu
 */

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include <ncbi_pch.hpp>
#include <objects/seqfeat/PCRPrimerSeq.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/hyperlink.hpp>
#include <wx/choice.h>
#include <wx/valnum.h>
#include <objects/misc/sequence_macros.hpp>
#include "pcr_primers_panel.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


IMPLEMENT_DYNAMIC_CLASS( CPCRPrimersPanel, wxPanel )


BEGIN_EVENT_TABLE( CPCRPrimersPanel, wxPanel )

EVT_HYPERLINK(wxID_ANY, CPCRPrimersPanel::OnDelete)

END_EVENT_TABLE()




CPCRPrimersPanel::CPCRPrimersPanel():
m_Source(NULL)
{
    Init();
}

CPCRPrimersPanel::CPCRPrimersPanel( wxWindow* parent, objects::CBioSource* source, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    m_Source = source;
    Init();
    x_FillList();
    Create(parent, id, pos, size, style);
}




bool CPCRPrimersPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{

    wxScrolledWindow::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}




CPCRPrimersPanel::~CPCRPrimersPanel()
{

}


/*
 * Member initialisation
 */

void CPCRPrimersPanel::Init()
{
    m_Sizer = NULL;
    m_TotalHeight = 0;
    m_TotalWidth = 0;
    m_ScrollRate = 0;
}


/*
 * Control creation for CSrcModListPanel
 */

void CPCRPrimersPanel::CreateControls()
{    
    SetScrollbars(1, 1, 0, 0);
    m_Sizer = new wxFlexGridSizer(0, 2, 0, 0);
    SetSizer(m_Sizer);
    FitInside();
}



void CPCRPrimersPanel::x_AddAllRows ()
{
    m_TotalHeight = 0;
    size_t cnt = m_Data.size();

    for (size_t i = 0; i < cnt; i++) 
        x_AddRow(m_Data[i]);
}

void CPCRPrimersPanel::x_AdjustScrollWindow()
{
//Adjusting size of the scroll window
    Freeze();
    int minHeight = 4 * m_TotalHeight/(m_Data.size()+1);
    SetVirtualSize(m_TotalWidth, m_TotalHeight);
    SetScrollRate(0, m_ScrollRate);
    FitInside();
    int win_height, win_width;
    GetSize(&win_width, &win_height);
    SetMinSize(wxSize(m_TotalWidth + 20, minHeight));
    Layout();
    Thaw();
}


bool CPCRPrimersPanel::TransferDataToWindow()
{
    wxSizerItemList& children = m_Sizer->GetChildren();
    wxSizerItemList::iterator node = children.begin();
    while ( node != children.end())
    {
        (**node).DeleteWindows();
        node++;
    }
    m_Sizer->Clear(true);
    x_AddAllRows();
    x_AdjustScrollWindow();

    if (!wxPanel::TransferDataToWindow())
        return false;

    return true;
}


void CPCRPrimersPanel::x_AddEmptyRow(int nset, bool fwd_dir)
{   
    CPCRPrimerEntry::SPrimerData data;
    data.set = nset;
    data.seq = wxEmptyString;
    data.name = wxEmptyString;
    data.forward_dir = fwd_dir;
    m_Data.push_back(data);
    x_AddRow(data);
}

void CPCRPrimersPanel::x_AddRow(const CPCRPrimerEntry::SPrimerData &data)
{
    CPCRPrimerEntry* panel = new CPCRPrimerEntry(this, data);
    m_Sizer->Add(panel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    panel->TransferDataToWindow();
    wxHyperlinkCtrl* itemHyperlinkCtrl = new CHyperlink( this, wxID_ANY, wxT("Delete"), wxT(""), wxDefaultPosition, wxSize(60,-1), wxHL_DEFAULT_STYLE );
    m_Sizer->Add(itemHyperlinkCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    int row_width = 0;
    int row_height = 0;
    panel->GetClientSize(&row_width, &row_height);
    int link_width = 0;
    int link_height = 0;
    itemHyperlinkCtrl->GetClientSize (&link_width, &link_height);
    if (link_height > row_height) 
        row_height = link_height;
    
    row_width += link_width;

    if (row_width > m_TotalWidth) 
        m_TotalWidth = row_width;
    
    m_TotalHeight += row_height;
    m_ScrollRate = row_height;
}

bool CPCRPrimersPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    wxSizerItemList& children = m_Sizer->GetChildren();
    wxSizerItemList::iterator node = children.begin();
    int node_count = 0;
    m_Data.clear();
    while (node != children.end()) 
    {
        wxWindow *w = (*node)->GetWindow();
        if (w)
        {
            CPCRPrimerEntry* panel = dynamic_cast<CPCRPrimerEntry*> (w);
            if (panel)
            {
                panel->TransferDataFromWindow();
                CPCRPrimerEntry::SPrimerData entry = panel->GetData();
                node_count++;
                m_Data.push_back(entry);
            }
        }
        node++;
    }
    if (m_Source)
        UpdateBioSourcePrimers(*m_Source);


    x_AdjustScrollWindow();

    return true;
}

void CPCRPrimersPanel::x_FillList()
{
    m_Data.clear();
    int setnum = 0;
    if (m_Source->IsSetPcr_primers())
    {
        FOR_EACH_PCRREACTION_IN_PCRREACTIONSET(reaction,m_Source->GetPcr_primers())
        {
            setnum++;
            if ((*reaction)->CanGetForward()) {
                FOR_EACH_PCRPRIMER_IN_PCRPRIMERSET(primer,(*reaction)->GetForward())
                {
                    if ((*primer)->IsSetSeq() || (*primer)->IsSetName())
                    {
                        string seq = "";
                        string name = "";
                        if ((*primer)->IsSetSeq()) {
                            seq = (*primer)->GetSeq();
                        }
                        if ((*primer)->IsSetName()) {
                            name = (*primer)->GetName();
                        }
                        CPCRPrimerEntry::SPrimerData entry;
                        entry.name = ToWxString(name);
                        entry.seq = ToWxString(seq);
                        entry.forward_dir = true;
                        entry.set = setnum;
                        m_Data.push_back(entry);
                    }
                }
            }
            if ((*reaction)->CanGetReverse()) {
                FOR_EACH_PCRPRIMER_IN_PCRPRIMERSET(primer,(*reaction)->GetReverse())
                {
                    if ((*primer)->IsSetSeq() || (*primer)->IsSetName())
                    {
                        string seq = "";
                        string name = "";
                        if ((*primer)->IsSetSeq()) {
                            seq = (*primer)->GetSeq();
                        }
                        if ((*primer)->IsSetName()) {
                            name = (*primer)->GetName();
                        }

                        CPCRPrimerEntry::SPrimerData entry;
                        entry.name = ToWxString(name);
                        entry.seq = ToWxString(seq);
                        entry.forward_dir = false;
                        entry.set = setnum;
                        m_Data.push_back(entry);
                    }
                }
            }
        }
    }
    if (m_Data.empty())
    {
        CPCRPrimerEntry::SPrimerData entry1,entry2;
        entry1.set = 1;
        entry1.forward_dir = true;
        entry2.set = 1;
        entry2.forward_dir = false;
        m_Data.push_back(entry1);
        m_Data.push_back(entry2);
    }
}

void CPCRPrimersPanel::UpdateBioSourcePrimers(objects::CBioSource& source)
{
    source.ResetPcr_primers();

    set<int> setnums;
    for (size_t i = 0; i < m_Data.size(); i++) {
        if (m_Data[i].set > 0) {
            setnums.insert(m_Data[i].set);
        }
    }

    for (set<int>::iterator s = setnums.begin(); s != setnums.end(); s++)
    {
        CRef< CPCRReaction > reaction(new CPCRReaction);
        for (size_t i = 0; i < m_Data.size(); i++) {
            if (m_Data[i].set == *s)
            {
                CRef< CPCRPrimer > primer(new CPCRPrimer);
                if (!m_Data[i].name.IsEmpty())
                    primer->SetName().Set(ToStdString(m_Data[i].name));
                if (!m_Data[i].seq.IsEmpty())
                    primer->SetSeq().Set(ToStdString(m_Data[i].seq));

                if (m_Data[i].forward_dir)
                    reaction->SetForward().Set().push_back(primer);
                else
                    reaction->SetReverse().Set().push_back(primer);
            }
        }
        source.SetPcr_primers().Set().push_back(reaction);
    }      
}


int CPCRPrimersPanel::x_FindRow(wxWindow* wnd, wxSizerItemList& itemList)
{
    if (NULL == wnd)
        return -1;

    // start with -1, first qual found will bump it to 0
    int row_num = -1;
    for(wxSizerItemList::iterator it = itemList.begin(); it != itemList.end(); ++it) 
    {
        wxWindow* child = (**it).GetWindow();
        if (child && child == wnd)
            return row_num;
        
        CPCRPrimerEntry* qual = dynamic_cast<CPCRPrimerEntry*> (child);
        if (qual) 
        {
            row_num++;
        }
    }
   
    return -1;
}


void CPCRPrimersPanel::OnDelete (wxHyperlinkEvent& event)
{
    // find the control, remove it and the item before it, reset the scrolling
    wxWindow* wnd = (wxWindow*)event.GetEventObject();
    if (wnd == NULL)  return;
      
    wxSizerItemList& itemList = m_Sizer->GetChildren();
    int row = x_FindRow (wnd, itemList);
    if (row > -1 && (size_t) row < m_Data.size()) 
    {       
        m_Data.erase(m_Data.begin()+row);
        TransferDataToWindow();
    }

}


/*
 * Should we show tooltips?
 */

bool CPCRPrimersPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CPCRPrimersPanel::GetBitmapResource( const wxString& name )
{
  
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*
 * Get icon resources
 */

wxIcon CPCRPrimersPanel::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}



IMPLEMENT_DYNAMIC_CLASS( CPCRPrimerEntry, wxPanel )

BEGIN_EVENT_TABLE( CPCRPrimerEntry, wxPanel )

    EVT_TEXT( ID_PCRPRIMER_SEQ, CPCRPrimerEntry::OnSeqUpdated )

END_EVENT_TABLE()


CPCRPrimerEntry::CPCRPrimerEntry()
{
    Init();
}

CPCRPrimerEntry::CPCRPrimerEntry( wxWindow* parent, SPrimerData primer_data, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    m_PrimerData = primer_data;   
    Init();
    Create(parent, id, pos, size, style);
}

bool CPCRPrimerEntry::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{

    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}




CPCRPrimerEntry::~CPCRPrimerEntry()
{
}


/*
 * Member initialisation
 */

void CPCRPrimerEntry::Init()
{
    m_PrimerDir = NULL;
    m_PrimerName = NULL;
    m_PrimerSeq = NULL;
    m_PrimerSet = NULL;
}



void CPCRPrimerEntry::CreateControls()
{    

    CPCRPrimerEntry* itemPanel1 = this;

    wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(sizer1);

    wxArrayString primerDirStrings;
    primerDirStrings.Add(_("Forward"));
    primerDirStrings.Add(_("Reverse"));
   
    wxIntegerValidator<unsigned int> val;
    m_PrimerSet = new wxTextCtrl( itemPanel1, ID_PCRPRIMER_SET, wxEmptyString, wxDefaultPosition, wxSize(30,-1), 0, val);
    sizer1->Add(m_PrimerSet, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_PrimerDir = new wxChoice( itemPanel1, ID_PCRPRIMER_DIR, wxDefaultPosition, wxSize(80,-1), primerDirStrings );
    sizer1->Add(m_PrimerDir, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_PrimerName = new wxTextCtrl( itemPanel1, ID_PCRPRIMER_NAME, wxEmptyString, wxDefaultPosition, wxSize(80,-1), 0 );
    sizer1->Add(m_PrimerName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_PrimerSeq = new wxTextCtrl( itemPanel1, ID_PCRPRIMER_SEQ, wxEmptyString, wxDefaultPosition, wxSize(90,-1), 0 );
    sizer1->Add(m_PrimerSeq, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
}


bool CPCRPrimerEntry::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;
    if (m_PrimerData.set > 0)
    {
        m_PrimerName->ChangeValue(m_PrimerData.name);
        m_PrimerSeq->ChangeValue(m_PrimerData.seq);
        m_PrimerSet->ChangeValue(wxString::Format(wxT("%i"),m_PrimerData.set));
        if (m_PrimerData.forward_dir)
        {
            int n = m_PrimerDir->FindString("Forward");
            m_PrimerDir->SetSelection(n);
        }
        else
        {
            int n = m_PrimerDir->FindString("Reverse");
            m_PrimerDir->SetSelection(n);
        }   
    }


    return true;
}

bool CPCRPrimerEntry::TransferDataFromWindow()
{   
    m_PrimerData.name = m_PrimerName->GetValue();
    string seq = ToStdString(m_PrimerSeq->GetValue());
    CPCRPrimerSeq::Clean(seq);
    m_PrimerData.seq = ToWxString(seq);
    m_PrimerData.set = wxAtoi(m_PrimerSet->GetValue());
    int n = m_PrimerDir->FindString("Forward");
    int i = m_PrimerDir->GetSelection();
    if (n == i)
        m_PrimerData.forward_dir = true;
    else
        m_PrimerData.forward_dir = false;
    return true;

    return wxPanel::TransferDataFromWindow();
}

CPCRPrimerEntry::SPrimerData CPCRPrimerEntry::GetData()
{
    return m_PrimerData;
}

void CPCRPrimerEntry::SetData(CPCRPrimerEntry::SPrimerData entry)
{
    m_PrimerData = entry;
}

/*
 * Should we show tooltips?
 */

bool CPCRPrimerEntry::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CPCRPrimerEntry::GetBitmapResource( const wxString& name )
{
     wxUnusedVar(name);
    return wxNullBitmap;
}

/*
 * Get icon resources
 */

wxIcon CPCRPrimerEntry::GetIconResource( const wxString& name )
{
     wxUnusedVar(name);
    return wxNullIcon;
}



void CPCRPrimerEntry::OnSeqUpdated( wxCommandEvent& event )
{
    wxTextCtrl* item = (wxTextCtrl*)event.GetEventObject();
    if (item->GetValue().IsEmpty()) return;

    TransferDataFromWindow();

    wxWindow* parent = this->GetParent();

    CPCRPrimersPanel* listpanel = dynamic_cast<CPCRPrimersPanel*>(parent);

    while (parent && !listpanel) 
    {
        parent = parent->GetParent();
        listpanel = dynamic_cast<CPCRPrimersPanel*>(parent);
    }
    if (!listpanel)     return;
    
    listpanel->TransferDataFromWindow();

    event.Skip();
}

END_NCBI_SCOPE
