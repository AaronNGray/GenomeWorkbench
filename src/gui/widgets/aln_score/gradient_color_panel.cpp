/*  $Id: gradient_color_panel.cpp 38289 2017-04-21 20:59:01Z katargir $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/aln_score/gradient_color_panel.hpp>

#include <gui/widgets/wx/color_picker_validator.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/statbox.h>
#include <wx/dcclient.h>
#include <wx/slider.h>
#include <wx/window.h>

BEGIN_NCBI_SCOPE

#define ID_STEPS_TXT   10000
#define ID_SLIDER      10001
#define ID_COLOR_PICK1 10002
#define ID_COLOR_PICK2 10003
#define ID_COLOR_PICK3 10004
#define ID_SWAP_COLORS 10005
#define ID_GRAD_WINDOW 10006

static const int kMaxGradColors = 64;

BEGIN_EVENT_TABLE( CGradientColorPanel, wxPanel )
    EVT_SLIDER( ID_SLIDER, CGradientColorPanel::OnSliderUpdated )
    EVT_BUTTON( ID_SWAP_COLORS, CGradientColorPanel::OnSwapClick )
    EVT_COLOURPICKER_CHANGED( ID_COLOR_PICK1, CGradientColorPanel::OnColourPickChanged)
    EVT_COLOURPICKER_CHANGED( ID_COLOR_PICK2, CGradientColorPanel::OnColourPickChanged)
    EVT_COLOURPICKER_CHANGED( ID_COLOR_PICK3, CGradientColorPanel::OnColourPickChanged)
END_EVENT_TABLE()


CGradientColorPanel::CGradientColorPanel(const stParams& params, wxWindow* parent, wxWindowID id)
    : m_Params(params), m_ItemStepsTxt(), m_GradWindow()
{
    Create(parent, id);
    CreateControls();
}

void CGradientColorPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer1);

    wxStaticBox* itemStaticBox = new wxStaticBox(this, wxID_STATIC, _(""));
    wxStaticBoxSizer* itemStaticBoxSizer = new wxStaticBoxSizer(itemStaticBox, wxVERTICAL);
    itemBoxSizer1->Add(itemStaticBoxSizer, 0, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer->Add(itemBoxSizer2, 0, wxGROW|wxALL, 5);

    m_ItemStepsTxt = new wxStaticText(this, ID_STEPS_TXT, _("999"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL);
    itemBoxSizer2->Add(m_ItemStepsTxt, 0, wxGROW|wxALL, 5);

    wxSlider* itemSlider = new wxSlider(this, ID_SLIDER, 0, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSUNKEN_BORDER );
    itemBoxSizer2->Add(itemSlider, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_GradWindow = new wxWindow(this, ID_GRAD_WINDOW, wxDefaultPosition, wxSize(250, 24), wxBORDER_SUNKEN );
    itemStaticBoxSizer->Add(m_GradWindow, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer->Add(itemBoxSizer3, 0, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);

    itemBoxSizer3->Add(itemBoxSizer4, 1, wxGROW|wxALL, 5);

    wxColourPickerCtrl* itemColorPicker1 = new wxColourPickerCtrl(this, ID_COLOR_PICK1);
    itemColorPicker1->SetValidator(CColorPickerValidator(&m_Params.m_FirstColor));
    itemBoxSizer4->Add(itemColorPicker1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    if (!m_Params.m_MinLabel.empty()) {
        wxStaticText* itemStaticText = new wxStaticText(this, wxID_STATIC, ToWxString(m_Params.m_MinLabel));
        itemBoxSizer4->Add(itemStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    }

    if (m_Params.m_ThreeColors) {
        wxColourPickerCtrl* itemColorPicker2 = new wxColourPickerCtrl(this, ID_COLOR_PICK2);
        itemColorPicker2->SetValidator(CColorPickerValidator(&m_Params.m_MidColor));
        itemBoxSizer3->Add(itemColorPicker2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

        if (!m_Params.m_MidLabel.empty()) {
            wxStaticText* itemStaticText = new wxStaticText(this, wxID_STATIC, ToWxString(m_Params.m_MidLabel));
            itemBoxSizer5->Add(itemStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        }
    }

    if (m_Params.m_Reversable) {
        wxButton* itemButton = new wxButton(this, ID_SWAP_COLORS,_("Swap Colors"));
        if (m_Params.m_ThreeColors)
            itemStaticBoxSizer->Add(itemButton, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
        else
            itemBoxSizer3->Add(itemButton, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    }

    itemBoxSizer5->Add(new wxBoxSizer(wxHORIZONTAL), 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    if (!m_Params.m_MaxLabel.empty()) {
        wxStaticText* itemStaticText = new wxStaticText(this, wxID_STATIC, ToWxString(m_Params.m_MaxLabel));
        itemBoxSizer5->Add(itemStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    }

    wxColourPickerCtrl* itemColorPicker3 = new wxColourPickerCtrl(this, ID_COLOR_PICK3);
    itemColorPicker3->SetValidator(CColorPickerValidator(&m_Params.m_LastColor));
    itemBoxSizer5->Add(itemColorPicker3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemBoxSizer3->Add(itemBoxSizer5, 1, wxGROW|wxALL, 5);

    int steps = m_Params.m_Steps;
    if(m_Params.m_ThreeColors)  {
        itemSlider->SetRange(3, kMaxGradColors - 1);

        if((steps & 0x1) == 0) { // even number
            ++steps;
            LOG_POST(Warning << "CGradientColorPanel::x_Update() - "
                     << "number of colors in gradient is even - make it odd.");
        }
        itemSlider->SetLineSize(2);
    }
    else {
        itemSlider->SetRange(2, kMaxGradColors);
        itemSlider->SetLineSize(1);
    }

    itemSlider->SetValue(steps);

    wxString str;
    str.Printf(_("%d"), steps);
    m_ItemStepsTxt->SetLabel(str);

    m_GradWindow->Disable();
    m_GradWindow->Connect(ID_GRAD_WINDOW, wxEVT_PAINT, wxPaintEventHandler(CGradientColorPanel::OnPaint), NULL, this);
}

void CGradientColorPanel::OnSliderUpdated( wxCommandEvent& event )
{
    wxString str;
    m_Params.m_Steps = event.GetInt();
    str.Printf(_("%d"), m_Params.m_Steps);
    m_ItemStepsTxt->SetLabel(str);
    m_GradWindow->Refresh();
}

void CGradientColorPanel::OnSwapClick( wxCommandEvent& event )
{
    wxUnusedVar(event);

    CRgbaColor color = m_Params.m_FirstColor;
    m_Params.m_FirstColor = m_Params.m_LastColor;
    m_Params.m_LastColor = color;

    TransferDataToWindow();
    m_GradWindow->Refresh();
}

void CGradientColorPanel::OnColourPickChanged( wxColourPickerEvent& event )
{
    wxUnusedVar(event);

    TransferDataFromWindow();
    m_GradWindow->Refresh();
}

void CGradientColorPanel::GetParams(stParams& params) const
{
    params = m_Params;
}

void CGradientColorPanel::OnPaint( wxPaintEvent& event )
{
    wxUnusedVar(event);

    wxPaintDC dc(m_GradWindow);
    dc.SetPen(*wxTRANSPARENT_PEN);

    int left = 0, top = 0, width, height;
    m_GradWindow->GetClientSize(&width, &height);

    int n = m_Params.m_Steps;
    double d = ((double) width) / n;

    if(m_Params.m_ThreeColors)  {
        int n1 = n / 2 + 1;
        // first half of the gradient bar
        for(int i = 0; i < n1 ; i++)    {
            int x1 = (int) floor(d * i);
            int x2 = (int) ceil(d * (i + 1));
            float k = ((float) i) / (n1 - 1);
            CRgbaColor  color = CRgbaColor::Interpolate(m_Params.m_MidColor,
                                                        m_Params.m_FirstColor,
                                                        k);
            wxBrush brush(wxColor(color.GetRedUC(),
                                  color.GetGreenUC(),
                                  color.GetBlueUC(),
                                  color.GetAlphaUC()));
            dc.SetBrush(brush);
            dc.DrawRectangle(left + x1, top, x2 - x1, height);
        }
        // second half of the gradient bar
        for( int i = n1; i < n ; i++)    {
            int x1 = (int) floor(d * i);
            int x2 = (int) ceil(d * (i + 1));
            float k = ((float) i - n1 + 1) / (n - n1);
            CRgbaColor  color = CRgbaColor::Interpolate(m_Params.m_LastColor,
                                                        m_Params.m_MidColor,
                                                        k);
            wxBrush brush(wxColor(color.GetRedUC(),
                                  color.GetGreenUC(),
                                  color.GetBlueUC(),
                                  color.GetAlphaUC()));

            dc.SetBrush(brush);
            dc.DrawRectangle(left + x1, top, x2 - x1, height);
        }

    } else { // draw gradient bar in one pass
        for(int i = 0; i < n ; i++)    {
            int x1 = (int) floor(d * i);
            int x2 = (int) ceil(d * (i + 1));
            float k = ((float) i) / (n - 1);
            CRgbaColor  color = CRgbaColor::Interpolate(m_Params.m_LastColor,
                                                        m_Params.m_FirstColor,
                                                        k);
            wxBrush brush(wxColor(color.GetRedUC(),
                                  color.GetGreenUC(),
                                  color.GetBlueUC(),
                                  color.GetAlphaUC()));

            dc.SetBrush(brush);
            dc.DrawRectangle(left + x1, top, x2 - x1, height);
        }
    }
}


END_NCBI_SCOPE
