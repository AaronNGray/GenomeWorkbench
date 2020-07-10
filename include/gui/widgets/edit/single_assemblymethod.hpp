#ifndef GUI_PKG_SEQ_EDIT___SINGLE_ASSEMBLYMETHOD__HPP
#define GUI_PKG_SEQ_EDIT___SINGLE_ASSEMBLYMETHOD__HPP
/*  $Id: single_assemblymethod.hpp 43420 2019-06-27 14:26:24Z filippov $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software / database is a "United States Government Work" under the
*  terms of the United States Copyright Act.It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.This software / database is freely available
*  to the public for use.The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data.The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors: Andrea Asztalos
*
*/

#include <corelib/ncbiobj.hpp>
#include <wx/panel.h>

class wxTextCtrl;
class wxComboBox;

BEGIN_NCBI_SCOPE
/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSINGLEASSEMBLYMETHOD_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSINGLEASSEMBLYMETHOD_TITLE _("Assembly Method ")
#define SYMBOL_CSINGLEASSEMBLYMETHOD_IDNAME ID_CSINGLEASSEMBLYMETHOD
#define SYMBOL_CSINGLEASSEMBLYMETHOD_SIZE wxSize(400, 300)
#define SYMBOL_CSINGLEASSEMBLYMETHOD_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSingleAssemblyMethod class declaration
 */

class CSingleAssemblyMethod: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CSingleAssemblyMethod )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSingleAssemblyMethod();
    CSingleAssemblyMethod( wxWindow* parent,
        wxWindowID id = SYMBOL_CSINGLEASSEMBLYMETHOD_IDNAME, 
        const wxPoint& pos = SYMBOL_CSINGLEASSEMBLYMETHOD_POSITION, 
        const wxSize& size = SYMBOL_CSINGLEASSEMBLYMETHOD_SIZE, 
        long style = SYMBOL_CSINGLEASSEMBLYMETHOD_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSINGLEASSEMBLYMETHOD_IDNAME, 
        const wxPoint& pos = SYMBOL_CSINGLEASSEMBLYMETHOD_POSITION, 
        const wxSize& size = SYMBOL_CSINGLEASSEMBLYMETHOD_SIZE, 
        long style = SYMBOL_CSINGLEASSEMBLYMETHOD_STYLE );

    /// Destructor
    ~CSingleAssemblyMethod();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSingleAssemblyMethod event handler declarations

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_GASSEMBLYMETHOD

////@end CSingleAssemblyMethod event handler declarations

////@begin CSingleAssemblyMethod member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSingleAssemblyMethod member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void SetValue(const string& val);
    string GetValue() const;
private:
////@begin CSingleAssemblyMethod member variables
    wxComboBox* m_Choice;
    wxTextCtrl* m_Value;
public:
    /// Control identifiers
    enum {
        ID_CSINGLEASSEMBLYMETHOD = 6550,
        ID_GASSEMBLYMETHOD = 6551,
        ID_GASSEMBLYVERSION = 6552
    };
////@end CSingleAssemblyMethod member variables
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___SINGLE_ASSEMBLYMETHOD__HPP
