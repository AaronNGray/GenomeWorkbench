#ifndef GUI_WIDGETS_SEQTEXT___SEQTEXTVIEWOPTIONS__HPP
#define GUI_WIDGETS_SEQTEXT___SEQTEXTVIEWOPTIONS__HPP

/*  $Id: seqtextviewoptions.h 33741 2015-09-11 18:14:53Z shkeda $
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
 * Authors:  Colleen Bollin
 *
 * File Description:
 *
 */


/*!
 * Includes
 */

#include <corelib/ncbiobj.hpp>
#include <wx/dialog.h>
#include <gui/widgets/seq_text/seq_text_conf.hpp>

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

class wxRadioButton;
class wxChoice;

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_SEQTEXTVIEWOPTIONS 10000
#define ID_SEQTEXTDLG_FONTSIZE 10003
#define ID_SEQTEXTDLG_FEATCOLOR 10004
#define ID_SEQTEXTDLG_CODONDISPLAY 10005
#define ID_SEQTEXTDLG_DISPLAYCOORD 10006
#define ID_SEQTEXTDLG_OK 10007
#define ID_SEQTEXTDLG_CANCEL 10008
#define SYMBOL_CWXSEQTEXTVIEWOPTIONS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CWXSEQTEXTVIEWOPTIONS_TITLE _("Sequence Text View Options")
#define SYMBOL_CWXSEQTEXTVIEWOPTIONS_IDNAME ID_SEQTEXTVIEWOPTIONS
#define SYMBOL_CWXSEQTEXTVIEWOPTIONS_SIZE wxSize(400, 300)
#define SYMBOL_CWXSEQTEXTVIEWOPTIONS_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CwxSeqTextViewOptions class declaration
 */

class CwxSeqTextViewOptions: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CwxSeqTextViewOptions )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CwxSeqTextViewOptions();
    CwxSeqTextViewOptions(wxWindow* parent, CRef<CSeqTextConfig> cfg, wxWindowID id = SYMBOL_CWXSEQTEXTVIEWOPTIONS_IDNAME, const wxString& caption = SYMBOL_CWXSEQTEXTVIEWOPTIONS_TITLE, const wxPoint& pos = SYMBOL_CWXSEQTEXTVIEWOPTIONS_POSITION, const wxSize& size = SYMBOL_CWXSEQTEXTVIEWOPTIONS_SIZE, long style = SYMBOL_CWXSEQTEXTVIEWOPTIONS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWXSEQTEXTVIEWOPTIONS_IDNAME, const wxString& caption = SYMBOL_CWXSEQTEXTVIEWOPTIONS_TITLE, const wxPoint& pos = SYMBOL_CWXSEQTEXTVIEWOPTIONS_POSITION, const wxSize& size = SYMBOL_CWXSEQTEXTVIEWOPTIONS_SIZE, long style = SYMBOL_CWXSEQTEXTVIEWOPTIONS_STYLE );

    /// Destructor
    ~CwxSeqTextViewOptions();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CwxSeqTextViewOptions event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEQTEXTDLG_OK
    void OnSeqtextdlgOkClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEQTEXTDLG_CANCEL
    void OnSeqtextdlgCancelClick( wxCommandEvent& event );

////@end CwxSeqTextViewOptions event handler declarations

////@begin CwxSeqTextViewOptions member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CwxSeqTextViewOptions member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CwxSeqTextViewOptions member variables
    wxChoice* m_FontSize;
    wxChoice* m_FeatColor;
    wxChoice* m_CodonDisplay;
    wxChoice* m_DispCoord;
////@end CwxSeqTextViewOptions member variables


protected:
    void    x_SaveSettings();
    void    x_LoadSettings();

    CRef<CSeqTextConfig> m_Config;

};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQTEXT___SEQTEXTVIEWOPTIONS__HPP
