/*  $Id: align_seq_table.cpp 44757 2020-03-05 18:58:50Z evgeniev $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/hit_matrix/align_seq_table.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/label.hpp>

#include <objmgr/util/sequence.hpp>
#include <objects/seqfeat/Org_ref.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CAlignSeqTableModel
wxString CAlignSeqTableModel::GetColumnType(int /*col_index*/) const
{
    return wxT("string");
}


void CAlignSeqTableModel::Init( IHitMatrixDataSource& ds, const TIdVector& ids )
{
    Clear();

    ITERATE( TIdVector, it, ids ){
        IHitSeqId* id = *it;
        CBioseq_Handle handle = ds.GetBioseqHandle(*id);

        SRow* row = new SRow();

        row->m_Id = id;
        row->m_IdLabel = ds.GetLabel( *id );
        row->m_Title = x_GetTitle( handle );
        row->m_Organism = x_GetOrganism( handle );
        row->m_Length = x_GetLength( handle );

        m_Rows.push_back(row);
    }

    x_FireDataChanged();
}


string CAlignSeqTableModel::x_GetTitle(CBioseq_Handle& handle) const
{
    return  handle ? sequence::CDeflineGenerator().GenerateDefline(handle) : "";
}


string CAlignSeqTableModel::x_GetOrganism(CBioseq_Handle& handle) const
{
    string str;
    try {
        const COrg_ref& ref = sequence::GetOrg_ref(handle);
        ref.GetLabel(&str);
    }
    catch (CException&) {
    }
    return str;
}


string CAlignSeqTableModel::x_GetLength(CBioseq_Handle& handle) const
{
    string str = handle ? NStr::IntToString(handle.GetBioseqLength()) : "";
    return str;
}


int CAlignSeqTableModel::GetNumRows() const
{
    return (int)m_Rows.size();
}

static const int kAlignSeqColumnsNum = 4;

int CAlignSeqTableModel::GetNumColumns() const
{
    return kAlignSeqColumnsNum;
}


wxVariant CAlignSeqTableModel::GetValueAt( int row, int col ) const
{
    if( row >= 0 && row < GetNumRows() ){
        const SRow& r = *m_Rows[row];
        switch( col ){
        case 0:
           return ToWxString( r.m_IdLabel );
        case 1:
            return ToWxString( r.m_Title );
        case 2:
            return ToWxString( r.m_Organism );
        case 3:
            return ToWxString( r.m_Length );
        default:
            break;
        }
    }
    return ToWxString( "Error" );
}

static const wxString sColumns[kAlignSeqColumnsNum+1] = {
    wxT("ID"),
    wxT("Title"),
    wxT("Organism"),
    wxT("Length"),
    wxT("")
};

wxString CAlignSeqTableModel::GetColumnName( int col ) const
{
    if( col >= 0 && col < kAlignSeqColumnsNum ){
        return sColumns[col];
    }

    return sColumns[kAlignSeqColumnsNum];
}


void CAlignSeqTableModel::Clear()
{
    for( int i = 0;  i < GetNumRows(); i++ ){
        delete m_Rows[i];
    }
    m_Rows.clear();
}


const IHitSeqId* CAlignSeqTableModel::GetId( int row ) const
{
    if( row >= 0 && row < GetNumRows() ){
        return m_Rows[row]->m_Id;
    }

    return NULL;
}


END_NCBI_SCOPE
