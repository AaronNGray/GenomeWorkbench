/*  $Id: singleassemblyprogrampanel.hpp 26367 2012-09-05 12:56:41Z bollin $
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
#ifndef _SINGLEASSEMBLYPROGRAMPANEL_H_
#define _SINGLEASSEMBLYPROGRAMPANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/assemblyprogrampanel.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/textctrl.h>
#include <wx/sizer.h>

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
#define SYMBOL_CSINGLEASSEMBLYPROGRAMPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSINGLEASSEMBLYPROGRAMPANEL_TITLE _("SingleAssemblyProgramPanel")
#define SYMBOL_CSINGLEASSEMBLYPROGRAMPANEL_IDNAME ID_CSINGLEASSEMBLYPROGRAMPANEL
#define SYMBOL_CSINGLEASSEMBLYPROGRAMPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSINGLEASSEMBLYPROGRAMPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSingleAssemblyProgramPanel class declaration
 */

class CSingleAssemblyProgramPanel: public CAssemblyProgramPanel
{    
    DECLARE_DYNAMIC_CLASS( CSingleAssemblyProgramPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSingleAssemblyProgramPanel();
    CSingleAssemblyProgramPanel( wxWindow* parent, wxWindowID id = SYMBOL_CSINGLEASSEMBLYPROGRAMPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSINGLEASSEMBLYPROGRAMPANEL_POSITION, const wxSize& size = SYMBOL_CSINGLEASSEMBLYPROGRAMPANEL_SIZE, long style = SYMBOL_CSINGLEASSEMBLYPROGRAMPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSINGLEASSEMBLYPROGRAMPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSINGLEASSEMBLYPROGRAMPANEL_POSITION, const wxSize& size = SYMBOL_CSINGLEASSEMBLYPROGRAMPANEL_SIZE, long style = SYMBOL_CSINGLEASSEMBLYPROGRAMPANEL_STYLE );

    /// Destructor
    ~CSingleAssemblyProgramPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual void SetValue( const string& val);
    virtual string GetValue ();

////@begin CSingleAssemblyProgramPanel event handler declarations

////@end CSingleAssemblyProgramPanel event handler declarations

////@begin CSingleAssemblyProgramPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSingleAssemblyProgramPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSingleAssemblyProgramPanel member variables
    wxTextCtrl* m_Program;
    wxTextCtrl* m_Version;
    /// Control identifiers
    enum {
        ID_CSINGLEASSEMBLYPROGRAMPANEL = 10121,
        ID_TEXTCTRL17 = 10123,
        ID_TEXTCTRL18 = 10124
    };
////@end CSingleAssemblyProgramPanel member variables
};

END_NCBI_SCOPE

#endif
    // _SINGLEASSEMBLYPROGRAMPANEL_H_
