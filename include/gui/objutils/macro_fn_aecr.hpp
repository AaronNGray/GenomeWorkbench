#ifndef GUI_OBJUTILS___MACRO_FN_AECR__HPP
#define GUI_OBJUTILS___MACRO_FN_AECR__HPP
/*  $Id: macro_fn_aecr.hpp 44728 2020-02-28 21:45:18Z asztalos $
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
 * Authors: Andrea Asztalos
 *
 * File Description: 
 *   Macro editing functions that correspond to the operations from the AECR dialog
 *
 */

/// @file macro_fn_aecr.hpp

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objtools/edit/string_constraint.hpp>
#include <objtools/edit/parse_text_options.hpp>
#include <objtools/cleanup/capitalization_string.hpp>
#include <gui/objutils/macro_util.hpp>
#include <gui/objutils/macro_edit_fn_base.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CScope;
    class CSeq_feat;
    class CSeq_entry_Handle;
    class CBioseq_set_Handle;
END_SCOPE(objects)
BEGIN_SCOPE(macro)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_SwapRelatedFeaturesQual_Depr)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_SetRnaProduct)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_GetRnaProduct)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_RemoveRnaProduct)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_EditSubfield)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_RemoveSubfield)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_SetCodonsRecognized)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_SetStructCommField)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_RmvRelFeatQual)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_LocalID)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_RemoveModifier)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_ValueFromTable)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_GeneQual)

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_SwapQual : public IEditMacroFunction
{
public:
    CMacroFunction_SwapQual(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual ~CMacroFunction_SwapQual() {};
    virtual void TheFunction();
    static CTempString GetFuncName();

    static bool s_SwapFields(CObjectInfo& src, CObjectInfo& dest);
protected:
    virtual bool x_ValidArguments() const;
    bool x_DoFieldsExist(CObjectInfo oi, CMQueryNodeValue::TObs& result, size_t index);
    bool x_SetFields(CObjectInfo oi, CMQueryNodeValue::TObs& result, size_t index);
    bool x_SwapFields(CObjectInfo& src, CObjectInfo& dest);
    void x_CopyFields(CMQueryNodeValue::TObs& src_objs, CMQueryNodeValue::TObs& dest_objs);
    bool x_CopyFields(CObjectInfo& src, CObjectInfo& dest);

    void x_SetField(CConstRef<CObject>& obj, const string& fieldname, CMQueryNodeValue::TObs& result);
};
 
class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_SwapRelFeatQual : public CMacroFunction_SwapQual
{
public:
    CMacroFunction_SwapRelFeatQual(EScopeEnum func_scope)
        : CMacroFunction_SwapQual(func_scope) {}

    virtual ~CMacroFunction_SwapRelFeatQual() {}
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments(void) const;
    bool x_DoDestFeatFieldsExist(
        const objects::CSeq_feat& src_feat, CMQueryNodeValue::TObs& result, const string& dest_feattype, const string& dest_field);
    void x_SetOrCreateDestFeatFields(const objects::CSeq_feat& src_feat, CMQueryNodeValue::TObs& result, size_t index);

    CConstRef<objects::CSeq_feat> m_ConstDestFeat{ nullptr };
    CRef<objects::CSeq_feat> m_EditDestFeat{ nullptr };
    CRef<objects::CSeq_feat> m_CreatedFeat{ nullptr };
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_SetStringQual : public IEditMacroFunction
{
public:
    CMacroFunction_SetStringQual(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual ~CMacroFunction_SetStringQual() {}
    virtual void TheFunction();
    static CTempString GetFuncName();

protected:
    virtual bool x_ValidArguments(void) const;
    void x_SetFields(CMQueryNodeValue::TObs& objs, const string& newValue,
        objects::edit::EExistingText existing_text, vector<string>& new_values);
    void x_SetNewPrimitiveValue(CObjectInfo& oi, const string& newValue,
        objects::edit::EExistingText existing_text, vector<string>& new_values);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_RemoveQual : public IEditMacroFunction
{
public:
    CMacroFunction_RemoveQual(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual ~CMacroFunction_RemoveQual() {}
    virtual void TheFunction();
    static CTempString GetFuncName();
    // changes the state of the data iterator
    static Int4 s_RemoveFields(CIRef<IMacroBioDataIter>& dataiter, CMQueryNodeValue::TObs& objs);
protected:
    virtual bool x_ValidArguments(void) const;
};


class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_EditStringQual : public IEditMacroFunction
{
public:
    enum ESearchLoc {
        eAnywhere,   // default value
        eBeginning,
        eEnd
    };

    CMacroFunction_EditStringQual(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual ~CMacroFunction_EditStringQual() {}
    virtual void TheFunction();
    static bool s_EditText(string& str, const string& find, const string& repl, ESearchLoc loc, bool case_sensitive, bool is_regex = false);
    static ESearchLoc s_GetLocFromName(const string& name);
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments(void) const;
    void x_EditFields(CMQueryNodeValue::TObs& resolved_objs, const string& find_txt, const string& repl_txt, ESearchLoc loc, bool case_sensitive, bool is_regex);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_SetRelFeatQual : public CMacroFunction_SetStringQual
{
public:
    CMacroFunction_SetRelFeatQual(EScopeEnum func_scope)
        : CMacroFunction_SetStringQual(func_scope) {}

    virtual ~CMacroFunction_SetRelFeatQual() {}
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments(void) const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_SetQual : public IEditMacroFunction
{
public:
    CMacroFunction_SetQual(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual ~CMacroFunction_SetQual() {}
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments(void) const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_AddDBLink : public IEditMacroFunction
{
public:
    CMacroFunction_AddDBLink(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual ~CMacroFunction_AddDBLink() {}
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments(void) const;
    void x_AddNewUserField(CRef<objects::CSeqdesc>& user_object_desc, const string& dblink, const string& newValue);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_EditRelFeatQual : public CMacroFunction_EditStringQual
{
public:
    CMacroFunction_EditRelFeatQual(EScopeEnum func_scope)
        : CMacroFunction_EditStringQual(func_scope) {}

    virtual ~CMacroFunction_EditRelFeatQual() {}
    virtual void TheFunction();
    static const char* sm_FunctionName;
protected:
    virtual bool x_ValidArguments(void) const;
};

class IOperateOnTwoQuals : public IEditMacroFunction
{
public:
    virtual ~IOperateOnTwoQuals() {}
    void ChangeFields(CMQueryNodeValue::TObs& src_objs, CMQueryNodeValue::TObs& dest_objs);
protected:
    IOperateOnTwoQuals(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    bool x_GetSourceFields(CObjectInfo oi, size_t index, CMQueryNodeValue::TObs& result);
    bool x_GetDestFields(CObjectInfo oi, size_t index, CMQueryNodeValue::TObs& result);
    string x_GetSourceString(CObjectInfo& src);
    objects::edit::EExistingText m_ExistingText{ objects::edit::eExistingText_cancel };
    objects::ECapChange m_CapChange{ objects::eCapChange_none };
private:
    virtual bool x_ChangeFields(CObjectInfo& src, CObjectInfo& dest) = 0;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ConvertStringQual : public IOperateOnTwoQuals
{
public:
    CMacroFunction_ConvertStringQual(EScopeEnum func_scope)
        : IOperateOnTwoQuals(func_scope) {}

    virtual ~CMacroFunction_ConvertStringQual() {}
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments(void) const;
    string x_GetSourceString(CObjectInfo& src);
private:
    virtual bool x_ChangeFields(CObjectInfo& src, CObjectInfo& dest);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ConvertFeatQual : public CMacroFunction_ConvertStringQual
{
public:
    CMacroFunction_ConvertFeatQual(EScopeEnum func_scope)
        : CMacroFunction_ConvertStringQual(func_scope) {}

    ~CMacroFunction_ConvertFeatQual() {}
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments(void) const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ParseStringQual : public IOperateOnTwoQuals
{
public:
    CMacroFunction_ParseStringQual(EScopeEnum func_scope)
        : IOperateOnTwoQuals(func_scope) {}

    virtual ~CMacroFunction_ParseStringQual() {}
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments(void) const;
    string x_GetSourceString(CObjectInfo& src);
private:
    virtual bool x_ChangeFields(CObjectInfo& src, CObjectInfo& dest);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ParseFeatQual : public CMacroFunction_ParseStringQual
{
public:
    CMacroFunction_ParseFeatQual(EScopeEnum func_scope)
        : CMacroFunction_ParseStringQual(func_scope) {}

    ~CMacroFunction_ParseFeatQual() {}
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments(void) const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_AddParsedText : public IEditMacroFunction
{
public:
    CMacroFunction_AddParsedText(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual ~CMacroFunction_AddParsedText() {}
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
    void x_ParseFields(CMQueryNodeValue::TObs& dest_objs, const string& text_portion, objects::edit::EExistingText existing_text);
};


class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_AddParsedToFeatQual : public CMacroFunction_AddParsedText
{
public:
    CMacroFunction_AddParsedToFeatQual(EScopeEnum func_scope)
        : CMacroFunction_AddParsedText(func_scope) {}

    ~CMacroFunction_AddParsedToFeatQual() {}
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments(void) const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ParsedText : public IEditMacroFunction
{
public:
    CMacroFunction_ParsedText(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual ~CMacroFunction_ParsedText() {};
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
    bool x_GetSpecialField(const string& field, CMQueryNodeValue::TObs& objs, objects::CScope& scope);
    CRef<objects::edit::CParseTextOptions> x_GetParseOptions();
    vector<pair<objects::CSeqdesc_CI, CRef<objects::CSeqdesc>>> m_ChangedDescriptors;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ParseToStructComm : public IEditMacroFunction
{
public:
    CMacroFunction_ParseToStructComm(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual ~CMacroFunction_ParseToStructComm() {}
    virtual void TheFunction();
    static CTempString GetFuncName();

protected:
    virtual bool x_ValidArguments(void) const;
private:
    enum EFieldType {
        eFieldValue,
        eDbName,
        eFieldName,
        eFieldError
    };

    void x_SetFieldType(const string& strcomm_field);
    EFieldType m_Type;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_CopyStringQual : public IOperateOnTwoQuals
{
public:
    CMacroFunction_CopyStringQual(EScopeEnum func_scope)
        : IOperateOnTwoQuals(func_scope) {}

    virtual ~CMacroFunction_CopyStringQual() {}
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments(void) const;
private:
    virtual bool x_ChangeFields(CObjectInfo& src, CObjectInfo& dest);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_CopyFeatQual : public CMacroFunction_CopyStringQual
{
public:
    CMacroFunction_CopyFeatQual(EScopeEnum func_scope)
        : CMacroFunction_CopyStringQual(func_scope) {}

    ~CMacroFunction_CopyFeatQual() {}
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments(void) const;
};


class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_RemoveOutside : public IEditMacroFunction
{
public:
    CMacroFunction_RemoveOutside(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual ~CMacroFunction_RemoveOutside() {}
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;

    void x_RmvOutsideFields(CMQueryNodeValue::TObs& resolved_objs, const CRemoveTextOptions& options);
    CRef<CRemoveTextOptions> x_GetRemoveTextOptions(size_t start_index) const;
    bool x_CheckArguments(size_t index) const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_RmvOutsideRelFeatQual : public CMacroFunction_RemoveOutside
{
public:
    CMacroFunction_RmvOutsideRelFeatQual(EScopeEnum func_scope)
        : CMacroFunction_RemoveOutside(func_scope) {}

    virtual ~CMacroFunction_RmvOutsideRelFeatQual() {}
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments(void) const;
};

class CMacroFunction_AddorSetContElement : public IEditMacroFunction
{
public:
    virtual ~CMacroFunction_AddorSetContElement() {}

    virtual void TheFunction();
    static bool s_AddSrcOrgMod(objects::CBioSource& bsrc, int subtype, const string& value);
    static bool s_AddSrcSubSource(objects::CBioSource& bsrc, int subtype, const string& value);
    static int s_AddGBQualifier(objects::CSeq_feat& feat, const string& qual_name, const string& newValue);
protected:
    CMacroFunction_AddorSetContElement(EScopeEnum func_scope, bool is_bsrc)
        : IEditMacroFunction(func_scope), m_IsBiosrc(is_bsrc) {}

    virtual bool x_ValidArguments() const;
    void x_AddBioSrcModifier(const string& subtype, const string& newValue);
    bool m_IsBiosrc;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_AddorSetBsrcModifier : public CMacroFunction_AddorSetContElement
{
public:
    CMacroFunction_AddorSetBsrcModifier(EScopeEnum func_scope)
        : CMacroFunction_AddorSetContElement(func_scope, true) {}

    ~CMacroFunction_AddorSetBsrcModifier() {}
    static CTempString GetFuncName();
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_AddorSetGbQual : public CMacroFunction_AddorSetContElement
{
public:
    CMacroFunction_AddorSetGbQual(EScopeEnum func_scope)
        : CMacroFunction_AddorSetContElement(func_scope, false) {}

    ~CMacroFunction_AddorSetGbQual() {}
    static CTempString GetFuncName();
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_SetStructCommDb : public IEditMacroFunction
{
public:
    CMacroFunction_SetStructCommDb(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    ~CMacroFunction_SetStructCommDb() {}

    virtual void TheFunction();
    static CTempString GetFuncName();
    static int s_UpdateStructCommentDb(objects::CSeqdesc& seqdesc, const string& field, const string& db_name, objects::edit::EExistingText existing_text);
    static int s_UpdateStructCommentDb(objects::CUser_object& user, const string& field, const string& db_name, objects::edit::EExistingText existing_text);
    static void s_MoveSuffixToTheEnd(objects::CUser_object& user_object);
protected:
    virtual bool x_ValidArguments() const;
    // creates a new user object descriptor
    bool x_MakeNewStructuredComment(const string& db_name, const string& kPrefix, const string& kSuffix);
};

class CMacroFunction_SatelliteFields : public IEditMacroFunction
{
public:
    enum ESatelliteFieldType {
        eSatelliteType,
        eSatelliteName
    };

    virtual ~CMacroFunction_SatelliteFields() {}
    virtual void TheFunction();
    static string s_GetSatelliteType(const string& value);
    static string s_GetSatelliteName(const string& value);
    static string s_MakeSatellite(const string& type, const string& name);

protected:
    CMacroFunction_SatelliteFields(EScopeEnum func_scope, ESatelliteFieldType type)
        : IEditMacroFunction(func_scope), m_FieldType(type) {}

    virtual bool x_ValidArguments() const;
    ESatelliteFieldType m_FieldType;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_SatelliteType : public CMacroFunction_SatelliteFields
{
public:
    CMacroFunction_SatelliteType(EScopeEnum func_scope)
        : CMacroFunction_SatelliteFields(func_scope, CMacroFunction_SatelliteFields::eSatelliteType) {}
    static CTempString GetFuncName();
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_SatelliteName : public CMacroFunction_SatelliteFields
{
public:
    CMacroFunction_SatelliteName(EScopeEnum func_scope)
        : CMacroFunction_SatelliteFields(func_scope, CMacroFunction_SatelliteFields::eSatelliteName) {}
    static CTempString GetFuncName();
};


class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_MobileElementTypeFields : public IEditMacroFunction
{
public:
    enum EMobileElemTFieldType {
        eMobileElemTType,
        eMobileElemTName
    };

    virtual ~CMacroFunction_MobileElementTypeFields() {}
    virtual void TheFunction();
    static string s_GetMobileElemTType(const string& value);
    static string s_GetMobileElemTName(const string& value);
    static string s_MakeMobileElementT(const string& type, const string& name);

protected:
    CMacroFunction_MobileElementTypeFields(EScopeEnum func_scope, EMobileElemTFieldType type)
        : IEditMacroFunction(func_scope), m_FieldType(type) {}

    virtual bool x_ValidArguments() const;
    EMobileElemTFieldType m_FieldType;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_MobileElementType : public CMacroFunction_MobileElementTypeFields
{
public:
    CMacroFunction_MobileElementType(EScopeEnum func_scope)
        : CMacroFunction_MobileElementTypeFields(func_scope, CMacroFunction_MobileElementTypeFields::eMobileElemTType) {}
    static CTempString GetFuncName();
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_MobileElementName : public CMacroFunction_MobileElementTypeFields
{
public:
    CMacroFunction_MobileElementName(EScopeEnum func_scope)
        : CMacroFunction_MobileElementTypeFields(func_scope, CMacroFunction_MobileElementTypeFields::eMobileElemTName) {}
    static CTempString GetFuncName();
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ApplyStrucVoucherPart : public IEditMacroFunction
{
public:
    CMacroFunction_ApplyStrucVoucherPart(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual void TheFunction();
    static CTempString GetFuncName();
    
protected:
    virtual bool x_ValidArguments() const;
    void x_ApplyPart(objects::COrgMod& orgmod,  NMacroUtil::EStructVoucherPart part, const string& newValue, objects::edit::EExistingText existing_text);
    void x_RemovePart(objects::COrgMod& orgmod, NMacroUtil::EStructVoucherPart part);
    CRef<objects::COrgMod> x_AddNewQual(objects::COrgMod::TSubtype subtype, NMacroUtil::EStructVoucherPart part, const string& newValue);
};

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif
    // GUI_OBJUTILS___MACRO_FN_AECR__HPP
