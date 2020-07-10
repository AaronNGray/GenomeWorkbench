#ifndef GUI_WIDGETS_SEQ_GRAPHIC___WX_SEQMARKER_SET_DLG__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___WX_SEQMARKER_SET_DLG__HPP
/*  $Id: wx_seqmarker_set_dlg.hpp 32869 2015-04-30 18:22:40Z falkrb $
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
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <gui/utils/rgba_color.hpp>

////@begin includes
#include "wx/clrpicker.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE
/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CWXSEQMARKSETDLG 10013
#define ID_TEXTCTRL4 10039
#define ID_TEXTCTRL1 10003
#define ID_COLOURCTRL 10040
#define SYMBOL_CWXSEQMARKERSETDLG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWXSEQMARKERSETDLG_TITLE _("New Marker")
#define SYMBOL_CWXSEQMARKERSETDLG_IDNAME ID_CWXSEQMARKSETDLG
#define SYMBOL_CWXSEQMARKERSETDLG_SIZE wxDefaultSize
#define SYMBOL_CWXSEQMARKERSETDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CwxSeqMarkerSetDlg class declaration
 */

class CwxSeqMarkerSetDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CwxSeqMarkerSetDlg )
    DECLARE_EVENT_TABLE()

public:
    CwxSeqMarkerSetDlg();
    CwxSeqMarkerSetDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWXSEQMARKERSETDLG_IDNAME, const wxString& caption = SYMBOL_CWXSEQMARKERSETDLG_TITLE, const wxPoint& pos = SYMBOL_CWXSEQMARKERSETDLG_POSITION, const wxSize& size = SYMBOL_CWXSEQMARKERSETDLG_SIZE, long style = SYMBOL_CWXSEQMARKERSETDLG_STYLE );

    bool CreateX( wxWindow* parent, wxWindowID id = SYMBOL_CWXSEQMARKERSETDLG_IDNAME, const wxString& caption = SYMBOL_CWXSEQMARKERSETDLG_TITLE, const wxPoint& pos = SYMBOL_CWXSEQMARKERSETDLG_POSITION, const wxSize& size = SYMBOL_CWXSEQMARKERSETDLG_SIZE, long style = SYMBOL_CWXSEQMARKERSETDLG_STYLE );

    ~CwxSeqMarkerSetDlg();

    void Init();

    void CreateControls();

    void SetDlgTitle(const string& title);
    void SetMarkerName(const string& name);
    void SetMarkerPos(TSeqPos pos);
    void SetMarkerRange(TSeqRange range);
    void SetMarkerColor(const CRgbaColor& color);

    string GetMarkerName() const;
    TSeqPos GetMarkerPos() const;
    bool GetIsRange() const;
    TSeqRange GetMarkerRange() const;
    const CRgbaColor GetMarkerColor() const;

////@begin CwxSeqMarkerSetDlg event handler declarations

    void OnOkClick( wxCommandEvent& event );

////@end CwxSeqMarkerSetDlg event handler declarations

////@begin CwxSeqMarkerSetDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CwxSeqMarkerSetDlg member function declarations

    static bool ShowToolTips();

////@begin CwxSeqMarkerSetDlg member variables
    wxTextCtrl* m_MarkerName;
    wxTextCtrl* m_MarkerPos;
protected:
    unsigned int m_Pos;
    unsigned int m_ToPos;
////@end CwxSeqMarkerSetDlg member variables
    CRgbaColor   m_Color;
};


END_NCBI_SCOPE


#endif // GUI_WIDGETS_SEQ_GRAPHIC___WX_SEQMARKER_SET_DLG__HPP
