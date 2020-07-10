/*  $Id: sequencingtechnologypanel.hpp 39385 2017-09-19 17:08:13Z filippov $
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
#ifndef _SEQUENCINGTECHNOLOGYPANEL_H_
#define _SEQUENCINGTECHNOLOGYPANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <objects/general/User_object.hpp>
#include <gui/widgets/edit/singleassemblyprogrampanel.hpp>
#include <gui/widgets/edit/multipleassemblyprogrampanel.hpp>
#include <gui/widgets/edit/utilities.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/hyperlink.h"
#include <wx/checkbox.h>
#include <wx/choice.h>
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
class CSingleAssemblyProgramPanel;
class wxHyperlinkCtrl;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSEQUENCINGTECHNOLOGYPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSEQUENCINGTECHNOLOGYPANEL_TITLE _("SequencingTechnologyPanel")
#define SYMBOL_CSEQUENCINGTECHNOLOGYPANEL_IDNAME ID_CSEQUENCINGTECHNOLOGYPANEL
#define SYMBOL_CSEQUENCINGTECHNOLOGYPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSEQUENCINGTECHNOLOGYPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSequencingTechnologyPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CSequencingTechnologyPanel: public wxPanel, public IDescEditorPanel
{    
    DECLARE_DYNAMIC_CLASS( CSequencingTechnologyPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSequencingTechnologyPanel();
    CSequencingTechnologyPanel( wxWindow* parent, CRef<objects::CUser_object> user, 
        wxWindowID id = SYMBOL_CSEQUENCINGTECHNOLOGYPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSEQUENCINGTECHNOLOGYPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSEQUENCINGTECHNOLOGYPANEL_SIZE, 
        long style = SYMBOL_CSEQUENCINGTECHNOLOGYPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSEQUENCINGTECHNOLOGYPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSEQUENCINGTECHNOLOGYPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSEQUENCINGTECHNOLOGYPANEL_SIZE, 
        long style = SYMBOL_CSEQUENCINGTECHNOLOGYPANEL_STYLE );

    /// Destructor
    ~CSequencingTechnologyPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    CRef<objects::CUser_object> GetUser_object() const;

////@begin CSequencingTechnologyPanel event handler declarations

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX12
    void OnOtherMethodClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_BUTTON2
    void OnButton2HyperlinkClicked( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_BUTTON
    void OnClearClicked( wxHyperlinkEvent& event );

////@end CSequencingTechnologyPanel event handler declarations

////@begin CSequencingTechnologyPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSequencingTechnologyPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();


////@begin CSequencingTechnologyPanel member variables
    wxBoxSizer* m_MethodsSizer;
    wxCheckBox* m_OtherMethod;
    wxTextCtrl* m_OtherMethodText;
    wxBoxSizer* m_AssemblyProgramSizer;
    CSingleAssemblyProgramPanel* m_SingleAssemblyProgram;
    wxHyperlinkCtrl* m_AddPrograms;
    wxTextCtrl* m_AssemblyName;
    wxTextCtrl* m_Coverage;
    /// Control identifiers
    enum {
        ID_CSEQUENCINGTECHNOLOGYPANEL = 10060,
        ID_CHECKBOX5 = 10061,
        ID_CHECKBOX6 = 10063,
        ID_CHECKBOX7 = 10114,
        ID_CHECKBOX8 = 10115,
        ID_CHECKBOX9 = 10116,
        ID_CHECKBOX10 = 10117,
        ID_CHECKBOX11 = 10118,
        ID_CHECKBOX12 = 10119,
        ID_CHECKBOX13 = 10130,
        ID_OTHERMETHODTEXTCTRL = 10120,
        ID_ASSEMBLYFOREIGN = 10129,
        ID_BUTTON2 = 10128,
        ID_TEXTCTRL19 = 10125,
        ID_TEXTCTRL20 = 10126,
        ID_BUTTON = 10127,
        ID_CHOICE11 = 10121
    };
////@end CSequencingTechnologyPanel member variables

    // IDescEditorPanel implementation
    virtual void ChangeSeqdesc(const objects::CSeqdesc& desc);
    virtual void UpdateSeqdesc(objects::CSeqdesc& desc);

    void SetRegistryPath(const string& reg_path);
    void SaveSettings() const;
    bool GetReplaceAll();

private:
    CRef<objects::CUser_object> m_User;
    CAssemblyProgramPanel* m_AssemblyProgram;

    void x_ClearMethods();
    void x_ChangeToMultiplePrograms();
    wxChoice* m_StandardCommentCtrl;
    string m_RegPath;
};

END_NCBI_SCOPE

#endif
    // _SEQUENCINGTECHNOLOGYPANEL_H_
