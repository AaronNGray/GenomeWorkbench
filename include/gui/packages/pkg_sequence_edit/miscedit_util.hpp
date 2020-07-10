/*  $Id: miscedit_util.hpp 41872 2018-10-31 15:16:50Z asztalos $
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
#ifndef _MISCEDIT_UTIL_H_
#define _MISCEDIT_UTIL_H_

#include <corelib/ncbistd.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/biblio/Imprint.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objmgr/scope.hpp>

#include <objtools/edit/loc_edit.hpp>
#include <objtools/edit/parse_text_options.hpp>
#include <objtools/cleanup/capitalization_string.hpp>

#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/packages/pkg_sequence_edit/featedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <gui/packages/pkg_sequence_edit/pub_field.hpp>


BEGIN_NCBI_SCOPE

class CFieldHandlerFactory
{
public:
    static CRef<ncbi::CFieldHandler> Create(const string &field_name);
};


class CTextDescriptorField : public ncbi::CFieldHandler
{
public:
    virtual vector<CConstRef<CObject> > GetObjects(objects::CBioseq_Handle bsh);
    virtual vector<CConstRef<CObject> > GetObjects(objects::CSeq_entry_Handle seh, const string& constraint_field, CRef<objects::edit::CStringConstraint> string_constraint);
    virtual vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(objects::CBioseq_Handle bsh);
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const CObject& object, CRef<objects::CScope> scope);
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const objects::edit::CApplyObject& object);

    virtual objects::CSeqFeatData::ESubtype GetFeatureSubtype() { return objects::CSeqFeatData::eSubtype_bad; } ;
    virtual objects::CSeqdesc::E_Choice GetDescriptorSubtype() { return m_Subtype; } ;


    virtual void SetConstraint(const string& field_name, CConstRef<objects::edit::CStringConstraint> string_constraint) {};
    objects::CSeqdesc::E_Choice GetSubtype() { return m_Subtype; };
    void SetSubtype(objects::CSeqdesc::E_Choice subtype) { m_Subtype = subtype; };
    virtual bool AllowMultipleValues() { return false; };
protected:
    objects::CSeqdesc::E_Choice m_Subtype;
};


class CCommentDescField : public CTextDescriptorField
{
public:
    CCommentDescField () { m_Subtype = objects::CSeqdesc::e_Comment; };

    virtual string GetVal(const CObject& object);
    virtual vector<string> GetVals(const CObject& object);
    virtual bool IsEmpty(const CObject& object) const;
    virtual void ClearVal(CObject& object);
    virtual bool SetVal(CObject& object, const string& val, objects::edit::EExistingText existing_text);
};


class CDefinitionLineField : public CTextDescriptorField
{
public:
    CDefinitionLineField () { m_Subtype = objects::CSeqdesc::e_Title; };
    virtual string GetVal(const CObject& object);
    virtual vector<string> GetVals(const CObject& object);
    virtual bool IsEmpty(const CObject& object) const;
    virtual void ClearVal(CObject& object);
    virtual bool SetVal(CObject& object, const string& val, objects::edit::EExistingText existing_text);
};


class CGenbankKeywordField : public CTextDescriptorField
{
public:
    CGenbankKeywordField() : m_StringConstraint(NULL) { m_Subtype = objects::CSeqdesc::e_Genbank; };
    virtual string GetVal(const CObject& object);
    virtual vector<string> GetVals(const CObject& object);
    virtual bool IsEmpty(const CObject& object) const;
    virtual void ClearVal(CObject& object);
    virtual bool SetVal(CObject& object, const string& val, objects::edit::EExistingText existing_text);
    virtual string IsValid(const string& val) { return kEmptyStr; };
    virtual vector<string> IsValid(const vector<string>& values) { vector<string> rval; return rval; };
    virtual void SetConstraint(const string& field, CConstRef<objects::edit::CStringConstraint> string_constraint);
    virtual bool AllowMultipleValues() { return true; };
protected:
    CRef<objects::edit::CStringConstraint> m_StringConstraint;
};


class CGenomeProjectField : public ncbi::CFieldHandler
{
public:
    CGenomeProjectField () {};
    virtual vector<CConstRef<CObject> > GetObjects(objects::CBioseq_Handle bsh);
    virtual vector<CConstRef<CObject> > GetObjects(objects::CSeq_entry_Handle seh, const string& constraint_field, CRef<objects::edit::CStringConstraint> string_constraint);
    virtual vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(objects::CBioseq_Handle bsh);
    virtual string GetVal(const CObject& object);
    virtual vector<string> GetVals(const CObject& object);
    virtual bool IsEmpty(const CObject& object) const;
    virtual void ClearVal(CObject& object);
    virtual bool SetVal(CObject& object, const string& val, objects::edit::EExistingText existing_text);
    virtual string IsValid(const string& val);
    virtual vector<string> IsValid(const vector<string>& values);
    virtual objects::CSeqFeatData::ESubtype GetFeatureSubtype() { return objects::CSeqFeatData::eSubtype_bad; };
    virtual objects::CSeqdesc::E_Choice GetDescriptorSubtype() { return objects::CSeqdesc::e_User; };
    virtual void SetConstraint(const string& field, CConstRef<objects::edit::CStringConstraint> string_constraint) {};
    virtual bool AllowMultipleValues() { return false; } ;
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const CObject& object, CRef<objects::CScope> scope);
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const objects::edit::CApplyObject& object);
    static bool IsGenomeProject(const objects::CUser_object& user);
    static bool IsGenomeProjectID(const objects::CUser_field& field);
};

class CBankITCommentField : public ncbi::CFieldHandler
{
public:
    CBankITCommentField() {}
    virtual ~CBankITCommentField() {}

    virtual vector<CConstRef<CObject> > GetObjects(objects::CBioseq_Handle bsh);
    virtual vector<CConstRef<CObject> > GetObjects(objects::CSeq_entry_Handle seh, const string& constraint_field, CRef<objects::edit::CStringConstraint> string_constraint);
    virtual vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(objects::CBioseq_Handle bsh);
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const CObject& object, CRef<objects::CScope> scope);
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const objects::edit::CApplyObject& object);

    virtual void SetConstraint(const string& field, CConstRef<objects::edit::CStringConstraint> string_constraint) {}
    virtual bool AllowMultipleValues() { return true; }

    virtual string GetVal(const CObject& object);
    virtual vector<string> GetVals(const CObject& object);
    virtual bool IsEmpty(const CObject& object) const { return false; }
    virtual void ClearVal(CObject& object) {}
    virtual bool SetVal(CObject& object, const string& val, objects::edit::EExistingText existing_text) { return false; }
    virtual objects::CSeqFeatData::ESubtype GetFeatureSubtype() { return objects::CSeqFeatData::eSubtype_bad; }
    virtual objects::CSeqdesc::E_Choice GetDescriptorSubtype() { return objects::CSeqdesc::e_User; }
};


class CGeneralCommentField : public ncbi::CBankITCommentField
{
public:
    CGeneralCommentField() {}
    virtual ~CGeneralCommentField() {}

    virtual vector<string> GetVals(const CObject& object);
};

class CSeqIdField : public ncbi::CFieldHandler
{
public:
    CSeqIdField () {} 
    virtual ~CSeqIdField() {}
    virtual vector<CConstRef<CObject> > GetObjects(objects::CBioseq_Handle bsh);
    virtual vector<CConstRef<CObject> > GetObjects(objects::CSeq_entry_Handle seh, const string& constraint_field, CRef<objects::edit::CStringConstraint> string_constraint);
    virtual vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(objects::CBioseq_Handle bsh);
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const CObject& object, CRef<objects::CScope> scope);
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const objects::edit::CApplyObject& object);

    virtual void SetConstraint(const string& field_name, CConstRef<objects::edit::CStringConstraint> string_constraint) {};
    virtual bool AllowMultipleValues() { return false; };

    virtual string GetVal(const CObject& object);
    virtual vector<string> GetVals(const CObject& object);
    virtual bool IsEmpty(const CObject& object) const { return false; }
    virtual void ClearVal(CObject& object) {};
    virtual bool SetVal(CObject& object, const string& val, objects::edit::EExistingText existing_text) { return false;}
    virtual objects::CSeqFeatData::ESubtype GetFeatureSubtype() { return objects::CSeqFeatData::eSubtype_bad; }
    virtual objects::CSeqdesc::E_Choice GetDescriptorSubtype() { return objects::CSeqdesc::e_not_set; }

};

// NCBIFILE File ID field
class CFileIDField : public CSeqIdField  
{
public:
    CFileIDField() {}
    virtual string GetVal(const CObject& object);
    virtual bool IsEmpty(const CObject& object) const;
private:
    bool x_IsNCBIFILEId(const objects::CSeq_id& seq_id);
};


class CGeneralIDField : public CSeqIdField
{
public:
    enum EGeneralIDPart {
        eUnknown ,
        eGenId ,
        eGenIdDb ,
        eGenIdTag
    };
    CGeneralIDField( EGeneralIDPart part, const string& db = kEmptyStr) 
        : m_GeneralIDPart(part), m_PreferredDB (db) {}
    virtual string GetVal(const CObject& object);

    static const string& GetName_GeneralIDPart( EGeneralIDPart stype_part );
    static EGeneralIDPart GetGeneralIDPart_FromName ( const string& name );
private:
    EGeneralIDPart m_GeneralIDPart;
    // used when only a specific DB tag is needed
    string m_PreferredDB; 
};


class CLocalIDField : public ncbi::CFieldHandler
{
public:
    CLocalIDField() {}
    virtual ~CLocalIDField() {}

    virtual vector<CConstRef<CObject> > GetObjects(objects::CBioseq_Handle bsh);
    virtual vector<CConstRef<CObject> > GetObjects(objects::CSeq_entry_Handle seh, const string& constraint_field, CRef<objects::edit::CStringConstraint> string_constraint);
    virtual vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(objects::CBioseq_Handle bsh);
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const CObject& object, CRef<objects::CScope> scope);
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const objects::edit::CApplyObject& object);
    
    virtual void SetConstraint(const string& field, CConstRef<objects::edit::CStringConstraint> string_constraint) {}
    virtual bool AllowMultipleValues() { return false; }

    virtual string GetVal(const CObject& object);
    virtual vector<string> GetVals(const CObject& object);
    virtual bool IsEmpty(const CObject& object) const;
    virtual void ClearVal(CObject& object) {}
    virtual bool SetVal(CObject& object, const string& val, objects::edit::EExistingText existing_text) { return false; }
    virtual objects::CSeqFeatData::ESubtype GetFeatureSubtype() { return objects::CSeqFeatData::eSubtype_bad; }
    virtual objects::CSeqdesc::E_Choice GetDescriptorSubtype() { return objects::CSeqdesc::e_not_set; }
};

// (label, data) members in User_field within a Structured Comment
class CStructCommFieldValuePair : public ncbi::CFieldHandler
{
public:
    CStructCommFieldValuePair(const string& fieldname)
        : m_FieldName(fieldname) {}

    virtual vector<CConstRef<CObject> > GetObjects(objects::CBioseq_Handle bsh);
    virtual vector<CConstRef<CObject> > GetObjects(objects::CSeq_entry_Handle seh, const string& constraint_field, CRef<objects::edit::CStringConstraint> string_constraint);
    virtual vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(objects::CBioseq_Handle bsh);
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const CObject& object, CRef<objects::CScope> scope);
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const objects::edit::CApplyObject& object);
    virtual void SetConstraint(const string& field_name, CConstRef<objects::edit::CStringConstraint> string_constraint) {}

    virtual string GetVal(const CObject& object);
    virtual vector<string> GetVals(const CObject& object);
    virtual void ClearVal(CObject& object);
    virtual bool SetVal(CObject& object, const string& val, objects::edit::EExistingText existing_text);
    virtual bool SetVal(objects::CUser_field& field, const string & newValue, objects::edit::EExistingText existing_text);
    virtual bool IsEmpty(const CObject& object) const;
    virtual bool AllowMultipleValues() { return false; }
    virtual objects::CSeqFeatData::ESubtype GetFeatureSubtype() { return objects::CSeqFeatData::eSubtype_bad; }
    virtual objects::CSeqdesc::E_Choice GetDescriptorSubtype() { return objects::CSeqdesc::e_User; }
protected:
    string m_FieldName;
    void x_InsertFieldAtCorrectPosition(objects::CUser_object& user, CRef<objects::CUser_field> field);
    const objects::CUser_object* x_GetUser_Object(const CObject& object) const;
    objects::CUser_object* x_GetUser_Object(CObject& object) const;
};

// data member of User_field corresponding to the StructuredCommentPrefix(Suffix) label
class CStructCommDBName : public CStructCommFieldValuePair
{
public:
    CStructCommDBName() : CStructCommFieldValuePair("StructuredComment") {}

    virtual vector<string> GetVals(const CObject& object);
    virtual void ClearVal(CObject& object);
    virtual bool SetVal(CObject& object, const string& val, objects::edit::EExistingText existing_text);
    virtual bool SetVal(objects::CUser_field& field, const string & newValue, objects::edit::EExistingText existing_text);
};

class CStructCommFieldName : public CStructCommFieldValuePair
{
public:
    CStructCommFieldName() : CStructCommFieldValuePair(kEmptyStr), m_Cleared(false), m_NrOfSetting(0) {}
    
    virtual vector<string> GetVals(const CObject& object);
    virtual void ClearVal(CObject& object);
    virtual bool SetVal(CObject& object, const string& val, objects::edit::EExistingText existing_text);
    virtual bool SetVal(objects::CUser_field& field, const string & newValue, objects::edit::EExistingText existing_text);
private:
    void x_Init(void);
    // for editing from the AECR dlg, we need to store the (label, data) pairs, and later restore them
    objects::CUser_object::TData m_FieldValuePairs;
    bool m_Cleared;
    unsigned int m_NrOfSetting;
};

class CFlatFileField : public ncbi::CFieldHandler
{
public:
    CFlatFileField() {}
    virtual ~CFlatFileField() {}

    virtual vector<CConstRef<CObject> > GetObjects(objects::CBioseq_Handle bsh);
    virtual vector<CConstRef<CObject> > GetObjects(objects::CSeq_entry_Handle seh, const string& constraint_field, CRef<objects::edit::CStringConstraint> string_constraint);
    virtual vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(objects::CBioseq_Handle bsh);
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const CObject& object, CRef<objects::CScope> scope);
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const objects::edit::CApplyObject& object);

    virtual void SetConstraint(const string& field, CConstRef<objects::edit::CStringConstraint> string_constraint) {}
    virtual bool AllowMultipleValues() { return false; }

    virtual string GetVal(const CObject& object);
    virtual vector<string> GetVals(const CObject& object) { vector<string> vals; return vals; }
    virtual bool IsEmpty(const CObject& object) const { return false; }
    virtual void ClearVal(CObject& object) {}
    virtual bool SetVal(CObject& object, const string& val, objects::edit::EExistingText existing_text) { return false; }
    virtual objects::CSeqFeatData::ESubtype GetFeatureSubtype() { return objects::CSeqFeatData::eSubtype_bad; }
    virtual objects::CSeqdesc::E_Choice GetDescriptorSubtype() { return objects::CSeqdesc::e_not_set; }
};

class CMiscSeqTableColumn : public CObject
{
public:
    CMiscSeqTableColumn(const string& name);
    ~CMiscSeqTableColumn() {}
    bool IsFeature() const { if (m_Feature) return true; else return false; }
    bool IsSource() const { if (m_Src) return true; else return false; }
    vector<CConstRef<CObject> > GetObjects(objects::CBioseq_Handle bsh);
    vector<CConstRef<CObject> > GetObjects(objects::CSeq_entry_Handle seh, const string& constraint_field, CRef<objects::edit::CStringConstraint> string_constraint);
    vector<CConstRef<CObject> > GetRelatedObjects(const CObject& object, CRef<objects::CScope> scope);
    vector<CConstRef<CObject> > GetRelatedObjects(const objects::edit::CApplyObject& object);
    vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(objects::CBioseq_Handle bsh);
    vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(objects::CSeq_entry_Handle seh, const string& constraint_field, CRef<objects::edit::CStringConstraint> string_constraint);
    void GetApplyObjectsValues(objects::CSeq_entry_Handle seh, const string& constraint_field, set<string> &values);
    void AddApplyObjectValue(const objects::edit::CApplyObject& object, const string& field, set<string> &values);
    vector<CRef<objects::edit::CApplyObject> > GetRelatedApplyObjects(objects::edit::CApplyObject& object);

    string GetVal(const CObject& object);
    vector<string> GetVals(const CObject& object);
    bool IsEmpty(const CObject& object) const;
    void ClearVal(CObject& object);
    CRef<CCmdComposite> ClearValCmd(CConstRef<CObject> object, bool update_mrna_product, objects::CScope& scope);
    bool SetVal(CObject& object, const string& val, objects::edit::EExistingText existing_text);
    CRef<CCmdComposite> SetValCmd(CConstRef<CObject> object, const string& val, objects::edit::EExistingText existing_text, objects::CScope& scope, bool update_mrna_product = false);
    CRef<CCmdComposite> ConvertValCmd(objects::edit::CApplyObject& object, CMiscSeqTableColumn& other, objects::edit::EExistingText existing_text, objects::ECapChange cap_change, const bool leave_original, const string& field_name, const bool update_mrna_product);
    CRef<CCmdComposite> ConvertValCmd(CRef<objects::edit::CApplyObject> src_object, CRef<objects::edit::CApplyObject> dst_object, CMiscSeqTableColumn& other, objects::edit::EExistingText existing_text, objects::ECapChange cap_change, const bool leave_original, const string& field_name, const bool update_mrna_product);
    CRef<CCmdComposite> CopyValCmd(objects::edit::CApplyObject& object, CMiscSeqTableColumn& other, objects::edit::EExistingText existing_text, bool update_mrna_product);
    CRef<CCmdComposite> CopyValCmd(CRef<objects::edit::CApplyObject> src_object, CRef<objects::edit::CApplyObject> dst_object, CMiscSeqTableColumn& other, objects::edit::EExistingText existing_text, bool update_mrna_product);
    CRef<CCmdComposite> SwapValCmd(objects::edit::CApplyObject& object,CMiscSeqTableColumn& other, objects::edit::EExistingText existing_text, bool update_mrna_product);
    CRef<CCmdComposite> SwapValCmd(CRef<objects::edit::CApplyObject> src_object, CRef<objects::edit::CApplyObject> dst_object, CMiscSeqTableColumn& other, objects::edit::EExistingText existing_text, bool update_mrna_product);
    CRef<CCmdComposite> ParseValCmd(objects::edit::CApplyObject& object, CMiscSeqTableColumn& other, const objects::edit::CParseTextOptions& parse_options, objects::edit::EExistingText existing_text, bool update_mrna_product);
    CRef<CCmdComposite> ParseValCmd(CRef<objects::edit::CApplyObject> src_object, CRef<objects::edit::CApplyObject> dst_object, CMiscSeqTableColumn& other, const objects::edit::CParseTextOptions& parse_options, objects::edit::EExistingText existing_text, bool update_mrna_product);
    string IsValid(const string& val);
    vector<string> IsValid(const vector<string>& values);
    int CountConflicts(set<CConstRef<CObject> > objs, const string& val);
    int CountConflicts(const CObject& obj, const string& val);
    CRef<CObject> GetNewObject(CConstRef<CObject> obj);
    CRef<CCmdComposite> GetReplacementCommand(CConstRef<CObject>, CRef<CObject> newobj, objects::CScope& scope, const string& cmd_name);
    objects::CSeqFeatData::ESubtype GetFeatureSubtype();
    objects::CSeqdesc::E_Choice GetDescriptorSubtype();
    void SetConstraint(string field, CRef<objects::edit::CStringConstraint> string_constraint);
    bool AllowMultipleValues();
    static bool s_IsSequenceIDField(const string& field);
    bool IsProductField( void );
    void SetScope(CRef<objects::CScope> scope);

private:
    CRef<CFeatureSeqTableColumnBase> m_Feature;
    CRef<CSrcTableColumnBase> m_Src;
    CRef<ncbi::CFieldHandler> m_FieldHandler;

    CRef<CCmdComposite> x_GetMrnaUpdate(objects::edit::CApplyObject& object, CMiscSeqTableColumn& other);
    CRef<CCmdComposite> x_GetMrnaUpdate(objects::edit::CApplyObject& src_object, objects::edit::CApplyObject& dst_object, CMiscSeqTableColumn& other, bool& changed_src);
};


typedef vector< CRef<CMiscSeqTableColumn> > TMiscSeqTableColumnList;

CRef<CCmdComposite> ConvertValCmd(CConstRef<CObject> object, 
                                  CRef<CMiscSeqTableColumn> col1, 
                                  CRef<CMiscSeqTableColumn> col2, 
                                  objects::edit::EExistingText existing_text, 
                                  objects::CScope& scope);


CRef<objects::CSeq_table> NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT GetMiscTableFromSeqEntry(const vector<string> &fields, objects::CSeq_entry_Handle entry);
void AddMiscColumnsToTable(CRef<objects::CSeq_table> table, const vector<string> &fields, objects::CSeq_entry_Handle entry);

string MakeTableMetaInfoString(objects::edit::EExistingText rule, bool erase_blanks);
CRef<objects::CUser_field> NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT MakeTableMetaInfoField(objects::edit::EExistingText rule, bool erase_blanks);

int CountColumnConflicts(CRef<CMiscSeqTableColumn> col, vector<CConstRef<CObject> > objs, const string& val);

bool DoesObjectMatchFieldConstraint (const CObject& object, const string& field_name, CRef<objects::edit::CStringConstraint> string_constraint, CRef<objects::CScope> scope);
bool DoesApplyObjectMatchFieldConstraint (const objects::edit::CApplyObject& object, const string& field_name, CRef<objects::edit::CStringConstraint> string_constraint);

CRef<CCmdComposite> GetPropagateDBLinkCmd(objects::CSeq_entry_Handle entry);

CRef<CCmdComposite> GetCommandFromApplyObject(objects::edit::CApplyObject& obj);

void GetViewObjects(IWorkbench* workbench, TConstScopedObjects &objects);

int GetNumberOfViewedBioseqs(TConstScopedObjects& objects);
bool FocusedOnNucleotides(TConstScopedObjects& objects, const objects::CSeq_entry_Handle& tse);
bool FocusedOnAll(TConstScopedObjects& objects, const objects::CSeq_entry_Handle& tse);

END_NCBI_SCOPE

#endif
    // _MISCEDIT_UTIL_H_
