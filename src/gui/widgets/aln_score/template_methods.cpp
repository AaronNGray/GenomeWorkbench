/*  $Id: template_methods.cpp 40969 2018-05-04 17:00:04Z katargir $
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
 * Authors:  Andrey Yazhuk
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbireg.hpp>
#include <util/tables/raw_scoremat.h>

#include <gui/widgets/aln_score/template_methods.hpp>
#include <gui/widgets/aln_score/gradient_color_panel.hpp>

#include "wx/valgen.h"
#include <gui/widgets/wx/hyperlink.hpp>
#include <wx/scrolwin.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>

#include <gui/widgets/wx/number_validator.hpp>
#include <gui/widgets/wx/color_picker_validator.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CColorTableMethod
///
const static string kDefaultColorStr = "224 224 224";

CColorTableMethod::CColorTableMethod()
 : m_DefaultBackColor(CRgbaColor::GetColor(kDefaultColorStr)),
   m_DefaultForeColor(CRgbaColor::GetColor(kDefaultColorStr).ContrastingColor(true))
{
}


CColorTableMethod::~CColorTableMethod()
{
}


IScoringMethod* CColorTableMethod::Clone() const
{
    return new CColorTableMethod(*this);
}


static const string kTablerow = "TableRows";
static const string kTablerowF = "TableRowsForeground";
static const string kTablerowB = "TableRowsBackground";

bool CColorTableMethod::Load(CNcbiRegistry& reg)
{

    if(LoadInfo(reg))   {
        Clear();

        list <string> entries;
        bool hasTR  =  reg.HasEntry(kTablerow);
        bool hasTRF =  reg.HasEntry(kTablerowF);
        bool hasFGColors = ( hasTR  ||  hasTRF );
        bool hasBGColors = (reg.HasEntry(kTablerowB));

        if (hasTR) {
            reg.EnumerateEntries(kTablerow, &entries);
            ITERATE(list<string>, it, entries) {
                string entry = *it;
                entry = NStr::TruncateSpaces(entry);
                if (entry.size() != 1) {
                    ERR_POST("CColorTableMethod::Load() - base must be a single character: \""
                        << entry << "\".");
                }
                char cbase = entry[0];
                AddSetBase(cbase);
                string str = reg.GetString(kTablerow, entry, kDefaultColorStr);
                m_ForeColors[cbase] = CRgbaColor::GetColor(str);
                if (!hasBGColors) {
                    m_Colors[cbase] = m_ForeColors[cbase].ContrastingColor(false);
                }
            }
        }
        if (hasTRF) {
            reg.EnumerateEntries(kTablerowF, &entries);
            ITERATE(list<string>, it, entries) {
                string entry = *it;
                entry = NStr::TruncateSpaces(entry);
                if (entry.size() != 1) {
                    ERR_POST("CColorTableMethod::Load() - base must be a single character: \""
                        << entry << "\".");
                }
                char cbase = entry[0];
                AddSetBase(cbase);
                string str = reg.GetString(kTablerowF, entry, kDefaultColorStr);
                m_ForeColors[cbase] = CRgbaColor::GetColor(str);
                if (!hasBGColors) {
                    m_Colors[cbase] = m_ForeColors[cbase].ContrastingColor(false);
                }
            }
        }
        if (hasBGColors) {
            reg.EnumerateEntries(kTablerowB, &entries);
            ITERATE(list<string>, it, entries) {
                string entry = *it;
                entry = NStr::TruncateSpaces(entry);
                if (entry.size() != 1) {
                    ERR_POST("CColorTableMethod::Load() - base must be a single character: \""
                        << entry << "\".");
                }
                char cbase = entry[0];
                AddSetBase(cbase);
                string str = reg.GetString(kTablerowB, entry, kDefaultColorStr);
                m_Colors[cbase] = CRgbaColor::GetColor(str);
                if (! hasFGColors) {
                    m_ForeColors[cbase] = m_Colors[cbase].ContrastingColor(false);
                }
            }
        }
        return true;
    }
    return false;
}


void CColorTableMethod::Clear()
{
    m_Colors.clear();
    m_Colors.resize(256, m_DefaultBackColor);

    m_ForeColors.clear();
    m_ForeColors.resize(256, m_DefaultForeColor);
    m_SetBases.erase();
}

void CColorTableMethod::SetColor(char cbase, const CRgbaColor& color, EColorType type)
{
    AddSetBase(cbase);
    if (type == fForeground) {
        m_ForeColors[cbase] = color;
    } else {
        m_Colors[cbase] = color;
    }
}


const CRgbaColor& CColorTableMethod::GetColorForNoScore(EColorType type) const
{
    if (type  == fForeground) {
        return m_DefaultForeColor;
    }
    return m_DefaultBackColor;
}


void CColorTableMethod::SetColorForNoScore(IScoringMethod::EColorType type, const CRgbaColor& color)
{
    if (type  == fForeground) {
        m_DefaultForeColor = color;
    } else {
        m_DefaultBackColor = color;
    }
}


CRgbaColor CColorTableMethod::GetColor(char cbase,
                                     EColorType type) const
{
    return GetColorForScore(static_cast<TScore>(cbase), type);
}


const string& CColorTableMethod::GetSetBases() const
{
    return m_SetBases;
}


void CColorTableMethod::AddSetBase(char cbase)
{
   if (m_SetBases.find(cbase) == NPOS) {
        m_SetBases.push_back(cbase);
    }
}


bool    CColorTableMethod::Save(CNcbiRegistry& reg)
{
    if (SaveInfo(reg)) {
        const string& bases = GetSetBases();
        ITERATE(string, base_it, bases) {
            char cbase = *base_it;
            string colorstr = GetColor(cbase, fForeground)
                .ToString(false, true); // no alpha, ints not float.
            reg.Set(kTablerowF, string(1, cbase), colorstr);
            colorstr = GetColor(cbase, fBackground)
                .ToString(false, true); // no alpha, ints not float.
            reg.Set(kTablerowB, string(1, cbase), colorstr);
        }
        return true;
    }
    return false;
}

void    CColorTableMethod::CalculateScores(IAlnExplorer::TNumrow row,
                                           const IScoringAlignment& aln,
                                           TScoreColl& scores)
{
    IAlnExplorer::TSignedRange range(aln.GetAlnStart(), aln.GetAlnStop());
    string  buf;
    aln.GetAlnSeqString(row, buf, range);
    scores.SetFrom(range.GetFrom());
    TSeqPos base_width = aln.GetBaseWidth(row);
    for (size_t i = 0; i < buf.size(); i++) {
        char c = buf[i];
        scores.push_back((TScore)c, base_width);
    }
}


int     CColorTableMethod::GetSupportedColorTypes() const
{
    return fBackground | fForeground;
}


const CRgbaColor& CColorTableMethod::GetColorForScore(TScore score,
                                                    EColorType type) const
{
    size_t i = (size_t) score;
    _ASSERT(i < m_Colors.size());

    return (type == fBackground) ? m_Colors[i] : m_ForeColors[i];
}


const CMenuItem*    CColorTableMethod::GetMenu()
{
    return NULL; // no menu
}


bool CColorTableMethod::HasPropertiesPanel() const
{
    return true;
}

class CColorPanel : public wxScrolledWindow
{
    DECLARE_EVENT_TABLE()

    enum {
        kIdBase = 10000
    };

public:
    struct SData
    {
        string  m_Bases;
        wxColor m_ForeColor;
        wxColor m_BackColor;
    };

    CColorPanel(vector<SData>& data, wxWindow* parent, wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize);

    void SwapColors();
    void ContrastFG();
    void ContrastBG();
    void AddNewRow();

    void UpdateData();

    void OnChar( wxKeyEvent& event );
    void OnTextCtrlUpdated( wxCommandEvent& event );

protected:
    void OnDelete ( wxHyperlinkEvent& event );
    void x_AddNewRow(const string& text, wxColor fgColor, wxColor bgColor);
    void x_DeleteRow(wxWindowID id);

    void CreateControls();

    vector<SData>& m_Data;
    wxWindowID m_NewID;
};

BEGIN_EVENT_TABLE( CColorPanel, wxScrolledWindow )
    EVT_TEXT(wxID_ANY, CColorPanel::OnTextCtrlUpdated)
    EVT_HYPERLINK(wxID_ANY, CColorPanel::OnDelete)
END_EVENT_TABLE()

CColorPanel::CColorPanel(vector<SData>& data, wxWindow* parent, wxWindowID id,
                         const wxPoint& pos, const wxSize& size) :
    m_Data(data), m_NewID(kIdBase)
{
    Create(parent, id, pos, size, wxScrolledWindowStyle|wxBORDER_SUNKEN|wxTAB_TRAVERSAL);
    SetScrollRate(5, 5);
    CreateControls();
}

void CColorPanel::OnChar( wxKeyEvent& event )
{
    int key = event.GetKeyCode();
    if (wxIsspace(key) || wxIsdigit(key))
        wxBell();
    else
        event.Skip();
}

void CColorPanel::OnTextCtrlUpdated( wxCommandEvent& event )
{
    wxTextCtrl* textCtrlBase = (wxTextCtrl*)event.GetEventObject();
    wxString textBase = textCtrlBase->GetValue().Upper();

    for (wxWindowID i = kIdBase; i < m_NewID; i += 4) {
        wxTextCtrl* textCtrl = (wxTextCtrl*)FindWindow(i);
        if (textCtrl == 0 || textCtrl == textCtrlBase)
            continue;

        bool changed = false;
        wxString text = textCtrl->GetValue().Upper();

        for(;;) {
            size_t pos = text.find_first_of(textBase);
            if (pos == wxString::npos)
                break;

            changed = true;
            text.erase(pos, 1);
        }

        if (changed) textCtrl->ChangeValue(text);
    }

    textCtrlBase->ChangeValue(textBase);
    textCtrlBase->SetInsertionPointEnd();
}

void CColorPanel::UpdateData()
{
    m_Data.clear();

    for (wxWindowID i = kIdBase; i < m_NewID; i += 4) {
        wxTextCtrl* textCtrl = (wxTextCtrl*)FindWindow(i);
        wxColourPickerCtrl* pickerFG = (wxColourPickerCtrl*)FindWindow(i + 1);
        wxColourPickerCtrl* pickerBG = (wxColourPickerCtrl*)FindWindow(i + 2);
        if (textCtrl != 0 && pickerFG != 0 && pickerBG != 0) {
            CColorPanel::SData elem;
            elem.m_Bases = ToStdString(textCtrl->GetValue());
            elem.m_ForeColor = pickerFG->GetColour();
            elem.m_BackColor = pickerBG->GetColour();
            m_Data.push_back(elem);
        }
    }
}

void CColorPanel::CreateControls()
{
    wxFlexGridSizer* itemSizer = new wxFlexGridSizer(4, 0, 0);
    this->SetSizer(itemSizer);
    itemSizer->AddGrowableCol(0);

    itemSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Bases")),
        1, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxBOTTOM, 5);
    itemSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Text")),
        0, wxALIGN_LEFT|wxTOP|wxBOTTOM, 5);
    itemSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Back")),
        0, wxALIGN_LEFT|wxTOP|wxBOTTOM, 5);
    itemSizer->Add(new wxStaticText(this, wxID_ANY, wxT("")),
        0, wxALIGN_LEFT|wxTOP|wxBOTTOM, 5);

    for (size_t i = 0; i < m_Data.size(); ++i)
        x_AddNewRow(m_Data[i].m_Bases, m_Data[i].m_ForeColor, m_Data[i].m_BackColor);

    FitInside();
}


void CColorPanel::OnDelete (wxHyperlinkEvent& event)
{
    x_DeleteRow(event.GetId());
    FitInside();
}


void CColorPanel::SwapColors()
{
    for (wxWindowID i = kIdBase; i < m_NewID; i += 4) {
        wxColourPickerCtrl* pickerFG = (wxColourPickerCtrl*)FindWindow(i + 1);
        wxColourPickerCtrl* pickerBG = (wxColourPickerCtrl*)FindWindow(i + 2);
        if (pickerFG != 0 && pickerBG != 0) {
            wxColor color = pickerFG->GetColour();
            pickerFG->SetColour(pickerBG->GetColour());
            pickerBG->SetColour(color);
        }
    }
}

void CColorPanel::ContrastFG()
{
    for (wxWindowID i = kIdBase; i < m_NewID; i += 4) {
        wxColourPickerCtrl* pickerFG = (wxColourPickerCtrl*)FindWindow(i + 1);
        wxColourPickerCtrl* pickerBG = (wxColourPickerCtrl*)FindWindow(i + 2);
        if (pickerFG != 0 && pickerBG != 0) {
            CRgbaColor color = ConvertColor(pickerBG->GetColour()).ContrastingColor(false);
            pickerFG->SetColour(ConvertColor(color));
        }
    }
}

void CColorPanel::ContrastBG()
{
    for (wxWindowID i = kIdBase; i < m_NewID; i += 4) {
        wxColourPickerCtrl* pickerFG = (wxColourPickerCtrl*)FindWindow(i + 1);
        wxColourPickerCtrl* pickerBG = (wxColourPickerCtrl*)FindWindow(i + 2);
        if (pickerFG != 0 && pickerBG != 0) {
            CRgbaColor color = ConvertColor(pickerFG->GetColour()).ContrastingColor(false);
            pickerBG->SetColour(ConvertColor(color));
        }
    }
}

void CColorPanel::x_AddNewRow(const string& text, wxColor fgColor, wxColor bgColor)
{
    wxSizer* itemSizer = GetSizer();

    wxTextCtrl* itemTextControl1 =
        new wxTextCtrl(this, m_NewID, wxEmptyString,
            wxDefaultPosition, wxDLG_UNIT(this, wxSize(48, -1)), wxTE_LEFT);

    itemTextControl1->ChangeValue(ToWxString(text));

    itemTextControl1->Connect(m_NewID, wxEVT_CHAR, wxKeyEventHandler(CColorPanel::OnChar), NULL, this);

    itemSizer->Add(itemTextControl1, 1, wxGROW|wxALL, 2);

    wxColourPickerCtrl* itemColorPicker1 = new wxColourPickerCtrl(this, m_NewID + 1, fgColor);
    itemSizer->Add(itemColorPicker1, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 2);

    wxColourPickerCtrl* itemColorPicker2 = new wxColourPickerCtrl(this, m_NewID + 2, bgColor);
    itemSizer->Add(itemColorPicker2, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 2);

    wxHyperlinkCtrl* itemHyperLink = new CHyperlink(this, m_NewID + 3, wxT("Delete"), wxEmptyString);
    itemSizer->Add(itemHyperLink, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);
    m_NewID += 4;
}

void CColorPanel::x_DeleteRow(wxWindowID id)
{
    wxWindow* window = FindWindow(id);
    if (window == 0)
        return;

    wxSizer* sizer = GetSizer();
    int index = 0;
    wxSizerItemList::iterator node = sizer->GetChildren().begin();
    for ( ;  node != sizer->GetChildren().end();  ++node, ++index) {
        wxSizerItem* item = *node;
        if (item->IsWindow()&& item->GetWindow() == window)
            break;
    }

    if (node == sizer->GetChildren().end()) {
        return;
    }

    sizer->GetItem(index)->DeleteWindows();
    sizer->GetItem(index-1)->DeleteWindows();
    sizer->GetItem(index-2)->DeleteWindows();
    sizer->GetItem(index-3)->DeleteWindows();
    sizer->Remove(index);
    sizer->Remove(index-1);
    sizer->Remove(index-2);
    sizer->Remove(index-3);
}

void CColorPanel::AddNewRow()
{
    x_AddNewRow("", wxColor(255, 255, 255), wxColor(0, 0, 0));
    FitInside();
    int h, hv;
    GetClientSize(0, &h);
    GetVirtualSize(0, &hv);
    if (hv > h) Scroll(-1, (hv - h + 5)/5);
    wxWindow* text = FindWindow(m_NewID-4);
    if (text != 0)
        text->SetFocus();
}

///////////////////////////////////////////////////////////////////////////////
/// CColorTableScoringPanel - Properties Panel for CColorTableMethod

class CColorTableScoringPanel : public wxPanel
{
    DECLARE_EVENT_TABLE()
public:
    CColorTableScoringPanel(CColorTableMethod& method, wxWindow* parent, wxWindowID id = wxID_ANY);
protected:
    void CreateControls();
    void Init();
    void OnApply ( wxCommandEvent& event );
    void OnSwapColors ( wxCommandEvent& event );
    void OnContrastFG ( wxCommandEvent& event );
    void OnContrastBG ( wxCommandEvent& event );
    void OnAddRow ( wxCommandEvent& event );
    CColorTableMethod& m_Method;
// data members
    vector<CColorPanel::SData> m_Data;
    CColorPanel* m_ColorPanel;
};

#define ID_ADD_ROW      10000
#define ID_SWAP_COLORS  10001
#define ID_CONTRAST_FG  10002
#define ID_CONTRAST_BG  10003

BEGIN_EVENT_TABLE( CColorTableScoringPanel, wxPanel )
    EVT_BUTTON( wxID_APPLY, CColorTableScoringPanel::OnApply )
    EVT_BUTTON( ID_CONTRAST_FG, CColorTableScoringPanel::OnContrastFG)
    EVT_BUTTON( ID_CONTRAST_BG, CColorTableScoringPanel::OnContrastBG)
    EVT_BUTTON( ID_SWAP_COLORS, CColorTableScoringPanel::OnSwapColors)
    EVT_BUTTON( ID_ADD_ROW, CColorTableScoringPanel::OnAddRow)
END_EVENT_TABLE()

CColorTableScoringPanel::CColorTableScoringPanel(
    CColorTableMethod& method, wxWindow* parent, wxWindowID id) :
        m_Method(method), m_ColorPanel()
{
    Init();
    Create(parent, id);
    CreateControls();
}

void CColorTableScoringPanel::Init()
{
    const string& bases = m_Method.GetSetBases();
    ITERATE(string, base_it, bases) {
        wxColor fc = ConvertColor(m_Method.GetColor(*base_it, IScoringMethod::fForeground));
        wxColor bc = ConvertColor(m_Method.GetColor(*base_it, IScoringMethod::fBackground));
        bool cat_found = false;
        NON_CONST_ITERATE(vector<CColorPanel::SData>, cat_it, m_Data) {
            if (cat_it->m_ForeColor == fc  && cat_it->m_BackColor == bc) {
                cat_it->m_Bases.append(1, *base_it);
                cat_found = true;
                break;
            }
        }
        if ( ! cat_found) {
            CColorPanel::SData new_cat;
            new_cat.m_Bases = string(1, *base_it);
            new_cat.m_ForeColor = fc;
            new_cat.m_BackColor = bc;
            m_Data.push_back(new_cat);
        }
    }
}

void CColorTableScoringPanel::CreateControls()
{
    wxBoxSizer* itemSizer1 = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(itemSizer1);

    wxStaticBox* staticBox = new wxStaticBox(this, wxID_ANY, wxT("Bases Text and Background Colors"));
    wxStaticBoxSizer* staticSizer = new wxStaticBoxSizer(staticBox, wxVERTICAL);

    wxSize clrPanelSize = wxDLG_UNIT(this, wxSize(150, 100));
    m_ColorPanel = new CColorPanel(m_Data, this, wxID_ANY,
                            wxDefaultPosition, clrPanelSize);
    m_ColorPanel->SetMinSize(clrPanelSize);

    staticSizer->Add(m_ColorPanel, 1, wxGROW|wxALL, 5);
    staticSizer->Add(new wxButton(this, ID_ADD_ROW, wxT("Add Row")),
                    0, wxALIGN_LEFT|wxALL, 2);
    itemSizer1->Add(staticSizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);


    wxBoxSizer* itemSizer2 = new wxBoxSizer(wxVERTICAL);
    itemSizer2->Add(new wxButton(this, ID_SWAP_COLORS, wxT("Swap Colors")),
                    0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);
    itemSizer2->Add(new wxButton(this, ID_CONTRAST_FG, wxT("Contrast Text")),
                    0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);
    itemSizer2->Add(new wxButton(this, ID_CONTRAST_BG, wxT("Contrast Back")),
                    0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);
    itemSizer1->Add(itemSizer2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

void CColorTableScoringPanel::OnApply ( wxCommandEvent& event )
{
    wxUnusedVar(event);

    m_ColorPanel->UpdateData();

    m_Method.Clear();

    for (size_t i = 0; i < m_Data.size(); ++i) {
        ITERATE(string, base_it, m_Data[i].m_Bases) {
            m_Method.SetColor(*base_it,
                ConvertColor(m_Data[i].m_ForeColor), IScoringMethod::fForeground);
            m_Method.SetColor(*base_it,
                ConvertColor(m_Data[i].m_BackColor), IScoringMethod::fBackground);
        }
    }
}

void CColorTableScoringPanel::OnSwapColors ( wxCommandEvent& event )
{
    wxUnusedVar(event);
    m_ColorPanel->SwapColors();
}

void CColorTableScoringPanel::OnAddRow ( wxCommandEvent& event )
{
    wxUnusedVar(event);
    m_ColorPanel->AddNewRow();
}

void CColorTableScoringPanel::OnContrastFG ( wxCommandEvent& event )
{
    wxUnusedVar(event);
    m_ColorPanel->ContrastFG();
}

void CColorTableScoringPanel::OnContrastBG ( wxCommandEvent& event )
{
    wxUnusedVar(event);
    m_ColorPanel->ContrastBG();
}

wxWindow* CColorTableMethod::CreatePropertiesPanel(wxWindow* parent)
{
    return new CColorTableScoringPanel(*this, parent);
}

///////////////////////////////////////////////////////////////////////////////
/// CColumnScoringMethod

CColumnScoringMethod::CColumnScoringMethod()
:   m_MinScore(0),
    m_MaxScore(1),
    m_MinColor("black"),
    m_MidColor("gray"),
    m_MaxColor("white"),
    m_UseConsensus(false),
    m_WindowSize(0)
{
    m_vScore.clear();
    m_vScore.resize(256, m_MaxScore);
    m_Averageable = true;

    CreateColorTable(sm_DefGradientSize);
}


CColumnScoringMethod::~CColumnScoringMethod()
{
}

IScoringMethod* CColumnScoringMethod::Clone() const
{
    return new CColumnScoringMethod(*this);
}


void    CColumnScoringMethod::CreateColorTable(int size)
{
    _ASSERT(size >= 0);

    m_ColorTable.SetSize(size);
    if(m_ColorGradType == eThreeColorGradient) {
        m_ColorTable.FillGradient(0, size, m_MinColor, m_MidColor, m_MaxColor);
    } else {
        m_ColorTable.FillGradient(0, size, m_MinColor, m_MaxColor);
    }
}


bool CColumnScoringMethod::Load(CNcbiRegistry& reg)
{
    if(LoadInfo(reg))   {
        string table = "Table";

        m_MinScore = (TScore)reg.GetDouble(table, "MinimumValue", 0.0);
        m_MaxScore = (TScore)reg.GetDouble(table, "MaximumValue", 1.0);

        TScore default_mid_score =  (m_MaxScore + m_MinScore)/2;
        m_MidScore = (TScore)reg.GetDouble(table, "MidValue", default_mid_score);
        m_MinColor = x_GetColor(reg, "MinimumColor", "firebrick2");
        m_MaxColor = x_GetColor(reg, "MaximumColor", "royal blue");
        m_MidColor = x_GetColor(reg, "MidColor", "grey");

        CreateColorTable((int)m_ColorTable.GetSize());

        m_vScore.clear();
        m_vScore.resize(256, m_MaxScore);

        list <string> entries;
        table = "TableRows";
        reg.EnumerateEntries(table, &entries);

        ITERATE(list<string>, it, entries) {
            string entry = *it;
            entry = NStr::TruncateSpaces(entry);
            _ASSERT(entry.length() == 1);

            char cbase = entry[0];
            TScore score = (TScore)reg.GetDouble(table, entry, 0);
            m_vScore[cbase] = score;
        }
        return true;
    }
    return false;
}

void CColumnScoringMethod::CalculateScores(char cons,  const string& column,
                                           TScore& col_score,
                                           TScoreVector& scores)
{
    if (m_UseConsensus) {
        x_CalculateConsensusScores(cons, column, col_score, scores);
    } else {
        x_CalculateNonConsensusScores(column, col_score, scores);
    }
}

void CColumnScoringMethod::x_CalculateNonConsensusScores(const string& column,
                                                         TScore& /*col_score*/,
                                                         TScoreVector& scores)
{
    for (size_t i = 0; i < column.size(); i++) {
        char c = column[i];
        scores[i] = m_vScore[c];
    }
}


void CColumnScoringMethod::x_CalculateConsensusScores(char cons,  const string& column,
                                                      TScore& col_score,
                                                      TScoreVector& scores)
{
    _ASSERT(scores.size() == column.size());

    if (cons) {
        col_score = m_vScore[cons];
    } else {
        // calculate column scores
        col_score = 0;
        ITERATE (string, iter, column) {
            col_score += m_vScore[*iter];
        }
        col_score /= column.size();
    }

    for (size_t i = 0; i < column.size(); i++) {
        char c = column[i];
        TScore c_score = m_vScore[c];
//        scores[i] = fabs(c_score - col_score);
        scores[i] = (c_score - col_score);
    }
}

int     CColumnScoringMethod::GetSupportedColorTypes() const
{
    return fBackground;
}


bool CColumnScoringMethod::CanCalculateScores(const IScoringAlignment& )
{
    return true;
}


const CRgbaColor& CColumnScoringMethod::GetColorForNoScore(EColorType ) const
{
    static CRgbaColor not_used;
    _ASSERT(false);
    return not_used;
}


void CColumnScoringMethod::SetColorForNoScore(EColorType , const CRgbaColor& )
{
}


const CRgbaColor& CColumnScoringMethod::GetColorForScore(TScore score,
                                                       EColorType type) const
{
    int size = (int)m_ColorTable.GetSize();
    _ASSERT(size);

    if(type == fBackground) {
        TScore mins = m_MinScore;
        TScore mids = m_MidScore;
        TScore maxs = m_MaxScore;
        if (m_UseConsensus) {
            const float fudge_factor = 0.75;
            mins = (m_MinScore - m_MaxScore) * fudge_factor; // scores are rarely *exact* mismatches.
            mids = 0;
            maxs =  -mins;
        }

        TScore normal;
        if (score <= mids){
            normal= ((score - mins) / (mids - mins))/2;
        } else {
            normal = ((score - mids) / (maxs - mids))/2 + 0.5f;
        }
        #if defined(_DEBUG) && 0
        // This is to help determine the fudge_factor above.
        if (m_UseConsensus) {
            static TScore maxnormal = 0.0, minnormal = 1.0;
            if (normal > maxnormal) {
                maxnormal = normal;
                LOG_POST( "CColumnScoringMethod::GetColorForScore - max normal: " << maxnormal);
            }
            if (normal < minnormal) {
                minnormal = normal;
                LOG_POST( "CColumnScoringMethod::GetColorForScore - min normal: " << minnormal);
            }
        }
        #endif
        int index = (int) (normal * size);
        index = max(0, index);
        index = min(index, size - 1);

        return m_ColorTable[index];
    } else {
        _ASSERT(false); // not supported
        return m_ColorTable[0];
    }
}


size_t CColumnScoringMethod::GetWindowSize() const
{
    return m_WindowSize;
}


const CMenuItem*    CColumnScoringMethod::GetMenu()
{
    return NULL; // no menu
}


bool CColumnScoringMethod::HasPropertiesPanel() const
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// CColumnScoringPanel

class CColumnScoringPanel : public wxPanel
{
    DECLARE_EVENT_TABLE()
public:
    CColumnScoringPanel(CColumnScoringMethod& method, wxWindow* parent, wxWindowID id = wxID_ANY);
protected:
    void CreateControls();
    void Init();
    void OnApply ( wxCommandEvent& event );
    CColumnScoringMethod& m_Method;
    CGradientColorPanel* m_GradPanel;
// data members
    long m_WindowSize;
};

BEGIN_EVENT_TABLE( CColumnScoringPanel, wxPanel )
    EVT_BUTTON( wxID_APPLY, CColumnScoringPanel::OnApply )
END_EVENT_TABLE()

CColumnScoringPanel::CColumnScoringPanel(
    CColumnScoringMethod& method, wxWindow* parent, wxWindowID id) :
        m_Method(method), m_GradPanel(), m_WindowSize(0)
{
    Init();
    Create(parent, id);
    CreateControls();
}

void CColumnScoringPanel::Init()
{
    m_WindowSize = (long)m_Method.m_WindowSize*2 + 1;
}

void CColumnScoringPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer1);

    CGradientColorPanel::stParams params;
    params.m_FirstColor = m_Method.m_MinColor;
    params.m_MidColor = m_Method.m_MidColor;
    params.m_LastColor = m_Method.m_MaxColor;

    params.m_Steps = (int)m_Method.m_ColorTable.GetSize();
    params.m_ThreeColors =
        (m_Method.m_ColorGradType == CColumnScoringMethod::eThreeColorGradient);
    params.m_Reversable = true;
    params.m_MinLabel = NStr::DoubleToString(m_Method.m_MinScore, 2);
    if (m_Method.m_ColorGradType == CColumnScoringMethod::eThreeColorGradient) {
        params.m_MidLabel =  NStr::DoubleToString(m_Method.m_MidScore, 2);
    }
    params.m_MaxLabel = NStr::DoubleToString(m_Method.m_MaxScore, 2);

    m_GradPanel = new CGradientColorPanel(params, this);
    itemBoxSizer1->Add(m_GradPanel, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_RIGHT|wxALL, 5);

    itemBoxSizer2->Add(new wxStaticText(this, wxID_STATIC, wxT("Averaging Window Size:")),
                       0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextControl1 =
        new wxTextCtrl(this, wxID_ANY, wxEmptyString,
            wxDefaultPosition, wxDLG_UNIT(this, wxSize(26, -1)), wxTE_RIGHT);
    itemTextControl1->SetMaxLength(4);
    itemTextControl1->SetValidator(CNumberValidator(&m_WindowSize, 1, 2001));
    itemBoxSizer2->Add(itemTextControl1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox1 = new wxCheckBox(this, wxID_ANY, wxT("Use Consensus"));
    itemCheckBox1->SetValidator(wxGenericValidator(&m_Method.m_UseConsensus));
    itemBoxSizer1->Add(itemCheckBox1, 0, wxALIGN_LEFT|wxALL, 5);
}

void CColumnScoringPanel::OnApply ( wxCommandEvent& event )
{
    wxUnusedVar(event);

    CGradientColorPanel::stParams params;
    m_GradPanel->GetParams(params);

    m_Method.m_MinColor = params.m_FirstColor;
    m_Method.m_MidColor = params.m_MidColor;
    m_Method.m_MaxColor = params.m_LastColor;

    m_Method.CreateColorTable(params.m_Steps);

    m_Method.m_WindowSize = (m_WindowSize-1)/2;
}

wxWindow* CColumnScoringMethod::CreatePropertiesPanel(wxWindow* parent)
{
    return new CColumnScoringPanel(*this, parent);
}

///////////////////////////////////////////////////////////////////////////////
/// CMatrixScoringMethod

const unsigned int  kMatrixSize = 28;

CMatrixScoringMethod::CMatrixScoringMethod() :
    m_CalculateRange(eNone),
    m_MinScore(0),
    m_MaxScore(1),
    m_MinColor("black"),
    m_MidColor("gray"),
    m_MaxColor("white"),
    m_UndefColor(0.8f, 0.8f, 1.0f),
    m_UseConsensus(false),
    m_WindowSize(0)
{
    m_vScore.Resize(kMatrixSize, kMatrixSize, m_MaxScore);
    m_Averageable = true;

    CreateColorTable(sm_DefGradientSize);
}


CMatrixScoringMethod::~CMatrixScoringMethod()
{
}

int CMatrixScoringMethod::x_BaseToIndex(char c)
{
    int i;
    if (c == '-') {
        i = 26;
    } else {
        i = c - 'A'; // 0 - 25
        if (i < 0  || i > 25)
            i = kMatrixSize - 1;
    }
    return i;
}

void    CMatrixScoringMethod::CreateColorTable(int size)
{
    _ASSERT(size >= 0);

    if (m_UseConsensus) {
        if (m_CalculateRange & eConsensus)
            x_CalculateConsensusRange();
        m_MinScore = m_ConsensusMinScore;
        m_MaxScore = m_ConsensusMaxScore;
    }
    else {
        if (m_CalculateRange & eNonConsesus)
            x_CalculateNonConsensusRange();
        m_MinScore = m_NonConsensusMinScore;
        m_MaxScore = m_NonConsensusMaxScore;
    }


    m_ColorTable.SetSize(size);
    if(m_ColorGradType == eThreeColorGradient) {
        m_ColorTable.FillGradient(0, size, m_MinColor, m_MidColor, m_MaxColor);
    } else {
        m_ColorTable.FillGradient(0, size, m_MinColor, m_MaxColor);
    }
}


IScoringMethod* CMatrixScoringMethod::Clone() const
{
    return new CMatrixScoringMethod(*this);
}


bool CMatrixScoringMethod::Load(CNcbiRegistry& reg)
{
    if(LoadInfo(reg))   {
        string section = "Table";

        m_UseConsensus = reg.GetBool(section, "UseConsensus", false);
        if (!reg.HasEntry(section, "ConsensusMinimumValue") || (!reg.HasEntry(section, "ConsensusMaximumValue")))
            m_CalculateRange |= eConsensus;
        m_ConsensusMinScore = (TScore)reg.GetDouble(section, "ConsensusMinimumValue", numeric_limits<TScore>::max());
        m_ConsensusMaxScore = (TScore)reg.GetDouble(section, "ConsensusMaximumValue", numeric_limits<TScore>::min());

        if (!reg.HasEntry(section, "NonConsensusMinimumValue") || (!reg.HasEntry(section, "NonConsensusMaximumValue")))
            m_CalculateRange |= eNonConsesus;
        m_NonConsensusMinScore = (TScore)reg.GetDouble(section, "NonConsensusMinimumValue", numeric_limits<TScore>::max());
        m_NonConsensusMaxScore = (TScore)reg.GetDouble(section, "NonConsensusMaximumValue", numeric_limits<TScore>::min());
        m_MinColor = x_GetColor(reg, "MinimumColor", "firebrick2");
        m_MaxColor = x_GetColor(reg, "MaximumColor", "royal blue");

        m_UndefColor = x_GetColor(reg, "NoAnchorColor", "204 204 255");
                
        m_vScore.Resize(kMatrixSize, kMatrixSize, m_MaxScore);

        const string kBuiltinKey("Builtin");

        if (reg.HasEntry(section, kBuiltinKey)) {
            // read our scores from the NCBI toolkit's builtin scoring matrixes.

            const string& builtin_name = reg.Get(section, kBuiltinKey);

            const SNCBIPackedScoreMatrix* builtin_matrix = 0;
            if (NStr::CompareNocase(builtin_name, "blosum45") == 0) {
                builtin_matrix = &NCBISM_Blosum45;
            } else if (NStr::CompareNocase(builtin_name, "blosum62") == 0) {
                builtin_matrix = &NCBISM_Blosum62;
            } else if (NStr::CompareNocase(builtin_name, "blosum80") == 0) {
                builtin_matrix = &NCBISM_Blosum80;
            } else if (NStr::CompareNocase(builtin_name, "pam30") == 0) {
                builtin_matrix = &NCBISM_Pam30;
            } else if (NStr::CompareNocase(builtin_name, "pam70") == 0) {
                builtin_matrix = &NCBISM_Pam70;
            } else if (NStr::CompareNocase(builtin_name, "pam250") == 0) {
                builtin_matrix = &NCBISM_Pam250;
            }
            if (! builtin_matrix) {
                return false;
            }
            int base_cnt = (int) strlen(builtin_matrix->symbols);
            for (int i = 0; i < base_cnt; ++i) {
                int x = x_BaseToIndex(builtin_matrix->symbols[i]);
                for (int j = 0; j < base_cnt; ++j) {
                    TScore score = builtin_matrix->scores[i*base_cnt + j];
                    int y = x_BaseToIndex(builtin_matrix->symbols[j]);
                    m_vScore(x, y) = score;
                }
            }
        } else {
            // read our scores from the registry, which came from a file.

            list<string> columns;
            NStr::Split(reg.GetString(section, "Columns", ""), ", ;\n\r\t", columns, NStr::fSplit_Tokenize);
            if (columns.empty()) {
                // TODO Log...
                // Read the rows and assume the columns will be in the same order?
                //  (order not guaranteed in a registry).
                // Have a fixed sequence?
                return false;
            }
            list <string> row_names;
            section = "TableRows";
            reg.EnumerateEntries(section, &row_names);

            // if the first row has only one item, assume
            // this is a triangular matrix, otherwise it is rectangular.
            bool triangular;
            {{
                string first_base = columns.front().substr(0,1);
                list<string> scores;
                NStr::Split(reg.GetString(section, first_base, ""), ", ;\n\r\t", scores, NStr::fSplit_Tokenize);
                triangular = scores.size() == 1;
            }}

            ITERATE(list<string>, it, row_names) {
                string name = *it;
                name = NStr::TruncateSpaces(name);
                int x = x_BaseToIndex(name[0]);

                list<string> scores;
                NStr::Split(reg.GetString(section, name, ""), ", ;\n\r\t", scores, NStr::fSplit_Tokenize);
                list<string>::iterator col_it = columns.begin();
                ITERATE(list<string>, scores_it, scores) {
                    TScore score = (TScore)NStr::StringToDouble(*scores_it);
                    int y = x_BaseToIndex((*col_it)[0]);
                    m_vScore(x, y) = score;
                    if (triangular) {
                        m_vScore(y, x) = score;
                    }
                    col_it++;
                }
            }
        }
        CreateColorTable((int)m_ColorTable.GetSize());
        return true;
    }
    return false;
}

void    CMatrixScoringMethod::CalculateScores(char cons,  const string& column,
                                     TScore& col_score,  TScoreVector& scores)
{
    _ASSERT(scores.size() == column.size());
    // _ASSERT(cons > 0);

    if (cons > 0) {
        TScore score_sum = 0;

        // calculate individual scores
        int x = x_BaseToIndex(cons);
        if (x < 0  ||  x >= (int) m_vScore.GetRows()) {
            x = (int) m_vScore.GetRows() - 1;
        }

        for( size_t i = 0; i < column.size(); i++  )  {
            int y = x_BaseToIndex(column[i]);
            if (y < 0  ||  y >= (int) m_vScore.GetCols()) {
                y = (int) m_vScore.GetCols() - 1;
            }
            TScore score = m_vScore(x, y);
            scores[i] = score;
            score_sum += score;
        }

        int num = (int) column.size() + 1;
        col_score = score_sum/num;
        _ASSERT (col_score <= m_MaxScore);
    } else {
        // No master row. Average matrix scores across the whole column.

        // vector indexed by base,
        // caches index of first row this base appears in this column.
        vector<int> col_scores(m_vScore.GetRows(), -1);

        for( size_t i = 0; i < column.size(); i++  )  {
            TScore score;
            int x = x_BaseToIndex(column[i]);
            if (col_scores[x] > -1) {
                score = scores[col_scores[x]];
            } else {
                TScore score_sum = 0;
                if (x < 0  ||  x >= (int) m_vScore.GetRows()) {
                    x = (int) m_vScore.GetRows() - 1;
                }
                for( size_t j = 0; j < column.size(); j++  )  {
                    int y = x_BaseToIndex(column[j]);
                    if (y < 0  ||  y >= (int) m_vScore.GetCols()) {
                        y = (int) m_vScore.GetCols() - 1;
                    }
                    score_sum += m_vScore(y, x);;
                }
                score = score_sum/(column.size() + 1);
                col_scores[x] = (int) i;
            }
            scores[i] = score;
        }

        if (m_UseConsensus) {
            TScore avg_score = 0;
            for (size_t i = 0; i < column.size(); i++)  {
                avg_score += scores[i];
            }
            avg_score /= column.size();
            for (size_t i = 0; i < column.size(); i++)  {
                scores[i] -= avg_score;
            }
        }
    }
}


int     CMatrixScoringMethod::GetSupportedColorTypes() const
{
    return fBackground;
}


bool CMatrixScoringMethod::CanCalculateScores(const IScoringAlignment& /*aln*/)
{
    return true;
}


const CRgbaColor& CMatrixScoringMethod::GetColorForNoScore(EColorType type) const
{
    _VERIFY(type == fBackground);
    return m_UndefColor;
}


void CMatrixScoringMethod::SetColorForNoScore(EColorType type,
                                              const CRgbaColor& color)
{
    _VERIFY(type == fBackground);
    m_UndefColor = color;
}


const CRgbaColor& CMatrixScoringMethod::GetColorForScore(TScore score, EColorType type) const
{
    int size = (int)m_ColorTable.GetSize();
    _ASSERT(size);

    if(type == fBackground) {
        TScore normal = (score - m_MinScore) / (m_MaxScore - m_MinScore);

        int index = (int) (normal * size);
        index = max(0, index);
        index = min(index, size - 1);

        return m_ColorTable[index];
    } else {
        _ASSERT(false); // not supported
        return m_ColorTable[0];
    }
}


size_t CMatrixScoringMethod::GetWindowSize() const
{
    return m_WindowSize;
}

const CMenuItem*    CMatrixScoringMethod::GetMenu()
{
    return NULL; // no menu
}


bool CMatrixScoringMethod::HasPropertiesPanel() const
{
    return true;
}

void CMatrixScoringMethod::x_CalculateConsensusRange()
{
    m_ConsensusMinScore = numeric_limits<TScore>::max();
    m_ConsensusMaxScore = numeric_limits<TScore>::min();
    for (size_t x = 0; x < m_vScore.GetRows(); ++x) {
        TScore avg_score = 0;
        for (size_t y = 0; y < m_vScore.GetCols(); ++y) {
            avg_score += m_vScore(x, y);
        }
        avg_score /= m_vScore.GetCols();
        m_ConsensusMinScore = min(m_ConsensusMinScore, avg_score);
        m_ConsensusMaxScore = max(m_ConsensusMaxScore, avg_score);
    }
    m_CalculateRange &= ~eConsensus;
}

void CMatrixScoringMethod::x_CalculateNonConsensusRange()
{
    m_NonConsensusMinScore = numeric_limits<TScore>::max();
    m_NonConsensusMaxScore = numeric_limits<TScore>::min();
    for (size_t x = 0; x < m_vScore.GetRows(); ++x) {
        for (size_t y = 0; y < m_vScore.GetCols(); ++y) {
            TScore score = m_vScore(x, y);
            m_NonConsensusMinScore = min(m_NonConsensusMinScore, score);
            m_NonConsensusMaxScore = max(m_NonConsensusMaxScore, score);
        }
    }
    m_CalculateRange &= ~eNonConsesus;
}

///////////////////////////////////////////////////////////////////////////////
/// CMatrixScoringPanel

class CMatrixScoringPanel : public wxPanel
{
    DECLARE_EVENT_TABLE()
public:
    CMatrixScoringPanel(CMatrixScoringMethod& method, wxWindow* parent, wxWindowID id = wxID_ANY);
protected:
    void CreateControls();
    void Init();
    void OnApply ( wxCommandEvent& event );
    CMatrixScoringMethod& m_Method;
    CGradientColorPanel* m_GradPanel;
// data members
    long m_WindowSize;
};

BEGIN_EVENT_TABLE( CMatrixScoringPanel, wxPanel )
    EVT_BUTTON( wxID_APPLY, CMatrixScoringPanel::OnApply )
END_EVENT_TABLE()

CMatrixScoringPanel::CMatrixScoringPanel(
    CMatrixScoringMethod& method, wxWindow* parent, wxWindowID id) :
        m_Method(method), m_GradPanel(), m_WindowSize(0)
{
    Init();
    Create(parent, id);
    CreateControls();
}

void CMatrixScoringPanel::Init()
{
    m_WindowSize = (long)m_Method.m_WindowSize*2 + 1;
}

void CMatrixScoringPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer1);

    CGradientColorPanel::stParams params;
    params.m_FirstColor = m_Method.m_MinColor;
    params.m_MidColor = m_Method.m_MidColor;
    params.m_LastColor = m_Method.m_MaxColor;

    params.m_Steps = (int)m_Method.m_ColorTable.GetSize();
    params.m_ThreeColors =
        (m_Method.m_ColorGradType == CColumnScoringMethod::eThreeColorGradient);
    params.m_Reversable = true;
    params.m_MinLabel = NStr::DoubleToString(m_Method.m_MinScore, 2);
    params.m_MaxLabel = NStr::DoubleToString(m_Method.m_MaxScore, 2);

    m_GradPanel = new CGradientColorPanel(params, this);
    itemBoxSizer1->Add(m_GradPanel, 1, wxGROW|wxALL, 5);


    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(new wxStaticText(this, wxID_STATIC, wxT("Neutral (no master specified)")),
                       0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxColourPickerCtrl* itemColorPicker = new wxColourPickerCtrl(this, wxID_ANY);
    itemColorPicker->SetValidator(CColorPickerValidator(&m_Method.m_UndefColor));
    itemBoxSizer3->Add(itemColorPicker, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemBoxSizer1->Add(itemBoxSizer3, 0, wxALIGN_RIGHT|wxALL, 0);


    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_RIGHT|wxALL, 0);

    itemBoxSizer2->Add(new wxStaticText(this, wxID_STATIC, wxT("Averaging Window Size:")),
                       0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextControl1 =
        new wxTextCtrl(this, wxID_ANY, wxEmptyString,
            wxDefaultPosition, wxDLG_UNIT(this, wxSize(26, -1)), wxTE_RIGHT);
    itemTextControl1->SetMaxLength(4);
    itemTextControl1->SetValidator(CNumberValidator(&m_WindowSize, 1, 2001));
    itemBoxSizer2->Add(itemTextControl1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox1 = new wxCheckBox(this, wxID_ANY, wxT("Use Consensus"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    itemCheckBox1->SetValidator(wxGenericValidator(&m_Method.m_UseConsensus));
    itemBoxSizer1->Add(itemCheckBox1, 0, wxALIGN_RIGHT | wxALL, 5);
}

void CMatrixScoringPanel::OnApply ( wxCommandEvent& event )
{
    wxUnusedVar(event);

    CGradientColorPanel::stParams params;
    m_GradPanel->GetParams(params);

    m_Method.m_MinColor = params.m_FirstColor;
    m_Method.m_MidColor = params.m_MidColor;
    m_Method.m_MaxColor = params.m_LastColor;
    m_Method.CreateColorTable(params.m_Steps);

    m_Method.m_WindowSize = (m_WindowSize-1)/2;
}

wxWindow* CMatrixScoringMethod::CreatePropertiesPanel(wxWindow* parent)
{
    return new CMatrixScoringPanel(*this, parent);
}

END_NCBI_SCOPE
