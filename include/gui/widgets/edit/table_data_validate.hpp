#ifndef GUI_WIDGETS_EDIT___TABLE_DATA_Validate__HPP
#define GUI_WIDGETS_EDIT___TABLE_DATA_Validate__HPP

/*  $Id: table_data_validate.hpp 33450 2015-07-27 18:17:17Z asztalos $
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

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <gui/objutils/table_data_base.hpp>
#include <objects/valerr/ValidError.hpp>
#include <objects/valerr/ValidErrItem.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT CTableDataValidate 
    : public CObject,
      public CTableDataBase
{
public:
    // number of entries and order must correspond with s_ColNames
    enum EColContents {
        eSeverity = 0,
        eAccessionVersion,
        eErrorTitle,
        eMessageAndObjectDesc,
        eMaxColNum
    }; // columns requested in GB-3845

    // maximal amount of Validate rows we should realistically see in a Validate table
    static const size_t c_MaxRows; // = 100000

    // width (in chars) of eMessageAndObjectDesc column
    // if wider, it will be wrapped
    static const size_t c_MsgObjectDescrWidth; // = 60

    CTableDataValidate(CObjectFor<objects::CValidError::TErrs>* errs, objects::CScope* scope);

    ~CTableDataValidate();

    /// @name ITableData interface implementation
    /// @{
    virtual ColumnType GetColumnType(size_t col) const;
    virtual bool       IsHtmlColumn(size_t col) const;
    virtual string     GetColumnLabel(size_t col) const;

    virtual size_t GetRowsCount() const;
    virtual size_t GetColsCount() const;

    virtual void GetStringValue(size_t row, size_t col, string& value) const;
    virtual string GetHtmlValue(size_t /*row*/, size_t /*col*/) const;
    virtual long   GetIntValue(size_t row, size_t col) const;
    virtual SConstScopedObject GetObjectValue(size_t row, size_t col) const;
    virtual SConstScopedObject GetRowObject(size_t row) const;
    CConstRef<objects::CValidErrItem> GetValidErrItem(size_t row) const { return x_GetRowData(row); }
    /// @}

    // IValidatorCntrl implementation
public:
    void SetSeverity(EDiagSev sev);
    EDiagSev GetSeverity() const {return m_MinSeverityToShow; };
    size_t CountBySeverity(EDiagSev sev) const;
    vector<string> GetErrorList() const;
    void SetErrCode(string errcode);
    string GetErrCode() const { return m_ErrCode; };

private:    
    CConstRef<objects::CValidErrItem> x_GetRowData(size_t row) const;
    size_t x_CountRows() const;
    bool x_MatchFilter(const objects::CValidErrItem& valitem) const;

    CRef<objects::CScope> m_Scope;

    // results of validation (obtained by the job)
    CRef<CObjectFor<objects::CValidError::TErrs> > m_Errs;

    EDiagSev m_MinSeverityToShow;
    string   m_ErrCode;
    size_t m_CurrentRowCount;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___TABLE_DATA_Validate__HPP
