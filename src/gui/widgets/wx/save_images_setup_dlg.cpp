
#include <ncbi_pch.hpp>
#include <gui/widgets/wx/save_images_setup_dlg.hpp>
#include <gui/widgets/wx/preview_setup_widget.hpp>
#include <gui/widgets/wx/image_grabber.hpp>
#include <gui/opengl/glframebuffer.hpp>

#include <gui/objutils/registry.hpp>

#include <util/image/image.hpp>

#include <wx/utils.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/slider.h>
#include <wx/combobox.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/cmndata.h>
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/artprov.h>


#if wxUSE_STATLINE
    #include <wx/statline.h>
#endif

#include <stdlib.h>
#include <string.h>

// ----------------------------------------------------------------------------
// Generic page setup dialog
// ---------------------------------------------------------------------------

BEGIN_NCBI_SCOPE

IMPLEMENT_CLASS(CSaveImagesSetupDlg, CPreviewSetupDlg)
DEFINE_EVENT_TYPE(wxEVT_TILE_PREVIEW)

BEGIN_EVENT_TABLE(CSaveImagesSetupDlg, CPreviewSetupDlg)
    EVT_BUTTON(ePrintIDSetup, CSaveImagesSetupDlg::OnPrinter)
    EVT_BUTTON(eDirSelectID, CSaveImagesSetupDlg::OnDirSelect)
    EVT_BUTTON(wxID_OK, CSaveImagesSetupDlg::OnSave)
    EVT_BUTTON(ePreviewID, CSaveImagesSetupDlg::OnPreview)
    EVT_COMBOBOX(eImageTypeID, CSaveImagesSetupDlg::OnImageTypeChange)
    EVT_TEXT(eFileRootNameID, CSaveImagesSetupDlg::OnFileTextChange)
    EVT_COMBOBOX(eNumberingID, CSaveImagesSetupDlg::OnFileTextChange)
    EVT_COMBOBOX(eSizeRatioID, CSaveImagesSetupDlg::OnFileTextChange)
    EVT_COMMAND(wxID_ANY, wxEVT_TILE_PREVIEW, CSaveImagesSetupDlg::OnTilePreview)
END_EVENT_TABLE()

static const string kSaveImagesBaseKey("GBPlugins.SaveImagesDialog"); 

// Image sizes for smallest, us letter, and a4
const float CSaveImagesSetupDlg::s_ImageAspectRatios[3] =
{ -1.0f, 215.9f/279.4f, 210.0f/297.0f};

CSaveImagesSetupDlg::CSaveImagesSetupDlg(CRef<CImage> img,   
                                         IImageGrabber* img_saver,
                                         wxWindow *parent)
    : CPreviewSetupDlg( img, parent)   
    , m_Directory(NULL)
    , m_DirSelectButton(NULL)
    , m_FileRootName(NULL)
    , m_ImageFormat(NULL)
    , m_NumberingFormat(NULL)
    , m_SizeRatio(NULL)
    , m_ImageSizes(NULL)
    , m_ImageSaver(img_saver)
{
    // Layout all the controls
    x_InitDialog(img);
    SetTitle(wxT("Save Images"));

    // Change names of default ok and cancel buttons to save and close.
    wxWindowList& children = this->GetChildren();

    wxWindowList::compatibility_iterator node = children.GetFirst();
    while (node)
    {
        wxWindow *child = node->GetData();

        if (child->GetId() == GetAffirmativeId()) {
            child->SetLabel(wxT("Save"));
            child->Refresh();
        }
        else if (child->GetId() == wxID_CANCEL) {
            child->SetLabel(wxT("Close")); 
            child->Refresh();
        }
	// Printer shows up on GTK version only...
        else if (child->GetLabel() == wxT("Printer...")) {
            child->Hide();
            child->Refresh();
        }

        node = node->GetNext();
    }
}

CSaveImagesSetupDlg::~CSaveImagesSetupDlg()
{
}

void CSaveImagesSetupDlg::x_Init(wxBoxSizer* mainsizer, int widget_width)
{
    int dir_width = 160;
    int fname_width = 60;

    // Directory and File Choice
    ///////////////////////////////////////////////////////////////////////////

    wxBoxSizer *file_sizer = new wxBoxSizer(wxHORIZONTAL);
    file_sizer->Add(new wxStaticText(this, 
                                     wxID_ANY, 
                                     wxT("Directory:")),
                                     0, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER,5);
    m_Directory = new wxTextCtrl(this, 
                                 wxID_ANY, 
                                 wxEmptyString, 
                                 wxDefaultPosition, 
                                 wxSize(dir_width, wxDefaultCoord));
    file_sizer->Add( m_Directory, 0, wxTOP|wxBOTTOM|wxALIGN_CENTER, 5 );

    m_DirSelectButton = new wxBitmapButton(this,
                                           eDirSelectID,
                                           wxArtProvider::GetBitmap(wxT("menu::open")),
                                           wxDefaultPosition,
                                           wxDefaultSize,
                                           wxBU_AUTODRAW);

    file_sizer->Add( m_DirSelectButton, 0, wxTOP|wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER, 5 );
    
    file_sizer->Add(new wxStaticText(this, 
                                     wxID_ANY, 
                                     wxT("File Name:")),
                                     0,wxTOP|wxBOTTOM|wxALIGN_CENTER,5);   
    m_FileRootName = new wxTextCtrl(this, 
                                    eFileRootNameID, 
                                    wxT("img"), 
                                    wxDefaultPosition, 
                                    wxSize(fname_width, wxDefaultCoord));

    file_sizer->Add(m_FileRootName, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER, 5);
    m_FileRootName->SetValue(wxT("img"));

    file_sizer->Add(new wxStaticText(this, 
                                     wxID_ANY, 
                                     wxT("Numbering:")),
                                     1,wxTOP|wxBOTTOM|wxALIGN_CENTER,5);   

    int numbering_count = 2;
    wxString* num_choices = new wxString[numbering_count]; 
    num_choices[0] = wxT("Sequential");
    num_choices[1] = wxT("Tile Index");

    m_NumberingFormat = new wxComboBox(this,
                                       eNumberingID,
                                       _("Sequential"),
                                       wxDefaultPosition,
                                       wxSize(100, wxDefaultCoord),
                                       numbering_count, num_choices);
    file_sizer->Add(m_NumberingFormat, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER, 5);


    mainsizer->Add( file_sizer, 0, wxALIGN_CENTER);
    wxBoxSizer *image_opt_sizer = new wxBoxSizer(wxHORIZONTAL);

    image_opt_sizer->Add(new wxStaticText(this, 
                                     wxID_ANY, 
                                     wxT("Image Type:")),
                                     1,wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER,5);   

    // gif and tiff don't seem to work on the mac.  Since they won't play
    // nicely with everybody, they can't play at all.
    int format_count = 3;
    wxString* choices = new wxString[format_count]; 
    choices[0] = wxT("jpeg");
    choices[1] = wxT("png");
    //choices[2] = wxT("gif");
    //choices[3] = wxT("tiff");

    m_ImageFormat = new wxComboBox(this,
                                   eImageTypeID,
                                   _("png"),
                                   wxDefaultPosition,
                                   wxSize(80, wxDefaultCoord),
                                   format_count, choices);
    image_opt_sizer->Add(m_ImageFormat, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER, 5);

    // See what image sizes are available and put those into a drop-down
    // box
    std::vector<int> sizes;
    sizes = CGLFrameBuffer::checkFramebufferAvailability(512, 5);
    wxString* size_choices = NULL;
    int default_idx = 0;

    if (sizes.size() > 0) {
        size_choices = new wxString[sizes.size()];
        for (size_t i=0; i<sizes.size(); ++i) {
            char buf[128];
            sprintf(buf,"%5d", sizes[i]);
            size_choices[i] = wxString(buf, wxConvUTF8);
        }
        if (size_choices->size() > 1U)
            default_idx = 1;
    }
    else {
        // This will probably fail when the image is saved, but the user
        // will get a nice-ish message at that time.       
        sizes.push_back(256);
        size_choices = new wxString[sizes.size()];
        char buf[128];
        sprintf(buf,"%5d", sizes[0]);
        size_choices[0] = wxString(buf, wxConvUTF8);        
    }

    image_opt_sizer->Add(new wxStaticText(this, 
                                     wxID_ANY, 
                                     wxT("Image Size:")),
                                     1,wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER,5);  
    m_ImageSizes = new wxComboBox(this,
                                  eImageSizeID,
                                  size_choices[default_idx],
                                  wxDefaultPosition,
                                  wxSize(80, wxDefaultCoord),
                                  (int)sizes.size(), size_choices);
    image_opt_sizer->Add(m_ImageSizes, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER, 5);

    image_opt_sizer->Add(new wxStaticText(this, 
                                     wxID_ANY, 
                                     wxT("Proportions:")),
                                     1,wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER,5);   

    int ratio_count = 3;
    wxString* ratio_choices = new wxString[ratio_count]; 

    /// Should match names in print_media.hpp:
    ratio_choices[0] = wxT("Smallest");   // -1.0f (let class compute best-fit)
    ratio_choices[1] = wxT("Letter");  // 215.9 	279.4 mm
    ratio_choices[2] = wxT("A4");         // 210 × 297  mm

    m_SizeRatio = new wxComboBox(this,
                                   eSizeRatioID,
                                   _("Smallest"),
                                   wxDefaultPosition,
                                   wxSize(100, wxDefaultCoord),
                                   ratio_count, ratio_choices);
    image_opt_sizer->Add(m_SizeRatio, 0,  wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER, 5);
    m_SizeRatio->SetSelection(0);

    mainsizer->Add( image_opt_sizer, 0, wxALIGN_CENTER);

    // Check registry for default values for fields (values are saved between
    // invocations of the dialog)

    CGuiRegistry& reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = reg.GetReadView(kSaveImagesBaseKey);
    
    std::string reg_value;

    reg_value = view.GetString("ImageDirectory");
    if (!reg_value.empty())
        m_Directory->SetValue(wxString(reg_value.c_str(), wxConvUTF8));

    reg_value = view.GetString("FileRootName");
    if (!reg_value.empty())
        m_FileRootName->SetValue(wxString(reg_value.c_str(), wxConvUTF8));

    reg_value = view.GetString("ImageFormat");
    if (!reg_value.empty())
        m_ImageFormat->SetValue(wxString(reg_value.c_str(), wxConvUTF8));


    reg_value = view.GetString("NumberingFormat");
    if (!reg_value.empty())
        m_NumberingFormat->SetValue(wxString(reg_value.c_str(), wxConvUTF8));

    int sl = m_SizeRatio->GetCurrentSelection();
    reg_value = view.GetString("SizeRatio");
    wxString wx_reg_value = wxString(reg_value.c_str(), wxConvUTF8);
    if (!reg_value.empty())
        m_SizeRatio->SetValue(wx_reg_value);
    for (int j=0; j<ratio_count; ++j) {
        if (wx_reg_value == ratio_choices[j]) {
            m_SizeRatio->SetSelection(j);
            break;
        }
    }
    sl = m_SizeRatio->GetCurrentSelection();


    // Initialize value in image widget
    wxCommandEvent dummy_evt;

    OnFileTextChange(dummy_evt);
    OnImageTypeChange(dummy_evt);
}

float CSaveImagesSetupDlg::x_getAspectRatio() const
{
    float ratio = s_ImageAspectRatios[m_SizeRatio->GetCurrentSelection()];

    if (ratio == -1.0f) return ratio;

    float target_ratio = 
        float(m_PreviewSetupWidget->GetImageWidth())/
        float(m_PreviewSetupWidget->GetImageHeight());
  
    if ((ratio < 1.0f && target_ratio > 1.0f) || (ratio > 1.0f && target_ratio < 1.0f))
        return 1.0f/ratio;

    return ratio;
}


bool CSaveImagesSetupDlg::TransferDataToWindow()
{
    CPreviewSetupDlg::TransferDataToWindow();

    return true;
}

bool CSaveImagesSetupDlg::TransferDataFromWindow()
{
    CPreviewSetupDlg::TransferDataFromWindow();

    return true;
}

void CSaveImagesSetupDlg::UpdateMargins()
{
    CPreviewSetupDlg::UpdateMargins();

    TransferDataFromWindow();
}

void CSaveImagesSetupDlg::OnDirSelect(wxCommandEvent& event)
{
    wxDirDialog dlg(this);

    if (dlg.ShowModal() == wxID_OK) {
        m_Directory->SetValue(dlg.GetPath());

        // After changing names, if any images were saved, flag them
        // as not being saved anymore.
        m_PreviewSetupWidget->RefreshImage(-1,-1);
    }
}

void CSaveImagesSetupDlg::OnFileTextChange(wxCommandEvent& evt)
{
    // Check that dialog is constructed first. This function is used
    // to update the preview widget regardless of what (related) data
    // item changed.
    if (NULL != m_FileRootName &&
        NULL != m_SizeRatio &&
        NULL != m_NumberingFormat &&
        NULL != m_PreviewSetupWidget) {
        wxString name = m_FileRootName->GetValue();

        std::string str = (const char*)name.mb_str(wxConvUTF8);

        m_PreviewSetupWidget->SetImageBaseName(str);   
        m_PreviewSetupWidget->SetAspectRatio(x_getAspectRatio());         

        if (GetNumberingFormat() == "Sequential")
            m_PreviewSetupWidget->SetNumberingFormat(IImageGrabber::eSequentialFormat);
        else
            m_PreviewSetupWidget->SetNumberingFormat(IImageGrabber::eCartesionFormat);

        // Call set partitions too, since that can change if the size-ratio changes
        // (the number of partions in x and y may change, not the total number)
        int pos = m_PageCountSlider->GetValue();        
        m_PreviewSetupWidget->SetPartitions(pos);

        // After changing names, if any images were saved, flag them
        // as not being saved anymore.
        m_PreviewSetupWidget->RefreshImage(-1,-1);
    }
}

void CSaveImagesSetupDlg::OnImageTypeChange(wxCommandEvent& event)
{
    // After changing image type, if any images were saved, flag them
    // as not being saved anymore (since images to save now have
    // a different name).
    m_PreviewSetupWidget->RefreshImage(-1,-1);

    if (NULL != m_ImageFormat) {
        string image_type = std::string( (const char*)m_ImageFormat->GetValue().mb_str(wxConvUTF8));

        if (NULL != m_NumberingFormat)
            m_NumberingFormat->Enable();
        if (NULL != m_SizeRatio)
            m_SizeRatio->Enable();
        if (NULL != m_PageCountSlider)
            m_PageCountSlider->Enable();
        if (NULL != m_PageCount)
            m_PageCount->Enable();
        if (NULL != m_ImageSizes)
            m_ImageSizes->Enable();
    }
}


void CSaveImagesSetupDlg::OnPrinter(wxCommandEvent& WXUNUSED(event))
{
    TransferDataFromWindow();
}

void CSaveImagesSetupDlg::OnSave(wxCommandEvent& event)
{
    TransferDataFromWindow();

    // Set cursor to hourglass cursor since user can't do anything until save
    // is over
    SetCursor(*wxHOURGLASS_CURSOR);

    // Save values so if user brings dialog up again, they can see
    // previously selected values.
    CGuiRegistry& reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = reg.GetWriteView(kSaveImagesBaseKey);
    view.Set("ImageDirectory", GetDirectoryName());
    view.Set("FileRootName", GetFileRootName());
    view.Set("ImageFormat", GetImageFormat());
    view.Set("NumberingFormat", GetNumberingFormat());
    view.Set("SizeRatio", GetOutputFormat());


    IImageGrabber::eCaptureResult res;

    m_ImageSaver->SetPartitions(GetPartitions());
    m_ImageSaver->SetOutputInfo(GetDirectoryName(),
        GetFileRootName(),
        GetImageFormat());

    if (GetNumberingFormat() == "Sequential")
        m_ImageSaver->SetNumberingFormat(IImageGrabber::eSequentialFormat);
    else
        m_ImageSaver->SetNumberingFormat(IImageGrabber::eCartesionFormat);

    m_ImageSaver->SetTileAspectRatio(x_getAspectRatio());
    m_ImageSaver->SetImageAspectRatio(((float)m_PreviewSetupWidget->GetImageWidth())/
        (float)m_PreviewSetupWidget->GetImageHeight());
    if (m_GuidesRadioBox->GetSelection() == 0)
        m_ImageSaver->SetPrintingGuidesEnabled(false);
    else
        m_ImageSaver->SetPrintingGuidesEnabled(true);        

    long img_size = 0;
    m_ImageSizes->GetValue().ToLong(&img_size);

    // Force preview widget to be current context, since the rendering to the background will
    // be interspersed with rendering to widget to show progress.
    m_PreviewSetupWidget->Update();

    res = m_ImageSaver->GrabImages(img_size, m_PreviewSetupWidget);


    // Set the cursor to its value prior to the save:
    SetCursor(*wxSTANDARD_CURSOR);

    if (res == IImageGrabber::eOpenGLError) {
        wxMessageBox(wxT("Graphics system does not support in-memory image save feature"), 
                     wxT("Error Saving"), wxOK);
    }
    else if (res == IImageGrabber::eFileError) {
        wxMessageBox(wxT("File error occured during save. Make sure file and directory names are valid."), 
                     wxT("Error Saving"), wxOK);
    }
}


void CSaveImagesSetupDlg::OnTilePreview(wxCommandEvent& event)
{
    TransferDataFromWindow();

    if (m_ImageSaver == NULL)
        return;

    // Set cursor to hourglass cursor since user can't do anything until save
    // is over
    SetCursor(*wxHOURGLASS_CURSOR);

    m_ImageSaver->SetPartitions(GetPartitions());
    m_ImageSaver->SetOutputInfo(GetDirectoryName(),
                                GetFileRootName(),
                                GetImageFormat());
    
    if (GetNumberingFormat() == "Sequential")
        m_ImageSaver->SetNumberingFormat(IImageGrabber::eSequentialFormat);
    else
        m_ImageSaver->SetNumberingFormat(IImageGrabber::eCartesionFormat);

    m_ImageSaver->SetTileAspectRatio(x_getAspectRatio());
    m_ImageSaver->SetImageAspectRatio(((float)m_PreviewSetupWidget->GetImageWidth())/
                                       (float)m_PreviewSetupWidget->GetImageHeight());
    if (m_GuidesRadioBox->GetSelection() == 0)
        m_ImageSaver->SetPrintingGuidesEnabled(false);
    else
        m_ImageSaver->SetPrintingGuidesEnabled(true);

    long img_size = 0;
    m_ImageSizes->GetValue().ToLong(&img_size);

    // Force preview widget to be current context, since the rendering to the background will
    // be interspersed with rendering to widget to show progress.
    m_PreviewSetupWidget->Update();

    // x and y tile coordinates were stored in int/long values of event
    m_ImageSaver->GrabImage(img_size, 
        CVect2<int>(event.GetInt(), event.GetExtraLong()), 
        m_PreviewSetupWidget);

    // Set the cursor to its value prior to the save:
    SetCursor(*wxSTANDARD_CURSOR);
}

void CSaveImagesSetupDlg::OnPreview(wxCommandEvent& event)
{
    TransferDataFromWindow();

    if (m_ImageSaver == NULL)
        return;

    // Set cursor to hourglass cursor since user can't do anything until save
    // is over
    SetCursor(*wxHOURGLASS_CURSOR);

    m_ImageSaver->SetPartitions(GetPartitions());
    m_ImageSaver->SetOutputInfo(GetDirectoryName(),
                                GetFileRootName(),
                                GetImageFormat());
    
    if (GetNumberingFormat() == "Sequential")
        m_ImageSaver->SetNumberingFormat(IImageGrabber::eSequentialFormat);
    else
        m_ImageSaver->SetNumberingFormat(IImageGrabber::eCartesionFormat);

    m_ImageSaver->SetTileAspectRatio(x_getAspectRatio());
    m_ImageSaver->SetImageAspectRatio(((float)m_PreviewSetupWidget->GetImageWidth())/
                                       (float)m_PreviewSetupWidget->GetImageHeight());
    if (m_GuidesRadioBox->GetSelection() == 0)
        m_ImageSaver->SetPrintingGuidesEnabled(false);
    else
        m_ImageSaver->SetPrintingGuidesEnabled(true);

    long img_size = 0;
    m_ImageSizes->GetValue().ToLong(&img_size);

    // Force preview widget to be current context, since the rendering to the background will
    // be interspersed with rendering to widget to show progress.
    m_PreviewSetupWidget->Update();

    m_ImageSaver->PreviewImages(img_size, 
                                m_PreviewSetupWidget->GetSize().GetWidth(),
                                m_PreviewSetupWidget->GetSize().GetHeight(),
                                m_PreviewSetupWidget);

    // Set the cursor to its value prior to the save:
    SetCursor(*wxSTANDARD_CURSOR);
}


std::string CSaveImagesSetupDlg::GetDirectoryName() const
{
    if (NULL != m_Directory)
        return std::string( (const char*)m_Directory->GetValue().mb_str(wxConvUTF8));
    else return std::string("");
}

std::string CSaveImagesSetupDlg::GetFileRootName() const
{
    if (NULL != m_FileRootName)
        return std::string( (const char*)m_FileRootName->GetValue().mb_str(wxConvUTF8));
    else return std::string("");
}

std::string CSaveImagesSetupDlg::GetImageFormat() const
{
    if (NULL != m_ImageFormat)
        return std::string( (const char*)m_ImageFormat->GetValue().mb_str(wxConvUTF8));
    else return std::string("");
}

std::string CSaveImagesSetupDlg::GetNumberingFormat() const
{
    if (NULL != m_NumberingFormat)
        return std::string( (const char*)m_NumberingFormat->GetValue().mb_str(wxConvUTF8));
    else return std::string("");
}

std::string CSaveImagesSetupDlg::GetOutputFormat() const
{
    if (NULL != m_SizeRatio)
        return std::string( (const char*)m_SizeRatio->GetValue().mb_str(wxConvUTF8));
    else return std::string("");
}




END_NCBI_SCOPE
