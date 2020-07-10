#ifndef GUI_WIDGETS_HIT_MATRIX___WX_SCORE_DLG__HPP
#define GUI_WIDGETS_HIT_MATRIX___WX_SCORE_DLG__HPP

/*  $Id: wx_score_dlg.hpp 25799 2012-05-10 17:11:19Z katargir $
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


/*!
 * Includes
 */

#include <gui/widgets/hit_matrix/hit_coloring.hpp>

#include <wx/dialog.h>

////@begin includes
#include "wx/valgen.h"
#include "wx/statline.h"
#include "wx/clrpicker.h"
////@end includes


////@begin forward declarations
class wxColourPickerCtrl;
class CGradientPanel;
////@end forward declarations

class wxListBox;
class wxTextCtrl;
class wxCheckBox;


/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CWXSCOREDLG 10002
#define ID_CHECKBOX 10013
#define ID_LISTBOX 10003
#define ID_MIN_VALUE 10004
#define ID_MAX_VALUE 10012
#define ID_MIN_GRAD_CHECK 10000
#define ID_MIN_GRAD 10001
#define ID_MAX_GRAD_CHECK 10005
#define ID_MAX_GRAD 10010
#define ID_MIN_COLOR_BTN 10007
#define ID_MAX_COLOR_BTN 10008
#define ID_CHECKBOX1 10009
#define ID_CGRADIENTPANEL1 10011
#define SYMBOL_CWXSCOREDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWXSCOREDLG_TITLE _("Dot Matrix - Color ByScore")
#define SYMBOL_CWXSCOREDLG_IDNAME ID_CWXSCOREDLG
#define SYMBOL_CWXSCOREDLG_SIZE wxDefaultSize
#define SYMBOL_CWXSCOREDLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

class CGradientPanel;


class CwxScoreDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CwxScoreDlg )
    DECLARE_EVENT_TABLE()

public:
    CwxScoreDlg();
    CwxScoreDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWXSCOREDLG_IDNAME, const wxString& caption = SYMBOL_CWXSCOREDLG_TITLE, const wxPoint& pos = SYMBOL_CWXSCOREDLG_POSITION, const wxSize& size = SYMBOL_CWXSCOREDLG_SIZE, long style = SYMBOL_CWXSCOREDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWXSCOREDLG_IDNAME, const wxString& caption = SYMBOL_CWXSCOREDLG_TITLE, const wxPoint& pos = SYMBOL_CWXSCOREDLG_POSITION, const wxSize& size = SYMBOL_CWXSCOREDLG_SIZE, long style = SYMBOL_CWXSCOREDLG_STYLE );

    ~CwxScoreDlg();

    void Init();

    void CreateControls();

////@begin CwxScoreDlg event handler declarations

    void OnCheckboxClick( wxCommandEvent& event );

    void OnListboxSelected( wxCommandEvent& event );

    void OnMinGradCheckClick( wxCommandEvent& event );

    void OnKillFocus( wxFocusEvent& event );

    void OnMaxGradCheckClick( wxCommandEvent& event );

    void OnMinColorBtnColourPickerChanged( wxColourPickerEvent& event );

    void OnMaxColorBtnColourPickerChanged( wxColourPickerEvent& event );

    void OnCheckbox1Click( wxCommandEvent& event );

    void OnOkClick( wxCommandEvent& event );

////@end CwxScoreDlg event handler declarations

////@begin CwxScoreDlg member function declarations

    vector<SHitColoringParams*>* GetOrigParams() const { return m_OrigParams ; }
    void SetOrigParams(vector<SHitColoringParams*>* value) { m_OrigParams = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CwxScoreDlg member function declarations

    static bool ShowToolTips();

    void SetItems(vector<SHitColoringParams*>& params, const string curr_name);
    string GetSelectedName() const;

    virtual bool    TransferDataToWindow();
    virtual bool    TransferDataFromWindow();

protected:
////@begin CwxScoreDlg member variables
    wxCheckBox* m_EnableCheck;
    wxListBox* m_List;
    wxTextCtrl* m_MinValueText;
    wxTextCtrl* m_MaxValueText;
    wxCheckBox* m_MinGradCheck;
    wxTextCtrl* m_MinGradInput;
    wxCheckBox* m_MaxGradCheck;
    wxTextCtrl* m_MaxGradInput;
    wxColourPickerCtrl* m_MinColorBtn;
    wxColourPickerCtrl* m_MaxColorBtn;
    wxCheckBox* m_LogCheck;
    CGradientPanel* m_GradPanel;
protected:
    vector<SHitColoringParams> m_Params;
    vector<SHitColoringParams*>* m_OrigParams;
    SHitColoringParams m_CurrParams;
    bool m_EnableControls;
    SHitColoringParams m_EmptyParams;
    wxArrayInt m_ScoreSel; // array os selected lines in the listbox
    int m_CurrIndex;
////@end CwxScoreDlg member variables

    bool mf_FinalCopy;
};


END_NCBI_SCOPE

#endif
