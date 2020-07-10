#ifndef GUI_WIDGETS_WX___SAVE_IMAGES_SETUP_DLG_WX__HPP
#define GUI_WIDGETS_WX___SAVE_IMAGES_SETUP_DLG_WX__HPP

/*  $Id: save_images_setup_dlg.hpp 27937 2013-04-26 13:38:43Z falkrb $
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
 * Authors: Bob Falk
 *
 * File Description:
 *   
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <gui/widgets/wx/preview_setup_dlg.hpp>

#include <wx/defs.h>

#include <wx/dialog.h>
#include <wx/cmndata.h>
#include <wx/prntbase.h>
#include <wx/printdlg.h>
#include <wx/listctrl.h>

class  wxTextCtrl;
class  wxButton;
class  wxCheckBox;
class  wxComboBox;
class  wxStaticText;
class  wxRadioBox;
class  wxSlider;
class  wxPageSetupData;

BEGIN_NCBI_SCOPE

class CGlPreviewSetupWidget;
class  IImageGrabber;
class CImage;

class  NCBI_GUIWIDGETS_WX_EXPORT CSaveImagesSetupDlg : public CPreviewSetupDlg
{    
public:
    enum ESaveImagesIDs {
        eDirSelectID = eLastPreviewSetupID + 1,
        eFileRootNameID,
        eImageTypeID,
        eNumberingID,
        eImageSizeID,
        eSizeRatioID
    };

    CSaveImagesSetupDlg(CRef<CImage> img,
                        IImageGrabber* img_saver,
                        wxWindow *parent = NULL);
    virtual ~CSaveImagesSetupDlg();

    virtual bool TransferDataFromWindow();
    virtual bool TransferDataToWindow();

    /// @name wxWidgets events implementation
    /// @{
    void OnPrinter(wxCommandEvent& event);
    void OnDirSelect(wxCommandEvent& event);
    void OnImageTypeChange(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnPreview(wxCommandEvent& event);
    /// Called to update image on screen
    void OnFileTextChange(wxCommandEvent& evt);
    void OnTilePreview(wxCommandEvent& event);
    /// @}

    void UpdateMargins();

    std::string GetDirectoryName() const;
    std::string GetFileRootName() const;
    std::string GetImageFormat() const;
    std::string GetNumberingFormat() const;
    std::string GetOutputFormat() const;

protected:
    virtual void x_Init(wxBoxSizer* mainsizer, int widget_width);

    // Get aspect ratio from s_ImageAspectRatios[] using value from m_SizeRatio.
    // Invert this ratio if output image aspect ratio (w/h) > 1.
    float x_getAspectRatio() const;

    /// Directory in which to save the image
    wxTextCtrl* m_Directory;
    wxButton* m_DirSelectButton;
    /// Part of file name before image number
    wxTextCtrl* m_FileRootName;
    /// Image save format (jpeg, png...)
    wxComboBox* m_ImageFormat;
    /// How the numbers are included in filenames, e.g. img_{1..n} or img_x_y;
    wxComboBox* m_NumberingFormat;
    /// Size ratio derived from ouput format - US Letter, A4, ...
    wxComboBox* m_SizeRatio;
    /// Aspect ratios for image sizes (w/h)
    static const float s_ImageAspectRatios[3];

    /// Output size (this will be the size of larger dimension in output images)
    wxComboBox* m_ImageSizes;

    // Interface to object which renders and saves images
    IImageGrabber* m_ImageSaver;


private:
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS_NO_COPY(CSaveImagesSetupDlg)
};


END_NCBI_SCOPE

#endif //GUI_WIDGETS_WX___SAVE_IMAGES_SETUP_DLG_WX__HPP
