#ifndef GUI_WIDGETS_WX___PREVIEW_SETUP_DLG_WX__HPP
#define GUI_WIDGETS_WX___PREVIEW_SETUP_DLG_WX__HPP

/*  $Id: preview_setup_dlg.hpp 33050 2015-05-19 16:06:18Z falkrb $
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
 * Authors: 
 *
 * File Description:
 *   
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <gui/utils/vect2.hpp>

#include <util/image/image.hpp>

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
class  wxBoxSizer;

BEGIN_NCBI_SCOPE

class CGlPreviewSetupWidget;
class CImage;

class  NCBI_GUIWIDGETS_WX_EXPORT CPreviewSetupDlg : public wxPageSetupDialogBase
{
public:
    enum EPreviewSetupIDs {
        ePageCountSliderID = wxID_HIGHEST + 100,
        ePreviewSetupWidget,
        eMultiPageID,
        eGuidesID,
        ePageCountID,
        ePreviewID,

        ePrintIDStatic,
        ePrintIDRange,
        ePrintIDFrom,
        ePrintIDTo,
        ePrintIDCopies,
        ePrintIDPrintToFile,
        ePrintIDSetup,

        ePrintIDLeftMargin,
        ePrintIDRightMargin,
        ePrintIDTopMargin,
        ePrintIDBottomMargin,

        ePrintIDPrintColor,
        ePrintIDOrientation,
        ePrintIDCommand,
        ePrintIDOptions,
        ePrintIDPaperSize,
        ePrintIDPrint,        

        eLastPreviewSetupID
    };
        
public:
    CPreviewSetupDlg(CRef<CImage> img, 
                     bool add_preview_button,
                     wxWindow *parent = NULL);

    virtual ~CPreviewSetupDlg();

    virtual bool TransferDataFromWindow();
    virtual bool TransferDataToWindow();

    /// Enable/disable zoom in x && y
    void SetZoomBehavior(bool zoomx, bool zoomy);
    bool GetZoomX() const;
    bool GetZoomY() const;

    /// @name wxWidgets events implementation
    /// @{
    virtual void OnPrinter(wxCommandEvent& event);
    virtual void OnScroll(wxScrollEvent& event);
    virtual void OnMultiPage(wxCommandEvent& event);
    virtual void OnGuides(wxCommandEvent& event);
    /// @}

    //int GetTargetImageWidth() const { return m_ImageWidthX; }
    //int GetTargetImageHeight() const { return m_ImageWidthY; }

    virtual void UpdateMargins();

    /// Required as a subclass of wxPageSetupDialogBase.  Maybe I shouldn't derive
    /// from that since I also want to have a save-images subclass...
    virtual wxPageSetupDialogData& GetPageSetupDialogData() { return m_psdd; }

    /// Get the number of image partitions in x and y
    CVect2<int> GetPartitions() const;

protected:

    wxPageSetupDialogData m_psdd;
    /// Layout all controls owned by this class
    void x_InitDialog(CRef<CImage> img);

    /// Provide virtual hook for subclasses to add their own controls below
    /// the main controls.
    virtual void x_Init(wxBoxSizer* /*mainsizer*/, int /*widget_width*/) {}

    wxButton*       m_PrinterButton;
    wxRadioBox*     m_MultiPageRadioBox;
    wxRadioBox*     m_GuidesRadioBox;
    wxSlider*       m_PageCountSlider;
    wxStaticText*   m_PageCount;

    CGlPreviewSetupWidget*  m_PreviewSetupWidget;

    bool            m_PreviewButton;

private:
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS_NO_COPY(CPreviewSetupDlg)
};


END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX___PREVIEW_SETUP_DLG_WX__HPP

