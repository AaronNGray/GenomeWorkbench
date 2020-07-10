#ifndef GUI_WIDGETS_EDIT___FIELD_NAME_PANEL__HPP
#define GUI_WIDGETS_EDIT___FIELD_NAME_PANEL__HPP
/*  $Id: field_name_panel.hpp 42321 2019-01-31 21:06:40Z filippov $
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
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>

#include <wx/panel.h>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT CFieldNamePanelParent {
public:
    virtual void UpdateEditor() = 0;
    virtual ~CFieldNamePanelParent() {};
};


/*!
 * CFieldNamePanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CFieldNamePanel : public wxPanel
{    
public:
    /// Constructors
    CFieldNamePanel () {};

    /// Destructor
    ~CFieldNamePanel () {};
    ///Returns the name of the field as selected in the panel.
    ///In case of names composed of multiple words, for example, feature_name + qualifier 
    ///when the subfield parameter is true, the qualifier name is returned only.
    virtual string GetFieldName(const bool subfield = false) = 0;
    virtual bool SetFieldName(const string& field) = 0;
    virtual vector<string> GetChoices(bool& allow_other);
    virtual vector<string> GetMacroValues();
    virtual void ClearValues(void) {}
    virtual void SetShowLegalOnly(bool show) {}

    virtual string GetMacroFieldName(const string &target, const string& selected_field = kEmptyStr){return kEmptyStr;}
    virtual string GetMacroFieldLabel(const string& target, const string& selected_field = kEmptyStr) {return kEmptyStr;}
    enum EFieldType {
        eFieldType_Unknown = 0,
        eFieldType_Source,
        eFieldType_Feature,
        eFieldType_CDSGeneProt,
        eFieldType_RNA,
        eFieldType_Gene,
        eFieldType_Protein,
        eFieldType_MolInfo,
        eFieldType_Pub,
        eFieldType_StructuredComment,
        eFieldType_BankITComment,
        eFieldType_Comment,
        eFieldType_DBLink,
        eFieldType_Dbxref,
        eFieldType_Misc,
        eFieldType_DefLine,
        eFieldType_TaxnameAfterBinomial,
        eFieldType_FlatFile,
        eFieldType_FileId,
        eFieldType_GeneralId,
        eFieldType_LocalId,
        eFieldType_SeqId,
        eFieldType_Taxname
    };

    static string GetFieldTypeName(EFieldType field_type);
    static EFieldType GetFieldTypeFromName(const string & field_type_name);
    static bool IsFeature(const string& target);
    static bool IsDescriptor(const string& target);

protected:
    void x_UpdateParent(void);
    bool x_IsParentAECRDlg(void);
    void x_UpdateAECRDlgLayout(void);
    // returns true if the CStringConstraintPanel should be smaller in the parent dialog
    bool x_ShouldDecreaseStrConstrPanel(void);
};


END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___FIELD_NAME_PANEL__HPP
