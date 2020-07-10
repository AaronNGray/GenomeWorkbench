#ifndef GUI_WIDGETS_MACRO_EDIT___MACRO_EDIT_DLG__HPP
#define GUI_WIDGETS_MACRO_EDIT___MACRO_EDIT_DLG__HPP

/*  $Id: edit_macro_dlg.hpp 44983 2020-05-01 15:28:14Z asztalos $
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
 * Authors:  Roman Katargin, Anatoly Osipov
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/widgets/wx/dialog.hpp>
#include <gui/objutils/macro_engine.hpp>


class wxTextCtrl;
class wxListBox;

/*!
 * Control identifiers
 */
////@begin control identifiers
#define SYMBOL_CEDITMACRODLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CEDITMACRODLG_TITLE _("Run Macro")
#define SYMBOL_CEDITMACRODLG_IDNAME ID_CEDITMACRODLG
#define SYMBOL_CEDITMACRODLG_SIZE wxDefaultSize
#define SYMBOL_CEDITMACRODLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(macro)
    class CMacroDataException;
END_SCOPE(macro)

/*!
 * CEditMacroDlg class declaration
 */

class NCBI_GUIWIDGETS_MACRO_EDIT_EXPORT CEditMacroDlg: public CDialog
{    
    DECLARE_DYNAMIC_CLASS( CEditMacroDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CEditMacroDlg();
    CEditMacroDlg( wxWindow* parent, 
                   wxWindowID id = SYMBOL_CEDITMACRODLG_IDNAME, 
                   const wxString& caption = SYMBOL_CEDITMACRODLG_TITLE, 
                   const wxPoint& pos = SYMBOL_CEDITMACRODLG_POSITION, 
                   const wxSize& size = SYMBOL_CEDITMACRODLG_SIZE, 
                   long style = SYMBOL_CEDITMACRODLG_STYLE );

    void SetParams(objects::CSeq_entry_Handle TopSeqEntry, CConstRef<objects::CSeq_submit> submit, ICommandProccessor* CmdProccessor)
    {
        m_TopSeqEntry = TopSeqEntry;
        m_SeqSubmit = submit;
        m_CmdProccessor = CmdProccessor;
    }

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CEDITMACRODLG_IDNAME, 
        const wxString& caption = SYMBOL_CEDITMACRODLG_TITLE, 
        const wxPoint& pos = SYMBOL_CEDITMACRODLG_POSITION, 
        const wxSize& size = SYMBOL_CEDITMACRODLG_SIZE, 
        long style = SYMBOL_CEDITMACRODLG_STYLE );

    /// Destructor
    ~CEditMacroDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CEditMacroDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
    void OnRunClick( wxCommandEvent& event );
    void OnPrintClick(wxCommandEvent& event);
    void OnPrintParallelClick(wxCommandEvent& event);

    void OnListItemSelected( wxCommandEvent& event);

    void LoadAutofix_GB( wxCommandEvent& event );
    void LoadAutofix_TSA(wxCommandEvent& event);
    void LoadAutofix_WGS( wxCommandEvent& event );
    void LoadOtherMacros( wxCommandEvent& event );

////@end CEditMacroDlg event handler declarations

////@begin CEditMacroDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CEditMacroDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CEditMacroDlg member variables
    wxTextCtrl* m_TextCtrl;
    wxListBox*  m_MacroList;
    /// Control identifiers
    enum {
        ID_CEDITMACRODLG = wxID_HIGHEST + 100,
        ID_LISTBOX,
        ID_TEXTCTRL,
        ID_RUN,
        ID_PRINT,
        ID_PRINTPARALLEL,
        ID_AUTOFIXGB,
        ID_AUTOFIXTSA,
        ID_AUTOFIXWGS,
        ID_OTHERMACROS
    };
////@end CEditMacroDlg member variables
private:
    void x_LoadMacroNames();
    void x_LoadAutofixMacro(const string& name);

    objects::CSeq_entry_Handle m_TopSeqEntry;
    CConstRef<objects::CSeq_submit> m_SeqSubmit;
    ICommandProccessor* m_CmdProccessor;
    vector<string> m_Macros;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_MACRO_EDIT___MACRO_EDIT_DLG__HPP
