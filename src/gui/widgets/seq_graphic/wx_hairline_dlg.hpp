#ifndef GUI_WIDGETS_SEQ_GRAPHIC___WX_HAIRLINE_DLG__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___WX_HAIRLINE_DLG__HPP

/*  $Id: wx_hairline_dlg.hpp 25250 2012-02-14 18:58:17Z wuliangs $
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

#include <gui/widgets/seq_graphic/seqgraphic_conf.hpp>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/clrpicker.h>
#include <wx/radiobox.h>
#include <wx/stattext.h>

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations
class wxCheckBox;


BEGIN_NCBI_SCOPE


////@begin control identifiers
#define ID_CWXHAIRLINEDLG 10000
#define ID_RADIOBOX1 10007
#define ID_CHECKBOX 10038
#define ID_COLOURPICKERCTRL 10001
#define ID_COLOURPICKERCTRL1 10009
#define ID_COLOURPICKERCTRL2 10008
#define SYMBOL_CWXHAIRLINEDLG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWXHAIRLINEDLG_TITLE _("Interval Rendering Options")
#define SYMBOL_CWXHAIRLINEDLG_IDNAME ID_CWXHAIRLINEDLG
#define SYMBOL_CWXHAIRLINEDLG_SIZE wxSize(-1, 280)
#define SYMBOL_CWXHAIRLINEDLG_POSITION wxDefaultPosition
////@end control identifiers


class CwxHairlineDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CwxHairlineDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CwxHairlineDlg();
    CwxHairlineDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWXHAIRLINEDLG_IDNAME, const wxString& caption = SYMBOL_CWXHAIRLINEDLG_TITLE, const wxPoint& pos = SYMBOL_CWXHAIRLINEDLG_POSITION, const wxSize& size = SYMBOL_CWXHAIRLINEDLG_SIZE, long style = SYMBOL_CWXHAIRLINEDLG_STYLE );

    /// Creation
    bool CreateX( wxWindow* parent, wxWindowID id = SYMBOL_CWXHAIRLINEDLG_IDNAME, const wxString& caption = SYMBOL_CWXHAIRLINEDLG_TITLE, const wxPoint& pos = SYMBOL_CWXHAIRLINEDLG_POSITION, const wxSize& size = SYMBOL_CWXHAIRLINEDLG_SIZE, long style = SYMBOL_CWXHAIRLINEDLG_STYLE );

    /// Destructor
    ~CwxHairlineDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CwxHairlineDlg event handler declarations

    void OnCheckboxClick( wxCommandEvent& event );

    void OnOkClick( wxCommandEvent& event );

////@end CwxHairlineDlg event handler declarations

////@begin CwxHairlineDlg member function declarations

    CRgbaColor GetDownward() const { return m_Downward ; }
    void SetDownward(CRgbaColor value) { m_Downward = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CwxHairlineDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    void SetConfig(CRef<CSeqGraphicConfig> cfg);
protected:
////@begin CwxHairlineDlg member variables
    wxRadioBox* m_RadioGroup;
    wxCheckBox* m_ShowDownward;
    wxBoxSizer* m_DownColorGroup;
    CRef<CSeqGraphicConfig> m_Cfg;
protected:
    CRgbaColor m_UnShared;
    CRgbaColor m_Shared;
    CRgbaColor m_Downward;
////@end CwxHairlineDlg member variables
};


END_NCBI_SCOPE

#endif //GUI_WIDGETS_SEQ_GRAPHIC___WX_HAIRLINE_DLG__HPP
