/*  $Id: grid_table_export.cpp 39039 2017-07-21 18:27:48Z katargir $
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

#include <wx/grid.h>

#include <gui/widgets/wx/csv_exporter.hpp>
#include <gui/widgets/wx/grid_table_export.hpp>

BEGIN_NCBI_SCOPE

CwxGridTableExport::CwxGridTableExport(wxGridTableBase& data, ostream& os, bool header, char delimiter, char quote)
{
    CCSVExporter exporter(os, delimiter, quote);

    int rowNum = data.GetNumberRows(), colNum = data.GetNumberCols();

    if (header) {
        for (int col = 0; col < colNum; col++) {
            exporter.Field(data.GetColLabelValue(col));
        }
        exporter.NewRow();
    }

    for (int row = 0; row < rowNum; ++row) {
        for( int col = 0; col < colNum; col++ ){
            if (data.CanGetValueAs(row, col, wxGRID_VALUE_NUMBER)) {
                long value = data.GetValueAsLong(row, col);
                exporter.Field(value);
            }
            else if (data.CanGetValueAs(row, col, wxGRID_VALUE_FLOAT)) {
                double value = data.GetValueAsDouble(row, col);
                exporter.Field(value);
            }
            else {
                wxString value = data.GetValue(row, col);
                exporter.Field(value);
            }
        }
        exporter.NewRow();
    }
}

CwxGridTableExport::CwxGridTableExport(wxGridTableBase& data, ostream& os,
                                       const vector<int>& rows, const vector<int>& cols,
                                       bool header, char delimiter, char quote)
{
    CCSVExporter exporter(os, delimiter, quote, cols.size() > 1);

    int rowNum = data.GetNumberRows(), colNum = data.GetNumberCols();

    if (header) {
        for (vector<int>::const_iterator itCol = cols.begin(); itCol != cols.end(); ++itCol) {
            int col = *itCol;
            if (col >= 0 && col < colNum) {
                exporter.Field(data.GetColLabelValue(col));
            }
        }
        exporter.NewRow();
    }

    for (vector<int>::const_iterator itRow = rows.begin(); itRow != rows.end(); ++itRow) {
        int row = *itRow;
        if (row >= 0 && row < rowNum) {
            for (vector<int>::const_iterator itCol = cols.begin(); itCol != cols.end(); ++itCol) {
                int col = *itCol;
                if (col >= 0 && col < colNum) {
                    if (data.CanGetValueAs(row, col, wxGRID_VALUE_NUMBER)) {
                        long value = data.GetValueAsLong(row, col);
                        exporter.Field(value);
                    }
                    else if (data.CanGetValueAs(row, col, wxGRID_VALUE_FLOAT)) {
                        double value = data.GetValueAsDouble(row, col);
                        exporter.Field(value);
                    }
                    else {
                        wxString value = data.GetValue(row, col);
                        exporter.Field(value);
                    }
                }
            }
            exporter.NewRow();
        }
    }
}

END_NCBI_SCOPE
