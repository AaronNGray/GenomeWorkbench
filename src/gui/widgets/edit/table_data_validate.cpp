/*  $Id: table_data_validate.cpp 37305 2016-12-22 14:24:03Z filippov $
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
 * Authors: Dmitry Rudnev
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/edit/table_data_validate.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

// maximal amount of Validate we should realistically see in a Validate table
const size_t CTableDataValidate::c_MaxRows(100000);

// widths (in chars) of eMessageAndObjectDesc columns
// if wider, it will be wrapped
const size_t CTableDataValidate::c_MsgObjectDescrWidth(60);

CTableDataValidate::CTableDataValidate(CObjectFor<objects::CValidError::TErrs>* errs, objects::CScope* scope)
    : m_Scope(scope), m_Errs(errs), m_MinSeverityToShow(eDiagSevMin)
{
    m_CurrentRowCount = x_CountRows();
}

CTableDataValidate::~CTableDataValidate()
{
}

void CTableDataValidate::SetSeverity(EDiagSev sev)
{
    m_MinSeverityToShow = sev;
    m_CurrentRowCount = x_CountRows();
}


void CTableDataValidate::SetErrCode(string errcode)
{
    m_ErrCode = errcode;
    if (NStr::EqualNocase(m_ErrCode, "ALL")) {
        m_ErrCode = "";
    }

    m_CurrentRowCount = x_CountRows();
}


size_t CTableDataValidate::CountBySeverity(EDiagSev sev) const
{
    size_t count = 0;

    for (size_t row = 0; row < m_Errs->GetData().size(); row++) {
        CConstRef<CValidErrItem> pValidErrItem(m_Errs->GetData()[row]);
        if (pValidErrItem->GetSeverity() == sev) {
            count++;
        }
    }
    return count;
}


vector<string> CTableDataValidate::GetErrorList() const
{
    vector<string> list;

    for (size_t row = 0; row < m_Errs->GetData().size(); row++) {
        CConstRef<CValidErrItem> pValidErrItem(m_Errs->GetData()[row]);
        list.push_back(pValidErrItem->GetErrCode());
    }
    sort(list.begin(), list.end());
    vector<string>::iterator final = unique(list.begin(), list.end());
    list.resize( std::distance(list.begin(), final) );

    return list; 
}


bool CTableDataValidate::x_MatchFilter(const objects::CValidErrItem& valitem) const
{
    if (valitem.GetSeverity() >= m_MinSeverityToShow &&
        (NStr::IsBlank(m_ErrCode) || NStr::EqualNocase(m_ErrCode, valitem.GetErrCode()))) {
        return true;
    } else {
        return false;
    }
}


size_t CTableDataValidate::x_CountRows() const
{
    size_t max = 0;
    for (size_t row = 0; row < m_Errs->GetData().size(); row++) {
        CConstRef<CValidErrItem> pValidErrItem(m_Errs->GetData()[row]);
        if (pValidErrItem && x_MatchFilter(*pValidErrItem)) {
            max++;
        }
    }
    return max;
}


// number of entries and order must correspond with CTableDataValidate::EColContents
static const string s_ColNames[] = {
    "Severity",
    "Sequence",
    "Error title",
    "Message & Object Description"
};

ITableData::ColumnType CTableDataValidate::GetColumnType(size_t col) const
{
    return (col < eMaxColNum) ? ITableData::kString : ITableData::kNone;
}


bool CTableDataValidate::IsHtmlColumn(size_t col) const
{
    return false;
}

string CTableDataValidate::GetColumnLabel(size_t col) const
{
    if (col < CTableDataValidate::eMaxColNum) {
        return s_ColNames[col];
    }

    return kEmptyStr;
}


size_t CTableDataValidate::GetRowsCount() const
{
    return m_CurrentRowCount;
}


size_t CTableDataValidate::GetColsCount() const
{
    return (size_t)CTableDataValidate::eMaxColNum;
}


void CTableDataValidate::GetStringValue(size_t row, size_t col, string& value) const
{
    value.clear();

    if (row < GetRowsCount()) {
        CConstRef<CValidErrItem> pValidErrItem(x_GetRowData(row));

        switch(col){
            case eSeverity:
                value = CValidErrItem::ConvertSeverity(pValidErrItem->GetSeverity());
                if (pValidErrItem->GetSeverity() == eDiag_Critical)
                    value = "Reject"; // per GB-6508
                break;

            case eMessageAndObjectDesc:
                value = pValidErrItem->CanGetMsg() ? pValidErrItem->GetMsg() + "; " : "";
                value.append(pValidErrItem->CanGetObjDesc() ? pValidErrItem->GetObjDesc() : "");
                break;
            
            case eErrorTitle:
                value = (pValidErrItem->CanGetErrorGroup() ? pValidErrItem->GetErrorGroup() + " " : "") + 
                        (pValidErrItem->CanGetErrorName() ? pValidErrItem->GetErrorName() : "");
                break;

            case eAccessionVersion:
                value = pValidErrItem->CanGetAccnver() ? pValidErrItem->GetAccnver() : "";
                break;

            default:
                break;
        }
    }

}


string CTableDataValidate::GetHtmlValue(size_t row, size_t col) const
{
    list<string> values;
    string sDelimiter("<br>");
    if (row < GetRowsCount()) {
        CConstRef<CValidErrItem> pValidErrItem(x_GetRowData(row));

        switch(col){
            // for now, this column is not an html column
            case eMessageAndObjectDesc:
                {
                    string val = pValidErrItem->CanGetMsg() ? pValidErrItem->GetMsg() + "; " : "";
                    val.append(pValidErrItem->CanGetObjDesc() ? pValidErrItem->GetObjDesc() : "");
                    NStr::Wrap(val, c_MsgObjectDescrWidth, values);
                    break;
                }
           
            default:
                break;
        }
    }
    return NStr::Join(values, sDelimiter);
}


long CTableDataValidate::GetIntValue(size_t row, size_t col) const
{
    return 0L;
}


SConstScopedObject CTableDataValidate::GetObjectValue(size_t row, size_t col) const
{
    return GetRowObject(row);
}


SConstScopedObject CTableDataValidate::GetRowObject(size_t row) const
{
    SConstScopedObject value;
    if (row < GetRowsCount()) {
        CConstRef<CValidErrItem> err_item = x_GetRowData(row);
        if (err_item) {
            //!! this is not quite correct since there may be several objects 
            //!! differently scoped
            value.scope = m_Scope;
            value.object = CConstRef<CSerialObject>(&err_item->GetObject());
        }
    }
    return value;
}


CConstRef<CValidErrItem> CTableDataValidate::x_GetRowData(size_t row) const
{
    if (!m_Errs) {
        return CConstRef<CValidErrItem>();
    }
    size_t pos = 0;
    size_t offset = 0;
    CConstRef<CValidErrItem> pValidErrItem(NULL);
    while (pos < row + 1 && offset < m_Errs->GetData().size()) {
        pValidErrItem.Reset(m_Errs->GetData()[offset]);
        if (pValidErrItem && x_MatchFilter(*pValidErrItem)) {
            pos++;
        }
        offset++;
    }
    if (pos != row + 1) {
        pValidErrItem.Reset(NULL);
    }
    return pValidErrItem;
}


END_NCBI_SCOPE

