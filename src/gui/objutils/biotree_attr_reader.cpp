/*  $Id: biotree_attr_reader.cpp 36594 2016-10-12 20:17:36Z evgeniev $
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
 * Authors:  Anatoliy Kuznetsov
 *
 * File Description:
 *    BioTree attribute reader
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/ncbistr.hpp>
#include <util/line_reader.hpp>

#include <gui/objutils/biotree_attr_reader.hpp>

BEGIN_NCBI_SCOPE


CBioTreeAttrReader::CBioTreeAttrReader()
{
}


// Format description/example:
// Note that the first column will be used as the key, whether or not it
// is a seq-id
//
//
// #BKBTA-1
// #seq-id cluster-id label
// #optional comments related to file, dates, file origin, generation params, etc
//
// 9456789 12 "sequence name 1"
// 9478900 12 "sequence name 2"
// 8456789 15 "sequence name 3"


void CBioTreeAttrReader::Read(CNcbiIstream& is, TAttrTable& attr_table)
{
    CStreamLineReader line_reader(is);
    CTempString  l;

    string ls;
    vector<string> str_arr;
    int seq_id_idx = -1;

    // read the header
    //
    {{
    ++line_reader;
    CheckLineReader(line_reader);

    l = *line_reader;

    // signature match?  If not we will try to parse this line for column
    // headers
    if (l != "#BKBTA-1") {
        //NCBI_THROW2(CObjReaderParseException, eFormat,
        //            "BioTree attributes format: header signature is missing ",
        //            line_reader.GetLineNumber());   
        LOG_POST(Warning << "BioTree attributes format: header signature '#BKBTA-1' is missing ");
    }
    else {
        // found header, advance to next line for column names
        ++line_reader;
        CheckLineReader(line_reader);
    }

    // get list of columns
    ls = *line_reader;

    if (ls.empty() || ls[0] != '#') {
        NCBI_THROW2(CObjReaderParseException, eFormat,
            "BioTree attributes format: line with column names expected: #name1 name2 name3... ",
                    line_reader.GetLineNumber());        
    }
    ls.erase(0, 1); // remove '#'
    NStr::Split(ls, " \t", str_arr, NStr::fSplit_Tokenize);
    if (str_arr.size() == 0) {
        NCBI_THROW2(CObjReaderParseException, eFormat,
                    "BioTree attributes format: column name list is missing ",
                    line_reader.GetLineNumber());        
    }

    ITERATE(vector<string>, it, str_arr) {
        const string& attr_name = *it;
        attr_table.AddColumn(attr_name);
    }

    }}

    // Read the content 
    //
    {{
    vector<string> row_values;
    unsigned row = 0;

    // check eof Before reading next line since reading a valid
    // line can trigger AtEOF to be true (and you don't want to 
    // ignore the line you just read)
    bool at_eof = line_reader.AtEOF();
    for (++line_reader;!at_eof; ++line_reader) {
        at_eof = line_reader.AtEOF();
        ls = *line_reader;
        if (ls.empty() || (ls[0] == '#')) {
            continue;
        }
        row_values.resize(0);
        NStr::Split(ls, "\t", row_values);
        if (row_values.size() == 0) {
            continue;
        }

        //attr_table.Resize(row+1, (unsigned)str_arr.size());
        attr_table.AddRow(row);
        
        for (size_t i = 0; i < row_values.size(); ++i) {
            if (i >= attr_table.Cols()) {
                ERR_POST("Attribute reader too many columns at line=" << line_reader.GetLineNumber());
                break;
            }
            string& s = row_values[i];
            TableValueDecode(s);

            /*
            if (i == (size_t)seq_id_idx) {
                if (s.empty()) {
                    NCBI_THROW2(CObjReaderParseException, eFormat,
                                "BioTree attributes: seq-id attribute missing",
                                line_reader.GetLineNumber());        
                }
            }
            */

            string& tcell = attr_table.GetCell(row, (unsigned)i);
            tcell = s;
        } // for

        ++row;

    } // for
    }}

}

void CBioTreeAttrReader::TableValueDecode(string& s)
{
    NStr::TruncateSpacesInPlace(s);
    if (s.empty()) return;

    if (s[0] == '"') {
        s.erase(0, 1);
        if (s.empty()) return;
    }
    if (s[s.length()-1] == '"') {
        s.erase(s.length()-1, 1);
        if (s.empty()) return;
    }
}


void CBioTreeAttrReader::CheckLineReader(ILineReader& ilr)
{
    if (ilr.AtEOF()) {
        NCBI_THROW2(CObjReaderParseException, eEOF,
                    "Input stream no longer valid ",
                    ilr.GetLineNumber());
    }
}

END_NCBI_SCOPE
