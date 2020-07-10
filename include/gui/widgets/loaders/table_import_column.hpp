#ifndef GUI_CORE___TABLE_IMPORT_COLUMN__HPP
#define GUI_CORE___TABLE_IMPORT_COLUMN__HPP

/*  $Id: table_import_column.hpp 30726 2014-07-16 21:15:25Z falkrb $
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

#include <gui/widgets/loaders/map_assembly_params.hpp>

#include <objects/general/User_field.hpp>

#include <gui/gui_export.h>


BEGIN_NCBI_SCOPE

USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CTableImportColumn - 
///
/// This class holds meta-data for the columns in the tabular data. The size
/// and number of columns is automatically updated when the delimiter info
/// is changed. 
///
class NCBI_GUIWIDGETS_LOADERS_EXPORT CTableImportColumn {
public:
    enum eColumnType { eSeqIdColumn=0, eNumberColumn, eRealNumberColumn,
        eTextColumn, eSkippedColumn, eTypeUndefined };

    enum eDataType { 
        eLocationID = 0, 
        eLocationGI, 
        eLocalID,
        eChromosome,
        eRsid,
        eUnspecifiedID,      // IDs
        eGeneName, 
        eGenotype,           // Encodes a variation, e.g. "CT" or "G" (x,y or MT on human)
        eStrand,
        eVariationName, 
        eSnpName, 
        eChromosomeNumber,
        eDataRegion,
        eUnspecifiedText,    // Text
        eUnspecifiedReal,    // Real Number 
        eStartPosition, 
        eStopPosition, 
        eLength, 
        eUnspecifiedInt,
        eUndefined,
        eLastType
    };

    static const char* m_TypeNameList[];
    static const char* m_ShortTypeNameList[];

public:
    CTableImportColumn() 
        : m_ColumnWidth(8)
        , m_ColumnType(eTextColumn)
        , m_OneBased(true)
        , m_Match(false)
        , m_DataType(eUndefined)
        , m_IsCurrent(false) {}

    void SetWidth(int w) { m_ColumnWidth = w; }
    int GetWidth() const { return m_ColumnWidth; }

    void SetType(eColumnType t) { m_ColumnType = t; }
    eColumnType GetType() const { return m_ColumnType; }

    void SetOneBased(bool b) { m_OneBased = b; }
    bool GetOneBased() const { return m_OneBased; }

    /// For attribute table import we need to specify a column
    /// as the column to use in matching the table against to entry
    void SetMatchColumn(bool b) { m_Match = b; }
    bool GetMatchColumn() const { return m_Match; }

    /// Get set bio-type info for the column
    void SetDataType(eDataType t) { m_DataType = t; }
    eDataType GetDataType() const { return m_DataType; }
    string GetDataTypeString() const { return GetStringFromDataType(m_DataType); }
    string GetShortDataTypeString() const { return GetShortStringFromDataType(m_DataType); }

    /// Return a string version of a data-type (e.g. "Length" for eLength)
    static string GetStringFromDataType(eDataType t);
    /// Return a short string of a data-type, and blank for 'undefined' types
    static string GetShortStringFromDataType(eDataType t);
    /// Return enum data-type from string
    static eDataType GetDataTypeFromString(const string& s);
    static vector<eDataType> GetMatchingDataTypes(eColumnType c);


    void SetName(const string& n);
    string GetName() const { return m_ColumnName; }
    string GetNameEx() const { return GetSkipped() ? "" : m_ColumnName; }

    void SetAssembly(const CMapAssemblyParams& a) { m_Assembly = a; }
    CMapAssemblyParams GetAssembly() const { return m_Assembly; }

    bool GetSkipped() const { return (m_ColumnType == eSkippedColumn); }

    void SetIsCurrent(bool b) { m_IsCurrent = b; }
    bool GetIsCurrent() const { return m_IsCurrent; }

    void SetQualifier(const string& q) { m_Qualifier = q; }
    string GetQualifier() const { return m_Qualifier; }

    void SetQualifierType(const string& q) { m_QualifierType = q; }
    string GetQualifierType() const { return m_QualifierType; }

    /// Log column parameters
    void LogColumnInfo() const;

    /// Export column info in ASN user-object format
    void SaveAsn(CUser_field& user_field) const;
    /// Import column info from ASN user-object format
    void LoadAsn(CUser_field& user_field);

    /// Add/update a column property
    void SetProperty(const string& prop_name, const string& prop_value) { m_Properties[prop_name] = prop_value; }
    bool HasProperty(const string& prop_name) { return (m_Properties.count(prop_name) > 0); }
    string GetProperty(const string& prop_name) { return (HasProperty(prop_name) ? m_Properties[prop_name] : ""); }
    map<string,string>&  GetPropertyValues() { return m_Properties; }

protected:
    /// width in chracters
    int m_ColumnWidth;

    /// Column name
    string m_ColumnName;

    /// Matching Assembly for column identfier
    CMapAssemblyParams m_Assembly;

    /// column type
    eColumnType m_ColumnType;

    /// If integer, is it one based or 0 based?
    bool m_OneBased;

    /// (For attribute table loading) Is this the match column
    bool m_Match;

    /// The semantic meaning of the column, e.g. id, start pos, stop pos, strand
    eDataType m_DataType;

    /// If true, this is the current column for editing purposes 
    /// (for views that allow you to edit column type info)
    bool m_IsCurrent;

    /// For table import in sequence editor - the qualifier field name
    string m_Qualifier;

    /// Also for sequence editor import - text version of 
    /// CFieldNamePanel::EFieldType
    string m_QualifierType;

    /// Any properties (text only) user wants to set for the column. When 
    /// converted to seq-annot, we add these to user data as &propname=prop_value
    map<string,string> m_Properties;
};


END_NCBI_SCOPE


#endif  // GUI_CORE___TABLE_IMPORT_COLUMN__HPP
