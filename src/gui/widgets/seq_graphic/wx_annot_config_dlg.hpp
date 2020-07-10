#ifndef GUI_WIDGETS_SEQ_GRAPHIC___WX_ANNOT_CONFIG_DLG__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___WX_ANNOT_CONFIG_DLG__HPP

/*  $Id: wx_annot_config_dlg.hpp 39658 2017-10-24 20:52:55Z katargir $
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
 * Authors:  Vlad Lebedev
 *
 * File Description:
 *
 */



/*!
 * Includes
 */

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/clrpicker.h>
#include <wx/checkbox.h>
#include <wx/listbox.h>
#include <wx/statbox.h>
#include <wx/stattext.h>


#include <gui/widgets/seq_graphic/seqgraphic_conf.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/annot_selector.hpp>



////@begin includes
#include "wx/clrpicker.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

/*!
 * Control identifiers
 */

BEGIN_NCBI_SCOPE

////@begin control identifiers
#define ID_CWXANNOTCONFIGDLG 10013
#define ID_LISTBOX1 10001
#define ID_CHECKBOX1 10002
#define ID_CHECKBOX2 10010
#define ID_COLOURPICKERCTRL 10011
#define SYMBOL_CWXANNOTCONFIGDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWXANNOTCONFIGDLG_TITLE _("Annotation Rendering Options")
#define SYMBOL_CWXANNOTCONFIGDLG_IDNAME ID_CWXANNOTCONFIGDLG
#define SYMBOL_CWXANNOTCONFIGDLG_SIZE wxDefaultSize
#define SYMBOL_CWXANNOTCONFIGDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CwxAnnotConfigDlg class declaration
 */

class CwxAnnotConfigDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CwxAnnotConfigDlg )
    DECLARE_EVENT_TABLE()

public:
    CwxAnnotConfigDlg();
    CwxAnnotConfigDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWXANNOTCONFIGDLG_IDNAME, const wxString& caption = SYMBOL_CWXANNOTCONFIGDLG_TITLE, const wxPoint& pos = SYMBOL_CWXANNOTCONFIGDLG_POSITION, const wxSize& size = SYMBOL_CWXANNOTCONFIGDLG_SIZE, long style = SYMBOL_CWXANNOTCONFIGDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWXANNOTCONFIGDLG_IDNAME, const wxString& caption = SYMBOL_CWXANNOTCONFIGDLG_TITLE, const wxPoint& pos = SYMBOL_CWXANNOTCONFIGDLG_POSITION, const wxSize& size = SYMBOL_CWXANNOTCONFIGDLG_SIZE, long style = SYMBOL_CWXANNOTCONFIGDLG_STYLE );

    ~CwxAnnotConfigDlg();

    void Init();

    void CreateControls();

////@begin CwxAnnotConfigDlg event handler declarations

    void OnAnnotBrowserSelected( wxCommandEvent& event );

    void OnSmearAlnClicked( wxCommandEvent& event );

    void OnOkClick( wxCommandEvent& event );

////@end CwxAnnotConfigDlg event handler declarations

////@begin CwxAnnotConfigDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CwxAnnotConfigDlg member function declarations

    static bool ShowToolTips();

    void SetBioseq(const objects::CBioseq_Handle& bsh);

protected:
    typedef map<string, CRef<objects::CSeq_annot> >  TAnnotMap;
    void x_SaveWidgetValues();
    void x_LoadWidgetValues();
    void x_InitAnnotList(const objects::CBioseq_Handle& bsh);
    CRef<objects::CSeq_annot> x_FindAnnot(const string& label);
    bool x_GetSelectedAnnotName(string& name);
    void x_CheckControls();
    void x_AnnotBrowserSelectionChanged();
////@begin CwxAnnotConfigDlg member variables
    wxListBox* m_AnnotBrowser;
    wxCheckBox* m_SmearAln;
    wxCheckBox* m_ShowLabel;
    wxBoxSizer* m_AnnotColorPicker;
private:
    CRgbaColor m_AnnotBgColor;
    CAnnotWriteConfig m_AnnotConfig;
    TAnnotMap m_LabelAnnots;
////@end CwxAnnotConfigDlg member variables
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___WX_ANNOT_CONFIG_DLG__HPP
