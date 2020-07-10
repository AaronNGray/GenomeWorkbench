#ifndef GUI_WIDGETS_MACRO_EDIT___VAR_RESOLVER_DLG__HPP
#define GUI_WIDGETS_MACRO_EDIT___VAR_RESOLVER_DLG__HPP

/*  $Id: var_resolver_dlg.hpp 37718 2017-02-10 14:20:50Z asztalos $
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
 * Authors: Anatoly Osipov
 */

#include <gui/gui_export.h>
#include <gui/widgets/wx/dialog.hpp>
#include <gui/objutils/macro_rep.hpp>

////@begin forward declarations
class wxPropertyGrid;
class wxPGProperty;
////@end forward declarations

/*!
 * Control identifiers
 */
////@begin control identifiers
#define SYMBOL_CVARRESOLVERDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CVARRESOLVERDLG_TITLE _("Variable Resolver")
#define SYMBOL_CVARRESOLVERDLG_IDNAME ID_CVARRESOLVERDLG
#define SYMBOL_CVARRESOLVERDLG_SIZE wxDefaultSize
#define SYMBOL_CVARRESOLVERDLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CVarResolverDlg class declaration
 */

class NCBI_GUIWIDGETS_MACRO_EDIT_EXPORT CVarResolverDlg: public CDialog
{    
    DECLARE_DYNAMIC_CLASS( CVarResolverDlg )
    DECLARE_EVENT_TABLE()

private:
    /// Constructor for DYNAMIC CLASS
    CVarResolverDlg( );
public:
    typedef map<string, string> TMacroMap;
    /// Constructor
    CVarResolverDlg( macro::CMacroRep& macro_rep,
                     wxWindow* parent,
                     wxWindowID id = SYMBOL_CVARRESOLVERDLG_IDNAME, 
                     const wxString& caption = SYMBOL_CVARRESOLVERDLG_TITLE, 
                     const wxPoint& pos = SYMBOL_CVARRESOLVERDLG_POSITION, 
                     const wxSize& size = SYMBOL_CVARRESOLVERDLG_SIZE, 
                     long style = SYMBOL_CVARRESOLVERDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
                 wxWindowID id = SYMBOL_CVARRESOLVERDLG_IDNAME, 
                 const wxString& caption = SYMBOL_CVARRESOLVERDLG_TITLE, 
                 const wxPoint& pos = SYMBOL_CVARRESOLVERDLG_POSITION, 
                 const wxSize& size = SYMBOL_CVARRESOLVERDLG_SIZE, 
                 long style = SYMBOL_CVARRESOLVERDLG_STYLE );

    /// Destructor
    ~CVarResolverDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    bool GetStatus() { return m_Status; }
////@begin CVarResolverDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
    void OnButtonDoneClick( wxCommandEvent& event );
    void OnButtonCancelClick( wxCommandEvent& event );
    void OnButtonResetClick( wxCommandEvent& event );
////@end CVarResolverDlg event handler declarations

////@begin CVarResolverDlg member function declarations

    
private:
    /// Control identifiers
    enum {
        ID_CVARRESOLVERDLG = 10000,
        ID_PROPERTY_GRID_CTRL = 10001,
        ID_BUTTON_RESET = 10002,
        ID_BUTTON_CANCEL = 10003,
        ID_BUTTON_DONE = 10004
    };

    /// Private types and data members
    struct SPropPair {
        SPropPair(wxPGProperty* WxProp, macro::IMacroVar* MacroVar, wxString s) {
            m_WxProp = WxProp;
            m_MacroVar = MacroVar;

            m_DefaultVisible = s;
        }
        wxPGProperty* m_WxProp;
        macro::IMacroVar* m_MacroVar;

        wxString m_DefaultVisible;
    };
    typedef list<SPropPair*> TPropPairsList;
    
    TPropPairsList m_PropPairsList;
    wxPropertyGrid* m_PropGridCtrl;
    CRef<macro::CMacroRep> m_MacroRep;

    bool m_Status;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_MACRO_EDIT___VAR_RESOLVER_DLG__HPP
