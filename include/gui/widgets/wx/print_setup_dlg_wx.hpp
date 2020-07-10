#ifndef GUI_WIDGETS_WX___PRINT_SETUP_DLG_WX__HPP
#define GUI_WIDGETS_WX___PRINT_SETUP_DLG_WX__HPP

/*  $Id: print_setup_dlg_wx.hpp 25836 2012-05-21 13:52:56Z falkrb $
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
class CImage;

class  NCBI_GUIWIDGETS_WX_EXPORT CPrintSetupDlg : public CPreviewSetupDlg
{       
public:
    CPrintSetupDlg(CRef<CImage> img,
                   wxWindow *parent = NULL,
                   wxPageSetupDialogData* data = NULL);
    virtual ~CPrintSetupDlg();

    virtual bool TransferDataFromWindow();
    virtual bool TransferDataToWindow();

    virtual wxPageSetupDialogData& GetPageSetupDialogData();

    void OnPrinter(wxCommandEvent& event);

    int GetDrawAreaWidth() const { return m_PrintCoordWidth; }
    int GetDrawAreaHeight() const { return m_PrintCoordHeight; }

    int GetTargetImageWidth() const { return m_ImageWidthX; }
    int GetTargetImageHeight() const { return m_ImageWidthY; }

    int GetPrintOffsetX() const { return m_PrintOffsetX; }
    int GetPrintOffsetY() const { return m_PrintOffsetY; }

    wxComboBox *CreatePaperTypeChoice(int* x, int* y);
    void UpdateMargins();

protected:
    virtual void x_Init(wxBoxSizer* mainsizer, int widget_width);

    wxRadioBox*     m_OrientationRadioBox;
    wxTextCtrl*     m_MarginLeftText;
    wxTextCtrl*     m_MarginTopText;
    wxTextCtrl*     m_MarginRightText;
    wxTextCtrl*     m_MarginBottomText;
    wxComboBox*     m_PaperTypeChoice;

    wxPageSetupDialogData m_PageData;

    int m_PrintCoordWidth;
    int m_PrintCoordHeight;
    int m_ImageWidthX;
    int m_ImageWidthY;
    int m_PrintOffsetX;
    int m_PrintOffsetY;

private:
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS_NO_COPY(CPrintSetupDlg)
};


END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX___PRINT_SETUP_DLG_WX__HPP
