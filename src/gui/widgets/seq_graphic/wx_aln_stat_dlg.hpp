/*  $Id: wx_aln_stat_dlg.hpp 30858 2014-07-31 14:05:43Z ucko $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */
#ifndef _WX_ALN_STAT_DLG_H_
#define _WX_ALN_STAT_DLG_H_


/*!
 * Includes
 */

////@begin includes
////@end includes
#include <corelib/ncbiobj.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/radiobox.h>
#include <wx/stattext.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations
class wxTextCtrl;
class wxSlider;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CWXALNSTATOPTIONSDLG 10031
#define ID_SLIDER 10037
#define ID_RADIOBOX2 10034
#define ID_RADIOBOX 10033
#define ID_RADIOBOX3 10036
#define ID_TEXTCTRL3 10032
#define SYMBOL_CWXALNSTATOPTIONSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWXALNSTATOPTIONSDLG_TITLE _("Alignment statistics settings")
#define SYMBOL_CWXALNSTATOPTIONSDLG_IDNAME ID_CWXALNSTATOPTIONSDLG
#define SYMBOL_CWXALNSTATOPTIONSDLG_SIZE wxSize(400, 300)
#define SYMBOL_CWXALNSTATOPTIONSDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CwxAlnStatOptionsDlg class declaration
 */

class CwxAlnStatOptionsDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CwxAlnStatOptionsDlg )
    DECLARE_EVENT_TABLE()

public:
    CwxAlnStatOptionsDlg();
    CwxAlnStatOptionsDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWXALNSTATOPTIONSDLG_IDNAME, const wxString& caption = SYMBOL_CWXALNSTATOPTIONSDLG_TITLE, const wxPoint& pos = SYMBOL_CWXALNSTATOPTIONSDLG_POSITION, const wxSize& size = SYMBOL_CWXALNSTATOPTIONSDLG_SIZE, long style = SYMBOL_CWXALNSTATOPTIONSDLG_STYLE );

    bool CreateX( wxWindow* parent, wxWindowID id = SYMBOL_CWXALNSTATOPTIONSDLG_IDNAME, const wxString& caption = SYMBOL_CWXALNSTATOPTIONSDLG_TITLE, const wxPoint& pos = SYMBOL_CWXALNSTATOPTIONSDLG_POSITION, const wxSize& size = SYMBOL_CWXALNSTATOPTIONSDLG_SIZE, long style = SYMBOL_CWXALNSTATOPTIONSDLG_STYLE );

    ~CwxAlnStatOptionsDlg();

    void Init();

    void CreateControls();

    void SetContent(bool is_agtc);
    void SetValueType(bool is_count);
    void SetDisplay(bool is_graph);
    void SetGraphHeight(int h);
    void SetZoomLevel(int l);
    bool IsAGTC() const;
    bool IsCount() const;
    bool IsBarGraph() const;
    int  GetGraphHeight() const;
    int  GetZoomLevel() const;

////@begin CwxAlnStatOptionsDlg event handler declarations

////@end CwxAlnStatOptionsDlg event handler declarations

////@begin CwxAlnStatOptionsDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CwxAlnStatOptionsDlg member function declarations

    static bool ShowToolTips();

////@begin CwxAlnStatOptionsDlg member variables
    wxSlider* m_ZoomLevel;
    wxRadioBox* m_Content;
    wxRadioBox* m_ValueType;
    wxRadioBox* m_Display;
    wxTextCtrl* m_GraphHeight;
////@end CwxAlnStatOptionsDlg member variables
};

END_NCBI_SCOPE

#endif
    // _WX_ALN_STAT_DLG_H_
