#ifndef GUI_WIDGETS_EDIT___MACRO_TREEITEMDATA__HPP
#define GUI_WIDGETS_EDIT___MACRO_TREEITEMDATA__HPP
/*  $Id: macro_treeitemdata.hpp 44820 2020-03-23 14:59:16Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 *  Classes to be used in the macro editor storing user specific data
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/pub_fieldtype.hpp>
#include <gui/widgets/edit/macro_action_tree.hpp>
#include <gui/widgets/edit/macro_panel_loader.hpp>
#include <gui/widgets/edit/macro_editor_context.hpp>

#include <wx/treebase.h>
#include <wx/sizer.h>

BEGIN_NCBI_SCOPE


/// Interface class for providing the parts of a macro script
///
class IMacroScriptElements
{
public:
    using TConstraints = vector<pair<string, string>>;

    virtual ~IMacroScriptElements() {}
    virtual string GetMacroDescription() const = 0;

    virtual bool HasVariables() const = 0;
    virtual string GetVariables() = 0;

    virtual string GetFunction(TConstraints& constraints) const = 0;
};



class CMIteratorItemData : public wxTreeItemData
{
public:
    CMIteratorItemData(const CTempString& target) : m_Target(target) {}
    const CTempString& GetTarget() const { return m_Target; }
private:
    CTempString m_Target;
};

class IMacroActionItemData : public wxTreeItemData, public IMacroScriptElements
{
public:
    using TArguments = CArgumentList::TArguments;

    virtual ~IMacroActionItemData() {}

    void CreateParamPanel(wxStaticBoxSizer& action_sizer, wxPanel* parent);
    void DeleteParamPanel();

    const string& GetGeneralDescription() const { return m_Description; }

    /// @name IMacroScriptElements interface implementation
    /// @{
    virtual string GetMacroDescription() const { return m_Description; }
    virtual bool HasVariables() const { return false; }
    virtual string GetVariables() { return kEmptyStr; }
    virtual string GetFunction(TConstraints& constraints) const = 0;
    virtual const vector<string>& GetKeywords() const { return m_Keywords; }
    /// @}

    void SetTarget(const CTempString& target) { m_Target = target; }
    const pair<string, string> GetTarget() const { return make_pair(m_Target, m_SelectedField); }
    virtual bool UpdateTarget() { return false; }
    virtual void UpdateTarget(const TConstraints& constraints) {}

    const string& GetWarning() const { return m_Warning; }
    void ResetWarning() { m_Warning.clear(); }

    bool TransferDataFromPanel();

protected:
    IMacroActionItemData(const string& description) : m_Description(description) {}
    
    void x_LoadPanel(wxWindow* parent, const string &panel_name, const TArgumentsVector &arguments);
    string x_GetVariables(const vector<CTempString>& args) const;

    string m_Description;

    CTempString m_Target;
    // this may be different from m_Target
    // it's used to build constraints
    string m_SelectedField; 

    unique_ptr<CMacroPanelLoader> m_Panel{ nullptr }; ///< constructed panel for specifying function arguments
    CArgumentList m_ArgList;
    TArguments m_Args;
    vector<string> m_Keywords;
    mutable string m_Warning;

private:
    // implement this function in subclasses if panel is needed for the action
    virtual void x_AddParamPanel(wxWindow* parent) = 0;
};

class CMacroActionSimpleItemData : public IMacroActionItemData
{
public:
    CMacroActionSimpleItemData(const string& func_name, const string& description)
        : IMacroActionItemData(description), m_FuncName(func_name) {}

    /// @name IMacroScriptElements interface implementation
    /// @{
    virtual string GetFunction(TConstraints& constraints) const { return m_FuncName + "();"; }
    /// @}
private:
    virtual void x_AddParamPanel(wxWindow* parent);
    const string m_FuncName;
};

class CMacroActionItemData : public IMacroActionItemData
{
protected:
    CMacroActionItemData(const string& description, EMacroFieldType type = EMacroFieldType::eNotSet)
        : IMacroActionItemData(description), m_FieldType(type) {}

public:
    virtual ~CMacroActionItemData() {}
    virtual const vector<string>& GetKeywords() const;

protected:
    mutable EMacroFieldType m_FieldType;  ///< for actions such as appply/edit/etc.
};

class CMTreeItemDataBuilder
{
public:
    CMTreeItemDataBuilder()
    {
        x_Init();
    }
    wxTreeItemData* GetTreeItemData(const CMActionNode& node) const;
private:
    void x_Init();
    map<EMActionType, wxTreeItemData*> m_ActionMap;
};


namespace NMItemData {
    using TConstraints = IMacroScriptElements::TConstraints;

    void WrapInQuotes(string& str);
    void GetFeatureAndField(const string& field_name, string& feature, string& field);
    void UpdateFeatureType(string& feature, const string& field);
    void NCBI_GUIWIDGETS_EDIT_EXPORT GetRNASpecificFields(const string& field_name, string& rna_type, string& ncRNA_type, string& rna_qual);

    /// Generates the definition of rt_var based on input "field"
    string GetResolveFuncForQual(const string& container, const string& field, const string& rt_var, TConstraints& constraints, bool remove_constraint = true);

    string GetEditableFieldForPair(const string& field, const string& rt_var);

    string GetResolveFuncForPubQual(const string& field, string& rt_var, TConstraints& constraints);
    string GetResolveFuncForPubQualConst(const string& field, const string& rt_var, TConstraints& constraints, EMacroFieldType field_type);
    string GetResolveFuncForApplyAuthors(CPubFieldType::EPubFieldType type, const string& rt_var, TConstraints& constraints);
    string GetResolveFuncForDBLink(const string& field, string& rt_var, TConstraints& constraints, bool& found_constraint);
    string GetResolveFuncForDbXref(const string& path, const string& rt_var, TConstraints& constraints);
    // function for structured voucher part
    string GetResolveFuncForSVPart(const string& field, const string& rt_var, TConstraints& constraints, bool& found_constraint);

    string GetResolveFuncForMultiValuedQual(const string& field, const string& rt_var, TConstraints& constraints, bool remove_constraint);
    // pair<resolve_function, variable_name/path_to_asn_member>
    pair<string, string> GetResolveFuncForMultiValuedQuals(EMacroFieldType type, const string& fieldname, const string& rt_var, TConstraints& constraints, bool remove_constraint);
    pair<string, string> GetResolveFuncForFeatQuals(const string& feature, const string& field, const string& rt_var, TConstraints& constraints, bool remove_constraint);
    pair<string, string> GetResolveFuncForCdsGeneProtQuals(const string& fieldname, const string& rt_var, TConstraints& constraints, bool remove_constraint);
    pair<string, string> GetResolveFuncForRNAQuals(const string& fieldname, const string& rt_var, TConstraints& constraints, bool remove_constraint);
    pair<string, string> GetResolveFuncForSwapQuals(EMacroFieldType fieldtype, const string& fieldname, const string& rt_var, 
                                                    TConstraints& constraints, bool remove_constraint, bool& is_gbqual);
    pair<string, string> GetResolveFuncForSwapFeatQuals(const string& feature, const string& field, const string& rt_var,
                                                    TConstraints& constraints, bool remove_constraint, bool& is_gbqual);


    bool MultipleValuesAllowed(const string& field);
    bool IsGeneQualifier(const string& field);
    bool IsGeneField(const string& field_name);

    string GetResolveFuncForStrCommQual(const string& field, const string& field_name, const string& rt_var, TConstraints& constraints);

    string GetResolveFuncForGeneQuals(const string& container, const string& field, const string& rt_var, TConstraints& constraints, bool remove_constraint = true);


    string GetFuncFromTable_Depr(const string& rt_var, const string& filename, const string& col, TConstraints& constraints, string& warning);
    string GetFuncFromTable(const string& rt_var, const string& filename, const string& col, TConstraints& constraints, string& warning);

    string GetDelimiterDescr(const string& del);
    string GetExistingTextDescr(const CArgumentList& arg_list);
    string GetHandleOldValue(const string& type);
    string GetDelimiterChar(const string& del);
    string GetHandleBlank(const string& str);
    string GetStructCommQualDescr(const CArgumentList& arg_list);
    enum EDirType {
        eFrom,
        eTo
    };
    EMacroFieldType GetSourceTypeInSinglePanel(const CArgumentList& arg_list);
    EMacroFieldType GetSourceTypeInDoublePanel(const CArgumentList& arg_list, EDirType dir);
    string GetSelectedCapType(const CArgumentList& arg_list);

    CTempString UpdateTargetForFeatures(const string& feature, const string& qualifier, bool for_removal = false);
    void UpdateConstraintsForFeatures(TConstraints& constraints, const string& feature);
    CTempString UpdateTargetForDualFeatures(const string& feature, const string& from_field);

    CTempString UpdateTargetForRnas(const string& rna_type);
    void UpdateConstraintsForRnas(TConstraints& constraints, const CArgumentList& arg_list);
    void UpdateConstraintsForRnas(TConstraints& constraints, const string& rna_type, const string& ncRNA_class);
    CTempString UpdateTargetForDualRnas(const string& rna_type, const string& from_field);

    CTempString UpdateTargetForCdsGeneProt(const string& fieldname);
    void UpdateConstraintsForDualCdsGeneProt(TConstraints& constraints, const string& src_feat);

    CTempString UpdateTargetForMolinfo(const string& molinfo_type);

    CTempString UpdateTargetForMisc(const string& fieldname);
    void UpdateConstraintsForMisc(TConstraints& constraints, const string& fieldname, const string& target);
    
    void AddConstraintIgnoreNewText(const string& field, TConstraints& constraints, const CArgumentList& arg_list, const string& target = kEmptyStr);

    string GetUpdatemRNADescription(const CArgument& arg);
    string GetUpdatemRNAVar(const CArgumentList& arg_list);
    void AppendUpdatemRNAFnc(bool update_mrna, string& function);

    void OnExistingTextChanged(CArgumentList& list, CArgument& arg);
    void OnProteinNameSingle(CArgumentList& list, CArgument& arg);
    void OnProteinNameDouble(CArgumentList& list, CArgument& arg);
    void OnProductDouble(CArgumentList& list, CArgument& arg);
    void OnRnaTypeChanged(CArgumentList& list, CArgument& arg);
    void OnControlsAddRNAPanelSelected(CArgumentList& list, CArgument& arg);
    void OnMiscFeatRuleSelected(CArgumentList& list, CArgument& arg);
    void OnIllegalDbXrefSelected(CArgumentList& list, CArgument& arg);

    void OnRemoveFromParsedField(CArgumentList& list, CArgument& arg);
    void OnLeftDigitsLettersInParsePanels(CArgumentList& list, CArgument& arg);
    void OnRightDigitsLettersInParsePanels(CArgumentList& list, CArgument& arg);
    void OnBeforeCheck(CArgumentList& list, CArgument& arg);
    void OnAfterCheck(CArgumentList& list, CArgument& arg);

    void OnStructCommFieldChanged(CArgumentList& list, CArgument& arg);
    void OnStructCommFieldDblChanged(CArgumentList& list, CArgument& arg);
    void OnConvertFeatFromCDS(CArgumentList& list, CArgument& arg);
    void OnConvertFeatToChanged(CArgumentList& list, CArgument& arg);

    void OnSelectTable(CArgumentList& list, CArgument& arg);
    void OnSelectFeature(CArgumentList& list, CArgument& arg);
}


END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_TREEITEMDATA__HPP
