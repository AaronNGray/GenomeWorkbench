/*  $Id: multipleassemblyprogrampanel.hpp 32698 2015-04-14 13:39:22Z asztalos $
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
#ifndef _MULTIPLEASSEMBLYPROGRAMPANEL_H_
#define _MULTIPLEASSEMBLYPROGRAMPANEL_H_

#include <corelib/ncbistd.hpp>
#include "assemblyprogrampanel.hpp"

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/scrolwin.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxFlexGridSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CMULTIPLEASSEMBLYPROGRAMPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CMULTIPLEASSEMBLYPROGRAMPANEL_TITLE _("MultipleAssemblyProgramPanel")
#define SYMBOL_CMULTIPLEASSEMBLYPROGRAMPANEL_IDNAME ID_CMULTIPLEASSEMBLYPROGRAMPANEL
#define SYMBOL_CMULTIPLEASSEMBLYPROGRAMPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CMULTIPLEASSEMBLYPROGRAMPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CMultipleAssemblyProgramPanel class declaration
 */

class CMultipleAssemblyProgramPanel: public CAssemblyProgramPanel
{    
    DECLARE_DYNAMIC_CLASS( CMultipleAssemblyProgramPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMultipleAssemblyProgramPanel();
    CMultipleAssemblyProgramPanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CMULTIPLEASSEMBLYPROGRAMPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CMULTIPLEASSEMBLYPROGRAMPANEL_POSITION, 
        const wxSize& size = SYMBOL_CMULTIPLEASSEMBLYPROGRAMPANEL_SIZE, 
        long style = SYMBOL_CMULTIPLEASSEMBLYPROGRAMPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CMULTIPLEASSEMBLYPROGRAMPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CMULTIPLEASSEMBLYPROGRAMPANEL_POSITION, 
        const wxSize& size = SYMBOL_CMULTIPLEASSEMBLYPROGRAMPANEL_SIZE, 
        long style = SYMBOL_CMULTIPLEASSEMBLYPROGRAMPANEL_STYLE );

    /// Destructor
    ~CMultipleAssemblyProgramPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual void SetValue( const string& val);
    virtual string GetValue ();

////@begin CMultipleAssemblyProgramPanel event handler declarations

////@end CMultipleAssemblyProgramPanel event handler declarations
    void OnVersionEntered( wxCommandEvent& event );

////@begin CMultipleAssemblyProgramPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMultipleAssemblyProgramPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CMultipleAssemblyProgramPanel member variables
    wxScrolledWindow* m_ScrolledWindow;
    wxFlexGridSizer* m_Sizer;
    /// Control identifiers
    enum {
        ID_CMULTIPLEASSEMBLYPROGRAMPANEL = 10122,
        ID_SCROLLEDWINDOW = 10045
    };
////@end CMultipleAssemblyProgramPanel member variables
#define ID_LASTVERSION 10046

private:
    int m_TotalHeight;
    int m_RowHeight;
    int m_TextWidth;

    void x_AddBlankRow ();
};

END_NCBI_SCOPE

#endif
    // _MULTIPLEASSEMBLYPROGRAMPANEL_H_
