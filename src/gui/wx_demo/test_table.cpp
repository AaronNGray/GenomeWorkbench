/*  $Id: test_table.cpp 25633 2012-04-13 20:51:47Z katargir $
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
*
* File Description:
*
*/

#include <ncbi_pch.hpp>

#include <gui/wx_demo/test_table.hpp>

BEGIN_NCBI_SCOPE

wxString   CProteinModel::GetColumnName( int aColIx ) const
{
    static const wxString strMark		( wxT("Mark") );
    static const wxString strOrganism		( wxT("Organism") );
    static const wxString strName			( wxT("Name") );
    static const wxString strUniProtName	( wxT("UniProtName") );
    static const wxString strGeneLocusTag	( wxT("Gene Locus tag") );
    static const wxString strGeneSynonym	( wxT("Gene Synonym") );
    static const wxString strGeneLocus	( wxT("Gene Locus") );
    static const wxString strAccession	( wxT("Accession") );
    static const wxString strGeneID		( wxT("Gene ID") );
    static const wxString strID			( wxT("ID") );
    static const wxString strCDDs			( wxT("CDDs") );
    static const wxString strHAMAPs		( wxT("HAMAPs") );
    static const wxString strKOs			( wxT("KOs") );
    static const wxString strPubs			( wxT("Pubs") );
    static const wxString strArCOGs		( wxT("arCOGs") );
    static const wxString strStructures		( wxT("Structures") );
    static const wxString strUndefined    ( wxT("Undefined Column") );

    switch (EColumns(aColIx)) {
        case eMark:		    return strMark;
        case eOrganism:		return strOrganism;
        case eName:			return strName;
        case eUniProtName:	return strUniProtName;
        case eGeneLocusTag:	return strGeneLocusTag;
        case eGeneSynonym:	return strGeneSynonym;
        case eGeneLocus:	return strGeneLocus;
        case eAccession:	return strAccession;
        case eGeneID:		return strGeneID;
        case eID:			return strID;
        case eCDDs:			return strCDDs;
        case eHAMAPs:		return strHAMAPs;
        case eKOs:			return strKOs;
        case ePubs:			return strPubs;
        case eArCOGs:		return strArCOGs;
        case eStructures:   return strStructures;
        case eNumColumns:	break;
    }
    return strUndefined;
}

CProteinModel::CProteinModel()
{
    m_Marked.resize( 5, false );
}

CProteinModel::~CProteinModel()
{
}

wxString CProteinModel::GetColumnType( int aColIx ) const
{
    switch (EColumns(aColIx)) {
        case eMark:		    return wxT("bool");
        case eOrganism:		return wxT("string");
        case eName:			return wxT("string");
        case eUniProtName:	return wxT("string");
        case eGeneLocusTag:	return wxT("string");
        case eGeneSynonym:	return wxT("string");
        case eGeneLocus:	return wxT("string");
        case eAccession:	return wxT("string");
        case eGeneID:		return wxT("string");
        case eID:			return wxT("int");
        case eCDDs:			return wxT("string");
        case eHAMAPs:		return wxT("string");
        case eKOs:			return wxT("string");
        case ePubs:			return wxT("int");
        case eArCOGs:		return wxT("string");
        case eStructures:   return wxT("int");
        case eNumColumns:	break;
    }
    return wxT("string");
}

int CProteinModel::GetNumColumns() const
{
    return eNumColumns;
}


int CProteinModel::GetNumRows() const
{
    return 5;
}

bool CProteinModel::IsMarked(int row) const
{
    return m_Marked[row];
}
void CProteinModel::SetMarked(int row, bool value)
{
    m_Marked[row] = value;
}

static int seed = 1;

wxVariant CProteinModel::GetValueAt( int row, int column ) const
{
    if( column == eMark ){
        return IsMarked(row);
    }

    wxString type = GetColumnType( column );
    if( type == wxT("int") ){
        return seed++;
    } 

    return wxString::Format( wxT("Str: %i"), seed++ );
}

END_NCBI_SCOPE


