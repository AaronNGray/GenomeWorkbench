#ifndef _GUI_PACKAGES_EDIT__TABLE_READER_HPP_
#define _GUI_PACKAGES_EDIT__TABLE_READER_HPP_

/*  $Id: table_reader.hpp 43907 2019-09-18 13:03:57Z filippov $
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
* Authors:  Colleen Bollin, Andrea Asztalos
*
* File Description:
*
*/

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objtools/edit/string_constraint.hpp>
#include <gui/objutils/macro_cmd_composite.hpp>
#include <gui/packages/pkg_sequence_edit/field_choice_panel.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CSeq_table;
    class CUser_object;
END_SCOPE(objects)

class IEditingAction;

class NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT CMiscSeqTable
{
public:
    CMiscSeqTable(CRef<objects::CSeq_table> table, 
		  objects::CSeq_entry_Handle entry, 
		  edit::CSeqIdGuesser  &id_guesser,
		  CRef<objects::CUser_object> column_properties,
		  const string& constraint_field = kEmptyStr,
		  CRef<objects::edit::CStringConstraint> string_constraint = CRef<edit::CStringConstraint>(NULL),
		  ICommandProccessor* cmd_processor = NULL);
    ~CMiscSeqTable() {}

    void ApplyTableToEntry();
    static void s_GetValueToSehMap(objects::CSeq_entry_Handle seh,  map<string, set<CSeq_entry_Handle> > &value_to_sehs, string id_field_name, CFieldNamePanel::EFieldType id_field_type, int id_subtype);
private:
    bool x_ApplyMiscSeqTableToSeqEntry(CRef<CMacroCmdComposite> apply_cmd);

    // returns the index of the column that is used for ID
    size_t x_FindIDColumn();

    // finds the m_IdFieldName string, and based on that, it finds its fieldtype and subtype
    // these values are used later on to find the relevant top seq-entry for each editing action
    void x_GetIdColumnProps(size_t id_col_num);

    // for each column, except the ID_column, create editing action
    // this function populates the m_ActionList vector
    void x_CreateEditingActions(size_t id_col_num);

    // for each column, except the ID_column, read the m_ColProperties user object
    // this function populates m_EraseIfBlank and m_ExistingText vectors
    void x_InterpretColumnProperties(size_t id_col_num);

    // using the Id column properties and its value, set the top seq-entry for each editing action
    void x_SetTopSeqEntryForActions(const string& id_value, set<CSeq_entry_Handle> &sehs, const map<string, set<CSeq_entry_Handle> > &value_to_sehs);



    bool x_SetConstraints(size_t id_col_num, CRef<edit::CStringConstraint> id_constraint);

    // perform actual change and store the commands 
    bool x_ApplyRowToEntry(int row, size_t id_col_num, CRef<CMacroCmdComposite> apply_cmd);

    void x_CheckDuplicateIds(CRef<CSeqTable_column> id_col);

    CRef<objects::CSeq_table> m_Table;
    objects::CSeq_entry_Handle m_Seh;
    edit::CSeqIdGuesser  &m_id_guesser;
    CRef<objects::CUser_object> m_ColProperties;
    string m_ConstrField;
    CRef<objects::edit::CStringConstraint> m_StrConstraint;
    ICommandProccessor* m_CmdProccessor;

    typedef vector<CIRef<IEditingAction>> TEditingActionVec;
    TEditingActionVec m_ActionList;

    vector<bool> m_EraseIfBlank;
    vector<objects::edit::EExistingText> m_ExistingText;

    // id column properties
    string m_IdFieldName;
    CFieldNamePanel::EFieldType m_IdFieldType;
    int m_IdSubtype;

private:
    CMiscSeqTable(const CMiscSeqTable&);
    CMiscSeqTable& operator=(const CMiscSeqTable&);
};

END_NCBI_SCOPE

#endif
    // _GUI_PACKAGES_EDIT__TABLE_READER_HPP_
