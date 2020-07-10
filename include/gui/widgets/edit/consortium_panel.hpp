#ifndef GUI_WIDGETS_EDIT___CONSORTIUM_PANEL__HPP
#define GUI_WIDGETS_EDIT___CONSORTIUM_PANEL__HPP

/*  $Id: consortium_panel.hpp 42706 2019-04-04 17:21:31Z asztalos $
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

#include <corelib/ncbistd.hpp>
#include <objmgr/scope.hpp>

#include <wx/panel.h>
#include <wx/textctrl.h>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CAuthor;
END_SCOPE(objects)

/*!
 * Control identifiers
 */


////@begin control identifiers
#define ID_CONSORTIUM 10030
#define ID_TEXTCTRL 10014
#define ID_HYPERLINKCTRL1 10032
#define SYMBOL_CCONSORTIUMPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CCONSORTIUMPANEL_TITLE _("Consortium")
#define SYMBOL_CCONSORTIUMPANEL_IDNAME ID_CONSORTIUM
#define SYMBOL_CCONSORTIUMPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CCONSORTIUMPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CConsortiumPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CConsortiumPanel : public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CConsortiumPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CConsortiumPanel();
    CConsortiumPanel( wxWindow* parent, objects::CAuthor& author, wxWindowID id = SYMBOL_CCONSORTIUMPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCONSORTIUMPANEL_POSITION, const wxSize& size = SYMBOL_CCONSORTIUMPANEL_SIZE, long style = SYMBOL_CCONSORTIUMPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCONSORTIUMPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCONSORTIUMPANEL_POSITION, const wxSize& size = SYMBOL_CCONSORTIUMPANEL_SIZE, long style = SYMBOL_CCONSORTIUMPANEL_STYLE );

    /// Destructor
    ~CConsortiumPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    CRef<objects::CAuthor> GetAuthor() const;

////@begin CConsortiumPanel event handler declarations



////@end CConsortiumPanel event handler declarations

////@begin CConsortiumPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CConsortiumPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CConsortiumPanel member variables
    wxTextCtrl* m_Consortium;
////@end CConsortiumPanel member variables
private:
    CRef<objects::CAuthor> m_Author;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___CONSORTIUM_PANEL__HPP
