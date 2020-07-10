
#include <ncbi_pch.hpp>

#include <gui/widgets/wx/preview_setup_dlg.hpp>

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

// ----------------------------------------------------------------------------
// global vars
// ----------------------------------------------------------------------------

extern WXDLLEXPORT_DATA(wxPrintPaperDatabase*) wxThePrintPaperDatabase;

// ----------------------------------------------------------------------------
// Generic page setup dialog
// ----------------------------------------------------------------------------

BEGIN_NCBI_SCOPE

IMPLEMENT_CLASS(CPreviewSetupDlg, wxPageSetupDialogBase)

BEGIN_EVENT_TABLE(CPreviewSetupDlg, wxPageSetupDialogBase)
    EVT_BUTTON(ePrintIDSetup, CPreviewSetupDlg::OnPrinter)
    EVT_SCROLL(CPreviewSetupDlg::OnScroll)
    EVT_RADIOBOX(eMultiPageID, CPreviewSetupDlg::OnMultiPage)
    EVT_RADIOBOX(eGuidesID, CPreviewSetupDlg::OnGuides)
END_EVENT_TABLE()

CPreviewSetupDlg::CPreviewSetupDlg(CRef<CImage> img,
                                   bool add_preview_button,
                                   wxWindow *parent)
    : wxPageSetupDialogBase( parent,
                wxID_ANY,
                wxT("Page Setup"),
                wxPoint(0,0),
                wxSize(600, 600),
                wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL )
    , m_PreviewButton(add_preview_button)
{
}


CPreviewSetupDlg::~CPreviewSetupDlg()
{
}


bool CPreviewSetupDlg::TransferDataToWindow()
{
    return true;
}

bool CPreviewSetupDlg::TransferDataFromWindow()
{
    return true;
}

void CPreviewSetupDlg::SetZoomBehavior(bool zoomx, bool zoomy) 
{ 
    m_PreviewSetupWidget->SetZoomBehavior(zoomx, zoomy);
}

bool CPreviewSetupDlg::GetZoomX() const
{ 
    return m_PreviewSetupWidget->GetZoomX();
}

bool CPreviewSetupDlg::GetZoomY() const
{ 
    return m_PreviewSetupWidget->GetZoomY();
}

void CPreviewSetupDlg::UpdateMargins()
{
    TransferDataFromWindow();
}

void CPreviewSetupDlg::OnScroll(wxScrollEvent& event)
{
    if (event.GetEventObject() == m_PageCountSlider) {
        int pos = m_PageCountSlider->GetValue();

        m_PageCount->SetLabel(wxString(NStr::IntToString(pos).c_str(), wxConvUTF8));
        m_PageCount->Refresh();

        m_PreviewSetupWidget->SetPartitions(pos);
        m_PreviewSetupWidget->Refresh();
    }
}

// Do we need this - or is the slider alone sufficient? (slider at 0 => no
// multi-page).
void CPreviewSetupDlg::OnMultiPage( wxCommandEvent& event )
{
    if (m_MultiPageRadioBox->GetSelection() == 0) {
        //disable sizing
        //m_PageCountSlider->SetValue(1);
        m_PageCountSlider->Disable();
        m_PageCountSlider->Refresh();
        //m_PageCount->SetLabel(NStr::IntToString(1));
        m_PageCount->Disable();
        m_PageCount->Refresh();
              
        m_PreviewSetupWidget->SetPartitions(1);
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
        m_PreviewSetupWidget->SetPartitions(m_PageCountSlider->GetValue());
        m_PreviewSetupWidget->Refresh();
    }

    UpdateMargins();

}

void CPreviewSetupDlg::OnGuides( wxCommandEvent& event )
{
}


void CPreviewSetupDlg::OnPrinter(wxCommandEvent& WXUNUSED(event))
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


void CPreviewSetupDlg::x_InitDialog(CRef<CImage> img) 
{
    wxBoxSizer *mainsizer = new wxBoxSizer( wxVERTICAL );

    // 1. Image (top)
    ///////////////////////////////////////////////////////////////////////////
    int widget_width = 520;

    float size_ratio = ((float)img->GetHeight())/(float)img->GetWidth();

    int setup_widget_width = widget_width;
    if (size_ratio > 1.4f) {
        setup_widget_width = (int) ((((float)widget_width)*1.4f)/size_ratio);
    }

    // Add image widget to dialog.  Add spacers on the sides of the image
    // in case the image is taller than wide.
    wxBoxSizer *setup_widget_sizer = new wxBoxSizer( wxHORIZONTAL );
    if (setup_widget_width < widget_width)
        setup_widget_sizer->Add((widget_width-setup_widget_width)/2, 
                                 1, 0, wxEXPAND);

    int widget_height = (int) (((float)setup_widget_width)*size_ratio);
    m_PreviewSetupWidget = new CGlPreviewSetupWidget(img,
                                                     this, 
                                                     ePreviewSetupWidget, 
                                                     wxDefaultPosition,
                                                     wxSize(setup_widget_width,
                                                            widget_height) );
    m_PreviewSetupWidget->SetName(wxT("PreviewSetupWidget"));
    setup_widget_sizer->Add(m_PreviewSetupWidget, 0, wxALL|wxEXPAND, 5);

    if (setup_widget_width < widget_width)
        setup_widget_sizer->Add((widget_width-setup_widget_width)/2, 1, 0, wxEXPAND);
    mainsizer->Add(setup_widget_sizer, 1, wxALL|wxEXPAND, 5);

    // 2. Slider for paper page count
    /////////////////////////////////////////////////////////////////////////// 
    wxStaticBoxSizer *page_count_box = new wxStaticBoxSizer(
        new wxStaticBox(this, ePrintIDStatic, wxT("")), 
        wxHORIZONTAL );

    wxString *choices4 = new wxString[2];
    choices4[0] = wxT("Hide");
    choices4[1] = wxT("Display");
    m_GuidesRadioBox = new wxRadioBox(this, eGuidesID, wxT("Printing Guides"),
        wxDefaultPosition, wxDefaultSize, 2, choices4, 2);
    m_GuidesRadioBox->SetSelection(0);
    page_count_box->Add(m_GuidesRadioBox, 0, wxALL | wxEXPAND, 5 );
    delete [] choices4;

    page_count_box->Add( new wxStaticText(this, ePrintIDStatic, wxT("Partitions:")),0,wxALL|wxALIGN_LEFT|wxALIGN_CENTER,5 );

    m_PageCountSlider = new wxSlider(this, 
                                     ePageCountSliderID, 
                                     1, 
                                     1, 
                                     150, 
                                     wxDefaultPosition, 
                                     wxSize(160, wxDefaultCoord),
                                     wxHORIZONTAL|wxSL_AUTOTICKS);
    page_count_box->Add(m_PageCountSlider, 1, wxALL|wxALIGN_CENTER, 5);

    m_PageCount = new wxStaticText(this, 
                                   ePageCountID, 
                                   wxT("1"), 
                                   wxDefaultPosition, 
                                   wxSize(15, wxDefaultCoord));
    page_count_box->Add(m_PageCount, 0, wxALL|wxALIGN_CENTER, 5);   

    mainsizer->Add(page_count_box, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER, 5);    

    // 4. Subclass widgets
    ///////////////////////////////////////////////////////////////////////////
    x_Init(mainsizer, widget_width);

#if wxUSE_STATLINE
    // 5.  static line
    ///////////////////////////////////////////////////////////////////////////
    mainsizer->Add( new wxStaticLine( this, 
                                      wxID_ANY, 
                                      wxDefaultPosition, 
                                      wxSize(widget_width, wxDefaultCoord) ), 
                    0, wxEXPAND | wxLEFT|wxRIGHT|wxTOP, 10 );
#endif

    // 6. buttons
    ///////////////////////////////////////////////////////////////////////////
    wxSizer* buttonsizer = CreateButtonSizer( wxOK|wxCANCEL);

    if (m_PreviewButton) {
        wxButton* m_PreviewButton = new wxButton(this, ePreviewID, wxT("Preview") );
        buttonsizer->Add( m_PreviewButton, 0, wxLEFT|wxRIGHT|wxALIGN_CENTER, 10 );
    }

    if (wxPrintFactory::GetFactory()->HasPrintSetupDialog())
    {
        m_PrinterButton = new wxButton(this, ePrintIDSetup, wxT("Printer...") );
        buttonsizer->Add( m_PrinterButton, 0, wxLEFT|wxRIGHT|wxALIGN_CENTER, 10 );
    }
    else
    {
        m_PrinterButton = NULL;
    }

    //  if (m_PrintData.GetEnableHelp())
    //  wxButton *helpButton = new wxButton(this, (wxFunction)wxGenericPageSetupHelpProc, wxT("Help"), wxDefaultCoord, wxDefaultCoord, buttonWidth, buttonHeight);
    mainsizer->Add( buttonsizer, 0, wxEXPAND|wxALL, 10 );

    SetAutoLayout( true );
    SetSizer( mainsizer );

    mainsizer->Fit( this );
    Centre(wxBOTH);

    InitDialog();
}

CVect2<int> CPreviewSetupDlg::GetPartitions() const 
{ 
    if (m_PreviewSetupWidget != NULL)
        return m_PreviewSetupWidget->GetPartitions(); 
    else return CVect2<int>(0,0);
}


END_NCBI_SCOPE
