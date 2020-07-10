/*  $Id: pub_field.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
#ifndef _PUB_FIELD_H_
#define _PUB_FIELD_H_

#include <corelib/ncbistd.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/biblio/Imprint.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/seq/Pubdesc.hpp>

#include <objmgr/scope.hpp>

#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/pub_fieldtype.hpp>

#include <gui/packages/pkg_sequence_edit/field_handler.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CConstRef<CAuth_list> GetAuthList(const CPubdesc& pubdesc);
CConstRef<CAuth_list> GetAuthList(CConstRef<CObject> sel_pub);
bool SetAuthList (CPubdesc& pdesc, const CAuth_list& auth_list);
bool SetAuthorNames (CPubdesc& pdesc, const CAuth_list& auth_list);

string GetTitleName(const CTitle& title);
string GetPubTitle(CConstRef<CObject> sel_pub);
void SetTitleName(CTitle& title, string val);

class CPubField : public ncbi::CFieldHandler
{
public:

    CPubField(CPubFieldType::EPubFieldType field_type) : m_FieldType(field_type)
    {
        m_ConstraintFieldType = CPubFieldType::ePubFieldType_Unknown;
              m_StringConstraint = NULL; };

    virtual vector<CConstRef<CObject> > GetObjects(CBioseq_Handle bsh);
    virtual vector<CConstRef<CObject> > GetObjects(CSeq_entry_Handle seh, const string& constraint_field, CRef<edit::CStringConstraint> string_constraint);
    virtual vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(CBioseq_Handle bsh);
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const CObject& object, CRef<CScope> scope);
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const objects::edit::CApplyObject& object);

    virtual bool IsEmpty(const CObject& object) const;

    virtual string GetVal(const CObject& object);
    string GetVal(const CPubdesc& pdesc);
    string GetVal(const CPub& pub);
    string GetVal(const CCit_sub& sub);
    virtual vector<string> GetVals(const CObject& object);
    vector<string> GetVals( const CPubdesc& pdesc);
    vector<string> GetVals( const CPub& pub);
    vector<string> GetVals( const CCit_sub& sub);
    vector<string> GetVals( const CSeqdesc & in_out_desc) ;

    virtual void ClearVal(CObject& object);
    void ClearVal(CSeqdesc & in_out_desc);
    void ClearVal( CPubdesc& pdesc);
    void ClearVal( CCit_sub& sub);

    virtual CSeqFeatData::ESubtype GetFeatureSubtype() { return CSeqFeatData::eSubtype_pub; };
    virtual CSeqdesc::E_Choice GetDescriptorSubtype() { return CSeqdesc::e_Pub; };
    virtual void SetConstraint(const string& field_name, CConstRef<edit::CStringConstraint> string_constraint);
    virtual bool AllowMultipleValues();
    virtual bool SetVal(CObject& object, const string & newValue, objects::edit::EExistingText existing_text);
    void SetVal(CSeqdesc & desc, const string & newValue, objects::edit::EExistingText existing_text);
    void SetVal(CPubdesc& pdesc, const string & newValue, objects::edit::EExistingText existing_text);
    void SetVal(CCit_sub& sub, const string & newValue, objects::edit::EExistingText existing_text);
    virtual string GetLabel() const;
    static CPubFieldType::EPubFieldType GetTypeForLabel(string label);
    static string GetLabelForType(CPubFieldType::EPubFieldType field_type);
    static bool IsAffilField(CPubFieldType::EPubFieldType field_type);
    static bool IsAuthorField(CPubFieldType::EPubFieldType field_type);
    static bool IsImprintField(CPubFieldType::EPubFieldType field_type);
    static string GetAuthorField(const CAuthor& auth, CPubFieldType::EPubFieldType field_type);
    static bool SetAuthorField(CAuthor& auth, CPubFieldType::EPubFieldType field_type, string val, objects::edit::EExistingText existing_text);
    static bool DoesAuthorMatchConstraint(const CAuthor& auth, CPubFieldType::EPubFieldType constraint_field, CRef<edit::CStringConstraint> string_constraint);
    static string NormalizePubFieldName(string orig_label);
    static vector<string> GetChoicesForField(CPubFieldType::EPubFieldType field_type, bool& allow_other);
    static string GetPubTitle (const CPubdesc& pdesc);
    static string GetPubTitle (const CPub& pub);
    static string GetPubTitle (const CCit_sub& sub);
    static bool SetPubTitle (CPubdesc& pdesc, const string& title, objects::edit::EExistingText existing_text, bool skip_sub = false);

    static string GetJournal (const CPubdesc& pdesc);
    static string GetJournal (const CPub& pub);
    static bool SetJournal (CPubdesc& desc, const string& journal, objects::edit::EExistingText existing_text);
    static bool SetJournal (CPub& pub, const string& journal, objects::edit::EExistingText existing_text);


    static CConstRef<CImprint> GetImprint (const CPub& pub);
    static bool SetImprint(CPub& pub, const CImprint& imp);
    static vector<string> GetFieldNames();

    static string GetLabelForStatus(CPubFieldType::EPubFieldStatus status);
    static CPubFieldType::EPubFieldStatus GetStatusFromString(const string& str);
    static string GetStatus(const CObject& obj);
    static string GetStatus(const CPubdesc& pdesc);
    static string GetStatus (const CPub& pub);
    static CPubFieldType::EPubFieldStatus GetStatusEnum(const CPub& pub);
    static CRef<CDate> GuessDate(const string &val);
protected:
    CPubFieldType::EPubFieldType m_FieldType;
    CPubFieldType::EPubFieldType m_ConstraintFieldType;
    CRef<objects::edit::CStringConstraint> m_StringConstraint;

    bool x_SetAffilField(CPubdesc &pdesc, CPubFieldType::EPubFieldType field_type, string value, objects::edit::EExistingText existing_text);
    bool x_SetAffilField(CAuth_list &auth_list, CPubFieldType::EPubFieldType field_type, string value, objects::edit::EExistingText existing_text);
    string x_GetAffilField(const CPubdesc &pdesc, CPubFieldType::EPubFieldType field_type);
    string x_GetAffilField(const CPub &pub, CPubFieldType::EPubFieldType field_type);
    string x_GetAffilField(const CCit_sub &sub, CPubFieldType::EPubFieldType field_type);
    string x_GetAffilField(const CAuth_list& auth_list, CPubFieldType::EPubFieldType field_type);
    bool x_SetAuthorField(CPubdesc &pdesc, CPubFieldType::EPubFieldType field_type, string value, CPubFieldType::EPubFieldType constraint_field, CRef<edit::CStringConstraint> string_constraint, objects::edit::EExistingText existing_text);
    bool x_SetAuthorField(CAuth_list &auth_list, CPubFieldType::EPubFieldType field_type, string value, CPubFieldType::EPubFieldType constraint_field, CRef<edit::CStringConstraint> string_constraint, objects::edit::EExistingText existing_text);
    string x_GetAuthorField(const CPubdesc &pdesc, CPubFieldType::EPubFieldType field_type, CPubFieldType::EPubFieldType constraint_field, CRef<edit::CStringConstraint> string_constraint);
    string x_GetAuthorField(const CAuth_list &auth_list, CPubFieldType::EPubFieldType field_type, CPubFieldType::EPubFieldType constraint_field, CRef<edit::CStringConstraint> string_constraint);

    string x_GetImprintField (const CPubdesc& pdesc, CPubFieldType::EPubFieldType field_type);
    string x_GetImprintField (const CPub& pub, CPubFieldType::EPubFieldType field_type);
    string x_GetImprintField (const CCit_gen& gen, CPubFieldType::EPubFieldType field_type);
    string x_GetImprintField (const CCit_sub& sub, CPubFieldType::EPubFieldType field_type);
    string x_GetImprintField (const CImprint& imp, CPubFieldType::EPubFieldType field_type);
    bool x_SetImprintField (CPubdesc& desc, CPubFieldType::EPubFieldType field_type, const string& val, objects::edit::EExistingText existing_text);
    bool x_SetImprintField (CPub& pub, CPubFieldType::EPubFieldType field_type, const string& val, objects::edit::EExistingText existing_text);
    bool x_SetImprintField (CCit_gen& gen, CPubFieldType::EPubFieldType field_type, const string& val);
    bool x_SetImprintField (CImprint& imp, CPubFieldType::EPubFieldType field_type, const string& val);

    bool x_SetStatus (CPubdesc& pdesc, const string& val);
    bool x_SetStatus(CPubdesc& pdesc, CPubFieldType::EPubFieldStatus status);
    bool x_SetStatus(CPub& pub, CPubFieldType::EPubFieldStatus status);

};





END_NCBI_SCOPE

#endif
    // _PUB_FIELD_H_
