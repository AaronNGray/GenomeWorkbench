/*  $Id: molinfoedit_util.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
#ifndef _MOLINFOEDIT_UTIL_H_
#define _MOLINFOEDIT_UTIL_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>

#include <objects/seqtable/Seq_table.hpp>
#include <objects/seqtable/SeqTable_column.hpp>
#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <objects/seqtable/SeqTable_column_info.hpp>
#include <objects/seq/MolInfo.hpp>


#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/grid_widget/grid_widget.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/objutils/table_data.hpp>
#include <gui/widgets/edit/molinfo_fieldtype.hpp>


#include <gui/objutils/cmd_composite.hpp>

BEGIN_NCBI_SCOPE


CRef<objects::CSeq_table> BuildMolInfoValuesTableFromSeqEntry (objects::CSeq_entry_Handle seh, CSourceRequirements::EWizardType wizard_type);
CRef<CCmdComposite> ApplyMolInfoValuesTableToSeqEntry (CRef<objects::CSeq_table>values_table, objects::CSeq_entry_Handle seh, bool add_confirmed = true);

CRef<objects::CSeq_table> BuildCommentDescriptorValuesTableFromSeqEntry (objects::CSeq_entry_Handle seh, const string& label);
CRef<CCmdComposite> ApplyCommentValuesTableToSeqEntry (CRef<objects::CSeq_table>values_table, objects::CSeq_entry_Handle seh, const string& label);

CRef<objects::CSeq_table> BuildDBLinkValuesTableFromSeqEntry (objects::CSeq_entry_Handle seh);
CRef<CCmdComposite> ApplyDBLinkValuesTableToSeqEntry (CRef<objects::CSeq_table>values_table, objects::CSeq_entry_Handle seh);

vector<string> GetMoleculeTypeOptions (CSourceRequirements::EWizardType wizard_type);
vector<string> GetSrcGenomeOptions (CSourceRequirements::EWizardType wizard_type, CSourceRequirements::EWizardSrcType src_type);
CRef<objects::CSeq_table> BuildChimeraValuesTableFromSeqEntry (objects::CSeq_entry_Handle seh, const string& label, CSourceRequirements::EWizardSrcType src_type);
CRef<CCmdComposite> ApplyChimeraValuesTableToSeqEntry (CRef<objects::CSeq_table>values_table, objects::CSeq_entry_Handle seh, const string& label);

class CMolInfoField : public CTextDescriptorField
{
public:
    CMolInfoField(CMolInfoFieldType::EMolInfoFieldType field_type) : m_FieldType(field_type)
    {
        m_ConstraintFieldType = CMolInfoFieldType::e_Unknown;
              m_StringConstraint = NULL; 
              m_Subtype = CSeqdesc::e_Molinfo; };

    virtual vector<CConstRef<CObject> > GetObjects(CBioseq_Handle bsh);
    virtual vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(CBioseq_Handle bsh);


    virtual string GetVal(const CObject& object);
    virtual vector<string> GetVals(const CObject& object);
    virtual bool IsEmpty(const CObject& object) const;
    virtual void ClearVal(CObject& object);
    virtual bool SetVal(CObject& object, const string& val, objects::edit::EExistingText existing_text);
    virtual string IsValid(const string& val) { return ""; };
    virtual vector<string> IsValid(const vector<string>& values) {vector<string> rval; return rval; };
    virtual void SetConstraint(const string& field, CConstRef<objects::edit::CStringConstraint> string_constraint);
    virtual bool AllowMultipleValues() { return false; } ;
    //TODO: remove these three functions
    static vector<string> GetFieldNames();
    static string GetFieldName(CMolInfoFieldType::EMolInfoFieldType field_type);
    static CMolInfoFieldType::EMolInfoFieldType GetFieldType(const string& field_name);

    static vector<string> GetChoicesForField(CMolInfoFieldType::EMolInfoFieldType field_type, bool& allow_other);
    static string GetBiomolLabel(CMolInfo::TBiomol biomol);
    static CMolInfo::TBiomol GetBiomolFromLabel(const string& val);
    static string GetTechLabel(CMolInfo::TTech tech);
    static CMolInfo::TTech GetTechFromLabel(const string& val);
    static string GetCompletenessLabel(CMolInfo::TCompleteness tech);
    static CMolInfo::TCompleteness GetCompletenessFromLabel(const string& val);
    static string GetMolLabel(CSeq_inst::TMol val);
    static CSeq_inst::TMol GetMolFromLabel(const string& val);
    static string GetTopologyLabel(CSeq_inst::TTopology tech);
    static CSeq_inst::TTopology GetTopologyFromLabel(const string& val);
    static string GetStrandLabel(CSeq_inst::TStrand val);
    static CSeq_inst::TStrand GetStrandFromLabel(const string& val);

protected:
    CMolInfoFieldType::EMolInfoFieldType m_FieldType;
    CMolInfoFieldType::EMolInfoFieldType m_ConstraintFieldType;
    CRef<objects::edit::CStringConstraint> m_StringConstraint;

};

END_NCBI_SCOPE

#endif
    // _MOLINFOEDIT_UTIL_H_
