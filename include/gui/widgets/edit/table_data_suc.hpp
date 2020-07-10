#ifndef GUI_WIDGETS_EDIT___TABLE_DATA_SUC__HPP
#define GUI_WIDGETS_EDIT___TABLE_DATA_SUC__HPP

/*  $Id: table_data_suc.hpp 32240 2015-01-22 21:53:03Z katargir $
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
 * Authors: Colleen Bollin
 *
 * File Description:
 *
 */
#include <gui/objutils/table_data_base.hpp>
#include <gui/widgets/edit/suc_data.hpp>

#include <objmgr/scope.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT CTableDataSUC : public CObject, public CTableDataBase
{
public:
    // number of entries and order must correspond with s_ColNames
    enum EColContents {
        eBlock = 0,
        eCount,
        eLine,
        eMaxColNum
    };

    // maximal amount of Validate rows we should realistically see in a Validate table
    static const size_t c_MaxRows; // = 100000

    // widths (in chars) of eMessage, eVerboseMessage and eObjectDesc columns
	// if wider, they will be wrapped
    static const size_t c_BlockWidth; // = 20
    static const size_t c_LineWidth; // = 100

	// objects[i].object is supposed to contain one of: Seq-entry, Seq-submit, Seq-feat, CSeq_annot_Handle, CBioSource, or CPubdesc 
	// which will be validated, anything else will be ignored
    CTableDataSUC(CSUCResults* results, objects::CScope* scope);
    virtual ~CTableDataSUC();

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

    CRef<objects::CScope> GetScope() { return m_Scope; }
    /// @}

private:
    CConstRef<CSUCLine> x_GetRowData(size_t row) const;
    CRef<objects::CScope> m_Scope;
    CRef<CSUCResults> m_SUCResults;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___TABLE_DATA_SUC__HPP
