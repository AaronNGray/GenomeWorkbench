/*  $Id: tbl_edit_dlg.hpp 34165 2015-11-17 14:28:55Z filippov $
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
#ifndef _TBL_EDIT_DLG_H_
#define _TBL_EDIT_DLG_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/seq_table_grid.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
#include <gui/framework/workbench.hpp>
#include <wx/sizer.h>
#include <wx/button.h>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

class CTableCommandConverter 
{
public:
    CTableCommandConverter() {};
    virtual ~CTableCommandConverter() {};

    virtual CRef<objects::CSeq_table> GetValuesTableFromSeqEntry(objects::CSeq_entry_Handle seh) = 0;
    virtual CRef<CCmdComposite> GetCommandFromValuesTable(CRef<objects::CSeq_table>, objects::CSeq_entry_Handle seh) = 0;
    virtual string GetErrorMessage(CRef<objects::CSeq_table>) = 0;
    virtual string CheckForMixedValues(CRef<objects::CSeq_table> values_table, string column_name);
    virtual CRef<objects::CSeq_table> GetChoices(CRef<objects::CSeq_table> values_table) { CRef<objects::CSeq_table> choices; return choices; } ;
    virtual bool IsReadOnlyColumn(string column_name) { return false; };
    virtual int GetCollapsible() {return -1;}
};


class CTableFieldCommandConverter : public CTableCommandConverter
{
public:
    CTableFieldCommandConverter() {};
    virtual ~CTableFieldCommandConverter() {};

    virtual CRef<CCmdComposite> ClearAllValues(objects::CSeq_entry_Handle seh) = 0;
    virtual string RemapColumnName(string column_name) = 0;
};

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CTBLEDITDLG 10079
#define ID_IMPORT_TABLE 10086
#define ID_EXPORT_TABLE 10087
#define ID_CLEAR_TABLE 10088
#define SYMBOL_CTBLEDITDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CTBLEDITDLG_TITLE _("TblEditDlg")
#define SYMBOL_CTBLEDITDLG_IDNAME ID_CTBLEDITDLG
#define SYMBOL_CTBLEDITDLG_SIZE wxSize(400, 300)
#define SYMBOL_CTBLEDITDLG_POSITION wxDefaultPosition
////@end control identifiers
#define ID_UPDATE_PROBLEMS 10100

/*!
 * CTblEditDlg class declaration
 */

class CTblEditDlg: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CTblEditDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CTblEditDlg();
    // Dialog takes ownership of converter
    CTblEditDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                 CTableCommandConverter *converter,
                 IWorkbench* workbench = NULL,
                 wxWindowID id = SYMBOL_CTBLEDITDLG_IDNAME, const wxString& caption = SYMBOL_CTBLEDITDLG_TITLE, const wxPoint& pos = SYMBOL_CTBLEDITDLG_POSITION, const wxSize& size = SYMBOL_CTBLEDITDLG_SIZE, long style = SYMBOL_CTBLEDITDLG_STYLE, wxString label = wxT("") );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTBLEDITDLG_IDNAME, const wxString& caption = SYMBOL_CTBLEDITDLG_TITLE, const wxPoint& pos = SYMBOL_CTBLEDITDLG_POSITION, const wxSize& size = SYMBOL_CTBLEDITDLG_SIZE, long style = SYMBOL_CTBLEDITDLG_STYLE );

    /// Destructor
    ~CTblEditDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CTblEditDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_IMPORT_TABLE
    void OnImportTableClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EXPORT_TABLE
    void OnExportTableClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CLEAR_TABLE
    void OnClearTableClick( wxCommandEvent& event );

////@end CTblEditDlg event handler declarations

    void OnUpdateProblems( wxCommandEvent& event );

////@begin CTblEditDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTblEditDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CTblEditDlg member variables
    wxBoxSizer* m_GridHolder;
    wxBoxSizer* m_TableControlsSizer;
    wxButton* m_ImportBtn;
    wxButton* m_ExportBtn;
////@end CTblEditDlg member variables
    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();
    wxGrid* GetGrid() { return m_Grid->GetGrid(); }
private:
    objects::CSeq_entry_Handle m_TopSeqEntry;
    CTableCommandConverter *m_Converter;
    CSeqTableGridPanel* m_Grid;
    IWorkbench* m_Workbench;
    wxString m_SaveFileDir;
    wxString m_SaveFileName;
    wxString m_StaticLabel;
    string x_FindBadImportColumns (CRef<objects::CSeq_table> table);
};


class CDBLinkTableCommandConverter : public CTableFieldCommandConverter 
{
public:
    CDBLinkTableCommandConverter(vector<string> fields) 
      : m_DBLinkFields(fields)
      {};
    virtual ~CDBLinkTableCommandConverter() {};

    virtual CRef<objects::CSeq_table> GetValuesTableFromSeqEntry(objects::CSeq_entry_Handle seh);
    virtual CRef<CCmdComposite> GetCommandFromValuesTable(CRef<objects::CSeq_table>, objects::CSeq_entry_Handle seh);
    virtual CRef<CCmdComposite> ClearAllValues(objects::CSeq_entry_Handle seh);
    virtual string RemapColumnName(string column_name);
    virtual string GetErrorMessage(CRef<objects::CSeq_table> table);
private: 
    vector<string> m_DBLinkFields;
};


class CCommentDescriptorTableCommandConverter : public CTableFieldCommandConverter 
{
public:
    CCommentDescriptorTableCommandConverter(const string& label) : m_Label(label) {};
    virtual ~CCommentDescriptorTableCommandConverter() {};

    virtual CRef<objects::CSeq_table> GetValuesTableFromSeqEntry(objects::CSeq_entry_Handle seh);
    virtual CRef<CCmdComposite> GetCommandFromValuesTable(CRef<objects::CSeq_table> values_table, objects::CSeq_entry_Handle seh);
    virtual CRef<CCmdComposite> ClearAllValues(objects::CSeq_entry_Handle seh);
    virtual string RemapColumnName(string column_name);
    virtual string GetErrorMessage(CRef<objects::CSeq_table> values_table);
    virtual string CheckForMixedValues(CRef<objects::CSeq_table> table, string column_name = "");
private:
    string m_Label;
};


class CChimeraCommentDescriptorTableCommandConverter : public CTableFieldCommandConverter 
{
public:
    CChimeraCommentDescriptorTableCommandConverter(const string& label, CSourceRequirements::EWizardSrcType src_type) 
          : m_Label(label), m_SrcType (src_type) {};
    virtual ~CChimeraCommentDescriptorTableCommandConverter() {};

    virtual CRef<objects::CSeq_table> GetValuesTableFromSeqEntry(objects::CSeq_entry_Handle seh);
    virtual CRef<CCmdComposite> GetCommandFromValuesTable(CRef<objects::CSeq_table>, objects::CSeq_entry_Handle seh);
    virtual CRef<CCmdComposite> ClearAllValues(objects::CSeq_entry_Handle seh);
    virtual string RemapColumnName(string column_name);
    virtual string GetErrorMessage(CRef<objects::CSeq_table>);
    virtual string CheckForMixedValues(CRef<objects::CSeq_table> table, string column_name = "");
private:
    string m_Label;
    CSourceRequirements::EWizardSrcType m_SrcType;
};


class CMolInfoTableCommandConverter : public CTableFieldCommandConverter 
{
public:
    CMolInfoTableCommandConverter(CSourceRequirements::EWizardType wizard_type = CSourceRequirements::eWizardType_standard) : m_WizardType (wizard_type) {};
    virtual ~CMolInfoTableCommandConverter() {};

    virtual CRef<objects::CSeq_table> GetValuesTableFromSeqEntry(objects::CSeq_entry_Handle seh);
    virtual CRef<CCmdComposite> GetCommandFromValuesTable(CRef<objects::CSeq_table>, objects::CSeq_entry_Handle seh);
    CRef<CCmdComposite> GetCommandFromValuesTable(CRef<objects::CSeq_table> values_table, objects::CSeq_entry_Handle seh, bool add_confirmed);
    virtual CRef<CCmdComposite> ClearAllValues(objects::CSeq_entry_Handle seh);
    virtual string RemapColumnName(string column_name);
    virtual string GetErrorMessage(CRef<objects::CSeq_table>);
    virtual CRef<objects::CSeq_table> GetChoices(CRef<objects::CSeq_table> values_table);
private:
    CSourceRequirements::EWizardType m_WizardType;
};


class CGenomeTableCommandConverter : public CTableFieldCommandConverter 
{
public:
    CGenomeTableCommandConverter(CSourceRequirements::EWizardType wizard_type = CSourceRequirements::eWizardType_standard,
                                 CSourceRequirements::EWizardSrcType src_type = CSourceRequirements::eWizardSrcType_any)
                                 : m_WizardType (wizard_type) , m_SrcType (src_type) {};
    virtual ~CGenomeTableCommandConverter() {};

    virtual CRef<objects::CSeq_table> GetValuesTableFromSeqEntry(objects::CSeq_entry_Handle seh);
    virtual CRef<CCmdComposite> GetCommandFromValuesTable(CRef<objects::CSeq_table>, objects::CSeq_entry_Handle seh);
    virtual CRef<CCmdComposite> ClearAllValues(objects::CSeq_entry_Handle seh);
    virtual string RemapColumnName(string column_name);
    virtual string GetErrorMessage(CRef<objects::CSeq_table>);
    virtual CRef<objects::CSeq_table> GetChoices(CRef<objects::CSeq_table> values_table);
    virtual string CheckForMixedValues(CRef<objects::CSeq_table> values_table, string column_name = "");
private:
    CSourceRequirements::EWizardType m_WizardType;
    CSourceRequirements::EWizardSrcType m_SrcType;
};

class CPrimerTableCommandConverter : public CTableFieldCommandConverter 
{
public:
    CPrimerTableCommandConverter() {};
    virtual ~CPrimerTableCommandConverter() {};

    virtual CRef<objects::CSeq_table> GetValuesTableFromSeqEntry(objects::CSeq_entry_Handle seh);
    virtual CRef<CCmdComposite> GetCommandFromValuesTable(CRef<objects::CSeq_table>, objects::CSeq_entry_Handle seh);
    virtual CRef<CCmdComposite> ClearAllValues(objects::CSeq_entry_Handle seh);
    virtual string RemapColumnName(string column_name);
    virtual string GetErrorMessage(CRef<objects::CSeq_table>);
    virtual CRef<objects::CSeq_table> GetChoices(CRef<objects::CSeq_table> values_table);
    string  GetPrimerTypeColumnName();
    virtual string CheckForMixedValues(CRef<objects::CSeq_table> values_table, string column_name = "");
};

class CStructuredCommentCommandConverter : public CTableFieldCommandConverter
{
public:
    CStructuredCommentCommandConverter(string prefix, vector<string> fields)
        : m_Prefix (prefix), m_Fields(fields) {};
    virtual ~CStructuredCommentCommandConverter() {};
    virtual CRef<objects::CSeq_table> GetValuesTableFromSeqEntry(objects::CSeq_entry_Handle seh);
    virtual CRef<CCmdComposite> GetCommandFromValuesTable(CRef<objects::CSeq_table>, objects::CSeq_entry_Handle seh);
    virtual string GetErrorMessage(CRef<objects::CSeq_table>);
    virtual CRef<CCmdComposite> ClearAllValues(objects::CSeq_entry_Handle seh);
    virtual string RemapColumnName(string column_name);
private:
    string m_Prefix;
    vector<string> m_Fields;
};


class CFeatureTableCommandConverter : public CTableFieldCommandConverter
{
public:
    CFeatureTableCommandConverter(const objects::CSeq_feat& ftemplate, 
                                  const TFeatureSeqTableColumnList& reqs,
                                  const TFeatureSeqTableColumnList& opts);
    virtual ~CFeatureTableCommandConverter() {};
    virtual CRef<objects::CSeq_table> GetValuesTableFromSeqEntry(objects::CSeq_entry_Handle seh);
    virtual CRef<CCmdComposite> GetCommandFromValuesTable(CRef<objects::CSeq_table>, objects::CSeq_entry_Handle seh);
    virtual string GetErrorMessage(CRef<objects::CSeq_table>);
    virtual CRef<CCmdComposite> ClearAllValues(objects::CSeq_entry_Handle seh);
    virtual string RemapColumnName(string column_name);
    virtual CRef<objects::CSeq_table> GetChoices(CRef<objects::CSeq_table> values_table);
    virtual CRef<objects::CSeqTable_column> MakeProblemsColumn(CRef<objects::CSeq_table> values_table);
private:
    CRef<objects::CSeq_feat> m_Template;
    TFeatureSeqTableColumnList m_Reqs;
    TFeatureSeqTableColumnList m_Opts;
};


vector<string> GetAssemblyDataFields();

END_NCBI_SCOPE

#endif
    // _TBL_EDIT_DLG_H_
