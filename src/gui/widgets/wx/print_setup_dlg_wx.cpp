
#include <ncbi_pch.hpp>
#include <gui/widgets/wx/print_setup_dlg_wx.hpp>

#include <gui/widgets/wx/preview_setup_widget.hpp>

#include <util/image/image.hpp>

#include "wx/utils.h"
#include "wx/dc.h"
#include "wx/stattext.h"
#include "wx/statbox.h"
#include "wx/button.h"
#include "wx/checkbox.h"
#include "wx/textctrl.h"
#include "wx/radiobox.h"
#include "wx/slider.h"
#include "wx/combobox.h"
#include "wx/intl.h"
#include "wx/sizer.h"
#include "wx/cmndata.h"

#if wxUSE_STATLINE
    #include "wx/statline.h"
#endif

#include "wx/prntbase.h"
#include "wx/printdlg.h"
#include "wx/paper.h"
#include "wx/filename.h"
#include "wx/tokenzr.h"
#include "wx/imaglist.h"

#include <stdlib.h>
#include <string.h>

/*
#ifndef __WXUNIVERSAL__

#if wxUSE_LIBGNOMEPRINT
    #include "wx/link.h"
    wxFORCE_LINK_MODULE(gnome_print)
#endif

#endif // !__WXUNIVERSAL__
*/

// ----------------------------------------------------------------------------
// global vars
// ----------------------------------------------------------------------------

//extern wxPrintPaperDatabase *wxThePrintPaperDatabase;
extern WXDLLEXPORT_DATA(wxPrintPaperDatabase*) wxThePrintPaperDatabase;

// ----------------------------------------------------------------------------
// Generic page setup dialog
// ----------------------------------------------------------------------------

BEGIN_NCBI_SCOPE

IMPLEMENT_CLASS(CPrintSetupDlg, CPreviewSetupDlg)

BEGIN_EVENT_TABLE(CPrintSetupDlg, CPreviewSetupDlg)
    EVT_BUTTON(ePrintIDSetup, CPrintSetupDlg::OnPrinter)
    EVT_SCROLL(CPrintSetupDlg::OnScroll)
    EVT_RADIOBOX(eMultiPageID, CPrintSetupDlg::OnMultiPage)
    EVT_RADIOBOX(eGuidesID, CPrintSetupDlg::OnGuides)
END_EVENT_TABLE()

CPrintSetupDlg::CPrintSetupDlg(CRef<CImage> img,
                               wxWindow *parent,
                               wxPageSetupDialogData* data)
    : CPreviewSetupDlg( img, parent)
    , m_OrientationRadioBox(NULL)
    , m_MarginLeftText(NULL)        
    , m_MarginTopText(NULL)       
    , m_MarginRightText(NULL)      
    , m_MarginBottomText(NULL)
    , m_PaperTypeChoice(NULL)
{
    if (data)
        m_PageData = *data;

    // Layout all the controls
    x_InitDialog(img);
}

CPrintSetupDlg::~CPrintSetupDlg()
{
}

void CPrintSetupDlg::x_Init(wxBoxSizer* mainsizer, int widget_width)
{
    int text_width = 80;

    // 1. Paper Choice
    ///////////////////////////////////////////////////////////////////////////
    size_t      n = wxThePrintPaperDatabase->GetCount();
    wxString   *choices = new wxString [n];

    for (size_t i = 0; i < n; i++)
    {
        wxPrintPaperType *paper = wxThePrintPaperDatabase->Item(i);
        choices[i] = paper->GetName();
    }

    
    m_PaperTypeChoice = new wxComboBox( this,
                                        ePrintIDPaperSize,
                                        wxT("Paper"),
                                        wxDefaultPosition,
                                        wxSize(widget_width, wxDefaultCoord),
                                        (int)n, choices );

    wxStaticBoxSizer *paper_choice_sizer = new wxStaticBoxSizer(
        new wxStaticBox(this,ePrintIDStatic, wxT("Paper size")), 
        wxHORIZONTAL );
    
    paper_choice_sizer->Add( m_PaperTypeChoice, 1, wxEXPAND|wxALL|wxALIGN_BOTTOM, 5 );
    wxBoxSizer *paper_box = new wxBoxSizer( wxHORIZONTAL );
    paper_box->Add(paper_choice_sizer, 0, wxALL, 5);

    wxString *choices2 = new wxString[2];
    choices2[0] = wxT("Portrait");
    choices2[1] = wxT("Landscape");
    m_OrientationRadioBox = new wxRadioBox(this, ePrintIDOrientation, wxT("Paper Orientation"),
        wxDefaultPosition, wxDefaultSize, 2, choices2, 2);
    m_OrientationRadioBox->SetSelection(0);

    paper_box->Add(m_OrientationRadioBox, 0, wxALL|wxALIGN_TOP, 5 );

    mainsizer->Add( paper_box, 0, wxALL|wxALIGN_CENTER, 5 );
   

    // 2. margins
    ///////////////////////////////////////////////////////////////////////////
    wxBoxSizer *table = new wxBoxSizer( wxHORIZONTAL );

    wxBoxSizer *column1 = new wxBoxSizer( wxVERTICAL );
    column1->Add( new wxStaticText(this, ePrintIDStatic, wxT("Left margin (mm):")),1,wxALL|wxALIGN_CENTER,5 );
    column1->Add( new wxStaticText(this, ePrintIDStatic, wxT("Top margin (mm):")),1,wxALL|wxALIGN_CENTER,5 );
    table->Add( column1, 0, wxALL |wxEXPAND, 5 );

    wxBoxSizer *column2 = new wxBoxSizer( wxVERTICAL );
    m_MarginLeftText = new wxTextCtrl(this, ePrintIDLeftMargin, wxEmptyString, wxDefaultPosition, wxSize(text_width, wxDefaultCoord));
    m_MarginTopText = new wxTextCtrl(this, ePrintIDTopMargin, wxEmptyString, wxDefaultPosition, wxSize(text_width, wxDefaultCoord));
    column2->Add( m_MarginLeftText, 1, wxALL, 5 );
    column2->Add( m_MarginTopText, 1, wxALL, 5 );
    table->Add( column2, 0, wxRIGHT|wxTOP|wxBOTTOM |wxEXPAND, 5 );

    wxBoxSizer *column3 = new wxBoxSizer( wxVERTICAL );
    column3->Add( new wxStaticText(this, ePrintIDStatic, wxT("Right margin (mm):")),1,wxALL|wxALIGN_CENTER,5 );
    column3->Add( new wxStaticText(this, ePrintIDStatic, wxT("Bottom margin (mm):")),1,wxALL|wxALIGN_CENTER,5 );
    table->Add( column3, 0, wxALL|wxEXPAND, 5 );

    wxBoxSizer *column4 = new wxBoxSizer( wxVERTICAL );
    m_MarginRightText = new wxTextCtrl(this, ePrintIDRightMargin, wxEmptyString, wxDefaultPosition, wxSize(text_width, wxDefaultCoord));
    m_MarginBottomText = new wxTextCtrl(this, ePrintIDBottomMargin, wxEmptyString, wxDefaultPosition, wxSize(text_width, wxDefaultCoord));
    column4->Add( m_MarginRightText, 1, wxALL, 5 );
    column4->Add( m_MarginBottomText, 1, wxALL, 5 );
    table->Add( column4, 0, wxRIGHT|wxTOP|wxBOTTOM|wxEXPAND, 5 );

    mainsizer->Add( table, 0, wxALIGN_CENTER );
}

wxPageSetupDialogData& CPrintSetupDlg::GetPageSetupDialogData()
{
    return m_PageData;
}

bool CPrintSetupDlg::TransferDataToWindow()
{
    // Don't transfer data until window controls have been created
    if (!m_MarginLeftText)
        return false;

    CPreviewSetupDlg::TransferDataToWindow();
    
    m_MarginLeftText->SetValue(wxString::Format(wxT("%d"), m_PageData.GetMarginTopLeft().x));    
    m_MarginTopText->SetValue(wxString::Format(wxT("%d"), m_PageData.GetMarginTopLeft().y)); 
    m_MarginRightText->SetValue(wxString::Format(wxT("%d"), m_PageData.GetMarginBottomRight().x));    
    m_MarginBottomText->SetValue(wxString::Format(wxT("%d"), m_PageData.GetMarginBottomRight().y));

    if (m_PageData.GetPrintData().GetOrientation() == wxPORTRAIT)
        m_OrientationRadioBox->SetSelection(0);
    else
        m_OrientationRadioBox->SetSelection(1);

    // Find the paper type from either the current paper size in the wxPageSetupDialogData, or
    // failing that, the id in the wxPrintData object.

    wxPrintPaperType* type = wxThePrintPaperDatabase->FindPaperType(
             wxSize(m_PageData.GetPaperSize().x * 10, m_PageData.GetPaperSize().y * 10));

    if (!type && m_PageData.GetPrintData().GetPaperId() != wxPAPER_NONE)
        type = wxThePrintPaperDatabase->FindPaperType(m_PageData.GetPrintData().GetPaperId());

    if (type)
    {
        m_PaperTypeChoice->SetStringSelection(type->GetName());
    }

    return true;
}

bool CPrintSetupDlg::TransferDataFromWindow()
{
    CPreviewSetupDlg::TransferDataFromWindow();

    if (m_MarginLeftText && m_MarginTopText)
    {
        int left = wxAtoi( m_MarginLeftText->GetValue().c_str() );
        int top = wxAtoi( m_MarginTopText->GetValue().c_str() );
        m_PageData.SetMarginTopLeft( wxPoint(left,top) );
    }
    if (m_MarginRightText && m_MarginBottomText)
    {
        int right = wxAtoi( m_MarginRightText->GetValue().c_str() );
        int bottom = wxAtoi( m_MarginBottomText->GetValue().c_str() );
        m_PageData.SetMarginBottomRight( wxPoint(right,bottom) );
    }

    if (m_OrientationRadioBox)
    {
        int sel = m_OrientationRadioBox->GetSelection();
        if (sel == 0)
        {
            m_PageData.GetPrintData().SetOrientation(wxPORTRAIT);
        }
        else
        {
            m_PageData.GetPrintData().SetOrientation(wxLANDSCAPE);
        }
    }

    if (m_PaperTypeChoice)
    {        
        int selectedItem = m_PaperTypeChoice->GetSelection();
        if (selectedItem != -1)
        {
            wxPrintPaperType *paper = wxThePrintPaperDatabase->Item(selectedItem);
            if ( paper )
            {
                m_PageData.SetPaperSize(wxSize(paper->GetWidth()/10, paper->GetHeight()/10));
                m_PageData.GetPrintData().SetPaperId(paper->GetId());
            }
        }        
    }

    return true;
}

wxComboBox *CPrintSetupDlg::CreatePaperTypeChoice(int *x, int *y)
{
/*
    if (!wxThePrintPaperDatabase)
    {
        wxThePrintPaperDatabase = new wxPrintPaperDatabase;
        wxThePrintPaperDatabase->CreateDatabase();
    }
*/

    size_t      n = wxThePrintPaperDatabase->GetCount();
    wxString   *choices = new wxString [n];

    for (size_t i = 0; i < n; i++)
    {
        wxPrintPaperType *paper = wxThePrintPaperDatabase->Item(i);
        choices[i] = paper->GetName();
    }

    (void) new wxStaticText(this, ePrintIDStatic, wxT("Paper size"), wxPoint(*x, *y));
    *y += 25;

    wxComboBox *choice = new wxComboBox( this,
                                         ePrintIDPaperSize,
                                         wxT("Paper Size"),
                                         wxPoint(*x, *y),
                                         wxSize(300, wxDefaultCoord),
                                         (int)n, choices );
    *y += 35;
    delete[] choices;

//    choice->SetSelection(sel);
    return choice;
}

void CPrintSetupDlg::UpdateMargins()
{
    CPreviewSetupDlg::UpdateMargins();

    TransferDataFromWindow();

    // take paper size and margin sizes into account when setting up coordinate system
    // so that units are "square" (1 unit x is a wide as 1 unit y is high)
    // (actually, if we don't make it square, on some platforms wx will even resize your
    //  margins to make it so)
    wxSize paperSize = m_PageData.GetPaperSize();  // in millimeters

    // still in millimeters
    float large_side = std::max(paperSize.GetWidth(), paperSize.GetHeight());
    float small_side = std::min(paperSize.GetWidth(), paperSize.GetHeight());

    //float large_side_cm = large_side / 10.0f;  // in centimeters
    float small_side_cm = small_side / 10.0f;  // in centimeters

    wxPoint marginTopLeft = m_PageData.GetMarginTopLeft();
    wxPoint marginBottomRight = m_PageData.GetMarginBottomRight();
    int m_margin_left   = marginTopLeft.x;
    int m_margin_right  = marginBottomRight.x;
    int m_margin_top    = marginTopLeft.y;
    int m_margin_bottom = marginBottomRight.y;

    float m_units_per_cm = 30.0f;

    if (m_PageData.GetPrintData().GetOrientation() == wxPORTRAIT)
    {
        float ratio = float(large_side - m_margin_top  - m_margin_bottom) /
            float(small_side - m_margin_left - m_margin_right);

        m_PrintCoordWidth  = (int)((small_side_cm - m_margin_left/10.f -
            m_margin_right/10.0f)*m_units_per_cm);
        m_PrintCoordHeight = (int)(((float)m_PrintCoordWidth)*ratio);
    }
    else
    {
        float ratio = float(large_side - m_margin_left - m_margin_right) /
            float(small_side - m_margin_top  - m_margin_bottom);

        m_PrintCoordHeight = (int)((small_side_cm - m_margin_top/10.0f -
            m_margin_bottom/10.0f)* m_units_per_cm);
        m_PrintCoordWidth  = (int)(((float)m_PrintCoordHeight)*ratio);
    }

    // If we are printing a single page;
    int image_width = m_PreviewSetupWidget->GetImageWidth();
    int image_height = m_PreviewSetupWidget->GetImageHeight();
    float img_ratio = ((float)image_width)/(float)image_height;
    float paper_ratio = ((float)m_PrintCoordWidth)/(float)m_PrintCoordHeight;

    //float xpos, ypos;
    float xlen, ylen;

    if (m_MultiPageRadioBox->GetSelection() == 0 ||
        m_PageCountSlider->GetValue() == 0) {
       
        m_PreviewSetupWidget->SetRotated(false);

        if (paper_ratio > 1.0f && img_ratio < 1.0f) {
            m_PreviewSetupWidget->SetRotated(true);
            std::swap(image_width, image_height);
            img_ratio = 1.0f/img_ratio;
        }
        else if (paper_ratio < 1.0f && img_ratio > 1.0f) {
            m_PreviewSetupWidget->SetRotated(true);
            std::swap(image_width, image_height);
            img_ratio = 1.0f/img_ratio;
        }

        float wr = ((float)image_width)/(float)m_PrintCoordWidth;
        float hr = ((float)image_height)/(float)m_PrintCoordHeight;

        if (wr > hr) {          
            xlen = (float)m_PrintCoordWidth; // = image_width*(paper_width/image_width) = paper_width
            ylen = ((float)image_height) * (((float)m_PrintCoordWidth)/((float)image_width));

            m_PrintOffsetX = 0;
            m_PrintOffsetY = (int) ((((float)m_PrintCoordHeight) - ylen)/2.0f);

            m_ImageWidthX = m_PrintCoordWidth;
            m_ImageWidthY = (int)ylen;
        }
        else {
            xlen = ((float)image_width) * (((float)m_PrintCoordHeight)/((float)image_height));
            ylen = (float)m_PrintCoordHeight; //= image_height*(paper_height/image_height) = paper_height

            m_PrintOffsetX = (int)((((float)m_PrintCoordWidth) - xlen)/2.0f);
            m_PrintOffsetY = 0;

            m_ImageWidthX = (int)xlen;
            m_ImageWidthY = m_PrintCoordHeight;
        }
    }
    // If we are printing multiple pages
    else { 
    }

}

/*
void CPrintSetupDlg::OnScroll(wxScrollEvent& event)
{
    if (event.GetEventObject() == m_PageCountSlider) {
        int pos = m_PageCountSlider->GetValue();

        m_PageCount->SetLabel(NStr::IntToString(pos));
        m_PageCount->Refresh();

        //int m_PrintCoordWidth;
        //int m_PrintCoordHeight;

        m_PreviewSetupWidget->SetPartitions(3, 3);
    }
}

void CPrintSetupDlg::OnMultiPage( wxCommandEvent& event )
{
    if (m_MultiPageRadioBox->GetSelection() == 0) {
        //disable sizing
        //m_PageCountSlider->SetValue(1);
        m_PageCountSlider->Disable();
        m_PageCountSlider->Refresh();
        //m_PageCount->SetLabel(NStr::IntToString(1));
        m_PageCount->Disable();
        m_PageCount->Refresh();
       
        //m_PreviewSetupWidget->HideBars();
        m_PreviewSetupWidget->Refresh();
    }
    else {
        // enable sizing
        m_PageCountSlider->Enable();
        m_PageCountSlider->Refresh();
        m_PageCount->Enable();
        m_PageCount->Refresh();
        //m_PageCountSlider->Enable();
        //m_PreviewSetupWidget->ShowBars();
        m_PreviewSetupWidget->Refresh();
    }

    UpdateMargins();

}


void CPrintSetupDlg::OnGuides( wxCommandEvent& event )
{
}
*/


void CPrintSetupDlg::OnPrinter(wxCommandEvent& WXUNUSED(event))
{
    // We no longer query GetPrintMode, so we can eliminate the need
    // to call SetPrintMode.
    // This has the limitation that we can't explicitly call the PostScript
    // print setup dialog from the generic Page Setup dialog under Windows,
    // but since this choice would only happen when trying to do PostScript
    // printing under Windows (and only in 16-bit Windows which
    // doesn't have a Windows-specific page setup dialog) it's worth it.

    // First save the current settings, so the wxPrintData object is up to date.
    TransferDataFromWindow();
}


END_NCBI_SCOPE
