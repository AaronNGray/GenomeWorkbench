#ifndef GUI_CORE___SEQ_FEAT_REMOVE_DLG__HPP
#define GUI_CORE___SEQ_FEAT_REMOVE_DLG__HPP

/*  $Id: seq_feat_remove_dlg.hpp 22112 2010-09-22 21:14:41Z falkrb $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

/*!
 * Includes
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>

////@begin includes
#include "wx/valtext.h"
////@end includes

#include <wx/dialog.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

BEGIN_NCBI_SCOPE

////@begin control identifiers
#define SYMBOL_CSEQFEATREMOVEDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSEQFEATREMOVEDLG_TITLE _("Feature Delete Dialog")
#define SYMBOL_CSEQFEATREMOVEDLG_IDNAME ID_CSEQFEATREMOVEDLG
#define SYMBOL_CSEQFEATREMOVEDLG_SIZE wxSize(400, 300)
#define SYMBOL_CSEQFEATREMOVEDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSeqFeatRemoveDlg class declaration
 */

class CSeqFeatRemoveDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CSeqFeatRemoveDlg )
    DECLARE_EVENT_TABLE()

public:
    CSeqFeatRemoveDlg();
    CSeqFeatRemoveDlg( wxWindow* parent, wxWindowID id = SYMBOL_CSEQFEATREMOVEDLG_IDNAME, const wxString& caption = SYMBOL_CSEQFEATREMOVEDLG_TITLE, const wxPoint& pos = SYMBOL_CSEQFEATREMOVEDLG_POSITION, const wxSize& size = SYMBOL_CSEQFEATREMOVEDLG_SIZE, long style = SYMBOL_CSEQFEATREMOVEDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSEQFEATREMOVEDLG_IDNAME, const wxString& caption = SYMBOL_CSEQFEATREMOVEDLG_TITLE, const wxPoint& pos = SYMBOL_CSEQFEATREMOVEDLG_POSITION, const wxSize& size = SYMBOL_CSEQFEATREMOVEDLG_SIZE, long style = SYMBOL_CSEQFEATREMOVEDLG_STYLE );

    ~CSeqFeatRemoveDlg();

    void Init();

    void CreateControls();

////@begin CSeqFeatRemoveDlg event handler declarations

////@end CSeqFeatRemoveDlg event handler declarations

////@begin CSeqFeatRemoveDlg member function declarations

    wxString GetText() const { return m_Text ; }
    void SetText(wxString value) { m_Text = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CSeqFeatRemoveDlg member function declarations

    static bool ShowToolTips();

////@begin CSeqFeatRemoveDlg member variables
private:
    wxString m_Text;
    enum {
        ID_CSEQFEATREMOVEDLG = 10028,
        ID_TEXTCTRL1 = 10029
    };
////@end CSeqFeatRemoveDlg member variables
};

END_NCBI_SCOPE

#endif  // GUI_CORE___SEQ_FEAT_REMOVE_DLG__HPP
