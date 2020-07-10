#ifndef GUI_WIDGETS_EDIT___MACRO_APPLY_SRCTABLE__HPP
#define GUI_WIDGETS_EDIT___MACRO_APPLY_SRCTABLE__HPP
/*  $Id: macro_applysrc_table.hpp 44637 2020-02-11 20:01:54Z asztalos $
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

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/macro_editor_context.hpp>
#include <wx/hyperlink.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>

class wxScrolledWindow;
class wxFlexGridSizer;
class wxStaticText;
class wxTextCtrl;
class wxRadioButton;
class wxListCtrl;

BEGIN_NCBI_SCOPE

class CSingleQualPanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CMACROAPPLYSRCTABLEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CMACROAPPLYSRCTABLEPANEL_TITLE _("Macro Apply Source Table")
#define SYMBOL_CMACROAPPLYSRCTABLEPANEL_IDNAME ID_CMACROAPPLYSRCTABLEPANEL
#define SYMBOL_CMACROAPPLYSRCTABLEPANEL_SIZE wxDefaultSize
#define SYMBOL_CMACROAPPLYSRCTABLEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CMacroApplySrcTablePanel class declaration
 */

class CMacroApplySrcTablePanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS(CMacroApplySrcTablePanel)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMacroApplySrcTablePanel();
    CMacroApplySrcTablePanel( wxWindow* parent, EMacroFieldType field_type,
        wxWindowID id = SYMBOL_CMACROAPPLYSRCTABLEPANEL_IDNAME,
        const wxPoint& pos = SYMBOL_CMACROAPPLYSRCTABLEPANEL_POSITION,
        const wxSize& size = SYMBOL_CMACROAPPLYSRCTABLEPANEL_SIZE,
        long style = SYMBOL_CMACROAPPLYSRCTABLEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CMACROAPPLYSRCTABLEPANEL_IDNAME,
        const wxPoint& pos = SYMBOL_CMACROAPPLYSRCTABLEPANEL_POSITION,
        const wxSize& size = SYMBOL_CMACROAPPLYSRCTABLEPANEL_SIZE,
        long style = SYMBOL_CMACROAPPLYSRCTABLEPANEL_STYLE);

    /// Destructor
    ~CMacroApplySrcTablePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CMacroApplySrcTablePanel event handler declarations

    void OnDelete(wxHyperlinkEvent& event);

    void OnOpenFile(wxCommandEvent& event);

    void OnHyperlinkClicked(wxHyperlinkEvent& event);

    void OnDelimiterSelected(wxCommandEvent& event);

    void OnNewFileEntered(wxCommandEvent& event);
///@end CMacroApplySrcTablePanel event handler declarations

////@begin CMacroApplySrcTablePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMacroApplySrcTablePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    string GetFilename() const;
    string GetDelimiter() const;
    bool IsMergeDelimiterSet() const { return m_MergeDel->IsChecked(); }
    bool IsSplitFirstSet() const { return m_SplitFirstCol->IsChecked(); }
    bool IsConvertMultiSet() const { return m_ConvertMulti->IsChecked(); }
    bool IsMergeFirstSet() const { return m_MergeTwoCols->IsChecked(); }
    vector<pair<string, string>> GetValues() const;

    SFieldFromTable GetMatchField() const;
    /// Control identifiers
    enum {
        ID_CMACROAPPLYSRCTABLEPANEL = 12000,
        ID_APPLYSRCTBLPREVIEW = 12001,
        ID_APPLYSRCTBLFILE,
        ID_APPLYSRCTBLBTMBTN,
        ID_APPLYSRCTBLWND,
        ID_APPLYSRCTBLADDQUAL,
        ID_APPLYTBLRBTN1,
        ID_APPLYTBLRBTN2,
        ID_APPLYTBLRBTN3,
        ID_APPLYTBLRBTN4,
        ID_APPLYTBLRBTN5,
        ID_APPLYTBLTXT1,
        ID_APPLYTBLCB1,
        ID_APPLYTBLCB2,
        ID_APPLYTBLCB3,
        ID_APPLYTBLCB4,
        ID_APPLYSRCTBLMATCHQUAL,
    };

private:
    CSingleQualPanel* x_AddEmptyRow();
    void x_AddRowToWindow(wxWindow* row);
    void x_ProcessFirstLine();

    int m_RowHeight{ 0 };
    EMacroFieldType m_FieldType{ EMacroFieldType::eNotSet };
    wxScrolledWindow* m_ScrolledWindow;
    wxFlexGridSizer* m_Sizer;
    wxListCtrl* m_TablePreview;
    wxStaticText* m_QualLabel;
    wxTextCtrl* m_Filename;
    wxRadioButton* m_Tabdel;
    wxRadioButton* m_Semidel;
    wxRadioButton* m_Commadel;
    wxRadioButton* m_Spacedel;
    wxRadioButton* m_Otherdel;
    wxTextCtrl* m_OtherText;
    wxCheckBox* m_MergeDel;
    wxCheckBox* m_SplitFirstCol;
    wxCheckBox* m_ConvertMulti;
    wxCheckBox* m_MergeTwoCols;
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_APPLY_SRCTABLE__HPP
