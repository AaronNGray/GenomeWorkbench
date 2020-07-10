#ifndef GUI_WX_DEMO___TEST_TABLE_HPP
#define GUI_WX_DEMO___TEST_TABLE_HPP


/* $Id: test_table.hpp 21484 2010-06-08 00:18:11Z voronov $
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
* Authors: Igor Tolstoy
*
* File Description:
*
*/

#include <corelib/ncbistl.hpp>

#include <gui/widgets/wx/table_model.hpp>

BEGIN_NCBI_SCOPE

class CProteinModel : public CwxAbstractTableModel 
{
public :
    enum EColumns {
        eMark,
        eOrganism,
        eName,
        eUniProtName,
        eGeneLocusTag,
        eGeneSynonym,
        eGeneLocus,
        eAccession,
        eGeneID,
        eID,
        eCDDs,
        eHAMAPs,
        eKOs,
        ePubs,
        eArCOGs,
        eStructures,
        eNumColumns
    };
    CProteinModel();
    ~CProteinModel();

    virtual wxString GetColumnName( int aColIx ) const;
    virtual wxString GetColumnType( int aColIx ) const;
    virtual int GetNumColumns() const;
    virtual int GetNumRows() const;
    virtual wxVariant GetValueAt( int i, int j ) const;

    bool IsMarked( int row ) const;
    void SetMarked( int row, bool value );

private:
    void CalculateComputedFields( int first_row, int last_row );

    vector<bool> m_Marked;
    struct SComputedFields {
        string m_organism;
        string m_gene_id;
        string m_cdd;
        string m_hamap;
        string m_ko;
        int    m_pub_count;
        string m_arCOGs;
        int    m_struct_count;
        string m_uniprot;
    };
    vector<SComputedFields> m_ComputedFields;
};

END_NCBI_SCOPE

#endif  //  GUI_WX_DEMO___TEST_TABLE_HPP

