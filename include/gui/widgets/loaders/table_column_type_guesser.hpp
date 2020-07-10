#ifndef GUI_CORE___COLUMN_TYPE_GUESSER__HPP
#define GUI_CORE___COLUMN_TYPE_GUESSER__HPP

/*  $Id: table_column_type_guesser.hpp 40055 2017-12-13 22:31:15Z katargir $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  `This software/database is freely available
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
 * Authors:  Bob Falk
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/loaders/table_import_data_source.hpp>

#include <gui/widgets/wx/async_call.hpp>


#include <objmgr/scope.hpp>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CTableColumnTypeGuesser - 
///
/// Guess types for columns by iterating over some fixed number of rows and 
/// trying to see both what the underlying type is (int, float, string..) as
/// while as the biological (seamntic) type, e.g. ID, start position, strand etc.
class NCBI_GUIWIDGETS_LOADERS_EXPORT CTableColumnTypeGuesser
{
public:
    /// ctor
    CTableColumnTypeGuesser(CRef<CTableImportDataSource> ds);

    /// dtor
    virtual ~CTableColumnTypeGuesser() {}

    /// Update columns in data source with type info based on best-guesses
    void GuessColumns();

    /// Iterate over the data in a specific column (but not necessarily all rows) to 
    /// guess type information
    void GuessColumn(size_t col_num);

    /// This takes the column data specified in the table and checks if it is
    /// compatible with the data in the column (can be used to validate
    /// a guess made by a user)
    void ValidateColumn(int col) {}


protected:
    /// Guess the type for a specific field (string)
    CTableImportColumn::eColumnType x_GuessType(const string& field,
        CTableImportColumn::eDataType& dt);


    /// Data table to be rendered in the list
    CRef<CTableImportDataSource> m_ImportedTableData;

    /// Scope is used in guessing what kind of ids we have
    CRef<objects::CScope> m_Scope;
};

END_NCBI_SCOPE

#endif // GUI_CORE___COLUMN_TYPE_GUESSER__HPP
