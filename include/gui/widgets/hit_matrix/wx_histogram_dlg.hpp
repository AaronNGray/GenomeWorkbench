#ifndef GUI_WIDGETS_HIT_MATRIX___WX_HISTOGRAM_DLG__HPP
#define GUI_WIDGETS_HIT_MATRIX___WX_HISTOGRAM_DLG__HPP

/*  $Id: wx_histogram_dlg.hpp 25806 2012-05-11 15:55:59Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <gui/widgets/hit_matrix/hit_matrix_pane.hpp>

#include <wx/dialog.h>


////@begin forward declarations
class wxColourPickerCtrl;
////@end forward declarations

class wxColourPickerEvent;
class wxCheckListBox;
class wxStaticText;

BEGIN_NCBI_SCOPE


////@begin control identifiers
#define ID_CWXHISTOGRAMDLG 10000
#define ID_SUBJECT_LIST 10004
#define ID_QUERY_LIST 10005
#define ID_COLOURPICKERCTRL1 10001
#define SYMBOL_CWXHISTOGRAMDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWXHISTOGRAMDLG_TITLE _("Setup Dot Matrix Graphs")
#define SYMBOL_CWXHISTOGRAMDLG_IDNAME ID_CWXHISTOGRAMDLG
#define SYMBOL_CWXHISTOGRAMDLG_SIZE wxDefaultSize
#define SYMBOL_CWXHISTOGRAMDLG_POSITION wxDefaultPosition
////@end control identifiers


class CwxHistogramDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CwxHistogramDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CwxHistogramDlg();
    CwxHistogramDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWXHISTOGRAMDLG_IDNAME, const wxString& caption = SYMBOL_CWXHISTOGRAMDLG_TITLE, const wxPoint& pos = SYMBOL_CWXHISTOGRAMDLG_POSITION, const wxSize& size = SYMBOL_CWXHISTOGRAMDLG_SIZE, long style = SYMBOL_CWXHISTOGRAMDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWXHISTOGRAMDLG_IDNAME, const wxString& caption = SYMBOL_CWXHISTOGRAMDLG_TITLE, const wxPoint& pos = SYMBOL_CWXHISTOGRAMDLG_POSITION, const wxSize& size = SYMBOL_CWXHISTOGRAMDLG_SIZE, long style = SYMBOL_CWXHISTOGRAMDLG_STYLE );

    /// Destructor
    ~CwxHistogramDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CwxHistogramDlg event handler declarations

    void OnSubjectListSelected( wxCommandEvent& event );

    void OnQueryListSelected( wxCommandEvent& event );

    void OnColourpickerctrl1ColourPickerChanged( wxColourPickerEvent& event );

////@end CwxHistogramDlg event handler declarations

////@begin CwxHistogramDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CwxHistogramDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    CRgbaColor  GetGraphColor(const string& name);
    void    GetSelected(vector<string>& s_items, vector<string>& q_items);
    void    SetItems(const vector<CHitMatrixRenderer::SGraphDescr>& graphs,
                     const vector<string>& s_sel,
                     const vector<string>& q_sel,
                     bool en_s, bool en_q);

protected:
    void    x_FillList(wxCheckListBox* list, const vector<string>& items);
    void    x_SelectGraph(const string& name);

protected:
////@begin CwxHistogramDlg member variables
    wxStaticText* m_SubjectLabel;
    wxStaticText* m_QueryLabel;
    wxCheckListBox* m_SubjectList;
    wxCheckListBox* m_QueryList;
    wxColourPickerCtrl* m_ColorBtn;
    wxStaticText* m_ColorText;
protected:
    vector<CHitMatrixRenderer::SGraphDescr> m_GraphTypes;
    string m_GraphTypeName; // name of the currently selected graph
////@end CwxHistogramDlg member variables
};


END_NCBI_SCOPE

#endif //GUI_WIDGETS_HIT_MATRIX___WX_HISTOGRAM_DLG__HPP
