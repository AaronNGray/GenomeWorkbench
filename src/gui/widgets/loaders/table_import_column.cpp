/*  $Id: table_import_column.cpp 30259 2014-04-21 13:18:54Z falkrb $
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
 * Authors: Bob Falk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/table_import_column.hpp>
#include <corelib/ncbistre.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>

BEGIN_NCBI_SCOPE

USING_SCOPE(objects);

/*****************************************************************************/
/*************************** CTableImportColumn ******************************/
const char* CTableImportColumn::m_TypeNameList[] = { 
        "Accession", 
        "GI", 
        "Local ID",
        "Chromosome", 
        "Rsid",
        "Unspecified ID",
        "Gene Name", 
        "Genotype",
        "Strand",
        "Variation Name", 
        "SNP Name", 
        "Chromosome Number", 
        "Data Region",
        "Unspecified Text",
        "Unspecified Numeric",
        "Start Position", 
        "Stop Position", 
        "Length",
        "Unspecified Int",
        " "
};

string CTableImportColumn::GetStringFromDataType(eDataType t)
{
    return m_TypeNameList[(int)t];
}

const char* CTableImportColumn::m_ShortTypeNameList[] = { 
        "Accession", 
        "GI", 
        "Local ID",
        "Chromosome", 
        "Rsid",
        "ID",
        "Gene Name", 
        "Genotype",
        "Strand",
        "Variation", 
        "SNP", 
        "Chromosome",
        "Data Region",
        "",
        "",
        "Start", 
        "Stop", 
        "Length",
        "",
        " "
};

string CTableImportColumn::GetShortStringFromDataType(eDataType t)
{
    return m_ShortTypeNameList[(int)t];
}

vector<CTableImportColumn::eDataType>
CTableImportColumn::GetMatchingDataTypes(eColumnType c) 
{
    vector<CTableImportColumn::eDataType>  result;

    switch (c) {
        case eSeqIdColumn:
            result.push_back(eLocationID);
            result.push_back(eLocationGI);
            result.push_back(eLocalID);
            result.push_back(eChromosome);
            result.push_back(eRsid);
            result.push_back(eUnspecifiedID);
            break;

        case eNumberColumn:            
            result.push_back(eStartPosition);
            result.push_back(eStopPosition);
            result.push_back(eLength);
            result.push_back(eStrand);
            result.push_back(eUnspecifiedInt);
            break;

        case eRealNumberColumn:
            result.push_back(eUnspecifiedReal);
            break;

        case eTextColumn:
            result.push_back(eChromosomeNumber);
            result.push_back(eGeneName);
            result.push_back(eGenotype);
            result.push_back(eStrand); // strands can be numeric or text
            result.push_back(eVariationName);
            result.push_back(eSnpName);
            result.push_back(eDataRegion);
            result.push_back(eUnspecifiedText);
            break;

        case eSkippedColumn:
            result.push_back(eUndefined);
        default:
            break;
    };

    return result;
}

 void CTableImportColumn::SetName(const string& n) 
 { 
     m_ColumnName = n; 
 }

CTableImportColumn::eDataType 
CTableImportColumn::GetDataTypeFromString(const std::string& s)
{
    for (int i=0; i<(int)eLastType-1; ++i) {
        if (s == m_TypeNameList[i])
            return (eDataType)i;
    }

    // oops..
    _TRACE("In GetDataTypeFromString didn't find string for string: " << s);
    return eUndefined;

}

void CTableImportColumn::LogColumnInfo() const
{
    if (!m_Assembly.GetUseMapping() || m_Assembly.GetAssemblyAcc().empty()) {
        if (m_ColumnType==eNumberColumn) {
            ERR_POST(Info << m_ColumnName << " Type: " << m_ColumnType << " data-type: "
                << m_DataType << " one-based: " << m_OneBased << " Width: " 
                << m_ColumnWidth);
        }
        else {
            ERR_POST(Info << m_ColumnName << " Type: " << m_ColumnType << " data-type: "
                << m_DataType << " Width: " << m_ColumnWidth);
        }
    }
    else {
        if (m_ColumnType==eNumberColumn) {
            ERR_POST(Info << m_ColumnName << " Type: " << m_ColumnType << " data-type: "
                << m_DataType << " one-based: " << m_OneBased << " Width: " 
                << m_ColumnWidth << " Assembly: " << m_Assembly.GetAssemblyAcc());
        }
        else {
            ERR_POST(Info << m_ColumnName << " Type: " << m_ColumnType << " data-type: "
                << m_DataType << " Width: " << m_ColumnWidth << " Assembly: " 
                << m_Assembly.GetAssemblyAcc());
        }
    }
}

void CTableImportColumn::SaveAsn(CUser_field& user_field) const
{
    user_field.AddField("column-name", m_ColumnName);
    user_field.AddField("column-type", (int)m_ColumnType);
    user_field.AddField("data-type", (int)m_DataType);
    user_field.AddField("one-based", m_OneBased);
    user_field.AddField("column-width", m_ColumnWidth);

    CRef<CUser_field> assembly_object(new CUser_field());
    assembly_object->SetLabel().SetStr() = "assembly";
    m_Assembly.SaveAsn(assembly_object.GetObject());
    user_field.SetData().SetFields().push_back(assembly_object);
}

void CTableImportColumn::LoadAsn(CUser_field& user_field)
{
    if (user_field.HasField("column-name") && 
        user_field.GetField("column-name").GetData().IsStr()) {
            m_ColumnName = user_field.
                GetField("column-name").GetData().GetStr();
    }

    if (user_field.HasField("column-type") && 
        user_field.GetField("column-type").GetData().IsInt()) {
            m_ColumnType = (eColumnType)user_field.
                GetField("column-type").GetData().GetInt();
    }

    if (user_field.HasField("data-type") && 
        user_field.GetField("data-type").GetData().IsInt()) {
            m_DataType = (eDataType)user_field.
                GetField("data-type").GetData().GetInt();
    }

    if (user_field.HasField("one-based") && 
        user_field.GetField("one-based").GetData().IsBool()) {
            m_OneBased = (eDataType)user_field.
                GetField("one-based").GetData().GetBool();
    }

    if (user_field.HasField("column-width") && 
        user_field.GetField("column-width").GetData().IsInt()) {
            m_ColumnWidth = (eDataType)user_field.
                GetField("column-width").GetData().GetInt();
    }

    if (user_field.HasField("assembly")) {      
        CUser_field& assembly = user_field.SetField("assembly");
        m_Assembly.LoadAsn(assembly);
    }
}

END_NCBI_SCOPE