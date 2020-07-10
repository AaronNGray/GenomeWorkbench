/*  $Id: table_data_ftable.cpp 32240 2015-01-22 21:53:03Z katargir $
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

#include <gui/objutils/label.hpp>
#include <objmgr/util/feature.hpp>

#include "table_data_ftable.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CTableDataFTable* CTableDataFTable::CreateObject(SConstScopedObject& object, ICreateParams*)
{
    CTableDataFTable* table_data = new CTableDataFTable();
    table_data->m_Object = object.object;
    table_data->m_Scope  = object.scope;
    table_data->Init();
    return table_data;
}

void CTableDataFTable::Init()
{
    const CSeq_annot& seq_annot = dynamic_cast<const CSeq_annot&>(*m_Object);
    if (seq_annot.IsFtable()) {
        const CSeq_annot::TData::TFtable& ftable = seq_annot.GetData().GetFtable();
        copy(ftable.begin(), ftable.end(), back_inserter(m_FTable));
    }
}

SConstScopedObject CTableDataFTable::GetRowObject(size_t row) const
{
    SConstScopedObject value;

    if (row < m_FTable.size()) {
        value.object = m_FTable[row];
        value.scope = m_Scope;
    }

    return value;
}

ITableData::ColumnType CTableDataFTable::GetColumnType(size_t col) const
{
    try {
        if (col == 0) {
            return kString;
        }
        else if (col == 1) {
            return kString;
        }
        else if (col == 2) {
            return kInt;
        }
        else if (col == 3) {
            return kInt;
        }
        else if (col == 4) {
            return kInt;
        }
        else if (col == 5) {
            return kString;
        }
        else if (col == 6) {
            return kString;
        }
        else if (col == 7) {
            return kInt;
        }
        return kNone;
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataFTable: " << e.what());
        return kNone;
    }
}

string CTableDataFTable::GetColumnLabel(size_t col) const
{
    string label;

    try {
        if (col == 0)
            label  = "Label";
        else if (col == 1)
            label  = "Type";
        else if (col == 2)
            label  = "Start";
        else if (col == 3)
            label  = "Stop";
        else if (col == 4)
            label  = "Length";
        else if (col == 5)
            label  = "Strand";
        else if (col == 6)
            label  = "Product";
        else if (col == 7)
            label  = "Intervals";
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataFTable: " << e.what());
    }

    return label;
}

size_t CTableDataFTable::GetRowsCount() const
{
    try {
        return m_FTable.size();
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataFTable: " << e.what());
        return 0;
    }
}

size_t CTableDataFTable::GetColsCount() const
{
    try {
        return 8;
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataFTable: " << e.what());
        return 0;
    }
}

static int s_CountIntervals( const CSeq_loc& loc ){

    switch( loc.Which() ){
    case CSeq_loc::e_Null:
    case CSeq_loc::e_Empty:
    case CSeq_loc::e_Whole:
    case CSeq_loc::e_Int:
    case CSeq_loc::e_Pnt:
    case CSeq_loc::e_Bond:
    case CSeq_loc::e_Feat:
        return 1;

    case CSeq_loc::e_Packed_int:
        return (int)loc.GetPacked_int().Get().size();

    case CSeq_loc::e_Packed_pnt:
        return (int)loc.GetPacked_pnt().GetPoints().size();

    case CSeq_loc::e_Mix:
        {{
            int ivals = 0;
            ITERATE( CSeq_loc::TMix::Tdata, iter, loc.GetMix().Get() ){
                ivals += s_CountIntervals(**iter);
            }
            return ivals;
        }}

    default:
        {{
            int size = 0;
            for( CSeq_loc_CI iter( loc );  iter;  ++iter ){
                ++size;
            }
            return size;
        }}
    }
}

void CTableDataFTable::GetStringValue(size_t row, size_t col, string& value) const
{
    value.resize(0);
    try {
        if (row < m_FTable.size()) {

            switch(col) {
            case 0 :
                CLabel::GetLabel( *m_FTable[row], &value, CLabel::eDefault, m_Scope );
                break;
            case 1 :
                feature::GetLabel( *m_FTable[row], &value, feature::fFGL_Type, m_Scope );
                break;
            case 5 : 
                {{
                    ENa_strand naStrand = sequence::GetStrand( m_FTable[row]->GetLocation() );
                    switch( naStrand ){
                    case eNa_strand_unknown:
                        value = "?";
                        break;
                    case eNa_strand_plus:
                        value = "+";
                        break;
                    case eNa_strand_minus:
                        value = "-";
                        break;
                    case eNa_strand_both:
                        value = "+-";
                        break;
                    case eNa_strand_both_rev:
                        value = "+- (rev)";
                        break;
                    case eNa_strand_other:
                        value = "other";
                        break;
                    }
                }}
                break;
            case 6 :
                if (m_FTable[row]->IsSetProduct()) {
                    const CSeq_loc& loc = m_FTable[row]->GetProduct();
                    CLabel::GetLabel( loc, &value, CLabel::eDefault, m_Scope );
                }
                break;
            default:
                break;
            }
        }
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataFTable: " << e.what());
    }
}


long CTableDataFTable::GetIntValue(size_t row, size_t col) const
{
    long value = 0;
    try {
        if (row < m_FTable.size()) {
            const CSeq_loc& loc = m_FTable[row]->GetLocation();
            CSeq_loc::TRange range = loc.GetTotalRange();
            switch(col) {
            case 2 :
                value = (int) range.GetFrom() + 1;
                break;
            case 3 :
                value = (int) range.GetTo() + 1;
                break;
            case 4 :
                value = (int) range.GetLength();
                break;
            case 7 :
                value = s_CountIntervals(loc);
                break;
            default:
                break;
            }
        }
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataFTable: " << e.what());
    }
    return value;
}

double CTableDataFTable::GetRealValue(size_t row, size_t col) const
{
    double value = 0;
    return value;
}

SConstScopedObject CTableDataFTable::GetObjectValue(size_t row, size_t col) const
{
    SConstScopedObject value;
    return value;
}

END_NCBI_SCOPE
