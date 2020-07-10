#ifndef GUI_OBJUTILS___MACRO_FN_PUBFIELDS__HPP
#define GUI_OBJUTILS___MACRO_FN_PUBFIELDS__HPP
/*  $Id: macro_fn_pubfields.hpp 44987 2020-05-04 15:47:41Z asztalos $
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
 * Macro functions resolving publications fields
 *
 */

/// @file macro_fn_pubfields.hpp
#include <corelib/ncbistd.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/biblio/Title.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/submit/Submit_block.hpp>
#include <objects/general/Name_std.hpp>
#include <objects/mla/mla_client.hpp>
#include <objects/macro/Pub_type.hpp>
#include <objects/macro/Publication_field.hpp>
#include <objtools/edit/string_constraint.hpp>

#include <gui/objutils/macro_edit_fn_base.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_SetPubCitation)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_SetSerialNumber)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_RemovePubAuthorMI)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_SetPubPMID)

DECLARE_FUNC_CLASS_WITH_ARGS(CMacroFunction_ApplyPmidToEntry)

DECLARE_FUNC_CLASS_WITH_ARGS(CMacroFunction_ApplyDOIToEntry)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_DOILookup)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_RemovePubAuthors)

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_PubFields : public IEditMacroFunction
{
public:
    CMacroFunction_PubFields(EScopeEnum func_scope, objects::EPublication_field field);

    virtual ~CMacroFunction_PubFields() {}
    virtual void TheFunction();

    static void s_GetObjectsFromTitle(const CObjectInfo& title, CMQueryNodeValue::TObs& objs);
    static void s_GetObjectsFromImprint(const CObjectInfo& imp, const string& field_name, CMQueryNodeValue::TObs& objs);
    static void s_GetObjectsFromDate(const CObjectInfo& date, const string& field_name, CMQueryNodeValue::TObs& objs);
    static void s_GetObjectsFromAffil(const CObjectInfo& affil, const string& field_name, CMQueryNodeValue::TObs& objs);
    static void s_GetObjectsFromAuthListNames(const CObjectInfo& names, const string& field_name, CMQueryNodeValue::TObs& objs);
    static void s_GetObjectsFromPersonID(const CObjectInfo& names, const string& field_name, CMQueryNodeValue::TObs& objs);
    static void s_CopyResolvedObjs(const CMQueryNodeValue::TObs& objs_from, CMQueryNodeValue::TObs& objs_to);

    // types for defining the Pubclass 
    typedef pair< objects::CPub::E_Choice, objects::EPub_type >     TPubChoice_Type;
    typedef pair< TPubChoice_Type, unsigned int > TPubChoiceType_Int;
    typedef map< TPubChoiceType_Int, const char*> MapPubClass;

    // functions to obtain the publication class
    static void s_Init_PubClassMap(MapPubClass& map);
    static objects::EPub_type s_GetPubMLStatus (const objects::CPub& pub);
    static void s_SetStatus(const objects::CImprint& imp, const objects::CPub& pub, objects::EPub_type& status);

    static const char* sm_PubTitle;
    static const char* sm_PubAffil;
    static const char* sm_PubAuthors;
    static const char* sm_PubCit;
    static const char* sm_PubDate;
    static const char* sm_PubIssue;
    static const char* sm_PubJournal;
    static const char* sm_PubPages;
    static const char* sm_PubPMID;
    static const char* sm_PubClass;
    static const char* sm_PubSerialNumber;
    static const char* sm_PubVolume;

protected:
    virtual bool x_ValidArguments() const;
    virtual void x_ResetState() { m_ResField = kEmptyStr; }
private:
    void x_GetPubFieldObjectFromPub(const CObjectInfo& pub_oi, CMQueryNodeValue::TObs& objs);
    void x_GetObjectsForPubField_Cit(const objects::CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs);
    void x_GetObjectsForPubField_Title(const objects::CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs);
    void x_GetObjectsForPubField_SerialNumber(const objects::CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs);
    void x_GetObjectsForPubField_Journal(const objects::CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs);
    void x_GetObjectsForPubField_VolIssuePage(const objects::CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs);
    void x_GetObjectsForPubField_Date(const objects::CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs);
    void x_GetObjectsForPubField_AffilField(const objects::CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs);
    void x_GetObjectsForPubField_PMID(objects::CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs);
    void x_GetObjectsForPubField_Authors(const objects::CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs);
    string x_GetPubClassFromPub(const CObjectInfo& pub_var);
    void x_GetPubFieldObjectFromSubmitBlock(const CObjectInfo& block_oi, CMQueryNodeValue::TObs& objs);
    
private:

    objects::EPublication_field m_FieldType;
    /// used for resolving fields for authors, date, affiliation
    string m_ResField;  
};


class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ApplyPublication : public IEditMacroFunction
{
public:
    CMacroFunction_ApplyPublication(EScopeEnum func_scope, objects::CPub::E_Choice pub_type)
        : IEditMacroFunction(func_scope), m_PubType(pub_type) {}
    virtual ~CMacroFunction_ApplyPublication() {}
    virtual void TheFunction();
    static const char* sm_FunctionName;
protected:
    virtual bool x_ValidArguments() const;
    objects::CPub::E_Choice m_PubType;
};


class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_AuthorFix : public IEditMacroFunction
{
public:
    enum EActionType {
        eAuthorFixNotSet,
        eMoveMiddleName,
        eStripSuffix,
        eReverseNames,
        eTruncateMI
    };
    
    virtual ~CMacroFunction_AuthorFix() {}
    virtual void TheFunction();

    static const string& GetDescription(EActionType fix_type);
    static EActionType GetActionType(const string& descr);

    static bool s_MoveMiddleToFirst(objects::CName_std& name);
    static bool s_TruncateMiddleInitials(objects::CName_std& name);
    static bool s_FixInitials(objects::CName_std& name);
    static bool s_ReverseAuthorNames(objects::CName_std& name);

    static string s_GetFirstNameInitials(const string& first_name);
    static string s_InsertInitialPeriods(const string& orig);
    static string s_GetFirstNameInitialsWithoutStops(const string& first_name);

    static void s_BuildName(const string& firstname, const string& mid_initials,
                            const string& lastname, const string& suffix, objects::CName_std& name);
protected:
    CMacroFunction_AuthorFix(EScopeEnum func_scope, EActionType type)
        : IEditMacroFunction(func_scope), m_FixType(type) {}

    virtual bool x_ValidArguments() const;
    int x_MakeAuthorChanges(objects::CAuth_list& auth_list);

    EActionType m_FixType;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_MoveMiddleName : public CMacroFunction_AuthorFix
{
public:
    CMacroFunction_MoveMiddleName(EScopeEnum func_scope)
        :CMacroFunction_AuthorFix(func_scope, CMacroFunction_AuthorFix::eMoveMiddleName) {}
    ~CMacroFunction_MoveMiddleName() {}
    static CTempString GetFuncName();
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_StripSuffix : public CMacroFunction_AuthorFix
{
public:
    CMacroFunction_StripSuffix(EScopeEnum func_scope)
        :CMacroFunction_AuthorFix(func_scope, CMacroFunction_AuthorFix::eStripSuffix) {}
    ~CMacroFunction_StripSuffix() {}
    static CTempString GetFuncName();
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_TruncateMI : public CMacroFunction_AuthorFix
{
public:
    CMacroFunction_TruncateMI(EScopeEnum func_scope)
        :CMacroFunction_AuthorFix(func_scope, CMacroFunction_AuthorFix::eTruncateMI) {}
    ~CMacroFunction_TruncateMI() {}
    static CTempString GetFuncName();
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ReverseAuthNames : public CMacroFunction_AuthorFix
{
public:
    CMacroFunction_ReverseAuthNames(EScopeEnum func_scope)
        :CMacroFunction_AuthorFix(func_scope, CMacroFunction_AuthorFix::eReverseNames) {}
    ~CMacroFunction_ReverseAuthNames() {}
    static CTempString GetFuncName();
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ISOJTALookup : public IEditMacroFunction
{
public:
    CMacroFunction_ISOJTALookup(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}
    
    ~CMacroFunction_ISOJTALookup() {}

    virtual void TheFunction();
    static CTempString GetFuncName();
    static string s_GetISOShortcut(const string& old_title);
protected:
    virtual bool x_ValidArguments() const;
    void x_LookupTitleInJournal(objects::CCit_jour& journal);
    bool x_TitleToISO_Jta(objects::CTitle::C_E& title_type);

    objects::CMLAClient m_MLAClient;
    // for logging
    vector<pair<string, string>> m_TitlePairs;
    map<string, vector<string>> m_Hits;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_LookupPub : public IEditMacroFunction
{
public:
    CMacroFunction_LookupPub(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    ~CMacroFunction_LookupPub() {}

    virtual void TheFunction();
    static CTempString GetFuncName();

    static CRef<objects::CPub> s_GetArticleFromEntrezById(int id);
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_SetPubTitle : public IEditMacroFunction
{
public:
    CMacroFunction_SetPubTitle(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    ~CMacroFunction_SetPubTitle() {}

    virtual void TheFunction();
    static CTempString GetFuncName();

protected:
    virtual bool x_ValidArguments() const;
    void x_SetTitleInPubdesc(objects::CPubdesc& pubdesc, const string& value);
    void x_SetTitleInSubmitBlock(objects::CSubmit_block& submit_block, const string& value);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_SetPubAffil : public IEditMacroFunction
{
public:
    CMacroFunction_SetPubAffil(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    ~CMacroFunction_SetPubAffil() {}

    virtual void TheFunction();
    static CTempString GetFuncName();

protected:
    virtual bool x_ValidArguments() const;
    void x_SetAffilField(objects::CAuth_list& auth_list, const string& field, const string& value, objects::edit::EExistingText existing_text);
    void x_RemoveAffilField(objects::CAuth_list& auth_list, const string& field);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_SetPubAuthor : public IEditMacroFunction
{
public:
    CMacroFunction_SetPubAuthor(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
    void x_SetAuthorInPubdesc(objects::CPubdesc& pubdesc, const string& field, const CMQueryNodeValue& value, objects::edit::EExistingText existing_text);
    void x_SetNewAuthor(objects::CAuth_list& auth_list, const string& field, const CMQueryNodeValue& value);
};

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_SetPubAuthorMI);

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_AddPubAuthor : public IEditMacroFunction
{
public:
    CMacroFunction_AddPubAuthor(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual void TheFunction();
    static CTempString GetFuncName();

protected:
    virtual bool x_ValidArguments() const;
    void x_AddAuthorInPubdesc(objects::CPubdesc& pubdesc, objects::edit::EExistingText existing_text);
    void x_AddAuthor(objects::CAuth_list& auth_list, objects::edit::EExistingText existing_text);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_AddAuthorList : public IEditMacroFunction
{
public:
    CMacroFunction_AddAuthorList(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual void TheFunction();
    static CTempString GetFuncName();

protected:
    virtual bool x_ValidArguments() const;
    void x_AddAuthorListInPubdesc(objects::CPubdesc& pubdesc);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_SetPubJournal : public IEditMacroFunction
{
public:
    CMacroFunction_SetPubJournal(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    ~CMacroFunction_SetPubJournal() {}

    virtual void TheFunction();
    static CTempString GetFuncName();

protected:
    virtual bool x_ValidArguments() const;
    void x_SetJournalInPubdesc(objects::CPubdesc& pubdesc, const string& value, objects::edit::EExistingText existing_text);
    void x_RemoveJournalInPubdesc(objects::CPubdesc& pubdesc);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_SetPubVolIssuePages : public IEditMacroFunction
{
public:
    CMacroFunction_SetPubVolIssuePages(EScopeEnum func_scope, const string& field)
        : IEditMacroFunction(func_scope), m_Field(field) {}

    ~CMacroFunction_SetPubVolIssuePages() {}

    virtual void TheFunction();
    static const char* sm_FuncVolume;
    static const char* sm_FuncIssue;
    static const char* sm_FuncPages;

protected:
    virtual bool x_ValidArguments() const;
    void x_SetVolIssuePagesInPubdesc(objects::CPubdesc& pubdesc, const string& value, objects::edit::EExistingText existing_text);
    void x_RemoveVolIssuePagesInPubdesc(objects::CPubdesc& pubdesc);
    string m_Field;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_SetPubDate : public IEditMacroFunction
{
public:
    CMacroFunction_SetPubDate(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    ~CMacroFunction_SetPubDate() {}

    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
    void x_AddDate(CMQueryNodeValue::TObs& objs);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_SetPubDateField : public IEditMacroFunction
{
public:
    CMacroFunction_SetPubDateField(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    ~CMacroFunction_SetPubDateField() {}

    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
    void x_AddDateField(CMQueryNodeValue::TObs& objs);
    void x_RemoveDateField(CMQueryNodeValue::TObs& objs);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_SetPubStatus : public IEditMacroFunction
{
public:
    CMacroFunction_SetPubStatus(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    ~CMacroFunction_SetPubStatus() {}

    virtual void TheFunction();
    static CTempString GetFuncName();
    static bool s_SetStatus(objects::CPubdesc& pubdesc, const string& value);
    static bool s_SetStatus(objects::CImprint& imp, const string& value);
protected:
    virtual bool x_ValidArguments() const;
};

// DEPRECATED
class CMacroFunction_SetPubField : public IEditMacroFunction
{
public:
    virtual ~CMacroFunction_SetPubField() {}

    virtual void TheFunction();
protected:
    enum EPubFieldType {
        ePubField_Affil,
        ePubField_Date
    };

    CMacroFunction_SetPubField(EScopeEnum func_scope, EPubFieldType pub_field)
        : IEditMacroFunction(func_scope), m_PubField(pub_field) {}

    virtual bool x_ValidArguments() const;
    void x_SetPubFieldInPub(CObjectInfo& pub_oi);
    void x_SetObjects_Date(objects::CPub& pub, CObjectInfo& pub_var);
    void x_SetObjects_Affil(objects::CPub& pub, CObjectInfo& pub_var);
    EPubFieldType m_PubField;
};

// DEPRECATED
class CMacroFunction_SetPubAffil_Depr : public CMacroFunction_SetPubField
{
public:
    CMacroFunction_SetPubAffil_Depr(EScopeEnum func_scope)
        : CMacroFunction_SetPubField(func_scope, CMacroFunction_SetPubField::ePubField_Affil) {}
    ~CMacroFunction_SetPubAffil_Depr() {}

    static const char* sm_FunctionName;
};

// DEPRECATED
class CMacroFunction_SetPubDate_Depr : public CMacroFunction_SetPubField
{
public:
    CMacroFunction_SetPubDate_Depr(EScopeEnum func_scope)
        : CMacroFunction_SetPubField(func_scope, CMacroFunction_SetPubField::ePubField_Date) {}
    ~CMacroFunction_SetPubDate_Depr() {}

    static const char* sm_FunctionName;
};


END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_FN_PUBFIELDS__HPP
