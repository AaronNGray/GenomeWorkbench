/*  $Id: parse_text_dlg.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
 * Authors:  Igor Filippov
 *
 * File Description:
 *
 */

#ifndef PARSE_TEXT_DLG__HPP
#define PARSE_TEXT_DLG__HPP


#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */


////@begin includes
#include <wx/choicebk.h>
////@end includes
#include <objects/macro/String_constraint.hpp>
#include <gui/widgets/edit/cds_gene_prot_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/parse_text_options_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/remove_text_outside_string_panel.hpp>
#include <gui/packages/pkg_sequence_edit/cap_change_panel.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>


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
#define SYMBOL_CPARSE_TEXT_DLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CPARSE_TEXT_DLG_TITLE _("Parse Text")
#define SYMBOL_CPARSE_TEXT_DLG_IDNAME wxID_ANY
#define SYMBOL_CPARSE_TEXT_DLG_SIZE wxDefaultSize
#define SYMBOL_CPARSE_TEXT_DLG_POSITION wxDefaultPosition

////@end control identifiers

/*!
 * CParseTextDlg class declaration
 */

class CConstraintPanel;
class CQualChoicePanel;

class CParseTextDlg: public CBulkCmdDlg, public CFieldNamePanelParent
{    
    DECLARE_DYNAMIC_CLASS( CParseTextDlg )
    DECLARE_EVENT_TABLE()

public:
    CParseTextDlg();
    CParseTextDlg( wxWindow* parent, IWorkbench* wb, 
                wxWindowID id = SYMBOL_CPARSE_TEXT_DLG_IDNAME, const wxString& caption = SYMBOL_CPARSE_TEXT_DLG_TITLE, const wxPoint& pos = SYMBOL_CPARSE_TEXT_DLG_POSITION, const wxSize& size = SYMBOL_CPARSE_TEXT_DLG_SIZE, long style = SYMBOL_CPARSE_TEXT_DLG_STYLE );

    bool Create( wxWindow* parent, IWorkbench* wb,
                 wxWindowID id = SYMBOL_CPARSE_TEXT_DLG_IDNAME, const wxString& caption = SYMBOL_CPARSE_TEXT_DLG_TITLE, const wxPoint& pos = SYMBOL_CPARSE_TEXT_DLG_POSITION, const wxSize& size = SYMBOL_CPARSE_TEXT_DLG_SIZE, long style = SYMBOL_CPARSE_TEXT_DLG_STYLE );

    ~CParseTextDlg();

    void Init();

    void CreateControls();

////@begin CParseTextDlg event handler declarations

////@end CParseTextDlg event handler declarations

////@begin CParseTextDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CParseTextDlg member function declarations

    static bool ShowToolTips();

    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage() {return kEmptyStr;}

    void ProcessUpdateFeatEvent( wxCommandEvent& event );

    enum EFieldType {
        eFieldType_Taxname = 0,
        eFieldType_Source,
        eFieldType_Feature,
        eFieldType_CDSGeneProt,
        eFieldType_RNA,
        eFieldType_MolInfo,
        eFieldType_Pub,
        eFieldType_StructuredComment,
        eFieldType_DBLink,
        eFieldType_Misc,
        eFieldType_LocalId,
        eFieldType_DefLine,
        eFieldType_BankITComment,
        eFieldType_TaxnameAfterBinomial,
        eFieldType_Dbxref,
        eFieldType_FileId,
        eFieldType_GeneralId,
        eFieldType_Comment,
        eFieldType_FlatFile
    };

    virtual void UpdateEditor(void) {}
    void SetFromFieldType(EFieldType field_type);
    void SetToFieldType(EFieldType field_type);
private:
    void UpdateChildrenFeaturePanels( wxWindow* win );
    virtual void SetRegistryPath(const string& reg_path);  
    virtual void LoadSettings();
    virtual void SaveSettings() const;

////@begin CParseTextDlg member variables
    CParseTextOptionsDlg* m_ParseOptions;
    CQualChoicePanel* m_Field1;
    CQualChoicePanel* m_Field2;
    CConstraintPanel* m_Constraint;
    string m_RegPath;
};


class CQualChoicePanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CQualChoicePanel )
    DECLARE_EVENT_TABLE()
public:
    CQualChoicePanel();
    CQualChoicePanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CQualChoicePanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    void SetSelection(int page);
    string GetField(bool subfield = true);
    string GetFieldType(void);
    CFieldNamePanel* GetFieldNamePanel(void);
    CIRef<IEditingAction> CreateEditingAction(CSeq_entry_Handle seh);

private:
    wxChoicebook* m_Notebook;
};



END_NCBI_SCOPE

#endif  // PARSE_TEXT_DLG__HPP
