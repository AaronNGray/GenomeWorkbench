/*  $Id: alnspan_settings_dlg.hpp 22112 2010-09-22 21:14:41Z falkrb $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:
 */
#ifndef _ALNSPAN_SETTINGS_DLG_H_
#define _ALNSPAN_SETTINGS_DLG_H_

#include <corelib/ncbistd.hpp>
#include <wx/wx.h>

/*!
 * Includes
 */

////@begin includes
#include "wx/valtext.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CALNSPANSETTINGSDLG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CALNSPANSETTINGSDLG_TITLE _("Alignment span settings")
#define SYMBOL_CALNSPANSETTINGSDLG_IDNAME ID_CALNSPANSETTINGSDLG
#define SYMBOL_CALNSPANSETTINGSDLG_SIZE wxSize(400, 300)
#define SYMBOL_CALNSPANSETTINGSDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAlnSpanSettingsDlg class declaration
 */

class CAlnSpanSettingsDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CAlnSpanSettingsDlg )
    DECLARE_EVENT_TABLE()

public:

public:
    /// Constructors
    CAlnSpanSettingsDlg();
    CAlnSpanSettingsDlg( wxWindow* parent, wxWindowID id = SYMBOL_CALNSPANSETTINGSDLG_IDNAME, const wxString& caption = SYMBOL_CALNSPANSETTINGSDLG_TITLE, const wxPoint& pos = SYMBOL_CALNSPANSETTINGSDLG_POSITION, const wxSize& size = SYMBOL_CALNSPANSETTINGSDLG_SIZE, long style = SYMBOL_CALNSPANSETTINGSDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CALNSPANSETTINGSDLG_IDNAME, const wxString& caption = SYMBOL_CALNSPANSETTINGSDLG_TITLE, const wxPoint& pos = SYMBOL_CALNSPANSETTINGSDLG_POSITION, const wxSize& size = SYMBOL_CALNSPANSETTINGSDLG_SIZE, long style = SYMBOL_CALNSPANSETTINGSDLG_STYLE );

    /// Destructor
    ~CAlnSpanSettingsDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAlnSpanSettingsDlg event handler declarations

////@end CAlnSpanSettingsDlg event handler declarations

////@begin CAlnSpanSettingsDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAlnSpanSettingsDlg member function declarations

    int GetThreshold() const;
    void SetThreshold( int th );

    bool GetShowIndels() const;
    void SetShowIndels( bool indel );

    /// Should we show tooltips?
    static bool ShowToolTips();

private:
////@begin CAlnSpanSettingsDlg member variables
    wxTextCtrl* m_ThreshText;
    wxCheckBox* m_ShowIndels;
    /// Control identifiers
    enum {
        ID_CALNSPANSETTINGSDLG = 10000,
        ID_TEXTCTRL1 = 10001,
        ID_CHECKBOX = 10002
    };
////@end CAlnSpanSettingsDlg member variables
};

#endif
    // _ALNSPAN_SETTINGS_DLG_H_
