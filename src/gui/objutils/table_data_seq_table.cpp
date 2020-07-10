/*  $Id: table_data_seq_table.cpp 32240 2015-01-22 21:53:03Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/table_data.hpp>
#include <gui/objutils/label.hpp>

#include "table_data_seq_table.hpp"

#include <objects/seqtable/Seq_table.hpp>
#include <objects/seqtable/SeqTable_column.hpp>
#include <objects/seqtable/SeqTable_column_info.hpp>
#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <objects/seqtable/SeqTable_single_data.hpp>
#include <objects/seqtable/SeqTable_sparse_index.hpp>
#include <objects/seqtable/CommonString_table.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void initCTableDataSeq_table()
{
    CInterfaceRegistry::RegisterFactory(
            typeid(ITableData).name(),
            CSeq_table::GetTypeInfo(),
            new CObjectInterfaceFactory<CTableDataSeq_table>());
}

CTableDataSeq_table* CTableDataSeq_table::CreateObject(SConstScopedObject& object, ICreateParams*)
{
    CTableDataSeq_table* table_data = new CTableDataSeq_table();
    table_data->m_Object = object.object;
    table_data->m_Scope  = object.scope;
    table_data->Init();
    return table_data;
}

void CTableDataSeq_table::Init()
{
    try {
        const CSeq_table& seq_table = dynamic_cast<const CSeq_table&>(*m_Object);
        size_t colIndex = 0;
        ITERATE(CSeq_table::TColumns, it, seq_table.GetColumns()) {
            const CSeqTable_column_info& header = (*it)->GetHeader();
            if (header.IsSetField_name() && "disabled" == header.GetField_name()) {
                if ((*it)->IsSetSparse()) {
                    const CSeqTable_sparse_index& sparse_index = (*it)->GetSparse();
                    if (sparse_index.IsIndexes()) {
                        ITERATE(CSeqTable_sparse_index::TIndexes, it2, sparse_index.GetIndexes())
                            m_InvalidRows.insert(*it2);
                    }
                }
            }
            else
                m_ColMap.push_back(colIndex);
            ++colIndex;
        }
    }
    catch (const exception& e) {
        LOG_POST(Error << "CTableDataSeq_table::Init()" << e.what());
    }
}

ITableData::ColumnType CTableDataSeq_table::GetColumnType(size_t col) const
{
    try {
        const CSeqTable_column& column = x_GetColumn(col);
        if (!column.IsSetData()) {
            if (column.IsSetDefault()) {
                const CSeqTable_single_data& defVal = column.GetDefault();
                switch(defVal.Which()) {
                    case CSeqTable_single_data::e_String:
                        return kString;
                    case CSeqTable_single_data::e_Int:
                        return kInt;
                    case CSeqTable_single_data::e_Real:
                        return kReal;
                    case CSeqTable_single_data::e_Loc:
                    case CSeqTable_single_data::e_Id:
                    case CSeqTable_single_data::e_Interval:
                        return kObject;
                    default:
                        return kNone;
                }
            }
            return kNone;
        }

        const CSeqTable_multi_data& data = column.GetData();
        switch(data.Which()) {
        case CSeqTable_multi_data_Base::e_not_set:
            return kNone;
        case CSeqTable_multi_data_Base::e_Int:
            {{
                const CSeqTable_column_info& header = column.GetHeader();
                if (header.IsSetField_id() &&
                    header.GetField_id() == CSeqTable_column_info::eField_id_location_strand) {
                    return kCommonString;
                }
            }}
            return kInt;
        case CSeqTable_multi_data_Base::e_Real:
            return kReal;
        case CSeqTable_multi_data_Base::e_String:
            return kString;
        case CSeqTable_multi_data_Base::e_Bytes:
            return kBytes;
        case CSeqTable_multi_data_Base::e_Common_string:
            return kCommonString;
        case CSeqTable_multi_data_Base::e_Common_bytes:
            return kCommonBytes;
        case CSeqTable_multi_data_Base::e_Loc:
        case CSeqTable_multi_data_Base::e_Id:
        case CSeqTable_multi_data_Base::e_Interval:
            return kObject;
        default:
            return kNone;
        }
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataSeq_table: " << e.what());
        return kNone;
    }
}

string CTableDataSeq_table::GetColumnLabel(size_t col) const
{
    string label;

    try {
        const CSeqTable_column_info& header = x_GetColumn(col).GetHeader();
        if (header.IsSetTitle()) {
            label = header.GetTitle();
        }
        else if (header.IsSetField_name()) {
            label = header.GetField_name();
        }
        else if (header.IsSetField_id()) {
            CSeqTable_column_info::TField_id field_id = header.GetField_id();
            label = CSeqTable_column_info::ENUM_METHOD_NAME(EField_id)()->FindName(field_id, true);
        }
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataSeq_table: " << e.what());
    }

    if (label.empty()) {
        label = "Column" + NStr::NumericToString(col);
    }

    return label;
}

string CTableDataSeq_table::GetRowLabel(size_t row) const
{
    if (m_InvalidRows.find(row) != m_InvalidRows.end())
        return "(invalid)";
    return "";
}

static const char* s_StrandValues[] =
{
    "Unknown",
    "Pos",
    "Neg",
    "Both",
    "Both Rev"
};


vector<string> CTableDataSeq_table::GetColumnCommonStrings(size_t col) const
{
    vector<string> strings;

    try {
        const CSeqTable_column& column = x_GetColumn(col);


        if (column.IsSetData()) {
            const CSeqTable_multi_data& data = column.GetData();
            if (data.IsCommon_string()) {
                const CCommonString_table& string_table = data.GetCommon_string();
                ITERATE(CSeqTable_multi_data::TString, it, string_table.GetStrings()) {
                    strings.push_back(*it);
                }
            }
            else if (data.IsInt()) {
                const CSeqTable_column_info& header = column.GetHeader();
                if (header.IsSetField_id() &&
                    header.GetField_id() == CSeqTable_column_info::eField_id_location_strand) {
                    for (int i = 0; (size_t)i < sizeof(s_StrandValues)/sizeof(s_StrandValues[0]); ++i)
                        strings.push_back(s_StrandValues[i]);
                }
            }
        }
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataSeq_table: " << e.what());
    }

    return strings;
}

size_t CTableDataSeq_table::GetRowsCount() const
{
    try {
        const CSeq_table& seq_table = dynamic_cast<const CSeq_table&>(*m_Object);
        return seq_table.GetNum_rows();
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataSeq_table: " << e.what());
        return 0;
    }
}

size_t CTableDataSeq_table::GetColsCount() const
{
    return m_ColMap.size();
}

const CSeqTable_column& CTableDataSeq_table::x_GetColumn(size_t col) const
{
    size_t colIndex = m_ColMap[col];
    const CSeq_table& seq_table = dynamic_cast<const CSeq_table&>(*m_Object);
    return *seq_table.GetColumns()[colIndex];
}

objects::CSeqTable_column& CTableDataSeq_table::x_GetColumn(size_t col)
{
    size_t colIndex = m_ColMap[col];
    const CSeq_table& seq_table = dynamic_cast<const CSeq_table&>(*m_Object);
    return *const_cast<CSeq_table&>(seq_table).SetColumns()[colIndex];
}

void CTableDataSeq_table::GetStringValue(size_t row, size_t col, string& value) const
{
    value.resize(0);
    try {
        const CSeqTable_column& column = x_GetColumn(col);
        if (column.IsSetData()) {
            const CSeqTable_multi_data& data = column.GetData();

            switch(data.Which()) {
                case CSeqTable_multi_data::e_Int:
                    {{
                        const CSeqTable_column_info& header = column.GetHeader();
                        if (header.IsSetField_id() &&
                            header.GetField_id() == CSeqTable_column_info::eField_id_location_strand) {
                            long strand = GetIntValue(row, col);
                            if (strand >= 0 && (size_t)strand < sizeof(s_StrandValues)/sizeof(s_StrandValues[0]))
                                value = s_StrandValues[strand];
                            else
                                value = "Other";
                        }
                        else {
                            value = NStr::NumericToString(GetIntValue(row, col));
                        }
                    }}
                    break;
                case CSeqTable_multi_data_Base::e_Real:
                    {{
                        const vector<double>& reals = data.GetReal();
                        if (row < reals.size())
                            value = NStr::DoubleToString(reals[row]);
                    }}
                    break;
                case CSeqTable_multi_data::e_Loc:
                    {{
                        vector<SLocLabel>::iterator iter = std::find(
                            m_LocLabels.begin(), m_LocLabels.end(), SLocLabel((int)col));

                        if (iter == m_LocLabels.end()) {
                            m_LocLabels.push_back(SLocLabel((int)col));
                            const CSeq_table& seq_table = dynamic_cast<const CSeq_table&>(*m_Object); 
                            m_LocLabels[m_LocLabels.size()-1].m_Labels.resize(seq_table.GetNum_rows());
                            iter = m_LocLabels.begin() + m_LocLabels.size() - 1;
                        }
                            
                        if ((*iter).m_Labels[row].empty()) {
                            string label;
                            const CSeqTable_multi_data::TLoc& locs = data.GetLoc();
                            if (row < locs.size()) {
                                const CSeq_loc& loc = *locs[row];
                                CLabel::GetLabel(loc, &label, CLabel::eContent, m_Scope);
                            }
                            (*iter).m_Labels[row] = label;
                        }
                        value = (*iter).m_Labels[row];
                    }}
                    break;
                case CSeqTable_multi_data::e_Id:
                    {{
                        const CSeqTable_multi_data::TId& ids = data.GetId();
                        if (row < ids.size()) {
                            const CSeq_id& seq_id = *ids[row];
                            CLabel::GetLabel(seq_id, &value, CLabel::eContent, m_Scope);
                        }
                    }}
                    break;
                case CSeqTable_multi_data::e_String:
                    {{
                        const CSeqTable_multi_data::TString& strings = data.GetString();
                        if (row < strings.size())
                            value = strings[row];
                    }}
                    break;
                case CSeqTable_multi_data::e_Common_string:
                    {{
                        const CCommonString_table& string_table = data.GetCommon_string();
                        const vector<int>& indexes = string_table.GetIndexes();
                        const CSeqTable_multi_data::TString& strings = string_table.GetStrings();
                        if (row < indexes.size()) {
                            int index = indexes[row];
                            if (index >= 0 && index < (int)strings.size())
                                value = strings[index];
                        }
                    }}
                    break;
                default:
                    break;
            }
        }
        else if (column.IsSetDefault()) {
            const CSeqTable_single_data& defVal = column.GetDefault();
            switch(defVal.Which()) {
                case CSeqTable_single_data::e_String:
                    value = defVal.GetString();
                    break;
                case CSeqTable_single_data::e_Int:
                    value = NStr::DoubleToString(defVal.GetInt());
                    break;
                case CSeqTable_single_data::e_Real:
                    value = NStr::DoubleToString(defVal.GetReal());
                    break;
                case CSeqTable_single_data::e_Loc:
                    CLabel::GetLabel(defVal.GetLoc(), &value, CLabel::eContent, m_Scope);
                    break;
                case CSeqTable_single_data::e_Id:
                    CLabel::GetLabel(defVal.GetId(), &value, CLabel::eContent, m_Scope);
                    break;
                case CSeqTable_single_data::e_Interval:
                    CLabel::GetLabel(defVal.GetInterval(), &value, CLabel::eContent, m_Scope);
                    break;
                default:
                    break;
            }
        }
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataSeq_table: " << e.what());
    }
}


long CTableDataSeq_table::GetIntValue(size_t row, size_t col) const
{
    long value = 0;
    try {
        bool valueSet = false;
        const CSeqTable_column& column = x_GetColumn(col);
        if (column.IsSetData()) {
            const CSeqTable_multi_data& data = column.GetData();
            if (data.Which() == CSeqTable_multi_data::e_Int) {
                const vector<int>& ints = data.GetInt();
                if (row < ints.size()) {
                    const CSeqTable_column_info& header = column.GetHeader();
                    if (header.IsSetField_id()) {
                        switch (header.GetField_id()) {
                        case CSeqTable_column_info::eField_id_location_from :
                        case CSeqTable_column_info::eField_id_location_to :
                        case CSeqTable_column_info::eField_id_product_from :
                        case CSeqTable_column_info::eField_id_product_to :
                            value = ints[row] + 1;
                            break;
                        default:
                            value = ints[row];
                            break;
                        }
                    }
                    else {
                        value = ints[row];
                    }
                    valueSet = true;
                }
            }
        }
        if (!valueSet && column.IsSetDefault()) {
            const CSeqTable_single_data& defVal = column.GetDefault();
            if (defVal.IsInt()) {
                value = defVal.GetInt();
            }
        }
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataSeq_table: " << e.what());
    }

    return value;
}

double CTableDataSeq_table::GetRealValue(size_t row, size_t col) const
{
    double value = 0;
    try {
        bool valueSet = false;
        const CSeqTable_column& column = x_GetColumn(col);
        if (column.IsSetData()) {
            const CSeqTable_multi_data& data = column.GetData();
            if (data.Which() == CSeqTable_multi_data::e_Real) {
                const vector<double>& reals = data.GetReal();
                if (row < reals.size()) {
                    value = reals[row];
                    valueSet = true;
                }
            }
        }
        if (!valueSet && column.IsSetDefault()) {
            const CSeqTable_single_data& defVal = column.GetDefault();
            if (defVal.IsReal()) {
                value = defVal.GetReal();
            }
        }
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataSeq_table: " << e.what());
    }

    return value;
}

SConstScopedObject CTableDataSeq_table::GetObjectValue(size_t row, size_t col) const
{
    SConstScopedObject value;

    try {
        const CSeqTable_column& column = x_GetColumn(col);
        if (column.IsSetData()) {
            const CSeqTable_multi_data& data = column.GetData();
            if (data.Which() == CSeqTable_multi_data::e_Loc) {
                const CSeqTable_multi_data::TLoc& locs = data.GetLoc();
                if (row < locs.size()) {
                    value.object.Reset(locs[row].GetPointer());
                    value.scope.Reset(m_Scope.GetPointer());
                }
            }
            else if (data.Which() == CSeqTable_multi_data::e_Id) {
                const CSeqTable_multi_data::TId& ids = data.GetId();
                if (row < ids.size()) {
                    value.object.Reset(ids[row].GetPointer());
                    value.scope.Reset(m_Scope.GetPointer());
                }
            }
            else if (data.Which() == CSeqTable_multi_data::e_Interval) {
                const CSeqTable_multi_data::TInterval& intervals = data.GetInterval();
                if (row < intervals.size()) {
                    value.object.Reset(intervals[row].GetPointer());
                    value.scope.Reset(m_Scope.GetPointer());
                }
            }
        }
        else if (column.IsSetDefault()) {
            const CSeqTable_single_data& defVal = column.GetDefault();
            if (defVal.IsLoc()) {
                value.object.Reset(&defVal.GetLoc());
                value.scope.Reset(m_Scope.GetPointer());
            }
            else if (defVal.IsId()) {
                value.object.Reset(&defVal.GetId());
                value.scope.Reset(m_Scope.GetPointer());
            }
            else if (defVal.IsInterval()) {
                value.object.Reset(&defVal.GetInterval());
                value.scope.Reset(m_Scope.GetPointer());
            }
        }
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataSeq_table: " << e.what());
    }

    return value;
}

bool CTableDataSeq_table::AllowEdit(size_t col)
{
    try {
        const CSeqTable_column& column = x_GetColumn(col);
        if (!column.IsSetData())
            return false;

        const CSeqTable_multi_data& data = column.GetData();
        switch(data.Which()) {
        case CSeqTable_multi_data_Base::e_String:
            return true;
        case CSeqTable_multi_data_Base::e_Int:
            return true;
        case CSeqTable_multi_data_Base::e_Real:
            return true;
        case CSeqTable_multi_data_Base::e_Common_string:
            return true;
        default:
            return false;
        }
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataSeq_table: " << e.what());
        return false;
    }
}

void CTableDataSeq_table::SetStringValue(size_t row, size_t col, const string& value)
{
    try {
        CSeqTable_column& column = x_GetColumn(col);
        if (!column.IsSetData())
            return;

        CSeqTable_multi_data& data = column.SetData();
        switch(data.Which()) {
        case CSeqTable_multi_data_Base::e_String:
            {{
                CSeqTable_multi_data::TString& strings = data.SetString();
                if (row < strings.size()) {
                    strings[row] = value;
                }
            }}
            break;
        case CSeqTable_multi_data_Base::e_Common_string:
            {{
                CCommonString_table& string_table = data.SetCommon_string();
                vector<int>& indexes = string_table.SetIndexes();
                const CSeqTable_multi_data::TString& strings = string_table.GetStrings();
                CSeqTable_multi_data::TString::const_iterator it =
                    std::find (strings.begin(), strings.end(), value);
                if (it != strings.end()) {
                    indexes[row] = (int)(it - strings.begin());
                }
            }}
            break;
        case CSeqTable_multi_data_Base::e_Int:
            {{
                const CSeqTable_column_info& header = column.GetHeader();
                if (header.IsSetField_id() &&
                    header.GetField_id() == CSeqTable_column_info::eField_id_location_strand) {
                    for (int i = 0; (size_t)i < sizeof(s_StrandValues)/sizeof(s_StrandValues[0]); ++i) {
                        if (value == s_StrandValues[i]) {
                            vector<int>& ints = data.SetInt();
                            if (row < ints.size()) {
                                ints[row] = i;
                            }
                            break;
                        }
                    }
                }
            }}
        default:
            break;
        }
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataSeq_table: " << e.what());
    }
}

void CTableDataSeq_table::SetIntValue(size_t row, size_t col, long value)
{
    try {
        CSeqTable_column& column = x_GetColumn(col);
        if (!column.IsSetData())
            return;

        CSeqTable_multi_data& data = column.SetData();
        switch(data.Which()) {
        case CSeqTable_multi_data_Base::e_Int:
            {{
                vector<int>& ints = data.SetInt();
                if (row < ints.size()) {
                    ints[row] = value;
                }
            }}
            break;
        default:
            break;
        }
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataSeq_table: " << e.what());
    }
}

void CTableDataSeq_table::SetRealValue(size_t row, size_t col, double value)
{
    try {
        CSeqTable_column& column = x_GetColumn(col);
        if (!column.IsSetData())
            return;

        CSeqTable_multi_data& data = column.SetData();
        switch(data.Which()) {
        case CSeqTable_multi_data_Base::e_Real:
            {{
                vector<double>& doubles = data.SetReal();
                if (row < doubles.size()) {
                    doubles[row] = value;
                }
            }}
            break;
        default:
            break;
        }
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataSeq_table: " << e.what());
    }
}

END_NCBI_SCOPE
