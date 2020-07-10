#ifndef _GUI_PACKAGES__LOCUS_TAG_EUK_DLG_H_
#define _GUI_PACKAGES__LOCUS_TAG_EUK_DLG_H_
/*  $Id: locus_tag_euk_dlg.hpp 43444 2019-07-01 15:47:55Z filippov $
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
 * Authors:  Igor Filippov
 *
 */

#include <corelib/ncbistd.hpp>

#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/radiobox.h>

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_LOCUS_TAG_EUK_DLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_LOCUS_TAG_EUK_DLG_TITLE _("Locus Tag, Euk/Prok")
#define SYMBOL_LOCUS_TAG_EUK_DLG_IDNAME ID_LOCUS_TAG_EUK_DLG
#define SYMBOL_LOCUS_TAG_EUK_DLG_SIZE wxSize(400, 300)
#define SYMBOL_LOCUS_TAG_EUK_DLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CLocusTagEukDlg class declaration
 */

class CLocusTagEukDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CLocusTagEukDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CLocusTagEukDlg();
    CLocusTagEukDlg( wxWindow* parent, const string& locus_tag,
        wxWindowID id = SYMBOL_LOCUS_TAG_EUK_DLG_IDNAME, 
        const wxString& caption = SYMBOL_LOCUS_TAG_EUK_DLG_TITLE, 
        const wxPoint& pos = SYMBOL_LOCUS_TAG_EUK_DLG_POSITION, 
        const wxSize& size = SYMBOL_LOCUS_TAG_EUK_DLG_SIZE, 
        long style = SYMBOL_LOCUS_TAG_EUK_DLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_LOCUS_TAG_EUK_DLG_IDNAME, 
        const wxString& caption = SYMBOL_LOCUS_TAG_EUK_DLG_TITLE, 
        const wxPoint& pos = SYMBOL_LOCUS_TAG_EUK_DLG_POSITION, 
        const wxSize& size = SYMBOL_LOCUS_TAG_EUK_DLG_SIZE, 
        long style = SYMBOL_LOCUS_TAG_EUK_DLG_STYLE );

    /// Destructor
    ~CLocusTagEukDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CLocusTagEukDlg event handler declarations

////@end CLocusTagEukDlg event handler declarations

////@begin CLocusTagEukDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CLocusTagEukDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    string GetLocusTag();
    bool GetEuk() {return m_Euk->GetSelection() == 0;}
////@begin CLocusTagEukDlg member variables
    /// Control identifiers
    enum {
        ID_LOCUS_TAG_EUK_DLG = 10447,
        ID_LOCUS_TAG_EUK_TXT,
        ID_LOCUS_TAG_EUK_CHK
    };
////@end CLocusTagEukDlg member variables
private:
    wxTextCtrl* m_LocusTag;
    wxRadioBox* m_Euk;
    string m_locus_tag_value;
};

END_NCBI_SCOPE

#endif
    // _GUI_PACKAGES__LOCUS_TAG_EUK_DLG_H_
