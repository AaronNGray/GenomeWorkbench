#ifndef GUI_CORE___TABLE_IMPORT_DATA_SOURCE__HPP
#define GUI_CORE___TABLE_IMPORT_DATA_SOURCE__HPP

/*  $Id: table_import_data_source.hpp 40063 2017-12-14 17:09:38Z katargir $
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
 * Authors:  Bob Falk
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <wx/string.h>

#include <util/icanceled.hpp>

#include <gui/widgets/loaders/table_import_column.hpp>

#include <objects/seq/Seq_annot.hpp>

#include <gui/gui_export.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CCharHistogram
///
/// This class keeps a table that defines the frequency of occurances for
/// ascii characters in tabular (field-oriented) data.  Using this occurnace 
/// frequency data the class can make an educated guess as to what character(s) 
/// are the most likely delimiters between fields, or if the data doesn't have
/// a character delimiter (in which case it may have fixed-length fields).
///
class CCharHistogram {
public:
    /// ctor
    CCharHistogram();

    /// Updates occurance data used by GetDelimiterProbabilities() based on
    /// current row.
    void UpdateHistogram(const string& row);
    /// Updates occurance data used by GetDelimiterProbabilites while
    /// ignoring characters enclosed by string delimiters, e.g. ' or ".
    void UpdateHistogramWithDelim(const string& row, char delim);
    /// First rows in files may contain headers. Record these for later analysis.
    void AddInitialRows(const string& row) { m_InitialRows.push_back(row); }

    /// Return the most likely delimiter token(s) and its weighted probability 
    /// (max_score: 0..1) that it is a separating token.  This function chooses
    /// a delimiter based on: consistent number of occurances between rows, 
    /// number of tokens created, and length of the resulting tokens. 
    void GetDelimiterProbablities(float& max_score, 
                                  vector<char>& delims,
                                  NStr::EMergeDelims& merge);
    size_t GetMaxRowLen() const { return m_CharFrequency.size(); }

    /// Capture point at which # of occurances of characters between
    /// from_char and to_char approach 'normal' levels for dataset.  This
    /// function is for finding index (if any) first non-header row.
    /// Returns -1 if no header is established.
    void GetGroupOccuranceAverage(char from_char, char to_char,
                                  float& certainty, int& first_non_header) const;

    /// Return number of characters processed so far
    int GetCharCount() const { return m_CharCount; }

    /// Set/get number of rows to record for parsing (if needed)
    void SetCaptureTarget(size_t c) { m_CaptureTarget = c; }
    size_t GetCaptureTarget() const { return m_CaptureTarget; }

protected:
    /// Number of entries in m_CharFrequency (256 + merged delimiters we check)
    static const int s_NumDelimiters = 259;

    /// Updates m_CharFrequency to reflect the current maximum row length
    void x_UpdateMaxRowLen(int len);

    /// Holds all scoring parameters for a given character
    struct CDelimScore {
        CDelimScore(int c) 
            : m_DelimChar(c)
            , m_OccuranceScore(0.0f)
            , m_FrequencyScore(0.0f)
            , m_TokenLenScore(0.0f)
            , m_CombinedScore(0.0f) {}

        /// To let us sort by highest score
        bool operator<(const CDelimScore& rhs) const { 
            if (m_CombinedScore == rhs.m_CombinedScore) 
                return (m_DelimChar > rhs.m_DelimChar);
            else 
                return m_CombinedScore < rhs.m_CombinedScore; 
        }

        bool operator>(const CDelimScore& rhs) const {
            if (m_CombinedScore == rhs.m_CombinedScore) 
                return (m_DelimChar < rhs.m_DelimChar);
            else 
                return m_CombinedScore > rhs.m_CombinedScore; 
        }

        /// The character we are scoring
        int m_DelimChar;

        /// Reflects consistency in number of time character appears in each row
        float m_OccuranceScore;
        /// Scores based on mean number of occurances per row (small #'s are bad)
        float m_FrequencyScore;
        /// Scores on tokenizing a few rows (empty or single-char tokens are worse)
        float m_TokenLenScore;
        ///  A weighted combination of the above (m_OccuranceScore is most impt)
        float m_CombinedScore;
    };

    /// The number of rows that went into updating the histogram (this is
    /// the number of times that UpdateHistogram[WithDelim] was called)
    int m_RowCount;

    /// Total number of characters that went into updating the histogram.
    /// Used to compute average row length (m_CharCount/m_RowCount)
    int m_CharCount;    

    /// Hold properties for a single character or set of characters that are
    /// candidates for merging, e.g. where adjacent delimiters do not indicate
    /// an empty field, but should instead be merged into a single delimiter.
    struct CMergedChar {
        CMergedChar(int idx) : m_FreqArrayIdx(idx), m_PrevMatch(false) {}
        CMergedChar() : m_FreqArrayIdx(-1), m_PrevMatch(false) {}

        /// Return true if current character follows a string of 1 or more chars
        /// from  m_Chars
        bool NextChar(char c) { 
            bool repeat_end = false;
            bool match = (m_Chars.find(c, 0) != string::npos);
            if (!match && m_PrevMatch)
                repeat_end = true;

            m_PrevMatch = match;
            return repeat_end;
        }

        void NewLine() { m_PrevMatch = false; }

        bool operator==(const CMergedChar& rhs) const 
            { return m_FreqArrayIdx == rhs.m_FreqArrayIdx; }

        string m_Chars;
        int m_FreqArrayIdx;
        bool m_PrevMatch;
    };

    vector<CMergedChar> m_Repeats;

    /// The outer vector m_CharFrequency[i] represents the number of times a
    /// character occurs 'i' times among the processed rows. The size of the
    /// vector is equal to the length of the longest row since no character
    /// can appear more often than that.  The inner vector is always size  
    /// s_NumDelimiters, matching the corresponding ascii chracters + merged
    /// delimiter types.
    vector<vector<int> >  m_CharFrequency;

    /// Number of rows we will try to capture (max)
    size_t m_CaptureTarget;

    /// Up to m_CaptureTarget rows recorded in calls to UpdateHistogram*()
    vector<string> m_RecordedRows;

    /// Set of initial rows in file (may contain headers)
    vector<string> m_InitialRows;
};

///////////////////////////////////////////////////////////////////////////////
/// CTableDelimiterRules - 
///
/// Hold all the delimiter information used to break up a table entry into
/// separate fields.  This includes the delimiter characters, how quoted 
/// strings are handled, and whether delimiters should be merged.
///
class NCBI_GUIWIDGETS_LOADERS_EXPORT CTableDelimiterRules {
public:
    /// Ctor defaults to not having quotes
    CTableDelimiterRules() 
        : m_QuoteChar(' ')        
        , m_MultiLineQuotes(false)
        , m_MergeDelimiters(false)
        , m_MultipleSpacesOnly(false) {}

    /// Return true if the delimiters match (even if order is different)
    bool MatchingDelimiters(vector<char> other_delims) const;

    /// Clear delimiters and set all values to defaults
    void Reset() { *this = CTableDelimiterRules(); }

    /// Get/set delimiter characters
    void SetDelimiters(const vector<char>& d) { m_Delimiters = d; }
    const vector<char>& GetDelimiters() const { return m_Delimiters; }

    /// Get set quote character used for quoting strings
    void SetQuoteChar(char c) { m_QuoteChar = c; }
    char GetQuoteChar() const { return m_QuoteChar; }

    /// Get/set merge delimiters rule for tokenizing table into fields
    void SetMergeDelimiters(bool b) { m_MergeDelimiters = b; }
    bool GetMergeDelimiters() const { return m_MergeDelimiters; }
    
    /// Get/set merge delimiters rule for tokenizing table into fields
    void SetMultipleSpacesOnly(bool b) { m_MultipleSpacesOnly = b; }
    bool GetMultipleSpacesOnly() const { return m_MultipleSpacesOnly; }

    /// Write delims information to log
    void LogDelims() const;

    /// Export delimiter rules in ASN user-object format
    void SaveAsn(CUser_field& user_field) const;

    /// Import delimiter rules from ASN user-object format
    void LoadAsn(CUser_field& delimiter_object);

protected:
    /// One or more single characters that divide the table entries into
    /// separate fields
    vector<char> m_Delimiters;

    /// Any delimiters inside strings enclosed in m_QuoteChar are not considered
    /// as field separators.  Valid values are: [blank, ", ']
    char m_QuoteChar;

    /// CSV formats allows quotes to extend over multiple lines. This flag 
    /// tells us to look for that (not yet supported)
    bool m_MultiLineQuotes;

    /// If m_MergeDelimiters is true, adjacent delimiters (characters from 
    /// m_Delimiters) next to each other are treated as a single delimiter
    bool m_MergeDelimiters;

    /// This is a special delimiter for spaces only - more than one blank
    /// is considered a delimiter but a single blank space is not. 
    /// One of the characters in m_Delimiters must be ' '.
    bool m_MultipleSpacesOnly;
};

///////////////////////////////////////////////////////////////////////////////
/// CTableImportRow - 
///
/// This class represents a single row in the table input. The data is
/// subdivided into fields via m_Fields which holds the starting point and
/// length of each field in the record.  These are used by GetField(int) to
/// return a specific field.
///
class  NCBI_GUIWIDGETS_LOADERS_EXPORT  CTableImportRow
{
public:
    /// ctor
    CTableImportRow() : m_RowNum(-1) {}

    /// Adds a single field to m_Fields for the full length of 's'.
    CTableImportRow(string& s);

    /// Set/get row number
    void SetRowNum(int n) { m_RowNum = n; }
    int GetRowNum() const { return m_RowNum; }

    /// Get the entire row
    const string& GetValue() const { return m_TableEntry; }
    string& GetValue() { return m_TableEntry; }

    /// Get the number of fields in this row
    size_t GetNumFields() const { return m_Fields.size(); }

    /// Get the array of field start/length pairs
    vector<pair<size_t,size_t> >& GetFields() { return m_Fields; }

    /// Get a specific field or "" if column_idx > m_Fields.size()
    string GetField(int column_idx) const;

protected:
    /// Number in the list of all rows minus any preceeding header and/orcomment
    /// rows.  Header and comment rows have a row number of -1.
    int m_RowNum;

    /// String content of this row in the table
    string m_TableEntry;
    
    /// Each field is entry represents a field as a start/length pair.  
    vector<pair<size_t,size_t> > m_Fields;
};

///////////////////////////////////////////////////////////////////////////////
/// CTableImportDataSource - 
///
/// This is a data source for tabular data being read in from a (possibly) CSV
/// formatted file.  The data is assumed to be divided up into fields based
/// on some (initially unknowen) delimiter or to be divided into fixed-length
/// fields.  As the data is read in it ia anyalyzed and a guess is made as to
/// the most likely delimiter (which may be no delimiter implying the data is
/// fixed-field format).
///
/// The data source also holds an array of CTableImport columns which are
/// created and set to default values initially to match the number of fields
/// but then can be updated to specify the datatype of the underying table.
//
class  NCBI_GUIWIDGETS_LOADERS_EXPORT  CTableImportDataSource : public CObject
{
public:
    /// Possible delimiter options
    enum EFieldSeparatorType {
        eDelimitedTable = 0,
        eFixedWidthTable
    };

    /// Possible file types - allows special processing for table types
    /// that are not totally generic..
    enum ETableFileType {
        eBlastResultsFile = 0,
        eUndefinedFile
    };

public:
    /// ctor
    CTableImportDataSource();

    /// clears all columns rows and delimiters
    void ClearTable();

    bool LoadTable(const wxString& fname, CUser_object& user_object);

    /// Loads the entire table from ifs and updates delimiter and column
    /// data based on delimiter guess
    bool LoadTable(const wxString& fname, Int8 filesize=-1, ICanceled* call=NULL);

    /// Save possible edited table (edits possible w/merge-split cols and row edits)
    void SaveTable(CNcbiOfstream& ofs);

    /// Get name of file last passed to LoadTable
    wxString GetFileName() const { return m_FileName; }

    /// Update current table type
    void SetTableType(EFieldSeparatorType e);
    EFieldSeparatorType GetTableType() const { return m_TableType; }
    
    /// Updated the individual rows and columns to match the current
    /// delimiter choice. Requires parsing all the rows (otherwise you can't
    /// know how many columns there are or wide those columns should be).
    void RecomputeFields(bool recreate_columns, int recompute_count = -1);
    
    /// Do same but only for 1 row
    void RecomputeRowFields(size_t row_idx);

    /// Update fields in rows to reflect column widths in
    /// fixed tables (use character widths in m_Columns)
    void ExtractFixedFields();

    /// Log fixed field widths
    void LogFixedFieldWidths() const;

    /// Update columns to genereated names or names parsed from row 
    /// m_ColumnHeaderRow
    void RecomputeHeaders();

    /// Return copy of current delimiter(s)
    const vector<char>& GetDelimiters() const { return m_DelimRules.GetDelimiters(); }

    /// Get set all delimiter rules (for delimited tables)
    //void SetDelimiterRules(CTableDelimiterRules& r);
    const CTableDelimiterRules& GetDelimiterRules() const { return m_DelimRules; }
    CTableDelimiterRules& GetDelimiterRules() { return m_DelimRules; }

    /// return total number of rows read
    size_t GetNumRows() const { return m_TableEntries.size(); }
    
    /// return a specific field from a specific row, based on current table type
    /// and delimiter
    string GetField(size_t row, size_t col) const;

    /// Return length of longest row in m_TableEntries
    size_t GetMaxRowLength() const { return m_MaxRowLen; }
    /// Get the maximum length of the 'header' rows which will not be imported
    size_t GetMaxNonImportedRowLength() const { return m_MaxNonImportedRowLength; }

    /// Return a specific row
    const CTableImportRow& GetRow(size_t row) const { return m_TableEntries[row]; }
    CTableImportRow& GetRow(size_t row) { return m_TableEntries[row]; }

    /// Get width of specified column
    int GetColumnWidth(size_t col) const 
        { return (col<m_Columns.size()) ? m_Columns[col].GetWidth() : 0; }
    /// Return name of specified column
    string GetColumnName(size_t col) const 
        { return (col<m_Columns.size()) ? m_Columns[col].GetName() : 0; }

    /// Return the specified column
    const CTableImportColumn& GetColumn(size_t col) const 
        { _ASSERT(col<m_Columns.size()); return m_Columns[col]; }
    CTableImportColumn& GetColumn(size_t col)
        { _ASSERT(col<m_Columns.size()); return m_Columns[col]; }

    /// return the array of column data
    vector<CTableImportColumn>& GetColumns() { return m_Columns; }
    const vector<CTableImportColumn>& GetColumns() const { return m_Columns; }

    /// set/get first row for import (0-based)
    void SetFirstImportRow(int r);
    int GetFirstImportRow() const { return m_ImportFromRow; }

    /// set/get (optional) row from which to parse column names
    void SetColumnHeaderRow(int c);
    int GetColumnHeaderRow() const { return m_ColumnHeaderRow; }

    /// Set column header row and first row (more efficient when updating both)
    void SetHeaderAndFirstRow(int column_header_row, int first_row);

    /// Get the total # of rows imported (row num>m_ImportFromRow and not 
    /// starting with m_CommentChar
    int GetNumImportedRows() const { return m_NumImportedRows; }

    /// set/get comment character (lines beginning with this are not imported)
    void SetCommentChar(char c);
    char GetCommentChar() const { return m_CommentChar; }

    /// Replace all delimiter characters separating cols in the array
    /// 'col_indices' with the delimiter char 'ch' to merge the cols
    void MergeColumns(vector<size_t> col_indices, char ch, 
                      bool no_merge_char=false);

    /// Split column col_idx into 2 columns using the character 'ch' (if 'ch' does
    /// not appear in the column, no split occurs)
    bool SplitColumn(size_t col_idx, char ch, bool split_on_whitespace=false);

    /// Replace all instances of multiple spaces with the current delimiter
    bool ReplaceSpaces();

    /// Save data in table into annot_container. 
    void ConvertToSeqAnnot(CRef<CSeq_annot> annot_container);

    /// Log column information
    void LogColumnInfo() const;

    /// Export table load parameters in ASN user-data format
    void ExportTableParms(CUser_object& user_object);

    /// Import table load parameters in ASN user-data format
    void ImportTableParms(CUser_object& user_object);
 
protected:

    /// After loading rows, this tests for any distinctive file types (which would allow
    /// delimiter and header information to be set correctly).  This does not check
    /// for all standard file types (see CFormatGuesser), just ones intended to be
    /// loaded via table import.  Returns true if type is not eUndefinedFile.
    bool x_PickFileType();

    /// Examines in put data and makes a best-guest at how many header rows there are
    /// and if there is a specific leading comment character for header rows.
    void x_FindHeaderRows(const CCharHistogram& hist);

    /// Parse fields from 'str' returning position and lengths, respectively,
    /// of parsed fields in token_pos.  Based on NStr::Tokenize
    static void x_ParseEntry(const CTempString& str,
                             const CTempString& delim,
                             NStr::EMergeDelims merge,
                             bool               multiple_spaces_only,
                             vector<pair<size_t,size_t> >& token_pos);

    /// Parse fields from 'str' returning position and lengths, respectively,
    /// of parsed fields in token_pos.  Do not parse any tokens from parts of
    /// 'str' enclosed in quote character 'quote_char'.  (and quote_char must
    /// be appended to 'delim' in 'delim_and_quote'.  Based on NStr::Tokenize
    static void x_ParseQuotedEntry(const CTempString& tr,
                                   const CTempString& delim,
                                   const CTempString& delim_and_quote,
                                   NStr::EMergeDelims merge,
                                   bool               multiple_spaces_only,
                                   char               quote_char,                      
                                   vector<pair<size_t,size_t> >& token_pos);

    /// Recompute the fields for the provided row
    void x_RecomputeRowFields(CTableImportRow& row,
                              const CTempString& delims_ts,
                              const CTempString& delims_quote_ts,
                              NStr::EMergeDelims merge_delims,
                              bool multiple_spaces_only);

    /// Name of file from which table was loaded
    wxString m_FileName;

    /// Tells if table fields are delimited by characters or are fixed width
    EFieldSeparatorType  m_TableType;
    /// The underlying file type (or undefined)
    ETableFileType m_FileType;

    /// Maximum row length - useful for displaying data in single-column mode
    size_t m_MaxRowLen;

    /// For character-delimited tables, the delimiter character(s), merge rule,
    /// and quote-handling option
    CTableDelimiterRules m_DelimRules;

    /// Descriptors for columns
    vector<CTableImportColumn> m_Columns;

    /// Holds a string and field position and size for each line from the file
    vector<CTableImportRow> m_TableEntries;

    /// A generic mechanism to avoid loading comment or header column rows
    /// at the top of the file - a user-settable parameter which is the first
    /// row to load.
    int m_ImportFromRow;

    /// Number of rows to actually be imported. Excludes rows with row number <
    /// m_ImportFromRow and rows that start with m_CommentChar
    int m_NumImportedRows;

    /// Optional comment character - 
    /// lines beginning with this character are not imported
    char m_CommentChar;

    /// Rows that are not imported are displayed differently so it's helpful
    /// to know their maximum width (in characters) to help with layout
    size_t m_MaxNonImportedRowLength;

    /// If != -1, parse selected row to get column names
    int m_ColumnHeaderRow;

    /// If true we should use pre-determined delimiters when loading
    bool m_UseCurrentDelimiters;

};

END_NCBI_SCOPE


#endif  // GUI_CORE___TABLE_IMPORT_DATA_SOURCE__HPP

