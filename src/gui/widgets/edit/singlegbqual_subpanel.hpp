/*  $Id: singlegbqual_subpanel.hpp 37148 2016-12-08 16:17:23Z filippov $
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
 * Authors:  Colleen Bollin
 */
#ifndef _SIMPLEGBQUAL_SUBPANEL_H_
#define _SIMPLEGBQUAL_SUBPANEL_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <objects/seqfeat/Gb_qual.hpp>
#include <gui/widgets/edit/formattedqual_panel.hpp>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/frame.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/valtext.h>


/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations
class wxTextCtrl;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSINGLEGBQUALSUBPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSINGLEGBQUALSUBPANEL_TITLE _("SingleGbQual SubPanel")
#define SYMBOL_CSINGLEGBQUALSUBPANEL_IDNAME ID_SINGLEGBQUALSUBPANEL
#define SYMBOL_CSINGLEGBQUALSUBPANEL_SIZE wxDefaultSize
#define SYMBOL_CSINGLEGBQUALSUBPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSingleGbQualSubPanel class declaration
 */

class CSingleGbQualSubPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CSingleGbQualSubPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSingleGbQualSubPanel();
    CSingleGbQualSubPanel( wxWindow* parent, objects::CGb_qual& qual, wxWindowID id = SYMBOL_CSINGLEGBQUALSUBPANEL_IDNAME, const wxString& caption = SYMBOL_CSINGLEGBQUALSUBPANEL_TITLE, const wxPoint& pos = SYMBOL_CSINGLEGBQUALSUBPANEL_POSITION, const wxSize& size = SYMBOL_CSINGLEGBQUALSUBPANEL_SIZE, long style = SYMBOL_CSINGLEGBQUALSUBPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSINGLEGBQUALSUBPANEL_IDNAME, const wxString& caption = SYMBOL_CSINGLEGBQUALSUBPANEL_TITLE, const wxPoint& pos = SYMBOL_CSINGLEGBQUALSUBPANEL_POSITION, const wxSize& size = SYMBOL_CSINGLEGBQUALSUBPANEL_SIZE, long style = SYMBOL_CSINGLEGBQUALSUBPANEL_STYLE );

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    CRef<objects::CGb_qual> GetGbQual();

    void SetGbQual(CRef<objects::CGb_qual>& qual);

    /// Destructor
    ~CSingleGbQualSubPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSingleGbQualSubPanel event handler declarations

////@end CSingleGbQualSubPanel event handler declarations

////@begin CSingleGbQualSubPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSingleGbQualSubPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSingleGbQualSubPanel member variables
    wxBoxSizer* m_TheSizer;
    wxStaticText* m_KeyCtrl;
    /// Control identifiers
    enum {
        ID_SINGLEGBQUALSUBPANEL = 10046
    };
////@end CSingleGbQualSubPanel member variables

private:
    CRef<objects::CGb_qual> m_Qual;
    CFormattedQualPanel* m_ValueCtrl;
};

END_NCBI_SCOPE

#endif
    // _SIMPLEGBQUAL_SUBPANEL_H_
