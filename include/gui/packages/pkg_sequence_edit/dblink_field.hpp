/*  $Id: dblink_field.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
#ifndef _DBLINK_FIELD_H_
#define _DBLINK_FIELD_H_

#include <corelib/ncbistd.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>

#include <objmgr/scope.hpp>

#include <objtools/edit/seqid_guesser.hpp>

#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/dblink_fieldtype.hpp>
#include <gui/packages/pkg_sequence_edit/field_handler.hpp>

BEGIN_NCBI_SCOPE

class CDBLinkField : public CFieldHandler
{
public:

    CDBLinkField(CDBLinkFieldType::EDBLinkFieldType field_type) : m_FieldType(field_type)
    {
        m_ConstraintFieldType = CDBLinkFieldType::eDBLinkFieldType_Unknown;
              m_StringConstraint = NULL; };

    virtual vector<CConstRef<CObject> > GetObjects(CBioseq_Handle bsh);
    virtual vector<CConstRef<CObject> > GetObjects(CSeq_entry_Handle seh, const string& constraint_field, CRef<edit::CStringConstraint> string_constraint);
    virtual vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(CBioseq_Handle bsh);
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const CObject& object, CRef<CScope> scope);
    virtual vector<CConstRef<CObject> > GetRelatedObjects(const objects::edit::CApplyObject& object);

    virtual bool IsEmpty(const CObject& object) const;

    virtual string GetVal(const CObject& object);
    virtual vector<string> GetVals(const CObject& object);

    virtual void ClearVal(CObject& object);

    virtual CSeqFeatData::ESubtype GetFeatureSubtype() { return CSeqFeatData::eSubtype_bad; };
    virtual CSeqdesc::E_Choice GetDescriptorSubtype() { return CSeqdesc::e_User; };
    virtual void SetConstraint(const string& field_name, CConstRef<edit::CStringConstraint> string_constraint);
    virtual bool AllowMultipleValues() { return true; }
    virtual bool SetVal(CObject& object, const string & newValue, objects::edit::EExistingText existing_text);
    bool SetVal(CUser_field& field, const string & newValue, objects::edit::EExistingText existing_text);
    virtual string GetLabel() const;
    static CDBLinkFieldType::EDBLinkFieldType GetTypeForLabel(string label);
    static string GetLabelForType(CDBLinkFieldType::EDBLinkFieldType field_type);
    static bool IsDBLink (const CUser_object& user);
    static void NormalizeDBLinkFieldName(string& orig_label);
    static vector<string> GetFieldNames();

protected:
    CDBLinkFieldType::EDBLinkFieldType m_FieldType;
    CDBLinkFieldType::EDBLinkFieldType m_ConstraintFieldType;
    CRef<objects::edit::CStringConstraint> m_StringConstraint;
};



END_NCBI_SCOPE

#endif
    // _DBLINK_FIELD_H_
